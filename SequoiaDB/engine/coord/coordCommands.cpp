/*******************************************************************************

   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = coordCommands.cpp

   Descriptive Name = Coord Commands

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   user command processing on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "coordCommands.hpp"
#include "pmd.hpp"
#include "rtnCB.hpp"
#include "pmdOptions.h"
#include "utilCommon.hpp"
#include "coordQueryOperator.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

using namespace bson ;

namespace engine
{

   /*
      _coordCMDTestCollectionSpace implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDTestCollectionSpace,
                                      CMD_NAME_TEST_COLLECTIONSPACE,
                                      TRUE ) ;
   _coordCMDTestCollectionSpace::_coordCMDTestCollectionSpace()
   {
   }

   _coordCMDTestCollectionSpace::~_coordCMDTestCollectionSpace()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_CMD_TESTCS_EXE, "_coordCMDTestCollectionSpace::execute" )
   INT32 _coordCMDTestCollectionSpace::execute( MsgHeader *pMsg,
                                                pmdEDUCB *cb,
                                                INT64 &contextID,
                                                rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_CMD_TESTCS_EXE ) ;
      SDB_RTNCB *pRtncb = pmdGetKRCB()->getRTNCB() ;
      coordCommandFactory *pFactory = coordGetFactory() ;
      coordOperator *pOperator = NULL ;
      rtnContextBuf buffObj ;

      contextID = -1 ;

      rc = pFactory->create( CMD_NAME_LIST_COLLECTIONSPACES, pOperator ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Create operator by name[%s] failed, rc: %d",
                 CMD_NAME_LIST_COLLECTIONSPACES, rc ) ;
         goto error ;
      }
      rc = pOperator->init( _pResource, cb, getTimeout() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init operator failed[%s], rc: %d",
                 pOperator->getName(), rc ) ;
         goto error ;
      }
      rc = pOperator->execute( pMsg, cb, contextID, buf ) ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Execute operator[%s] failed, rc: %d",
                  pOperator->getName(), rc ) ;
         goto error ;
      }

      /// get more
      rc = rtnGetMore( contextID, -1, buffObj, cb, pRtncb ) ;
      if ( rc )
      {
         contextID = -1 ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_DMS_CS_NOTEXIST ;
         }
         else
         {
            PD_LOG ( PDERROR, "getmore failed, rc: %d", rc ) ;
         }
      }

   done:
      if ( contextID >= 0 )
      {
         pRtncb->contextDelete( contextID, cb ) ;
         contextID = -1 ;
      }
      if ( pOperator )
      {
         pFactory->release( pOperator ) ;
      }
      PD_TRACE_EXITRC ( COORD_CMD_TESTCS_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDTestCollection implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDTestCollection,
                                      CMD_NAME_TEST_COLLECTION,
                                      TRUE ) ;
   _coordCMDTestCollection::_coordCMDTestCollection()
   {
   }

   _coordCMDTestCollection::~_coordCMDTestCollection()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_CMD_TESTCL_EXE, "_coordCMDTestCollection::execute" )
   INT32 _coordCMDTestCollection::execute( MsgHeader *pMsg,
                                           pmdEDUCB *cb,
                                           INT64 &contextID,
                                           rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_CMD_TESTCL_EXE ) ;
      SDB_RTNCB *pRtncb = pmdGetKRCB()->getRTNCB() ;
      coordCommandFactory *pFactory = coordGetFactory() ;
      coordOperator *pOperator = NULL ;
      rtnContextBuf buffObj ;

      contextID                        = -1 ;

      rc = pFactory->create( CMD_NAME_LIST_COLLECTIONS, pOperator ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Create operator by name[%s] failed, rc: %d",
                 CMD_NAME_LIST_COLLECTIONS, rc ) ;
         goto error ;
      }
      rc = pOperator->init( _pResource, cb, getTimeout() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init operator failed[%s], rc: %d",
                 pOperator->getName(), rc ) ;
         goto error ;
      }
      rc = pOperator->execute( pMsg, cb, contextID, buf ) ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Execute operator[%s] failed, rc: %d",
                  pOperator->getName(), rc ) ;
         goto error ;
      }

      rc = rtnGetMore( contextID, -1, buffObj, cb, pRtncb ) ;
      if ( rc )
      {
         contextID = -1 ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_DMS_NOTEXIST ;
         }
         else
         {
            PD_LOG ( PDERROR, "Getmore failed, rc: %d", rc ) ;
         }
      }

   done:
      if ( contextID >= 0 )
      {
         pRtncb->contextDelete( contextID, cb ) ;
         contextID = -1 ;
      }
      if ( pOperator )
      {
         pFactory->release( pOperator ) ;
      }
      PD_TRACE_EXITRC ( COORD_CMD_TESTCL_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCmdWaitTask implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCmdWaitTask,
                                      CMD_NAME_WAITTASK,
                                      TRUE ) ;
   _coordCmdWaitTask::_coordCmdWaitTask()
   {
   }

   _coordCmdWaitTask::~_coordCmdWaitTask()
   {
   }

   INT32 _coordCmdWaitTask::execute( MsgHeader *pMsg,
                                     pmdEDUCB *cb,
                                     INT64 &contextID,
                                     rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      SET_RC ignoreRC ;
      rtnContextCoord *pContext        = NULL ;
      rtnContextBuf buffObj ;
      pmdKRCB *pKRCB                   = pmdGetKRCB() ;
      contextID                        = -1 ;
      pMsg->opCode                     = MSG_CAT_QUERY_TASK_REQ ;
      pMsg->TID                        = cb->getTID() ;

      ignoreRC.insert( SDB_DMS_EOC ) ;
      ignoreRC.insert( SDB_CAT_TASK_NOTFOUND ) ;

      while ( TRUE )
      {
         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         rc = executeOnCataGroup( pMsg, cb, TRUE, &ignoreRC, &pContext, buf ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Query task on catalog failed, rc: %d", rc ) ;
            goto error ;
         }
         rc = pContext->getMore( -1, buffObj, cb ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Get more failed, rc: %d", rc ) ;
            goto error ;
         }

         pKRCB->getRTNCB()->contextDelete( pContext->contextID(), cb ) ;
         pContext = NULL ;
         ossSleep( OSS_ONE_SEC ) ;
      }

   done:
      if ( pContext )
      {
         pKRCB->getRTNCB()->contextDelete( pContext->contextID(),  cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCmdCancelTask implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCmdCancelTask,
                                      CMD_NAME_CANCEL_TASK,
                                      TRUE ) ;
   _coordCmdCancelTask::_coordCmdCancelTask()
   {
   }

   _coordCmdCancelTask::~_coordCmdCancelTask()
   {
   }

   INT32 _coordCmdCancelTask::execute( MsgHeader *pMsg,
                                       pmdEDUCB *cb,
                                       INT64 &contextID,
                                       rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      coordCommandFactory *pFactory    = NULL ;
      coordOperator *pOperator         = NULL ;
      BOOLEAN async                    = FALSE ;

      contextID                        = -1 ;

      CoordGroupList groupLst ;
      INT32 rcTmp = SDB_OK ;

      // extract msg
      CHAR *pQueryBuf = NULL ;
      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL, NULL, NULL, &pQueryBuf,
                            NULL, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to extract query msg, rc: %d", rc ) ;

      try
      {
         BSONObj matcher( pQueryBuf ) ;
         rc = rtnGetBooleanElement( matcher, FIELD_NAME_ASYNC, async ) ;
         if ( SDB_FIELD_NOT_EXIST == rc )
         {
            rc = SDB_OK ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                      FIELD_NAME_ASYNC, rc ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      pMsg->opCode                     = MSG_CAT_SPLIT_CANCEL_REQ ;

      rc = executeOnCataGroup( pMsg, cb, &groupLst, NULL, TRUE,
                               NULL, buf ) ;
      PD_RC_CHECK( rc, PDERROR, "Excute on catalog failed, rc: %d", rc ) ;

      pMsg->opCode                     = MSG_BS_QUERY_REQ ;
      // notify to data node
      rcTmp = executeOnDataGroup( pMsg, cb, groupLst,
                                  TRUE, NULL, NULL, NULL,
                                  buf ) ;
      if ( rcTmp )
      {
         PD_LOG( PDWARNING, "Failed to notify to data node, rc: %d", rcTmp ) ;
      }

      // if sync
      if ( !async )
      {
         pFactory = coordGetFactory() ;
         rc = pFactory->create( CMD_NAME_WAITTASK, pOperator ) ;
         PD_RC_CHECK( rc, PDERROR, "Create operator by name[%s] failed, rc: %d",
                      CMD_NAME_WAITTASK, rc ) ;
         rc = pOperator->init( _pResource, cb, getTimeout() ) ;
         PD_RC_CHECK( rc, PDERROR, "Init operator[%s] failed, rc: %d",
                      pOperator->getName(), rc ) ;
         rc = pOperator->execute( pMsg, cb, contextID, buf ) ;
         if ( rc )
         {
            goto error ;
         }
      }

   done:
      if ( pOperator )
      {
         pFactory->release( pOperator ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDStatisticsBase implement
   */
   _coordCMDStatisticsBase::_coordCMDStatisticsBase()
   {
   }

   _coordCMDStatisticsBase::~_coordCMDStatisticsBase()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_CMDSTATBASE_EXE, "_coordCMDStatisticsBase::execute" )
   INT32 _coordCMDStatisticsBase::execute( MsgHeader *pMsg,
                                           pmdEDUCB *cb,
                                           INT64 &contextID,
                                           rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_CMDSTATBASE_EXE ) ;

      SDB_RTNCB *pRtncb                = pmdGetKRCB()->getRTNCB() ;

      // fill default-reply(execute success)
      contextID                        = -1 ;

      coordQueryOperator queryOpr( isReadOnly() ) ;
      rtnContextCoord *pContext = NULL ;
      coordQueryConf queryConf ;
      coordSendOptions sendOpt ;
      queryConf._openEmptyContext = openEmptyContext() ;

      CHAR *pHint = NULL ;

      // extract request-message
      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL,
                            NULL, NULL, NULL, NULL,
                            NULL, &pHint );
      PD_RC_CHECK ( rc, PDERROR, "Execute failed, failed to parse query "
                    "request, rc: %d", rc ) ;

      try
      {
         BSONObj boHint( pHint ) ;
         //get collection name
         BSONElement ele = boHint.getField( FIELD_NAME_COLLECTION ) ;
         PD_CHECK ( ele.type() == String,
                    SDB_INVALIDARG, error, PDERROR,
                    "Execute failed, failed to get the field(%s)",
                    FIELD_NAME_COLLECTION ) ;
         queryConf._realCLName = ele.str() ;
      }
      catch( std::exception &e )
      {
         PD_RC_CHECK ( rc, PDERROR, "Execute failed, occured unexpected "
                       "error:%s", e.what() ) ;
      }

      rc = queryOpr.queryOrDoOnCL( pMsg, cb, &pContext,
                                   sendOpt, &queryConf, buf ) ;
      PD_RC_CHECK( rc, PDERROR, "Query failed, rc: %d", rc ) ;

      // statistics the result
      rc = generateResult( pContext, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to execute statistics, rc: %d", rc ) ;

      contextID = pContext->contextID() ;
      pContext->reopen() ;

   done:
      PD_TRACE_EXITRC ( COORD_CMDSTATBASE_EXE, rc ) ;
      return rc;
   error:
      if ( pContext )
      {
         pRtncb->contextDelete( pContext->contextID(), cb ) ;
      }
      goto done ;
   }

   /*
      _coordCMDGetIndexes implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDGetIndexes,
                                      CMD_NAME_GET_INDEXES,
                                      TRUE ) ;
   _coordCMDGetIndexes::_coordCMDGetIndexes()
   {
   }

   _coordCMDGetIndexes::~_coordCMDGetIndexes()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_GETINDEX_GENRESULT, "_coordCMDGetIndexes::generateResult" )
   INT32 _coordCMDGetIndexes::generateResult( rtnContext *pContext,
                                              pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_GETINDEX_GENRESULT ) ;

      CoordIndexMap indexMap ;
      rtnContextBuf buffObj ;

      // get index from all nodes
      while( TRUE )
      {
         rc = pContext->getMore( 1, buffObj, cb ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }
            else
            {
               PD_LOG ( PDERROR, "Failed to get index data, rc: %d", rc ) ;
               goto error ;
            }
         }

         try
         {
            BSONObj boTmp( buffObj.data() ) ;
            BSONObj boIndexDef ;
            BSONElement ele ;
            string strIndexName ;
            CoordIndexMap::iterator iter ;

            ele = boTmp.getField( IXM_FIELD_NAME_INDEX_DEF ) ;
            PD_CHECK ( ele.type() == Object, SDB_INVALIDARG, error,
                       PDERROR, "Failed to get the field(%s)",
                       IXM_FIELD_NAME_INDEX_DEF ) ;

            boIndexDef = ele.embeddedObject() ;
            ele = boIndexDef.getField( IXM_NAME_FIELD ) ;
            PD_CHECK ( ele.type() == String, SDB_INVALIDARG, error,
                       PDERROR, "Failed to get the field(%s)",
                       IXM_NAME_FIELD ) ;

            strIndexName = ele.valuestr() ;
            iter = indexMap.find( strIndexName ) ;
            if ( indexMap.end() == iter )
            {
               indexMap[ strIndexName ] = boTmp.getOwned() ;
            }
            else
            {
               // check the index
               BSONObjIterator newIter( boIndexDef ) ;
               BSONObj boOldDef ;

               ele = iter->second.getField( IXM_FIELD_NAME_INDEX_DEF ) ;
               PD_CHECK ( ele.type() == Object, SDB_INVALIDARG, error,
                          PDERROR, "Failed to get the field(%s)",
                          IXM_FIELD_NAME_INDEX_DEF ) ;
               boOldDef = ele.embeddedObject() ;

               BSONElement beTmp1, beTmp2 ;
               while( newIter.more() )
               {
                  beTmp1 = newIter.next() ;
                  if ( 0 == ossStrcmp( beTmp1.fieldName(), "_id" ) )
                  {
                     continue ;
                  }
                  beTmp2 = boOldDef.getField( beTmp1.fieldName() ) ;
                  if ( 0 != beTmp1.woCompare( beTmp2 ) )
                  {
                     PD_LOG( PDWARNING, "Corrupted index(name:%s, define1:%s, "
                             "define2:%s)", strIndexName.c_str(),
                             boIndexDef.toString().c_str(),
                             boOldDef.toString().c_str() ) ;
                     break ;
                  }
               }
            }
         }
         catch ( std::exception &e )
         {
            PD_RC_CHECK( rc, PDERROR, "Failed to get index, occured unexpected"
                         "error:%s", e.what() ) ;
         }
      }

      {
         CoordIndexMap::iterator iterMap = indexMap.begin();
         while( iterMap != indexMap.end() )
         {
            rc = pContext->append( iterMap->second ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to get index, append the data "
                         "failed(rc=%d)", rc ) ;
            ++iterMap ;
         }
      }

   done:
      PD_TRACE_EXITRC ( COORD_GETINDEX_GENRESULT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDGetCount implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDGetCount,
                                      CMD_NAME_GET_COUNT,
                                      TRUE ) ;
   _coordCMDGetCount::_coordCMDGetCount()
   {
   }

   _coordCMDGetCount::~_coordCMDGetCount()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_GETCOUNT_GENRESULT, "_coordCMDGetCount::generateResult" )
   INT32 _coordCMDGetCount::generateResult( rtnContext *pContext,
                                            pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_GETCOUNT_GENRESULT ) ;

      SINT64 totalCount = 0 ;
      rtnContextBuf buffObj ;

      while( TRUE )
      {
         rc = pContext->getMore( 1, buffObj, cb ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }
            else
            {
               PD_LOG ( PDERROR, "Failed to generate count result"
                        "get data failed, rc: %d", rc ) ;
               goto error ;
            }
         }

         try
         {
            BSONObj boTmp( buffObj.data() ) ;
            BSONElement beTotal = boTmp.getField( FIELD_NAME_TOTAL ) ;
            PD_CHECK( beTotal.isNumber(), SDB_INVALIDARG, error,
                      PDERROR, "Failed to get the field(%s)",
                      FIELD_NAME_TOTAL ) ;
            totalCount += beTotal.number() ;
         }
         catch ( std::exception &e )
         {
            rc = SDB_SYS ;
            PD_RC_CHECK( rc, PDERROR, "Failed to generate count result,"
                         "occured unexpected error:%s", e.what() );
         }
      }

      try
      {
         rc = pContext->append( BSON( FIELD_NAME_TOTAL << totalCount ) ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to generate count result,"
                      "append the data failed, rc: %d", rc ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_RC_CHECK( rc, PDERROR, "Failed to generate count result,"
                      "occured unexpected error:%s", e.what() ) ;
      }

   done:
      PD_TRACE_EXITRC ( COORD_GETCOUNT_GENRESULT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDGetDatablocks implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDGetDatablocks,
                                      CMD_NAME_GET_DATABLOCKS,
                                      TRUE ) ;
   _coordCMDGetDatablocks::_coordCMDGetDatablocks()
   {
   }

   _coordCMDGetDatablocks::~_coordCMDGetDatablocks()
   {
   }

   INT32 _coordCMDGetDatablocks::generateResult( rtnContext * pContext,
                                                 pmdEDUCB * cb )
   {
      // don't merge data, do nothing
      return SDB_OK ;
   }

   /*
      _coordCMDGetQueryMeta implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDGetQueryMeta,
                                      CMD_NAME_GET_QUERYMETA,
                                      TRUE ) ;
   _coordCMDGetQueryMeta::_coordCMDGetQueryMeta()
   {
   }

   _coordCMDGetQueryMeta::~_coordCMDGetQueryMeta()
   {
   }

   /*
      _coordCMDCrtProcedure implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDCrtProcedure,
                                      CMD_NAME_CRT_PROCEDURE,
                                      FALSE ) ;
   _coordCMDCrtProcedure::_coordCMDCrtProcedure()
   {
   }

   _coordCMDCrtProcedure::~_coordCMDCrtProcedure()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_CRTPROCEDURE_EXE, "_coordCMDCrtProcedure::execute" )
   INT32 _coordCMDCrtProcedure::execute( MsgHeader *pMsg,
                                         pmdEDUCB *cb,
                                         INT64 &contextID,
                                         rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( COORD_CRTPROCEDURE_EXE ) ;

      contextID = -1 ;

      MsgOpQuery *forward  = (MsgOpQuery *)pMsg ;
      forward->header.opCode = MSG_CAT_CRT_PROCEDURES_REQ ;

      _printDebug ( (const CHAR*)pMsg, getName() ) ;

      rc = executeOnCataGroup ( pMsg, cb, TRUE, NULL, NULL, buf ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "failed to crt procedures, rc = %d", rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( COORD_CRTPROCEDURE_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDEval implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDEval, CMD_NAME_EVAL, TRUE ) ;
   _coordCMDEval::_coordCMDEval()
   {
   }

   _coordCMDEval::~_coordCMDEval()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_EVAL_EXE, "_coordCMDEval::execute" )
   INT32 _coordCMDEval::execute( MsgHeader *pMsg,
                                 pmdEDUCB *cb,
                                 INT64 &contextID,
                                 rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( COORD_EVAL_EXE ) ;
      spdSession *session = NULL ;
      contextID           = -1 ;

      CHAR *pQuery = NULL ;
      BSONObj procedures ;
      spdCoordDownloader downloader( this, cb ) ;
      BSONObj runInfo ;

      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL,
                            NULL, NULL, &pQuery, NULL,
                            NULL, NULL );
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract eval msg:%d", rc) ;
         goto error ;
      }

      try
      {
         procedures = BSONObj( pQuery ) ;
         PD_LOG( PDDEBUG, "eval:%s", procedures.toString().c_str() ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      session = SDB_OSS_NEW _spdSession() ;
      if ( NULL == session )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      rc = session->eval( procedures, &downloader, cb ) ;
      if ( SDB_OK != rc )
      {
         const BSONObj &errmsg = session->getErrMsg() ;
         if ( !errmsg.isEmpty() )
         {
            *buf = rtnContextBuf( errmsg.getOwned() ) ;
         }
         PD_LOG( PDERROR, "failed to eval store procedure:%d", rc ) ;
         goto error ;
      }

      if ( FMP_RES_TYPE_VOID != session->resType() )
      {
         rc = _buildContext( session, cb, contextID ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to prepare reply msg:%d", rc ) ;
            goto error ;
         }
      }

      runInfo = BSON( FIELD_NAME_RTYPE << session->resType() ) ;
      *buf = rtnContextBuf( runInfo ) ;

   done:
      /// when -1 != contextID, session will be freed
      /// in context destructor.
      if ( -1 == contextID )
      {
         SAFE_OSS_DELETE( session ) ;
      }
      PD_TRACE_EXITRC( COORD_EVAL_EXE, rc ) ;
      return rc ;
   error:
      if ( contextID >= 0 )
      {
         pmdGetKRCB()->getRTNCB()->contextDelete( contextID, cb ) ;
         contextID = -1 ;
      }
      goto done ;
   }

   INT32 _coordCMDEval::_buildContext( _spdSession *session,
                                       pmdEDUCB *cb,
                                       SINT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      const BSONObj &evalRes = session->getRetMsg() ;
      SDB_ASSERT( !evalRes.isEmpty(), "impossible" ) ;

      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;
      rtnContextSP *context = NULL ;
      rc = rtnCB->contextNew ( RTN_CONTEXT_SP, (rtnContext**)&context,
                               contextID, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create new context, rc: %d", rc ) ;

      rc = context->open( session ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to open context[%lld], rc: %d",
                   context->contextID(), rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDRmProcedure implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDRmProcedure,
                                      CMD_NAME_RM_PROCEDURE,
                                      FALSE ) ;
   _coordCMDRmProcedure::_coordCMDRmProcedure()
   {
   }

   _coordCMDRmProcedure::~_coordCMDRmProcedure()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_RMPROCEDURE_EXE, "_coordCMDRmProcedure::execute" )
   INT32 _coordCMDRmProcedure::execute( MsgHeader *pMsg,
                                        pmdEDUCB *cb,
                                        INT64 &contextID,
                                        rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( COORD_RMPROCEDURE_EXE ) ;
      contextID = -1 ;

      MsgOpQuery *forward  = (MsgOpQuery *)pMsg ;
      forward->header.opCode = MSG_CAT_RM_PROCEDURES_REQ ;

      rc = executeOnCataGroup ( pMsg, cb, TRUE, NULL, NULL, buf ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "failed to rm procedures, rc = %d",
                  rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( COORD_RMPROCEDURE_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDSetSessionAttr implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDSetSessionAttr,
                                      CMD_NAME_SETSESS_ATTR,
                                      TRUE ) ;
   _coordCMDSetSessionAttr::_coordCMDSetSessionAttr()
   {
   }

   _coordCMDSetSessionAttr::~_coordCMDSetSessionAttr()
   {
   }

   //PD_TRACE_DECLARE_FUNCTION( COORD_SETSESSIONATTR_EXE, "_coordCMDSetSessionAttr::execute" )
   INT32 _coordCMDSetSessionAttr::execute( MsgHeader *pMsg,
                                           pmdEDUCB *cb,
                                           INT64 &contextID,
                                           rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_SETSESSIONATTR_EXE ) ;
      coordSessionPropSite *pPropSite = NULL ;
      pmdRemoteSessionSite *pSite = NULL ;
      // fill default-reply(delete success)
      contextID = -1 ;

      CHAR *pQuery                     = NULL ;
      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL, NULL, NULL,
                            &pQuery, NULL, NULL, NULL );
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to parse unlink collection request(rc=%d)",
                   rc ) ;

      pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;
      if ( pSite )
      {
         pPropSite = ( coordSessionPropSite* )pSite->getUserData() ;
      }
      if ( !pPropSite )
      {
         PD_LOG( PDERROR, "Session's prop site is NULL" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      try
      {
         BSONObj boQuery ;
         BSONElement bePreferRepl ;
         INT32 sessReplType = PREFER_REPL_TYPE_MIN ;

         boQuery = BSONObj( pQuery );
         bePreferRepl = boQuery.getField( FIELD_NAME_PREFERED_INSTANCE );
         PD_CHECK( bePreferRepl.type() == NumberInt, SDB_INVALIDARG, error,
                   PDERROR, "Failed to set session attribute, failed to get "
                   "the field[%s]", FIELD_NAME_PREFERED_INSTANCE );
         sessReplType = bePreferRepl.Int();
         PD_CHECK( sessReplType > PREFER_REPL_TYPE_MIN &&
                   sessReplType < PREFER_REPL_TYPE_MAX,
                   SDB_INVALIDARG, error, PDERROR,
                   "Failed to set preferedInstanace, invalid value[%d], "
                   "Value range:(%d~%d)", sessReplType,
                   PREFER_REPL_TYPE_MIN, PREFER_REPL_TYPE_MAX ) ;

         /// set and clear last nodes
         pPropSite->setPreferInsType( sessReplType ) ;
         pPropSite->clear() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Failed to set sessionAttr, received unexpected "
                 "error:%s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( COORD_SETSESSIONATTR_EXE, rc ) ;
      return rc;
   error:
      goto done;
   }

   /*
      _coordCMDReelection implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDReelection,
                                      CMD_NAME_REELECT,
                                      TRUE ) ;
   _coordCMDReelection::_coordCMDReelection()
   {
   }

   _coordCMDReelection::~_coordCMDReelection()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_REELECT_EXE, "_coordCMDReelection::execute" )
   INT32 _coordCMDReelection::execute( MsgHeader *pMsg,
                                       pmdEDUCB *cb,
                                       INT64 &contextID,
                                       rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( COORD_REELECT_EXE ) ;
      CHAR *pQuery = NULL ;
      const CHAR *gpName = NULL ;
      CoordGroupInfoPtr gpInfo ;
      CoordGroupList gpLst ;

      contextID = -1 ;

      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL,
                            NULL, NULL, &pQuery,
                            NULL, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to parse the message, rc: %d", rc ) ;

      try
      {
         BSONObj query( pQuery ) ;
         BSONElement ele = query.getField( FIELD_NAME_GROUPNAME ) ;
         if ( String != ele.type() )
         {
            PD_LOG( PDERROR, "Invalid reelection msg:%s",
                    query.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         gpName = ele.valuestr() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "unexpected error happened:%s", e.what() ) ;
         goto error ;
      }

      rc = _pResource->getOrUpdateGroupInfo( gpName, gpInfo, cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Update group info by name[%s] failed, rc: %d",
                 gpName, rc ) ;
         goto error ;
      }

      gpLst[gpInfo->groupID()] = gpInfo->groupID() ;
      rc = executeOnDataGroup( pMsg, cb, gpLst, TRUE, NULL, NULL,
                               NULL, buf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to execute on group[%s], rc: %d",
                 gpName, rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( COORD_REELECT_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDTruncate implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDTruncate,
                                      CMD_NAME_TRUNCATE,
                                      FALSE ) ;
   _coordCMDTruncate::_coordCMDTruncate()
   {
   }

   _coordCMDTruncate::~_coordCMDTruncate()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_TRUNCATE_EXE, "_coordCMDTruncate::execute" )
   INT32 _coordCMDTruncate::execute( MsgHeader *pMsg,
                                     pmdEDUCB *cb,
                                     INT64 &contextID,
                                     rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( COORD_TRUNCATE_EXE ) ;
      CHAR *option = NULL;
      BSONObj boQuery ;
      const CHAR *fullName = NULL ;
      rc = msgExtractQuery( ( CHAR * )pMsg, NULL, NULL,
                            NULL, NULL, &option, NULL,
                            NULL, NULL );
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract msg:%d", rc ) ;
         goto error ;
      }

      try
      {
         boQuery = BSONObj( option );
         BSONElement e = boQuery.getField( FIELD_NAME_COLLECTION );
         if ( String != e.type() )
         {
            PD_LOG( PDERROR, "invalid truncate msg:%s",
                    boQuery.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         fullName = e.valuestr() ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error;
      }

      rc = executeOnCL( pMsg, cb, fullName, FALSE, NULL, NULL,
                        NULL, NULL, buf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to truncate cl:%s, rc:%d",
                 fullName, rc ) ;
         goto error ;
      }
   done:
      if ( fullName )
      {
         PD_AUDIT_COMMAND( AUDIT_DDL, CMD_NAME_TRUNCATE, AUDIT_OBJ_CL,
                           fullName, rc, "" ) ;
      }
      PD_TRACE_EXITRC( COORD_TRUNCATE_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

}

