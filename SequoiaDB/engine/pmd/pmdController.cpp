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

   Source File Name = pmdController.cpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          02/05/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdController.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"

namespace engine
{
   _pmdController::_pmdController ()
   {
      _pTcpListener        = NULL ;
      _pHttpListener       = NULL ;
   }

   _pmdController::~_pmdController ()
   {
      _pTcpListener        = NULL ;
      _pHttpListener       = NULL ;
   }

   SDB_CB_TYPE _pmdController::cbType () const
   {
      return SDB_CB_PMDCTRL ;
   }

   const CHAR* _pmdController::cbName () const
   {
      return "PMDCONTROLLER" ;
   }

   INT32 _pmdController::init ()
   {
      INT32 rc = SDB_OK ;
      pmdOptionsCB *pOptCB = pmdGetOptionCB() ;
      UINT16 port = 0 ;

      // 1. create tcp listerner
      port = pOptCB->getServicePort() ;
      _pTcpListener = SDB_OSS_NEW ossSocket( port ) ;
      if ( !_pTcpListener )
      {
         PD_LOG( PDERROR, "Failed to alloc socket" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = _pTcpListener->initSocket() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to init tcp listener socket, rc: %d",
                   rc ) ;

      rc = _pTcpListener->bind_listen() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to bind tcp listener socket, rc: %d",
                   rc ) ;
      PD_LOG( PDEVENT, "Listerning on port[%d]", port ) ;

      // 2. create http listerner
      rc = ossGetPort( pOptCB->getRestService(), port ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get port by service name: %s, "
                   "rc: %d", pOptCB->getRestService(), rc ) ;
      _pHttpListener = SDB_OSS_NEW ossSocket( port ) ;
      if ( !_pHttpListener )
      {
         PD_LOG( PDERROR, "Failed to alloc socket" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = _pHttpListener->initSocket() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to init http listener socket, rc: %d",
                   rc ) ;
      rc = _pHttpListener->bind_listen() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to bin http listerner socket, rc: %d",
                   rc ) ;
      PD_LOG( PDEVENT, "Http Listerning on port[%d]", port ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdController::active ()
   {
      INT32 rc = SDB_OK ;
      pmdEDUMgr *pEDUMgr = pmdGetKRCB()->getEDUMgr() ;
      EDUID eduID = PMD_INVALID_EDUID ;

      // start time sync edu
      rc = pEDUMgr->startEDU( EDU_TYPE_SYNCCLOCK, NULL, &eduID ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_SYNCCLOCK, eduID ) ;

      // start tcp listern edu and http listerner edu
      rc = pEDUMgr->startEDU( EDU_TYPE_TCPLISTENER, (void*)_pTcpListener,
                              &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start tcp listerner, rc: %d",
                   rc ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_TCPLISTENER, eduID ) ;

      // wait until tcp listener starts
      rc = pEDUMgr->waitUntil ( eduID, PMD_EDU_RUNNING ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait Tcp Listerner active failed, rc: %d",
                   rc ) ;

      rc = pEDUMgr->startEDU( EDU_TYPE_HTTPLISTENER, (void*)_pHttpListener,
                              &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start http listerner, rc: %d",
                   rc ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_HTTPLISTENER, eduID ) ;

      // wait until http listener starts
      rc = pEDUMgr->waitUntil ( eduID, PMD_EDU_RUNNING ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait Http Listener active failed, rc: %d",
                   rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdController::deactive ()
   {
      return SDB_OK ;
   }

   INT32 _pmdController::fini ()
   {
      if ( _pTcpListener )
      {
         SDB_OSS_DEL _pTcpListener ;
         _pTcpListener = NULL ;
      }
      if ( _pHttpListener )
      {
         SDB_OSS_DEL _pHttpListener ;
         _pHttpListener = NULL ;
      }
      return SDB_OK ;
   }

   void _pmdController::registerCB( SDB_ROLE dbrole )
   {
      if ( SDB_ROLE_DATA == dbrole )
      {
         PMD_REGISTER_CB( sdbGetDPSCB() ) ;        // DPS
         PMD_REGISTER_CB( sdbGetTransCB() ) ;      // TRANS
         PMD_REGISTER_CB( sdbGetClsCB() ) ;        // CLS
         PMD_REGISTER_CB( sdbGetBPSCB() ) ;        // BPS
      }
      else if ( SDB_ROLE_COORD == dbrole )
      {
         PMD_REGISTER_CB( sdbGetTransCB() ) ;      // TRANS
         PMD_REGISTER_CB( sdbGetCoordCB() ) ;      // COORD
         PMD_REGISTER_CB( sdbGetFMPCB () ) ;       // FMP
      }
      else if ( SDB_ROLE_CATALOG == dbrole )
      {
         PMD_REGISTER_CB( sdbGetDPSCB() ) ;        // DPS
         PMD_REGISTER_CB( sdbGetTransCB() ) ;      // TRANS
         PMD_REGISTER_CB( sdbGetClsCB() ) ;        // CLS
         PMD_REGISTER_CB( sdbGetCatalogueCB() ) ;  // CATALOGUE
         PMD_REGISTER_CB( sdbGetBPSCB() ) ;        // BPS
         PMD_REGISTER_CB( sdbGetAuthCB() ) ;       // AUTH
      }
      else if ( SDB_ROLE_STANDALONE == dbrole )
      {
         PMD_REGISTER_CB( sdbGetDPSCB() ) ;        // DPS
         PMD_REGISTER_CB( sdbGetTransCB() ) ;      // TRANS
         PMD_REGISTER_CB( sdbGetBPSCB() ) ;        // BPS
      }

      PMD_REGISTER_CB( sdbGetDMSCB() ) ;           // DMS
      PMD_REGISTER_CB( sdbGetRTNCB() ) ;           // RTN
      PMD_REGISTER_CB( sdbGetSQLCB() ) ;           // SQL
      PMD_REGISTER_CB( sdbGetAggrCB() ) ;          // AGGR
      PMD_REGISTER_CB( sdbGetPMDController() ) ;   // CONTROLLER
   }

   /*
      get global pointer
   */
   pmdController* sdbGetPMDController()
   {
      static pmdController s_pmdctrl ;
      return &s_pmdctrl ;
   }

}


