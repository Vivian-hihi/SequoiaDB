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
#include "pmd.hpp"
#include "ossVer.hpp"


namespace engine
{

   /*
      _omAgentOptions implement
   */
   _omAgentOptions::_omAgentOptions()
   {
      ossMemset( _cmServiceName, 0, sizeof( _cmServiceName ) ) ;
      _restartCount        = -1 ;
      _restartInterval     = 0 ;
      _autoStart           = FALSE ;

      ossMemset( _cfgFileName, 0, sizeof( _cfgFileName ) ) ;
      ossMemset( _localCfgPath, 0, sizeof( _localCfgPath ) ) ;
      ossMemset( _startProcFile, 0, sizeof( _startProcFile ) ) ;
      ossMemset( _stopProcFile, 0, sizeof( _stopProcFile ) ) ;

      // defaut service name
      ossSnprintf( _cmServiceName, OSS_MAX_SERVICENAME, "%u",
                   SDBCM_DFT_PORT ) ;
   }

   _omAgentOptions::~_omAgentOptions()
   {
   }

   INT32 _omAgentOptions::init ( const CHAR *pRootPath )
   {
      INT32 rc = SDB_OK ;
      const CHAR *hostName = pmdGetKRCB()->getHostName() ;
      po::options_description desc ( "Command options" ) ;
      po::variables_map vm ;

      _hostKey = hostName ;
      _hostKey += SDBCM_CONF_PORT ;

      PMD_ADD_PARAM_OPTIONS_BEGIN( desc )
         ( PMD_COMMANDS_STRING (PMD_OPTION_HELP, ",h"), "help" )
         ( PMD_OPTION_VERSION, "show version" )
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
      PMD_ADD_PARAM_OPTIONS_END

      if ( !pRootPath )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // build 'config/local' file path
      rc = utilBuildFullPath( pRootPath, SDBCM_LOCAL_PATH, OSS_MAX_PATHSIZE,
                              _localCfgPath ) ;
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
         PD_LOG( PDERROR, "Failed to read config from file[%s], rc: %d",
                 _cfgFileName, rc ) ;
         goto error ;
      }

      /// read cmd first
      if ( vm.count( PMD_OPTION_HELP ) )
      {
         cout << desc << endl ;
         rc = SDB_PMD_HELP_ONLY ;
         goto done ;
      }
      if ( vm.count( PMD_OPTION_VERSION ) )
      {
         ossPrintVersion( "Version" ) ;
         rc = SDB_PMD_VERSION_ONLY ;
         goto done ;
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

      //  end map configs }}

      return getResult () ;
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
      return ossPack32To64( CLS_BASE_HANDLE_ID + handle, header->TID ) ;
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

   UINT32 _omAgentSessionMgr::_maxCatchSize() const
   {
      return 0 ;
   }

   void _omAgentSessionMgr::_onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                              const NET_HANDLE &handle,
                                              clsSession *pSession )
   {
      _reply( handle, rc, pReq ) ;
   }

   clsSession* _omAgentSessionMgr::_createSession( SDB_SESSION_TYPE sessionType,
                                                   INT32 startType,
                                                   UINT64 sessionID,
                                                   void * data )
   {
      // TODO:XUJIANHUI
      return NULL ;
   }

   /*
      omAgentMgr Message MAP
   */
   BEGIN_OBJ_MSG_MAP( _omAgentMgr, _clsObjBase )
      
   END_OBJ_MSG_MAP()

   /*
      omAgentMgr implement
   */
   _omAgentMgr::_omAgentMgr()
   : _msgHandler( &_sessionMgr ),
     _timerHandler( &_sessionMgr ),
     _netAgent( &_msgHandler )
   {
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

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omAgentMgr::active()
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   INT32 _omAgentMgr::deactive()
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   INT32 _omAgentMgr::fini()
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   omAgentOptions* _omAgentMgr::getOptions()
   {
      return &_options ;
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


