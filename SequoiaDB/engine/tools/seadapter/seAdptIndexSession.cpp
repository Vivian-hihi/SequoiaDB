/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = seAdptIndexSession.cpp

   Descriptive Name = Index session on search engine adapter.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#include "msgMessage.hpp"
#include "seAdptIndexSession.hpp"
#include "seAdptMgr.hpp"

#define SEADPT_FIELD_NAME_RID        "_rid"
#define SEADPT_NORMAL_CL_DONE_ID     "SDBCOMMIT"
#define SEADPT_NORMAL_CL_DONE_BODY   "{}"

namespace engine
{
   #define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
   const CHAR *_statDescription[] =
   {
      "Begin",
      "Update collection version",
      "Consult",
      "Query last logical id in capped collection",
      "Query normal collection",
      "Query capped collection",
      "Pop capped collection"
   } ;
   const CHAR* _seadptStatus2Desp( SEADPT_SESSION_STATUS status )
   {
      SDB_ASSERT( ARRAY_SIZE(_statDescription) ==
                  ( SEADPT_SESSION_STAT_MAX - 1 ),
                  "Stat enum and description size dose not match" ) ;
      if ( status > sizeof( _statDescription ) )
      {
         return NULL ;
      }

      return _statDescription[ status - 1 ] ;
   }

   BEGIN_OBJ_MSG_MAP( _seAdptIndexSession, _pmdAsyncSession)
      ON_MSG( MSG_BS_QUERY_RES, _onResMsg )
      ON_MSG( MSG_BS_GETMORE_RES, _onResMsg )
      ON_MSG( MSG_CAT_QUERY_CATALOG_RSP, _onCatalogResMsg )
   END_OBJ_MSG_MAP()

   _seAdptIndexSession::_seAdptIndexSession( UINT64 sessionID,
                                             netRouteAgent *rtAgent,
                                             const seIndexTask *task )
   : _pmdAsyncSession( sessionID )
   {
      SDB_ASSERT( rtAgent && task, "Route agent or index task is NULL" ) ;
      _esClt = NULL ;
      _rtAgent = rtAgent ;
      _status = SEADPT_SESSION_STAT_BEGIN ;
      _startLID = -1 ;
      _lastPopLID = -1 ;
      _requestID = 1 ;
      _quit = FALSE ;
      _origCLFullName = task->_origCSName + "." + task->_origCLName ;
      _cappedCLFullName = task->_cappedCSName + "." + task->_cappedCLName ;
      _origIdxName = task->_origIdxName ;
      _indexName = task->_esIdxName ;
      _typeName = task->_esTypeName ;
      _indexDef = task->_indexDef.copy() ;
      _queryCtxID = -1 ;
      _queryBusy = FALSE ;
   }

   _seAdptIndexSession::~_seAdptIndexSession()
   {
      if ( _esClt )
      {
         sdbGetSeAdapterCB()->getSeCltMgr()->releaseClt( &_esClt ) ;
      }
   }

   SDB_SESSION_TYPE _seAdptIndexSession::sessionType() const
   {
      return SDB_SESSION_SE_INDEX ;
   }

   EDU_TYPES _seAdptIndexSession::eduType() const
   {
      return EDU_TYPE_SE_INDEX ;
   }

   INT32 _seAdptIndexSession::handleQueryRes( NET_HANDLE handle,
                                              MsgHeader* msg )
   {
      INT32 rc = SDB_OK ;
      MsgOpReply *reply = ( MsgOpReply * )msg ;
      INT32 flag = 0 ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> docObjs ;

      // When get the reply of query, go on to send a get more request.
      if ( SDB_DMS_EOC == reply->flags )
      {
         _onSDBEOC() ;
         goto done ;
      }
      else if ( SDB_DMS_CS_NOTEXIST == reply->flags ||
                SDB_DMS_NOTEXIST == reply->flags )
      {
         _quit = TRUE ;
         PD_LOG( PDEVENT, "Query target collection[ %s ] dose not exist any "
                 "more. Task ready to exit",
                 ( SEADPT_SESSION_STAT_QUERY_NORMAL_TBL == _status ) ?
                 _origCLFullName.c_str() : _cappedCLFullName.c_str() ) ;
         rc = _ensureESClt() ;
         PD_RC_CHECK( rc, PDERROR, "The search engine client is not active[ %d ]",
                      rc ) ;
         rc = _esClt->dropIndex( _indexName.c_str() ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Drop index[ %s ] on search engine failed[ %d ]",
                    _indexName.c_str(), rc ) ;
            goto error ;
         }
         PD_LOG( PDEVENT, "Drop index[ %s ] on search engine successfully",
                 _indexName.c_str() ) ;

         goto done ;
      }
      else if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "Query failed[ %d ]. Current status[ %s ]",
                 rc, _seadptStatus2Desp( _status ) ) ;
         goto error ;
      }
      else
      {
         rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                               &numReturned, docObjs ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to extract query result, rc: %d", rc ) ;
            goto error ;
         }

         if ( SEADPT_SESSION_STAT_QUERY_LAST_LID == _status )
         {
            if ( 0 == docObjs.size() )
            {
               _lastPopLID = -1 ;
            }
            else
            {
               SDB_ASSERT( 1 == docObjs.size(),
                           "Returned object number is wrong" ) ;
               _lastPopLID =
                  docObjs[0].getField(SEADPT_FIELD_NAME_ID).Number() ;
            }
            _switchStatus( SEADPT_SESSION_STAT_QUERY_NORMAL_TBL ) ;
            goto done ;
         }

         // pop is after a query/getmore on a capped collection. A new context
         // will be created for pop operation. We need to fetch the remaining
         // data in the capped collection, so need to store the original context
         // id, and use it for getmore after the pop operation.
         if ( SEADPT_SESSION_STAT_POP_CAP == _status )
         {
            _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
            contextID = _queryCtxID ;
         }
         else
         {
            _queryCtxID = contextID ;
         }

         rc = _sendGetmoreReq( contextID, msg->requestID ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get more records, rc: %d", rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // When some records are fetched from data node, different operation will be
   // done based on the status.
   INT32 _seAdptIndexSession::handleGetMoreRes( NET_HANDLE handle,
                                                MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;

      switch ( _status )
      {
         case SEADPT_SESSION_STAT_QUERY_NORMAL_TBL:
            rc = _processNormalCLRecords( handle, msg ) ;
            PD_RC_CHECK( rc, PDERROR, "Process data of original collection "
                         "failed[ %d ]", rc ) ;
            break ;
         case SEADPT_SESSION_STAT_QUERY_CAP_TBL:
            rc = _processCappedCLRecords( handle, msg ) ;
            PD_RC_CHECK( rc, PDERROR, "Process data of capped collection "
                         "failed[ %d ]", rc ) ;
            break ;
         case SEADPT_SESSION_STAT_CONSULT:
            rc = _getLastIndexedLID( handle, msg ) ;
            PD_RC_CHECK( rc, PDERROR, "Get last indexed record logical id "
                         "failed[ %d ]", rc ) ;
            break ;
         default:
            SDB_ASSERT( FALSE, "Invalid status" ) ;
            break ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_onResMsg( NET_HANDLE handle, MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      // TODO: The version of the collection may have changed. What to do then?
      if ( MSG_BS_GETMORE_RES == msg->opCode )
      {
         rc = handleGetMoreRes( handle, msg ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to handle query respond, rc: %d", rc ) ;
            goto error ;
         }
      }
      else if ( MSG_BS_QUERY_RES == msg->opCode )
      {
         rc = handleQueryRes( handle, msg ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to handle query respond, rc: %d", rc ) ;
            goto error ;
         }
      }
      else
      {
         SDB_ASSERT( FALSE, "Invalid mesage" ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_onCatalogResMsg( NET_HANDLE handle,
                                                MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> docObjs ;
      INT32 version = 0 ;

      SDB_ASSERT( SEADPT_SESSION_STAT_UPDATE_CL_VERSION == _status,
                  "Status is wrong" ) ;

      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, docObjs ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to extract query result, rc: %d", rc ) ;
         goto error ;
      }

      SDB_ASSERT( 1 == docObjs.size(),
                  "Respond size from catalog is wrong" ) ;
      version = docObjs[0].getIntField( FIELD_NAME_VERSION ) ;
      _updateCLVersion( version ) ;
      _switchStatus( SEADPT_SESSION_STAT_CONSULT ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _seAdptIndexSession::onRecieve( const NET_HANDLE netHandle,
                                        MsgHeader *msg )
   {
   }

   BOOLEAN _seAdptIndexSession::timeout( UINT32 interval )
   {
      return _quit ;
   }

   void _seAdptIndexSession::onTimer( UINT64 timerID, UINT32 interval )
   {
      INT32 rc = SDB_OK ;

      if ( _quit )
      {
         goto done ;
      }

      // Consult and query triggers by timer.
      switch ( _status )
      {
         case SEADPT_SESSION_STAT_BEGIN:
            rc = _sendUpdateCLVersionReq() ;
            PD_RC_CHECK( rc, PDERROR, "Update collection version failed" ) ;
            _switchStatus( SEADPT_SESSION_STAT_UPDATE_CL_VERSION ) ;
            break ;
         case SEADPT_SESSION_STAT_CONSULT:
            rc = _consult() ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "Consult failed[ %d ]. Try to start from the "
                       "beginning...", rc ) ;
               _switchStatus( SEADPT_SESSION_STAT_BEGIN ) ;
               goto error ;
            }
            break ;
         case SEADPT_SESSION_STAT_UPDATE_CL_VERSION:
            rc = _sendUpdateCLVersionReq() ;
            PD_RC_CHECK( rc, PDERROR, "Update collection version failed" ) ;
            break ;
         case SEADPT_SESSION_STAT_QUERY_NORMAL_TBL:
            // Only when the query is not in progress that we start a fresh
            // query.
            if ( !_isQueryBusy() )
            {
               rc = _queryOrigCollection() ;
               PD_RC_CHECK( rc, PDERROR,
                            "Query original collection[ %s ] failed[ %d ]",
                            _origCLFullName.c_str(), rc ) ;
               _setQueryBusyFlag( TRUE ) ;
            }
            break ;
         case SEADPT_SESSION_STAT_QUERY_CAP_TBL:
            if ( !_isQueryBusy() )
            {
               rc = _queryCappedCollection() ;
               PD_RC_CHECK( rc, PDERROR,
                            "Query capped collection[ %s ] failed[ %d ]",
                            _cappedCLFullName.c_str(), rc ) ;
               _setQueryBusyFlag( TRUE ) ;
            }
            break ;
         default:
            break ;
      }

   done:
      return ;
   error:
      goto done ;
   }

   void _seAdptIndexSession::_onAttach()
   {
      BSONObjBuilder builder ;
      BSONObjIterator idxItr( _indexDef ) ;

      builder.append( SEADPT_FIELD_NAME_ID, "" ) ;
      while ( idxItr.more() )
      {
         BSONElement ele = idxItr.next() ;
         const CHAR *fieldName = ele.fieldName() ;
         SDB_ASSERT( 0 != ossStrcmp( fieldName, SEADPT_FIELD_NAME_ID ),
                     "Text index should not include _id" ) ;
         builder.append( fieldName, "" ) ;
      }

      _selector = builder.obj() ;

      PD_LOG( PDDEBUG, "Selector for the original collection: %s",
              _selector.toString().c_str() ) ;

      PD_LOG( PDEVENT, "New index task starts for original collection[%s], "
              "index[%s], capped collection[%s], search engine index[%s], "
              "search engine type[%s]", _origCLFullName.c_str(),
              _origIdxName.c_str(), _cappedCLFullName.c_str(),
              _indexName.c_str(), _typeName.c_str() ) ;
   }

   void _seAdptIndexSession::_onDetach()
   {
      PD_LOG( PDDEBUG, "Indexer session detached" ) ;
   }

   void _seAdptIndexSession::_updateCLVersion( INT32 version )
   {
      PD_LOG( PDDEBUG, "Change local version for collection[ %s ] from [ %d ] "
              "to [ %d ] according to catalog", _origCLFullName.c_str(),
              _origCLVersion, version ) ;
      _origCLVersion = version ;
   }

   void _seAdptIndexSession::_switchStatus( SEADPT_SESSION_STATUS newStatus )
   {
      PD_LOG( PDDEBUG, "Switch status from [ %s ] to [ %s ]",
              _seadptStatus2Desp( _status ), _seadptStatus2Desp( newStatus ) ) ;
      _status = newStatus ;
   }

   INT32 _seAdptIndexSession::_sendUpdateCLVersionReq()
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 buffSize = 0 ;
      pmdEDUCB *cb = eduCB() ;
      BSONObj query ;
      BSONObj selector ;
      INT32 flag = FLG_QUERY_WITH_RETURNDATA ;

      query = BSON( FIELD_NAME_NAME << _origCLFullName.c_str() ) ;
      selector = BSON( FIELD_NAME_VERSION << "" ) ;

      rc = msgBuildQueryCatalogReqMsg( (CHAR **)&msg, &buffSize, flag, 0, 0,
                                       -1, cb->getTID(), &query, &selector,
                                       NULL, NULL, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query catalog message failed[ %d ]",
                   rc ) ;
      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToCataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send collection[ %s ] version query to catalog"
                   " failed[ %d ]", _origCLFullName.c_str(), rc ) ;
      PD_LOG( PDDEBUG, "Send collection[ %s ] version query to catalog "
              "succesfully", _origCLFullName.c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_sendGetmoreReq( INT64 contextID,
                                               UINT64 requestID )
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msgBuf = NULL ;
      INT32 bufSize = 0 ;

      rc = msgBuildGetMoreMsg( (CHAR **)&msgBuf, &bufSize, -1,
                               contextID, requestID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to build get more request, rc: %d", rc ) ;
         goto error ;
      }
      msgBuf->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msgBuf ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to send get more request to data node, "
                 "rc: %d", rc ) ;
         goto error ;
      }

      PD_LOG( PDDEBUG, "Send getmore request to data node successfully" ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_queryOrigCollection()
   {
      INT32 rc = SDB_OK ;
      MsgOpQuery *msg = NULL ;
      INT32 bufSize = 0 ;

      rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize,
                              _origCLFullName.c_str(),
                              0, 0, 0, -1, NULL, &_selector ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[ %d ]", rc ) ;
      msg->version = _origCLVersion ;
      msg->header.TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( (MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[ %d ]",
                   rc ) ;

      PD_LOG( PDDEBUG, "Send query on normal collection[ %s ] to data node "
              "successfully", _origCLFullName.c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_queryLastCappedRecLID()
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;
      BSONObj selector ;
      BSONObj orderBy ;

      selector = BSON( "_id" << "" ) ;
      orderBy = BSON( "_id" << -1 ) ;

      rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize, _cappedCLFullName.c_str(),
                             FLG_QUERY_WITH_RETURNDATA, 0, 0, 1, NULL,
                             &selector, &orderBy ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[ %d ]", rc ) ;
      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[ %d ]",
                   rc ) ;
      PD_LOG( PDDEBUG, "Send query on capped collection[ %s ] to data node "
              "successfully", _cappedCLFullName.c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_queryCappedCollection()
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;
      BSONObj condition ;

      if ( _lastPopLID >= 0 )
      {
         condition = BSON( "_id" << BSON( "$gt" << _lastPopLID ) ) ;
         PD_LOG( PDDEBUG, "Query condition: %s",
                 condition.toString().c_str() ) ;
      }

      rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize,
                             _cappedCLFullName.c_str(),
                             0, 0, 0, -1,
                             ( condition.isEmpty() ) ? NULL : &condition,
                             NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[ %d ]", rc ) ;
      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[ %d ]",
                   rc ) ;
      PD_LOG( PDDEBUG, "Send query on capped collection[ %s ] to data node "
              "successfully", _cappedCLFullName.c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // Check on ES, to get the next expected record LID.
   INT32 _seAdptIndexSession::_getExpectRLID( INT64 &expectRLID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN normalCLDone = FALSE ;

      // First, check if the normal collection end mark record is there.
      // If yes, continue with capped collection.
      // Otherwise, restart from the beginning.
      rc = _chkDoneMark( normalCLDone ) ;
      PD_RC_CHECK( rc, PDERROR, "Check normal collection done mark "
                   "failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_cleanData( INT64 recLID )
   {
      // Build a pop command to pop the data from capped collection.
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 msgLen = 0 ;
      BSONObj query ;
      BSONObjBuilder builder ;

      builder.append( FIELD_NAME_COLLECTION, _cappedCLFullName.c_str() ) ;
      builder.appendIntOrLL( FIELD_NAME_LOGICAL_ID, recLID ) ;
      builder.appendIntOrLL( FIELD_NAME_DIRECTION, 1 ) ;

      query = builder.done() ;

      rc = msgBuildQueryMsg( (CHAR **)&msg, &msgLen,
                             CMD_ADMIN_PREFIX CMD_NAME_POP,
                             0, 0, -1, -1, &query, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Build pop message failed[ %d ], logical "
                   "id: %lld", rc, recLID ) ;

      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( (MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send pop command to data node failed[ %d ]",
                   rc ) ;
      PD_LOG( PDDEBUG, "Send pop command to data node: %s",
              query.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // Parse the original object and get the items we want.
   INT32 _seAdptIndexSession::_parseSrcData( const BSONObj &origObj,
                                             _dmsExtOprType &oprType,
                                             const CHAR **origOID,
                                             BSONObj &sourceObj )
   {
      INT32 rc = SDB_OK ;
      INT32 type = 0 ;

      SDB_ASSERT( origOID, "OID pointer should not be NULL" ) ;

      try
      {
         type = origObj.getIntField( FIELD_NAME_TYPE ) ;

         if ( type < DMS_EXT_INSERT || type > DMS_EXT_TRUNCATE )
         {
            PD_LOG( PDERROR, "Operation type[ %d ] is invalid in source object",
                    type ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         oprType = ( _dmsExtOprType )type ;
         *origOID = origObj.getStringField( SEADPT_FIELD_NAME_RID ) ;
         if ( 0 == ossStrcmp( *origOID, "" ) )
         {
            PD_LOG( PDERROR, "_rid for the original record is invalid. "
                    "Object: %s", origObj.toString().c_str() ) ;
            rc = SDB_SYS ;
            goto done ;
         }

         sourceObj = origObj.getObjectField( "_source" ) ;
         if ( sourceObj.isEmpty() || !sourceObj.isValid() )
         {
            PD_LOG( PDERROR, "_source field is invalid. Object: %s",
                    origObj.toString().c_str() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         PD_LOG( PDDEBUG, "Operation type: %d, _id: %s, _source: %s", oprType,
                 *origOID, sourceObj.toString().c_str() ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_processNormalCLRecords( NET_HANDLE handle,
                                                       MsgHeader *msg )
   {
      // We fetched the _id in the record, but should remove it when indexing
      // on elasticsearch, for we use the value of _id as the _id in ES.
      INT32 rc = SDB_OK ;
      MsgOpReply *reply = (MsgOpReply *)msg ;
      INT32 flag = 0 ;
      INT64 contextID = 0 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> docObjs ;
      BOOLEAN idExist = TRUE ;

      if ( SDB_DMS_EOC == reply->flags )
      {
         // When reach to the end of the original collection, switch to capped
         // collection.
         rc = _markNormalCLDone() ;
         PD_RC_CHECK( rc, PDERROR, "Write end mark for normal collection[ %s ] "
                      "on search engine failed[ %d ]",
                      _origCLFullName.c_str(), rc ) ;
         PD_LOG( PDDEBUG, "Write end mark for normal collection[ %s ] on "
                 "search engine successfully", _origCLFullName.c_str() ) ;
         _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
         _setQueryBusyFlag( FALSE ) ;
         goto done ;
      }
      else if ( SDB_DMS_CS_NOTEXIST == reply->flags ||
                SDB_DMS_NOTEXIST == reply->flags )
      {
         _quit = TRUE ;
         goto done ;
      }
      else if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "Get more failed[ %d ]", rc ) ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, docObjs ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to extract query result, rc: %d", rc ) ;
         goto error ;
      }

      rc = _ensureESClt() ;
      PD_RC_CHECK( rc, PDERROR, "The search engine client is not active[ %d ]",
                   rc ) ;

      for ( vector<BSONObj>::const_iterator itr = docObjs.begin();
            itr != docObjs.end(); ++itr )
      {
         BSONObj newNameObj ;
         string idStr ;
         BSONObjBuilder builder ;
         BSONObj sourceObj ;
         BSONElement idEle ;
         OID id ;
         BSONElement sourceEle ;

         idExist = itr->getObjectID( idEle ) ;
         if ( !idExist )
         {
            PD_LOG( PDERROR, "_id dose not exist in source object" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         for ( BSONObj::iterator eleItr = itr->begin(); eleItr.more(); )
         {
            BSONElement ele = eleItr.next() ;
            if ( jstOID != ele.type() )
            {
               builder.append( ele ) ;
            }
         }

         sourceObj = builder.done() ;

         // For data in the original collection, the only operation is insertion.
         rc = _esClt->indexDocument( _indexName.c_str(), _typeName.c_str(),
                                     idEle.OID().toString().c_str(),
                                     sourceObj.toString().c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Index record on search engine "
                      "failed[ %d ], id: %s", rc,
                      idEle.toString().c_str() ) ;
         // TODO: what to do if error happened? Restart from the beginning?
      }

      rc = _sendGetmoreReq( contextID, msg->requestID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get more request, rc: %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      _switchStatus( SEADPT_SESSION_STAT_BEGIN ) ;
      _setQueryBusyFlag( FALSE ) ;
      PD_LOG( PDEVENT, "Error happened when processiong data of collection"
              "[ %s ]. Ready to restart the task from beginning",
              _origCLFullName.c_str() ) ;
      goto done ;
   }

   INT32 _seAdptIndexSession::_processCappedCLRecords( NET_HANDLE handle,
                                                       MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      MsgOpReply *reply = (MsgOpReply *)msg ;
      INT32 flag = 0 ;
      INT64 contextID = 0 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      INT64 nextLastLID = -1 ;
      vector<BSONObj> docObjs ;

      // If reach end of the capped collection, start another query on it. It
      // will be started in onTimer.
      if ( SDB_DMS_EOC == reply->flags )
      {
         _setQueryBusyFlag( FALSE ) ;
         PD_LOG( PDDEBUG, "All records in capped collection[ %s ] have been "
                 "processed. Ready to start a new query on it",
                 _cappedCLFullName.c_str() ) ;
         goto done ;
      }
      else if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "Get more failed[ %d ]", rc ) ;
         // Start another query.
         _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
         _setQueryBusyFlag( FALSE ) ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, docObjs ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to extract query result, rc: %d", rc ) ;
         goto error ;
      }

      rc = _ensureESClt() ;
      PD_RC_CHECK( rc, PDERROR, "The search engine client is not active[ %d ]",
                   rc ) ;

      if ( docObjs.size() > 0 )
      {
         try
         {
            BSONObj lastObj = docObjs.back() ;
            BSONElement lidEle = lastObj.getField( SEADPT_FIELD_NAME_ID ) ;
            nextLastLID = lidEle.Long() ;
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }

      for ( vector<BSONObj>::const_iterator itr = docObjs.begin();
            itr != docObjs.end(); ++itr )
      {
         BSONObj newNameObj ;
         string idStr ;
         BSONObj sourceObj ;
         _dmsExtOprType oprType = DMS_EXT_INVALID ;
         const CHAR *origOID = NULL ;
         BSONElement sourceEle ;

         // Parse and the original object into 4 items: operation type, id,
         // source data( the real data to be indexed on Elasticsearch ).
         rc = _parseSrcData( *itr, oprType, &origOID, sourceObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Get id string and source object "
                      "failed[ %d ]", rc ) ;
         switch ( oprType )
         {
            case DMS_EXT_INSERT:
               rc = _esClt->indexDocument( _indexName.c_str(),
                                           _typeName.c_str(),
                                           origOID,
                                           sourceObj.toString().c_str() ) ;
               PD_RC_CHECK( rc, PDERROR, "Index record on search engine "
                            "failed[ %d ], id: %s", rc,
                            origOID ) ;
               break ;
            case DMS_EXT_DELETE:
               rc = _esClt->deleteDocument( _indexName.c_str(),
                                            _typeName.c_str(),
                                            origOID ) ;
               PD_RC_CHECK( rc, PDERROR, "Delete document on search engine "
                            "failed[ %d ], _id: %s", rc,
                            origOID ) ;
               break ;
            case DMS_EXT_UPDATE:
               rc = _esClt->updateDocument( _indexName.c_str(),
                                            _typeName.c_str(),
                                            origOID,
                                            sourceObj.toString().c_str()  ) ;
               PD_RC_CHECK( rc, PDERROR, "Update document on search engine "
                            "failed[ %d ], id: %s", rc,
                            origOID ) ;
               break ;
            case DMS_EXT_TRUNCATE:
               // TODO:
               break ;
            default:
               PD_LOG( PDERROR, "Invalid operation type[ %d ] in source data",
                       oprType ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
         }
      }

      // After each batch of data processed successfully, pop the last batch of
      // data out. If -1 == _lastPopLID, it's the first time. We do not pop in
      // this round, just to fetch the next batch of results.
      if ( _lastPopLID >= 0 )
      {
         rc = _cleanData( _lastPopLID ) ;
         PD_RC_CHECK( rc, PDERROR, "Clean data failed[ %d ]", rc ) ;
         _switchStatus( SEADPT_SESSION_STAT_POP_CAP ) ;
      }
      else
      {
         rc = _sendGetmoreReq( contextID, msg->requestID ) ;
         PD_RC_CHECK( rc, PDERROR, "Send get more request failed[ %d ]", rc ) ;
      }

      PD_LOG( PDDEBUG, "Change last pop LogicalID from [ %lld ] to [ %lld ]",
              _lastPopLID, nextLastLID ) ;
      _lastPopLID = nextLastLID ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // Compare records from capped collection, and compare with data on ES, to
   // find the last one which has been index on ES successfully.
   INT32 _seAdptIndexSession::_getLastIndexedLID( NET_HANDLE handle,
                                                  MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      INT64 contextID = 0 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> docObjs ;
      string queryStr ;

      MsgOpReply *reply = (MsgOpReply *)msg ;

      if ( SDB_DMS_EOC == reply->flags )
      {
         // If reach end of the capped collection, and find out all the records
         // have been indexed on ES, then there is no data loss on ES. Let's
         // just go on from the current position.
         // We may process the last record once again. but that's OK.
         PD_LOG( PDDEBUG, "All data has been indexed on search engine "
                 "succesfully. Start normal sync now." ) ;
         _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
         goto done ;
      }
      else if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "Get more data from capped collection[ %s ] failed"
                 "[ %d ]. Try to start from the beinning...",
                 _cappedCLFullName.c_str(), rc ) ;
         // Change the status back to begin to start over.
         _switchStatus( SEADPT_SESSION_STAT_BEGIN ) ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, docObjs ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract reply failed[ %d ]", rc ) ;

      for ( vector<BSONObj>::iterator itr = docObjs.end() - 1;
            itr != docObjs.begin() - 1; --itr )
      {
         BOOLEAN found = FALSE ;
         BSONElement ele = itr->getField( SEADPT_FIELD_NAME_ID ) ;
         SDB_ASSERT( NumberLong == ele.type(), "Element type is wrong" ) ;
         rc = _findRecWithLID( ele.Long(), found ) ;
         PD_RC_CHECK( rc, PDERROR, "Find record with _id[ %lld ] on search "
                      "engine failed[ %d ]", ele.Long(), rc ) ;
         if ( !found )
         {
            // If not found, the targe document dose not exist on search engine.
            // And that's the record where should we start.
            PD_LOG( PDDEBUG, "Sucessfully found the logical id to start "
                    "from[ %lld ]", ele.Long() ) ;
            _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
            break ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // Write a mark record on elasticsearch to specify all the data in the normal
   // collection has been indexed.
   INT32 _seAdptIndexSession::_markNormalCLDone()
   {
      INT32 rc = SDB_OK ;

      if ( !_esClt )
      {
         rc = sdbGetSeAdapterCB()->getSeCltMgr()->getSeClt( &_esClt ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get search engine client, rc: %d",
                    rc ) ;
            goto error ;
         }
      }

      rc = _esClt->indexDocument( _indexName.c_str(), _typeName.c_str(),
                                  SEADPT_NORMAL_CL_DONE_ID,
                                  SEADPT_NORMAL_CL_DONE_BODY ) ;
      PD_RC_CHECK( rc, PDERROR, "Index document failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_chkDoneMark( BOOLEAN &found )
   {
      INT32 rc = SDB_OK ;

      if ( !_esClt )
      {
         rc = sdbGetSeAdapterCB()->getSeCltMgr()->getSeClt( &_esClt ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get search engine client, rc: %d",
                    rc ) ;
            goto error ;
         }
      }

      rc = _esClt->documentExist( _indexName.c_str(),
                                  _typeName.c_str(),
                                  SEADPT_NORMAL_CL_DONE_ID,
                                  found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check document existense failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_consult()
   {
      // First, check if the index exist on ES. If not, then need to do full
      // indexing.
      // If index exists, check the original collection end mark. If not found,
      // drop the index and do full indexing.
      // If index exists and the end mark is found, then go on to find the
      // logical id of the last indexed document.

      INT32 rc = SDB_OK ;
      BOOLEAN found = FALSE ;

      rc = _ensureESClt() ;
      PD_RC_CHECK( rc, PDERROR, "The search engine client is not active[ %d ]",
                   rc ) ;

      // 1. Check if the index exists on ES.
      // 2. Compare the index logical id. If not the same, drop it and do full
      //    index.
      // 3. Get normal collection end mark. If it's not there, drop the index on
      //    search engine, and do full index.
      // 4. If the right index is there, then try to check the last successfully
      //    indexed document. We want its logical ID. Send a query to data node,
      //    fetch some data, and compare.
      rc = _esClt->indexExist( _indexName.c_str(), found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check index[ %s ] existence on search engine "
                   "failed[ %d ]", _indexName.c_str(), rc ) ;
      if ( !found )
      {
         PD_LOG( PDEVENT, "Target index[ %s ] dose not exist on search engine. "
                 "Start to query the original collection[ %s ] and the index "
                 "will be created", _indexName.c_str(), _origIdxName.c_str() ) ;
         _switchStatus( SEADPT_SESSION_STAT_QUERY_LAST_LID ) ;
         rc = _queryLastCappedRecLID() ;
         PD_RC_CHECK( rc, PDERROR, "Query last logical id failed[ %d ]", rc ) ;
         goto done ;
      }

      rc = _esClt->documentExist( _indexName.c_str(),
                                  _typeName.c_str(),
                                  SEADPT_NORMAL_CL_DONE_ID,
                                  found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check document existense failed[ %d ]", rc ) ;
      if ( !found )
      {
         PD_LOG( PDEVENT, "End mark of original collection for index[ %s ] "
                 "dose not exist. Index will be dropped and recreated",
                 _indexName.c_str() ) ;
         rc = _esClt->dropIndex( _indexName.c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Drop index[ %s ] on search engine "
                      "failed[ %d ]", _indexName.c_str(), rc ) ;
         PD_LOG( PDEVENT, "Index[ %s ] dropped successfully",
                 _indexName.c_str() ) ;
         _switchStatus( SEADPT_SESSION_STAT_QUERY_LAST_LID ) ;
         rc = _queryLastCappedRecLID() ;
         PD_RC_CHECK( rc, PDERROR, "Query last logical id failed[ %d ]", rc ) ;
         goto done ;
      }

      PD_LOG( PDEVENT, "Begin to query the original collection[ %s ]...",
              _origCLFullName.c_str() ) ;
      rc = _queryCappedCollection() ;
      PD_RC_CHECK( rc, PDERROR, "Send query of capped collection failed[ %d ]",
                   rc ) ;

   done:
      return rc ;
   error:
      // In case of error, switch back to begin status, and wait for next try
      // of consult.
      _switchStatus( SEADPT_SESSION_STAT_BEGIN ) ;
      goto done ;
   }

   INT32 _seAdptIndexSession::_ensureESClt()
   {
      INT32 rc = SDB_OK ;
      if ( !_esClt )
      {
         rc = sdbGetSeAdapterCB()->getSeCltMgr()->getSeClt( &_esClt ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get search engine client, rc: %d",
                    rc ) ;
            goto error ;
         }
      }
      else
      {
         if ( !_esClt->isActive() )
         {
            rc = _esClt->active() ;
            PD_RC_CHECK( rc, PDERROR, "Activate search engine client "
                         "failed[ %d ]", rc ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _seAdptIndexSession::_onSDBEOC()
   {
      if ( SEADPT_SESSION_STAT_CONSULT == _status ||
           SEADPT_SESSION_STAT_QUERY_NORMAL_TBL == _status )
      {
         _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
         _setQueryBusyFlag( FALSE ) ;
      }
      else if ( SEADPT_SESSION_STAT_UPDATE_CL_VERSION == _status )
      {
         // If the collection can not be found on catalog, it has been dropped.
         // So let's quit.
         PD_LOG( PDEVENT, "Collection[ %s ] can not be found on catalog. It "
                 "may have been dropped. Task ready to exit.",
                 _origCLFullName.c_str() ) ;
         _quit = TRUE ;
      }
   }
}

