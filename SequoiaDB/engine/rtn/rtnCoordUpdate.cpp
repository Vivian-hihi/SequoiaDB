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

   Source File Name = rtnCoordUpdate.cpp

   Descriptive Name = Runtime Coord Update

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   update operation on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "rtnCoordUpdate.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "msgMessage.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "mthModifier.hpp"

using namespace bson;

namespace engine
{
   //PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOUPDATE_EXECUTE, "rtnCoordUpdate::execute" )
   INT32 rtnCoordUpdate::execute( CHAR *pReceiveBuffer,
                                  SINT32 packSize,
                                  pmdEDUCB *cb,
                                  MsgOpReply &replyHeader,
                                  rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      INT32 rcTmp = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNCOUPDATE_EXECUTE ) ;
      pmdKRCB *pKrcb                   = pmdGetKRCB() ;
      CoordCB *pCoordcb                = pKrcb->getCoordCB() ;
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent() ;

      // process define
      rtnSendOptions sendOpt( TRUE ) ;
      rtnSendMsgIn inMsg( (MsgHeader*)pReceiveBuffer ) ;
      rtnProcessResult result ;
      ROUTE_RC_MAP nokRC ;
      result._pNokRC = &nokRC ;

      CoordCataInfoPtr cataInfo ;
      MsgRouteID errNodeID ;
      UINT64 updateNum = 0 ;
      inMsg._pvtData = ( CHAR* )&updateNum ;
      inMsg._pvtType = PRIVATE_DATA_NUMBERLONG ;

      BSONObj newUpdator ;
      CHAR *pMsgBuff = NULL ;
      INT32 buffLen  = 0 ;
      MsgOpUpdate *pNewUpdate          = NULL ;
      BOOLEAN emptyUpdateCata          = FALSE ;

      // fill default-reply(update success)
      MsgOpUpdate *pUpdate             = (MsgOpUpdate *)pReceiveBuffer;
      replyHeader.header.messageLength = sizeof( MsgOpReply );
      replyHeader.header.opCode        = MSG_BS_UPDATE_RES;
      replyHeader.header.requestID     = pUpdate->header.requestID;
      replyHeader.header.routeID.value = 0;
      replyHeader.header.TID           = pUpdate->header.TID;
      replyHeader.contextID            = -1;
      replyHeader.flags                = SDB_OK;
      replyHeader.numReturned          = 0;
      replyHeader.startFrom            = 0;

      INT32 flag                       = 0;
      CHAR *pCollectionName            = NULL ;
      CHAR *pSelector                  = NULL ;
      CHAR *pUpdator                   = NULL ;
      CHAR *pHint                      = NULL ;
      BSONObj boSelector ;
      BSONObj boHint ;
      BSONObj boUpdator ;
      rc = msgExtractUpdate( pReceiveBuffer, &flag, &pCollectionName,
                             &pSelector, &pUpdator, &pHint );
      PD_RC_CHECK( rc, PDERROR, "Failed to parse update request, rc: %d",
                   rc ) ;

      try
      {
         boSelector = BSONObj( pSelector ) ;
         boHint = BSONObj( pHint ) ;
         boUpdator = BSONObj( pUpdator ) ;

         if ( boUpdator.isEmpty() )
         {
            PD_LOG( PDERROR, "modifier can't be empty" ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK( SDB_INVALIDARG, PDERROR,
                      "Update failed, received unexpected error: %s",
                      e.what() ) ;
      }

      rc = rtnCoordGetCataInfo( cb, pCollectionName, FALSE, cataInfo ) ;
      PD_RC_CHECK( rc, PDERROR, "Update failed, failed to get the "
                   "catalogue info(collection name: %s), rc: %d",
                   pCollectionName, rc ) ;

      pNewUpdate = pUpdate ;

   retry:
      do
      {
         BSONObj tmpNewObj = boUpdator ;
         BOOLEAN hasShardingKey = FALSE ;

         if ( cataInfo->isSharded() )
         {
            rc = kickShardingKey( cataInfo, boUpdator, tmpNewObj,
                                  hasShardingKey ) ;
            PD_RC_CHECK( rc, PDERROR, "Update failed, failed to kick the "
                         "sharding-key field(rc=%d)", rc ) ;

            if ( cataInfo->isMainCL() )
            {
               BSONObj newSubObj ;
               CoordSubCLlist subCLList ;
               rcTmp = cataInfo->getMatchSubCLs( boSelector, subCLList ) ;
               if ( rcTmp )
               {
                  PD_LOG( PDERROR,"Failed to get match sub-collection, "
                          "rc: %d", rcTmp ) ;
                  break ;
               }

               rc = kickShardingKeyForSubCL( subCLList, tmpNewObj,
                                             newSubObj,
                                             hasShardingKey, cb ) ;
               //rc = checkModifierForSubCL( subCLList, pUpdator, cb ) ;
               PD_RC_CHECK( rc, PDERROR,
                            "Failed to kick the sharding-key field "
                            "for sub-collection, rc: %d",
                            rc ) ;
               tmpNewObj = newSubObj ;
            }
         }

         if ( !hasShardingKey )
         {
            // no sharding key
            pNewUpdate = pUpdate ;
         }
         else if ( !pMsgBuff || !tmpNewObj.equal( newUpdator ) )
         {
            if ( tmpNewObj.isEmpty() )
            {
               if ( flag & FLG_UPDATE_UPSERT )
               {
                  tmpNewObj = BSON( "$null" << BSON( "null" << 1 ) ) ;
               }
               else if ( !emptyUpdateCata )
               {
                  rc = rtnCoordGetCataInfo( cb, pCollectionName, TRUE,
                                            cataInfo ) ;
                  PD_RC_CHECK( rc, PDERROR, "Update failed, failed to get the "
                               "catalogue info(collection name: %s), rc: %d",
                               pCollectionName, rc ) ;
                  emptyUpdateCata = TRUE ;
                  ++sendOpt._retryTimes ;
                  goto retry ;
               }
               else
               {
                  // don't do anything( return error?)
                  goto done ;
               }
            }

            newUpdator = tmpNewObj ;
            rc = msgBuildUpdateMsg( &pMsgBuff, &buffLen, pUpdate->name,
                                    flag, 0, &boSelector,
                                    &newUpdator, &boHint ) ;
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to build update request, rc: %d", rc ) ;
            pNewUpdate = (MsgOpUpdate *)pMsgBuff ;
         }

         pNewUpdate->version = cataInfo->getVersion() ;
         pNewUpdate->w = 0 ;
         if ( pNewUpdate->flags | FLG_UPDATE_UPSERT )
         {
            pNewUpdate->flags &= ~FLG_UPDATE_UPSERT ;
            pNewUpdate->flags |= FLG_UPDATE_RETURNNUM ;
         }
         inMsg._pMsg = ( MsgHeader* )pNewUpdate ;

         if ( cataInfo->isMainCL() )
         {
            rcTmp = doOpOnMainCL( cataInfo, boSelector, inMsg, sendOpt,
                                  pRouteAgent, cb, result ) ;
         }
         else
         {
            rcTmp = doOpOnCL( cataInfo, boSelector, inMsg, sendOpt,
                              pRouteAgent, cb, result ) ;
         }
      }while( FALSE ) ;

      if ( SDB_OK == rcTmp && nokRC.empty() )
      {
         // do nothing, for upsert
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
         PD_LOG( PDERROR, "Update failed on node[%s], rc: %d",
                 routeID2String( errNodeID ).c_str(), rc ) ;
         goto error ;
      }

      // upsert
      if ( ( flag & FLG_UPDATE_UPSERT ) && 0 == updateNum )
      {
         mthModifier modifier;
         BSONObj source ;
         BSONObj target ;
         rtnCoordProcesserFactory *pProcesserFactory = NULL ;
         rtnCoordOperator *pOpProcesser = NULL ;

         rc = modifier.loadPattern( boUpdator ) ;
         PD_RC_CHECK( rc, PDERROR, "Invalid pattern is detected for "
                      "updator: %s, rc: %d",
                      boUpdator.toString().c_str(), rc ) ;

         rc = modifier.modify( source, target ) ;
         PD_RC_CHECK( rc, PDERROR, "failed to generate upsertor "
                      "record(rc=%d)", rc ) ;

         pProcesserFactory = pCoordcb->getProcesserFactory() ;
         pOpProcesser = pProcesserFactory->getOperator( MSG_BS_INSERT_REQ ) ;
         SDB_ASSERT( pOpProcesser , "pCmdProcesser can't be NULL" ) ;

         rc = msgBuildInsertMsg( &pMsgBuff, &buffLen, pUpdate->name, 0,
                                 0, &target ) ;
         PD_RC_CHECK( rc, PDERROR, "failed to build insert message, rc: %d",
                      rc ) ;

         rc = pOpProcesser->execute( pMsgBuff,
                                     ((MsgHeader*)pMsgBuff)->messageLength,
                                     cb, replyHeader, buf ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to insert the data[%s], rc: %d",
                      target.toString().c_str(), rc ) ;
      }

   done:
      if ( flag & FLG_UPDATE_RETURNNUM )
      {
         replyHeader.contextID = updateNum ;
      }
      if ( pMsgBuff )
      {
         SDB_OSS_FREE( pMsgBuff ) ;
      }
      PD_TRACE_EXITRC ( SDB_RTNCOUPDATE_EXECUTE, rc ) ;
      return rc ;
   error:
      replyHeader.flags = rc ;
      goto done;
   }

   void rtnCoordUpdate::_prepareForTrans( pmdEDUCB *cb, MsgHeader *pMsg )
   {
      pMsg->opCode = MSG_BS_TRANS_UPDATE_REQ ;
   }

   INT32 rtnCoordUpdate::_prepareMainCLOp( CoordCataInfoPtr &cataInfo,
                                           CoordGroupSubCLMap &grpSubCl,
                                           rtnSendMsgIn &inMsg,
                                           rtnSendOptions &options,
                                           netMultiRouteAgent *pRouteAgent,
                                           pmdEDUCB *cb,
                                           rtnProcessResult &result,
                                           ossValuePtr &outPtr )
   {
      INT32 rc                = SDB_OK ;
      MsgOpUpdate *pUpMsg     = ( MsgOpUpdate* )inMsg.msg() ;

      INT32 flag              = 0 ;
      CHAR *pCollectionName   = NULL;
      CHAR *pSelector         = NULL ;
      CHAR *pUpdator          = NULL ;
      CHAR *pHint             = NULL;
      BSONObj boSelector ;
      BSONObj boUpdator ;
      BSONObj boHint ;
      BSONObj boNew ;

      CHAR *pBuff             = NULL ;
      INT32 buffLen           = 0 ;
      INT32 buffPos           = 0 ;

      CoordGroupSubCLMap::iterator it ;

      outPtr                  = (ossValuePtr)0 ;
      inMsg.data()->clear() ;

      rc = msgExtractUpdate( (CHAR*)pUpMsg, &flag, &pCollectionName,
                             &pSelector, &pUpdator, &pHint ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to parse update request, rc: %d",
                   rc ) ;

      boSelector = BSONObj( pSelector ) ;
      boUpdator = BSONObj( pUpdator ) ;
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
         ioItem.iovLen = ossRoundUpToMultipleX ( offsetof(MsgOpUpdate, name) +
                                                 pUpMsg->nameLength + 1, 4 ) -
                         sizeof( MsgHeader ) ;
         iovec.push_back( ioItem ) ;

         // 2. new deletor vec( selector )
         boNew = _buildNewSelector( boSelector, subCLLst ) ;
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

         // 3. for last( updator + hint )
         ioItem.iovBase = boUpdator.objdata() ;
         ioItem.iovLen = ossRoundUpToMultipleX( boUpdator.objsize(), 4 ) +
                         boHint.objsize() ;
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

   void rtnCoordUpdate::_doneMainCLOp( ossValuePtr itPtr,
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

   BSONObj rtnCoordUpdate::_buildNewSelector( const BSONObj &selector,
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
      builder.appendElements( selector ) ;
      builder.appendArray( CAT_SUBCL_NAME, babSubCL.arr() ) ;
      return builder.obj() ;
   }

   //PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOUPDATE_CKIFINSHKEY, "rtnCoordUpdate::checkIfIncludeShardingKey" )
   INT32 rtnCoordUpdate::checkIfIncludeShardingKey ( const CoordCataInfoPtr &cataInfo,
                                                     const CHAR *pUpdator,
                                                     BOOLEAN &isInclude,
                                                     pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOUPDATE_CKIFINSHKEY ) ;
      isInclude = FALSE;
      try
      {
         BSONObj boUpdator( pUpdator );
         BSONObjIterator iter( boUpdator );
         while ( iter.more() )
         {
            BSONElement beTmp = iter.next();
            BSONObj boTmp = beTmp.Obj();
            isInclude = cataInfo->isIncludeShardingKey( boTmp );
            if ( isInclude )
            {
               goto done;
            }
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG;
         PD_LOG ( PDERROR, "Failed to check the record is include sharding-key,"
                  "occured unexpected error:%s", e.what() );
         goto error;
      }
      done :
         PD_TRACE_EXITRC ( SDB_RTNCOUPDATE_CKIFINSHKEY, rc ) ;
         return rc;
      error :
         goto done;
   }

   INT32 rtnCoordUpdate::checkModifierForSubCL ( const CoordSubCLlist &subCLList,
                                                 const CHAR *pUpdator,
                                                 pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      BOOLEAN isInclude;
      CoordSubCLlist::const_iterator iterCL = subCLList.begin() ;
      while( iterCL != subCLList.end() )
      {
         CoordCataInfoPtr subCataInfo ;
         rc = rtnCoordGetCataInfo( cb, (*iterCL).c_str(), FALSE,
                                   subCataInfo ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "get catalog of sub-collection(%s) failed(rc=%d)",
                      (*iterCL).c_str(), rc ) ;
         rc = checkIfIncludeShardingKey( subCataInfo, pUpdator,
                                         isInclude, cb ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "failed to check if include sharding-key(rc=%d)",
                      rc ) ;
         if ( isInclude )
         {
            rc = SDB_UPDATE_SHARD_KEY ;
            goto done ;
         }
         ++iterCL ;
      }
   done:
      return rc;
   error:
      goto done;
   }

   INT32 rtnCoordUpdate::kickShardingKey( const CoordCataInfoPtr &cataInfo,
                                          const BSONObj &boUpdator,
                                          bson::BSONObj &boNewUpdator,
                                          BOOLEAN &hasShardingKey )
   {
      INT32 rc = SDB_OK ;
      try
      {
         BSONObj boShardingKey ;
         cataInfo->getShardingKey( boShardingKey ) ;
         BSONObjBuilder bobNewUpdator ;
         BSONObjIterator iter( boUpdator ) ;
         while ( iter.more() )
         {
            BSONElement beTmp = iter.next() ;
            if ( beTmp.type() != Object )
            {
               rc = SDB_INVALIDARG;
               PD_LOG( PDERROR, "updator's element must be an Object type:"
                       "updator=%s", boUpdator.toString().c_str() ) ;
               goto error;
            }
            BSONObj boTmp = beTmp.Obj() ;
            BSONObjBuilder bobFields;
            BSONObjIterator iterField( boTmp ) ;
            while( iterField.more() )
            {
               BSONElement beField = iterField.next() ;
               BSONObjIterator iterKey( boShardingKey ) ;
               BOOLEAN isKey = FALSE ;
               while( iterKey.more() )
               {
                  BSONElement beKey = iterKey.next();
                  const CHAR *pKey = beKey.fieldName();
                  const CHAR *pField = beField.fieldName();
                  while( *pKey == *pField && *pKey != '\0' )
                  {
                     ++pKey;
                     ++pField;
                  }

                  // shardingkey_fieldName == updator_fieldName
                  if ( *pKey == *pField
                     || ( '\0' == *pKey && '.' == *pField )
                     || ( '\0' == *pField && '.' == *pKey ) )
                  {
                     isKey = TRUE;
                     break;
                  }
               }
               if ( isKey )
               {
                  hasShardingKey = TRUE;
               }
               else
               {
                  bobFields.append( beField );
               }
            }
            BSONObj boFields = bobFields.obj();
            if ( !boFields.isEmpty() )
            {
               bobNewUpdator.appendObject( beTmp.fieldName(),
                                           boFields.objdata() );
            }
         }
         boNewUpdator = bobNewUpdator.obj() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG;
         PD_LOG ( PDERROR,"Failed to check the record is include sharding-key,"
                  "occured unexpected error: %s", e.what() ) ;
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 rtnCoordUpdate::kickShardingKeyForSubCL( const CoordSubCLlist &subCLList,
                                                  const BSONObj &boUpdator,
                                                  BSONObj &boNewUpdator,
                                                  BOOLEAN &hasShardingKey,
                                                  pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      CoordSubCLlist::const_iterator iterCL = subCLList.begin();
      BSONObj boCur = boUpdator;
      BSONObj boNew = boUpdator;

      while( iterCL != subCLList.end() )
      {
         CoordCataInfoPtr subCataInfo;
         rc = rtnCoordGetCataInfo( cb, (*iterCL).c_str(), FALSE,
                                   subCataInfo ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "get catalog of sub-collection(%s) failed(rc=%d)",
                      (*iterCL).c_str(), rc ) ;
         rc = kickShardingKey( subCataInfo, boCur, boNew, hasShardingKey ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to kick sharding-key for "
                      "sub-collection(rc=%d)", rc ) ;
         boCur = boNew ;
         ++iterCL ;
      }
      boNewUpdator = boNew ;

   done:
      return rc;
   error:
      goto done;
   }
}
