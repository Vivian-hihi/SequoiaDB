#include "msgMessage.hpp"
#include "seAdptIndexSession.hpp"
#include "seAdptMgr.hpp"

#define SEADPT_NORMAL_CL_DONE_ID     "SDBCOMMIT"
#define SEADPT_NORMAL_CL_DONE_BODY   "{}"

namespace engine
{
   BEGIN_OBJ_MSG_MAP( _seAdptIndexSession, _pmdAsyncSession)
      ON_MSG( MSG_BS_QUERY_RES, _onResMsg )
      ON_MSG( MSG_BS_GETMORE_RES, _onResMsg )
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

   void _seAdptIndexSession::_begin()
   {
      INT32 rc = SDB_OK ;

      rc = _consult() ;
      PD_RC_CHECK( rc, PDERROR, "Consult indexing progress of index[ %s ] "
                   "failed[ %d ]", _indexName.c_str(), rc ) ;

   done:
      return ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_getMoreSdbRecords( INT64 contextID,
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

   done:
      return rc ;
   error:
      goto done ;
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

      // When get the replay of query, go on to send a get more request.
      if ( SDB_DMS_EOC == reply->flags )
      {
         _onSDBEOC() ;
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
         PD_LOG( PDERROR, "Query failed[ %d ]", rc ) ;
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

         rc = _getMoreSdbRecords( contextID, msg->requestID ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get more records, rc: %d", rc ) ;
            goto error ;
         }

         switch ( _status )
         {
            case SEADPT_SESSION_STAT_QUERY_NORMAL_TBL:
               _status = SEADPT_SESSION_STAT_GETMORE_NORMAL ;
               break ;
            case SEADPT_SESSION_STAT_QUERY_CAP_TBL:
               _status = SEADPT_SESSION_STAT_GETMORE_CAP ;
               break ;
            case SEADPT_SESSION_STAT_POP_CAP:
               _status = SEADPT_SESSION_STAT_GETMORE_CAP ;
               break ;
            case SEADPT_SESSION_STAT_CONSULT:
               break ;
            case SEADPT_SESSION_STAT_WAIT_NQ_RES:
               break ;
            default:
               SDB_ASSERT( FALSE, "Status is wrong" ) ;
               rc = SDB_SYS ;
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
         case SEADPT_SESSION_STAT_WAIT_NQ_RES:
         case SEADPT_SESSION_STAT_GETMORE_NORMAL:
            rc = _processNormalCLRecords( handle, msg ) ;
            PD_RC_CHECK( rc, PDERROR, "Process data of original collection "
                         "failed[ %d ]", rc ) ;
            break ;
         case SEADPT_SESSION_STAT_GETMORE_CAP:
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
      PD_LOG( PDDEBUG, "Received message" ) ;

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
            PD_LOG( PDERROR, "Failed to handle get more respond, rc: %d", rc ) ;
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
            rc = _consult() ;
            break ;
         case SEADPT_SESSION_STAT_CONSULT:
            rc = _sendQueryReq( TRUE ) ;
            if ( SDB_OK == rc )
            {
               _switchStatus( SEADPT_SESSION_STAT_CONSULT_CQ_RES ) ;
            }
            break ;
         case SEADPT_SESSION_STAT_QUERY_NORMAL_TBL:
            rc = _sendQueryReq( FALSE ) ;
            if ( SDB_OK == rc )
            {
               _switchStatus( SEADPT_SESSION_STAT_WAIT_NQ_RES ) ;
            }
            break ;
         case SEADPT_SESSION_STAT_QUERY_CAP_TBL:
            rc = _sendQueryReq( TRUE ) ;
            if ( SDB_OK == rc )
            {
               _switchStatus( SEADPT_SESSION_STAT_WAIT_CQ_RES ) ;
            }
            break ;
         default:
            break ;
      }

   done:
      return ;
   }

   void _seAdptIndexSession::_onAttach()
   {
      BSONObjBuilder builder ;
      BSONObjIterator idxItr( _indexDef ) ;

      while ( idxItr.more() )
      {
         BSONElement ele = idxItr.next() ;
         const CHAR *fieldName = ele.fieldName() ;
         builder.append( fieldName, "" ) ;
      }

      _selector = builder.obj() ;

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

   INT32 _seAdptIndexSession::_progressConsult()
   {
      INT32 rc = SDB_OK ;

      // The consult logic is as follows:
      // 1. Get a batch of records from capped collection.
      // 2. Traverse the records, get the _lid and check if it's on ES. We can
      //    not use the _id field to check, as there may have multiple records
      //    with the same _id in the capped collection. We should be sure the
      //    last one is in ES.
      _status = SEADPT_SESSION_STAT_CONSULT ;
      rc = _sendQueryReq() ;
      PD_RC_CHECK( rc, PDERROR, "Send query request to data node failed[ %d ]",
                   rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _seAdptIndexSession::_switchStatus( SEADPT_SESSION_STATUS newStatus )
   {
      _status = newStatus ;
   }

   INT32 _seAdptIndexSession::_sendQueryReq()
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;

      // Check on ES if the index exist. If not, create one. Then begin
      // Check on ES the status of the current index. And send query of the
      // capped collection to data node.
      switch ( _status )
      {
         case SEADPT_SESSION_STAT_CONSULT:
         case SEADPT_SESSION_STAT_QUERY_CAP_TBL:
            rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize,
                                   _cappedCLFullName.c_str(),
                                   0, 0, 0, -1, NULL, NULL ) ;
            break ;
         case SEADPT_SESSION_STAT_QUERY_NORMAL_TBL:
            // of all the data from the original table, go ahead with the data
            // in capped collection which are behind this point.
            // We can pop all the records in the capped collection. truncate ?
            rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize,
                                    _origCLFullName.c_str(),
                                    0, 0, 0, -1, NULL, &_selector ) ;
            break ;
         default:
            SDB_ASSERT( FALSE, "Status invalid" ) ;
            break ;
      }
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[ %d ]", rc ) ;

      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[ %d ]",
                   rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_sendQueryReq( BOOLEAN queryCappedCL )
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 bufSize = 0 ;

      if ( queryCappedCL )
      {
         rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize,
                                _cappedCLFullName.c_str(),
                                0, 0, 0, -1, NULL, NULL ) ;
      }
      else
      {
         rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize,
                                 _origCLFullName.c_str(),
                                 0, 0, 0, -1, NULL, &_selector ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[ %d ]", rc ) ;

      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[ %d ]",
                   rc ) ;

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

      PD_LOG( PDDEBUG, "Pop command to data node: %s",
              query.toString().c_str() ) ;

      rc = msgBuildQueryMsg( (CHAR **)&msg, &msgLen,
                             CMD_ADMIN_PREFIX CMD_NAME_POP,
                              0, 0, -1, -1, &query, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Build pop message failed[ %d ], logical "
                   "id: %lld", rc, recLID ) ;

      msg->TID = SEADPT_TID( _sessionID ) ;
      rc = sdbGetSeAdapterCB()->sendToDataNode( (MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send pop command to data node failed[ %d ]",
                   rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // Parse the original object and get the items we want.
   INT32 _seAdptIndexSession::_parseSrcData( const BSONObj &origObj,
                                             _dmsExtOprType &oprType,
                                             OID &oid,
                                             BSONObj &sourceObj )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN exist = FALSE ;
      INT32 type = 0 ;
      BSONElement idEle ;
      BSONElement sourceEle ;

      type = origObj.getIntField( FIELD_NAME_TYPE ) ;

      if ( type < DMS_EXT_INSERT || type > DMS_EXT_TRUNCATE )
      {
         PD_LOG( PDERROR, "Operation type[ %d ] is invalid in source object",
                 type ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      oprType = ( _dmsExtOprType )type ;
      exist = origObj.getObjectID( idEle ) ;
      if ( !exist )
      {
         PD_LOG( PDERROR, "_id dose not exist in source object" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      oid = idEle.OID() ;
      sourceObj = origObj.getObjectField( "_source" ) ;

      PD_LOG( PDDEBUG, "Operation type: %d, _id: %s, _source: %s", oprType,
              oid.toString().c_str(), sourceObj.toString().c_str() ) ;

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

      if ( SEADPT_SESSION_STAT_GETMORE_NORMAL != _status )
      {
         _status = SEADPT_SESSION_STAT_GETMORE_NORMAL ;
      }

      if ( SDB_DMS_EOC == reply->flags )
      {
         // When reach to the end of the original collection, switch to capped
         // collection.
         rc = _markNormalCLDone() ;
         PD_RC_CHECK( rc, PDERROR, "Write end mark of normal collection on "
                      "search engine failed[ %d ]", rc ) ;

         _status = SEADPT_SESSION_STAT_QUERY_CAP_TBL ;

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

      rc = _getMoreSdbRecords( contextID, msg->requestID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get more request, rc: %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
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

      if ( SEADPT_SESSION_STAT_GETMORE_CAP != _status)
      {
         _status = SEADPT_SESSION_STAT_GETMORE_CAP ;
      }

      // If reach end of the capped collection, start another query on it.
      if ( SDB_DMS_EOC == reply->flags )
      {
         if ( _lastPopLID >= 0 )
         {
            rc = _cleanData( _lastPopLID ) ;
            PD_RC_CHECK( rc, PDERROR, "Clean data failed[ %d ]", rc ) ;
         }
         _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
         goto done ;
      }
      else if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "Get more failed[ %d ]", rc ) ;
         // Start another query.
         _status = SEADPT_SESSION_STAT_QUERY_CAP_TBL ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, docObjs ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to extract query result, rc: %d", rc ) ;
         goto error ;
      }

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

      if ( docObjs.size() > 0 )
      {
         BSONObj lastObj = docObjs.back() ;
         BSONElement lidEle = lastObj.getField( SEADPT_LOGICAL_ID ) ;
         nextLastLID = lidEle.Long() ;
      }

      for ( vector<BSONObj>::const_iterator itr = docObjs.begin();
            itr != docObjs.end(); ++itr )
      {
         BSONObj newNameObj ;
         string idStr ;
         BSONObj sourceObj ;
         _dmsExtOprType oprType = DMS_EXT_INVALID ;
         OID idEle ;
         BSONElement sourceEle ;

         // Parse and the original object into 4 items: operation type, id,
         // source data( the real data to be indexed on Elasticsearch ).
         rc = _parseSrcData( *itr, oprType, idEle, sourceObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Get id string and source object "
                      "failed[ %d ]", rc ) ;
         switch ( oprType )
         {
            case DMS_EXT_INSERT:
               rc = _esClt->indexDocument( _indexName.c_str(),
                                           _typeName.c_str(),
                                           idEle.toString().c_str(),
                                           sourceObj.toString().c_str() ) ;
               PD_RC_CHECK( rc, PDERROR, "Index record on search engine "
                            "failed[ %d ], id: %s", rc,
                            idEle.toString().c_str() ) ;
               break ;
            case DMS_EXT_DELETE:
               rc = _esClt->deleteDocument( _indexName.c_str(),
                                            _typeName.c_str(),
                                            idEle.toString().c_str() ) ;
               PD_RC_CHECK( rc, PDERROR, "Delete document on search engine "
                            "failed[ %d ], _id: %s", rc,
                            idEle.toString().c_str() ) ;
               break ;
            case DMS_EXT_UPDATE:
               rc = _esClt->updateDocument( _indexName.c_str(),
                                            _typeName.c_str(),
                                            idEle.toString().c_str(),
                                            sourceObj.toString().c_str()  ) ;
               PD_RC_CHECK( rc, PDERROR, "Update document on search engine "
                            "failed[ %d ], id: %s", rc,
                            idEle.toString().c_str() ) ;
               break ;
            case DMS_EXT_TRUNCATE:
               break ;
            default:
               PD_LOG( PDERROR, "Invalid operation type[ %d ] in source data",
                       oprType ) ;
               rc = SDB_INVALIDARG ;
               goto error ;

         }

      }

      rc = _getMoreSdbRecords( contextID, msg->requestID ) ;
      PD_RC_CHECK( rc, PDERROR, "Send get more request failed[ %d ]", rc ) ;

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
         _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
         goto done ;
      }
      else if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "Error received in respond[ %d ]", rc ) ;
         // Change the status back to begin to start over.
         _status = SEADPT_SESSION_STAT_BEGIN ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, docObjs ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract reply failed[ %d ]", rc ) ;

      for ( vector<BSONObj>::iterator itr = docObjs.end() - 1;
            itr != docObjs.begin() - 1; --itr )
      {
         BOOLEAN found = FALSE ;
         BSONElement ele = itr->getField( SEADPT_LOGICAL_ID ) ;
         SDB_ASSERT( NumberLong == ele.type(), "Element type is wrong" ) ;
         rc = _findRecWithLID( ele.Long(), found ) ;
         PD_RC_CHECK( rc, PDERROR, "Find record with _lid[ %lld ] on search "
                      "engine failed[ %d ]", ele.Long(), rc ) ;
         if ( !found )
         {
            // If not found, the targe document dose not exist on search engine.
            // And that's the record where should we start.
            _status = SEADPT_SESSION_STAT_QUERY_CAP_TBL ;
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
      // If index exists, check the logical version. If not the same,
      // drop the collection and do full indexing.
      // If index exists, and the version is the same, then go on to find the
      // logical id of the last indexed document.

      INT32 rc = SDB_OK ;
      BOOLEAN found = FALSE ;

      _switchStatus( SEADPT_SESSION_STAT_CONSULT ) ;

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
         _switchStatus( SEADPT_SESSION_STAT_QUERY_NORMAL_TBL ) ;
         goto done ;
      }

      rc = _esClt->documentExist( _indexName.c_str(),
                                  _typeName.c_str(),
                                  SEADPT_NORMAL_CL_DONE_ID,
                                  found ) ;
      PD_RC_CHECK( rc, PDERROR, "Check document existense failed[ %d ]", rc ) ;
      if ( !found )
      {
         rc = _esClt->dropIndex( _indexName.c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Drop index[ %s ] on search engine "
                      "failed[ %d ]", _indexName.c_str(), rc ) ;
         _switchStatus( SEADPT_SESSION_STAT_QUERY_NORMAL_TBL ) ;
         goto done ;
      }

      SDB_ASSERT( _status == SEADPT_SESSION_STAT_CONSULT, "Status is wrong" ) ;

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

   done:
      return rc ;
   error:
      goto done ;
   }

   void _seAdptIndexSession::_onSDBEOC()
   {
      if ( SEADPT_SESSION_STAT_CONSULT == _status ||
           SEADPT_SESSION_STAT_QUERY_NORMAL_TBL == _status ||
           SEADPT_SESSION_STAT_GETMORE_NORMAL == _status )
      {
         _switchStatus( SEADPT_SESSION_STAT_QUERY_CAP_TBL ) ;
      }
   }
}

