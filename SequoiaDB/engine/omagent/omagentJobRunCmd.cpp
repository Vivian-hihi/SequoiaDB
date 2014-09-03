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

   Source File Name = omagentJobRunCmd.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentUtil.hpp"
#include "omagentCommand.hpp"
//#include "omagentJobRunCmd.hpp"
#include "utilStr.hpp"
#include "omagentMgr.hpp"

using namespace bson ;

#define FILE_CREATE_CATALOG              "createCatalog.js"
#define FILE_CREATE_COORD                "createCoord.js"
#define FILE_CREATE_DATANODE             "createData.js"

namespace engine
{
   /*
      _omaJobRunCmd
   */
/*
   _omaJobRunCmd::_omaJobRunCmd ()
   {
      _scope      = NULL ;
      _fileBuff   = NULL ;
      _buffSize   = 0 ;
      _readSize   = 0 ;
      ossMemset( _jsFileName, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset( _jsFileArgs, 0, JS_ARG_LEN + 1 ) ;
   }

   _omaJobRunCmd::~_omaJobRunCmd ()
   {
      if ( _scope )
      {
         _scope->shutdown() ;
         SAFE_OSS_DELETE ( _scope ) ;
      }
      if ( _fileBuff )
      {
         SAFE_OSS_FREE ( _fileBuff ) ;
      }
   }

   INT32 _omaJobRunCmd::setJSFile( const CHAR *fileName )
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
*/

   /*
      _omaJobRunInstallCatalogCmd
   */
   _omaJobRunInstallCatalogCmd::_omaJobRunInstallCatalogCmd(
                                                      InstallInfo &info )
   {
      _info._hostName = info._hostName ;
      _info._svcName = info._svcName ;
      _info._dbPath = info._dbPath ;
      _info._confPath = info._dataGroupName ;
      _info._conf = info._conf.getOwned() ;
   }

   _omaJobRunInstallCatalogCmd::~_omaJobRunInstallCatalogCmd()
   {
   }

   INT32 _omaJobRunInstallCatalogCmd::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
/*
      BSONObj arg( pInstallInfo ) ;
      BSONObj conf ;
      BSONObj pattern ;

      // _dataGroupName
      rc = omaGetStringElement( arg, OMA_OPTION_DATAGROUPNAME,
                                &_info._dataGroupName ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d",
                OMA_OPTION_DATAGROUPNAME, rc ) ;
      // _hostname
      rc = omaGetStringElement( arg, OMA_FIELD_HOSTNAME, &_info._hostName ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME, rc ) ;
      // _svcName
      rc = omaGetStringElement( arg, OMA_OPTION_SVCNAME, &_info._svcName ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_SVCNAME, rc ) ;
      // _dbPath
      rc = omaGetStringElement( arg, OMA_OPTION_DBPATH, &_info._dbPath ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_DBPATH, rc ) ;
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME << 1 <<
                      OMA_OPTION_DBPATH << 1 ) ;
      conf = (*it).filterFieldsUndotted( pattern, false ) ;
      _info._conf = conf.getOwned() ;
*/
      // set js file
      rc = setJSFile( FILE_CREATE_CATALOG ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to set js file[%s], rc = %d",
                      FILE_CREATE_CATALOG, rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff, &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read js file: %s, rc = %d",
                  _jsFileName, rc ) ;
         goto error ;
      }
      // build js arguments
      ossSnprintf( _jsFileArgs, JS_ARG_LEN,
                   " var INSTALL_HOSTNAME = \'%s\'; "
                   "var INSTALL_SERVICE = \'%s\'; "
                   "var INSTALL_PATH = \'%s\'; var CONFIG = \'%s\'; ",
                   _info._hostName.c_str(), _info._svcName.c_str(),
                   _info._dbPath.c_str(), _info._conf.toString().c_str() ) ;
  
      PD_LOG ( PDDEBUG, "Create catalog passes arguments: "
               "var INSTALL_HOSTNAME = %s; var INSTALL_SERVICE = %s; "
               "var INSTALL_PATH = %s; var CONFIG = %s;",
               _info._hostName.c_str(), _info._svcName.c_str(),
               _info._dbPath.c_str(),
               _info._conf.toString().c_str() ) ;
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
   error:
      goto done ;
   }

   INT32 _omaJobRunInstallCatalogCmd::doit ( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
//      INT32 retRc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      BSONObj subObj ;

      // execute js
      rc = _scope->eval( _content.c_str(), _content.size(),
                         _jsFileName, 1, 1, rval, detail ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR,
                  "Failed to eval js file: %s, rc = %d, errmsg = %s",
                  _jsFileName, rc, detail.toString().c_str() ) ;
         BSONObjBuilder bob ;
         bob.append ( OMA_FIELD_RC, rc ) ;
         bob.append ( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
         retObj = bob.obj() ;
         goto error ;
      }
      // extract subObj
      // TODO: tanzhaobo
      // how to deal with this kill of error
      rc = omaGetObjElement( rval, "", subObj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", "", rc ) ;
      retObj = subObj.getOwned() ;
/*
      // extract return rc
      rc = omaGetIntElement ( subObj, OMA_FIELD_RC, retRc ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Get field[%s] failed, rc: %d",
                  OMA_FIELD_RC, rc ) ;
         goto error ;
      }
      if ( retRc )
      {
         rc = retRc ;
         PD_LOG_MSG ( PDERROR, "Failed to install catalog[%s:%s]",
                      _host._hostName, _host._svcName ) ;
         goto error;
      }
*/
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _omaJobRunInstallCoordCmd
   */
   _omaJobRunInstallCoordCmd::_omaJobRunInstallCoordCmd( InstallInfo &info )
   {
      _info._hostName = info._hostName ;
      _info._svcName = info._svcName ;
      _info._dbPath = info._dbPath ;
      _info._confPath = info._dataGroupName ;
      _info._conf = info._conf.getOwned() ;
   }

   _omaJobRunInstallCoordCmd::~_omaJobRunInstallCoordCmd()
   {
   }

   INT32 _omaJobRunInstallCoordCmd::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
/*
      std::vector<BSONObj>::iterator it = objs.begin() ;
      while( it != objs.end() )
      {
         InstallInfo info ;
         BSONObj conf ;
         BSONObj pattern ;

         // _dataGroupName
         rc = omaGetStringElement( *it, OMA_OPTION_DATAGROUPNAME,
                                       &info._dataGroupName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d",
                   OMA_OPTION_DATAGROUPNAME, rc ) ;
         // _hostname
         rc = omaGetStringElement( *it, OMA_FIELD_HOSTNAME,
                                       &info._hostName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME, rc ) ;
         // _svcName
         rc = omaGetStringElement( *it, OMA_OPTION_SVCNAME,
                                       &info._svcName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_OPTION_SVCNAME, rc ) ;
         // _dbPath
         rc = omaGetStringElement( *it, OMA_OPTION_DBPATH,
                                       &info._dbPath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_OPTION_DBPATH, rc ) ;

//         // _confPath
//         rc = omaGetStringElement( *it, OMA_OPTION_CONFPATH,
//                                       &info._confPath ) ;
//         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
//                   "Get field[%s] failed, rc: %d", OMA_OPTION_CONFPATH, rc ) ;

         // _conf
         pattern = BSON( OMA_FIELD_HOSTNAME << 1 <<
                         OMA_OPTION_DATAGROUPNAME << 1 <<
                         OMA_OPTION_SVCNAME << 1 <<
                         OMA_OPTION_DBPATH << 1 ) ;
         conf = (*it).filterFieldsUndotted( pattern, false ) ;
         info._conf = conf ;
         // save info
         _installInfos.push_back( info ) ;
         // get next install info
         it++ ;
      }
*/
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff, &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read js file: %s, rc = %d",
                  _jsFileName, rc ) ;
         goto error ;
      }
      // set js file
      rc = setJSFile( FILE_CREATE_COORD ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to set js file[%s], rc = %d", rc ) ;
         goto error ;
      }
      // build js arguments
      ossSnprintf( tempBuff, JS_ARG_LEN,
                   " var INSTALL_HOSTNAME = \'%s\'; "
                   "var INSTALL_SERVICE = \'%s\'; "
                   "var INSTALL_PATH = \'%s\'; var CONFIG = \'%s\'; ",
                   _info._hostName.c_str(), _info._svcName.c_str(),
                   _info._dbPath.c_str(), _info._conf.toString().c_str() ) ;

      PD_LOG ( PDDEBUG, "Create coord passes arguments: "
                        "var INSTALL_HOSTNAME = %s; "
                        "var INSTALL_SERVICE = %s;  "
                        "var INSTALL_PATH = %s; var CONFIG = %s;",
                        _info._hostName.c_str(), _info._svcName.c_str(),
                        _info._dbPath.c_str(),
                        _info._conf.toString().c_str() ) ;
      _content.clear() ;
      _content += tempBuff ;
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
   error:
      goto done ;
   }

   INT32 _omaJobRunInstallCoordCmd::doit ( BSONObj &retObj )
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
         PD_LOG ( PDERROR,
                  "Failed to eval js file: %s, rc = %d, errmsg = %s",
                  _jsFileName, rc, detail.toString().c_str() ) ;
         BSONObjBuilder bob ;
         bob.append ( OMA_FIELD_RC, rc ) ;
         bob.append ( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
         retObj = bob.obj() ;
         goto error ;
      }
      // extract subObj
      rc = omaGetObjElement( rval, "", subObj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", "", rc ) ;
      retObj = subObj.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }


   /*
      _omaJobRunInstallDataCmd
   */
   _omaJobRunInstallDataCmd::_omaJobRunInstallDataCmd( InstallInfo &info )
   {
      _info._hostName = info._hostName ;
      _info._svcName = info._svcName ;
      _info._dbPath = info._dbPath ;
      _info._dataGroupName = info._dataGroupName ;
      _info._conf = info._conf.getOwned() ;
   }

   _omaJobRunInstallDataCmd::~_omaJobRunInstallDataCmd()
   {
   }

   INT32 _omaJobRunInstallDataCmd::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
/*
      std::vector<BSONObj>::iterator it = objs.begin() ;
      while( it != objs.end() )
      {
         InstallInfo info ;
         BSONObj conf ;
         BSONObj pattern ;

         // _dataGroupName
         rc = omaGetStringElement( *it, OMA_OPTION_DATAGROUPNAME,
                                       &info._dataGroupName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d",
                   OMA_OPTION_DATAGROUPNAME, rc ) ;
         // _hostname
         rc = omaGetStringElement( *it, OMA_FIELD_HOSTNAME,
                                       &info._hostName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME, rc ) ;
         // _svcName
         rc = omaGetStringElement( *it, OMA_OPTION_SVCNAME,
                                       &info._svcName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_OPTION_SVCNAME, rc ) ;
         // _dbPath
         rc = omaGetStringElement( *it, OMA_OPTION_DBPATH,
                                       &info._dbPath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_OPTION_DBPATH, rc ) ;

//         // _confPath
//         rc = omaGetStringElement( *it, OMA_OPTION_CONFPATH,
//                                       &info._confPath ) ;
//         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
//                   "Get field[%s] failed, rc: %d", OMA_OPTION_CONFPATH, rc ) ;

         // _conf
         pattern = BSON( OMA_FIELD_HOSTNAME << 1 <<
                         OMA_OPTION_DATAGROUPNAME << 1 <<
                         OMA_OPTION_SVCNAME << 1 <<
                         OMA_OPTION_DBPATH << 1 ) ;
         conf = (*it).filterFieldsUndotted( pattern, false ) ;
         info._conf = conf.getOwned() ;
         // save info
         _installInfos.push_back( info ) ;
         // get next install info
         it++ ;
      }
*/
      // set js file
      rc = setJSFile( FILE_CREATE_DATANODE ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to set js file[%s], rc = %d", rc ) ;
         goto error ;
      }
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff, &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read js file: %s, rc = %d",
                  _jsFileName, rc ) ;
         goto error ;
      }
      // build js arguments
      ossSnprintf( tempBuff, JS_ARG_LEN,
                   " var GROUPNAME = \'%s\'; "
                   "var INSTALL_HOSTNAME = \'%s\'; "
                   "var INSTALL_SERVICE = \'%s\';  "
                   "var INSTALL_PATH = \'%s\'; var CONFIG = \'%s\'; ",
                   _info._dataGroupName.c_str(),
                   _info._hostName.c_str(),
                   _info._svcName.c_str(),
                   _info._dbPath.c_str(),
                   _info._conf.toString().c_str() ) ;

      PD_LOG ( PDDEBUG, "Create data node passes arguments: "
               "groupname = %s; hostname = %s; svcname = %s; "
               "dbpath = %s; config = %s;",
               _info._dataGroupName.c_str(),
               _info._hostName.c_str(),
               _info._svcName.c_str(),
               _info._dbPath.c_str(),
               _info._conf.toString().c_str() ) ;
      _content.clear() ;
      _content += tempBuff ;
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
   error:
      goto done ;
   }

   INT32 _omaJobRunInstallDataCmd::doit ( BSONObj &retObj )
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
         PD_LOG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                  _jsFileName, rc, detail.toString().c_str() ) ;
         BSONObjBuilder bob ;
         bob.append ( OMA_FIELD_RC, rc ) ;
         bob.append ( OMA_FIELD_DETAIL, detail.toString().c_str() ) ;
         retObj = bob.obj() ;
         goto error ;
      }
      // extract subObj
      // TODO: tanzhaobo
      // how to deal with this kill of error
      rc = omaGetObjElement( rval, "", subObj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", "", rc ) ;
      retObj = subObj.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }


}


