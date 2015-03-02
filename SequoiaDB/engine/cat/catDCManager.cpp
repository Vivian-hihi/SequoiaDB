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

   Source File Name = catDCManager.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =     XJH Opt

*******************************************************************************/

#include "catCommon.hpp"
#include "msgCatalog.hpp"
#include "pmdCB.hpp"
#include "rtn.hpp"
#include "catDCManager.hpp"
#include "clsDCMgr.hpp"
#include "msgMessage.hpp"
#include "pdTrace.hpp"
#include "catTrace.hpp"

using namespace bson ;

namespace engine
{

   /*
      _catDCManager implement
   */
   _catDCManager::_catDCManager()
   {
      _pDmsCB = NULL ;
      _pDpsCB = NULL ;
      _pRtnCB = NULL ;
      _pCatCB = NULL ;
      _pEduCB = NULL ;
   }

   _catDCManager::~_catDCManager()
   {
   }

   INT32 _catDCManager::init()
   {
      pmdKRCB *krcb     = pmdGetKRCB() ;
      _pDmsCB           = krcb->getDMSCB();
      _pDpsCB           = krcb->getDPSCB();
      _pRtnCB           = krcb->getRTNCB();
      _pCatCB           = krcb->getCATLOGUECB();
      return SDB_OK ;
   }

   void _catDCManager::attachCB( pmdEDUCB * cb )
   {
      _pEduCB = cb ;
   }

   void _catDCManager::detachCB( pmdEDUCB * cb )
   {
      _pEduCB = NULL ;
   }

   INT32 _catDCManager::updateGlobalAddr()
   {
      // not primary
      if ( !pmdIsPrimary() )
      {
         return SDB_CLS_NOT_PRIMARY ;
      }
      return catUpdateBaseInfoAddr( pmdGetOptionCB()->getCatAddr().c_str(),
                                    TRUE, _pEduCB, 1 ) ;
   }

   INT32 _catDCManager::active()
   {
      INT32 rc = SDB_OK;

      // update global info
      rc = _updateGlobalInfo() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to update global info, rc: %d", rc ) ;

      // update imange info
      rc = _updateImageInfo() ;
      if ( rc )
      {
         PD_LOG( PDWARNING, "Update image info failed, rc: %d", rc ) ;
         // when update image info failed, ignore
         rc = SDB_OK ;
      }

   done :
      return rc ;
   error :
      PMD_SHUTDOWN_DB( rc ) ;
      goto done ;
   }

   INT32 _catDCManager::deactive()
   {
      return SDB_OK ;
   }

   INT32 _catDCManager::processMsg( const NET_HANDLE &handle,
                                    MsgHeader *pMsg )
   {
      INT32 rc = SDB_OK;

      switch ( pMsg->opCode )
      {
      // command message entry, should dispatch in the entry function
      case MSG_CAT_ATTACH_IMAGE_REQ :
         rc = processCommandMsg( handle, pMsg, TRUE ) ;
         break ;

      default :
            rc = SDB_UNKNOWN_MESSAGE;
            PD_LOG( PDWARNING, "Received unknown message (opCode: [%d]%u )",
                    IS_REPLY_TYPE(pMsg->opCode),
                    GET_REQUEST_TYPE(pMsg->opCode) ) ;
            break;
      }
      return rc ;
   }

   INT32 _catDCManager::processCommandMsg( const NET_HANDLE &handle,
                                           MsgHeader *pMsg,
                                           BOOLEAN writable )
   {
      INT32 rc = SDB_OK ;
      MsgOpQuery *pQueryReq = (MsgOpQuery *)pMsg ;

      MsgOpReply replyHeader ;
      rtnContextBuf ctxBuff ;

      INT32 flag = 0 ;
      CHAR *pCMDName = NULL ;
      INT64 numToSkip = 0 ;
      INT64 numToReturn = 0 ;
      CHAR *pQuery = NULL ;
      CHAR *pFieldSelector = NULL ;
      CHAR *pOrderBy = NULL ;
      CHAR *pHint = NULL ;

      // init reply msg
      replyHeader.header.messageLength = sizeof( MsgOpReply ) ;
      replyHeader.contextID = -1 ;
      replyHeader.flags = SDB_OK ;
      replyHeader.numReturned = 0 ;
      replyHeader.startFrom = 0 ;
      _fillRspHeader( &(replyHeader.header), &(pQueryReq->header) ) ;

      // extract msg
      rc = msgExtractQuery( (CHAR*)pMsg, &flag, &pCMDName, &numToSkip,
                            &numToReturn, &pQuery, &pFieldSelector,
                            &pOrderBy, &pHint ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to extract query msg, rc: %d", rc ) ;

      if ( writable && !pmdIsPrimary() )
      {
         rc = SDB_CLS_NOT_PRIMARY ;
         PD_LOG ( PDWARNING, "Service deactive but received command: %s"
                  "opCode: %d", pCMDName, pQueryReq->header.opCode ) ;
         goto error ;
      }

      // the second dispatch msg
      switch ( pQueryReq->header.opCode )
      {
         case MSG_CAT_ATTACH_IMAGE_REQ :
            rc = processCmdAttachImage( handle, pQuery, ctxBuff ) ;
            break ;
         default :
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "Recieved unknow command: %s, opCode: %d",
                    pCMDName, pQueryReq->header.opCode ) ;
            break ;
      }

      PD_RC_CHECK( rc, PDERROR, "Process command[%s] failed, opCode: %d, "
                   "rc: %d", pCMDName, pQueryReq->header.opCode, rc ) ;

   done:
      // send reply
      if ( !_pCatCB->isDelayed() )
      {
         if ( 0 == ctxBuff.size() )
         {
            rc = _pCatCB->netWork()->syncSend( handle, (void*)&replyHeader ) ;
         }
         else
         {
            replyHeader.header.messageLength += ctxBuff.size() ;
            replyHeader.numReturned = ctxBuff.recordNum() ;
            rc = _pCatCB->netWork()->syncSend( handle,
                                               &(replyHeader.header),
                                               (void*)ctxBuff.data(),
                                               ctxBuff.size() ) ;
         }
      }
      return rc ;
   error:
      replyHeader.flags = rc ;
      goto done ;
   }

   INT32 _catDCManager::processCmdAttachImage( const NET_HANDLE &handle,
                                               const CHAR *pQuery,
                                               rtnContextBuf &ctxBuff )
   {
      INT32 rc = SDB_OK ;
      clsDCMgr dcMgr ;
      clsDCBaseInfo *pBaseInfo = NULL ;
      const CHAR *clusterName = NULL ;
      const CHAR *businessName = NULL ;
      string address ;
      BSONObjBuilder retObjBuild ;

      rc = _mapData2DCMgr( &dcMgr ) ;
      PD_RC_CHECK( rc, PDERROR, "Map dc base data to dc manager failed, "
                   "rc: %d", rc ) ;

      pBaseInfo = dcMgr.getDCBaseInfo() ;

      try
      {
         vector< string > vecSourceGrp ;
         BSONElement eleGroups ;
         BSONObj objGroups ;
         BSONObj objQuery( pQuery ) ;

         if ( !pBaseInfo->hasImage() )
         {
            BSONElement eleAddr = objQuery.getField( FIELD_NAME_ADDRESS ) ;
            if ( String != eleAddr.type() ||
                 0 == ossStrlen( eleAddr.valuestr() ) )
            {
               PD_LOG( PDERROR, "Param[%s] is invalid in obj[%s]",
                       FIELD_NAME_ADDRESS, objQuery.toString().c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            rc = dcMgr.setImageCatAddr( eleAddr.valuestr() ) ;
            PD_RC_CHECK( rc, PDERROR, "Parse image catalog address failed, "
                         "rc: %d", rc ) ;

            // update image catalog
            rc = dcMgr.updateImageCataGroup( _pEduCB ) ;
            PD_RC_CHECK( rc, PDERROR, "Update image catalog group failed, "
                         "rc: %d", rc ) ;
            address = dcMgr.getImageCatAddr() ;

            // update image dc base info
            rc = dcMgr.updateImageDCBaseInfo( _pEduCB ) ;
            PD_RC_CHECK( rc, PDERROR, "Update image dc base info failed, "
                         "rc: %d", rc ) ;
            clusterName = dcMgr.getImageDCBaseInfo( _pEduCB,
                                    FALSE )->getClusterName() ;
            businessName = dcMgr.getImageDCBaseInfo( _pEduCB,
                                    FALSE )->getBusinessName() ;
         }

         // analysis groups
         eleGroups = objQuery.getField( FIELD_NAME_GROUPS ) ;
         if ( Array == eleGroups.type() )
         {
            objGroups = eleGroups.embeddedObject() ;
         }
         else if ( !eleGroups.eoo() )
         {
            PD_LOG( PDERROR, "Field[%s] is invalid in obj[%s]",
                    FIELD_NAME_GROUPS, objQuery.toString().c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         // if objGroups is empty, will map all groups by the same name
         if ( objGroups.isEmpty() || 0 == objGroups.nFields() )
         {
            sdbCatalogueCB::GRP_ID_MAP *grp = _pCatCB->getGroupMap( TRUE ) ;
            sdbCatalogueCB::GRP_ID_MAP::iterator it = grp->begin() ;
            while ( it != grp->end() )
            {
               pBaseInfo->addGroup( it->second, it->second ) ;
               PD_RC_CHECK( rc, PDERROR, "Add group[%s:%s] failed when attach "
                            "image, rc: %d", it->second.c_str(),
                            it->second.c_str(), rc ) ;
               vecSourceGrp.push_back( it->second ) ;
               ++it ;
            }
            grp = _pCatCB->getGroupMap( FALSE ) ;
            it = grp->begin() ;
            while ( it != grp->end() )
            {
               rc = pBaseInfo->addGroup(  it->second, it->second ) ;
               PD_RC_CHECK( rc, PDERROR, "Add group[%s:%s] failed when attach "
                            "image, rc: %d", it->second.c_str(),
                            it->second.c_str(), rc ) ;
               vecSourceGrp.push_back( it->second ) ;
               ++it ;
            }
         }
         else
         {
            map< string, string > mapAddGrps ;
            map< string, string >::iterator it ;
            rc = pBaseInfo->addGroups( objGroups, &mapAddGrps ) ;
            PD_RC_CHECK( rc, PDERROR, "Add groups[%s] failed when attach "
                         "image, rc: %d", objGroups.toString().c_str(), rc ) ;
            it = mapAddGrps.begin() ;
            while ( it != mapAddGrps.end() )
            {
               vecSourceGrp.push_back( it->first ) ;
               ++it ;
            }
         }

         rc = _makeGroupsObj( retObjBuild, vecSourceGrp ) ;
         PD_RC_CHECK( rc, PDERROR, "Make groups obj failed, rc: %d", rc ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Attach image occurs exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // update info to collection
      {
         BSONObjBuilder builder ;
         if ( clusterName )
         {
            builder.append( FIELD_NAME_IMAGE"."FIELD_NAME_CLUSTERNAME,
                            clusterName ) ;
         }
         if ( businessName )
         {
            builder.append( FIELD_NAME_IMAGE"."FIELD_NAME_BUSINESSNAME,
                            businessName ) ;
         }
         if ( !address.empty() )
         {
            builder.append( FIELD_NAME_IMAGE"."FIELD_NAME_ADDRESS,
                            address ) ;
         }
         _dcBaseInfoGroups2Obj( pBaseInfo, builder,
                                FIELD_NAME_IMAGE"."FIELD_NAME_GROUPS ) ;
         BSONObj updator = BSON( "$set" << builder.obj() ) ;
         BSONObj matcher = BSON( FIELD_NAME_TYPE <<
                                 CAT_BASE_TYPE_GLOBAL_STR ) ;
         BSONObj hint ;
         rc = rtnUpdate( CAT_SYSDCBASE_COLLECTION_NAME, matcher, updator,
                         hint, 0, _pEduCB, _pDmsCB, _pDpsCB, _majoritySize(),
                         NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Update obj[%s] to collection[%s] failed, "
                      "rc: %d", updator.toString().c_str(),
                      CAT_SYSDCBASE_COLLECTION_NAME, rc ) ;
      }

      // get return groups
      ctxBuff = rtnContextBuf( retObjBuild.obj() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _catDCManager::_fillRspHeader( MsgHeader * rspMsg,
                                       const MsgHeader * reqMsg )
   {
      rspMsg->opCode = MAKE_REPLY_TYPE( reqMsg->opCode ) ;
      rspMsg->requestID = reqMsg->requestID ;
      rspMsg->routeID.value = 0 ;
      rspMsg->TID = reqMsg->TID ;
   }

   INT32 _catDCManager::_mapData2DCMgr( _clsDCMgr *pDCMgr )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN exist = FALSE ;
      BSONObj infoObj ;

      rc = pDCMgr->initialize() ;
      PD_RC_CHECK( rc, PDERROR, "Init dc manager failed, rc: %d", rc ) ;

      // get data
      rc = catCheckBaseInfoExist( CAT_BASE_TYPE_GLOBAL_STR, exist,
                                  infoObj, _pEduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Check dc base info exist failed, "
                   "rc: %d", rc ) ;

      if ( exist )
      {
         rc = pDCMgr->updateDCBaseInfo( infoObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Update dc base info failed, rc: %d", rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _catDCManager::_makeGroupsObj( BSONObjBuilder &builder,
                                        vector< string > &vecGroups )
   {
      INT32 rc = SDB_OK ;
      UINT32 groupID = 0 ;
      BSONArrayBuilder sub( builder.subarrayStart( CAT_GROUP_NAME ) ) ;
      for ( UINT32 i = 0 ; i < vecGroups.size() ; ++i )
      {
         groupID = _pCatCB->groupName2ID( vecGroups[ i ] ) ;
         if ( CAT_INVALID_GROUPID == groupID )
         {
            rc = SDB_CAT_GRP_NOT_EXIST ;
            goto error ;
         }
         sub.append( BSON( CAT_GROUPID_NAME << groupID <<
                           CAT_GROUPNAME_NAME << vecGroups[ i ] ) ) ;
      }
      sub.done() ;
   done:
      return rc ;
   error:
      goto done ;
   }

   void _catDCManager::_dcBaseInfoGroups2Obj( _clsDCBaseInfo *pInfo,
                                              BSONObjBuilder &builder,
                                              const CHAR *pFieldName )
   {
      BSONArrayBuilder arrayBuild( builder.subarrayStart( pFieldName ) ) ;
      map<string, string> *pGroups = pInfo->getImageGroups() ;
      map<string, string>::iterator it = pGroups->begin() ;
      while ( it != pGroups->end() )
      {
         arrayBuild.append( BSON_ARRAY( it->first << it->second ) ) ;
         ++it ;
      }
   }

   INT16 _catDCManager::_majoritySize()
   {
      return _pCatCB->majoritySize() ;
   }

   INT32 _catDCManager::_updateGlobalInfo()
   {
      INT32 rc = SDB_OK ;
      BOOLEAN exist = FALSE ;
      BSONObj infoObj ;
      pmdOptionsCB *option = pmdGetOptionCB() ;

      string clusterName ;
      string businessName ;
      option->getFieldStr( PMD_OPTION_CLUSTER_NAME, clusterName, "" ) ;
      option->getFieldStr( PMD_OPTION_BUSINESS_NAME, businessName, "" ) ;

      rc = catCheckBaseInfoExist( CAT_BASE_TYPE_GLOBAL_STR, exist,
                                  infoObj, _pEduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Check dc base info exist failed, "
                   "rc: %d", rc ) ;

      if ( !exist )
      {
         // if the global info not exist, need to create
         infoObj = BSON( FIELD_NAME_TYPE << CAT_BASE_TYPE_GLOBAL_STR <<
                         FIELD_NAME_DATACENTER << BSON(
                           FIELD_NAME_CLUSTERNAME << clusterName <<
                           FIELD_NAME_BUSINESSNAME << businessName <<
                           FIELD_NAME_ADDRESS << option->getCatAddr() ) <<
                         FIELD_NAME_ACTIVE << true ) ;
         rc = rtnInsert( CAT_SYSDCBASE_COLLECTION_NAME, infoObj, 1, 0,
                         _pEduCB, _pDmsCB, _pDpsCB, 1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Insert global info[%s] to collection[%s] "
                      "failed, rc: %d", infoObj.toString().c_str(),
                      CAT_SYSDCBASE_COLLECTION_NAME, rc ) ;
      }
      else
      {
         INT64 updateNum = 0 ;
         string tmpClsName ;
         string tmpBusName ;
         // if the global info exist, update
         BSONElement dcEle = infoObj.getField( FIELD_NAME_DATACENTER ) ;
         if ( Object ==  dcEle.type() )
         {
            BSONObj dcObj = dcEle.embeddedObject() ;
            BSONElement e1 = dcObj.getField( FIELD_NAME_CLUSTERNAME ) ;
            BSONElement e2 = dcObj.getField( FIELD_NAME_BUSINESSNAME ) ;
            tmpClsName = e1.valuestrsafe() ;
            tmpBusName = e2.valuestrsafe() ;
         }

         if ( clusterName != tmpClsName || businessName != tmpBusName )
         {
            PD_LOG( PDEVENT, "Cluster name[%s] or business name[%s] has "
                    "changed to %s:%s", tmpClsName.c_str(), tmpBusName.c_str(),
                    clusterName.c_str(), businessName.c_str() ) ;
            BSONObj updator = BSON( "$set" << BSON(
              FIELD_NAME_DATACENTER"."FIELD_NAME_CLUSTERNAME << clusterName <<
              FIELD_NAME_DATACENTER"."FIELD_NAME_BUSINESSNAME << businessName )
                                   ) ;
            BSONObj matcher = BSON( FIELD_NAME_TYPE <<
                                    CAT_BASE_TYPE_GLOBAL_STR ) ;
            rc = rtnUpdate( CAT_SYSDCBASE_COLLECTION_NAME, matcher, updator,
                            BSONObj(), 0, _pEduCB, _pDmsCB, _pDpsCB, 1,
                            &updateNum ) ;
            PD_RC_CHECK( rc, PDERROR, "Update global info[%s] failed, rc: %d",
                         updator.toString().c_str(), rc ) ;
            if ( updateNum <= 0 )
            {
               PD_LOG( PDERROR, "Not found global info, matcher: %s",
                       matcher.toString().c_str() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _catDCManager::_updateImageInfo()
   {
      INT32 rc = SDB_OK ;
      clsDCMgr dcMgr ;
      clsDCBaseInfo *pBaseInfo = NULL ;
      clsDCBaseInfo *pImageBaseInfo = NULL ;

      rc = _mapData2DCMgr( &dcMgr ) ;
      PD_RC_CHECK( rc, PDERROR, "Map dc base data to dc manager failed, "
                   "rc: %d", rc ) ;

      pBaseInfo = dcMgr.getDCBaseInfo() ;

      // if has image, need to update image's cluster name, business name and
      // cat address
      // because catalog is a single thread, so don't lock_r
      if ( pBaseInfo->hasImage() )
      {
         BSONObjBuilder builder ;
         BSONObj newObj ;
         string catAddr ;
         const CHAR *imageClsName = NULL ;
         const CHAR *imageBsName = NULL ;

         rc = dcMgr.updateImageCataGroup( _pEduCB ) ;
         PD_RC_CHECK( rc, PDWARNING, "Update image catagroup failed, rc: %d",
                      rc ) ;

         // if catalog address has update, need to reflush
         catAddr = dcMgr.getImageCatAddr() ;
         if ( 0 != ossStrcmp( catAddr.c_str(),
                              pBaseInfo->getImageAddress() ) )
         {
            rc = catUpdateBaseInfoAddr( catAddr.c_str(), FALSE, _pEduCB, 1 ) ;
            PD_RC_CHECK( rc, PDERROR, "Update image address to dc base info "
                         "failed, rc: %d", rc ) ;
         }

         // if image clustername or businessname has update, need to reflush
         rc = dcMgr.updateImageDCBaseInfo( _pEduCB ) ;
         PD_RC_CHECK( rc, PDWARNING, "Update image dc base info failed, "
                      "rc: %d", rc ) ;
         pImageBaseInfo = dcMgr.getImageDCBaseInfo( _pEduCB, FALSE ) ;
         imageClsName = pImageBaseInfo->getClusterName() ;
         imageBsName = pImageBaseInfo->getBusinessName() ;

         if ( 0 != ossStrcmp( pBaseInfo->getImageClusterName(),
                              imageClsName ) )
         {
            builder.append( FIELD_NAME_IMAGE"."FIELD_NAME_CLUSTERNAME,
                            imageClsName ) ;
         }
         if ( 0 != ossStrcmp( pBaseInfo->getImageBusinessName(),
                              imageBsName ) )
         {
            builder.append( FIELD_NAME_IMAGE"."FIELD_NAME_BUSINESSNAME,
                            imageBsName ) ;
         }
         newObj = builder.obj() ;
         if ( !newObj.isEmpty() )
         {
            BSONObj matcher = BSON( FIELD_NAME_TYPE <<
                                    CAT_BASE_TYPE_GLOBAL_STR ) ;
            BSONObj updator = BSON( "$set" << newObj ) ;
            BSONObj hint ;
            rc = rtnUpdate( CAT_SYSDCBASE_COLLECTION_NAME, matcher,
                            updator, hint, 0, _pEduCB, _pDmsCB, _pDpsCB,
                            1, NULL ) ;
            PD_RC_CHECK( rc, PDERROR, "Update obj[%s] to collection[%s] "
                         "failed, rc: %d", updator.toString().c_str(),
                         CAT_SYSDCBASE_COLLECTION_NAME, rc ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

}


