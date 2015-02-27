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
      rc = _updateGlobalInfo () ;
      PD_RC_CHECK( rc, PDERROR, "Failed to update global info, rc: %d", rc ) ;

   done :
      return rc;
   error :
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
      default :
            rc = SDB_UNKNOWN_MESSAGE;
            PD_LOG( PDWARNING, "Received unknown message (opCode: [%d]%u )",
                    IS_REPLY_TYPE(pMsg->opCode),
                    GET_REQUEST_TYPE(pMsg->opCode) ) ;
            break;
      }
      return rc ;
   }

   void _catDCManager::_fillRspHeader( MsgHeader * rspMsg,
                                       const MsgHeader * reqMsg )
   {
      rspMsg->opCode = MAKE_REPLY_TYPE( reqMsg->opCode ) ;
      rspMsg->requestID = reqMsg->requestID ;
      rspMsg->routeID.value = 0 ;
      rspMsg->TID = reqMsg->TID ;
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
      if ( rc )
      {
         goto error ;
      }
      if ( !exist )
      {
         // if the global info not exist, need to create
         infoObj = BSON( FIELD_NAME_TYPE << CAT_BASE_TYPE_GLOBAL_STR <<
                         FIELD_NAME_DATACENTER << BSON(
                           FIELD_NAME_CLUSTERNAME << clusterName <<
                           FIELD_NAME_BUSINESSNAME << businessName <<
                           FIELD_NAME_ADDRESS << option->getCatAddr() ) <<
                         FIELD_NAME_ACTIVE << true ) ;
         rc = rtnInsert( CAT_SYSBASE_COLLECTION_NAME, infoObj, 1, 0,
                         _pEduCB, _pDmsCB, _pDpsCB, 1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Insert global info[%s] to collection[%s] "
                      "failed, rc: %d", infoObj.toString().c_str(),
                      CAT_SYSBASE_COLLECTION_NAME, rc ) ;
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
            rc = rtnUpdate( CAT_SYSBASE_COLLECTION_NAME, matcher, updator,
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

}


