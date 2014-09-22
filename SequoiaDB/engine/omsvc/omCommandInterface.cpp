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

   Source File Name = omCommandInterface.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/12/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#include "omCommandInterface.hpp"
#include "omDef.hpp"
#include "rtn.hpp"


using namespace bson;

namespace engine
{
   omCommandInterafce::omCommandInterafce()
   {
   }

   omCommandInterafce::~omCommandInterafce()
   {
   }

   omRestCommandBase::omRestCommandBase()
   {
      _pKRCB  = pmdGetKRCB() ;
      _pDMDCB = _pKRCB->getDMSCB() ;
      _pRTNCB = _pKRCB->getRTNCB() ;
      _pDMSCB = _pKRCB->getDMSCB() ;
      _cb     = NULL ;
   }

   omRestCommandBase::~omRestCommandBase()
   {
   }

   INT32 omRestCommandBase::init( pmdEDUCB * cb )
   {
      _cb = cb ;

      return SDB_OK ;
   }

   bool omRestCommandBase::isFetchAgentResponse( UINT64 requestID )
   {
      return false ;
   }

   INT32 omRestCommandBase::doAgentResponse ( MsgHeader* pAgentResponse )
   {
      return SDB_OK ;
   }

   INT32 omRestCommandBase::_getBusinessInfo( string business, 
                                              BSONObj &businessInfo )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_BUSINESS_FIELD_NAME << business ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_BUSINESS, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "fail to query table:%s,rc=%d",
                     OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
         _errorDetail = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

      while ( TRUE )
      {
         BSONObjBuilder innerBuilder ;
         BSONObj tmp ;
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            contextID = -1 ;
            PD_LOG_MSG( PDERROR, "failed to get record from table:%s,rc=%d", 
                        OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
            _errorDetail = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         businessInfo = result.copy() ;
         break ;
      }
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omRestCommandBase::_getHostInfo( string hostName, 
                                           BSONObj &hostInfo )
   {
      INT32 rc = SDB_OK ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID = -1 ;

      matcher = BSON( OM_HOST_FIELD_NAME << hostName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "fail to query table:%s,rc=%d",
                     OM_CS_DEPLOY_CL_HOST, rc ) ;
         _errorDetail = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            contextID = -1 ;
            PD_LOG_MSG( PDERROR, "failed to get record from table:%s,rc=%d", 
                        OM_CS_DEPLOY_CL_HOST, rc ) ;
            _errorDetail = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
            goto error ;
         }

         BSONObj record( buffObj.data() ) ;
         BSONObj filter = BSON( OM_HOST_FIELD_PASSWORD << "" ) ;
         BSONObj result = record.filterFieldsUndotted( filter, false ) ;
         hostInfo = result.copy() ;
         break ;
      }
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omRestCommandBase::_deleteHost( const string &hostName )
   {
      INT32 rc          = SDB_OK ;
      BSONObj condition = BSON( OM_HOST_FIELD_NAME << hostName ) ;
      BSONObj hint ;

      rc = rtnDelete( OM_CS_DEPLOY_CL_HOST, condition, hint, 0, _cb );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to delete record from table:%s,"
                     "%s=%s,rc=%d", OM_CS_DEPLOY_CL_HOST, 
                     OM_HOST_FIELD_NAME, hostName.c_str(), rc ) ;
         _errorDetail = _cb->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omRestCommandBase::_getClusterInfo( const string &clusterName, 
                                             BSONObj &clusterInfo )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_CLUSTER_FIELD_NAME << clusterName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d", 
                    OM_CS_DEPLOY_CL_CLUSTER, rc ) ;
         _errorDetail = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            contextID = -1 ;
            PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d", 
                    OM_CS_DEPLOY_CL_CLUSTER, rc ) ;
            _errorDetail = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         clusterInfo = result.copy() ;
         break ;
      }
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   omAgentReqBase::omAgentReqBase( BSONObj &request )
                  :_request( request.copy() ), _response( BSONObj() )
   {
   }

   omAgentReqBase::~omAgentReqBase()
   {
   }

   void omAgentReqBase::getResponse( BSONObj &response )
   {
      _response = response ;
   }

}

