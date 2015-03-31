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

   Source File Name = rtnCoordQuery.cpp

   Descriptive Name = Runtime Coord Query

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   query on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "coordSession.hpp"
#include "rtnCoord.hpp"
#include "rtnContext.hpp"
#include "rtnCB.hpp"
#include "msg.h"
#include "coordCB.hpp"
#include "rtnCoordCommon.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "msgMessage.hpp"
#include "rtnCoordQuery.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "rtn.hpp"
#include "rtnCoordCommands.hpp"
#include "pmdEDU.hpp"

using namespace bson;

namespace engine
{
   INT32 rtnCoordQuery::_checkQueryModify( rtnSendMsgIn &inMsg,
                                           rtnSendOptions &options )
   {
      MsgOpQuery *queryMsg = ( MsgOpQuery* )inMsg.msg() ;
      INT32 rc = SDB_OK ;

      if ( queryMsg->flags & FLG_QUERY_MODIFY )
      {
         if ( options._groupLst.size() > 1 )
         {
            rtnQueryPvtData *privateData = ( rtnQueryPvtData* )inMsg._pvtData ;
            if ( privateData->_pContext->getLimitNum() > 0 ||
                 privateData->_pContext->getSkipNum() > 0 )
            {
               rc = SDB_RTN_QUERYMODIFY_MULTI_NODES ;
               PD_LOG( PDERROR, "query and modify can't use skip and limit "
                  "in multiple nodes, rc: %d", rc ) ;
            }
         }
      }

      return rc ;
   }

   void rtnCoordQuery::_optimize( rtnSendMsgIn &inMsg,
                                  rtnSendOptions &options,
                                  rtnProcessResult &result )
   {
      SDB_ASSERT( inMsg._pvtData &&
                  inMsg._pvtType == PRIVATE_DATA_USER,
                  "Private data invalid" ) ;

      rtnQueryPvtData *pvtData = ( rtnQueryPvtData* )inMsg._pvtData ;

      if ( pvtData->_pContext && 0 == result._sucGroupLst.size() )
      {
         MsgOpQuery *pQueryMsg = ( MsgOpQuery* )inMsg.msg() ;
         INT64 ctxRetNum = pvtData->_pContext->getLimitNum() ;
         INT64 ctxSkipNum = pvtData->_pContext->getSkipNum() ;

         /// if send to one node
         if ( options._groupLst.size() <= 1 )
         {
            if ( ctxSkipNum > 0 )
            {
               pQueryMsg->numToSkip = ctxSkipNum ;
               pvtData->_pContext->setSkipNum( 0 ) ;
               if ( ctxRetNum > 0 &&
                    pQueryMsg->numToReturn == ctxRetNum + ctxSkipNum )
               {
                  pQueryMsg->numToReturn -= ctxSkipNum ;
               }
            }
         }
         else
         {
            if ( pQueryMsg->numToSkip > 0 )
            {
               if ( pQueryMsg->numToReturn > 0 )
               {
                  pvtData->_pContext->setLimitNum( pQueryMsg->numToReturn ) ;
                  pQueryMsg->numToReturn += pQueryMsg->numToSkip ;
               }
               pvtData->_pContext->setSkipNum( pQueryMsg->numToSkip ) ;
               pQueryMsg->numToSkip = 0 ;
            }
         }
      }
   }

   INT32 rtnCoordQuery::_prepareCLOp( CoordCataInfoPtr &cataInfo,
                                      rtnSendMsgIn &inMsg,
                                      rtnSendOptions &options,
                                      netMultiRouteAgent *pRouteAgent,
                                      pmdEDUCB *cb,
                                      rtnProcessResult &result,
                                      ossValuePtr &outPtr )
   {
      INT32 rc = SDB_OK ;

      _optimize( inMsg, options, result ) ;

      rc = _checkQueryModify( inMsg, options ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void rtnCoordQuery::_doneCLOp( ossValuePtr itPtr,
                                  CoordCataInfoPtr &cataInfo,
                                  rtnSendMsgIn &inMsg,
                                  rtnSendOptions &options,
                                  netMultiRouteAgent *pRouteAgent,
                                  pmdEDUCB *cb,
                                  rtnProcessResult &result )
   {
      INT32 rc = SDB_OK ;
      /// add succeed reply to context, and release the reply
      SDB_ASSERT( inMsg._pvtData &&
                  inMsg._pvtType == PRIVATE_DATA_USER,
                  "Private data invalid" ) ;

      rtnQueryPvtData *pvtData = ( rtnQueryPvtData* )inMsg._pvtData ;
      ROUTE_REPLY_MAP *pOkReply = result._pOkReply ;

      SDB_ASSERT( pOkReply, "Ok reply invalid" ) ;

      BOOLEAN takeOver = FALSE ;
      MsgOpReply *pReply = NULL ;
      MsgRouteID nodeID ;
      ROUTE_REPLY_MAP::iterator it = pOkReply->begin() ;
      while( it != pOkReply->end() )
      {
         takeOver = FALSE ;
         pReply = (MsgOpReply*)(it->second) ;
         nodeID.value = pReply->header.routeID.value ;

         if ( SDB_OK == pReply->flags )
         {
            if ( pvtData->_pContext )
            {
               rc = pvtData->_pContext->addSubContext( pReply, takeOver ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Add sub data[node: %s, context: %lld] to "
                          "context[%s] failed, rc: %d",
                          routeID2String( nodeID ).c_str(), pReply->contextID,
                          pvtData->_pContext->toString().c_str(), rc ) ;
                  pvtData->_ret = rc ;
               }
            }
            else
            {
               SDB_ASSERT( pReply->contextID == -1, "Context leak" ) ;
            }
         }

         if ( !takeOver )
         {
            SDB_OSS_FREE( pReply ) ;
         }
         ++it ;
      }
      pOkReply->clear() ;
   }

   INT32 rtnCoordQuery::_prepareMainCLOp( CoordCataInfoPtr &cataInfo,
                                          CoordGroupSubCLMap &grpSubCl,
                                          rtnSendMsgIn &inMsg,
                                          rtnSendOptions &options,
                                          netMultiRouteAgent *pRouteAgent,
                                          pmdEDUCB *cb,
                                          rtnProcessResult &result,
                                          ossValuePtr &outPtr )
   {
      INT32 rc                = SDB_OK ;
      MsgOpQuery *pQueryMsg   = ( MsgOpQuery* )inMsg.msg() ;

      INT32 flags             = 0 ;
      CHAR *pCollectionName   = NULL ;
      INT64 numToSkip         = 0 ;
      INT64 numToReturn       = -1 ;
      CHAR *pQuery            = NULL ;
      CHAR *pFieldSelector    = NULL ;
      CHAR *pOrderBy          = NULL ;
      CHAR *pHint             = NULL ;

      BSONObj objQuery ;
      BSONObj objSelector ;
      BSONObj objOrderby ;
      BSONObj objHint ;
      BSONObj newQuery ;

      CHAR *pBuff             = NULL ;
      INT32 buffLen           = 0 ;
      INT32 buffPos           = 0 ;

      outPtr                  = (ossValuePtr)0 ;

      CoordGroupSubCLMap::iterator it ;

      _optimize( inMsg, options, result ) ;

      rc = _checkQueryModify( inMsg, options ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      if ( options._useSpecialGrp )
      {
         goto done ;
      }

      inMsg.data()->clear() ;

      rc = msgExtractQuery( (CHAR*)pQueryMsg, &flags, &pCollectionName,
                            &numToSkip, &numToReturn, &pQuery,
                            &pFieldSelector, &pOrderBy, &pHint ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract query msg failed, rc: %d", rc ) ;

      try
      {
         objQuery = BSONObj( pQuery ) ;
         objSelector = BSONObj( pFieldSelector ) ;
         objOrderby = BSONObj( pOrderBy ) ;
         objHint = BSONObj( pHint ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Extrace query msg occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = cb->allocBuff( DMS_PAGE_SIZE4K, &pBuff, buffLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Alloc buff[%d] failed, rc: %d",
                   DMS_PAGE_SIZE4K, rc ) ;

      it = grpSubCl.begin() ;
      while( it != grpSubCl.end() )
      {
         CoordSubCLlist &subCLLst = it->second ;

         netIOVec &iovec = inMsg._datas[ it->first ] ;
         netIOV ioItem ;

         // 1. first vec
         ioItem.iovBase = (CHAR*)inMsg.msg() + sizeof( MsgHeader ) ;
         ioItem.iovLen = ossRoundUpToMultipleX ( offsetof(MsgOpQuery, name) +
                         pQueryMsg->nameLength + 1, 4 ) - sizeof( MsgHeader ) ;
         iovec.push_back( ioItem ) ;

         // 2. new query vec
         newQuery = _buildNewQuery( objQuery, subCLLst ) ;
         // 2.1 add to buff
         INT32 roundLen = ossRoundUpToMultipleX( newQuery.objsize(), 4 ) ;
         if ( buffPos + roundLen > buffLen )
         {
            rc = cb->reallocBuff( roundLen + buffLen, &pBuff, buffLen ) ;
            PD_RC_CHECK( rc, PDERROR, "Realloc buff[%d] failed, rc: %d",
                         roundLen + buffLen, rc ) ;
         }
         ossMemcpy( &pBuff[ buffPos ], newQuery.objdata(),
                    newQuery.objsize() ) ;
         ioItem.iovBase = &pBuff[ buffPos ] ;
         ioItem.iovLen = roundLen ;
         buffPos += roundLen ;
         iovec.push_back( ioItem ) ;

         // 3. last vec
         ioItem.iovBase = objSelector.objdata() ;
         ioItem.iovLen = ossRoundUpToMultipleX( objSelector.objsize(), 4 ) +
                         ossRoundUpToMultipleX( objOrderby.objsize(), 4 ) +
                         objHint.objsize() ;
         iovec.push_back( ioItem ) ;         

         ++it ;
      }

      outPtr = ( ossValuePtr )pBuff ;

   done:
      return rc ;
   error:
      if ( pBuff )
      {
         cb->releaseBuff( pBuff ) ;
      }
      goto done ;
   }

   void rtnCoordQuery::_doneMainCLOp( ossValuePtr itPtr,
                                      CoordCataInfoPtr &cataInfo,
                                      CoordGroupSubCLMap &grpSubCl,
                                      rtnSendMsgIn &inMsg,
                                      rtnSendOptions &options,
                                      netMultiRouteAgent *pRouteAgent,
                                      pmdEDUCB *cb,
                                      rtnProcessResult &result )
   {
      CHAR *pBuff = ( CHAR* )itPtr ;
      if ( NULL != pBuff )
      {
         cb->releaseBuff( pBuff ) ;
      }
      inMsg._datas.clear() ;

      _doneCLOp( itPtr, cataInfo, inMsg, options,
                 pRouteAgent, cb, result ) ;
   }

   BSONObj rtnCoordQuery::_buildNewQuery( const BSONObj &query,
                                          const CoordSubCLlist &subCLList )
   {
      BSONObjBuilder builder ;
      BSONArrayBuilder babSubCL ;
      CoordSubCLlist::const_iterator iterCL = subCLList.begin();
      while( iterCL != subCLList.end() )
      {
         babSubCL.append( *iterCL ) ;
         ++iterCL ;
      }
      builder.appendElements( query ) ;
      builder.appendArray( CAT_SUBCL_NAME, babSubCL.arr() ) ;
      return builder.obj() ;
   }

   INT32 rtnCoordQuery::execute( MsgHeader *pMsg,
                                 pmdEDUCB *cb,
                                 INT64 &contextID,
                                 rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      rtnContextCoord *pContext        = NULL ;

      // fill default-reply(query success)
      contextID                        = -1 ;

      CHAR *pCollectionName            = NULL ;

      rc = msgExtractQuery( (CHAR*)pMsg, NULL, &pCollectionName,
                            NULL, NULL, NULL, NULL, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR,
                  "Failed to parse query request, rc: %d", rc ) ;

      // process command
      if ( pCollectionName != NULL && '$' == pCollectionName[0] )
      {
         rtnCoordCommand *pCmdProcesser = NULL;
         rtnCoordProcesserFactory *pProcesserFactory
                  = pCoordcb->getProcesserFactory();
         pCmdProcesser = pProcesserFactory->getCommandProcesser(
                                             pCollectionName ) ;
         PD_CHECK( pCmdProcesser != NULL, SDB_INVALIDARG, error, PDERROR,
                  "unknown command:%s", pCollectionName ) ;

         rc = pCmdProcesser->execute( pMsg, cb, contextID, buf ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to execute the "
                      "command(command:%s, rc=%d)",
                      pCollectionName, rc ) ;
      }
      else
      {
         rtnSendOptions sendOpt ;
         rc = queryOrDoOnCL( pMsg, pRouteAgent, cb, &pContext,
                             sendOpt ) ;
         PD_RC_CHECK( rc, PDERROR, "query failed, rc: %d", rc ) ;

         contextID = pContext->contextID() ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnCoordQuery::queryOrDoOnCL( MsgHeader *pMsg,
                                       netMultiRouteAgent *pRouteAgent,
                                       pmdEDUCB *cb,
                                       rtnContextCoord **pContext,
                                       rtnSendOptions & sendOpt,
                                       rtnQueryConf *pQueryConf )
   {
      return _queryOrDoOnCL( pMsg, pRouteAgent, cb, pContext,
                             sendOpt, NULL, pQueryConf ) ;
   }

   INT32 rtnCoordQuery::queryOrDoOnCL( MsgHeader *pMsg,
                                       netMultiRouteAgent *pRouteAgent,
                                       pmdEDUCB *cb,
                                       rtnContextCoord **pContext,
                                       rtnSendOptions &sendOpt,
                                       CoordGroupList &sucGrpLst,
                                       rtnQueryConf *pQueryConf )
   {
      return _queryOrDoOnCL( pMsg, pRouteAgent, cb, pContext,
                             sendOpt, &sucGrpLst, pQueryConf ) ;
   }

   INT32 rtnCoordQuery::_queryOrDoOnCL( MsgHeader *pMsg,
                                        netMultiRouteAgent *pRouteAgent,
                                        pmdEDUCB *cb,
                                        rtnContextCoord **pContext,
                                        rtnSendOptions &sendOpt,
                                        CoordGroupList *pSucGrpLst,
                                        rtnQueryConf *pQueryConf )
   {
      INT32 rc = SDB_OK ;
      INT32 rcTmp = SDB_OK ;
      INT64 contextID = -1 ;
      CoordCataInfoPtr cataInfo ;
      const CHAR *pRealCLName = NULL ;
      BOOLEAN openEmptyContext = FALSE ;
      BOOLEAN updateCata = FALSE ;
      BOOLEAN allCataGroup = FALSE ;

      MsgRouteID errNodeID ;
      rtnSendMsgIn inMsg( pMsg ) ;

      rtnProcessResult result ;
      ROUTE_RC_MAP nokRC ;
      result._pNokRC = &nokRC ;
      ROUTE_REPLY_MAP okReply ;
      result._pOkReply = &okReply ;

      rtnQueryPvtData pvtData ;
      inMsg._pvtType = PRIVATE_DATA_USER ;
      inMsg._pvtData = (CHAR*)&pvtData ;

      if ( pQueryConf )
      {
         openEmptyContext = pQueryConf->_openEmptyContext ;
         updateCata = pQueryConf->_updateAndGetCata ;
         if ( !pQueryConf->_realCLName.empty() )
         {
            pRealCLName = pQueryConf->_realCLName.c_str() ;
         }
         allCataGroup = pQueryConf->_allCataGroups ;
      }

      SET_RC *pOldIgnoreRC = sendOpt._pIgnoreRC ;
      SET_RC ignoreRC ;
      if ( pOldIgnoreRC )
      {
         ignoreRC = *pOldIgnoreRC ;
      }
      ignoreRC.insert( SDB_DMS_EOC ) ;
      sendOpt._pIgnoreRC = &ignoreRC ;

      MsgOpQuery *pQueryMsg   = ( MsgOpQuery* )pMsg ;
      SDB_RTNCB *pRtncb       = pmdGetKRCB()->getRTNCB() ;
      CHAR *pNewMsg           = NULL ;
      BOOLEAN needReset       = FALSE ;

      INT32 flags             = 0 ;
      CHAR *pCollectionName   = NULL ;
      INT64 numToSkip         = 0 ;
      INT64 numToReturn       = -1 ;
      CHAR *pQuery            = NULL ;
      CHAR *pFieldSelector    = NULL ;
      CHAR *pOrderBy          = NULL ;

      BSONObj objQuery ;
      BSONObj objSelector ;
      BSONObj objOrderby ;

      rc = msgExtractQuery( (CHAR*)pMsg, &flags, &pCollectionName,
                            &numToSkip, &numToReturn, &pQuery,
                            &pFieldSelector, &pOrderBy, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract query msg failed, rc: %d", rc ) ;

      try
      {
         if ( !allCataGroup )
         {
            objQuery = BSONObj( pQuery ) ;
         }
         objSelector = BSONObj( pFieldSelector ) ;
         objOrderby = BSONObj( pOrderBy ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Extrace query msg occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( pContext )
      {
         if ( NULL == *pContext )
         {
            // create context
            rc = pRtncb->contextNew( RTN_CONTEXT_COORD,
                                     (rtnContext **)pContext,
                                     contextID, cb ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to allocate context(rc=%d)",
                         rc ) ;
         }
         else
         {
            contextID = (*pContext)->contextID() ;
            // the context is create in out side, do nothing
         }
         pvtData._pContext = *pContext ;
      }

      if ( pvtData._pContext && !pvtData._pContext->isOpened() )
      {
         // open context, explain only in query msg
         if ( ( ( FLG_QUERY_EXPLAIN & pQueryMsg->flags ) &&
                '$' != pCollectionName[ 0 ] ) ||
              openEmptyContext )
         {
            rc = pvtData._pContext->open( BSONObj(), BSONObj(), -1, 0 ) ;
         }
         else
         {
            // build new selector
            rtnNeedResetSelector( objSelector, objOrderby, needReset ) ;
            if ( needReset )
            {
               rc = _buildNewMsg( (const CHAR*)pMsg, BSONObj(), pNewMsg ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "Failed to build new msg: %d", rc ) ;
                  goto error ;
               }
               pQueryMsg = (MsgOpQuery *)pNewMsg ;
               inMsg._pMsg = ( MsgHeader* )pNewMsg ;
            }

            // open context
            rc = pvtData._pContext->open( objOrderby,
                                          needReset ? objSelector : BSONObj(),
                                          pQueryMsg->numToReturn,
                                          pQueryMsg->numToSkip ) ;

            // change some data
            if ( pQueryMsg->numToReturn > 0 && pQueryMsg->numToSkip > 0 )
            {
               // some record may skip on coord,
               // so the num of records from data-node must
               // more than "numToReturn + numToSkip"
               pQueryMsg->numToReturn += pQueryMsg->numToSkip ;
            }
            pQueryMsg->numToSkip = 0 ;
         }
         PD_RC_CHECK( rc, PDERROR, "Open context failed(rc=%d)", rc ) ;
      }

      // get collection catalog info
      rc = rtnCoordGetCataInfo( cb, pRealCLName ? pRealCLName : pCollectionName,
                                updateCata, cataInfo ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to get the catalog info(collection:%s), rc: %d",
                   pRealCLName ? pRealCLName : pCollectionName, rc ) ;
      if ( updateCata )
      {
         ++sendOpt._retryTimes ;
      }

   retry:
      do
      {
         pQueryMsg->version = cataInfo->getVersion() ;

         if ( cataInfo->isMainCL() )
         {
            rcTmp = doOpOnMainCL( cataInfo, objQuery, inMsg, sendOpt,
                                  pRouteAgent, cb, result ) ;
         }
         else
         {
            rcTmp = doOpOnCL( cataInfo, objQuery, inMsg, sendOpt,
                              pRouteAgent, cb, result ) ;
         }         
      }while( FALSE ) ;

      if ( SDB_OK != pvtData._ret )
      {
         rc = pvtData._ret ;
         PD_LOG( PDERROR, "Query failed, rc: %d", rc ) ;
         goto error ;
      }
      else if ( SDB_OK == rcTmp && nokRC.empty() )
      {
         goto done ;
      }
      else if ( checkRetryForCLOpr( rcTmp, &nokRC, inMsg.msg(),
                                    sendOpt._retryTimes,
                                    cataInfo, cb, rc, &errNodeID, TRUE ) )
      {
         nokRC.clear() ;
         ++sendOpt._retryTimes ;
         goto retry ;
      }
      else
      {
         PD_LOG( PDERROR, "Query failed on node[%s], rc: %d",
                 routeID2String( errNodeID ).c_str(), rc ) ;
         goto error ;
      }

      if ( pvtData._pContext )
      {
         pvtData._pContext->addSubDone( cb ) ;
      }

   done:
      if ( pNewMsg )
      {
         SDB_OSS_FREE( pNewMsg ) ;
      }
      if ( pSucGrpLst )
      {
         *pSucGrpLst = result._sucGroupLst ;
      }
      sendOpt._pIgnoreRC = pOldIgnoreRC ;
      return rc ;
   error:
      if ( SDB_CAT_NO_MATCH_CATALOG == rc )
      {
         rc = SDB_OK ;
         goto done ;
      }
      if ( -1 != contextID  )
      {
         pRtncb->contextDelete( contextID, cb ) ;
         contextID = -1 ;
         *pContext = NULL ;
      }
      goto done ;
   }

   INT32 rtnCoordQuery::_buildNewMsg( const CHAR *msg,
                                      const bson::BSONObj &newSelector,
                                      CHAR *&newMsg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0;
      CHAR *pCollectionName = NULL;
      SINT64 numToSkip = 0;
      SINT64 numToReturn = 0;
      CHAR *pQuery = NULL;
      CHAR *pFieldSelector = NULL;
      CHAR *pOrderBy = NULL;
      CHAR *pHint = NULL;
      BSONObj query ;
      BSONObj selector ;
      BSONObj orderBy ;
      BSONObj hint ;
      INT32 bufSize = 0 ;
      MsgOpQuery *pSrc = (MsgOpQuery *)msg;

      rc = msgExtractQuery( ( CHAR * )msg, &flag, &pCollectionName,
                            &numToSkip, &numToReturn, &pQuery,
                            &pFieldSelector, &pOrderBy, &pHint );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to parse query request(rc=%d)", rc );

      try
      {
         query = BSONObj( pQuery ) ;
         selector = BSONObj( pFieldSelector ) ;
         orderBy = BSONObj( pOrderBy ) ;
         hint = BSONObj( pHint ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected error happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = msgBuildQueryMsg( &newMsg, &bufSize,
                             pCollectionName,
                             flag, pSrc->header.requestID,
                             numToSkip, numToReturn,
                             &query, &newSelector,
                             &orderBy, &hint ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to build new msg:%d", rc ) ;
         goto error ;
      } 
   done:
      return rc ;
   error:
      SAFE_OSS_FREE( newMsg ) ;
      goto done ;
   }

}
