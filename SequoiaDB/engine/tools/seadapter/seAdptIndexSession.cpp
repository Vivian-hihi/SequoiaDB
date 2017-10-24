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
#define SEADPT_COMMIT_ID             "SDBCOMMIT"
#define SEADPT_FIELD_NAME_LID        "_lid"
#define SEADPT_NORMAL_CL_DONE_BODY   "{}"
#define SEADPT_TID(sessionID)          ((UINT32)(sessionID & 0xFFFFFFFF))

namespace engine
{
   #define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
   const CHAR *_statDescription[] =
   {
      "Begin",
      "Update collection version",
      "Consult",
      "Query last logical id in capped collection",
      "Compare last logical id",
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
      ON_MSG( MSG_BS_QUERY_RES, handleQueryRes )
      ON_MSG( MSG_BS_GETMORE_RES, handleGetMoreRes )
   END_OBJ_MSG_MAP()

   _seAdptIndexSession::_seAdptIndexSession( UINT64 sessionID,
                                             const seIndexMeta *idxMeta )
   : _pmdAsyncSession( sessionID )
   {
      SDB_ASSERT( idxMeta && idxMeta->getIdxDef().valid()
                  && !idxMeta->getIdxDef().isEmpty(),
                  "Index definition is invalid" ) ;

      _origCLFullName = idxMeta->getOrigCLName() ;
      _cappedCLFullName = idxMeta->getCappedCLName() ;
      _origCLVersion = -1 ;
      _origIdxName = idxMeta->getOrigIdxName();
      _indexName = idxMeta->getEsIdxName() ;
      _typeName = idxMeta->getEsTypeName() ;
      _indexDef = idxMeta->getIdxDef().copy() ;
      _esClt = NULL ;
      _status = SEADPT_SESSION_STAT_CONSULT ;
      _lastPopLID = -1 ;
      _quit = FALSE ;
      _queryCtxID = -1 ;
      _queryBusy = FALSE ;
      _expectLID = -1 ;
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
                 "more. Task ready to exit. Index on search engine will be "
                 "dropped.",
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
         // If we find the node is not primary when pop, switch may have
         // happened. So quit the current indexing work.
         if ( SEADPT_SESSION_STAT_POP_CAP == _status )
         {
            PD_LOG( PDEVENT, "Node is not primary when pop. Switch of primary "
                    "may have happened. Exiting current indexing work..." ) ;
            _quit = TRUE ;
            goto done ;
         }

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
               _expectLID = 0 ;
            }
            else
            {
               SDB_ASSERT( 1 == docObjs.size(),
                           "Returned object number is wrong" ) ;
               _lastPopLID =
                  docObjs[0].getField(SEADPT_FIELD_NAME_ID).Number() ;
               _expectLID = _lastPopLID ;
               PD_LOG( PDDEBUG, "Last logical id in capped collection[ %s ] is "
                       "[ %lld ]", _cappedCLFullName.c_str(), _lastPopLID ) ;
            }
            _switchStatus( SEADPT_SESSION_STAT_QUERY_NORMAL_TBL ) ;
            goto done ;
         }
         else if ( SEADPT_SESSION_STAT_COMP_LAST_LID == _status )
         {
            if ( 0 == docObjs.size() )
            {
               if ( 0 == _expectLID )
               {
                  _lastPopLID = -1 ;
                  _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
                  _setQueryBusyFlag( FALSE ) ;
                  goto done ;
               }
               else
               {
                  // ERROR, need to start from beginning
                  PD_LOG( PDERROR, "The expected logical id is [ %lld ], but "
                          "the capped collection [ %s ] is empty. Begin to "
                          "start all over again", _expectLID,
                          _cappedCLFullName.c_str() ) ;
                  rc = _startOver() ;
                  PD_RC_CHECK( rc, PDERROR, "Restart the index work "
                               "failed[ %d ]", rc ) ;
                  goto done ;
               }
            }
            else
            {
               SDB_ASSERT( 1 == docObjs.size(),
                           "Returned object number is wrong" ) ;
               INT64 lastLID =
                  docObjs[0].getField(SEADPT_FIELD_NAME_ID).Number() ;
               if ( _expectLID >= lastLID )
               {
                  _lastPopLID = _expectLID ;
                  _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
                  _setQueryBusyFlag( FALSE ) ;
                  goto done ;
               }
               else
               {
                  // ERROR
                  // need to clean the index and start over
                  PD_LOG( PDERROR, "The expected logical id is [ %lld ], but "
                          "the actual last logical id in capped collection "
                          "[ %s ] is [ %lld ]. Begin to start all over again",
                          _expectLID, _cappedCLFullName.c_str(), lastLID ) ;
                  rc = _startOver() ;
                  PD_RC_CHECK( rc, PDERROR, "Restart the index work "
                               "failed[ %d ]", rc ) ;
                  goto done ;
               }
            }
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
         PD_RC_CHECK( rc, PDERROR, "Send get more request failed[ %d ]", rc ) ;
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

   void _seAdptIndexSession::onRecieve( const NET_HANDLE netHandle,
                                        MsgHeader *msg )
   {
   }

   BOOLEAN _seAdptIndexSession::timeout( UINT32 interval )
   {
      return _quit ;
   }

   // Called by pmdAsyncSessionAgentEntryPoint. It will be invoked every one
   // second.
   void _seAdptIndexSession::onTimer( UINT64 timerID, UINT32 interval )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN inUpdate = FALSE ;

      if ( _quit )
      {
         goto done ;
      }

      // Consult and query triggers by timer.
      switch ( _status )
      {
         case SEADPT_SESSION_STAT_BEGIN:
            if ( !inUpdate )
            {
               INT32 clVersion = -1 ;
               inUpdate = TRUE ;
               rc = sdbGetSeAdapterCB()->syncUpdateCLVersion( _origCLFullName.c_str(),
                                                              OSS_ONE_SEC, eduCB(),
                                                              clVersion ) ;
               PD_RC_CHECK( rc, PDERROR, "Update collection version failed" ) ;
               PD_LOG( PDDEBUG, "Change cl[ %s ] version from [ %d ] to [ %d ]"
                       "accorrding to catalog", _origCLFullName.c_str(),
                       _origCLVersion, clVersion ) ;
               _origCLVersion = clVersion ;
               _switchStatus( SEADPT_SESSION_STAT_QUERY_NORMAL_TBL ) ;
            }
            break ;
         case SEADPT_SESSION_STAT_CONSULT:
            rc = _consult() ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "Consult failed[ %d ]. Try to start from the "
                       "beginning...", rc ) ;
               // _switchStatus( SEADPT_SESSION_STAT_BEGIN ) ;
               _startOver() ;
               goto error ;
            }
            break ;
         case SEADPT_SESSION_STAT_QUERY_NORMAL_TBL:
            // Only when the query is not in progress that we start a fresh
            // query.
            if ( !_isQueryBusy() )
            {
               INT32 clVersion = -1 ;
               rc = sdbGetSeAdapterCB()->syncUpdateCLVersion( _origCLFullName.c_str(),
                                                              OSS_ONE_SEC, eduCB(),
                                                              clVersion ) ;
               PD_RC_CHECK( rc, PDERROR, "Update collection version "
                            "failed[ %d ]", rc ) ;
               PD_LOG( PDDEBUG, "Change cl[ %s ] version from [ %d ] to [ %d ]"
                       "accorrding to catalog", _origCLFullName.c_str(),
                       _origCLVersion, clVersion ) ;
               _origCLVersion = clVersion ;

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
               BSONObj condition ;
               if ( _lastPopLID >= 0 )
               {
                  condition = BSON( "_id" << BSON( "$gt" << _lastPopLID ) ) ;
               }
               rc = _queryCappedCollection( condition ) ;
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
      try
      {
         // Build the selector for querying the original collection.
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

         PD_LOG( PDEVENT, "New index task starts: original collection[ %s ], "
                 "index[ %s ], capped collection[ %s ], search engine "
                 "index[ %s ], search engine type[ %s ]",
                 _origCLFullName.c_str(), _origIdxName.c_str(),
                 _cappedCLFullName.c_str(), _indexName.c_str(),
                 _typeName.c_str() ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         // In case of exception, quit.
         _quit = TRUE ;
      }
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

      // In the capped colleciton, the field name of logical id is '_id', and
      // the original '_id' is named '_rid'.
      selector = BSON( SEADPT_FIELD_NAME_ID << "" ) ;
      orderBy = BSON( SEADPT_FIELD_NAME_ID << -1 ) ;

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

   INT32 _seAdptIndexSession::_queryCappedCollection( BSONObj &condition )
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;

      PD_LOG( PDDEBUG, "Query condition: %s", condition.toString().c_str() ) ;

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
                                             INT64 &logicalID,
                                             BSONObj &sourceObj )
   {
      INT32 rc = SDB_OK ;
      INT32 type = 0 ;

      SDB_ASSERT( origOID, "OID pointer should not be NULL" ) ;

      try
      {
         BSONElement lidField = origObj.getField( SEADPT_FIELD_NAME_ID ) ;
         type = origObj.getIntField( FIELD_NAME_TYPE ) ;

         if ( type < DMS_EXT_INSERT || type > DMS_EXT_TRUNCATE )
         {
            PD_LOG( PDERROR, "Operation type[ %d ] is invalid in source object",
                    type ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         oprType = ( _dmsExtOprType )type ;
         logicalID = lidField.Long() ;
         *origOID = origObj.getStringField( SEADPT_FIELD_NAME_RID ) ;
         if ( 0 == ossStrcmp( *origOID, "" ) )
         {
            PD_LOG( PDERROR, "_rid for the original record is invalid. "
                    "Object: %s", origObj.toString().c_str() ) ;
            rc = SDB_SYS ;
            goto done ;
         }

         sourceObj = origObj.getObjectField( "_source" ) ;
         if ( !sourceObj.isEmpty() && !sourceObj.isValid() )
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
         // collection. Write an end mark of _lid : -1.
         BSONObj emptyObj = BSON( SEADPT_FIELD_NAME_LID << _expectLID ) ;
         rc = _markProgress( emptyObj ) ;
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
      }

      rc = _sendGetmoreReq( contextID, msg->requestID ) ;
      PD_RC_CHECK( rc, PDERROR, "Send get more request failed[ %d ]", rc ) ;

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
      INT64 logicalID = -1 ;

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
         rc = _parseSrcData( *itr, oprType, &origOID, logicalID, sourceObj ) ;
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
               rc = _esClt->deleteAllByType( _indexName.c_str(),
                                             _typeName.c_str() ) ;
               PD_RC_CHECK( rc, PDERROR, "Delete all documents for index[ %s ] "
                            "and type[ %s ] failed[ %d ]", _indexName.c_str(),
                            _typeName.c_str(), rc ) ;
               break ;
            default:
               PD_LOG( PDERROR, "Invalid operation type[ %d ] in source data",
                       oprType ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
         }
      }

      if ( -1 != logicalID )
      {
         rc = _updateProgress( logicalID ) ;
         PD_RC_CHECK( rc, PDERROR, "Update progress failed[ %d ]", rc ) ;
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

   // Compare records from capped collection and data on ES, to
   // find the last one which has been index on ES successfully.
   INT32 _seAdptIndexSession::_getLastIndexedLID( NET_HANDLE handle,
                                                  MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      vector<BSONObj> docObjs ;
      string queryStr ;

      MsgOpReply *reply = (MsgOpReply *)msg ;

      if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "Get more data from capped collection[ %s ] failed"
                 "[ %d ]. Try to start from the beinning...",
                 _cappedCLFullName.c_str(), rc ) ;
         // Change the status back to begin to start over.
         _switchStatus( SEADPT_SESSION_STAT_BEGIN ) ;
         goto error ;
      }

      _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // Write a mark record on elasticsearch to specify all the data in the normal
   // collection has been indexed.
   INT32 _seAdptIndexSession::_markProgress( BSONObj &infoObj )
   {
      INT32 rc = SDB_OK ;

      rc = _ensureESClt() ;
      PD_RC_CHECK( rc, PDERROR, "The search engine client is not active[ %d ]",
                   rc ) ;
      rc = _esClt->indexDocument( _indexName.c_str(), _typeName.c_str(),
                                  SEADPT_COMMIT_ID,
                                  infoObj.toString().c_str() ) ;
      PD_RC_CHECK( rc, PDERROR, "Index document failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_updateProgress( INT64 logicalID )
   {
      BSONObj lidObj = BSON( SEADPT_FIELD_NAME_LID << logicalID ) ;
      return _markProgress( lidObj ) ;
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
                                  SEADPT_COMMIT_ID,
                                  found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check document existense failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // The consult step and strategy is as follows:
   // First, check if the index exist on ES. If not, then need to do full
   // indexing.
   // If index exists, check the commit mark. If not found, it means the
   // last indexing process is interrupted when processiong data of the
   // original collection. In this case, we do not know where to start. So
   // we start all around.
   // If index exists and the commit mark is found, get the _lid in it.
   // Suggest it's called m. Get the last logical id in the capped
   // collection. Suggest it's n.
   // (1) If n does not exist( capped collection is empty), and m is not -1,
   //     start all around.
   // (2) If m is -1 and n is greate than 0, start all around.
   // (3) If m is greater than n, start all around.
   // (4) If m is less than or equal to n, continue to fetch data from capped
   //     collection with logical id greater than m.
   INT32 _seAdptIndexSession::_consult()
   {
      INT32 rc = SDB_OK ;
      BOOLEAN found = FALSE ;
      BSONObj resultObj ;
      BSONElement lidEle ;
      BSONObj condition ;

      rc = _ensureESClt() ;
      PD_RC_CHECK( rc, PDERROR, "The search engine client is not active[ %d ]",
                   rc ) ;

      // 1. Check index existence.
      rc = _esClt->indexExist( _indexName.c_str(), found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check index[ %s ] existence on search engine "
                   "failed[ %d ]", _indexName.c_str(), rc ) ;
      if ( !found )
      {
         PD_LOG( PDEVENT, "Target index[ %s ] dose not exist on search engine. "
                 "Start all over again and the index will be re-created",
                 _indexName.c_str() ) ;

         rc = _queryLastCappedRecLID() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Query last logical id failed[ %d ]", rc ) ;
            goto error ;
         }
         _switchStatus( SEADPT_SESSION_STAT_QUERY_LAST_LID ) ;
         goto done ;
      }

      try
      {
         rc = _esClt->getDocument( _indexName.c_str(), _typeName.c_str(),
                                   SEADPT_COMMIT_ID, resultObj, FALSE ) ;
         if ( SDB_INVALIDARG == rc )
         {
            PD_LOG( PDEVENT, "Commit mark for index[ %s ] "
                    "dose not exist. Index will be dropped and recreated",
                    _indexName.c_str() ) ;
            rc = _esClt->dropIndex( _indexName.c_str() ) ;
            PD_RC_CHECK( rc, PDERROR, "Drop index[ %s ] on search engine "
                         "failed[ %d ]", _indexName.c_str(), rc ) ;
            PD_LOG( PDEVENT, "Index[ %s ] dropped successfully",
                    _indexName.c_str() ) ;
            rc = _queryLastCappedRecLID() ;
            PD_RC_CHECK( rc, PDERROR, "Query last logical id failed[ %d ]", rc ) ;
            _switchStatus( SEADPT_SESSION_STAT_QUERY_LAST_LID ) ;
            goto done ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Check commit mark on search engine failed[ %d ]",
                    rc ) ;
            goto error ;
         }

         // Get the commit logical id as expect logical id.
         lidEle = resultObj.getField( SEADPT_FIELD_NAME_LID ) ;
         PD_LOG( PDDEBUG, "Commit object: %s", resultObj.toString().c_str() ) ;
         _expectLID = lidEle.numberLong() ;

         rc = _queryLastCappedRecLID() ;
         PD_RC_CHECK( rc, PDERROR, "Query last logical id failed[ %d ]", rc ) ;
         _switchStatus( SEADPT_SESSION_STAT_COMP_LAST_LID ) ;
         goto done ;
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

   // Start all over again. If the index exists, it will be dropped and
   // re-created later.
   INT32 _seAdptIndexSession::_startOver()
   {
      INT32 rc = SDB_OK ;
      BOOLEAN found = FALSE ;

      rc = _ensureESClt() ;
      PD_RC_CHECK( rc, PDERROR, "The search engine client is not active[ %d ]",
                   rc ) ;

      rc = _esClt->indexExist( _indexName.c_str(), found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check index[ %s ] existence on search engine "
                   "failed[ %d ]", _indexName.c_str(), rc ) ;

      if ( found )
      {
         rc = _esClt->dropIndex( _indexName.c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Drop index[ %s ] on search engine "
                         "failed[ %d ]", _indexName.c_str(), rc ) ;
         PD_LOG( PDEVENT, "Index[ %s ] dropped successfully",
                 _indexName.c_str() ) ;
      }

      // _switchStatus( SEADPT_SESSION_STAT_BEGIN ) ;
      _switchStatus( SEADPT_SESSION_STAT_CONSULT ) ;
      _setQueryBusyFlag( FALSE ) ;

   done:
      return rc ;
   error: goto done ;
   }
}

