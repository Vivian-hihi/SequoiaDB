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

   Source File Name = omagentCommand.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentCommand.hpp"
#include "omagentUtil.hpp"
#include "omagentHelper.hpp"
#include "ossProc.hpp"
#include "utilPath.hpp"
#include "ossPath.h"
#include "omagentJob.hpp"
#include "omagentMgr.hpp"

using namespace bson ;

#define HOSTS_FILE_PROMPT "##############add by omagent##############"
#define DEF_VIRTUAL_COORD_SERVICE        "13579"

#define FILE_SCAN_HOST                   "scanHost.js"
#define FILE_BASIC_CHECK_HOST            "basicCheckHost.js"
#define FILE_INSTALL_REMOTE_AGENT        "installRemoteAgent.js"
#define FILE_CHECK_HOST                  "checkHost.js"
#define FILE_UNINSTALL_REMOTE_AGENT      "uninstallRemoteAgent.js"
#define FILE_ADD_HOST                    "addHost.js"

#define FILE_GET_REMOTE_AGENT_STATUS     "getRemoteAgentStatus.js"
#define FILE_CREATE_VIRTUAL_COORD        "createVirtualCoord.js"
#define FILE_REMOVE_VIRTUAL_COORD        "removeVirtualCoord.js"
#define FILE_GET_PORT_STATUS             "getPortStatus.js"
#define FILE_REG_HOSTS_INFO              "regHostsInfo.js"
#define FILE_GET_HOST_NAME               "getHostName.js"
#define FILE_ADDHOST_ROLLBACK_INTERNAL   "addHostRollbackInternal.js"

// TODO:tanzhaobo
// what is the path in windows
#if defined (_WINDOWS)

#define REMOTE_OMAGENT_PROG ""
#define START_DB_PROG "sdbstart.exe"
#define SDB_CM_PROG   "sdbcm.exe"
#define SDB_CM_START  "sdbcmart.exe"
#define SDB_CM_STOP   "sdbcmtop.exe"

#else

#define REMOTE_OMAGENT_PROG "/tmp/sdbcm"
#define START_DB_PROG "sdbstart"
#define SDB_CM_PROG   "sdbcm"
#define SDB_CM_START  "sdbcmart"
#define SDB_CM_STOP   "sdbcmtop"

#endif

#define SDB_CM_CONF   "sdbcm.conf"


namespace engine
{
   // command list:
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaScanHost )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaBasicCheckHost )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaInstallRemoteAgent )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaCheckHost )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaUninstallRemoteAgent )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaAddHost )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaInstallDBBusiness )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaInstallDBStatus )

   /*
      _omaCommand
   */
   _omaCommand::_omaCommand ()
   {
      _scope      = NULL ;
      _fileBuff   = NULL ;
      _buffSize   = 0 ;
      _readSize   = 0 ;
      ossMemset( _jsFileName, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset( _jsFileArgs, 0, JS_ARG_LEN + 1 ) ;
   }

   _omaCommand::~_omaCommand ()
   {
      if ( _scope )
      {
         sdbGetOMAgentMgr()->releaseScope( _scope ) ;
         _scope = NULL ;
      }
      if ( _fileBuff )
      {
         SAFE_OSS_FREE ( _fileBuff ) ;
      }
   }

   INT32 _omaCommand::setJSFile( const CHAR *fileName )
   {
      INT32 rc = SDB_OK ;
      const CHAR *tmp = NULL ;
      if ( NULL == fileName )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      tmp = sdbGetOMAgentOptions()->getScriptPath() ;
      ossStrncpy ( _jsFileName, tmp, OSS_MAX_PATHSIZE ) ;
      rc = utilCatPath ( _jsFileName, OSS_MAX_PATHSIZE, fileName ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to build js file full path, rc = %d",
                      rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _omaCmdAssit
   */
   _omaCmdAssit::_omaCmdAssit ( OA_NEW_FUNC pFunc )
   {
      if ( pFunc )
      {
         _omaCommand *pCommand = (*pFunc)() ;
         if ( pCommand )
         {
            getOmaCmdBuilder()->_register ( pCommand->name(), pFunc ) ;
            SDB_OSS_DEL pCommand ;
            pCommand = NULL ;
         }
      }
   }

   _omaCmdAssit::~_omaCmdAssit ()
   {
   }

   /*
      _omaCmdBuilder
   */
   _omaCmdBuilder::_omaCmdBuilder ()
   {
   }

   _omaCmdBuilder::~_omaCmdBuilder ()
   {
   }

   _omaCommand* _omaCmdBuilder::create ( const CHAR *command )
   {
      OA_NEW_FUNC pFunc = _find ( command ) ;
      if ( pFunc )
      {
         return (*pFunc)() ;
      }
      return NULL ;
   }

   void _omaCmdBuilder::release ( _omaCommand *&pCommand )
   {
      if ( pCommand )
      {
         SDB_OSS_DEL pCommand ;
         pCommand = NULL ;
      }
   }

   INT32 _omaCmdBuilder::_register ( const CHAR *name, OA_NEW_FUNC pFunc )
   {
      INT32 rc = SDB_OK ;

      pair< MAP_OACMD_IT, BOOLEAN > ret ;
      ret = _cmdMap.insert( pair<const CHAR*, OA_NEW_FUNC>(name, pFunc) ) ;
      if ( FALSE == ret.second )
      {
         PD_LOG_MSG ( PDERROR, "Failed to register omagent command[%s], "
                      "already exist", name ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   OA_NEW_FUNC _omaCmdBuilder::_find ( const CHAR *name )
   {
      if ( name )
      {
         MAP_OACMD_IT it = _cmdMap.find( name ) ;
         if ( it != _cmdMap.end() )
         {
            return it->second ;
         }
      }
      return NULL ;
   }

   /*
      get omagent command builder
   */
   _omaCmdBuilder* getOmaCmdBuilder()
   {
      static _omaCmdBuilder cmdBuilder ;
      return &cmdBuilder ;
   }

   /******************************* scan host ********************************/
   /*
      _omaScanHost
   */
   _omaScanHost::_omaScanHost()
   {
      // get js file
      setJSFile( FILE_SCAN_HOST ) ;
   }

   _omaScanHost::~_omaScanHost()
   {
   }

   INT32 _omaScanHost::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInstallInfo ) ;
      ele = arg.getField ( OMA_FIELD_HOSTINFO ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
     goto done ;
   }

   INT32 _omaScanHost::doit ( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;
      while ( itr != _hosts.end() )
      {
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;

         const CHAR *pIp       = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassWord = NULL ;
         const CHAR *pSshPort  = NULL ;
         // get field
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_USER, &pUserName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_USER, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &pPassWord ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_SSH_PORT, &pSshPort ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_SSH_PORT, rc ) ;
            goto error ;
         }
         // build js file argument
         ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                      "var IP = \'%s\'; var USERNAME = \'%s\'; "
                      "var PASSWORD = \'%s\'; var SSHPORT = \'%s\'",
                      pIp, pUserName, pPassWord, pSshPort ) ;

         _content.clear() ;
         _content += _jsFileArgs ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                         "Failed to eval js file[%s]: %s, rc = %d",
                         _jsFileName, detail.toString().c_str(), rc ) ;
            // TODO: tanzhaobo
            // what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_IP, pIp ) ;
            bob.append( OMA_FIELD_RC, rc ) ;
            bob.append( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
            bob.append( OMA_FIELD_PING, false ) ;
            bob.append( OMA_FIELD_SSH, false ) ;
            bob.appendNull( OMA_FIELD_HOSTNAME ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         // save the js eval result
         rc = omaGetObjElement( rval, "", subObj ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d", "", rc ) ;
            goto error ;
         }
         bob.append( OMA_FIELD_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;
      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_HOSTINFO, bab.arr() ) ;
      retObj = bob.obj() ;
   done:
      return rc ;
   error:
     goto done ;
   }

   /******************************* install remote agent *********************/
   _omaBasicCheckHost::_omaBasicCheckHost ()
   {
      // get js file
      setJSFile( FILE_BASIC_CHECK_HOST ) ;
   }
   _omaBasicCheckHost::~_omaBasicCheckHost ()
   {
   }

   /******************************* install remote agent *********************/
   /*
      _omaInstallRemoteAgent
   */
   _omaInstallRemoteAgent::_omaInstallRemoteAgent ()
   {
      ossMemset ( _prog_path, 0 , OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset ( _spt_path, 0 , OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset ( _conf_path, 0 , OSS_MAX_PATHSIZE + 1 ) ;
   }

   _omaInstallRemoteAgent::~_omaInstallRemoteAgent ()
   {
   }

   INT32 _omaInstallRemoteAgent::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInstallInfo ) ;
      ele = arg.getField ( OMA_FIELD_HOSTINFO ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
      }
      // get js file
      rc = setJSFile( FILE_INSTALL_REMOTE_AGENT ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // set program, script, cm config file path
      rc = setLocalPath() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set program's or script's path, rc = %d",
                  rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }

      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallRemoteAgent::doit ( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;

      while ( itr != _hosts.end() )
      {
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObj status ;
         BSONObjBuilder bob ;

         const CHAR *pIp = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;
//         const CHAR *pVersion   = NULL ;
//         BOOLEAN isRunning      = FALSE ;

         // get fields
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_USER, &pUserName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_USER, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &pPassword ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }
/*
         // check whether the remote machine has install omagent or not
         rc = getRemoteAgentStatus ( pIp, pUserName, pPassword, status ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                         "Failed to get remote mechine's status, rc = %d",
                         rc ) ;
            goto error ;
         }
         rc = omaGetBooleanElement( status, OMA_FIELD_AGENT_IS_RUNNING,
                                    isRunning ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_AGENT_IS_RUNNING, rc ) ;
            goto error ;
         }
         if ( isRunning )
         {
            const CHAR *ver = getVersion() ;
            rc = omaGetStringElement( status, OMA_FIELD_VERSION,
                                      &pVersion ) ;
            if ( rc )
            {
               PD_LOG_MSG ( PDERROR, "Get filed[%s] failed, rc = %d",
                            OMA_FIELD_VERSION, rc ) ;
               goto error ;
            }
            if ( 0 != ossStrncmp( pVersion, ver, ossStrlen( ver ) ) )
            {
               PD_LOG( PDDEBUG,
                       "Remote omagent's version is: %s, "
                       "and we are going to instll version %s",
                       pVersion, ver ) ;
               BSONObj errObj ;
               BSONObjBuilder bob ;
               bob.append( OMA_FIELD_IP, pIp ) ;
               bob.append( OMA_FIELD_RC, SDB_OMA_DIFF_VER_AGT_IS_RUNNING ) ;
               bob.append( OMA_FIELD_DETAIL,
                           getErrDesp( SDB_OMA_DIFF_VER_AGT_IS_RUNNING ) ) ;
               errObj = bob.obj() ;
               result.push_back( errObj ) ;
               continue ;
            }
         }
*/
         // build js file's argument
         ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                      " var IP = \"%s\"; var USERNAME = \"%s\"; "
                      "var PASSWORD = \"%s\"; var LOCAL_PROG_PATH = \"%s\"; "
                      "var LOCAL_SPT_PATH = \"%s\"; "
                      "var LOCAL_CM_CONF = \"%s\" ",
                      pIp, pUserName, pPassword,
                      _prog_path, _spt_path, _conf_path ) ;
         PD_LOG ( PDDEBUG, "Install remote agent passes arguments: "
                  "var IP = \"%s\"; var USERNAME = \"%s\"; "
                  "var PASSWORD = \"%s\"; "
                  "var LOCAL_PROG_PATH = \"%s\"; "
                  "var LOCAL_SPT_PATH = \"%s\"; "
                  "var LOCAL_CM_CONF = \"%s\" ",
                  pIp, "xxx", "xxx", _prog_path,
                  _spt_path, _conf_path ) ;

         _content.clear() ;
         _content += _jsFileArgs ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file[%s]: "
                         " %s, rc = %d",
                          _jsFileName,
                          detail.toString().c_str(), rc ) ;
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_IP, pIp ) ;
            bob.append( OMA_FIELD_RC, rc ) ;
            bob.append( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         // extract the result from the return value
         rc = omaGetObjElement( rval, "", subObj ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d", "", rc ) ;
            goto error ;
         }
         bob.append( OMA_FIELD_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;
      } // while
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_HOSTINFO, bab.arr() ) ;
      retObj = bob.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallRemoteAgent::getRemoteAgentStatus ( const CHAR *pIp,
                                                        const CHAR *pUserName,
                                                        const CHAR *pPassword,
                                                        BSONObj &result )
   {
      INT32 rc = SDB_OK ;
      _omaGetRemoteAgentStatus checkRemote ;
      rc = checkRemote.getStatus( pIp, pUserName, pPassword, result ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR,
                      "Faled to check remote mechine's status, rc = %d",
                      rc ) ;
      }
      return rc ;
   }

   INT32 _omaInstallRemoteAgent::setLocalPath ()
   {
      INT32 rc = SDB_OK ;
      std::string str ;
      std::string key ;
      UINT32 found = 0 ;
      CHAR tmp[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

      // program path
      rc = ossGetEWD ( tmp, OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR,
                      "Failed to get program's work directory, rc = %d", rc ) ;
         goto error ;
      }
      rc = utilCatPath ( tmp, OSS_MAX_PATHSIZE, SDB_CM_PROG ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR,
                      "Failed to build program's full directory, rc = %d",
                      rc ) ;
         goto error ;
      }
      str = tmp ;
      key = SDB_CM_PROG ;
      found = str.rfind( key ) ;
      if ( found != std::string::npos )
      {
         str.replace( found, key.length(), "\0" ) ;
         ossStrncpy( _prog_path, str.c_str(), OSS_MAX_PATHSIZE ) ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG_MSG ( PDERROR, "Failed to set program's path" ) ;
         goto error ;
      }

      // cm conf file path
      ossStrncpy( _conf_path, sdbGetOMAgentOptions()->getCfgFileName(),
                  OSS_MAX_PATHSIZE ) ;

      // script file path
      str = _jsFileName ;
      key = FILE_INSTALL_REMOTE_AGENT ;
      found = str.rfind( key ) ;
      if ( found != std::string::npos )
      {
         str.replace( found, key.length(), "\0" ) ;
         ossStrncpy( _spt_path, str.c_str(), OSS_MAX_PATHSIZE ) ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG_MSG ( PDERROR, "Failed to set script's path" ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /******************************* check host ********************************/
   /*
      _omaCheckHost
   */
   _omaCheckHost::_omaCheckHost ()
   {
      _pIp       = NULL ;
      _pHostName = NULL ;
      _pUserName = NULL ;
      _pPassword = NULL ;
   }

   _omaCheckHost::~_omaCheckHost ()
   {
   }

   INT32 _omaCheckHost::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONObj host( pInstallInfo ) ;
      // get fields
      rc = omaGetStringElement( host, OMA_FIELD_IP, &_pIp ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_IP, rc ) ;
         goto error ;
      }
      rc = omaGetStringElement( host, OMA_FIELD_HOSTNAME, &_pHostName ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_HOSTNAME, rc ) ;
         goto error ;
      }
      rc = omaGetStringElement( host, OMA_FIELD_USER, &_pUserName ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_USER, rc ) ;
         goto error ;
      }
      rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &_pPassword ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_PASSWD, rc ) ;
         goto error ;
      }

      // get js file
      rc = setJSFile( FILE_CHECK_HOST ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff, &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // build js argument for js file
      ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                   " var IP = \"%s\"; var HOSTNAME = \"%s\"; "
                   "var USERNAME = \"%s\"; var PASSWORD = \"%s\"; ",
                   _pIp, _pHostName, _pUserName, _pPassword ) ;
      PD_LOG ( PDDEBUG, "Excute check host infomation passes arguments: "
               "var IP = %s; var HOSTNAME = %s; "
               "var USERNAME = %s; var PASSWORD = %s",
               _pIp, _pHostName, "xxx", "xxx" ) ;

      _content.clear() ;
      _content += _jsFileArgs ;
      _content += OSS_NEWLINE ;
      _content += _fileBuff ;

      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaCheckHost::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      INT32 retRc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONObj rval ;
      BSONObj detail ;
      BSONObj subObj ;
      
      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;

      if ( rc )
      {
         PD_LOG_MSG ( PDERROR,
                      "Failed to eval js file[%s]: %s, rc = %d",
                      _jsFileName, detail.toString().c_str(), rc ) ;
         goto error ;
      }
      // extract result
      rc = omaGetObjElement( rval, "", subObj ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d", "", rc ) ;
         goto error ;
      }
      // check result
      rc = omaGetIntElement( subObj, OMA_FIELD_RC, retRc ) ;
      if ( retRc )
      {
         const CHAR *pDetail = "" ;
         omaGetStringElement( subObj, OMA_FIELD_DETAIL, &pDetail ) ;
         PD_LOG_MSG ( PDERROR, "Failed to chek host[%s]:%s, rc = %d",
                      _pIp, pDetail, retRc ) ;
         rc = retRc ;
         goto error ;
         
      }
      // format the result for omsvc
      rc = _adaptTheResult( subObj, retObj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build result for check host, rc = %d",
                  rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptTheResult ( BSONObj &obj, BSONObj &result )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder builder ;
      BSONElement ele ;
      BSONObj pattern ;
      BSONObj tmpObj ;

      // adopt ip
      rc = _adaptIP( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt ip info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      // adopt cpu
      rc = _adaptCpu( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt cpu info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      // adopt net
      rc = _adaptNet( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt net card info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      // adopt disk
      rc = _adaptDisk( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt disk info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      // adopt memory
      rc = _adaptMemory( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt memory info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      // adopt port 
      rc = _adaptPortStatus( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt port status info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      // adopt service
      rc = _adaptService( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt service info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      // adopt om status
      rc = _adaptOMStatus( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt om svc info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      // adopt safety
      rc = _adaptSafety( obj, builder ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to adapt safety info for check host, "
                  "rc = %d", rc ) ;
         goto error ;
      }

      // append the other info
      pattern = BSON ( OMA_FIELD_IP << 1 <<
                       OMA_FIELD_CPU << 1 << 
                       OMA_FIELD_NET << 1 << 
                       OMA_FIELD_DISK << 1 <<
                       OMA_FIELD_MEMORY << 1 <<
                       OMA_FIELD_OM << 1 <<
                       OMA_FIELD_SERVICE << 1 <<
                       OMA_FIELD_PORT << 1 <<
                       OMA_FIELD_SAFETY << 1 <<
                       OMA_FIELD_RC << 1 <<
                       OMA_FIELD_DETAIL << 1 ) ;
      tmpObj = obj.filterFieldsUndotted( pattern, false ) ;
      builder.appendElements( tmpObj ) ;
      result = builder.obj() ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptIP ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;

      try
      {
         builder.append( OMA_FIELD_IP, _pIp ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to build bson, "
                  "received unexpect error: %s", e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }


   INT32 _omaCheckHost::_adaptCpu ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONElement ele ;
      BSONObj tmpObj ;

      // adopt disk
      rc = omaGetSubObjArrayElement ( obj, OMA_FIELD_CPU,
                                      OMA_FIELD_CPUS,
                                      OMA_FIELD_CPU, bob ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get sub obj array's element, rc = %d",
                  rc ) ;
         goto error ;
      }
      tmpObj = bob.obj() ;
      ele = tmpObj.getField( OMA_FIELD_CPU ) ;        
      if ( Array == ele.type() )
      {
         const CHAR *pID = NULL ;
         const CHAR *pModel = NULL ;
         INT32 coreNum = NULL ;
         const CHAR *pFreq = NULL ;
         BSONObj cpu ;
         BSONObjIterator itr( ele.embeddedObject() ) ;

         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_SYS ;
               PD_LOG_MSG ( PDERROR, "Wrong bson format" ) ;
               goto error ;
            }
            BSONObj subObj = ele.embeddedObject() ;
            rc = omaGetStringElement( subObj, OMA_FIELD_ID, &pID ) ;
            if ( SDB_OK != rc ) pID = "" ;
            rc = omaGetStringElement( subObj, OMA_FIELD_MODEL, &pModel ) ;
            if ( SDB_OK != rc ) pModel = "" ;
            rc = omaGetIntElement( subObj, OMA_FIELD_CORE, coreNum ) ;
            if ( SDB_OK != rc ) coreNum = 0 ;
            rc = omaGetStringElement( subObj, OMA_FIELD_FREQ, &pFreq ) ;
            if ( SDB_OK != rc ) pFreq = "" ;
            rc = SDB_OK ;
            cpu = BSON( OMA_FIELD_ID << pID <<
                        OMA_FIELD_MODEL << pModel <<
                        OMA_FIELD_CORE << coreNum <<
                        OMA_FIELD_FREQ << pFreq ) ;
            bab.append ( cpu ) ;
         }
         builder.appendArray( OMA_FIELD_CPU, bab.arr() ) ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Wrong bson format: %s",
                  obj.toString( FALSE, TRUE ).c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptNet ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONElement ele ;
      BSONObj tmpObj ;

      // adopt disk
      rc = omaGetSubObjArrayElement ( obj, OMA_FIELD_NET,
                                      OMA_FIELD_NETCARDS,
                                      OMA_FIELD_NET, bob ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get sub obj array's element, rc = %d",
                  rc ) ;
         goto error ;
      }
      tmpObj = bob.obj() ;
      ele = tmpObj.getField( OMA_FIELD_NET ) ;
      if ( Array == ele.type() )
      {
         const CHAR *pName      = NULL ;
         const CHAR *pModel     = NULL ;
         const CHAR *pBandwidth = NULL ;
         const CHAR *pIP        = NULL ;
         BSONObj net ;
         BSONObjIterator itr( ele.embeddedObject() ) ;

         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_SYS ;
               PD_LOG_MSG ( PDERROR, "Wrong bson format" ) ;
               goto error ;
            }
            BSONObj subObj = ele.embeddedObject() ;
            rc = omaGetStringElement( subObj, OMA_FIELD_NAME, &pName ) ;
            if ( SDB_OK != rc ) pName = "" ;
            rc = omaGetStringElement( subObj, OMA_FIELD_MODEL, &pModel ) ;
            if ( SDB_OK != rc ) pModel = "" ;
            rc = omaGetStringElement( subObj, OMA_FIELD_BANDWIDTH,
                                      &pBandwidth ) ;
            if ( SDB_OK != rc ) pBandwidth = "" ;
            rc = omaGetStringElement( subObj, OMA_FIELD_IP2, &pIP ) ;
            if ( SDB_OK != rc ) pIP = "" ;
            rc = SDB_OK ;
            net = BSON( OMA_FIELD_NAME << pName <<
                        OMA_FIELD_MODEL << pModel <<
                        OMA_FIELD_BANDWIDTH << pBandwidth <<
                        OMA_FIELD_IP << pIP ) ;
            bab.append ( net ) ;
         }
         builder.appendArray( OMA_FIELD_NET, bab.arr() ) ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Wrong bson format: %s",
                  obj.toString( FALSE, TRUE ).c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptDisk ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONElement ele ;
      BSONObj tmpObj ;

      // adopt disk
      rc = omaGetSubObjArrayElement ( obj, OMA_FIELD_DISK,
                                      OMA_FIELD_DISKS,
                                      OMA_FIELD_DISK, bob ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get sub obj array's element, rc = %d",
                  rc ) ;
         goto error ;
      }
      tmpObj = bob.obj() ;
      ele = tmpObj.getField( OMA_FIELD_DISK ) ;        
      if ( Array == ele.type() )
      {
         const CHAR *pName  = NULL ;
         const CHAR *pMount = NULL ;
         INT64 size         = 0 ;
         INT64 free         = 0 ;
         INT64 use          = 0 ;
         BOOLEAN isLocal    = FALSE ;
         BSONObj disk ;
         BSONObjIterator itr( ele.embeddedObject() ) ;

         while ( itr.more() )
         {
            BSONObjBuilder bob2 ;
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_SYS ;
               PD_LOG_MSG ( PDERROR, "Wrong bson format" ) ;
               goto error ;
            }
            BSONObj subObj = ele.embeddedObject() ;
            rc = omaGetStringElement( subObj, OMA_FIELD_FILESYSTEM, &pName ) ;
            if ( SDB_OK != rc )
               pName = "" ;
            rc = omaGetStringElement( subObj, OMA_FIELD_MOUNT, &pMount ) ;
            if ( SDB_OK != rc )
               pMount = "" ;
            size = subObj.getField( OMA_FIELD_SIZE ).numberLong() ;
            use = subObj.getField( OMA_FIELD_USED ).numberLong() ;
            free = size - use ;
            rc = omaGetBooleanElement( subObj, OMA_FIELD_ISLOCAL, isLocal ) ;
            if ( SDB_OK != rc )
               isLocal = FALSE ;
            rc = SDB_OK ;
            bob2.append ( OMA_FIELD_NAME, pName ) ;
            bob2.append ( OMA_FIELD_MOUNT, pMount ) ;
            bob2.append ( OMA_FIELD_SIZE, size ) ;
            bob2.append ( OMA_FIELD_FREE, free ) ;
            bob2.appendBool ( OMA_FIELD_ISLOCAL, isLocal ) ;
            disk = bob2.obj() ;
            bab.append ( disk ) ;
         }
         builder.appendArray( OMA_FIELD_DISK, bab.arr() ) ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Wrong bson format: %s",
                  obj.toString( FALSE, TRUE ).c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptMemory ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      BSONObj memory ;
      const CHAR *pModel = NULL ;
      INT64 size = 0 ;
      INT64 free = 0 ;
     
      try
      { 
         BSONObj tmpObj = obj.getObjectField( OMA_FIELD_MEMORY ) ;
         pModel = tmpObj.getStringField( OMA_FIELD_MODEL ) ;
         ele = tmpObj.getField( OMA_FIELD_SIZE ) ;
         size = ele.numberLong() ;
         ele = tmpObj.getField( OMA_FIELD_FREE ) ;
         free = ele.numberLong() ;
   
         memory = BSON( OMA_FIELD_MODEL << pModel << 
                        OMA_FIELD_SIZE << size <<
                        OMA_FIELD_FREE << free ) ;
         builder.append ( OMA_FIELD_MEMORY, memory ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to build bson, "
                  "received unexpect error: %s", e.what() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptPortStatus ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      const CHAR *pPort = "50000" ;
      BOOLEAN status = FALSE ;

      try
      {
         // TODO: tanzhaobo
         bob.append( OMA_FIELD_PORT, pPort ) ;
         bob.appendBool ( OMA_FIELD_STATUS, status ) ;
         bab.append( bob.obj() ) ;
         builder.append( OMA_FIELD_PORT, bab.arr() ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to build bson, "
                  "received unexpect error: %s", e.what() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptService ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;   
      const CHAR *pName    = "" ;
      const CHAR *pVersion = "" ;
      BOOLEAN status       = FALSE ;

      try
      {
         // TODO: tanzhaobo
         bob.append( OMA_FIELD_NAME, pName ) ;
         bob.appendBool( OMA_FIELD_STATUS, status ) ;
         bob.append( OMA_FIELD_VERSION, pVersion ) ;
         bab.append( bob.obj() ) ;
         builder.append( OMA_FIELD_SERVICE, bab.arr() ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to build bson, "
                  "received unexpect error: %s", e.what() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptOMStatus ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONObj omStatus ;
      BOOLEAN status = FALSE ;
      const CHAR *pVersion = "" ;

      try
      {
         BSONObj tmpObj = obj.getObjectField( OMA_FIELD_OM ) ;
         status = tmpObj.getBoolField( OMA_FIELD_STATUS ) ;
         pVersion = tmpObj.getStringField( OMA_FIELD_VERSION ) ;
   
         bob.appendBool( OMA_FIELD_STATUS, status ) ;
         bob.append ( OMA_FIELD_VERSION, pVersion ) ;
         omStatus = bob.obj() ;
         builder.append ( OMA_FIELD_OM, omStatus ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to build bson, "
                  "received unexpect error: %s", e.what() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCheckHost::_adaptSafety ( BSONObj &obj, BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONObj safety ;
      const CHAR *pName    = NULL ;
      const CHAR *pContext = NULL ;
      BOOLEAN status       = FALSE ;

      try
      {
         BSONObj tmpObj = obj.getObjectField( OMA_FIELD_SAFETY ) ;
         pName = tmpObj.getStringField( OMA_FIELD_NAME ) ;
         pContext = tmpObj.getStringField( OMA_FIELD_CONTEXT ) ;
         status = tmpObj.getBoolField( OMA_FIELD_STATUS ) ;
   
         bob.append( OMA_FIELD_NAME, pName ) ;
         bob.append( OMA_FIELD_CONTEXT, pContext ) ;
         bob.appendBool( OMA_FIELD_STATUS, status ) ;
         safety = bob.obj() ;
         builder.append ( OMA_FIELD_SAFETY, safety ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to build bson, "
                  "received unexpect error: %s", e.what() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /******************************* uninstall remote agent *******************/
   /*
      _omaUninstallRemoteAgent
   */
   _omaUninstallRemoteAgent::_omaUninstallRemoteAgent ()
   {
   }

   _omaUninstallRemoteAgent::~_omaUninstallRemoteAgent ()
   {
   }

   INT32 _omaUninstallRemoteAgent::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInstallInfo ) ;
      ele = arg.getField ( OMA_FIELD_HOSTINFO ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
      }
      // get js file
      rc = setJSFile( FILE_UNINSTALL_REMOTE_AGENT ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaUninstallRemoteAgent::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;
      while ( itr != _hosts.end() )
      {
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;

         const CHAR *pIp = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;

         // get fileds
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                         "Get field[%s] failed, rc: %d", OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_USER, &pUserName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                         "Get field[%s] failed, rc: %d", OMA_FIELD_USER, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &pPassword ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                         "Get field[%s] failed, rc: %d",
                         OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }

         // build argument for js file
         ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                      " var IP = \"%s\"; var USERNAME = \"%s\"; "
                      "var PASSWORD = \"%s\"; ",
                      pIp, pUserName, pPassword ) ;
         PD_LOG ( PDDEBUG, "Excute uninstall remote sdbcm[ip:%s]", pIp ) ;
         _content.clear() ;
         _content += _jsFileArgs ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file[%s]: %s, "
                         "rc = %d",
                         _jsFileName,
                         detail.toString().c_str(), rc ) ;
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_IP, pIp ) ;
            bob.append( OMA_FIELD_RC, rc ) ;
            bob.append( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         rc = omaGetObjElement( rval, "", subObj ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d", "", rc ) ;
            goto error ;
         }
         bob.append( OMA_FIELD_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;

      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_HOSTINFO, bab.arr() ) ;
      retObj = bob.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   /******************************* add host ********************************/
   /*
      _omaAddHost
   */
   _omaAddHost::_omaAddHost ()
   {
      _transactionID = 12345678 ;
   }

   _omaAddHost::~_omaAddHost ()
   {
      ossMemset ( _packet_path, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset ( _sdb_user, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset ( _sdb_passwd, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset ( _sdb_user_group, 0, OSS_MAX_PATHSIZE + 1 ) ;
   }

   INT32 _omaAddHost::init( const CHAR *pInstallInfo )
   {
      INT32 rc                  = SDB_OK ;
      const CHAR *pSdbUser      = NULL ;
      const CHAR *pSdbPassword  = NULL ;
      const CHAR *pSdbUserGroup = NULL ;
      const CHAR *pPacketPath   = NULL ;
      // parse bson and get arguments info for js file
      BSONObj arg( pInstallInfo ) ;
      BSONElement ele ;
      // get sdbuser/sdbpasswd/sdbusergroup
      rc = omaGetStringElement( arg, OMA_FIELD_SDBUSER, &pSdbUser ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_SDBUSER, rc ) ;
         goto error ;
      }
      rc = omaGetStringElement( arg, OMA_FIELD_SDBPASSWD, &pSdbPassword ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_SDBPASSWD, rc ) ;
         goto error ;
      }
      rc = omaGetStringElement( arg, OMA_FIELD_SDBUSERGROUP, &pSdbUserGroup ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_SDBUSERGROUP, rc ) ;
         goto error ;
      }
      rc = omaGetStringElement( arg, OMA_FIELD_PACKET_PATH, &pPacketPath ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_PACKET_PATH, rc ) ;
         goto error ;
      }
      ossStrncpy ( _sdb_user, pSdbUser, OSS_MAX_PATHSIZE ) ;
      ossStrncpy ( _sdb_passwd, pSdbPassword, OSS_MAX_PATHSIZE ) ;
      ossStrncpy ( _sdb_user_group, pSdbUserGroup, OSS_MAX_PATHSIZE ) ;
      ossStrncpy ( _packet_path, pPacketPath, OSS_MAX_PATHSIZE ) ;
      // get install host info
      ele = arg.getField ( OMA_FIELD_HOSTINFO ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
      }
      // set js file
      rc = setJSFile( FILE_ADD_HOST ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaAddHost::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;
      while ( itr != _hosts.end() )
      {
         BSONObj host = *itr++ ;
         BSONObj detail ;
         BSONObj rval ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;

         const CHAR *pIp             = NULL ;
         const CHAR *pHostName       = NULL ;
         const CHAR *pUserName       = NULL ;
         const CHAR *pPassword       = NULL ;
         const CHAR *pInstallPath    = NULL ;
         AddHost addHost ;

         // get fileds
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                         OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_HOSTNAME, &pHostName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                         OMA_FIELD_HOSTNAME, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_USER, &pUserName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                         OMA_FIELD_USER, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &pPassword ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                         OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_INSTALLPATH,
                                   &pInstallPath ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                         OMA_FIELD_INSTALLPATH, rc ) ;
            goto error ;
         }

         // build argument for js file
         // register
         addHost._ip          = pIp ;
         addHost._userName    = pUserName ;
         addHost._passwd      = pPassword ;
         addHost._installPath = pInstallPath ;
         _hasAddHosts.push_back( addHost ) ;

         // build argument for js file
         ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                      " var SDBUSER = \"%s\"; var SDBPASSWD = \"%s\"; "
                      "var SDBUSERGROUP = \"%s\"; "
                      "var IP = \"%s\"; var HOSTNAME = \"%s\"; "
                      "var USERNAME = \"%s\"; var PASSWORD = \"%s\"; "
                      "var PACKET_PATH = \"%s\"; var INSTALL_PATH = \"%s\"; ",
                      _sdb_user, _sdb_passwd, _sdb_user_group,
                      pIp, pHostName, pUserName, pPassword,
                      _packet_path, pInstallPath ) ;
         PD_LOG ( PDDEBUG, " Add host passes arguments: "
                  "var SDBUSER = %s; var SDBPASSWD = %s; "
                  "var SDBUSERGROUP = %s; var IP = %s; "
                  "var HOSTNAME = %s; var USERNAME = %s; "
                  "var PASSWORD = %s; var PACKET_PATH = %s; "
                  "var INSTALL_PATH = %s ",
                  _sdb_user, _sdb_passwd, _sdb_user_group,
                  pIp, pHostName,
                  "xxx", "xxx", _packet_path, pInstallPath ) ;
         _content.clear() ;
         _content += _jsFileArgs ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file[%s]: %s, rc = %d",
                         _jsFileName, detail.toString().c_str(), rc ) ;
            goto error ;
         }
         // set the rollback flag to be true, and when error happen
         // it's going to rollback
         _needRollback = TRUE ;
         // get the return result
         rc = omaGetObjElement( rval, "", subObj ) ;
         if ( rc )
         {
            PD_LOG_MSG( PDERROR, "Get field[%s] failed, rc = %d", "", rc ) ;
            goto error ;
         }
         {
         INT32 retRc = SDB_OK ;
         BSONObj value ;
         const CHAR *pErrMsg = NULL ;
         rc = omaGetIntElement( subObj, OMA_FIELD_RC, retRc ) ;
         if ( rc )
         {
            PD_LOG_MSG( PDERROR, "Get field[%s] failed, rc = %d",
                        OMA_FIELD_RC, rc ) ;
            goto error ;
         }
         if ( retRc )
         {
            rc = omaGetObjElement( subObj, OMA_FIELD_DETAIL, value ) ;
            if ( rc )
            {
               PD_LOG_MSG( PDERROR, "Get field[%s] failed, rc = %d",
                           OMA_FIELD_DETAIL, rc ) ;
               goto error ;
            }
            rc = omaGetStringElement( value, OMA_FIELD_ERRMSG, &pErrMsg ) ;
            if ( rc )
            {
               PD_LOG_MSG( PDERROR, "Get field[%s] failed, rc = %d",
                           OMA_FIELD_ERRMSG, rc ) ;
               goto error ;
            }
            PD_LOG_MSG ( PDERROR, "Failed to add host[%s]: %s, rc = %d",
                         pIp, pErrMsg, retRc ) ;
            rc = retRc ;
            goto error ;
         }
         }
         bob.append( OMA_FIELD_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;

      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_HOSTINFO, bab.arr() ) ;
      bob.append( OMA_FIELD_TRANSACTION_ID, _transactionID ) ;
      retObj = bob.obj() ;
   done:
      return rc ;
   error:
      if ( _needRollback )
      {
         PD_LOG ( PDERROR, "Something wrong for add host, "
                  "going to rollback internally, rc = %d", rc ) ; 
         rollback_internal() ;
      }
      goto done ;
   }

   INT32 _omaAddHost::rollback_internal ()
   {
      INT32 rc = SDB_OK ;
      _omaAddHostRollbackInternal _rollbackInternal ;
      if ( 0 == _hasAddHosts.size() )
         goto done ;
      rc = _rollbackInternal.rollback( _hasAddHosts ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to rollback in add host, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /******************************* add db business **************************/
   // _omaInstallDBBusiness
   _omaInstallDBBusiness::_omaInstallDBBusiness ()
   {
      ossMemset( _localHostName, OSS_MAX_HOSTNAME + 1, 0 ) ;
      ossMemset( _omaSvcName, OSS_MAX_SERVICENAME + 1, 0 ) ;
      ossMemset( _vCoordSvcName, OSS_MAX_SERVICENAME + 1, 0 ) ;
   }

   _omaInstallDBBusiness::~_omaInstallDBBusiness ()
   {
   }

   INT32 _omaInstallDBBusiness::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      BSONObj arg( pInstallInfo ) ;

      // get localhost name
      rc = ossGetHostName( _localHostName, OSS_MAX_HOSTNAME ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to get localhost name, rc = %d", rc ) ;
         goto error ;
      }
      // get local sdbcm service name
      ossStrncpy( _omaSvcName, sdbGetOMAgentOptions()->getCMServiceName(),
                  OSS_MAX_SERVICENAME ) ;
      // get a  service name for creating virtual coord
      // TODO:
      ossStrncpy ( _vCoordSvcName, DEF_VIRTUAL_COORD_SERVICE,
                   OSS_MAX_SERVICENAME ) ;

      // parse bson and get arguments info for js file
      ele = arg.getField ( OMA_FIELD_CONFIG ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            const CHAR *value = NULL ;
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            // category
            rc = omaGetStringElement ( temp, OMA_OPTION_ROLE, &value ) ;
            if ( rc )
            {
               PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                            OMA_OPTION_ROLE, rc ) ;
               goto error ;
            }
            if ( 0 == ossStrncmp( value, ROLE_DATA,
                                  ossStrlen( ROLE_DATA ) ) )
            {
               _data.push_back( temp ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_COORD,
                                       ossStrlen( ROLE_COORD ) ) )
            {
               _coord.push_back( temp ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_CATA,
                                       ossStrlen( ROLE_CATA ) ) )
            {
               _catalog.push_back( temp ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_STANDALONE,
                                       ossStrlen( ROLE_STANDALONE ) ) )
            {
               _standalone.push_back( temp ) ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG( PDERROR,
                           "Unknown role for install db business[%s]",
                           temp.toString().c_str() ) ;
               goto error ;
            }
         }
      } // todo: else

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaInstallDBBusiness::doit( BSONObj &objRet )
   {
      INT32 rc                         = SDB_OK ;
      _omaInstallDBBusinessTask *pTask = NULL ;
      BOOLEAN hasVCoordStart           = FALSE ;
      _omaTaskMgr *pTaskMgr            = getTaskMgr() ;
      UINT64 taskID                    = pTaskMgr->getTaskID() ;
      _omaCreateVirtualCoord createVCoord( _localHostName, _omaSvcName,
                                           _vCoordSvcName ) ;
      BSONObjBuilder bob ;
//      BSONObj retObj ;

      // create virtual coord
      rc = createVCoord.createVirtualCoord( hasVCoordStart ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to create virtual coord, rc = %d", rc ) ;
         goto error ;
      }
      // create install db task
      pTask = SDB_OSS_NEW _omaInstallDBBusinessTask( taskID ) ;
      if ( !pTask )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG( PDERROR,
                     "Failed to create install db business task, rc = %d",
                     rc ) ;
         goto error ;
      }
      // register install db task
      pTaskMgr->addTask( pTask ) ;
      // start install db task
      rc = pTask->init( _coord, _catalog, _data,
                        _localHostName, _omaSvcName, _vCoordSvcName ) ;
      if ( rc  )
      {
         PD_LOG_MSG( PDERROR,
                     "Failed to init install db busniness task, rc = %d",
                     rc ) ;
         goto error ;
      }
      rc = pTask->doit() ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR,
                     "Failed to do db busniness task, rc = %d", rc ) ;
         goto error ;
      }
      // return taskID
//      bob.append( OMA_FIELD_RC, rc ) ;
//      bob.append( OMA_FIELD_DETAIL, "" ) ;
      bob.append( OMA_FIELD_TASKID, (SINT64)taskID ) ;
      objRet = bob.obj() ;

   done:
/*
      if ( hasVCoordStart )
      {
         BOOLEAN hasVCoordRemove = FALSE ;
         _omaRemoveVirtualCoord removeVCoord( _localHostName, _omaSvcName,
                                              _vCoordSvcName ) ;
         rc = removeVCoord.removeVirtualCoord ( hasVCoordRemove ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to remove virtual coord, rc = %d", rc ) ;
         }
      }
*/
      return rc ;
   error:
      goto done ;
   }

   /******************************* query install status *********************/
   /*
      _omaInstallDBStatus
   */
   _omaInstallDBStatus::_omaInstallDBStatus ()
   {
      _scope = NULL ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
      _taskID = OMA_INVALID_TASKID ;
      _taskMrg = NULL ;
   }

   _omaInstallDBStatus::~_omaInstallDBStatus ()
   {
   }

   INT32 _omaInstallDBStatus::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // parse bson to get task id
      BSONElement ele ;
      BSONObj arg( pInstallInfo ) ;
      try
      {
         ele = arg.getField ( OMA_FIELD_TASKID ) ;
         if ( NumberLong != ele.type() )
         {
            rc = SDB_UNEXPECTED_RESULT ;
            PD_LOG_MSG ( PDERROR, "Failed to get taskID, rc = %d", rc ) ;
            goto error ;
         }
         _taskID = ele.numberLong () ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG_MSG ( PDERROR,
                      "Failed to get taskID, received unexpected error: %s",
                      e.what() ) ;
         goto error ;
      }
      // get task manager
      _taskMrg = getTaskMgr() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBStatus::doit ( BSONObj &objRet )
   {
      INT32 rc = SDB_OK ;
      _omaTask *pTask  = NULL ;
      _omaInstallDBBusinessTask *pChildTask = NULL ;
      pTask = _taskMrg->findTask( _taskID ) ;

      if ( ( pChildTask = dynamic_cast<_omaInstallDBBusinessTask*>(pTask) ) )
      {
         pChildTask->getInstallStatus( objRet ) ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG_MSG ( PDERROR, "Failed to get install db progress" ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // _omaCreateVirtualCoord
   _omaCreateVirtualCoord::_omaCreateVirtualCoord ( const CHAR *omaHostName,
                                                    const CHAR *omaSvcName,
                                                    const CHAR *vCoordSvcName )
   {
      _omaHostname = omaHostName ;
      _omaSvcName = omaSvcName ;
      _vCoordSvcName = vCoordSvcName ;
   }

   _omaCreateVirtualCoord::~_omaCreateVirtualCoord ()
   {
   }

   INT32 _omaCreateVirtualCoord::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // set js file
      rc = setJSFile( FILE_CREATE_VIRTUAL_COORD ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // build js arguments
      ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                   " var OMA_HOST_NAME = \"%s\"; var OMA_SVC_NAME = \"%s\"; "
                   "var V_COORD_SVC_NAME = \"%s\"; ",
                   _omaHostname, _omaSvcName, _vCoordSvcName ) ;

      PD_LOG ( PDDEBUG, "Create virtual coord passes arguments: "
               "var OMA_HOST_NAME = %s; var OMA_SVC_NAME = %s; "
               "var V_COORD_SVC_NAME = %s; ",
               _omaHostname, _omaSvcName, _vCoordSvcName ) ;

      _content.clear() ;
      _content += _jsFileArgs ;
      _content += OSS_NEWLINE ;
      _content += _fileBuff ;

      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaCreateVirtualCoord::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      BSONObj subObj ;

      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR,
                      "Failed to eval js file[%s]: %s, rc = %d",
                      _jsFileName, detail.toString().c_str(), rc ) ;
         goto error ;
      }
      rc = omaGetObjElement( rval, "", subObj ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc =%d", "", rc ) ;
         goto error ;
      }
      retObj = subObj.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateVirtualCoord::createVirtualCoord( BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      INT32 retRc = SDB_OK ;
      BSONObj retObj ;

      rc = init( NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR,
                  "Failed to init for creating virtual coord, rc = %d", rc ) ;
         goto error ;
      }
      rc = doit( retObj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR,
                  "Failed to create virtual coord, rc = %d", rc ) ;
         goto error ;
      }
      // extract return rc
      rc = omaGetIntElement ( retObj, OMA_FIELD_RC, retRc ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_RC, rc ) ;
         goto error ;
      }
      if ( retRc )
      {
         PD_LOG_MSG( PDERROR, "Omagent failed to create virtual "
                     "coord, rc = %d", retRc ) ;
         result = FALSE ;
         goto done;
      }
      result = TRUE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // _omaRemoveVirtualCoord
   _omaRemoveVirtualCoord::_omaRemoveVirtualCoord ( const CHAR *omaHostName,
                                                    const CHAR *omaSvcName,
                                                    const CHAR *vCoordSvcName )
   {
      _omaHostName = omaHostName ;
      _omaSvcName = omaSvcName ;
      _vCoordSvcName = vCoordSvcName ;
   }

   _omaRemoveVirtualCoord::~_omaRemoveVirtualCoord ()
   {
   }

   INT32 _omaRemoveVirtualCoord::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // get js file
      rc = setJSFile( FILE_REMOVE_VIRTUAL_COORD ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // build js arguments
      ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                   " var OMA_HOST_NAME = \"%s\"; var OMA_SVC_NAME = \"%s\"; "
                   "var V_COORD_SVC_NAME = \"%s\"; ",
                   _omaHostName, _omaSvcName, _vCoordSvcName ) ;

      PD_LOG ( PDDEBUG, "Remove virtual coord passes arguments: "
                        "var OMA_HOST_NAME = %s; var OMA_SVC_NAME = %s; "
                        "var V_COORD_SVC_NAME = %s; ",
                        _omaHostName, _omaSvcName, _vCoordSvcName ) ;

      _content.clear() ;
      _content += _jsFileArgs ;
      _content += OSS_NEWLINE ;
      _content += _fileBuff ;

      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaRemoveVirtualCoord::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      BSONObj tmpObj ;

      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to eval js file[%s]: %s, rc = %d ",
                      _jsFileName, detail.toString().c_str(), rc ) ;
         goto error ;
      }
      rc = omaGetObjElement( rval, "", tmpObj ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d", "", rc ) ;
         goto error ;
      }
      retObj = tmpObj.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaRemoveVirtualCoord::removeVirtualCoord( BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      INT32 retRc = SDB_OK ;
      BSONObj retObj ;

      rc = init( NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to init for creating virtual coord, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      rc = doit( retObj ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to create virtual coord, rc = %d", rc ) ;
         goto error ;
      }
      // extract return rc
      rc = omaGetIntElement ( retObj, OMA_FIELD_RC, retRc ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc= %d",
                      OMA_FIELD_RC, rc ) ;
         goto error ;
      }
      if ( retRc )
      {
         PD_LOG_MSG( PDERROR,
                     "Omagent failed to remove virtual coord, rc = %d",
                     retRc ) ;
         result = FALSE ;
         rc = retRc ;
         goto error;
      }
      result = TRUE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   //   _omaGetRemoteAgentStatus
   _omaGetRemoteAgentStatus::_omaGetRemoteAgentStatus ()
   {
   }

   _omaGetRemoteAgentStatus::~_omaGetRemoteAgentStatus ()
   {
   }

   INT32 _omaGetRemoteAgentStatus::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInstallInfo ) ;
      ele = arg.getField ( OMA_FIELD_HOSTS ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
      }
      // get js file
      rc = setJSFile( FILE_GET_REMOTE_AGENT_STATUS ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file: %s, rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaGetRemoteAgentStatus::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;
      while ( itr != _hosts.end() )
      {
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;

         const CHAR *pIp = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;

         // get fields
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                     OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_USER, &pUserName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                     OMA_FIELD_USER, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &pPassword ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                     OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }

         // build argument for js file
         ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                      "var IP = \"%s\"; var USERNAME = \"%s\"; "
                      "var PASSWORD = \"%s\";",
                      pIp, pUserName, pPassword ) ;
         PD_LOG ( PDDEBUG, "Argument for check remote progress is: %s",
                  _jsFileArgs ) ;
         _content.clear() ;
         _content += _jsFileArgs ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                         "Failed to eval js file[%s]: %s, rc = %d",
                         _jsFileName, detail.toString().c_str(), rc ) ;
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_IP, pIp ) ;
            bob.append( OMA_FIELD_RC, rc ) ;
            bob.append( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         rc = omaGetObjElement( rval, "", subObj ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d", "", rc ) ;
            goto error ;
         }
         bob.append( OMA_FIELD_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;
      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_HOSTINFO,
                       bab.arr() ) ;
      retObj = bob.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }


   INT32 _omaGetRemoteAgentStatus::getStatus ( const CHAR *pIp,
                                               const CHAR *pUserName,
                                               const CHAR *pPassword,
                                               BSONObj &result )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONObj host ;
      BSONObj subObj ;
      BSONObj retObj ;
      if ( !pIp || !pUserName || !pPassword )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
         goto error ;
      }
      subObj = BSON( OMA_FIELD_IP << pIp <<
                     OMA_FIELD_USER << pUserName <<
                     OMA_FIELD_PASSWD << pPassword ) ;
      bab.append( subObj ) ;
      bob.appendArray( OMA_FIELD_HOSTS, bab.arr() ) ;
      host = bob.obj() ;
      // init
      rc = init ( host.objdata() ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to init get remote agent status" ) ;
         goto error ;
      }
      // doit
      rc = doit ( retObj ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to execute get remote agent status" ) ;
         goto error ;
      }
      // extract result for BSONArray
      {
      BSONElement ele ;
      ele = retObj.getField ( OMA_FIELD_HOSTINFO ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_SYS ;
               PD_LOG_MSG ( PDERROR, "Wrong bson type" ) ;
               goto error ;
            }
            result = ele.embeddedObject() ;
            break ;
         }
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG_MSG ( PDERROR,
                      "Failed to get remote agent status, "
                      "for wrong bson type" ) ;
         goto error ;
      }
      }

      done:
         return rc ;
      error:
         goto done ;
   }

   // _omaPort
   _omaPort::_omaPort ( INT32 port )
   {
      _scope = NULL ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
      _port = port ;
   }

   _omaPort::~_omaPort ()
   {
   }

   INT32 _omaPort::init()
   {
      INT32 rc = SDB_OK ;
      // get js file
      rc = setJSFile( FILE_GET_PORT_STATUS ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaPort::doit( BOOLEAN &hasUsed )
   {
      INT32 rc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      BSONObj subObj ;

      // build js arguments
      ossSnprintf( _jsFileArgs, JS_ARG_LEN, " var PORT = \"%d\";", _port ) ;
      PD_LOG ( PDDEBUG, "Get port status passes arguments: %s",
               _jsFileArgs ) ;

      _content.clear() ;
      _content += _jsFileArgs ;
      _content += OSS_NEWLINE ;
      _content += _fileBuff ;

      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;

      if ( rc )
      {
         PD_LOG_MSG ( PDERROR,
                      "Failed to eval js file[%s]: %s, rc = %d",
                      _jsFileName, detail.toString().c_str(), rc ) ;
         goto error ;
      }
      // get result
      rc = omaGetObjElement( rval, "", subObj ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Get field[%s] failed, rc: %d", "", rc ) ;
         goto error ;
      }
      rc = omaGetBooleanElement ( subObj, OMA_FIELD_PORTHASUSED, hasUsed ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                      OMA_FIELD_PORTHASUSED, rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaPort::getPortStatus( INT32 port, BOOLEAN hasUsed )
   {
      INT32 rc= SDB_OK ;
      _setPort( port ) ;
      rc = init() ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to init for get port %d status, rc = %d",
                     port, rc ) ;
         goto error ;
      }
      rc = doit( hasUsed ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR,
                     "Failed to get port %d status, rc = %d", port,  rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }


   // _omaRegHosts
   _omaRegHosts::_omaRegHosts ()
   {
   }

   _omaRegHosts::~_omaRegHosts ()
   {
   }

   INT32 _omaRegHosts::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInstallInfo ) ;
      ele = arg.getField ( OMA_FIELD_HOSTS ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
      }
      // get js file
      rc = setJSFile( FILE_REG_HOSTS_INFO ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

      // prepare host table info
      rc = _getHostsTableInfo() ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to get hosts table info, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaRegHosts::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;
      while ( itr != _hosts.end() )
      {
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;
         std::vector<string> hostsInfo ;
         std::vector<string>::iterator it_h ;

         const CHAR *pIp       = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;
         string info  = HOSTS_FILE_PROMPT ;

         // get filed
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc= %d",
                         OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_USER, &pUserName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc= %d",
                         OMA_FIELD_USER, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &pPassword ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc= %d",
                         OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }
         // get hosts table info for js file to append
         rc = _getHostsToReg( pIp, hostsInfo ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                         "Faild to get hosts table info for js file, rc = %d",
                         rc ) ;
            goto error ;
         }
         for ( it_h = hostsInfo.begin();
               it_h != hostsInfo.end(); it_h++ )
         {
            // TODO: tanzhabo
            info += "\\n" ;
            info += *it_h ;
//            info += OSS_NEWLINE ;
         }
         // build up argument for js file
         ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                      " var IP = \"%s\"; var USERNAME = \"%s\"; "
                      "var PASSWORD = \"%s\"; var HOSTSINFO = \"%s\"; ",
                      pIp, pUserName, pPassword, info.c_str() ) ;

         PD_LOG ( PDDEBUG, "Reg hosts info passes arguments: %s",
                  _jsFileArgs ) ;

         _content.clear() ;
         _content += _jsFileArgs ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file[%s]: "
                         "%s, rc = %d",
                         _jsFileName, detail.toString().c_str(), rc ) ;
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_IP, pIp ) ;
            bob.append( OMA_FIELD_RC, rc ) ;
            bob.append( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         rc = omaGetObjElement( rval, "", subObj ) ;
         if ( rc )
         {
            PD_LOG_MSG( PDERROR, "Get field[%s] failed, rc = %d", "", rc ) ;
            goto error ;
         }
         bob.append( OMA_FIELD_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;
      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_HOSTINFO, bab.arr() ) ;
      retObj = bob.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaRegHosts::_getHostsTableInfo ()
   {
      INT32 rc              = SDB_OK ;
      const CHAR *pIp       = NULL ;
      const CHAR *pUserName = NULL ;
      const CHAR *pPassword = NULL ;
      // TODO: tanzhaobo
//      _omaGetHostNames ghn ;
      std::vector<BSONObj>::iterator it = _hosts.begin() ;
      if ( 0 == _hosts.size() )
         goto done ;
      while ( it != _hosts.end() )
      {
         BSONObj hostName ;
         BSONObj host = *it ;
         BSONObj tmp ;
         BSONElement ele ;
         _omaGetHostNames ghn ;

         const CHAR *pHostName1 = NULL ;
         it++ ;
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_USER, &pUserName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_USER, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &pPassword ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                         OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }
         // get remote host name by ip
         rc = ghn.getHostName( pIp, pUserName, pPassword, hostName ) ;
         if ( rc )
         {
            PD_LOG_MSG( PDERROR, "Field to get host name from %s, rc = %d",
                        pIp, rc ) ;
            goto error ;
         }
         // extract the hostname
         ele = hostName.getField ( OMA_FIELD_HOSTINFO ) ;
         if ( Array == ele.type() )
         {
            BSONObjIterator itr( ele.embeddedObject() ) ;
            while ( itr.more() )
            {
               ele = itr.next() ;
               if ( Object != ele.type() )
               {
                  rc = SDB_INVALIDARG ;
                  PD_LOG_MSG ( PDERROR, "Unexpected bson type" ) ;
                  goto error ;
               }
               BSONObj temp = ele.embeddedObject() ;

               rc = omaGetStringElement( temp, OMA_FIELD_HOSTNAME,
                                         &pHostName1 ) ;
               PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                            OMA_FIELD_HOSTNAME, rc ) ;
               goto error ;
               break ;
            }
         }
         else
         {
            rc = SDB_INVALIDARG ;
            PD_LOG_MSG( PDERROR, "Failed to get host name" ) ;
            goto error ;
         }
         // save hostname info
         _hostsTableInfo.insert( std::pair<string, string>( string(pIp),
                                 pHostName1 ) ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaRegHosts::_getHostsToReg ( const CHAR *pIp,
                                        std::vector<string> &hostsInfo )
   {
      INT32 rc = SDB_OK ;
      string str = "" ;
      std::map<string, string> temp( _hostsTableInfo ) ;
      SDB_ASSERT( pIp, "Ip can't be NULL" ) ;
      std::map<string, string>::iterator it = temp.find( string(pIp) ) ;
      if ( temp.end() == it )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "No host name info for ip %s", pIp ) ;
         goto error ;
      }
      temp.erase( it ) ;
      // get host info
      hostsInfo.clear() ;
      for ( it = temp.begin(); it != temp.end(); it++ )
      {
         str.clear() ;
         str += it->first ;
         str += "   " ;
         str += it->second ;
         hostsInfo.push_back( str ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   // _omaGetHostNames
   _omaGetHostNames::_omaGetHostNames ()
   {
   }

   _omaGetHostNames::~_omaGetHostNames ()
   {
   }

   INT32 _omaGetHostNames::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInstallInfo ) ;
      ele = arg.getField ( OMA_FIELD_HOSTS ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
      }
      // get js file
      rc = setJSFile( FILE_GET_HOST_NAME ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to read js file[%s], rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }


   INT32 _omaGetHostNames::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;
      while ( itr != _hosts.end() )
      {
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;

         const CHAR *pIp       = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;

         // get fields
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                         OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_USER, &pUserName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                         OMA_FIELD_USER, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_PASSWD, &pPassword ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
                         OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }

         // build argument for js file
         ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                      " var IP = \"%s\"; var USERNAME = \"%s\"; "
                      "var PASSWORD = \"%s\"; ",
                      pIp, pUserName, pPassword ) ;
         PD_LOG ( PDDEBUG, "Get host info passes arguments: %s",
                  _jsFileArgs ) ;

         _content.clear() ;
         _content += _jsFileArgs ;
         _content += OSS_NEWLINE ;
         // TODO: tanzhabo
//         _content += " " ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file[%s]: %s, rc = %d",
                         _jsFileName, detail.toString().c_str(), rc ) ;
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_IP, pIp ) ;
            bob.append( OMA_FIELD_RC, rc ) ;
            bob.append( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         rc = omaGetObjElement( rval, "", subObj ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d", "", rc ) ;
            goto error ;
         }
         bob.append( OMA_FIELD_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;
      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_HOSTINFO, bab.arr() ) ;
      retObj = bob.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaGetHostNames::getHostName ( const CHAR *pIp,
                                         const CHAR *pUserName,
                                         const CHAR *pPassword,
                                         BSONObj &result )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONObj host ;
      BSONObj subObj ;
      if ( !pIp || !pUserName || !pPassword )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG ( PDERROR, "Invalid argument" ) ;
         goto error ;
      }
      subObj = BSON( OMA_FIELD_IP << pIp <<
                     OMA_FIELD_USER << pUserName <<
                     OMA_FIELD_PASSWD << pPassword ) ;
      bab.append( subObj ) ;
      bob.appendArray( OMA_FIELD_HOSTS, bab.arr() ) ;
      host = bob.obj() ;
      // init
      rc = init ( host.objdata() ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to get remote host name" ) ;
         goto error ;
      }
      // doit
      rc = doit ( result ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to get remote host name" ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // _omaAddHostRollbackInternal
   _omaAddHostRollbackInternal::_omaAddHostRollbackInternal()
   {
   }

   _omaAddHostRollbackInternal::~_omaAddHostRollbackInternal()
   {
   }

   INT32 _omaAddHostRollbackInternal::init( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      // get js file
      rc = setJSFile( FILE_ADDHOST_ROLLBACK_INTERNAL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff,
                      &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read js file[%s], rc = %d",
                  _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = sdbGetOMAgentMgr()->getScope() ;
      if ( !_scope )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }
 
   INT32 _omaAddHostRollbackInternal::doit ( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONObj subObj ;
      BSONObj detail ;
      BSONObj rval ;

      // build argument for js file
      ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                   " var IP = \"%s\"; var USERNAME = \"%s\"; "
                   "var PASSWORD = \"%s\"; var INSTALL_PATH = \"%s\" ",
                   _pIp, _pUserName, _pPassword, _pInstallPath ) ;
      PD_LOG ( PDDEBUG, "Execute get host[%s] info", _pIp ) ;
 
      _content.clear() ;
      _content += _jsFileArgs ;
      _content += OSS_NEWLINE ;
      _content += _fileBuff ;
 
      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;
 
      if ( rc )
      {
         PD_LOG ( PDERROR,
                  "Failed to eval js file[%s]: %s, rc = %d",
                   _jsFileName, detail.toString().c_str(), rc ) ;
         goto error ;
      }
      rc = omaGetObjElement( rval, "", subObj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Get field[%s] failed, rc: %d", "", rc ) ;
         goto error ;
      }
      bob.append( OMA_FIELD_IP, _pIp ) ;
      bob.appendElements( subObj ) ;
      retObj = bob.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaAddHostRollbackInternal::rollback ( std::vector<AddHost> &hosts )
   {
      INT32 rc = SDB_OK ;
      std::vector<AddHost>::iterator it = hosts.begin() ;

      rc = init( NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to init for rollback in add host, rc = %d",
                  rc ) ;
         goto error ;
      }
     
      while ( it != hosts.end() ) 
      {
         BSONObj result ;
         INT32 retRc = SDB_OK ;
         const CHAR *pIp = NULL ;
         AddHost host = *it++ ;

         _pIp = host._ip.c_str() ;
         _pUserName = host._userName.c_str() ;
         _pPassword = host._passwd.c_str() ;
         _pInstallPath = host._installPath.c_str() ;   
         rc = doit ( result ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to execute rollback in add host, rc = %d",
                     rc ) ;
            goto error ;
         }
         rc = omaGetIntElement( result, OMA_FIELD_RC, retRc ) ;
         if ( rc )
         {
            PD_LOG ( PDWARNING, "Get field[%s] failed, rc = %d",
                     OMA_FIELD_IP, rc ) ;
            continue ;
         }
         if ( retRc )
         {
            rc = omaGetStringElement( result, OMA_FIELD_IP, &pIp ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                        OMA_FIELD_IP, rc ) ;
               continue ;
            }
            PD_LOG ( PDERROR,
                     "Failed to execute rollback in host[%s], rc = %d",
                     pIp, retRc ) ;
         }
     }

   done:
      return rc ;
   error:
      goto done ;
   }

}

