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
#include "sptContainer.hpp"
#include "ossProc.hpp"
#include "utilPath.hpp"
#include "ossPath.h"
#include "omagentJob.hpp"
#include "omagentMgr.hpp"

using namespace bson ;

#define HOSTS_FILE_PROMPT "##############add by omagent##############"
#define DEF_VIRTUAL_COORD_SERVICE 10810

#define FILE_SCAN_HOST                   "scanHost.js"
#define FILE_BASIC_CHECK_HOST            "basicCheckHost.js"
#define FILE_INSTALL_REMOTE_AGENT        "installRemoteAgent.js"
#define FILE_CHECK_HOST                  "checkHost.js"
#define FILE_EXIT_AGENT                  "exitAgent.js"
#define FILE_UNINSTALL_REMOTE_AGENT      "uninstallRemoteAgent.js"
#define FILE_ADD_HOST                    "addHost.js"

#define FILE_GET_REMOTE_AGENT_STATUS     "getRemoteAgentStatus.js"
#define FILE_CREATE_VIRTUAL_COORD        "createVirtualCoord.js"
#define FILE_REMOVE_VIRTUAL_COORD        "removeVirtualCoord.js"
#define FILE_GET_PORT_STATUS             "getPortStatus.js"
#define FILE_REG_HOSTS_INFO              "regHostsInfo.js"
#define FILE_GET_HOST_NAME               "getHostName.js"

#define ROLE_COORD "coord"
#define ROLE_CATA  "catalog"
#define ROLE_DATA  "data"

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
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaExitAgent )
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
   }

   _omaCommand::~_omaCommand ()
   {
      if ( _scope )
      {
         _scope->shutdown() ;
         SAFE_OSS_DELETE ( _scope ) ;
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
         PD_LOG_MSG ( PDERROR, "Failed to build js file full path, rc = " ) ;
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
      // TODO: tanzhaobo
      // do i need to release memory in map ?
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

   void _omaCmdBuilder::release ( const _omaCommand *pCommand )
   {
      if ( pCommand )
      {
         SDB_OSS_DEL pCommand ;
      }
   }

   INT32 _omaCmdBuilder::_register ( const CHAR *name, OA_NEW_FUNC pFunc )
   {
      INT32 rc = SDB_OK ;

      std::pair<MAP_OACMD_IT, BOOLEAN> ret ;
      ret = _cmdMap.insert( std::pair<const CHAR*, OA_NEW_FUNC>(name, pFunc) ) ;
      if ( FALSE == ret.second )
      {
         PD_LOG_MSG ( PDERROR,
                  "Failed to register omagent command %s, already exist",
                   name ) ;
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
         MAP_OACMD_IT it ;
         it = _cmdMap.find( name ) ;
         if ( it != _cmdMap.end() )
            return it->second ;
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

   INT32 _omaScanHost::init ( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
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
         PD_LOG_MSG ( PDERROR, "Failed to read js file: %s, rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
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

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
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
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      "var IP = \'%s\'; var USERNAME = \'%s\'; \
                      var PASSWORD = \'%s\'; var SSHPORT = \'%s\'",
                      pIp, pUserName, pPassWord, pSshPort ) ;

         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                         _jsFileName, rc, detail.toString().c_str() ) ;
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

   INT32 _omaInstallRemoteAgent::init ( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
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
         PD_LOG ( PDERROR, "Failed to set program's or script's path", rc ) ;
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
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
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

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
         const CHAR *pIp = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;
         const CHAR *pVersion   = NULL ;
         BOOLEAN isRunning      = FALSE ;

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
         // check whether the remote machine has install omagent or not
         rc = getRemoteAgentStatus ( pIp, pUserName, pPassword, status ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                     "Failed to get remote mechine's status, rc = %d", rc ) ;
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
                       "Remote omagent's version is: %s, \
and we are going to instll version %s", pVersion, ver ) ;
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
         // build js file's argument
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var IP = \"%s\"; var USERNAME = \"%s\"; \
                      var PASSWORD = \"%s\"; var LOCAL_PROG_PATH = \"%s\"; \
                      var LOCAL_SPT_PATH = \"%s\"; \
                      var LOCAL_CM_CONF = \"%s\" ",
                      pIp, pUserName, pPassword,
                      _prog_path, _spt_path, _conf_path ) ;
         PD_LOG ( PDDEBUG, "Install remote agent passes arguments: \
var IP = \"%s\"; var USERNAME = \"%s\"; var PASSWORD = \"%s\"; \
var LOCAL_PROG_PATH = \"%s\"; var LOCAL_SPT_PATH = \"%s\"; \
var LOCAL_CM_CONF = \"%s\" ",
                  pIp, "xxx", "xxx", _prog_path, _spt_path, _conf_path ) ;

         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                         _jsFileName, rc, detail.toString().c_str() ) ;
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
         PD_LOG_MSG ( PDERROR, "Faled to check remote mechine's status, rc = %d",
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
   }

   _omaCheckHost::~_omaCheckHost ()
   {
   }

   INT32 _omaCheckHost::init( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
      _hosts.push_back ( arg ) ;
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
         PD_LOG_MSG ( PDERROR, "Failed to read js file: %s, rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaCheckHost::doit( BSONObj& retObj )
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

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
         const CHAR *pIp = NULL ;
         const CHAR *pHostName = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;
         // get fields
         rc = omaGetStringElement( host, OMA_FIELD_IP, &pIp ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                     OMA_FIELD_IP, rc ) ;
            goto error ;
         }
         rc = omaGetStringElement( host, OMA_FIELD_HOSTNAME, &pHostName ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                     OMA_FIELD_HOSTNAME, rc ) ;
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
         // build js argument for js file
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var IP = \"%s\"; var HOSTNAME = \"%s\"; \ 
                      var USERNAME = \"%s\"; var PASSWORD = \"%s\"; ",
                      pIp, pHostName, pUserName, pPassword ) ;
         PD_LOG ( PDDEBUG, "Excute check host infomation passes arguments: \
var IP = %s; var HOSTNAME = %s; var USERNAME = %s; var PASSWORD = %s",
                  pIp, pHostName, "xxx", "xxx" ) ;

         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                         _jsFileName, rc, detail.toString().c_str() ) ;
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

   /******************************* exit agent *******************************/
   /*
       _omaExitAgent
   */
   _omaExitAgent::_omaExitAgent ()
   {
   }

   _omaExitAgent::~_omaExitAgent ()
   {
   }

   INT32 _omaExitAgent::init( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONObj arg( pInfomation ) ;
      _hosts.push_back ( arg ) ;
      // get js file
      rc = setJSFile( FILE_EXIT_AGENT ) ;
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
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaExitAgent::doit( BSONObj &retObj )
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

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;

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

         // build argument for js file
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var USERNAME = \"%s\"; var PASSWORD = \"%s\"; ",
                      pUserName, pPassword ) ;
         PD_LOG ( PDDEBUG, "Excute stop sdbcm program command" ) ;

         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                         _jsFileName, rc, detail.toString().c_str() ) ;
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
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

   INT32 _omaUninstallRemoteAgent::init( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
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
         PD_LOG_MSG ( PDERROR, "Failed to read js file: %s, rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
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

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
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
                     "Get field[%s] failed, rc: %d", OMA_FIELD_PASSWD, rc ) ;
            goto error ;
         }

         // build argument for js file
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var IP = \"%s\"; var USERNAME = \"%s\"; \
                      var PASSWORD = \"%s\"; ",
                      pIp, pUserName, pPassword ) ;
         PD_LOG ( PDDEBUG, "Excute uninstall remote sdbcm[ip:%s]", pIp ) ;
         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                         _jsFileName, rc, detail.toString().c_str() ) ;
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
   }

   _omaAddHost::~_omaAddHost ()
   {
      ossMemset ( _packet_path, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset ( _sdb_user, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset ( _sdb_passwd, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset ( _sdb_user_group, 0, OSS_MAX_PATHSIZE + 1 ) ;
   }

   INT32 _omaAddHost::init( const CHAR *pInfomation )
   {
      INT32 rc                  = SDB_OK ;
      const CHAR *pSdbUser      = NULL ;
      const CHAR *pSdbPassword  = NULL ;
      const CHAR *pSdbUserGroup = NULL ;
      // parse bson and get arguments info for js file
      BSONObj arg( pInfomation ) ;
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
      ossStrncpy ( _sdb_user, pSdbUser, OSS_MAX_PATHSIZE ) ;
      ossStrncpy ( _sdb_passwd, pSdbPassword, OSS_MAX_PATHSIZE ) ;
      ossStrncpy ( _sdb_user_group, pSdbUserGroup, OSS_MAX_PATHSIZE ) ;
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
      // TODO: tanzhaobo
      // maybe it need to set another path for install packet
      // set the database install packet's path
      ossStrncpy( _packet_path, sdbGetOMAgentOptions()->getCfgFileName(),
                  OSS_MAX_PATHSIZE ) ;
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
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
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
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
         const CHAR *pIp             = NULL ;
         const CHAR *pHostName       = NULL ;
         const CHAR *pUserName       = NULL ;
         const CHAR *pPassword       = NULL ;
         const CHAR *pInstallPath    = NULL ;

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
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var SDBUSER = \"%s\"; var SDBPASSWD = \"%s\"; \
                      var SDBUSERGROUP = \"%s\"; \
                      var IP = \"%s\"; var HOSTNAME = \"%s\"; \
                      var USERNAME = \"%s\"; var PASSWORD = \"%s\"; \
                      var PACKET_PATH = \"%s\"; var INSTALL_PATH = \"%s\"; ",
                      _sdb_user, _sdb_passwd, _sdb_user_group,
                      pIp, pHostName, pUserName, pPassword,
                      _packet_path, pInstallPath ) ;
         PD_LOG ( PDDEBUG, " Install db business passes arguments: \
var SDBUSER = %s; var SDBPASSWD = %s; var SDBUSERGROUP = %s; var IP = %s; \
var HOSTNAME = %s; var USERNAME = %s; var PASSWORD = %s; \
var PACKET_PATH = %s; var INSTALL_PATH = %s ",
                  _sdb_user, _sdb_passwd, _sdb_user_group,
                  pIp, pHostName,
                  "xxx", "xxx", _packet_path, pInstallPath ) ;
         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                     _jsFileName, rc, detail.toString().c_str() ) ;
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
            PD_LOG_MSG( PDERROR, "Get field[%s] failed, rc: %d", "", rc ) ;
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

   /******************************* add db business **************************/
   // _omaInstallDBBusiness
   _omaInstallDBBusiness::_omaInstallDBBusiness ()
   {
   }

   _omaInstallDBBusiness::~_omaInstallDBBusiness ()
   {
   }

   INT32 _omaInstallDBBusiness::init( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
      ele = arg.getField ( OMA_FIELD_HOSTS ) ;
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
               PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
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
            else
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG( PDERROR,
                       "Failed to install db business for wrong argument %s",
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
      INT32 rc = SDB_OK ;
      _omaCreateVirtualCoord vCoord( "", "" ) ;
      _omaTaskMgr *pTaskMgr = getTaskMgr() ;
      UINT64 taskID = pTaskMgr->getTaskID() ;
      _omaInstallDBBusinessTask *pTask = NULL ;
      BSONObjBuilder bob ;
      BSONObj retObj ;
/*
      // create virtual coord
      rc = vCoord.createVirtualCoord( coord_service, hasVCoordStart ) ;
*/
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
                 "Failed to new install db business task install, rc = %d", rc ) ;
         goto error ;
      }
      // register install db task
      pTaskMgr->addTask( pTask ) ;
      // start install db task
      rc = pTask->init( _coord, _catalog, _data ) ;
      if ( rc  )
      {
         PD_LOG_MSG( PDERROR,
                 "Failed to init install db busniness task, rc = %d", rc ) ;
         goto error ;
      }
      rc = pTask->doit() ;
      {
         PD_LOG_MSG( PDERROR,
                 "Failed to do db busniness task, rc = %d", rc ) ;
         goto error ;
      }
      // return taskID
      bob.append( OMA_FIELD_RC, rc ) ;
      bob.append( OMA_FIELD_DETAIL, "" ) ;
      bob.appendNumber( OMA_FIELD_TASKID, (INT64)taskID ) ;
      retObj = bob.obj() ;

   done:
/*
      // create remove virtual coord task
      if ( hasVCoordStart )
      {
//         pTask2 = SDB_OSS_NEW _omaRemoveVirtualCoordTask( taskID ) ;
         _omaRemoveVirtualCoordTask removeVCoordTask( taskID ) ;
         rc = removeVCoordTask.doit() ;
         if ( rc )
         {
            PD_LOG_MSG( PDERROR, "Failed to remove virtual coord, rc = %d", rc ) ;
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

   INT32 _omaInstallDBStatus::init ( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson to get task id
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
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

      if ( pChildTask = dynamic_cast<_omaInstallDBBusinessTask*>(pTask) )
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
   _omaCreateVirtualCoord::_omaCreateVirtualCoord ( const CHAR *username,
                                                    const CHAR *password )
   {
      _scope = NULL ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
      _username = username ;
      _password = password ;
   }

   _omaCreateVirtualCoord::~_omaCreateVirtualCoord ()
   {
   }

   INT32 _omaCreateVirtualCoord::init()
   {
      INT32 rc = SDB_OK ;
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
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaCreateVirtualCoord::doit( INT32 coord_service,
                                       BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      BSONObj subObj ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
      CHAR prog_agent[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
      CHAR prog_sequoiadb[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

      // get program path
      rc = getProgramPath( prog_agent ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to get omagent program path, rc = %d", rc ) ;
         goto error ;
      }
      rc = ossLocateExecutable ( prog_agent, START_DB_PROG, prog_sequoiadb,
                                 OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to get sequoiadb program path, rc = %d", rc ) ;
         goto error ;
      }

      // build js arguments
      ossSnprintf( tempBuff, JS_ARG_LEN,
                   " var USERNAME = \"%s\"; var PASSWORD = \"%s\"; \
                     var PROGRAM = \"%s\"; var COORD_SERVICE = \"%d\"; ",
                   _username, _password, prog_sequoiadb, coord_service ) ;

      PD_LOG ( PDDEBUG, "Create virtual coord passes arguments: var USERNAME = %s; var PASSWORD = %s; var PROGRAM = %s; var COORD_SERVICE = %d;",
               "xxx", "xxx", prog_sequoiadb, coord_service ) ;

      _content.clear() ;
      _content += tempBuff ;
      _content += OSS_NEWLINE ;
      _content += _fileBuff ;

      // get js file
      rc = setJSFile( FILE_CREATE_VIRTUAL_COORD ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to set js file, rc = %d", rc ) ;
         goto error ;
      }
      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                      _jsFileName, rc, detail.toString().c_str() ) ;
         goto error ;
      }
      rc = omaGetObjElement( rval, "", subObj ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d", "", rc ) ;
         goto error ;
      }
      // extract return rc
      {
      INT32 retRc = SDB_OK ;
      rc = omaGetIntElement ( subObj, OMA_FIELD_RC, retRc ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                  OMA_FIELD_RC, rc ) ;
         goto error ;
      }
      if ( retRc )
      {
         PD_LOG_MSG( PDERROR, "Omagent failed to start virtual coord, rc = %d", retRc ) ;
         goto error;
      }
      }
      result = TRUE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateVirtualCoord::createVirtualCoord( INT32 coord_service,
                                                     BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      rc = init() ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to init for creating virtual coord, rc = %d",
                  rc ) ;
         goto error ;
      }
      rc = doit( coord_service, result ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to create virtual coord, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   // _omaRemoveVirtualCoord
   _omaRemoveVirtualCoord::_omaRemoveVirtualCoord ( const CHAR *username,
                                                    const CHAR *password )
   {
      _scope = NULL ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
      _username = username ;
      _password = password ;
   }

   _omaRemoveVirtualCoord::~_omaRemoveVirtualCoord ()
   {
   }

   INT32 _omaRemoveVirtualCoord::init()
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
         PD_LOG_MSG ( PDERROR, "Failed to read js file: %s, rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to get scope, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaRemoveVirtualCoord::doit( INT32 coord_service,
                                       BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      BSONObj subObj ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
      CHAR prog_agent[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
      CHAR prog_sequoiadb[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

      // get program path
      rc = getProgramPath( prog_agent ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to get omagent program path, rc = %d", rc ) ;
         goto error ;
      }
      rc = ossLocateExecutable ( prog_agent, "sdbstop", prog_sequoiadb,
                                 OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to get sequoiadb program path, rc = %d", rc ) ;
         goto error ;
      }

      // build js arguments
      ossSnprintf( tempBuff, JS_ARG_LEN,
                   " var USERNAME = \"%s\"; var PASSWORD = \"%s\"; \
                     var PROGRAM = \"%s\"; var COORD_SERVICE = \"%d\";  ",
                   _username, _password, prog_sequoiadb, coord_service ) ;

      PD_LOG ( PDDEBUG, "Create virtual coord passes arguments: var USERNAME = %s; var PASSWORD = %s; var PROGRAM = %s; var COORD_SERVICE = %d;",
                        "xxx", "xxx", prog_sequoiadb, coord_service ) ;

      _content.clear() ;
      _content += tempBuff ;
      _content += OSS_NEWLINE ;
      _content += _fileBuff ;

      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                      _jsFileName, rc, detail.toString().c_str() ) ;
         goto error ;
      }
      rc = omaGetObjElement( rval, "", subObj ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d", "", rc ) ;
         goto error ;
      }
      // extract return rc
      {
      INT32 retRc = SDB_OK ;
      rc = omaGetIntElement ( subObj, OMA_FIELD_RC, retRc ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc= %d", OMA_FIELD_RC, rc ) ;
         goto error ;
      }
      if ( retRc )
      {
         PD_LOG_MSG( PDERROR, "Omagent failed to start virtual coord, rc = %d", retRc ) ;
         goto error;
      }
      }
      result = TRUE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaRemoveVirtualCoord::removeVirtualCoord( INT32 coord_service,
                                                     BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      rc = init() ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to init for creating virtual coord, rc = %d",
                  rc ) ;
         goto error ;
      }
      rc = doit( coord_service, result ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to create virtual coord, rc = %d", rc ) ;
         goto error ;
      }
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

   INT32 _omaGetRemoteAgentStatus::init( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
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
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
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

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
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
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      "var IP = \"%s\"; var USERNAME = \"%s\"; var PASSWORD = \"%s\";",
                      pIp, pUserName, pPassword ) ;
         PD_LOG ( PDDEBUG, "Arguments for checkRemoteAgentProcess.js is: %s",
                  tempBuff ) ;
         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                         _jsFileName, rc, detail.toString().c_str() ) ;
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
                      "Failed to get remote agent status, for wrong bson type" ) ;
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
         PD_LOG_MSG ( PDERROR, "Failed to read js file: %s, rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
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
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;

      // build js arguments
      ossSnprintf( tempBuff, JS_ARG_LEN, " var PORT = \"%d\";", _port ) ;
      PD_LOG ( PDDEBUG, "Get port status passes arguments: %s",
               tempBuff ) ;

      _content.clear() ;
      _content += tempBuff ;
      _content += OSS_NEWLINE ;
      _content += _fileBuff ;

      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;

      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                      _jsFileName, rc, detail.toString().c_str() ) ;
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
         PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc: %d",
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
         PD_LOG_MSG( PDERROR, "Failed to get port %d status, rc = %d",port,  rc ) ;
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

   INT32 _omaRegHosts::init( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
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
         PD_LOG_MSG ( PDERROR, "Failed to read js file: %s, rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
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

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
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
         rc = _getContentForJS( pIp, hostsInfo ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR,
                     "Faild to get hosts table info for js file, rc = %d", rc ) ;
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
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var IP = \"%s\"; var USERNAME = \"%s\";\
                      var PASSWORD = \"%s\"; var HOSTSINFO = \"%s\"; ",
                      pIp, pUserName, pPassword, info.c_str() ) ;

         PD_LOG ( PDDEBUG, "Reg hosts info passes arguments: %s",
                  tempBuff ) ;

         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                         _jsFileName, rc, detail.toString().c_str() ) ;
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
            PD_LOG_MSG( PDERROR, "Get field[%s] failed, rc: %d", "", rc ) ;
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

   INT32 _omaRegHosts::_getContentForJS ( const CHAR *pIp,
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

   INT32 _omaGetHostNames::init( const CHAR *pInfomation )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pInfomation ) ;
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
         PD_LOG_MSG ( PDERROR, "Failed to read js file: %s, rc = %d",
                      _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      rc = getSptScope ( &_scope ) ;
      if ( rc )
      {
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

         CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
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
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var IP = \"%s\"; var USERNAME = \"%s\";\
                      var PASSWORD = \"%s\"; ",
                      pIp, pUserName, pPassword ) ;
         PD_LOG ( PDDEBUG, "Get host info passes arguments: %s",
                  tempBuff ) ;

         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         // TODO: tanzhabo
//         _content += " " ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                         _jsFileName, rc, detail.toString().c_str() ) ;
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
}

