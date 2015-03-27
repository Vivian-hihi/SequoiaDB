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

   Source File Name = rtnCoordDelete.cpp

   Descriptive Name = Runtime Coord Delete

   When/how to use: this program may be used on binary and text-formatted
   version of runtime component. This file contains code logic for
   data delete request from coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "rtnCoordDelete.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "msgMessage.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

using namespace bson;

namespace engine
{
   //PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCODEL_EXECUTE, "rtnCoordDelete::execute" )
   INT32 rtnCoordDelete::execute( MsgHeader *pMsg,
                                  pmdEDUCB *cb,
                                  INT64 &contextID,
                                  rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      INT32 rcTmp = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNCODEL_EXECUTE ) ;
      pmdKRCB *pKrcb                   = pmdGetKRCB() ;
      CoordCB *pCoordcb                = pKrcb->getCoordCB() ;
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent() ;

      // process define
      rtnSendOptions sendOpt( TRUE ) ;
      rtnSendMsgIn inMsg( pMsg ) ;
      rtnProcessResult result ;
      ROUTE_RC_MAP nokRC ;
      result._pNokRC = &nokRC ;

      CoordCataInfoPtr cataInfo ;
      MsgRouteID errNodeID ;
      UINT64 deleteNum = 0 ;
      inMsg._pvtData = ( CHAR* )&deleteNum ;
      inMsg._pvtType = PRIVATE_DATA_NUMBERLONG ;

      BSONObj boDeletor ;

      // fill default-reply(delete success)
      MsgOpDelete *pDelMsg             = (MsgOpDelete *)pMsg ;
      contextID                        = -1 ;

      INT32 flag = 0;
      CHAR *pCollectionName = NULL ;
      CHAR *pDeletor = NULL ;
      CHAR *pHint = NULL ;
      rc = msgExtractDelete( (CHAR*)pMsg, &flag, &pCollectionName,
                             &pDeletor, &pHint ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to parse delete request, rc: %d", rc ) ;

      try
      {
         boDeletor = BSONObj( pDeletor ) ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK( SDB_INVALIDARG, PDERROR,
                      "Delete failed, received unexpected error:%s",
                      e.what() ) ;
      }

      rc = rtnCoordGetCataInfo( cb, pCollectionName, FALSE, cataInfo ) ;
      PD_RC_CHECK( rc, PDERROR, "Delete failed, failed to get the "
                   "catalogue info(collection name: %s), rc: %d",
                   pCollectionName, rc ) ;

   retry:
      do
      {
         pDelMsg->version = cataInfo->getVersion() ;
         pDelMsg->w = 0 ;

         if ( cataInfo->isMainCL() )
         {
            rcTmp = doOpOnMainCL( cataInfo, boDeletor, inMsg, sendOpt,
                                  pRouteAgent, cb, result ) ;
         }
         else
         {
            rcTmp = doOpOnCL( cataInfo, boDeletor, inMsg, sendOpt,
                              pRouteAgent, cb, result ) ;
         }
      }while( FALSE ) ;

      if ( SDB_OK == rcTmp && nokRC.empty() )
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
         PD_LOG( PDERROR, "Delete failed on node[%s], rc: %d",
                 routeID2String( errNodeID ).c_str(), rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_RTNCODEL_EXECUTE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   void rtnCoordDelete::_prepareForTrans( pmdEDUCB *cb, MsgHeader *pMsg )
   {
      pMsg->opCode = MSG_BS_TRANS_DELETE_REQ ;
   }

   INT32 rtnCoordDelete::_prepareMainCLOp( CoordCataInfoPtr &cataInfo,
                                           CoordGroupSubCLMap &grpSubCl,
                                           rtnSendMsgIn &inMsg,
                                           rtnSendOptions &options,
                                           netMultiRouteAgent *pRouteAgent,
                                           pmdEDUCB *cb,
                                           rtnProcessResult &result,
                                           ossValuePtr &outPtr )
   {
      INT32 rc                = SDB_OK ;
      MsgOpDelete *pDelMsg    = ( MsgOpDelete* )inMsg.msg() ;

      INT32 flag              = 0 ;
      CHAR *pCollectionName   = NULL;
      CHAR *pDeletor          = NULL;
      CHAR *pHint             = NULL;
      BSONObj boDeletor ;
      BSONObj boHint ;
      BSONObj boNew ;

      CHAR *pBuff             = NULL ;
      INT32 buffLen           = 0 ;
      INT32 buffPos           = 0 ;

      CoordGroupSubCLMap::iterator it ;

      outPtr                  = (ossValuePtr)0 ;
      inMsg.data()->clear() ;

      rc = msgExtractDelete( (CHAR*)inMsg.msg(), &flag, &pCollectionName,
                             &pDeletor, &pHint ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to parse delete request, rc: %d",
                   rc ) ;

      boDeletor = BSONObj( pDeletor ) ;
      boHint = BSONObj( pHint ) ;

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
         ioItem.iovLen = ossRoundUpToMultipleX ( offsetof(MsgOpDelete, name) +
                                                 pDelMsg->nameLength + 1, 4 ) -
                         sizeof( MsgHeader ) ;
         iovec.push_back( ioItem ) ;

         // 2. new deletor vec
         boNew = _buildNewDeletor( boDeletor, subCLLst ) ;
         // 2.1 add to buff
         INT32 roundLen = ossRoundUpToMultipleX( boNew.objsize(), 4 ) ;
         if ( buffPos + roundLen > buffLen )
         {
            rc = cb->reallocBuff( roundLen + buffLen, &pBuff, buffLen ) ;
            PD_RC_CHECK( rc, PDERROR, "Realloc buff[%d] failed, rc: %d",
                         roundLen + buffLen, rc ) ;
         }
         ossMemcpy( &pBuff[ buffPos ], boNew.objdata(), boNew.objsize() ) ;
         ioItem.iovBase = &pBuff[ buffPos ] ;
         ioItem.iovLen = roundLen ;
         buffPos += roundLen ;
         iovec.push_back( ioItem ) ;

         // 3. hinter vec
         ioItem.iovBase = boHint.objdata() ;
         ioItem.iovLen = boHint.objsize() ;
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

   void rtnCoordDelete::_doneMainCLOp( ossValuePtr itPtr,
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
   }

   BSONObj rtnCoordDelete::_buildNewDeletor( const BSONObj &deletor,
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
      builder.appendElements( deletor ) ;
      builder.appendArray( CAT_SUBCL_NAME, babSubCL.arr() ) ;
      return builder.obj() ;
   }

}

