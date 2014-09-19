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

   Source File Name = omagentMgr.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/


#include "omagentMgr.hpp"
#include "omagentSession.hpp"
#include "pmd.hpp"

namespace engine
{
   /*
      LOCAL DEFINE
   */
   #define OMAGENT_WAIT_CB_ATTACH_TIMEOUT             ( 300 * OSS_ONE_SEC )

   /*
      _omAgentOptions implement
   */
   _omAgentOptions::_omAgentOptions()
   {
      ossMemset( _cmServiceName, 0, sizeof( _cmServiceName ) ) ;
      _restartCount        = -1 ;
      _restartInterval     = 0 ;
      _autoStart           = FALSE ;
      _diagLevel           = PDWARNING ;

      ossMemset( _cfgFileName, 0, sizeof( _cfgFileName ) ) ;
      ossMemset( _localCfgPath, 0, sizeof( _localCfgPath ) ) ;
      ossMemset( _scriptPath, 0, sizeof( _scriptPath ) ) ;
      ossMemset( _startProcFile, 0, sizeof( _startProcFile ) ) ;
      ossMemset( _stopProcFile, 0, sizeof( _stopProcFile ) ) ;

      // defaut service name
      ossSnprintf( _cmServiceName, OSS_MAX_SERVICENAME, "%u",
                   SDBCM_DFT_PORT ) ;
   }

   _omAgentOptions::~_omAgentOptions()
   {
   }

   PDLEVEL _omAgentOptions::getDiagLevel() const
   {
      PDLEVEL level = PDWARNING ;
      if ( _diagLevel < PDSEVERE )
      {
         level = PDSEVERE ;
      }
      else if ( _diagLevel > PDDEBUG )
      {
         level = PDDEBUG ;
      }
      else
      {
         level= ( PDLEVEL )_diagLevel ;
      }
      return level ;
   }

   INT32 _omAgentOptions::init ( const CHAR *pRootPath )
   {
      INT32 rc = SDB_OK ;
      CHAR hostName[ OSS_MAX_HOSTNAME + 1 ] = { 0 } ;
      po::options_description desc ( "Command options" ) ;
      po::variables_map vm ;

      ossGetHostName( hostName, OSS_MAX_HOSTNAME ) ;

      _hostKey = hostName ;
      _hostKey += SDBCM_CONF_PORT ;

      PMD_ADD_PARAM_OPTIONS_BEGIN( desc )
         ( SDBCM_CONF_DFTPORT, po::value<string>(),
         "sdbcm default listening port" )
         ( _hostKey.c_str(), po::value<string>(),
         "sdbcm specified listening port" )
         ( SDBCM_RESTART_COUNT, po::value<INT32>(),
         "sequoiadb node restart max count" )
         ( SDBCM_RESTART_INTERVAL, po::value<INT32>(),
         "sequoiadb node restart time interval" )
         ( SDBCM_AUTO_START, po::value<string>(),
         "start sequoiadb node automatically when CM start" )
         ( SDBCM_DIALOG_LEVEL, po::value<INT32>(),
         "Dialog level" )
      PMD_ADD_PARAM_OPTIONS_END

      if ( !pRootPath )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // build 'conf/local' file path
      rc = utilBuildFullPath( pRootPath, SDBCM_LOCAL_PATH, OSS_MAX_PATHSIZE,
                              _localCfgPath ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Root path is too long: %s", pRootPath ) ;
         goto error ;
      }

      // build 'conf/script' path
      rc = utilBuildFullPath( pRootPath, SDBOMA_SCRIPT_PATH, OSS_MAX_PATHSIZE,
                              _scriptPath ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Root path is too long: %s", pRootPath ) ;
         goto error ;
      }

      // build sdbstart program file path
      rc = utilBuildFullPath ( pRootPath, SDBSTARTPROG, OSS_MAX_PATHSIZE,
                               _startProcFile ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Root path is too long: %s", pRootPath ) ;
         goto error ;
      }

      // build sdbstop program file path
      rc = utilBuildFullPath ( pRootPath, SDBSTOPPROG, OSS_MAX_PATHSIZE,
                               _stopProcFile ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Root path is too long: %s", pRootPath ) ;
         goto error ;
      }

      // build sdbcm config file path
      rc = utilBuildFullPath( pRootPath, SDBCM_CONF_PATH_FILE,
                              OSS_MAX_PATHSIZE, _cfgFileName ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Root path is too long: %s", pRootPath ) ;
         goto error ;
      }

      // read config from file
      rc = utilReadConfigureFile( _cfgFileName, desc, vm ) ;
      if ( rc )
      {
         if ( SDB_IO == rc )
         {
            // file or dir not exist
            PD_LOG( PDWARNING, "Config[%s] not exist, use default config",
                    _cfgFileName ) ;
            rc = postLoaded() ;
            goto done ;
         }
         PD_LOG( PDERROR, "Failed to read config from file[%s], rc: %d",
                 _cfgFileName, rc ) ;
         goto error ;
      }

      rc = pmdCfgRecord::init( &vm, NULL ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to init config record, rc: %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omAgentOptions::doDataExchange( pmdCfgExchange * pEX )
   {
      resetResult () ;

      pEX->setCfgStep( PMD_CFG_STEP_REINIT ) ;

      // {{ map configs begin

      // --defaultPort
      rdxString( pEX, SDBCM_CONF_DFTPORT , _cmServiceName,
                 sizeof( _cmServiceName ), FALSE, FALSE,
                 _cmServiceName ) ;
      // --$hostname$_Port
      rdxString( pEX, _hostKey.c_str(), _cmServiceName,
                 sizeof( _cmServiceName ), FALSE, FALSE,
                 _cmServiceName ) ;
      // --RestartCount
      rdxInt( pEX, SDBCM_RESTART_COUNT, _restartCount, FALSE, TRUE,
              _restartCount ) ;
      // --RestartInterval
      rdxInt( pEX, SDBCM_RESTART_INTERVAL, _restartInterval, FALSE, TRUE,
              _restartInterval ) ;
      // --AutoStart
      rdxBooleanS( pEX, SDBCM_AUTO_START, _autoStart, FALSE, TRUE,
                   _autoStart ) ;
      // --DiagLevel
      rdxInt( pEX, SDBCM_DIALOG_LEVEL, _diagLevel, FALSE, TRUE,
              _diagLevel ) ;

      //  end map configs }}

      return getResult () ;
   }

   INT32 _omAgentOptions::postLoaded()
   {
      INT32 rc = SDB_OK ;

      // make sure directory exist
      rc = ossMkdir( getLocalCfgPath(), OSS_CREATE|OSS_READWRITE ) ;
      if ( rc && SDB_FE != rc )
      {
         PD_LOG( PDERROR, "Failed to create dir: %s, rc: %d",
                 getLocalCfgPath(), rc ) ;
         goto error ;
      }
      rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _omAgentSessionMgr implement
   */
   _omAgentSessionMgr::_omAgentSessionMgr()
   {
   }

   _omAgentSessionMgr::~_omAgentSessionMgr()
   {
   }

   UINT64 _omAgentSessionMgr::makeSessionID( const NET_HANDLE & handle,
                                             const MsgHeader * header )
   {
      return ossPack32To64( PMD_BASE_HANDLE_ID + handle, header->TID ) ;
   }

   SDB_SESSION_TYPE _omAgentSessionMgr::_prepareCreate( UINT64 sessionID,
                                                        INT32 startType,
                                                        INT32 opCode )
   {
      return SDB_SESSION_OMAGENT ;
   }

   BOOLEAN _omAgentSessionMgr::_canReuse( SDB_SESSION_TYPE sessionType )
   {
      return FALSE ;
   }

   UINT32 _omAgentSessionMgr::_maxCacheSize() const
   {
      return 0 ;
   }

   void _omAgentSessionMgr::_onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                              const NET_HANDLE &handle,
                                              pmdAsyncSession *pSession )
   {
      _reply( handle, rc, pReq ) ;
   }

   pmdAsyncSession* _omAgentSessionMgr::_createSession( SDB_SESSION_TYPE sessionType,
                                                        INT32 startType,
                                                        UINT64 sessionID,
                                                        void * data )
   {
      pmdAsyncSession *pSession = NULL ;

      if ( SDB_SESSION_OMAGENT == sessionType )
      {
         pSession = SDB_OSS_NEW omaSession( sessionID ) ;
      }
      else
      {
         PD_LOG( PDERROR, "Invalid session type[%d]", sessionType ) ;
      }

      return pSession ;
   }

   /*
      omAgentMgr Message MAP
   */
   BEGIN_OBJ_MSG_MAP( _omAgentMgr, _pmdObjBase )
      
   END_OBJ_MSG_MAP()

   /*
      omAgentMgr implement
   */
   _omAgentMgr::_omAgentMgr()
   : _msgHandler( &_sessionMgr ),
     _timerHandler( &_sessionMgr ),
     _netAgent( &_msgHandler )
   {
      _oneSecTimer         = NET_INVALID_TIMER_ID ;
      _nodeMonitorTimer    = NET_INVALID_TIMER_ID ;
      _watchAndCleanTimer  = NET_INVALID_TIMER_ID ;
   }

   _omAgentMgr::~_omAgentMgr()
   {
   }

   SDB_CB_TYPE _omAgentMgr::cbType() const
   {
      return SDB_CB_OMAGT ;
   }

   const CHAR* _omAgentMgr::cbName() const
   {
      return "OMAGENT" ;
   }

   INT32 _omAgentMgr::init()
   {
      INT32 rc = SDB_OK ;
      const CHAR *hostName = pmdGetKRCB()->getHostName() ;
      const CHAR *cmService = _options.getCMServiceName() ;
      MsgRouteID nodeID ;
      nodeID.value = MSG_INVALID_ROUTEID ;

      // 1. create listen
      _netAgent.updateRoute( nodeID, hostName, cmService ) ;
      rc = _netAgent.listen( nodeID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Create listen[ServiceName: %s] failed, rc: %d",
                 cmService, rc ) ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "Create listen[ServiceName:%s] succeed",
               cmService ) ;

      // 2. init session manager
      rc = _sessionMgr.init( &_netAgent, &_timerHandler, OSS_ONE_SEC ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init session manager failed, rc: %d", rc ) ;
         goto error ;
      }

      // 3. init node manager
      rc = _nodeMgr.init() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init node manager failed, rc: %d", rc ) ;
         goto error ;
      }

      // 4. init scopt container
      rc = _sptScopePool.init() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init container failed, rc: %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omAgentMgr::active()
   {
      INT32 rc = SDB_OK ;
      pmdEDUMgr *pEDUMgr = pmdGetKRCB()->getEDUMgr() ;
      EDUID eduID = PMD_INVALID_EDUID ;

      // set primary
      pmdSetPrimary( TRUE ) ;

      // active node manager
      rc = _nodeMgr.active() ;
      PD_RC_CHECK( rc, PDERROR, "Active node manager failed, rc: %d", rc ) ;

      // 1. start om manager edu
      rc = pEDUMgr->startEDU( EDU_TYPE_OMMGR, (_pmdObjBase*)this, &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start OM Manager edu, rc: %d", rc ) ;
      // register
      pEDUMgr->regSystemEDU( EDU_TYPE_OMMGR, eduID ) ;
      // wait attach
      rc = _attachEvent.wait( OMAGENT_WAIT_CB_ATTACH_TIMEOUT ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait OM Manager edu attach failed, rc: %d",
                   rc ) ;

      // 2. start om net edu
      rc = pEDUMgr->startEDU( EDU_TYPE_OMNET, (netRouteAgent*)&_netAgent,
                              &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start om net, rc: %d", rc ) ;
      // register
      pEDUMgr->regSystemEDU( EDU_TYPE_OMNET, eduID ) ;

      // 3. register timer
      rc = _netAgent.addTimer( OSS_ONE_SEC, &_timerHandler, _oneSecTimer ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to set timer, rc: %d", rc ) ;
         goto error ;
      }

      rc = _netAgent.addTimer( 2 * OSS_ONE_SEC, &_timerHandler,
                               _nodeMonitorTimer ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to set timer, rc: %d", rc ) ;
         goto error ;
      }
      rc = _netAgent.addTimer( 120 * OSS_ONE_SEC, &_timerHandler,
                               _watchAndCleanTimer ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to set timer, rc: %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omAgentMgr::deactive()
   {
      iPmdProc::stop( 0 ) ;

      // 1. kill timer
      if ( NET_INVALID_TIMER_ID != _oneSecTimer )
      {
         _netAgent.removeTimer( _oneSecTimer ) ;
         _oneSecTimer = NET_INVALID_TIMER_ID ;
      }

      // 2. stop listen
      _netAgent.closeListen() ;

      // 3. stop io
      _netAgent.stop() ;

      // 4. set force
      _sessionMgr.setForced() ;
      
      return SDB_OK ;
   }

   INT32 _omAgentMgr::fini()
   {
      _nodeMgr.fini() ;
      _sessionMgr.fini() ;
      _sptScopePool.fini() ;

      return SDB_OK ;
   }

   void _omAgentMgr::attachCB( _pmdEDUCB * cb )
   {
      _msgHandler.attach( cb ) ;
      _timerHandler.attach( cb ) ;
      _attachEvent.signalAll() ;
   }

   void _omAgentMgr::detachCB( _pmdEDUCB * cb )
   {
      _msgHandler.detach() ;
      _timerHandler.detach() ;
   }

   void _omAgentMgr::onTimer( UINT64 timerID, UINT32 interval )
   {
      if ( _oneSecTimer == timerID )
      {
         //Check _deqShdDeletingSessions
         _sessionMgr.onTimer( interval ) ;
      }
      else if ( _nodeMonitorTimer == timerID )
      {
         _nodeMgr.monitorNodes() ;
      }
      else if ( _watchAndCleanTimer == timerID )
      {
         _nodeMgr.watchManualNodes() ;
         _nodeMgr.cleanDeadNodes() ;
      }
   }

   omAgentOptions* _omAgentMgr::getOptions()
   {
      return &_options ;
   }

   omAgentNodeMgr* _omAgentMgr::getNodeMgr()
   {
      return &_nodeMgr ;
   }

   netRouteAgent* _omAgentMgr::getRouteAgent()
   {
      return &_netAgent ;
   }

   sptContainer* _omAgentMgr::getSptScopePool()
   {
      return &_sptScopePool ;
   }

   sptScope* _omAgentMgr::getScope()
   {
      return _sptScopePool.newScope() ;
   }

   void _omAgentMgr::releaseScope( sptScope * pScope )
   {
      _sptScopePool.releaseScope( pScope ) ;
   }

   /*
      get the global om manager object point
   */
   omAgentMgr *sdbGetOMAgentMgr()
   {
      static omAgentMgr s_omagent ;
      return &s_omagent ;
   }

   omAgentOptions* sdbGetOMAgentOptions()
   {
      return sdbGetOMAgentMgr()->getOptions() ;
   }

}


