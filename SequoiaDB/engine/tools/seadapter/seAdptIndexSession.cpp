/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
#include "utilESUtil.hpp"

#define SEADPT_FIELD_NAME_LID        "_lid"
#define SEADPT_FIELD_NAME_CLUID      "_cluid"
#define SEADPT_FIELD_NAME_CLLID      "_cllid"
#define SEADPT_FIELD_NAME_IDXLID     "_idxlid"
#define SEADPT_OPERATOR_STR_OR       "$or"
#define SEADPT_OPERATOR_STR_EXIST    "$exists"
#define SEADPT_OPERATOR_STR_INCLUDE  "$include"
#define SEADPT_TID(sessionID)        ((UINT32)(sessionID & 0xFFFFFFFF))
#define SEADPT_INVALID_LID           -1
#define SEADPT_ID_MAX_SZ             512

namespace seadapter
{
   #define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
   const CHAR *_statDescription[] =
   {
      "Consult",
      "Begin",
      "Update collection version",
      "Clean capped collection",
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
      if ( status >= SEADPT_SESSION_STAT_MAX )
      {
         return NULL ;
      }

      return _statDescription[ status - 1 ] ;
   }

   BEGIN_OBJ_MSG_MAP( _seAdptIndexSession, _pmdAsyncSession)
      ON_MSG( MSG_BS_QUERY_RES, handleQueryRes )
      ON_MSG( MSG_BS_GETMORE_RES, handleGetMoreRes )
      ON_MSG( MSG_BS_KILL_CONTEXT_RES, handleKillCtxRes )
   END_OBJ_MSG_MAP()

   _seAdptIndexSession::_seAdptIndexSession( UINT64 sessionID,
                                             const seIndexMeta *idxMeta )
   : _pmdAsyncSession( sessionID )
   {
      SDB_ASSERT( idxMeta && idxMeta->getIdxDef().valid()
                  && !idxMeta->getIdxDef().isEmpty(),
                  "Index definition is invalid" ) ;

      _origCLVersion = -1 ;
      _meta = *idxMeta ;
      _esClt = NULL ;
      _status = SEADPT_SESSION_STAT_CONSULT ;
      _lastPopLID = -1 ;
      _quit = FALSE ;
      _queryCtxID = -1 ;
      _queryBusy = FALSE ;
      _expectLID = SEADPT_INVALID_LID ;
      _expectRecHash = 0 ;
      _emptyResultSet = TRUE ;
   }

   _seAdptIndexSession::~_seAdptIndexSession()
   {
      if ( _esClt )
      {
         SDB_OSS_DEL _esClt ;
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
         rc = _onSDBEOC() ;
         PD_RC_CHECK( rc, PDERROR, "Unexpected end of collection, rc[ %d ]",
                      rc );
      }
      else if ( SDB_DMS_CS_NOTEXIST == reply->flags ||
                SDB_DMS_NOTEXIST == reply->flags ||
                SDB_DMS_CS_DELETING == reply->flags )
      {
         rc = _dropIndex() ;
         PD_RC_CHECK( rc, PDERROR, "Clean operation on index dropped "
                      "failed[ %d ]", rc ) ;
         goto error ;
      }
      else if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         // If local version of collection is old, or catalogue node is not
         // primary, it will be updated in next round.
         PD_LOG( ( SDB_CLS_COORD_NODE_CAT_VER_OLD == rc ||
                   SDB_CLS_NOT_PRIMARY == rc ) ? PDDEBUG : PDERROR,
                 "Query request failed[ %d ]. Current status[ %s ]",
                 rc, _seadptStatus2Desp( _status ) ) ;
         goto error ;
      }
      else
      {
         rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                               &numReturned, docObjs ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Extract query result failed[%d]", rc ) ;
            goto error ;
         }

         if ( SEADPT_SESSION_STAT_CLEAN_DATA == _status )
         {
            _lastPopLID = -1 ;
            _expectLID = SEADPT_INVALID_LID ;
            _switchStatus( SEADPT_SESSION_STAT_QUERY_NORMAL_TBL ) ;
            goto done ;
         }
         else if ( SEADPT_SESSION_STAT_COMP_LID == _status )
         {
            if ( 0 == docObjs.size() )
            {
               if ( -1 == _expectLID )
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
                          _meta.getCappedCLName().c_str() ) ;
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
               INT64 firstLID =
                  docObjs[0].getField( SDB_SEADPT_FIELD_NAME_ID ).Number() ;
               if ( SEADPT_INVALID_LID != _expectLID && _expectLID < firstLID )
               {
                  // ERROR
                  // need to clean the index and start over
                  PD_LOG( PDERROR, "The expected logical id is [ %lld ], but "
                          "the actual first logical id in capped collection "
                          "[ %s ] is [ %lld ]. Begin to start all over again",
                          _expectLID, _meta.getCappedCLName().c_str(),
                          firstLID ) ;
                  rc = _startOver() ;
                  PD_RC_CHECK( rc, PDERROR, "Restart the index work "
                               "failed[ %d ]", rc ) ;
               }
               else
               {
                  _lastPopLID = _expectLID ;
                  _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
                  _setQueryBusyFlag( FALSE ) ;
               }
               goto done ;
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
      _quit = TRUE ;
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
            goto error ;
      }

   done:
      return rc ;
   error:
      // If cs or cl not exists, the index is not there any longer.
      if ( SDB_DMS_NOTEXIST == rc || SDB_DMS_CS_NOTEXIST == rc )
      {
         INT32 rcTmp = _dropIndex() ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Clean operation on index dropped failed[ %d ]",
                    rcTmp  ) ;
         }

         // The index is dropped, this is not error for us.
         rc = SDB_OK ;
      }
      _quit = TRUE ;
      goto done ;
   }

   INT32 _seAdptIndexSession::handleKillCtxRes( NET_HANDLE handle,
                                                MsgHeader *msg )
   {
      MsgOpReply *reply = (MsgOpReply *)msg ;

      if ( SDB_OK != reply->flags )
      {
         PD_LOG( PDERROR, "Kill context request processing failed[ %d ]",
                 reply->flags ) ;
      }

      return SDB_OK ;
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

      if ( _quit )
      {
         goto done ;
      }

      // Consult and query triggers by timer.
      switch ( _status )
      {
         case SEADPT_SESSION_STAT_CONSULT:
            rc = _consult() ;
            PD_RC_CHECK( rc, PDERROR, "Consult failed[ %d ]", rc ) ;
            break ;
         case SEADPT_SESSION_STAT_BEGIN:
            {
               INT32 clVersion = -1 ;
               rc = sdbGetSeAdapterCB()->
                     syncUpdateCLVersion( _meta.getOrigCLName().c_str(),
                                          OSS_ONE_SEC, eduCB(), clVersion ) ;
               PD_RC_CHECK( rc, PDERROR, "Update collection version failed" ) ;
               PD_LOG( PDDEBUG, "Change cl[ %s ] version from [ %d ] to [ %d ]"
                       "accorrding to catalog", _meta.getOrigCLName().c_str(),
                       _origCLVersion, clVersion ) ;
               _origCLVersion = clVersion ;
               _switchStatus( SEADPT_SESSION_STAT_QUERY_NORMAL_TBL ) ;
            }
            break ;
         case SEADPT_SESSION_STAT_QUERY_NORMAL_TBL:
            // Only when the query is not in progress that we start a fresh
            // query.
            if ( !_isQueryBusy() )
            {
               INT32 clVersion = -1 ;
               rc = sdbGetSeAdapterCB()->
                     syncUpdateCLVersion( _meta.getOrigCLName().c_str(),
                                          OSS_ONE_SEC, eduCB(), clVersion ) ;
               PD_RC_CHECK( rc, PDERROR, "Update collection version "
                            "failed[ %d ]", rc ) ;
               PD_LOG( PDDEBUG, "Change cl[ %s ] version from [ %d ] to [ %d ]"
                       "accorrding to catalog", _meta.getOrigCLName().c_str(),
                       _origCLVersion, clVersion ) ;
               _origCLVersion = clVersion ;

               rc = _queryOrigCollection() ;
               PD_RC_CHECK( rc, PDERROR,
                            "Query original collection[ %s ] failed[ %d ]",
                            _meta.getOrigCLName().c_str(), rc ) ;
               _setQueryBusyFlag( TRUE ) ;
            }
            break ;
         case SEADPT_SESSION_STAT_QUERY_CAP_TBL:
            if ( !_isQueryBusy() )
            {
               try
               {
                  BSONObj condition ;
                  if ( _lastPopLID >= 0 )
                  {
                     condition = BSON( "_id" << BSON( "$gte" << _expectLID ) ) ;
                  }
                  rc = _queryCappedCollection( condition ) ;
                  PD_RC_CHECK( rc, PDERROR,
                               "Query capped collection[ %s ] failed[ %d ]",
                               _meta.getCappedCLName().c_str(), rc ) ;
                  _setQueryBusyFlag( TRUE ) ;
               }
               catch ( std::exception &e )
               {
                  rc = SDB_SYS ;
                  PD_LOG( PDERROR, "Unexpected exception occurred when query "
                          "capped collection[ %s ], error: %s",
                          _meta.getCappedCLName().c_str(), e.what() ) ;
                  goto error ;
               }
            }
            break ;
         default:
            break ;
      }

   done:
      return ;
   error:
      // If the original/capped collection space or collection dose not exist
      // any more, let's quit.
      if ( ( SDB_DMS_CS_NOTEXIST == rc || SDB_DMS_NOTEXIST == rc ) && !_quit )
      {
         INT32 rcTmp = _dropIndex() ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Drop index on search engine failed[%d] when "
                             "collection not exists", rcTmp ) ;
         }
         _quit = TRUE ;
      }
      goto done ;
   }

   // Build the query condition and selector for querying the original
   // collection. It's in the format of:
   // db.cs.cl.find({$or:[{field1:{$exists:1}}, ..., {fieldn:{$exists:1}}]},
   //               {_id:"", field1:"", ..., fieldn:""})
   // field1~fieldn are the fields in the text index.
   // So records without any index field will not be indexed on ES.
   void _seAdptIndexSession::_onAttach()
   {
      try
      {
         BSONObjBuilder queryBuilder ;
         BSONObjBuilder selectorBuilder ;
         BSONObjIterator idxItr( _meta.getIdxDef() ) ;
         BSONArrayBuilder queryObj( queryBuilder.subarrayStart( SEADPT_OPERATOR_STR_OR ) ) ;
         BSONObj existTmp = BSON( SEADPT_OPERATOR_STR_EXIST << 1 ) ;
         BSONObj includeObj = BSON( SEADPT_OPERATOR_STR_INCLUDE << 1 ) ;

         selectorBuilder.appendObject( SDB_SEADPT_FIELD_NAME_ID,
                                       includeObj.objdata(),
                                       includeObj.objsize() ) ;

         while ( idxItr.more() )
         {
            BSONElement ele = idxItr.next() ;
            const CHAR *fieldName = ele.fieldName() ;
            SDB_ASSERT( 0 != ossStrcmp( fieldName, SDB_SEADPT_FIELD_NAME_ID ),
                        "Text index should not include _id" ) ;
            selectorBuilder.appendObject( fieldName, includeObj.objdata(),
                                          includeObj.objsize() ) ;
            BSONObjBuilder existObj( queryObj.subobjStart() ) ;
            existObj.appendObject( fieldName, existTmp.objdata(),
                                   existTmp.objsize() ) ;
            existObj.done() ;
         }
         queryObj.done() ;
         _queryCond = queryBuilder.obj() ;
         _selector = selectorBuilder.obj() ;

         PD_LOG( PDDEBUG, "Original collection query, condition: %s, "
                 "selector: %s", _queryCond.toString().c_str(),
                 _selector.toString().c_str() ) ;

         PD_LOG( PDEVENT, "New index task starts: original collection[ %s ], "
                 "index[ %s ], capped collection[ %s ], search engine "
                 "index[ %s ], search engine type[ %s ]",
                 _meta.getOrigCLName().c_str(), _meta.getOrigIdxName().c_str(),
                 _meta.getCappedCLName().c_str(), _meta.getEsIdxName().c_str(),
                 _meta.getEsTypeName().c_str() ) ;
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
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;
      UINT64 requestID = 0 ;

      rc = msgBuildDisconnectMsg( (CHAR **)&msg, &bufSize, requestID, _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build disconnect message failed[ %d ]",
                   rc ) ;
      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send disconnect message to data node "
                   "failed[ %d ]", rc ) ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( (CHAR *)msg, _pEDUCB ) ;
      }
      return ;
   error:
      goto done ;
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
                               contextID, requestID, _pEDUCB ) ;
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
      if ( msgBuf )
      {
         msgReleaseBuffer( (CHAR *)msgBuf, _pEDUCB ) ;
      }
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
                              _meta.getOrigCLName().c_str(),
                              0, 0, 0, -1, &_queryCond, &_selector,
                              NULL, NULL, _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[ %d ]", rc ) ;
      msg->version = _origCLVersion ;
      msg->header.TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( (MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[ %d ]",
                   rc ) ;

      PD_LOG( PDDEBUG, "Send query on normal collection[ %s ] to data node "
              "successfully", _meta.getOrigCLName().c_str() ) ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( (CHAR *)msg, _pEDUCB ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_queryOneCappedRec()
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;
      BSONObj selector ;

      try
      {
         // In the capped collection, the field name of logical id is '_id', and
         // the original '_id' is named '_rid'.
         selector = BSON( SDB_SEADPT_FIELD_NAME_ID << "" ) ;
         rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize,
                                _meta.getCappedCLName().c_str(),
                                FLG_QUERY_WITH_RETURNDATA, 0, 0, 1, NULL,
                                &selector, NULL, NULL, _pEDUCB ) ;
         PD_RC_CHECK( rc, PDERROR, "Build query message failed[ %d ]", rc ) ;
         msg->TID = SEADPT_TID( _sessionID ) ;
         rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
         PD_RC_CHECK( rc, PDERROR, "Send query message to data node "
                      "failed[ %d ]", rc ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception happened: %s", e.what() ) ;
         goto error ;
      }

   done:
      if ( msg )
      {
         msgReleaseBuffer( (CHAR *)msg, _pEDUCB ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_truncateSrcCappedData()
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;
      BSONObj query ;
      const BSONObj emptyObj ;

      try
      {
         BSONObjBuilder builder ;
         builder.append( FIELD_NAME_COLLECTION,
                         _meta.getCappedCLName().c_str() ) ;
         query = builder.obj() ;
      }
      catch ( exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() );
         goto error ;
      }

      rc = msgBuildQueryCMDMsg( (CHAR **)&msg, &bufSize,
                                CMD_ADMIN_PREFIX CMD_NAME_TRUNCATE,
                                query,emptyObj, emptyObj, emptyObj, 0,
                                _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build truncate message failed[ %d ]", rc ) ;
      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send truncate message to data node "
                   "failed[ %d ]", rc ) ;
      PD_LOG( PDDEBUG, "Send truncate command on capped collection[ %s ] to "
              "data node successfully", _meta.getCappedCLName().c_str() ) ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( (CHAR *)msg, _pEDUCB ) ;
      }
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
                             _meta.getCappedCLName().c_str(),
                             0, 0, 0, -1,
                             ( condition.isEmpty() ) ? NULL : &condition,
                             NULL, NULL, NULL, _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[ %d ]", rc ) ;
      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[ %d ]",
                   rc ) ;
      PD_LOG( PDDEBUG, "Send query on capped collection[ %s ] to data node "
              "successfully", _meta.getCappedCLName().c_str() ) ;
   done:
      if ( msg )
      {
         msgReleaseBuffer( (CHAR *)msg, _pEDUCB ) ;
      }
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

      try
      {
         builder.append( FIELD_NAME_COLLECTION,
                         _meta.getCappedCLName().c_str() ) ;
         builder.appendIntOrLL( FIELD_NAME_LOGICAL_ID, recLID ) ;
         builder.appendIntOrLL( FIELD_NAME_DIRECTION, 1 ) ;

         query = builder.done() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception happened: %s", e.what() ) ;
         goto error ;
      }

      rc = msgBuildQueryMsg( (CHAR **)&msg, &msgLen,
                             CMD_ADMIN_PREFIX CMD_NAME_POP,
                             0, 0, -1, -1, &query, NULL, NULL, NULL, _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build pop message failed[ %d ], logical "
                   "id: %lld", rc, recLID ) ;

      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( (MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send pop command to data node failed[ %d ]",
                   rc ) ;
      PD_LOG( PDDEBUG, "Send pop command to data node: %s",
              query.toString().c_str() ) ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( (CHAR *)msg, _pEDUCB ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   // Parse the original object and get the items we want.
   INT32 _seAdptIndexSession::_parseCappedRecord( const BSONObj &origObj,
                                                  _rtnExtOprType &oprType,
                                                  string &finalID,
                                                  INT64 &logicalID,
                                                  BSONObj &sourceObj,
                                                  string *newFinalID )
   {
      INT32 rc = SDB_OK ;
      INT32 type = 0 ;

      try
      {
         BSONElement lidField = origObj.getField( SDB_SEADPT_FIELD_NAME_ID ) ;
         BSONElement ridEle = origObj.getField( SDB_SEADPT_FIELD_NAME_RID ) ;
         type = origObj.getIntField( FIELD_NAME_TYPE ) ;

         if ( lidField.eoo() || ridEle.eoo() )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "_id or _rid not found in record: %s",
                    origObj.toString().c_str() ) ;
            goto error ;
         }

         if ( !_typeSupport( type ) )
         {
            PD_LOG( PDERROR, "Operation type[ %d ] is not supported in "
                    "source record: %s", type, origObj.toString().c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         oprType = ( _rtnExtOprType )type ;
         logicalID = lidField.Long() ;

         encodeID( ridEle, finalID ) ;

         // When updating the _id field, it's a little complicated, as some data
         // types are not supported now, $minKey, for example. It may be updated
         // from unsupported type to one that we  support, or reverse scenario.
         // In these cases, the encode of the old or new _id will fail.
         // Need careful checking to handle all these cases.
         if ( RTN_EXT_UPDATE_WITH_ID == oprType )
         {
            BSONElement newIdEle =
                  origObj.getField( SDB_SEADPT_FIELD_NAME_RID_NEW ) ;
            if ( newIdEle.eoo() )
            {
               rc = SDB_SYS ;
               PD_LOG( PDERROR, "Get new _id from object failed[ %d ]", rc ) ;
               goto error ;
            }
            encodeID( newIdEle, *newFinalID ) ;
            // Change from unsupported type to another unsupported type, nothing
            // should be done.
            if ( finalID.empty() && newFinalID->empty() )
            {
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
         else
         {
            // For any other cases(_id not changed), if the encoding fails,
            // return error.
            if ( finalID.empty() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDWARNING, "Encode _id[ %s ] failed[ %d ]",
                       ridEle.toString( false, true ).c_str(), rc ) ;
               goto error ;
            }
         }

         {
            BSONObjBuilder builder ;
            BSONObj source = origObj.getObjectField( "_source" ) ;
            if ( !source.isEmpty() && !source.isValid() )
            {
               PD_LOG( PDERROR, "_source field is invalid. Object: %s",
                       origObj.toString().c_str() ) ;
               rc = SDB_SYS ;
               goto error ;
            }

            // Only keep the string fields. Fields of other type(include oid)
            // will be ignored.
            for ( BSONObj::iterator eleItr = source.begin(); eleItr.more(); )
            {
               BSONElement ele = eleItr.next() ;
               if ( String == ele.type() )
               {
                  builder.append( ele ) ;
               }
            }

            sourceObj = builder.obj() ;
         }
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

   INT32 _seAdptIndexSession::_parseNormalRecord( const BSONObj &origRecord,
                                                  string &finalID,
                                                  BSONObj &finalRecord )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder builder ;
      BOOLEAN hasOID = FALSE ;
      BSONElement idEle ;
      BOOLEAN hasStrField = FALSE ;

      try
      {
         // Only keep the string fields. Fields of other type(include oid)
         // will be ignored.
         for ( BSONObj::iterator eleItr = origRecord.begin(); eleItr.more(); )
         {
            BSONElement ele = eleItr.next() ;
            // Only keep the _id field and other fields of type String.
            if ( 0 == ossStrcmp( ele.fieldName(), SDB_SEADPT_FIELD_NAME_ID ) )
            {
               hasOID = TRUE ;
               idEle = ele ;
            }
            else if ( String == ele.type() )
            {
               builder.append( ele ) ;
               if ( !hasStrField )
               {
                  hasStrField = TRUE ;
               }
            }
            else if ( Array == ele.type() )
            {
               // Array with only one element(String type) will be indexed.
               BSONObjIterator itr( ele.embeddedObject() ) ;
               BSONElement subEle = itr.next() ;
               if ( !itr.more() && ( String == subEle.type() ) )
               {
                  builder.appendAs( subEle, ele.fieldName() ) ;
                  if ( !hasStrField )
                  {
                     hasStrField = TRUE ;
                  }
               }
            }
         }

         if ( !hasOID )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "No _id field in record: %s",
                    origRecord.toString( false, true ).c_str() ) ;
            goto error ;
         }

         // Only when there are index field of string type that we add the _rid
         // field. Otherwise, the final object should be empty.
         if ( hasStrField )
         {
            encodeID( idEle, finalID ) ;
            if ( finalID.empty() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDWARNING, "Encode id[ %s ] failed[ %d ]",
                       idEle.toString( false, true ).c_str(), rc ) ;
               goto error ;
            }
         }

         finalRecord = builder.obj() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
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

      try
      {
         if ( SDB_DMS_EOC == reply->flags )
         {
            // When reach to the end of the original collection, switch to capped
            // collection.
            BSONObj emptyObj = BSON( SEADPT_FIELD_NAME_CLUID << (INT64)_meta.getCLUID() <<
                                     SEADPT_FIELD_NAME_CLLID << _meta.getCLLID() <<
                                     SEADPT_FIELD_NAME_IDXLID << _meta.getIdxLID() <<
                                     SEADPT_FIELD_NAME_LID << _expectLID ) ;
            rc = _markProgress( emptyObj ) ;
            PD_RC_CHECK( rc, PDERROR, "Write end mark[_lid: %lld] for normal "
                         "collection[ %s ] on search engine failed[ %d ]",
                         _expectLID, _meta.getOrigCLName().c_str(), rc ) ;
            PD_LOG( PDEVENT, "Write end mark[_lid: %lld] for normal "
                    "collection[ %s ] on search engine successfully",
                    _expectLID, _meta.getOrigCLName().c_str() ) ;
            _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
            _setQueryBusyFlag( FALSE ) ;
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

         rc = _bulkPrepare() ;
         PD_RC_CHECK( rc, PDERROR, "Prepare of bulk operation failed[ %d ]",
                      rc ) ;

         for ( vector<BSONObj>::const_iterator itr = docObjs.begin();
               itr != docObjs.end(); ++itr )
         {
            BSONObj finalRec ;
            string finalID ;
            rc = _parseNormalRecord( *itr, finalID, finalRec ) ;
            if ( SDB_INVALIDARG == rc || finalRec.isEmpty() )
            {
               continue ;
            }
            else if ( rc )
            {
               PD_LOG( PDERROR, "Format record[ %s ] failed[ %d ]",
                       itr->toString( false, true ).c_str(), rc ) ;
               goto error ;
            }

            if ( finalID.size() > SEADPT_ID_MAX_SZ )
            {
               PD_LOG( PDDEBUG, "Ignore document as actual id length[%d] "
                                "exceeds limit[%d]. id value: %s",
                                finalID.size(), SEADPT_ID_MAX_SZ,
                                finalID.c_str() ) ;
               continue ;
            }

            {
               utilESActionIndex item( _meta.getEsIdxName().c_str(), _meta.getEsTypeName().c_str() ) ;
               rc = item.setID( finalID ) ;
               PD_RC_CHECK( rc, PDERROR, "Set _id for action failed[ %d ]",
                            rc ) ;
               rc = item.setSourceData( finalRec.toString(false, true).c_str(),
                                        finalRec.toString(false, true).length(),
                                        TRUE ) ;
               PD_RC_CHECK( rc, PDERROR, "Set source data for action "
                            "failed[ %d ]", rc ) ;

               rc = _bulkProcess( item ) ;
               PD_RC_CHECK( rc, PDERROR, "Bulk processing item failed[ %d ]",
                            rc ) ;
            }
         }

         rc = _bulkFinish() ;
         PD_RC_CHECK( rc, PDERROR, "Finish operation of bulk failed[ %d ]",
                      rc ) ;

         rc = _sendGetmoreReq( contextID, msg->requestID ) ;
         PD_RC_CHECK( rc, PDERROR, "Send get more request failed[ %d ]", rc ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception happened: %s", e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      _quit = TRUE ;
      goto done ;
   }

   INT32 _seAdptIndexSession::_processCappedCLRecords( NET_HANDLE handle,
                                                       MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      INT64 contextID = 0 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      INT64 nextLastLID = -1 ;
      vector<BSONObj> docObjs ;
      INT64 logicalID = -1 ;
      UINT32 lastObjHash = 0 ;

      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, docObjs ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to extract query result, rc: %d", rc ) ;
         goto error ;
      }

      rc = flag ;

      // If reach end of the capped collection, start another query on it. It
      // will be started in onTimer.
      if ( SDB_DMS_EOC == rc )
      {
         // If the expected LID is -1 and the capped collection is empty, there
         // are two possibilities:
         // (1) All records in the original collection have been processed, and
         //     no more record change after the index creation
         // (2) The capped collection has been re-created. That may happen when
         //     the collection is re-created, or the original table is truncated.
         // So we check the meta data. If it's the second condition, let's just
         // quit. The new worker will drop and create the ES index during
         // consult.
         if ( SEADPT_INVALID_LID == _expectLID && _emptyResultSet )
         {
            if ( FALSE == ((_seIndexSessionMgr *)_pSessionMgr)->sessionMetaCheck( _meta ) )
            {
               // Not able to find the record we expected in the capped collection.
               rc = SDB_SYS ;
               PD_LOG( PDERROR, "Can not find expected record in capped "
                       "collection[ %s ]", _meta.getCappedCLName().c_str() ) ;
               goto error ;
            }
         }
         _emptyResultSet = TRUE ;
         rc = SDB_OK ;
         _setQueryBusyFlag( FALSE ) ;
         PD_LOG( PDDEBUG, "All records in capped collection[ %s ] have been "
                 "processed. Ready to start a new query on it",
                 _meta.getCappedCLName().c_str() ) ;
         goto done ;
      }
      else if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Get more failed[ %d ]", rc ) ;
         // Start another query.
         _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
         _setQueryBusyFlag( FALSE ) ;
         goto error ;
      }

      try
      {
         BOOLEAN firstBatch = FALSE ;
         if ( docObjs.size() > 0 )
         {
            // Check if the first record is the one we expected. If not, start
            // over again. Check only once in one query round.
            if ( _emptyResultSet && ( SEADPT_INVALID_LID != _expectLID )
                 && ( 0 != _expectRecHash ) )
            {
               BSONObj firstObj = docObjs.front() ;
               UINT32 objHash = ossHash( firstObj.objdata(),
                                         firstObj.objsize() ) ;
               if ( objHash != _expectRecHash )
               {
                  rc = SDB_SYS ;
                  PD_LOG( PDERROR, "The first record in capped collection[ %s ]"
                          " is not as we expected",
                          _meta.getCappedCLName().c_str() ) ;
                  goto error ;
               }
               _emptyResultSet = FALSE ;
               // If only one, it's the last one we processed in the last round.
               // No new data has been added. So nothing to do. Just try to get
               // more records.
               if ( docObjs.size() == 1 )
               {
                  rc = _sendGetmoreReq( contextID, msg->requestID ) ;
                  PD_RC_CHECK( rc, PDERROR,
                               "Send get more request failed[ %d ]", rc ) ;
                  goto done ;
               }
               firstBatch = TRUE ;
            }
            BSONObj lastObj = docObjs.back() ;
            BSONElement lidEle = lastObj.getField( SDB_SEADPT_FIELD_NAME_ID ) ;
            nextLastLID = lidEle.Long() ;
            lastObjHash = ossHash( lastObj.objdata(), lastObj.objsize() ) ;
            if ( _emptyResultSet )
            {
               _emptyResultSet = FALSE ;
            }
         }

         rc = _bulkPrepare() ;
         PD_RC_CHECK( rc, PDERROR, "Prepare of bulk operation failed[ %d ]",
                      rc ) ;

         vector<BSONObj>::const_iterator itr = docObjs.begin() ;

         // We always get one more record, if the _expectLID is not -1. So the
         // first one should be filtered out.
         if ( SEADPT_INVALID_LID != _expectLID && firstBatch )
         {
            itr++ ;
         }
         for ( ; itr != docObjs.end(); ++itr )
         {
            BSONObj newNameObj ;
            string idStr ;
            BSONObj sourceObj ;
            _rtnExtOprType oprType = RTN_EXT_INVALID ;
            string finalID ;
            string finalIdNew ;

            rc = _parseCappedRecord( *itr, oprType, finalID, logicalID,
                                     sourceObj, &finalIdNew ) ;
            if ( rc )
            {
               if ( SDB_INVALIDARG == rc )
               {
                  continue ;
               }
               PD_LOG( PDERROR, "Get id string and source object failed[ %d ]",
                       rc ) ;
               goto error ;
            }

            if ( sourceObj.isEmpty() )
            {
               if ( RTN_EXT_INSERT == oprType )
               {
                  // Nothing should be inserted.
                  continue ;
               }
               else if ( RTN_EXT_UPDATE == oprType )
               {
                  // If no index field of type string, the document should be
                  // removed. So we directly change it into delete.
                  oprType = RTN_EXT_DELETE ;
               }
            }

            if ( finalID.size() > SEADPT_ID_MAX_SZ )
            {
               PD_LOG( PDDEBUG, "Ignore document as actual id length[%d] "
                                "exceeds limit[%d]. id value: %s",
                       finalID.size(), SEADPT_ID_MAX_SZ,
                       finalID.c_str() ) ;
               continue ;
            }

            switch ( oprType )
            {
               // In case of update, we are going to update the whole document,
               // not part of it. So indexing should be done instead of updating.
               case RTN_EXT_INSERT:
               case RTN_EXT_UPDATE:
                  {
                     rc = _index( finalID, sourceObj ) ;
                     PD_RC_CHECK( rc, PDERROR, "Index document failed[ %d ]",
                                  rc ) ;
                     break ;
                  }
               case RTN_EXT_DELETE:
                  {
                     rc = _delete( finalID ) ;
                     PD_RC_CHECK( rc, PDERROR, "Delete document failed[ %d ]",
                                  rc ) ;
                     break ;
                  }
               case RTN_EXT_UPDATE_WITH_ID:
                  {
                     rc = _replace( finalID, finalIdNew, sourceObj ) ;
                     PD_RC_CHECK( rc, PDERROR, "Update record failed[ %d ]",
                                  rc ) ;
                     break ;
                  }
               default:
                  PD_LOG( PDERROR, "Invalid operation type[ %d ] in source data",
                          oprType ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
            }
         }

         rc = _bulkFinish() ;
         PD_RC_CHECK( rc, PDERROR, "Finish operation of bulk "
                      "failed[ %d ]", rc ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( -1 != logicalID )
      {
         rc = _updateProgress( logicalID ) ;
         PD_RC_CHECK( rc, PDERROR, "Update progress failed[ %d ]", rc ) ;
      }

      // After each batch of data processed successfully, pop the last batch of
      // data out. If -1 == _lastPopLID, it's the first time. We do not pop in
      // this round, just to fetch the next batch of results.
      // If _expectRecHash is 0, this is the first round after startup. We do
      // not clean any thing this round. Otherwise, all records in capped
      // collection may be cleaned.
      if ( _lastPopLID >= 0 && 0 != _expectRecHash )
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
      _expectLID = nextLastLID ;
      _expectRecHash = lastObjHash ;

   done:
      return rc ;
   error:
      _quit = TRUE ;
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
                 _meta.getCappedCLName().c_str(), rc ) ;
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

      try
      {
         rc = _esClt->indexDocument( _meta.getEsIdxName().c_str(), _meta.getEsTypeName().c_str(),
                                     SDB_SEADPT_COMMIT_ID,
                                     infoObj.toString().c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Index document failed[ %d ]", rc ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_updateProgress( INT64 logicalID )
   {
      BSONObj lidObj = BSON( SEADPT_FIELD_NAME_CLUID << (INT64)_meta.getCLUID()
                             << SEADPT_FIELD_NAME_CLLID << _meta.getCLLID() <<
                             SEADPT_FIELD_NAME_IDXLID << _meta.getIdxLID() <<
                             SEADPT_FIELD_NAME_LID << logicalID ) ;
      return _markProgress( lidObj ) ;
   }

   INT32 _seAdptIndexSession::_validate( const BSONObj &obj, BOOLEAN &valid )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN validate = FALSE ;
      try
      {
         validate = ( ( (UINT32)obj.getIntField( SEADPT_FIELD_NAME_CLUID ) == _meta.getCLUID() ) &&
         ( (UINT32)obj.getIntField( SEADPT_FIELD_NAME_CLLID ) == _meta.getCLLID() ) &&
         ( (UINT32)obj.getIntField( SEADPT_FIELD_NAME_IDXLID ) == _meta.getIdxLID() ) ) ;

         valid = validate ;
       }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // The consult step and strategy is as follows:
   // First, check if the index exist on ES. If not, then need to do full
   // indexing.
   // If index exists, check the commit mark. If not found, it means the
   // last indexing process is interrupted when processing data of the
   // original collection. In this case, we do not know where to start. So
   // we start all around.
   // If index exists and the commit mark is found, get the _lid in it.
   // Suggest it's called m. Get the last logical id in the capped
   // collection. Suggest it's n.
   // (1) If n does not exist( capped collection is empty), and m is not -1,
   //     start all around.
   // (2) If m is -1 and n is greater than 0, start all around.
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
      const CHAR *esIdxName = _meta.getEsIdxName().c_str() ;

      if ( !_esClt )
      {
         rc = sdbGetSeAdapterCB()->getSeCltFactory()->create( &_esClt ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get search engine client, rc: %d",
                    rc ) ;
            goto error ;
         }
      }

      // 1. Check index existence.
      rc = _esClt->indexExist( esIdxName, found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check index[ %s ] existence on search engine "
                   "failed[ %d ]", esIdxName, rc ) ;
      if ( !found )
      {
         PD_LOG( PDEVENT, "Target index[ %s ] dose not exist on search engine. "
                 "Start all over again and the index will be re-created",
                 esIdxName ) ;
         rc = _truncateSrcCappedData() ;
         PD_RC_CHECK( rc, PDERROR, "Clean source capped data failed[ %d ]",
                      rc ) ;
         PD_LOG( PDEVENT, "No index on search engine. Clean the source capped "
                 "data successfully" ) ;

         rc = _createIndex() ;
         PD_RC_CHECK( rc, PDERROR, "Create index failed[ %d ]", rc ) ;
         _switchStatus( SEADPT_SESSION_STAT_CLEAN_DATA ) ;
         goto done ;
      }

      try
      {
         rc = _esClt->getDocument( esIdxName, _meta.getEsTypeName().c_str(),
                                   SDB_SEADPT_COMMIT_ID, resultObj, FALSE ) ;
         if ( SDB_INVALIDARG == rc )
         {
            PD_LOG( PDEVENT, "Commit mark for index[ %s ] "
                    "dose not exist. Index will be dropped and recreated",
                    esIdxName ) ;

            rc = _truncateSrcCappedData() ;
            PD_RC_CHECK( rc, PDERROR, "Clean source capped data failed[ %d ]",
                      rc ) ;
            rc = _createIndex( TRUE ) ;
            PD_RC_CHECK( rc, PDERROR, "Create index by force failed[ %d ]",
                         rc ) ;
            _switchStatus( SEADPT_SESSION_STAT_CLEAN_DATA ) ;
            goto done ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Check commit mark on search engine failed[ %d ]",
                    rc ) ;
            goto error ;
         }

         {
            BOOLEAN valid = FALSE ;
            rc = _validate( resultObj, valid ) ;
            PD_RC_CHECK( rc, PDERROR, "Validate of ES document failed[ %d ]",
                         rc ) ;
            if ( FALSE == valid )
            {
               rc = _startOver() ;
               PD_RC_CHECK( rc, PDERROR, "Start over again failed[ %d ]", rc ) ;
               goto done ;
            }
         }
         // Get the commit logical id as expect logical id.
         lidEle = resultObj.getField( SEADPT_FIELD_NAME_LID ) ;
         PD_LOG( PDDEBUG, "Commit object: %s", resultObj.toString().c_str() ) ;
         _expectLID = lidEle.numberLong() ;

         rc = _queryOneCappedRec() ;
         PD_RC_CHECK( rc, PDERROR, "Query first logical id failed[ %d ]", rc ) ;
         _switchStatus( SEADPT_SESSION_STAT_COMP_LID ) ;
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
      _quit =  TRUE ;
      goto done ;
   }

   INT32 _seAdptIndexSession::_onSDBEOC()
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;
      UINT64 requestID = 0 ;

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
                 _meta.getOrigCLName().c_str() ) ;
         rc = SDB_DMS_NOTEXIST ;
         goto error ; ;
      }

      rc = msgBuildKillContextsMsg( (CHAR **)&msg, &bufSize, requestID,
                                    1, &_queryCtxID, _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build kill context message failed[ %d ]",
                   rc ) ;
      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send kill context message to data node "
                   "failed[ %d ]", rc ) ;
      _queryCtxID = -1 ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( (CHAR *)msg, _pEDUCB ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   // Start all over again. If the index exists, it will be dropped and
   // re-created later.
   INT32 _seAdptIndexSession::_startOver()
   {
      INT32 rc = SDB_OK ;

      rc = _dropIndex() ;
      PD_RC_CHECK( rc, PDERROR, "Drop index[ %s ] on search engine "
                   "failed[ %d ]", _meta.getEsIdxName().c_str(), rc ) ;

      _switchStatus( SEADPT_SESSION_STAT_CONSULT ) ;
      _setQueryBusyFlag( FALSE ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_bulkPrepare()
   {
      INT32 rc = SDB_OK ;
      if ( _bulkBuilder.isInit() )
      {
         _bulkBuilder.reset() ;
      }
      else
      {
         UINT32 bulkBuffSize = UTIL_ESBULK_DFT_SIZE ;
         const seAdptOptionsMgr *optionMgr =
            ((seIndexSessionMgr *)_pSessionMgr)->getOptionMgr() ; ;
         if ( optionMgr )
         {
            bulkBuffSize = optionMgr->getBulkBuffSize() ;
         }

         rc = _bulkBuilder.init( bulkBuffSize ) ;
         PD_RC_CHECK( rc, PDERROR, "Initialize bulk builder failed[ %d ]",
                      rc ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_bulkProcess( const utilESBulkActionBase &actionItem )
   {
      INT32 rc = SDB_OK ;

      // If reaching the size limit of the bulk builder, fire the operation and
      // go on with the next batch of data.
      if ( actionItem.outSizeEstimate() > _bulkBuilder.getFreeSize() )
      {
         if ( _bulkBuilder.getItemNum() > 0 )
         {
            rc = _esClt->bulk( _meta.getEsIdxName().c_str(),
                               _meta.getEsTypeName().c_str(),
                               _bulkBuilder.getData() ) ;
            PD_RC_CHECK( rc, PDERROR, "Bulk operation failed[ %d ]" ) ;

            PD_LOG( PDDEBUG, "Index documents in bulk mode successfully. "
                             "Document number[%u]. Total size[%u]",
                    _bulkBuilder.getItemNum(), _bulkBuilder.getDataLen() ) ;

            _bulkBuilder.reset() ;
         }
         else
         {
            // Buffer is not enough. Process it separately instead of using
            // _bulk.
            rc = _processBigItem( actionItem ) ;
            PD_RC_CHECK( rc, PDERROR, "Process big item failed[ %d ]", rc ) ;
            goto done ;
         }
      }
      rc = _bulkBuilder.appendItem( actionItem, FALSE, FALSE, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Append item to bulk builder failed[ %d ]",
                   rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_bulkFinish()
   {
      INT32 rc = SDB_OK ;

      if ( _bulkBuilder.getDataLen() > 0 )
      {
         rc = _esClt->bulk( _meta.getEsIdxName().c_str(),
                            _meta.getEsTypeName().c_str(),
                            _bulkBuilder.getData() ) ;
         PD_RC_CHECK( rc, PDERROR, "Bulk operation failed[ %d ]", rc ) ;
         PD_LOG( PDDEBUG, "Index documents in bulk mode successfully. "
                          "Document number[%u]. Total size[%u]",
                 _bulkBuilder.getItemNum(), _bulkBuilder.getDataLen() ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_processBigItem( const utilESBulkActionBase &actionItem )
   {
      INT32 rc = SDB_OK ;

      // Currently only index and update may exceed bulk buffer. They both use
      // the indexDocument interface.
      SDB_ASSERT( UTIL_ES_ACTION_INDEX == actionItem.getActionType(),
                  "Type is not index" ) ;

      rc = _esClt->indexDocument( actionItem.getIndexName().c_str(),
                                  actionItem.getTypeName().c_str(),
                                  actionItem.getID().c_str(),
                                  actionItem.getSrcData() ) ;
      PD_RC_CHECK( rc, PDERROR, "Index document failed[ %d ]", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_createIndex( BOOLEAN force )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN found = FALSE ;
      const CHAR *esIdxName = _meta.getEsIdxName().c_str() ;

      rc = _esClt->indexExist( esIdxName, found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check index[ %s ] existence on search engine "
                   "failed[ %d ]", esIdxName, rc ) ;
      if ( found )
      {
         if ( force )
         {
            rc = _esClt->dropIndex( esIdxName ) ;
            PD_RC_CHECK( rc, PDERROR, "Drop index[ %s ] on search engine "
                            "failed[ %d ]", esIdxName, rc ) ;
            PD_LOG( PDEVENT, "Index[ %s ] dropped successfully",
                    esIdxName ) ;
         }
         else
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "Duplicated index[ %s ] exists on search engine, "
                    "rc[ %d ]", esIdxName, rc ) ;
            goto error ;
         }
      }

      try
      {
         BSONObj mappingObj ;
         utilESMapping mapping( esIdxName, _meta.getEsTypeName().c_str() ) ;

         BSONObjIterator itr( _meta.getIdxDef() ) ;
         while ( itr.more() )
         {
            BSONElement ele = itr.next() ;
            mapping.addProperty( ele.fieldName(), ES_TEXT ) ;
         }

         rc = mapping.toObj( mappingObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Build mapping object failed[ %d ]", rc ) ;

         rc = _esClt->createIndex( esIdxName,
                                   mappingObj.toString( FALSE, TRUE).c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Create index[ %s ] with mapping[ %s ] on "
                      "search engine failed[ %d ]", esIdxName,
                      mappingObj.toString( FALSE, TRUE).c_str(), rc ) ;
         PD_LOG( PDEVENT, "Create index[ %s ] with mapping[ %s ] on "
                 "search engine successfully", esIdxName,
                 mappingObj.toString( FALSE, TRUE).c_str() ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_dropIndex()
   {
      INT32 rc = SDB_OK ;
      BOOLEAN found = FALSE ;
      const CHAR *esIdxName = _meta.getEsIdxName().c_str() ;

      rc = _esClt->indexExist( esIdxName, found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check index[ %s ] existence on search engine "
                   "failed[ %d ]", esIdxName, rc ) ;

      if ( found )
      {
         rc = _esClt->dropIndex( esIdxName ) ;
         PD_RC_CHECK( rc, PDERROR, "Drop index[ %s ] on search engine "
                         "failed[ %d ]", esIdxName, rc ) ;
         PD_LOG( PDEVENT, "Index[ %s ] dropped successfully", esIdxName ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_index( const string &id,
                                      const BSONObj &document )
   {
      INT32 rc = SDB_OK ;

      utilESActionIndex item( _meta.getEsIdxName().c_str(),
                              _meta.getEsTypeName().c_str() ) ;
      rc = item.setID( id ) ;
      PD_RC_CHECK( rc, PDERROR, "Set _id for action "
                                "failed[ %d ]", rc ) ;
      rc = item.setSourceData( document.toString(false, true).c_str(),
                               document.toString(false, true).length(),
                               TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Set source data failed[ %d ]",
                   rc ) ;
      rc = _bulkProcess( item ) ;
      PD_RC_CHECK( rc, PDERROR, "Bulk processing item "
                                "failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
       goto done ;
   }

   INT32 _seAdptIndexSession::_delete( const string &id )
   {
      INT32 rc = SDB_OK ;

      utilESActionDelete item( _meta.getEsIdxName().c_str(),
                               _meta.getEsTypeName().c_str() ) ;
      rc = item.setID( id ) ;
      PD_RC_CHECK( rc, PDERROR, "Set _id for action "
                                "failed[ %d ]", rc ) ;
      rc = _bulkProcess( item ) ;
      PD_RC_CHECK( rc, PDERROR, "Bulk processing item "
                                "failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // Replace is delete + index.
   INT32 _seAdptIndexSession::_replace( const string &id, const string &newId,
                                        const BSONObj &document )
   {
      INT32 rc = SDB_OK ;

      if ( !id.empty() )
      {
         rc = _delete( id ) ;
         PD_RC_CHECK( rc, PDERROR, "delete document of _id[ %d ] failed[ %d ]",
                      id.c_str(), rc ) ;
      }

      if ( !newId.empty() )
      {
         rc = _index( newId, document ) ;
         PD_RC_CHECK( rc, PDERROR, "Index document[ %s ] with _id[ %d ] "
                                   "failed[ %d ]",
                      document.toString(false, true).c_str(), newId.c_str(),
                      rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _seAdptIndexSession::_typeSupport( INT32 type )
   {
      return ( type > RTN_EXT_INVALID && type < RTN_EXT_DUMMY ) ;
   }
}

