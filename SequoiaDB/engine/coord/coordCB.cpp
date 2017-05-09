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

   Source File Name = coordCB.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "coordCB.hpp"
#include "pmd.hpp"
#include "pmdController.hpp"
#include "pmdStartup.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

using namespace bson ;

namespace engine
{

   /*
   note: _CoordCB implement
   */
   _CoordCB::_CoordCB()
   {
      _pMsgHandler = NULL ;
      _pTimerHandler = NULL ;
      _pAgent = NULL ;
   }

   _CoordCB::~_CoordCB()
   {
   }

   coordResource* _CoordCB::getResource()
   {
      return &_resource ;
   }

   INT32 _CoordCB::init ()
   {
      INT32 rc = SDB_OK ;
      pmdOptionsCB *optCB = pmdGetOptionCB() ;

      // 1. create objs: netAgent and handler
      _pTimerHandler = SDB_OSS_NEW _coordTimerHandler() ;
      if ( !_pTimerHandler )
      {
         PD_LOG( PDERROR, "Failed to alloc memory for timer handler" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      _pMsgHandler = SDB_OSS_NEW _coordMsgHandler( &_remoteSessionMgr ) ;
      if ( !_pMsgHandler )
      {
         PD_LOG( PDERROR, "Failed to alloc memory for message handler" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      _pAgent = SDB_OSS_NEW _netRouteAgent( _pMsgHandler ) ;
      if ( !_pAgent )
      {
         PD_LOG( PDERROR, "Failed to alloc memory for net agent" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      _pAgent->getFrame()->setBeatInfo( optCB->getOprTimeout() ) ;

      // 2. init param
      rc = _resource.init( _pAgent, optCB ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init resource failed, rc: %d", rc ) ;
         goto error ;
      }

      _sitePropMgr.setPreferInsType( (INT32)optCB->preferedReplica() ) ;
      rc = _remoteSessionMgr.init( _pAgent, &_sitePropMgr ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init session manager failed, rc: %d", rc ) ;
         goto error ;
      }

      // set remote session manager to pmdController
      sdbGetPMDController()->setRSManager( &_remoteSessionMgr ) ;

      // 3. set startup ok
      pmdGetStartup().ok( TRUE ) ;
      // set nodeid and group name
      pmdGetKRCB()->setGroupName( COORD_GROUPNAME ) ;
      {
         MsgRouteID id ;
         id.value = MSG_INVALID_ROUTEID ;
         id.columns.groupID = COORD_GROUPID ;
         pmdSetNodeID( id ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _CoordCB::active ()
   {
      INT32 rc = SDB_OK ;
      pmdEDUMgr* pEDUMgr = pmdGetKRCB()->getEDUMgr() ;
      EDUID eduID = PMD_INVALID_EDUID ;

      // set to primary
      pmdSetPrimary( TRUE ) ;
      sdbGetPMDController()->registerNet( _pAgent->getFrame() ) ;

      // 1. start coord net work
      rc = pEDUMgr->startEDU ( EDU_TYPE_COORDNETWORK, (void*)_pAgent,
                               &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start coord network edu, rc: %d",
                   rc ) ;
      pEDUMgr->regSystemEDU ( EDU_TYPE_COORDNETWORK, eduID ) ;
      rc = pEDUMgr->waitUntil( eduID , PMD_EDU_RUNNING ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait CoordNet active failed, rc: %d", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _CoordCB::deactive ()
   {
      if ( _pAgent )
      {
         // 1. unreg net from controller
         sdbGetPMDController()->unregNet( _pAgent->getFrame() ) ;
         // 2. close listen
         _pAgent->closeListen() ;
         // 3. stop io
         _pAgent->stop() ;
      }

      return SDB_OK ;
   }

   INT32 _CoordCB::fini ()
   {
      _remoteSessionMgr.fini() ;
      _resource.fini() ;

      if ( _pAgent )
      {
         SDB_OSS_DEL _pAgent ;
         _pAgent = NULL ;
      }
      if ( _pMsgHandler )
      {
         SDB_OSS_DEL _pMsgHandler ;
         _pMsgHandler = NULL ;
      }
      if ( _pTimerHandler )
      {
         SDB_OSS_DEL _pTimerHandler ;
         _pTimerHandler = NULL ;
      }

      return SDB_OK ;
   }

   void _CoordCB::onConfigChange ()
   {
      if ( _pAgent )
      {
         UINT32 oprtimeout = pmdGetOptionCB()->getOprTimeout() ;
         _pAgent->getFrame()->setBeatInfo( oprtimeout ) ;
      }
   }

   void _CoordCB::attachCB( _pmdEDUCB *cb )
   {
      _remoteSessionMgr.registerEDU( cb ) ;
      _pMsgHandler->attach( cb ) ;
      _pTimerHandler->attach( cb ) ;
   }

   void _CoordCB::detachCB( _pmdEDUCB *cb )
   {
      _pMsgHandler->detach() ;
      _pTimerHandler->detach() ;
      _remoteSessionMgr.unregEUD( cb ) ;
   }

   UINT32 _CoordCB::setTimer( UINT32 milliSec )
   {
      UINT32 timeID = NET_INVALID_TIMER_ID ;
      if ( _pAgent )
      {
         _pAgent->addTimer( milliSec, _pTimerHandler, timeID ) ;
      }
      return timeID ;
   }

   void _CoordCB::killTimer( UINT32 timerID )
   {
      if ( _pAgent )
      {
         _pAgent->removeTimer( timerID ) ;
      }
   }

   void _CoordCB::onTimer( UINT64 timerID, UINT32 interval )
   {
   }

   /*
      get global coord cb
   */
   CoordCB* sdbGetCoordCB ()
   {
      static CoordCB s_coordCB ;
      return &s_coordCB ;
   }

}

