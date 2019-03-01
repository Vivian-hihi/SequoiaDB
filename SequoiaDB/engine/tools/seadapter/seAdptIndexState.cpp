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

   Descriptive Name = Index session state.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/24/2018  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#include "seAdptIndexState.hpp"
#include "seAdptIndexSession.hpp"
#include "utilESUtil.hpp"

namespace seadapter
{
   // Consult should be done in 5 seconds.
   const UINT16 SEADPT_CONSULT_INTERVAL = 5000 ;

   // Max retry times for indexing operations.
   const UINT16 SEADPT_INDEX_MAX_RETRY = 5 ;

   // Request on data/cata node respond time limit.
   const UINT16 SEADPT_BASIC_OP_TIMEOUT = 3000 ;

   const CHAR* seAdptGetIndexerStateDesp( SEADPT_INDEXER_STATE state )
   {
      switch ( state )
      {
      case CONSULT:
         return "Consult" ;
      case FULL_INDEX:
         return "FullIndex" ;
      case INCREMENT_INDEX:
         return "IncrementalIndex" ;
      default:
         break ;
      }
      return "Unknow" ;
   }

   _seAdptIndexerState::_seAdptIndexerState()
   : _timeout( 0 ),
     _retryTimes( 0 )
   {
   }

   INT32 _seAdptIndexerState::updateProgress( seAdptIndexSession *session,
                                              BOOLEAN initial  )
   {
      INT32 rc = SDB_OK ;
      const seIndexMeta *meta = session->indexMeta() ;
      seAdptSEAssist *seAssist = session->seAssist() ;

      try
      {
         BSONObj progressObj =
               BSON( SEADPT_FIELD_NAME_CLUID << (INT64)meta->getCLUID() <<
                     SEADPT_FIELD_NAME_CLLID << meta->getCLLID() <<
                     SEADPT_FIELD_NAME_IDXLID << meta->getIdxLID() <<
                     SEADPT_FIELD_NAME_LID <<
                        ( initial ? -1 : session->getExpectLID() ) ) ;
         rc =
            seAssist->indexDocument( meta->getESIdxName().c_str(),
                                    meta->getESTypeName().c_str(),
                                    SEADPT_COMMIT_ID,
                                    progressObj.toString(false, true).c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Update progress in search engine "
                                   "failed[%d]. Progress information: %s",
                      rc, progressObj.toString(false, true).c_str() ) ;
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

   INT32 _seAdptIndexerState::_getProgressInfo( seAdptIndexSession *session,
                                                BSONObj &progressInfo )
   {
      const seIndexMeta *meta = session->indexMeta() ;
      seAdptSEAssist *seAssist = session->seAssist() ;

      // Try to get the SDBCOMMIT record.
      return seAssist->getDocument( meta->getESIdxName().c_str(),
                                    meta->getESTypeName().c_str(),
                                    SEADPT_COMMIT_ID, progressInfo ) ;
   }

   INT32 _seAdptIndexerState::_validateProgress( seAdptIndexSession *session,
                                                 const BSONObj &progressInfo,
                                                 BOOLEAN &valid )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN validate = FALSE ;
      const seIndexMeta *meta = session->indexMeta() ;
      try
      {
         validate =
               ( ( (UINT32)progressInfo.getIntField( SEADPT_FIELD_NAME_CLUID )
                   == meta->getCLUID() ) &&
                 ( (UINT32)progressInfo.getIntField( SEADPT_FIELD_NAME_CLLID )
                   == meta->getCLLID() ) &&
                 ( (UINT32)progressInfo.getIntField( SEADPT_FIELD_NAME_IDXLID )
                   == meta->getIdxLID() ) ) ;

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

   INT32 _seAdptIndexerState::_cleanSearchEngine( seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN idxExist = FALSE ;
      seAdptSEAssist *es= session->seAssist() ;
      const CHAR *idxName = session->indexMeta()->getESIdxName().c_str() ;

      rc = es->indexExist( idxName, idxExist ) ;
      PD_RC_CHECK( rc, PDERROR, "Check index[%s] existence failed[%d]", rc ) ;
      if ( idxExist )
      {
         rc = es->dropIndex( idxName ) ;
         PD_RC_CHECK( rc, PDERROR, "Drop index[%s] on search engine failed[%d]",
                      idxName, rc) ;
         PD_LOG( PDEVENT, "Drop index[%s] on search engine successfully",
                 idxName ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   _seAdptConsultState::_seAdptConsultState()
   : _checkLID( FALSE ),
     _expectLID( SEADPT_INVALID_LID )
   {
   }

   _seAdptConsultState::~_seAdptConsultState()
   {
   }

   INT32 _seAdptConsultState::processQueryRes( seAdptIndexSession *session,
                                               NET_HANDLE handle,
                                               MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> docObjs ;
      MsgOpReply *reply = (MsgOpReply *)msg ;
      INT32 result = reply->flags ;
      INT64 minLID = SEADPT_INVALID_LID ;
      const CHAR *esIdxName = session->indexMeta()->getESIdxName().c_str() ;

      if ( NET_INVALID_HANDLE != handle &&
           !session->dbAssist()->dataNetHandleValid() )
      {
         session->dbAssist()->setDataNetHandle( handle ) ;
      }

      if ( msg->requestID != session->currentRequestID() )
      {
         goto done ;
      }

      rc = msgExtractReply( (CHAR *)msg, &result, &contextID, &startFrom,
                            &numReturned, docObjs ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Extract query reply message failed[%d]", rc ) ;
         goto error ;
      }

      rc = result ;
      // If capped collection or collection space is deleted, remove the index.
      if ( SDB_DMS_CS_NOTEXIST == rc || SDB_DMS_NOTEXIST == rc
           || SDB_DMS_CS_DELETING == rc )
      {
         PD_LOG( PDEVENT, "Index on data node is dropped. Clean index[%s] on "
                          "search engine", esIdxName ) ;
         {
            INT32 rcTmp = _cleanSearchEngine( session ) ;
            if ( rcTmp )
            {
               PD_LOG( PDERROR, "Clean index[%s] failed", esIdxName ) ;
            }
         }
         goto error ;
      }
      else if ( rc )
      {
         PD_LOG( PDERROR, "Query request failed[%d]", rc ) ;
         goto error ;
      }

      if ( numReturned > 0 )
      {
         SDB_ASSERT( 1 == numReturned, "Number returned should be 1" ) ;
         minLID = docObjs[0].getField( SEADPT_FIELD_NAME_ID ).Number() ;
      }

      if ( _progressMatch( minLID ) )
      {
         session->setExpectLID( _expectLID ) ;
         session->triggerStateTransition( INCREMENT_INDEX ) ;
      }
      else
      {
         rc = _cleanSearchEngine( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Clean index[%s] on search engine "
                                   "failed[%d]", esIdxName, rc ) ;
         session->triggerStateTransition( FULL_INDEX ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptConsultState::processGetMoreRes( seAdptIndexSession *session,
                                                 NET_HANDLE handle,
                                                 MsgHeader *msg )
   {
      return SDB_OK ;
   }

   SEADPT_INDEXER_STATE _seAdptConsultState::type() const
   {
      return CONSULT ;
   }

   INT32 _seAdptConsultState::onTimer( seAdptIndexSession *session,
                                       UINT32 interval )
   {
      INT32 rc = SDB_OK ;

      _timeout += interval ;
      if ( _timeout < SEADPT_CONSULT_INTERVAL )
      {
         goto done ;
      }

      ++_retryTimes ;
      if ( _retryTimes > SEADPT_INDEX_MAX_RETRY )
      {
         rc = SDB_TIMEOUT ;
         PD_LOG( PDERROR, "Index consult timeout" ) ;
         goto error ;
      }

      _timeout = 0 ;

      rc = _consult( session ) ;
      PD_RC_CHECK( rc, PDERROR, "Indexing consult failed[%d]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptConsultState::_consult( seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN exist = FALSE ;
      const CHAR *esIdxName = session->indexMeta()->getESIdxName().c_str() ;

      // 1. Check if the index exists on search engine.
      rc = session->seAssist()->indexExist( esIdxName,  exist ) ;
      PD_RC_CHECK( rc, PDERROR, "Check index[%s] existance on search engine "
                                "failed[%d]", esIdxName, rc ) ;

      if ( !exist )
      {
         PD_LOG( PDEVENT, "Index[%s] not found on search engine. "
                          "Start full indexing", esIdxName ) ;
         session->triggerStateTransition( FULL_INDEX ) ;
         goto done ;
      }
      else
      {
         try
         {
            // 2. If index exists, get and validate the meta data.
            BSONObj progressInfo ;
            BOOLEAN valid = FALSE ;
            rc = _getProgressInfo( session, progressInfo ) ;
            if ( SDB_INVALIDARG == rc )
            {
               PD_LOG( PDEVENT, "Commit mark for index[%s] dose not exist. "
                                "The index on search engine will be re-created",
                       esIdxName ) ;
               rc = _cleanSearchEngine( session ) ;
               PD_RC_CHECK( rc, PDERROR, "Clean index[%s] on search engine "
                                         "failed[%d]", esIdxName, rc ) ;
               session->triggerStateTransition( FULL_INDEX ) ;
               goto done ;
            }
            else if ( rc )
            {
               PD_LOG( PDERROR, "Check commit mark for index[%s] on search "
                                "engine failed[%d]", esIdxName, rc ) ;
               goto error ;
            }

            rc = _validateProgress( session, progressInfo, valid ) ;
            PD_RC_CHECK( rc, PDERROR, "Validate progress information of "
                                      "index[%s] failed[%d]", esIdxName, rc ) ;
            if ( !valid )
            {
               PD_LOG( PDEVENT, "Commit mark for index[%s] is not as expected. "
                                "The index on search engine will be re-created",
                       esIdxName ) ;
               rc = _cleanSearchEngine( session ) ;
               PD_RC_CHECK( rc, PDERROR, "Clean index[%s] on search engine "
                                         "failed[%d]", esIdxName, rc ) ;
               session->triggerStateTransition( FULL_INDEX ) ;
               goto done ;
            }

            // 3. Get _lid in commit mark, and ready to compare with db.
            _expectLID =
                  progressInfo.getField(SEADPT_FIELD_NAME_LID).numberLong() ;

            rc = _queryMinIDOnDB( session ) ;
            PD_RC_CHECK( rc, PDERROR, "Query min logical ID on data node "
                                      "failed[%d]", rc ) ;
         }
         catch ( std::exception &e )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptConsultState::_queryMinIDOnDB( _seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      CHAR *msg = NULL ;
      INT32 bufSize = 0 ;
      BSONObj selector ;
      seAdptDBAssist* dbAssist = session->dbAssist() ;

      try
      {
         // In the capped collection, the field name of logical id is '_id', and
         // the original '_id' is named '_rid'.
         selector = BSON( SEADPT_FIELD_NAME_ID << "" ) ;
         rc = msgBuildQueryMsg( &msg, &bufSize,
                                session->indexMeta()->getCappedCLName().c_str(),
                                FLG_QUERY_WITH_RETURNDATA,
                                session->nextRequestID(), 0, 1, NULL,
                                &selector, NULL, NULL, session->eduCB() ) ;
         PD_RC_CHECK( rc, PDERROR, "Build query message failed[%d]", rc ) ;
         ((MsgHeader *)msg)->TID = session->tid() ;
         rc = dbAssist->sendToDataNode( (const MsgHeader *)msg ) ;
         PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[%d]",
                      rc ) ;
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
         msgReleaseBuffer( msg, session->eduCB() ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _seAdptConsultState::_progressMatch( INT64 minLID )
   {
      return minLID <= _expectLID ;
   }

   _seAdptFullIndexState::_seAdptFullIndexState()
   : _step( CLEAN_ES ),
     _clVersion( -1 )
   {
   }

   _seAdptFullIndexState::~_seAdptFullIndexState()
   {
   }

   SEADPT_INDEXER_STATE _seAdptFullIndexState::type() const
   {
      return FULL_INDEX ;
   }

   INT32 _seAdptFullIndexState::processQueryRes( seAdptIndexSession *session,
                                                 NET_HANDLE handle,
                                                 MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 resultFlag = 0 ;
      INT64 contextID = 0 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> resultSet ;

      // If it's not the reply of the current request, just ignore.
      if ( msg->requestID != session->currentRequestID() )
      {
         goto done ;
      }

      rc = msgExtractReply( (CHAR *)msg, &resultFlag, &contextID, &startFrom,
                            &numReturned, resultSet ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract query reply message failed[%d]", rc ) ;

      if ( SDB_DMS_NOTEXIST == resultFlag || SDB_DMS_CS_NOTEXIST == resultFlag )
      {
         rc = resultFlag ;
         (void)_cleanSearchEngine( session ) ;
         goto error ;
      }

      switch ( _step )
      {
      case CLEAN_DB_P1:
      {
         if ( resultFlag )
         {
            rc = resultFlag ;
            PD_LOG( PDERROR, "Query clean target on db failed[%d]", rc ) ;
            goto error ;
         }

         SDB_ASSERT( numReturned <=1, "Return number is not as expected") ;

         if ( 0 == numReturned )
         {
            // No Records in capped collection. No cleanup required.
            _step = QUERY_CL_VERSION ;
            rc = _queryCLVersion( session ) ;
            PD_RC_CHECK( rc, PDERROR, "Query version of collection failed[%d]",
                         rc ) ;
            goto done ;
         }

         rc = _doCleanDB( session, resultSet.front() ) ;
         PD_RC_CHECK( rc, PDERROR, "Clean data on data node failed[%d]", rc ) ;
         _step = CLEAN_DB_P2 ;
         break ;
      }
      case CLEAN_DB_P2:
      {
         // Make sure the pop succeeded.
         if ( resultFlag )
         {
            rc = resultFlag ;
            PD_LOG( PDERROR, "Pop capped collection data failed[%d]", rc ) ;
            goto error ;
         }
         _step = QUERY_CL_VERSION ;
         rc = _queryCLVersion( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Query version of collection failed[%d]",
                      rc ) ;
         break ;
      }
      case QUERY_CL_VERSION:
      {
         BSONObj record ;

         if ( NET_INVALID_HANDLE != handle &&
              !session->dbAssist()->cataNetHandleValid() )
         {
            session->dbAssist()->setCataNetHandle( handle ) ;
         }

         if ( 1 != numReturned )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Expect result number is 1. Actual: %d",
                    numReturned ) ;
            goto error ;
         }

         record = resultSet.front() ;
         rc = _handleCLVersionRes( session, resultFlag, record ) ;
         PD_RC_CHECK( rc, PDERROR, "Process query respond message failed[%d]",
                      rc ) ;
         break ;
      }
      case QUERY_DATA:
      {
         rc = _handleQueryDataRes( session, resultFlag, contextID ) ;
         PD_RC_CHECK( rc, PDERROR, "Process query respond message failed[%d]",
                      rc ) ;
         break ;
      }
      default:
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptFullIndexState::processGetMoreRes( seAdptIndexSession *session,
                                                   NET_HANDLE handle,
                                                   MsgHeader *msg )
   {
      INT32 rc = SDB_OK;
      MsgOpReply *reply = ( MsgOpReply * )msg;
      INT32 flag = 0;
      INT64 contextID = 0;
      INT32 startFrom = 0;
      INT32 numReturned = 0;
      vector<BSONObj> resultSet;
      seAdptSEAssist* seAssist = session->seAssist() ;
      const seIndexMeta* meta = session->indexMeta() ;

      if ( msg->requestID != session->currentRequestID() )
      {
         goto done;
      }

      rc = msgExtractReply( ( CHAR * )msg, &flag, &contextID, &startFrom,
                            &numReturned, resultSet );
      PD_RC_CHECK( rc, PDERROR, "Extract query reply message failed[%d]", rc );

      try
      {
         if ( SDB_DMS_EOC == flag )
         {
            rc = updateProgress( session, TRUE ) ;
            PD_RC_CHECK( rc, PDERROR, "Update indexing progress failed[%d]",
                         rc ) ;
            session->triggerStateTransition( INCREMENT_INDEX ) ;
            goto done ;
         }
         else if ( SDB_OK != reply->flags )
         {
            rc = reply->flags;
            PD_LOG( PDERROR, "Get more failed[%d]", rc );
            goto error;
         }

         rc = seAssist->bulkPrepare( meta->getESIdxName().c_str(),
                                     meta->getESTypeName().c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Prepare of bulk operation failed[%d]",
                      rc ) ;

         for ( vector<BSONObj>::const_iterator itr = resultSet.begin();
               itr != resultSet.end(); ++itr )
         {
            BSONObj finalRec ;
            string finalID ;
            rc = _parseRecord( *itr, finalID, finalRec ) ;
            if ( SDB_INVALIDARG == rc || finalRec.isEmpty() )
            {
               continue ;
            }
            else if ( rc )
            {
               PD_LOG( PDERROR, "Format record[ %s ] failed[%d]",
                       itr->toString( false, true ).c_str(), rc ) ;
               goto error ;
            }

            if ( finalID.size() > SEADPT_MAX_ID_SZ )
            {
               PD_LOG( PDDEBUG, "Ignore document as actual id length[%d] "
                                "exceeds limit[%d]. id value: %s",
                       finalID.size(), SEADPT_MAX_ID_SZ, finalID.c_str() ) ;
               continue ;
            }

            {
               utilESActionIndex item( meta->getESIdxName().c_str(),
                                       meta->getESTypeName().c_str() ) ;
               rc = item.setID( finalID.c_str() ) ;
               PD_RC_CHECK( rc, PDERROR, "Set _id for action failed[%d]",
                            rc ) ;
               item.setSourceData( finalRec ) ;

               rc = seAssist->bulkProcess( item ) ;
               PD_RC_CHECK( rc, PDERROR, "Bulk processing item failed[%d]",
                            rc ) ;
            }
         }

         rc = seAssist->bulkFinish() ;
         PD_RC_CHECK( rc, PDERROR, "Finish operation of bulk failed[%d]",
                      rc ) ;
         rc = _getMore( session, contextID ) ;
         PD_RC_CHECK( rc, PDERROR, "Send getmore request to data node "
                                   "failed[%d]", rc ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() );
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 _seAdptFullIndexState::onTimer( seAdptIndexSession *session,
                                         UINT32 interval )
   {
      INT32 rc = SDB_OK ;

      _timeout += interval ;
      if ( _timeout < SEADPT_BASIC_OP_TIMEOUT )
      {
         goto done ;
      }

      ++_retryTimes ;
      if ( _retryTimes > SEADPT_INDEX_MAX_RETRY )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Full index failed" ) ;
         goto error ;
      }

      _timeout = 0 ;

      switch ( _step )
      {
      case CLEAN_ES:
         rc = _cleanSearchEngine( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Clean data on ES failed[%d]", rc ) ;
         _step = CRT_ES_IDX ;
         // Intentially fall through.
      case CRT_ES_IDX:
         rc = _crtESIdx( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Create index on ES failed[%d]", rc ) ;
         // Intentially fall through.
      case CLEAN_DB_P1:
         rc = _prepareCleanDB( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Clean data on data node failed[%d]", rc ) ;
         break ;
      case QUERY_CL_VERSION:
         rc = _queryCLVersion( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Query version of collection failed[%d]",
                      rc ) ;
         break ;
      case QUERY_DATA:
         rc = _queryData( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Query data failed[%d]", rc ) ;
         break ;
      default:
         break ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptFullIndexState::_crtESIdx( seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      seAdptSEAssist *se = session->seAssist() ;
      const seIndexMeta *indexMeta = session->indexMeta() ;
      const CHAR *idxName = indexMeta->getESIdxName().c_str() ;
      const CHAR *typeName = indexMeta->getESTypeName().c_str() ;

      try
      {
         BSONObj mappingObj ;
         utilESMapping mapping( idxName, typeName ) ;

         // Generate the Elasticsearch index mapping. Only string fields.
         BSONObjIterator itr( indexMeta->getIdxDef() ) ;
         while ( itr.more() )
         {
            BSONElement ele = itr.next() ;
            mapping.addProperty( ele.fieldName(), ES_TEXT ) ;
         }

         rc = mapping.toObj( mappingObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Build index mapping object failed[%d]",
                      rc ) ;

         rc = se->createIndex( idxName,
                               mappingObj.toString( FALSE, TRUE).c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Create index[%s] with mapping[%s] on "
                                   "search engine failed[%d]",
                      idxName, mappingObj.toString( FALSE, TRUE).c_str(), rc ) ;
         PD_LOG( PDEVENT, "Create index[%s] with mapping[%s] on "
                          "search engine successfully",
                 idxName, mappingObj.toString( FALSE, TRUE).c_str() ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         goto error ;
      }

      _step = CLEAN_DB_P1 ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptFullIndexState::_prepareCleanDB( seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      BSONObj selector ;
      seAdptDBAssist *dbAssist = session->dbAssist() ;
      const CHAR *cappedCLName =
            session->indexMeta()->getCappedCLName().c_str() ;

      // Get the _id of the first record in capped collection.
      try
      {
         selector = BSON( "_id" << "" );
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         goto error ;
      }

      rc = dbAssist->queryOnDataNode( cappedCLName, session->nextRequestID(),
                                      session->tid(), FLG_QUERY_WITH_RETURNDATA,
                                      0, 1, NULL, &selector,
                                      NULL, NULL, session->eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Query _id of the first record in "
                                "collection[%s] failed[%d]",
                   cappedCLName, rc ) ;

      PD_LOG( PDDEBUG, "Ready to clean capped collection[%s]. Query for the "
                       "first record...", cappedCLName ) ;
      _timeout = 0 ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptFullIndexState::_doCleanDB( seAdptIndexSession *session,
                                            const BSONObj &targetObj )
   {
      INT32 rc = SDB_OK ;
      seAdptDBAssist *dbAssist = session->dbAssist() ;
      const CHAR *cappedCLName =
            session->indexMeta()->getCappedCLName().c_str() ;

      try
      {
         BSONObjBuilder builder ;
         BSONObj option ;
         BSONElement lidEle= targetObj.getField( "_id" ) ;
         builder.append( FIELD_NAME_COLLECTION, cappedCLName ) ;
         builder.appendAs( lidEle, FIELD_NAME_LOGICAL_ID ) ;
         builder.appendIntOrLL( FIELD_NAME_DIRECTION, -1 ) ;
         option = builder.done() ;

         rc = dbAssist->doCmdOnDataNode( CMD_ADMIN_PREFIX CMD_NAME_POP,
                                         option, session->nextRequestID(),
                                         session->tid(), session->eduCB() ) ;
         PD_RC_CHECK( rc, PDERROR, "Send pop command to data node failed[%d]",
                      rc ) ;
         PD_LOG( PDEVENT, "Begin to clean capped collection[%s] by pop "
                          "command. Pop options: %s",
                 cappedCLName, option.toString(false, true).c_str() ) ;
         _timeout = 0 ;
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

   INT32 _seAdptFullIndexState::_queryCLVersion( seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      BSONObj selector ;
      seAdptDBAssist *dbAssist = session->dbAssist() ;
      const CHAR *clName = session->indexMeta()->getOrigCLName().c_str() ;

      try
      {
         selector = BSON( FIELD_NAME_VERSION << "" ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Exception occurred when creating query: %s",
                 e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // Send query message to catalogue.
      rc = dbAssist->queryCLCataInfo( clName, session->nextRequestID(),
                                      session->tid(), &selector,
                                      session->eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Query catalogue information of collection[%s] "
                                " failed[%d]", clName, rc ) ;
      _timeout = 0 ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptFullIndexState::_queryData( seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      MsgOpQuery *msg = NULL ;
      INT32 bufSize = 0 ;
      BSONObj query ;
      BSONObj selector ;

      rc = _genQueryOptions( session, query, selector ) ;
      PD_RC_CHECK( rc, PDERROR, "Generate query and selector for the original "
                                "colleciton failed[%d]", rc ) ;
      PD_LOG( PDDEBUG, "Full indexing query condition: %s",
              query.toString(false, true).c_str() ) ;
      rc = msgBuildQueryMsg( (CHAR **)&msg, &bufSize,
                             session->indexMeta()->getOrigCLName().c_str(),
                             0, session->nextRequestID(), 0, -1, &query,
                             &selector, NULL, NULL, session->eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[%d]", rc ) ;
      msg->version = _clVersion ;
      msg->header.TID = session->tid() ;
      rc = session->dbAssist()->sendToDataNode( (MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[%d]",
                   rc ) ;
      _timeout = 0 ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( (CHAR *)msg, session->eduCB() ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptFullIndexState::_genQueryOptions( seAdptIndexSession *session,
                                                  BSONObj &query,
                                                  BSONObj &selector )
   {
      INT32 rc = SDB_OK ;
      const seIndexMeta* meta = session->indexMeta() ;

      SDB_ASSERT( meta, "index meta is NULL" ) ;

      try
      {
         BSONObjBuilder queryBuilder ;
         BSONObjBuilder selectorBuilder ;
         BSONObjIterator idxItr( session->indexMeta()->getIdxDef() ) ;
         BSONArrayBuilder
            queryObj( queryBuilder.subarrayStart( SEADPT_OPERATOR_STR_OR ) ) ;

         BSONObj existTmp = BSON( SEADPT_OPERATOR_STR_EXIST << 1 ) ;
         BSONObj includeObj = BSON( SEADPT_OPERATOR_STR_INCLUDE << 1 ) ;

         selectorBuilder.appendObject( SEADPT_FIELD_NAME_ID,
                                       includeObj.objdata(),
                                       includeObj.objsize() ) ;

         while ( idxItr.more() )
         {
            BSONElement ele = idxItr.next() ;
            const CHAR *fieldName = ele.fieldName() ;
            SDB_ASSERT( 0 != ossStrcmp( fieldName, SEADPT_FIELD_NAME_ID ),
                        "Text index should not include _id" ) ;
            selectorBuilder.appendObject( fieldName, includeObj.objdata(),
                                          includeObj.objsize() ) ;
            BSONObjBuilder existObj( queryObj.subobjStart() ) ;
            existObj.appendObject( fieldName, existTmp.objdata(),
                                   existTmp.objsize() ) ;
            existObj.done() ;
         }
         queryObj.done() ;
         query = queryBuilder.obj() ;
         selector = selectorBuilder.obj() ;

         PD_LOG( PDDEBUG, "Original collection query, condition: %s, "
                          "selector: %s",
                 query.toString().c_str(),
                 selector.toString().c_str() ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32
   _seAdptFullIndexState::_handleCLVersionRes( seAdptIndexSession *session,
                                               INT32 result,
                                               const BSONObj &record )
   {
      INT32 rc = SDB_OK ;
      const CHAR *origCLName = session->indexMeta()->getOrigCLName().c_str() ;

      rc = result ;
      if ( rc )
      {
         if ( SDB_DMS_NOTEXIST == rc || SDB_DMS_CS_NOTEXIST == rc )
         {
            INT32 rcTmp = _cleanSearchEngine( session ) ;
            if ( rcTmp )
            {
               PD_LOG( PDERROR, "Clean index[%s] on search engine failed[%d]",
                       session->indexMeta()->getESIdxName().c_str(), rcTmp ) ;
            }
            goto error ;
         }
         else if ( SDB_CLS_NOT_PRIMARY == rc || SDB_NET_CANNOT_CONNECT == rc )
         {
            rc = session->updateCataInfo( OSS_ONE_SEC ) ;
            PD_RC_CHECK( rc, PDERROR, "Update catalogue information failed[%d]",
                         rc ) ;
            // Stay in state of QUERY_CL_VERSION. Wait for next query of the
            // collection version.
            goto done ;
         }

         PD_LOG( PDERROR, "Query version of collection[%s] on catalogue "
                          "failed[%d]", origCLName, rc ) ;
         goto error ;
      }

      try
      {
         _clVersion = record.getIntField( FIELD_NAME_VERSION ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         goto error ;
      }
      PD_LOG( PDDEBUG, "Update local version of original collection to %d",
              _clVersion ) ;

      _step = QUERY_DATA ;
      rc = _queryData( session ) ;
      PD_RC_CHECK( rc, PDERROR, "Query data failed[%d]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32
   _seAdptFullIndexState::_handleQueryDataRes( seAdptIndexSession *session,
                                               INT32 result, INT64 contextID )
   {
      INT32 rc = SDB_OK ;

      rc = result ;
      PD_RC_CHECK( rc, PDERROR, "Query data failed[%d]", rc ) ;

      rc = _getMore( session, contextID ) ;
      PD_RC_CHECK( rc, PDERROR, "Send getmore request to data node "
                                "failed[%d]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptFullIndexState::_getMore( seAdptIndexSession *session,
                                          INT64 contextID )
   {
      INT32 rc = SDB_OK ;
      CHAR *msg = NULL ;
      INT32 bufSize = 0 ;

      rc = msgBuildGetMoreMsg( &msg, &bufSize, -1, contextID,
                               session->nextRequestID(), session->eduCB() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Build get more request failed[%d]", rc ) ;
         goto error ;
      }
      ((MsgHeader *)msg)->TID = session->tid() ;
      rc = session->dbAssist()->sendToDataNode( (const MsgHeader *)msg ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Send get more request to data node failed[%d]",
                 rc ) ;
         goto error ;
      }
      _timeout = 0 ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( msg, session->eduCB() ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptFullIndexState::_parseRecord( const BSONObj &origRecord,
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
            if ( 0 == ossStrcmp( ele.fieldName(), SEADPT_FIELD_NAME_ID ) )
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
               PD_LOG( PDWARNING, "Encode id[ %s ] failed[%d]",
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

   _seAdptIncIndexState::_seAdptIncIndexState()
   : _step( QUERY_DATA ),
     _hasNewData( FALSE ),
     _firstBatchData( TRUE ),
     _expectRecHash( 0 ),
     _queryCtxID( -1 )
   {

   }

   _seAdptIncIndexState::~_seAdptIncIndexState()
   {

   }

   INT32 _seAdptIncIndexState::processQueryRes( seAdptIndexSession *session,
                                                NET_HANDLE handle,
                                                MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 resultFlag = 0 ;
      INT64 contextID = 0 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> resultSet ;

      // If it's not the reply of the current request, just ignore.
      if ( msg->requestID != session->currentRequestID() )
      {
         goto done ;
      }

      rc = msgExtractReply( (CHAR *)msg, &resultFlag, &contextID, &startFrom,
                            &numReturned, resultSet ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract query reply message failed[%d]", rc ) ;

      if ( SDB_DMS_NOTEXIST == resultFlag || SDB_DMS_CS_NOTEXIST == resultFlag )
      {
         rc = resultFlag ;
         (void)_cleanSearchEngine( session ) ;
         goto error ;
      }

      switch ( _step )
      {
      case QUERY_DATA:
      {
         if ( resultFlag )
         {
            rc = resultFlag ;
            PD_LOG( PDERROR, "Query clean target on db failed[%d]", rc ) ;
            goto error ;
         }
         rc = _getMore( session, contextID ) ;
         PD_RC_CHECK( rc, PDERROR, "Getmore request failed[%d]", rc ) ;
         _queryCtxID = contextID ;
         _firstBatchData = TRUE ;
         break ;
      }
      case CLEAN_SRC:
         if ( resultFlag )
         {
            rc = resultFlag ;
            PD_LOG( PDERROR, "Clean source capped collection failed[%d]", rc ) ;
            goto error ;
         }

         // After clean the last batch of data, continue to fetch next batch of
         // data with the original query context.
         rc = _getMore( session, _queryCtxID ) ;
         PD_RC_CHECK( rc, PDERROR, "Getmore request failed[%d]", rc ) ;
      default:
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIncIndexState::processGetMoreRes( seAdptIndexSession *session,
                                                  NET_HANDLE handle,
                                                  MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0;
      INT64 contextID = 0;
      INT32 startFrom = 0;
      INT32 numReturned = 0;
      vector<BSONObj> resultSet;
      const seIndexMeta* meta = session->indexMeta() ;

      if ( msg->requestID != session->currentRequestID() )
      {
         goto done;
      }

      rc = msgExtractReply( ( CHAR * )msg, &flag, &contextID, &startFrom,
                            &numReturned, resultSet );
      PD_RC_CHECK( rc, PDERROR, "Extract query reply message failed[%d]", rc );


      if ( SDB_DMS_EOC == flag )
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
         if ( SEADPT_INVALID_LID == session->getExpectLID() && !_hasNewData )
         {
            if ( FALSE == session->validateMeta() )
            {
               rc = SDB_SYS ;
               PD_LOG( PDERROR, "Can not find expected record in capped "
                                "collection[%s]",
                       meta->getCappedCLName().c_str() ) ;
               goto error ;
            }
         }

         if ( _hasNewData )
         {
            _hasNewData = FALSE ;
            PD_LOG( PDDEBUG, "All records in capped collection[%s] have been "
                             "processed. Ready to start a new query on it",
                    meta->getCappedCLName().c_str() ) ;
         }

         // Wait for next round to be triggered by the timer.
         _timeout = 0 ;
         _step = QUERY_DATA ;
         goto done ;
      }
      else if ( flag )
      {
         rc = flag ;
         PD_LOG( PDERROR, "Get data failed[%d]", rc ) ;
         goto error ;
      }

      try
      {
         rc = _processDocuments( session, resultSet ) ;
         PD_RC_CHECK( rc, PDERROR, "Index documents on search engine "
                                   "failed[%d]", rc ) ;

         if ( _hasNewData &&
              session->getLastExpectLID() != SEADPT_INVALID_LID &&
              ( session->getLastExpectLID() < session->getExpectLID() ) )
         {
            // pop data
            rc = _cleanData( session ) ;
            PD_RC_CHECK( rc, PDERROR, "Send clean data request failed[%d]",
                         rc ) ;
            goto done ;
         }
         else
         {
            rc = _getMore( session, contextID ) ;
            PD_RC_CHECK( rc, PDERROR, "Send getmore request to data node "
                                      "failed[%d]", rc ) ;
         }
         if ( _firstBatchData )
         {
            _firstBatchData = FALSE ;
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() );
         goto error;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIncIndexState::onTimer( seAdptIndexSession *session,
                                        UINT32 interval )
   {
      INT32 rc = SDB_OK ;

      _timeout += interval ;

      if ( _timeout < SEADPT_BASIC_OP_TIMEOUT )
      {
         goto done ;
      }

      _timeout = 0 ;

      if ( QUERY_DATA == _step )
      {
         rc = _queryData( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Query data of capped collection failed[%d]",
                      rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   SEADPT_INDEXER_STATE _seAdptIncIndexState::type() const
   {
      return INCREMENT_INDEX ;
   }

   INT32 _seAdptIncIndexState::_genQueryOptions( seAdptIndexSession *session,
                                                 BSONObj &query,
                                                 BSONObj &selector )
   {
      INT32 rc = SDB_OK ;

      try
      {
         query = BSON( SEADPT_FIELD_NAME_ID <<
                       BSON( "$gte" << session->getExpectLID() ) ) ;
         PD_LOG( PDDEBUG, "Inremental condition: %s",
                 query.toString(false, true).c_str() ) ;
         selector = BSONObj() ;
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

   INT32 _seAdptIncIndexState::_queryData( seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      CHAR *msg = NULL ;
      INT32 bufSize = 0 ;
      BSONObj query ;
      BSONObj selector ;

      rc = _genQueryOptions( session, query, selector ) ;
      PD_RC_CHECK( rc, PDERROR, "Generate query and selector for the capped"
                                "colleciton failed[%d]", rc ) ;
      PD_LOG( PDDEBUG, "Incremental indexing query condition: %s",
              query.toString(false, true).c_str() ) ;
      rc = msgBuildQueryMsg( &msg, &bufSize,
                             session->indexMeta()->getCappedCLName().c_str(),
                             0, session->nextRequestID(), 0, -1, &query,
                             &selector, NULL, NULL, session->eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query message failed[%d]", rc ) ;
      ((MsgHeader *)msg)->TID = session->tid() ;
      rc = session->dbAssist()->sendToDataNode( (MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to data node failed[%d]",
                   rc ) ;
      _timeout = 0 ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( msg, session->eduCB() ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIncIndexState::_getMore( seAdptIndexSession *session,
                                         INT64 contextID )
   {
      INT32 rc = SDB_OK ;
      CHAR *msg = NULL ;
      INT32 bufSize = 0 ;

      rc = msgBuildGetMoreMsg( &msg, &bufSize, -1, contextID,
                               session->nextRequestID(), session->eduCB() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Build getmore request failed[%d]", rc ) ;
         goto error ;
      }
      ((MsgHeader *)msg)->TID = session->tid() ;
      rc = session->dbAssist()->sendToDataNode( (const MsgHeader *)msg ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Send getmore request to data node failed[%d]", rc ) ;
         goto error ;
      }
      _timeout = 0 ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( msg, session->eduCB() ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIncIndexState::_processDocuments( seAdptIndexSession *session,
                                                  const vector <BSONObj> &docs )
   {
      INT32 rc = SDB_OK ;
      UINT32 lastObjHash = 0 ;
      INT64 logicalID = SEADPT_INVALID_LID ;
      const seIndexMeta *meta = session->indexMeta() ;
      seAdptSEAssist* seAssist = session->seAssist() ;

      try
      {
         if ( docs.size() > 0 )
         {
            if ( _firstBatchData && 0 != _expectRecHash )
            {
               rc = _consistencyCheck( session, docs.front() ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "The first record in capped collection[%s]"
                                   " is not as we expected. Clean index[%s] on "
                                   "search engine",
                          meta->getCappedCLName().c_str(),
                          meta->getESIdxName().c_str() ) ;
                  {
                     INT32 rcTmp = _cleanSearchEngine( session ) ;
                     if ( rcTmp )
                     {
                        PD_LOG( PDERROR, "Cleanup on search engine when "
                                         "consistency check error failed[%d]",
                                rcTmp ) ;
                     }
                  }
                  goto error ;
               }
            }

            if ( !_firstBatchData && 1 == docs.size() )
            {
               _hasNewData = FALSE ;
               goto done ;
            }
            {
               // Calculate expect hash value of next round.
               BSONObj lastObj = docs.back() ;
               lastObjHash = ossHash( lastObj.objdata(), lastObj.objsize() ) ;
               if ( !_hasNewData )
               {
                  _hasNewData = TRUE ;
               }
            }
         }

         vector<BSONObj>::const_iterator itr = docs.begin() ;

         // We always get one more record, if the _expectLID is not -1. So the
         // first one should be filtered out.
         if ( SEADPT_INVALID_LID != session->getExpectLID() && _firstBatchData )
         {
            itr++ ;
            if ( itr == docs.end() )
            {
               _hasNewData = FALSE ;
               goto done ;
            }
         }

         rc = seAssist->bulkPrepare( meta->getESIdxName().c_str(),
                                     meta->getESTypeName().c_str() ) ;
         PD_RC_CHECK( rc, PDERROR, "Prepare of bulk operation failed[%d]",
                      rc ) ;
         for ( ; itr != docs.end(); ++itr )
         {
            rc = _processDocument( session, *itr, logicalID ) ;
            PD_RC_CHECK( rc, PDERROR, "Process document failed[%d]. "
                                      "Document: %s",
                         rc, itr->toString(false, true).c_str() ) ;
         }

         rc = seAssist->bulkFinish() ;
         PD_RC_CHECK( rc, PDERROR, "Finish operation of bulk "
                                   "failed[%d]", rc ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         goto error ;
      }

      if ( SEADPT_INVALID_LID != logicalID )
      {
         session->setExpectLID( logicalID ) ;
         rc = updateProgress( session ) ;
         PD_RC_CHECK( rc, PDERROR, "Update progress failed[%d]", rc ) ;
      }

      _expectRecHash = lastObjHash ;
      _firstBatchData = FALSE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIncIndexState::_processDocument( _seAdptIndexSession *session,
                                                 const BSONObj &document,
                                                 INT64 &logicalID )
   {
      INT32 rc = SDB_OK ;

      BSONObj newNameObj ;
      string idStr ;
      BSONObj sourceObj ;
      string finalID ;
      string finalIdNew ;
      _rtnExtOprType oprType = RTN_EXT_INVALID ;
      seAdptSEAssist *seAssist = session->seAssist() ;

      rc = _parseRecord( document, oprType, finalID, logicalID,
                         sourceObj, &finalIdNew ) ;
      if ( rc )
      {
         if ( SDB_INVALIDARG == rc )
         {
            // Unsupport type, just ignore.
            rc = SDB_OK ;
            goto done ;
         }
         PD_LOG( PDERROR, "Get id string and source object failed[%d]", rc ) ;
         goto error ;
      }

      if ( sourceObj.isEmpty() )
      {
         if ( RTN_EXT_INSERT == oprType )
         {
            // Nothing should be inserted.
            goto done ;
         }
         else if ( RTN_EXT_UPDATE == oprType )
         {
            // If no index field of type string, the document should be
            // removed. So we directly change it into delete.
            oprType = RTN_EXT_DELETE ;
         }
      }

      if ( finalID.size() > SEADPT_MAX_ID_SZ )
      {
         PD_LOG( PDDEBUG, "Ignore document as actual id length[%d] "
                          "exceeds limit[%d]. id value: %s",
                 finalID.size(), SEADPT_MAX_ID_SZ, finalID.c_str() ) ;
         goto done ;
      }

      switch ( oprType )
      {
         // In case of update, we are going to update the whole document,
         // not part of it. So indexing should be done instead of updating.
      case RTN_EXT_INSERT:
      case RTN_EXT_UPDATE:
      {
         rc = seAssist->bulkAppendIndex( finalID.length() > 0 ?
                                         finalID.c_str() : NULL, sourceObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Index document failed[%d]", rc ) ;
         break ;
      }
      case RTN_EXT_DELETE:
      {
         rc = seAssist->bulkAppendDel( finalID.length() > 0 ?
                                       finalID.c_str() : NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Delete document failed[%d]", rc ) ;
         break ;
      }
      case RTN_EXT_UPDATE_WITH_ID:
      {
         rc = seAssist->bulkAppendReplace( finalID.length() > 0 ?
                                           finalID.c_str() : NULL,
                                           finalIdNew.length() > 0 ?
                                           finalIdNew.c_str() : NULL,
                                           sourceObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Update record failed[%d]", rc ) ;
         break ;
      }
      default:
         PD_LOG( PDERROR, "Invalid operation type[%d] in source data",
                 oprType ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIncIndexState::_consistencyCheck( seAdptIndexSession *session,
                                                  const BSONObj &doc )
   {
      INT32 rc = SDB_OK ;

      // Check if the first record is the one we expected. If not, start
      // over again. Check only once in one query round.
      if ( SEADPT_INVALID_LID != session->getExpectLID() )
      {
         UINT32 objHash = ossHash( doc.objdata(), doc.objsize() ) ;
         if ( objHash != _expectRecHash )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "The first record in capped collection[ %s ]"
                             " is not as we expected",
                    session->indexMeta()->getCappedCLName().c_str() ) ;
            goto error ;
         }
         _hasNewData = FALSE ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptIncIndexState::_parseRecord( const BSONObj &origObj,
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
         BSONElement lidField = origObj.getField( SEADPT_FIELD_NAME_ID ) ;
         BSONElement ridEle = origObj.getField( SEADPT_FIELD_NAME_RID ) ;
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
            PD_LOG( PDERROR, "Operation type[%d] is not supported in "
                             "source record: %s",
                    type, origObj.toString().c_str() ) ;
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
                  origObj.getField( SEADPT_FIELD_NAME_RID_NEW ) ;
            if ( newIdEle.eoo() )
            {
               rc = SDB_SYS ;
               PD_LOG( PDERROR, "Get new _id from object failed[%d]", rc ) ;
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
               PD_LOG( PDWARNING, "Encode _id[ %s ] failed[%d]",
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

   INT32 _seAdptIncIndexState::_cleanData( _seAdptIndexSession *session )
   {
      INT32 rc = SDB_OK ;
      seAdptDBAssist *dbAssist = session->dbAssist() ;
      const CHAR *cappedCLName =
            session->indexMeta()->getCappedCLName().c_str() ;

      if ( SEADPT_INVALID_LID == session->getLastExpectLID() ||
           0 == _expectRecHash )
      {
         _step = QUERY_DATA ;
         goto done ;
      }

      try
      {
         BSONObjBuilder builder ;
         BSONObj option ;
         builder.append( FIELD_NAME_COLLECTION, cappedCLName ) ;
         builder.append( FIELD_NAME_LOGICAL_ID, session->getLastExpectLID() ) ;
         builder.appendIntOrLL( FIELD_NAME_DIRECTION, 1 ) ;
         option = builder.done() ;

         rc = dbAssist->doCmdOnDataNode( CMD_ADMIN_PREFIX CMD_NAME_POP,
                                         option, session->nextRequestID(),
                                         session->tid(), session->eduCB() ) ;
         PD_RC_CHECK( rc, PDERROR, "Send command to catalogue node failed[%d]",
                      rc ) ;
         PD_LOG( PDEVENT, "Clean capped collection[%s] by pop "
                          "command. Pop options: %s",
                 cappedCLName, option.toString(false, true).c_str() ) ;
         _timeout = 0 ;
         _step = CLEAN_SRC ;
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
}

