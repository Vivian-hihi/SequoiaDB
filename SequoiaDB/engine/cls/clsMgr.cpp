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

   Source File Name = clsMgr.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          29/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "clsMgr.hpp"
#include "msgMessage.hpp"
#include "pmd.hpp"
#include "clsShardSession.hpp"
#include "clsReplSession.hpp"
#include "clsFSDstSession.hpp"
#include "clsFSSrcSession.hpp"
#include "../bson/bson.h"
#include "pdTrace.hpp"
#include "clsTrace.hpp"
#include "dpsOp2Record.hpp"
#include "pmdStartup.hpp"

using namespace bson ;

namespace engine
{

   //The max del session deque size
   #define MAX_SHD_SESSION_DEL_DEQ_SIZE            (1000)
   #define MAX_SHD_SESSION_CHECK_SIZE              (1000)

   #define CLS_WAIT_CB_ATTACH_TIMEOUT              ( 60 * OSS_ONE_SEC )

   BEGIN_OBJ_MSG_MAP( _clsMgr, _clsObjBase )
      ON_MSG ( MSG_CAT_REG_RES, _onCatRegisterRes )
      ON_MSG ( MSG_CAT_QUERY_TASK_RSP, _onCatQueryTaskRes )
      //ON_EVENT FUCTION MAP
   END_OBJ_MSG_MAP()

   _clsMgr::_clsMgr ()
   :_shdMsgHandlerObj ( this ),
    _replMsgHandlerObj ( this ),
    _shdTimerHandler ( this ),
    _replTimerHandler ( this ),
    _replNetRtAgent ( &_replMsgHandlerObj ),
    _shardNetRtAgent ( &_shdMsgHandlerObj ),
    _shdObj ( &_shardNetRtAgent ),
    _replObj ( &_replNetRtAgent ),
    _shardServiceID ( MSG_ROUTE_SHARD_SERVCIE ),
    _replServiceID ( MSG_ROUTE_REPL_SERVICE ),
    _force ( FALSE ),
    _taskMgr( 0x7FFFFFFF ),
    _taskID ( 0 ),
    _regTimerID ( CLS_INVALID_TIMERID ),
    _oneSecTimerID ( CLS_INVALID_TIMERID ),
    _repl1SecTimerID ( CLS_INVALID_TIMERID ),
    _shd1MinTimerID ( CLS_INVALID_TIMERID ),
    _shd1SecTimerID ( CLS_INVALID_TIMERID ),
    _shdHandleCloseTimerID ( CLS_INVALID_TIMERID ),
    _replHandleCloseTimerID ( CLS_INVALID_TIMERID )
   {
      _replServiceName[0] = 0 ;
      _shdServiceName[0]  = 0 ;
      _selfNodeID.value   = MSG_INVALID_ROUTEID ;
   }

   _clsMgr::~_clsMgr ()
   {
      SDB_ASSERT( _vecEventHandler.size() == 0, "Has some handler not unreg" ) ;
   }

   SDB_CB_TYPE _clsMgr::cbType () const
   {
      return SDB_CB_CLS ;
   }

   const CHAR* _clsMgr::cbName () const
   {
      return "CLSCB" ;
   }

   INT32 _clsMgr::init ()
   {
      INT32 rc = SDB_OK ;
      NodeID nodeID = _selfNodeID ;
      const CHAR* hostName = pmdGetKRCB()->getHostName() ;
      pmdOptionsCB *optCB = pmdGetOptionCB() ;

      // 1. init param
      ossStrncpy( _shdServiceName, optCB->shardService(),
                  OSS_MAX_SERVICENAME ) ;
      ossStrncpy( _replServiceName, optCB->replService(),
                  OSS_MAX_SERVICENAME ) ;

      // 2. init members
      rc = _memPool.initialize() ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to init memory pool" ) ;
         goto error ;
      }

      INIT_OBJ_GOTO_ERROR ( getShardCB() ) ;
      INIT_OBJ_GOTO_ERROR ( getReplCB() ) ;

      // 3. create listen socket
      nodeID.columns.serviceID = _replServiceID ;
      _replNetRtAgent.updateRoute( nodeID, hostName, _replServiceName ) ;
      rc = _replNetRtAgent.listen( nodeID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Create listen[Hostname:%s, ServiceName:%s] failed",
                  hostName, _replServiceName ) ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "Create replicate group listen[ServiceName:%s] succeed",
               _replServiceName ) ;

      nodeID.columns.serviceID = _shardServiceID ;
      _shardNetRtAgent.updateRoute( nodeID, hostName, _shdServiceName ) ;
      rc = _shardNetRtAgent.listen( nodeID ) ;
      if (SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Create listen[Hostname:%s, ServiceName:%s] failed",
                  hostName, _shdServiceName ) ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "Create sharding listen[ServiceName:%s] succeed",
               _shdServiceName ) ;

      // 4. set bussiness not ok( need wait register to change )
      pmdGetKRCB()->setBusinessOK( FALSE ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_ACTIVE, "_clsMgr::active" )
   INT32 _clsMgr::active ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR_ACTIVE ) ;

      // 1. start cls edu and shard edu
      _attachEvent.reset() ;
      rc = _startEDU ( EDU_TYPE_CLUSTER, PMD_EDU_UNKNOW,
                       (void*)this, TRUE ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _attachEvent.wait( CLS_WAIT_CB_ATTACH_TIMEOUT ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait cluster edu attach failed, rc: %d", rc ) ;

      _attachEvent.reset() ;
      rc = _startEDU ( EDU_TYPE_CLUSTERSHARD, PMD_EDU_UNKNOW,
                       (void*)this, TRUE ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _attachEvent.wait( CLS_WAIT_CB_ATTACH_TIMEOUT ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait cluster-shard attach failed, rc: %d",
                   rc ) ;

      // Start log notify
      rc = _startEDU( EDU_TYPE_CLSLOGNTY, PMD_EDU_UNKNOW,
                      (void*)getReplCB(), TRUE ) ;
      if ( rc )
      {
         goto error ;
      }

      // 2. start net edus
      rc = _startEDU ( EDU_TYPE_SHARDR, PMD_EDU_RUNNING,
                       (void*)getShardRouteAgent(), TRUE ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _startEDU ( EDU_TYPE_REPR, PMD_EDU_RUNNING,
                       (void*)getReplRouteAgent(), TRUE ) ;
      if ( rc )
      {
         goto error ;
      }

      // 3. set timer
      _repl1SecTimerID = setTimer ( CLS_REPL, OSS_ONE_SEC ) ;
      _shd1MinTimerID = setTimer ( CLS_SHARD, 60 * OSS_ONE_SEC ) ;
      _oneSecTimerID = setTimer ( CLS_REPL, OSS_ONE_SEC ) ;

      if ( CLS_INVALID_TIMERID == _repl1SecTimerID ||
           CLS_INVALID_TIMERID == _shd1MinTimerID ||
           CLS_INVALID_TIMERID == _oneSecTimerID )
      {
         PD_LOG ( PDERROR, "Register repl/shard/one seccond timer failed" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _regTimerID = setTimer( CLS_SHARD, OSS_ONE_SEC ) ;

      if ( CLS_INVALID_TIMERID == _regTimerID )
      {
         PD_LOG ( PDERROR, "Register timer failed" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // 4. send register msg
      _sendRegisterMsg () ;

   done:
      PD_TRACE_EXITRC ( SDB__CLSMGR_ACTIVE, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsMgr::deactive ()
   {
      // 1. stop listen
      _replNetRtAgent.closeListen() ;
      _shardNetRtAgent.closeListen() ;

      // 2. members to deactive
      _replObj.deactive() ;
      _shdObj.deactive() ;

      // 3. stop io
      _replNetRtAgent.stop() ;
      _shardNetRtAgent.stop() ;

      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_FINAL, "_clsMgr::fini" )
   INT32 _clsMgr::fini ()
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR_FINAL );
      _force = TRUE ;

      _shdObj.final () ;
      _replObj.final () ;

      //Clear sessions
      MAPSESSION_IT it = _mapShdSessions.begin () ;
      while ( it != _mapShdSessions.end() )
      {
         _releaseSession_i( it->second, TRUE, FALSE ) ;
         ++it ;
      }
      _mapShdSessions.clear () ;

      it = _mapReplSessions.begin () ;
      while ( it != _mapReplSessions.end() )
      {
         _releaseSession_i( it->second, TRUE, FALSE ) ;
         ++it ;
      }
      _mapReplSessions.clear () ;

      while ( _deqShdSessions.size () > 0 )
      {
         _releaseSession_i( _deqShdSessions.front (), TRUE, FALSE ) ;
         _deqShdSessions.pop_front () ;
      }

      while ( _deqShdDeletingSessions.size() > 0 )
      {
         _releaseSession_i ( _deqShdDeletingSessions.front(), FALSE, FALSE ) ;
         _deqShdDeletingSessions.pop_front() ;
      }

      //Clear latch
      MAPMETA_IT itMeta = _mapShdMeta.begin() ;
      while ( itMeta != _mapShdMeta.end() )
      {
         SDB_OSS_DEL itMeta->second ;
         ++itMeta ;
      }
      _mapShdMeta.clear() ;

      itMeta = _mapReplMeta.begin() ;
      while ( itMeta != _mapReplMeta.end() )
      {
         SDB_OSS_DEL itMeta->second ;
         ++itMeta ;
      }
      _mapReplMeta.clear() ;

      INT32 rc = _memPool.final () ;
      PD_TRACE_EXITRC ( SDB__CLSMGR_FINAL, rc );
      return rc ;
   }

   void _clsMgr::onConfigChange ()
   {
      _shdObj.onConfigChange() ;
      _replObj.onConfigChange() ;
   }

   void* _clsMgr::queryInterface( SDB_INTERFACE_TYPE type )
   {
      if ( SDB_IF_EVT_HOLDER == type )
      {
         return (void*)static_cast< IEventHolder* >( this ) ;
      }
      return IControlBlock::queryInterface( type ) ;
   }

   void _clsMgr::attachMainCB ( pmdEDUCB *pMainCB )
   {
      if ( EDU_TYPE_CLUSTER == pMainCB->getType() )
      {
         //Set MsgHandler EDU
         _shdMsgHandlerObj.attach ( pMainCB ) ;
         _replMsgHandlerObj.attach ( pMainCB ) ;

         //Set TimerHandler EDU
         _shdTimerHandler.attach ( pMainCB ) ;
         _replTimerHandler.attach ( pMainCB ) ;
      }
      else if ( EDU_TYPE_CLUSTERSHARD == pMainCB->getType() )
      {
         _shdMsgHandlerObj.attachShardCB( pMainCB ) ;
      }

      _attachEvent.signalAll() ;
   }

   void _clsMgr::detachMainCB( pmdEDUCB *pMainCB )
   {
      if ( EDU_TYPE_CLUSTER == pMainCB->getType() )
      {
         //Set MsgHandler EDU
         _shdMsgHandlerObj.detach() ;
         _replMsgHandlerObj.detach () ;

         //Set TimerHandler EDU
         _shdTimerHandler.detach () ;
         _replTimerHandler.detach () ;
      }
      else if ( EDU_TYPE_CLUSTERSHARD == pMainCB->getType() )
      {
         _shdMsgHandlerObj.detachShardCB() ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__STRATEDU, "_clsMgr::_startEDU" )
   INT32 _clsMgr::_startEDU ( INT32 type, EDU_STATUS waitStatus,
                              void *agrs, BOOLEAN regSys )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR__STRATEDU );
      EDUID eduID = PMD_INVALID_EDUID ;
      pmdKRCB *pKRCB = pmdGetKRCB () ;
      pmdEDUMgr *pEDUMgr = pKRCB->getEDUMgr () ;

      //Start EDU
      rc = pEDUMgr->startEDU( (EDU_TYPES)type, (void *)agrs, &eduID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to create EDU[type:%d(%s)], rc = %d",
                  type, getEDUName( (EDU_TYPES)type ), rc );
         goto error ;
      }

      //Resiter EDU Type
      if ( regSys )
      {
         pEDUMgr->regSystemEDU( (EDU_TYPES)type, eduID ) ;
      }

      //Wait edu running
      if ( PMD_EDU_UNKNOW != waitStatus )
      {
         rc = pEDUMgr->waitUntil( (EDU_TYPES)type, waitStatus ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to wait EDU[type:%d(%s)] to "
                    "status[%d(%s)], rc: %d", type,
                    getEDUName( (EDU_TYPES)type ), waitStatus,
                    getEDUStatusDesp( waitStatus ), rc ) ;
            goto error ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSMGR__STRATEDU, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__ONPRMCHG, "_clsMgr::ntyPrimaryChange" )
   void _clsMgr::ntyPrimaryChange( BOOLEAN primary,
                                   SDB_EVENT_OCCUR_TYPE type )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR__ONPRMCHG );

      if ( SDB_EVT_OCCUR_AFTER == type )
      {
         PD_LOG ( PDEVENT, "Node change to [%s]",
                  primary ? "Primary" : "Secondary" ) ;
      }

      // if business is not ok
      if ( !pmdGetStartup().isOK() )
      {
         return ;
      }

      if ( primary && SDB_EVT_OCCUR_BEFORE == type )
      {
         // inc dps log version
         sdbGetDPSCB()->incVersion() ;
      }
      else if ( !primary && SDB_EVT_OCCUR_BEFORE == type )
      {
         // interrupt writing edus
         pmdGetKRCB()->getEDUMgr()->interruptWritingEDUS() ;
      }

      // notify sub members
      getShardCB()->ntyPrimaryChange( primary, type ) ;
      getReplCB()->ntyPrimaryChange( primary, type ) ;

      if ( SDB_EVT_OCCUR_AFTER == type )
      {
         // if change to primary, need to start query task
         if ( primary )
         {
            BSONObj match = BSON ( CAT_TARGETID_NAME <<
                                   _selfNodeID.columns.groupID ) ;
            startTaskCheck( match ) ;
         }
         // if change to secondary, need to clean up all query task
         else
         {
            ossScopedLock lock ( &_clsLatch, EXCLUSIVE ) ;
            _mapTaskQuery.clear () ;
         }
      }

      // call other handler
      _callPrimaryChangeHandler( primary, type ) ;

      PD_TRACE_EXIT ( SDB__CLSMGR__ONPRMCHG );
   }

   const CHAR *_clsMgr::getShardServiceName () const
   {
      return _shdServiceName ;
   }
   const CHAR *_clsMgr::getReplServiceName () const
   {
      return _replServiceName ;
   }
   NodeID _clsMgr::getNodeID () const
   {
      return _selfNodeID ;
   }
   UINT16 _clsMgr::getShardServiceID () const
   {
      return _shardServiceID ;
   }
   UINT16 _clsMgr::getReplServiceID () const
   {
      return _replServiceID ;
   }

   _netRouteAgent *_clsMgr::getShardRouteAgent ()
   {
      return &_shardNetRtAgent ;
   }
   _netRouteAgent *_clsMgr::getReplRouteAgent ()
   {
      return &_replNetRtAgent ;
   }
   shardCB *_clsMgr::getShardCB ()
   {
      return &_shdObj ;
   }
   replCB *_clsMgr::getReplCB ()
   {
      return &_replObj ;
   }
   catAgent *_clsMgr::getCatAgent ()
   {
      return _shdObj.getCataAgent() ;
   }
   nodeMgrAgent* _clsMgr::getNodeMgrAgent ()
   {
      return _shdObj.getNodeMgrAgent() ;
   }
   _clsMsgHandler* _clsMgr::getShardMsgHandle()
   {
      return &_shdMsgHandlerObj ;
   }
   _clsTaskMgr* _clsMgr::getTaskMgr()
   {
      return &_taskMgr ;
   }
   BOOLEAN _clsMgr::isPrimary ()
   {
      return _replObj.primaryIsMe () ;
   }
   BOOLEAN _clsMgr::isFullSync ()
   {
      return _replObj.isFullSync() ;
   }
   INT32 _clsMgr::clearAllData ()
   {
      return _shdObj.clearAllData () ;
   }

   void _clsMgr::pushMsgHandle ( void * msg, NET_HANDLE handle )
   {
      _msgAssister.pushMsgHandle( msg, handle ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_INVDATACAT, "_clsMgr::invalidateCata" )
   INT32 _clsMgr::invalidateCata( const CHAR * name )
   {
      INT32 rc = SDB_CLS_NOT_PRIMARY ;
      PD_TRACE_ENTRY ( SDB__CLSMGR_INVDATACAT );

      if ( isPrimary() )
      {
         /// write sync cata info log
         SDB_DPSCB *dpsCB = pmdGetKRCB()->getDPSCB() ;
         dpsMergeInfo info ;
         info.setInfoEx( ~0, ~0, DMS_INVALID_EXTENT, NULL ) ;
         dpsLogRecord &record = info.getMergeBlock().record() ;
         rc = dpsInvalidCata2Record( name, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build invalid-cata log:%d",rc ) ;
            goto error ;
         }
         rc = dpsCB->prepare(info ) ;
         if ( SDB_OK == rc )
         {
            dpsCB->writeData( info ) ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSMGR_INVDATACAT, rc );
      return rc ;
   error:
      goto done ;
   }

   NET_HANDLE _clsMgr::peekMsgHandle ( void * msg )
   {
      return _msgAssister.peekMsgHandle( msg ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_STARTINSN, "_clsMgr::startInnerSession" )
   INT32 _clsMgr::startInnerSession ( INT32 type, INT32 innerTID, void *data )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR_STARTINSN );
      ossScopedLock lock ( &_clsLatch, EXCLUSIVE ) ;

      _innerSessionInfo info ;
      info.type = type ;
      info.startType = CLS_SESSION_ACTIVE ;
      info.innerTid = innerTID ;
      info.data = data ;
      info.sessionID = ossPack32To64 ( _selfNodeID.columns.nodeID, innerTID ) ;

      _vecInnerSessionParam.push_back ( info ) ;

      PD_TRACE_EXIT ( SDB__CLSMGR_STARTINSN );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_STARTTSKCHK, "_clsMgr::startTaskCheck" )
   INT32 _clsMgr::startTaskCheck ( const BSONObj & match )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR_STARTTSKCHK );
      if ( !isPrimary() )
      {
         rc = SDB_CLS_NOT_PRIMARY ;
      }
      else
      {
         ossScopedLock lock ( &_clsLatch, EXCLUSIVE ) ;
         _mapTaskQuery[++_taskID] = match.copy() ;
      }
      PD_TRACE_EXIT ( SDB__CLSMGR_STARTTSKCHK );
      return rc ;
   }

   INT32 _clsMgr::stopTask( UINT64 taskID )
   {
      ossScopedLock lock ( &_clsLatch, SHARED ) ;
      map< UINT64, UINT64 >::iterator it = _mapTaskID.find( taskID ) ;
      if ( it != _mapTaskID.end() )
      {
         _taskMgr.stopTask( it->second ) ;
      }
      return SDB_OK ;
   }

   INT32 _clsMgr::removeTask( UINT64 taskID )
   {
      ossScopedLock lock ( &_clsLatch, EXCLUSIVE ) ;
      map< UINT64, UINT64 >::iterator it = _mapTaskID.find( taskID ) ;
      if ( it != _mapTaskID.end() )
      {
         _mapTaskID.erase( it ) ;
      }
      return SDB_OK ;
   }

   INT32 _clsMgr::regEventHandler( IEventHander * pHandler )
   {
      if ( !pHandler ) return SDB_INVALIDARG ;

      ossScopedLock lock ( &_handlerLatch, EXCLUSIVE ) ;
      for ( UINT32 i = 0 ; i < _vecEventHandler.size() ; ++i )
      {
         if ( _vecEventHandler[ i ] == pHandler )
         {
            return SDB_SYS ;
         }
      }
      _vecEventHandler.push_back( pHandler ) ;
      return SDB_OK ;
   }

   void _clsMgr::unregEventHandler( IEventHander * pHandler )
   {
      if ( !pHandler ) return ;

      ossScopedLock lock ( &_handlerLatch, EXCLUSIVE ) ;
      VEC_EVENTHANDLER::iterator it ;
      for ( it = _vecEventHandler.begin() ;
            it != _vecEventHandler.end() ;
            ++it )
      {
         if ( *it == pHandler )
         {
            _vecEventHandler.erase( it ) ;
            break ;
         }
      }
   }

   void _clsMgr::_callRegisterEventHandler()
   {
      IEventHander *pHandler = NULL ;
      ossScopedLock lock ( &_handlerLatch, SHARED ) ;
      for ( UINT32 i = 0 ; i < _vecEventHandler.size() ; ++i )
      {
         pHandler = _vecEventHandler[ i ] ;
         if ( pHandler->getMask() & EVENT_MASK_ON_REGISTERED )
         {
            pHandler->onRegistered( _selfNodeID ) ;
         }
      }
   }

   void _clsMgr::_callPrimaryChangeHandler( BOOLEAN primary,
                                            SDB_EVENT_OCCUR_TYPE type )
   {
      IEventHander *pHandler = NULL ;
      ossScopedLock lock ( &_handlerLatch, SHARED ) ;
      for ( UINT32 i = 0 ; i < _vecEventHandler.size() ; ++i )
      {
         pHandler = _vecEventHandler[ i ] ;
         if ( pHandler->getMask() & EVENT_MASK_ON_PRIMARYCHG )
         {
            pHandler->onPrimaryChange( primary, type ) ;
         }
      }
   }

   // note we do not need to request latch because getSession function is only
   // called in one thread, thus it's not possible for multiple threads calling
   // the same thread and ruin the map
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_GETSN, "_clsMgr::getSession" )
   _clsSession *_clsMgr::getSession( INT32 type, UINT64 sessionID,
                                     INT32 startType,
                                     const NET_HANDLE handle,
                                     BOOLEAN bCreate, void *data )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR_GETSN );
      _clsSession * pSession    = NULL ;
      MAPSESSION  * pMapSession = _getSessionMap( type ) ;

      //Find the session, if exist return it
      MAPSESSION_IT it = pMapSession->find ( sessionID ) ;
      if ( it != pMapSession->end () )
      {
         pSession = it->second ;

         // need to attach meta
         if ( !pSession->getMeta() && pSession->canAttachMeta() &&
              NET_INVALID_HANDLE != handle )
         {
            _attachSessionMeta( type, pSession, handle ) ;
         }
         goto done ;
      }

      if ( !bCreate )
      {
         goto done ;
      }

      // If shard session when the del queue is not empty, can reused
      if ( CLS_SHARD == type && CLS_NODEID( sessionID ) > CLS_BASE_HANDLE_ID &&
           _deqShdSessions.size () > 0 )
      {
         pSession = _deqShdSessions.front () ;
         _deqShdSessions.pop_front () ;
      }
      else
      {
         SDB_ASSERT ( CLS_REPL == type || CLS_SHARD == type,
                      "Invalid type" ) ;
         pSession = _createSession ( type, startType, sessionID, data ) ;
         if ( NULL == pSession )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for session" ) ;
            goto error ;
         }
      }

      //Set session information
      (*pMapSession)[sessionID] = pSession ;
      pSession->startType ( startType ) ;
      pSession->sessionID ( sessionID ) ;

      PD_LOG ( PDEVENT, "Create session[Name: %s, StartType: %d]",
               pSession->sessionName(), startType ) ;

      // attach meta
      if ( !pSession->getMeta() && pSession->canAttachMeta() &&
           NET_INVALID_HANDLE != handle )
      {
         rc = _attachSessionMeta( type, pSession, handle ) ;
         if ( rc )
         {
            goto error ;
         }
      }

      //Start session EDU
      rc = _startSessionEDU( pSession ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start session EDU, rc = %d", rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXIT ( SDB__CLSMGR_GETSN );
      return pSession ;
   error:
      if ( pSession )
      {
         _releaseSession ( pSession ) ;
         pSession = NULL ;
      }
      goto done ;
   }

   INT32 _clsMgr::_attachSessionMeta( INT32 type, _clsSession * pSession,
                                      const NET_HANDLE handle )
   {
      INT32 rc = SDB_OK ;
      MAPMETA *pMapMeta = _getMetaMap( type ) ;
      clsSessionMeta * pMeta = NULL ;
      MAPMETA_IT itMeta = pMapMeta->find ( handle ) ;
      if ( itMeta == pMapMeta->end() )
      {
         pMeta = SDB_OSS_NEW clsSessionMeta ( handle ) ;
         if ( NULL == pMeta )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for meta" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         (*pMapMeta)[handle] = pMeta ;
      }
      else
      {
         pMeta = itMeta->second ;
      }
      pMeta->incBaseHandleNum() ;
      pSession->meta ( pMeta ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_ASGNMEM, "_clsMgr::assignMemory" )
   INT32 _clsMgr::assignMemory( _clsSession * pSession, UINT32 msgLength )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR_ASGNMEM );
      CHAR *pNewBuff = NULL ;
      UINT32 buffSize = 0 ;

      clsBuffInfo * pBuffInfo = pSession->frontBuffer () ;
      while ( pBuffInfo && pBuffInfo->isFree() )
      {
         if ( !pNewBuff && pBuffInfo->size >= msgLength )
         {
            pNewBuff = pBuffInfo->pBuffer ;
            buffSize = pBuffInfo->size ;
         }
         else //release memory to pool
         {
            _memPool.release( pBuffInfo->pBuffer, pBuffInfo->size ) ;
         }
         pSession->popBuffer () ;
         pBuffInfo = pSession->frontBuffer () ;
      }

      if ( !pNewBuff )
      {
         pNewBuff = _memPool.alloc ( msgLength, buffSize ) ;
         if ( !pNewBuff )
         {
            PD_LOG ( PDERROR, "Memory pool assign memory failed[size:%d]",
               msgLength ) ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

      rc = pSession->pushBuffer ( pNewBuff, buffSize ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "push buffer failed in session[%s, rc:%d]", 
            pSession->sessionName(), rc ) ;
         _memPool.release ( pNewBuff, buffSize ) ;
         SDB_ASSERT ( 0, "why the buffer is full??? check" ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC (SDB__CLSMGR_ASGNMEM, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__CRTSN, "_clsMgr::_createSession" )
   _clsSession *_clsMgr::_createSession( INT32 type, INT32 startType,
                                         UINT64 sessionID, void *data )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR__CRTSN );
      _clsSession *p = NULL ;
      UINT32 nodeID = 0 ;
      UINT32 tid = 0 ;
      ossUnpack32From64 ( sessionID, nodeID, tid ) ;

      if ( CLS_REPL == type )
      {
         if ( CLS_TID_REPL_SYC == tid ) //repl sync session
         {
            p = SDB_OSS_NEW _clsReplSession ( sessionID ) ;
            goto done ;
         }
         else if ( CLS_TID_REPL_FS_SYC == tid ) //repl full sync session
         {
            if ( CLS_SESSION_ACTIVE == startType )
            {
               p = SDB_OSS_NEW _clsFSDstSession ( sessionID,
                                                  &_replNetRtAgent ) ;
               goto done ;
            }
            else
            {
               p = SDB_OSS_NEW _clsFSSrcSession ( sessionID,
                                                  &_replNetRtAgent ) ;
               goto done ;
            }
         }
         else
         {
            PD_LOG ( PDERROR, "inner TID[%d] and nodeID[%d] error",
                     tid, nodeID ) ;
         }
      }
      else if ( CLS_SHARD == type )
      {
         if ( CLS_BASE_HANDLE_ID >= nodeID )
         {
            if ( CLS_SESSION_ACTIVE == startType )
            {
               p = SDB_OSS_NEW _clsSplitDstSession ( sessionID,
                                                     &_shardNetRtAgent,
                                                     data ) ;
               goto done ;
            }
            else
            {
               p = SDB_OSS_NEW _clsSplitSrcSession ( sessionID,
                                                     &_shardNetRtAgent ) ;
               goto done ;
            }
         }
         else
         {
            p = SDB_OSS_NEW _clsShdSession ( sessionID ) ;
            goto done ;
         }
      }
      else
      {
         PD_LOG ( PDERROR, "The session type is error[type:%d]", type ) ;
      }
done :
      PD_TRACE_EXIT ( SDB__CLSMGR__CRTSN );
      return p ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__STARTSNEDU, "_clsMgr::_startSessionEDU" )
   INT32 _clsMgr::_startSessionEDU( _clsSession * pSession )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__CLSMGR__STARTSNEDU );
      pmdKRCB *pKRCB = pmdGetKRCB() ;
      pmdEDUMgr *pEDUMgr = pKRCB->getEDUMgr() ;
      EDUID eduID = PMD_INVALID_EDUID ;
      pmdEDUCB *cb = NULL ;

      rc = pEDUMgr->startEDU( pSession->eduType(), (void *)pSession, &eduID ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_QUIESCED == rc )
         {
            PD_LOG ( PDWARNING,
                     "Reject new connection due to quiesced database" ) ;
         }
         else
         {
            PD_LOG ( PDERROR,
                     "Failed to create subagent thread, rc = %d", rc ) ;
         }
         goto error ;
      }

      pSession->eduID( eduID ) ;
      cb = pEDUMgr->getEDUByID( eduID );
      SDB_ASSERT ( NULL != cb, "EDU CB cannot be NULL" ) ;
      pSession->eduCB( cb ) ;
      //Wait the EDUCB is in the session
      pSession->waitAttach () ;

   done:
      PD_TRACE_EXITRC ( SDB__CLSMGR__STARTSNEDU, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__RLSSN, "_clsMgr::_releaseSession" )
   INT32 _clsMgr::_releaseSession( _clsSession * pSession, BOOLEAN delay )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR__RLSSN );
      if ( !_force )
      {
         MAPSESSION * pMapSession = _getSessionMap ( pSession->type() );
         MAPSESSION_IT it = pMapSession->find( pSession->sessionID() ) ;
         if ( it != pMapSession->end() )
         {
            pMapSession->erase( it ) ;
         }
      }

      INT32 rc = _releaseSession_i ( pSession, TRUE, delay ) ;
      PD_TRACE_EXITRC ( SDB__CLSMGR__RLSSN, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__RLSSN_I, "_clsMgr::_releaseSession_i" )
   INT32 _clsMgr::_releaseSession_i ( _clsSession *pSession, BOOLEAN postQuit,
                                      BOOLEAN delay )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR__RLSSN_I );
      clsBuffInfo *pBuffInfo = NULL ;

      SDB_ASSERT ( pSession, "pSession can't be NULL" ) ;
      if ( !_force && postQuit && pSession->eduCB() )
      {
         //Notify the edu quit
         pSession->eduCB()->disconnect () ;
      }

      if ( delay )
      {
         ossScopedLock lock ( &_deqDeletingMutex ) ;
         _deqShdDeletingSessions.push_back ( pSession ) ;
         goto done ;
      }

      //Wait the EDUCB is out the session
      pSession->waitDetach () ;

      // dec based handle number
      if ( pSession->getMeta() )
      {
         pSession->getMeta()->decBaseHandleNum() ;
      }

      //Release Memory to pool
      pBuffInfo = pSession->frontBuffer ();
      while ( pBuffInfo )
      {
         _memPool.release ( pBuffInfo->pBuffer, pBuffInfo->size ) ;
         pSession->popBuffer () ;
         pBuffInfo = pSession->frontBuffer ();
      }
      pSession->clear() ;

      if ( !_force )
      {
         //If the deque size have overload the MAX_SESSION_DEL_DEQ_SIZE,
         //will destory
         if ( CLS_SHARD == pSession->type () &&
              CLS_NODEID( pSession->sessionID() ) > CLS_BASE_HANDLE_ID &&
              _deqShdSessions.size() < MAX_SHD_SESSION_DEL_DEQ_SIZE )
         {
            _deqShdSessions.push_back( pSession ) ;
            goto done ;
         }
      }

      SDB_OSS_DEL pSession ;

   done:
      PD_TRACE_EXIT ( SDB__CLSMGR__RLSSN_I );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__DFTMSGFUNC, "_clsMgr::_defaultMsgFunc" )
   INT32 _clsMgr::_defaultMsgFunc( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR__DFTMSGFUNC );
      // the msg is not mine, dispatch to sub object
      // restore the type
      INT32 type = (INT32) msg->TID ;
      UINT32 opCode = (UINT32)(msg->opCode) ;
      msg->TID = 0 ;

      if ( CLS_REPL == type || MSG_CAT_GRP_RES == opCode
         || MSG_CAT_PAIMARY_CHANGE_RES == opCode
         || MSG_CLS_GINFO_UPDATED == opCode )
      {
         rc = _replObj.dispatchMsg( handle, msg ) ;
      }
      else
      {
         rc = _shdObj.dispatchMsg ( handle, msg ) ;
      }
      PD_TRACE_EXITRC ( SDB__CLSMGR__DFTMSGFUNC, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_ONTMR, "_clsMgr::onTimer" )
   void _clsMgr::onTimer ( UINT64 timerID, UINT32 interval )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR_ONTMR );
      //Judge the timer is myself, if not my self will dispatch to sub object
      if ( timerID == _regTimerID )
      {
         _sendRegisterMsg () ;
      }
      else if ( timerID == _oneSecTimerID )
      {
         //Check _deqShdDeletingSessions
         {
            ossScopedLock lock ( &_deqDeletingMutex ) ;
            _clsSession *pSession = NULL ;
            DEQSESSION::iterator it = _deqShdDeletingSessions.begin() ;
            while ( it != _deqShdDeletingSessions.end() )
            {
               pSession = *it ;
               if ( !pSession->isDetached() )
               {
                  ++it ;
                  continue ;
               }
               it = _deqShdDeletingSessions.erase( it ) ;
               _releaseSession_i( pSession, FALSE, FALSE ) ;
            }
         }

         //prepare task
         _prepareTask () ;

         if ( _taskMgr.taskCount() > 0 &&
              CLS_INVALID_TIMERID == _shd1SecTimerID )
         {
            _shd1SecTimerID = setTimer ( CLS_SHARD, OSS_ONE_SEC ) ;
         }
         else if ( _shd1SecTimerID != CLS_INVALID_TIMERID &&
                   0 == _taskMgr.taskCount() )
         {
            killTimer ( _shd1SecTimerID ) ;
            _shd1SecTimerID = CLS_INVALID_TIMERID ;
         }
      }
      else
      {
         UINT32 type = 0 ;
         UINT32 netTimerID = 0 ;
         ossUnpack32From64 ( timerID, type, netTimerID ) ;
         _clsObjBase *pSubObj = &_shdObj ;
         if ( CLS_REPL == (INT32)type )
         {
            pSubObj = &_replObj ;
         }

         pSubObj->onTimer ( timerID, interval ) ;
      }
      PD_TRACE_EXIT ( SDB__CLSMGR_ONTMR );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_HNDSNTMOUT, "_clsMgr::handleSessionTimeout" )
   INT32 _clsMgr::handleSessionTimeout ( UINT64 timerID , UINT32 interval )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR_HNDSNTMOUT );
      UINT32 type = 0 ;
      UINT32 netTimerID = 0 ;
      BOOLEAN judgeNodeID = FALSE ;
      ossUnpack32From64 ( timerID, type, netTimerID ) ;

      MAPSESSION *pMapSession = NULL ;
      if ( _repl1SecTimerID == timerID )
      {
         pMapSession = &_mapReplSessions ;
      }
      else if ( _shd1MinTimerID == timerID )
      {
         if ( _mapShdSessions.size() > MAX_SHD_SESSION_CHECK_SIZE )
         {
            pMapSession = &_mapShdSessions ;
         }
         else
         {
            goto done ;
         }
      }
      else if ( _shd1SecTimerID == timerID )
      {
         pMapSession = &_mapShdSessions ;
         judgeNodeID = TRUE ;
      }
      else if ( _shdHandleCloseTimerID == timerID )
      {
         _checkSessionMeta( type ) ;
         killTimer( _shdHandleCloseTimerID ) ;
         _shdHandleCloseTimerID = CLS_INVALID_TIMERID ;
         goto done ;
      }
      else if ( _replHandleCloseTimerID == timerID )
      {
         _checkSessionMeta( type ) ;
         killTimer( _replHandleCloseTimerID ) ;
         _replHandleCloseTimerID = CLS_INVALID_TIMERID ;
         goto done ;
      }

      if ( pMapSession )
      {
         _clsSession *pSession = NULL ;
         MAPSESSION_IT it = pMapSession->begin() ;
         while ( it != pMapSession->end() )
         {
            pSession = it->second ;
            if ( judgeNodeID )
            {
               if ( CLS_NODEID( pSession->sessionID() ) > CLS_BASE_HANDLE_ID )
               {
                  ++it ;
                  continue ;
               }
            }

            if ( !pSession->isProcess() && pSession->timeout( interval ) )
            {
               PD_LOG ( PDEVENT, "Session[%s] timeout",
                        pSession->sessionName() ) ;
               _releaseSession_i ( pSession, TRUE, TRUE ) ;
               pMapSession->erase ( it++ ) ;
               continue ;
            }
            ++it ;
         }

         //start inner sessions
         _startInnerSession ( type ) ;

         //must return SDB_OK(0), otherwise the timer will dispatch to clsMgr
         goto done ;
      }

      //return not zero, the timer will dispath to clsMgr
      rc = -1 ;
   done :
      PD_TRACE_EXITRC ( SDB__CLSMGR_HNDSNTMOUT, rc );
      return rc ;
   }

   _clsMgr::MAPSESSION *_clsMgr::_getSessionMap ( INT32 type )
   {
      return ( CLS_REPL == type ) ? &_mapReplSessions : &_mapShdSessions ;
   }

   _clsMgr::MAPMETA* _clsMgr::_getMetaMap( INT32 type )
   {
      return ( CLS_REPL == type ) ? &_mapReplMeta : &_mapShdMeta ;
   }

   void _clsMgr::_checkSessionMeta( INT32 type )
   {
      MAPMETA *pMapMeta = _getMetaMap( type ) ;
      MAPMETA_IT it = pMapMeta->begin() ;
      while ( it != pMapMeta->end() )
      {
         clsSessionMeta *pMeta = it->second ;
         if ( 0 == pMeta->getBasedHandleNum() )
         {
            SDB_OSS_DEL pMeta ;
            pMapMeta->erase( it++ ) ;
            continue ;
         }
         ++it ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_HNDSNCLOSE, "_clsMgr::handleSessionClose" )
   INT32 _clsMgr::handleSessionClose ( INT32 type, const NET_HANDLE handle )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR_HNDSNCLOSE );
      _clsSession *pSession = NULL ;
      MAPSESSION *pMapSession = _getSessionMap ( type ) ;
      MAPSESSION_IT it = pMapSession->begin() ;
      while ( it != pMapSession->end() )
      {
         pSession = it->second ;
         if ( pSession->netHandle () == handle )
         {
            PD_LOG ( PDEVENT, "Session[%s, handle:%d] closed",
                     pSession->sessionName(), pSession->netHandle() ) ;
            _releaseSession_i ( pSession, TRUE, TRUE ) ;
            pMapSession->erase ( it++ ) ;
            continue ;
         }
         ++it ;
      }

      if ( CLS_REPL == type && CLS_INVALID_TIMERID == _replHandleCloseTimerID )
      {
         _replHandleCloseTimerID = setTimer( CLS_REPL, 30 * OSS_ONE_SEC ) ;
      }
      else if ( CLS_SHARD == type &&
                CLS_INVALID_TIMERID == _shdHandleCloseTimerID )
      {
         _shdHandleCloseTimerID = setTimer( CLS_SHARD, 30 * OSS_ONE_SEC ) ;
      }

      PD_TRACE_EXIT ( SDB__CLSMGR_HNDSNCLOSE );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__STARTINSN, "_clsMgr::_startInnerSession" )
   INT32 _clsMgr::_startInnerSession ( INT32 type )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR__STARTINSN );
      _clsSession *pSession = NULL ;
      ossScopedLock lock ( &_clsLatch, EXCLUSIVE ) ;

      VECINNERPARAM::iterator it = _vecInnerSessionParam.begin() ;
      while ( it != _vecInnerSessionParam.end() )
      {
         _innerSessionInfo &info = *it ;
         if ( info.type != type ||
              getSession( info.type, info.sessionID, info.startType,
                          NET_INVALID_HANDLE, FALSE ) )
         {
            ++it ;
            continue ;
         }

         pSession = getSession ( info.type, info.sessionID, info.startType,
                                 NET_INVALID_HANDLE, TRUE, info.data ) ;
         if ( pSession )
         {
            PD_LOG ( PDEVENT, "Create inner session[%s] succeed",
                     pSession->sessionName() ) ;
            it = _vecInnerSessionParam.erase ( it ) ;
            continue ;
         }

         PD_LOG ( PDERROR, "Create inner session[TID:%d] failed",
            info.innerTid ) ;
         rc = SDB_SYS ;

         ++it ;
      }

      PD_TRACE_EXITRC ( SDB__CLSMGR__STARTINSN, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__PREPTASK, "_clsMgr::_prepareTask" )
   INT32 _clsMgr::_prepareTask ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR__PREPTASK );
      ossScopedLock lock ( &_clsLatch, SHARED ) ;
      MAPTASKQUERY::iterator it = _mapTaskQuery.begin () ;
      while ( it != _mapTaskQuery.end() )
      {
         // send query msg to catalog
         rc = _sendQueryTaskReq ( it->first, "CAT", &(it->second) ) ;
         if ( SDB_OK != rc )
         {
            break ;
         }
         ++it ;
      }
      PD_TRACE_EXITRC ( SDB__CLSMGR__PREPTASK, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__ADDTSKINSN, "_clsMgr::_addTaskInnerSession" )
   INT32 _clsMgr::_addTaskInnerSession ( const CHAR * objdata )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR__ADDTSKINSN );
      INT32 jobType = CLS_TASK_UNKNOW ;
      _clsSplitTask *pTask = NULL ;
      UINT32 tid = 0 ;
      INT32 type = CLS_SHARD ;
      UINT64 taskID = CLS_INVALID_TASKID ;

      try
      {
         BSONObj resultObj ( objdata ) ;
         BSONElement ele = resultObj.getField( CAT_TASKTYPE_NAME ) ;
         PD_CHECK ( ele.type() == NumberInt, SDB_INVALIDARG, error, PDERROR,
                    "Field[%s] invalid in task[%s]", CAT_TASKTYPE_NAME,
                    resultObj.toString().c_str() ) ;
         jobType = ele.numberInt () ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "addTaskInnerSession exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      switch ( jobType )
      {
         case CLS_TASK_SPLIT :
            taskID = _taskMgr.getTaskID() ;
            pTask = SDB_OSS_NEW _clsSplitTask ( taskID ) ;
            type = CLS_SHARD ;
            break ;
         default :
            PD_LOG ( PDERROR, "Unknow job type[%d]", jobType ) ;
            rc = SDB_INVALIDARG ;
            break ;
      }

      if ( SDB_OK == rc && !pTask )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for task[type:%d]",
                  jobType ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      else if ( !pTask )
      {
         goto error ;
      }

      rc = pTask->init( objdata ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Init split task failed[rc:%d]", rc ) ;
         goto error ;
      }

      //add to taskMgr, the task will delete in taskMgr whether suc or failed
      rc = _taskMgr.addTask( pTask, taskID ) ;
      if ( SDB_OK != rc )
      {
         pTask = NULL ;
         goto error ;
      }

      _clsLatch.get() ;
      _mapTaskID[ pTask->taskID() ] = taskID ;
      _clsLatch.release() ;

      //start inner session
      tid = (UINT32)taskID ;
      rc = startInnerSession ( type, tid, (void *)pTask ) ;

   done:
      PD_TRACE_EXITRC ( SDB__CLSMGR__ADDTSKINSN, rc );
      return rc ;
   error:
      if ( pTask )
      {
         SDB_OSS_DEL pTask ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_SETTMR, "_clsMgr::setTimer" )
   UINT64 _clsMgr::setTimer ( CLS_MEMBER_TYPE type, UINT32 milliSec )
   {
      UINT64 rc;
      PD_TRACE_ENTRY ( SDB__CLSMGR_SETTMR );
      UINT32 timeID = 0 ;
      _netTimeoutHandler * pHandler = &_shdTimerHandler ;
      _netRouteAgent * pRtAgent = &_shardNetRtAgent ;

      if ( CLS_REPL == type )
      {
         pHandler = &_replTimerHandler ;
         pRtAgent = &_replNetRtAgent ;
      }

      if ( pRtAgent->addTimer( milliSec, pHandler, timeID ) == SDB_OK )
      {
         rc = ossPack32To64( (UINT32)type, timeID ) ;
      }
      else
      {
         rc = CLS_INVALID_TIMERID ;
      }
      PD_TRACE1( SDB__CLSMGR_SETTMR, PD_PACK_ULONG(rc) );
      PD_TRACE_EXIT ( SDB__CLSMGR_SETTMR );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR_KILLTMR, "_clsMgr::killTimer" )
   void _clsMgr::killTimer( UINT64 timerID )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR_KILLTMR );
      UINT32 type = 0 ;
      UINT32 netTimerID = 0 ;

      ossUnpack32From64 ( timerID, type, netTimerID ) ;

      _netRouteAgent * pRtAgent = &_shardNetRtAgent ;

      if ( CLS_REPL == (INT32)type )
      {
         pRtAgent = &_replNetRtAgent ;
      }

      pRtAgent->removeTimer( netTimerID ) ;
      PD_TRACE_EXIT ( SDB__CLSMGR_KILLTMR );
   }

   INT32 _clsMgr::sendToCatlog ( MsgHeader * msg )
   {
      return _shdObj.sendToCatlog ( msg ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__SNDREGMSG, "_clsMgr::_sendRegisterMsg" )
   INT32 _clsMgr::_sendRegisterMsg ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR__SNDREGMSG );
      pmdKRCB *pKRCB = pmdGetKRCB () ;
      BSONObjBuilder bsonBuilder ;
      const CHAR* hostName = pmdGetKRCB()->getHostName() ;

      bsonBuilder.append ( CAT_TYPE_FIELD_NAME,  (INT32)(pKRCB->getDBRole()) ) ;
      bsonBuilder.append ( CAT_HOST_FIELD_NAME, hostName ) ;

      BSONArrayBuilder arrayBuilder ;
      BSONObjBuilder subBuilderRepl, subBuilderShd ;

      subBuilderRepl.append ( CAT_SERVICE_TYPE_FIELD_NAME ,
                              (INT32)_replServiceID ) ;
      subBuilderRepl.append ( CAT_SERVICE_NAME_FIELD_NAME,
                              _replServiceName ) ;
      arrayBuilder.append( subBuilderRepl.obj() ) ;

      subBuilderShd.append ( CAT_SERVICE_TYPE_FIELD_NAME ,
                             (INT32)_shardServiceID) ;
      subBuilderShd.append ( CAT_SERVICE_NAME_FIELD_NAME,
                             _shdServiceName) ;
      arrayBuilder.append( subBuilderShd.obj() ) ;

      bsonBuilder.appendArray ( CAT_SERVICE_FIELD_NAME, arrayBuilder.arr() ) ;

      BSONObj regObj = bsonBuilder.obj () ;
      UINT32 length = regObj.objsize () + sizeof ( MsgCatRegisterReq ) ;
      // free by end of the function
      CHAR * buff = (CHAR *)SDB_OSS_MALLOC ( length ) ;
      MsgCatRegisterReq *pReq = NULL ;

      if ( buff == NULL )
      {
         PD_LOG ( PDERROR, "Failed to allocate memroy for register req" ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      pReq = (MsgCatRegisterReq*)buff ;
      pReq->header.messageLength = length ;
      pReq->header.opCode = MSG_CAT_REG_REQ ;
      pReq->header.requestID = 0 ;
      pReq->header.TID = 0 ;
      pReq->header.routeID.value = 0 ;
      ossMemcpy( pReq->data, regObj.objdata(), regObj.objsize() ) ;

      rc = sendToCatlog( (MsgHeader *) pReq ) ;
      PD_LOG ( PDDEBUG, "Send node register[rc: %d]", rc ) ;

   done:
      if ( buff )
      {
         SDB_OSS_FREE ( buff ) ;
         buff = NULL ;
      }
      PD_TRACE_EXITRC ( SDB__CLSMGR__SNDREGMSG, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__SNDQTSKREQ, "_clsMgr::_sendQueryTaskReq" )
   INT32 _clsMgr::_sendQueryTaskReq ( UINT64 requestID, const CHAR * clFullName,
                                      const BSONObj* match )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR__SNDQTSKREQ );
      CHAR *pBuff = NULL ;
      INT32 buffSize = 0 ;
      MsgHeader *msg = NULL ;
      INT32 rc = SDB_OK ;

      rc = msgBuildQueryMsg ( &pBuff, &buffSize, clFullName, 0, requestID,
                              0, -1, match, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
      msg = ( MsgHeader* )pBuff ;
      msg->opCode = MSG_CAT_QUERY_TASK_REQ ;
      msg->TID = 0 ;
      msg->routeID.value = 0 ;

      // send msg
      rc = sendToCatlog( msg ) ;
      PD_LOG ( PDDEBUG, "Send MSG_CAT_QUERY_TASK_REQ[%s] to catalog[rc:%d]",
               match->toString().c_str(), rc ) ;
   done:
      if ( pBuff )
      {
         SDB_OSS_FREE ( pBuff ) ;
         pBuff = NULL ;
      }
      PD_TRACE_EXITRC ( SDB__CLSMGR__SNDQTSKREQ, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsMgr::updateCatGroup ( BOOLEAN unsetPrimary, INT64 millisec )
   {
      return _shdObj.updateCatGroup ( unsetPrimary, millisec ) ;
   }

   //message function
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__ONCATREGRES, "_clsMgr::_onCatRegisterRes" )
   INT32 _clsMgr::_onCatRegisterRes ( NET_HANDLE handle, MsgHeader* msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMGR__ONCATREGRES );
      NodeID nodeID ;
      _MsgCatRegisterRsp * res = (_MsgCatRegisterRsp*)msg ;

      // have register succeed
      if ( _regTimerID == CLS_INVALID_TIMERID )
      {
         goto done ;
      }

      rc = res->header.res ;
      if ( SDB_CLS_NOT_PRIMARY == res->header.res )
      {
         updateCatGroup ( TRUE ) ;
         goto error ;
      }
      else if ( res->header.res != SDB_OK )
      {
         PD_LOG ( PDSEVERE, "Node register failed[Respone:%d]",
                  res->header.res ) ;
         goto error ;
      }

      {
         //get nodeid
         BSONObj object ( (const char*)(res->data) );
         BSONElement gidEl = object.getField ( CAT_GROUPID_NAME ) ;
         BSONElement nidEl = object.getField ( CAT_NODEID_NAME ) ;

         if ( gidEl.type() != NumberInt || nidEl.type() != NumberInt )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDERROR, "Node register response error" ) ;
            goto error ;
         }

         //Kill register timer
         killTimer ( _regTimerID ) ;
         _regTimerID = CLS_INVALID_TIMERID ;

         //Update the net route agent the local id
         _selfNodeID.columns.groupID = (UINT32)gidEl.Int () ;
         _selfNodeID.columns.nodeID = (UINT32)nidEl.Int () ;
         _shdObj.setNodeID( _selfNodeID ) ;
         PD_LOG ( PDEVENT, "Register succeed, groupID:%u, nodeID:%u",
                  _selfNodeID.columns.groupID,
                  _selfNodeID.columns.nodeID ) ;
      }

      nodeID.value = _selfNodeID.value ;
      nodeID.columns.serviceID = _replServiceID ;
      _replNetRtAgent.setLocalID ( nodeID ) ;
      nodeID.columns.serviceID = _shardServiceID ;
      _shardNetRtAgent.setLocalID ( nodeID ) ;

      // set global id
      pmdSetNodeID( _selfNodeID ) ;

      // callback event handler
      _callRegisterEventHandler() ;

      pmdGetKRCB()->setBusinessOK( TRUE ) ;

      //Update the primary catlog node
      if ( SDB_OK != _shdObj.updatePrimary( msg->routeID, TRUE ) )
      {
         _shdObj.updateCatGroup ( FALSE ) ;
      }

      //Active the shard and repl CBs
      rc = _shdObj.active () ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "active shardCB failed[rc:%d]", rc ) ;
         goto error ;
      }

      rc = _replObj.active () ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "active replCB failed[rc:%d]", rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC (SDB__CLSMGR__ONCATREGRES, rc );
      return rc ;
   error:
      //Need to shutdown
      if ( res->header.res == SDB_CAT_AUTH_FAILED )
      {
         PD_LOG ( PDSEVERE, "Catlog auth the db node failed, shutdown..." ) ;
         PMD_SHUTDOWN_DB( SDB_CAT_AUTH_FAILED ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMGR__ONCATQTSKRES, "_clsMgr::_onCatQueryTaskRes" )
   INT32 _clsMgr::_onCatQueryTaskRes ( NET_HANDLE handle, MsgHeader * msg )
   {
      PD_TRACE_ENTRY ( SDB__CLSMGR__ONCATQTSKRES );
      MsgCatQueryTaskRes *res = ( MsgCatQueryTaskRes* )msg ;
      PD_LOG ( PDDEBUG, "Recieve catalog query task response[requestID:%lld, "
               "flag: %d]", msg->requestID, res->flags ) ;

      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> objList ;

      // need to update catalog group
      if ( SDB_CLS_NOT_PRIMARY == res->flags )
      {
         updateCatGroup( TRUE ) ;
      }
      // need to clear the query task
      else if ( SDB_DMS_EOC == res->flags ||
                SDB_CAT_TASK_NOTFOUND == res->flags )
      {
         _clsLatch.get_shared () ;
         _mapTaskQuery.erase ( msg->requestID ) ;
         _clsLatch.release_shared () ;
         PD_LOG ( PDINFO, "The query task[%lld] has 0 jobs", msg->requestID ) ;
      }
      else if ( SDB_OK != res->flags )
      {
         PD_LOG ( PDERROR, "Query task[%lld] failed[rc=%d]",
                  msg->requestID, res->flags ) ;
         goto error ;
      }
      else
      {
         rc = msgExtractReply ( (CHAR *)msg, &flag, &contextID, &startFrom,
                                &numReturned, objList ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         // find the task query map, and remove it
         {
            ossScopedLock lock ( &_clsLatch, EXCLUSIVE ) ;
            MAPTASKQUERY::iterator it = _mapTaskQuery.find ( msg->requestID ) ;
            if ( it == _mapTaskQuery.end() )
            {
               PD_LOG ( PDWARNING, "The query task response[%lld] is not exist",
                        msg->requestID ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            //remove the query task
            _mapTaskQuery.erase ( it ) ;
         }

         PD_LOG ( PDINFO, "The query task[%lld] has %d jobs", msg->requestID,
                  numReturned ) ;

         // add task inner session
         {
            UINT32 index = 0 ;
            while ( index < objList.size() )
            {
               _addTaskInnerSession ( objList[index].objdata() ) ;
               ++index ;
            }
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSMGR__ONCATQTSKRES, rc );
      return rc ;
   error:
      goto done ;
   }

   /*
      get global cls cb
   */
   clsCB* sdbGetClsCB ()
   {
      static clsCB s_clsCB ;
      return &s_clsCB ;
   }
   shardCB* sdbGetShardCB ()
   {
      return sdbGetClsCB()->getShardCB() ;
   }
   replCB* sdbGetReplCB ()
   {
      return sdbGetClsCB()->getReplCB() ;
   }

}

