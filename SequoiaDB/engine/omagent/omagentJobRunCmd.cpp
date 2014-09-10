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

#define FILE_REMOVE_CATALOG              "removeCatalog.js"
#define FILE_REMOVE_COORD                "removeCoord.js"
#define FILE_REMOVE_DATANODE             "removeData.js"

#define FILE_ROLLBACK_CATALOG            "rollbackCatalog.js"
#define FILE_ROLLBACK_COORD              "rollbackCoord.js"
#define FILE_ROLLBACK_DATANODE           "rollbackDataNode.js"

namespace engine
{
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
      // how to deal with this kind of error
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
         PD_LOG_MSG ( PDERROR, "Failed to set js file[%s], rc = %d",
                      FILE_CREATE_COORD, rc ) ;
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

      // set js file
      rc = setJSFile( FILE_CREATE_DATANODE ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to set js file[%s], rc = %d",
                      FILE_CREATE_DATANODE, rc ) ;
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
      // how to deal with this kind of error
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
      install db business task run rollback coord job
   */

   _omaRunRollbackCoordJob::_omaRunRollbackCoordJob (
                                   string &vCoordHostName,
                                   string &vCoordSvcName,
                                   map< string, vector<InstalledNode> > &info )
   :_info( info )
   {
      _vCoordHostName = vCoordHostName ;
      _vCoordSvcName = vCoordSvcName ;
   }

   _omaRunRollbackCoordJob::~_omaRunRollbackCoordJob ()
   {
   }
   
   INT32 _omaRunRollbackCoordJob::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      BSONObj installedCoord ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;

      // get installed coord node info
      _getInstalledCoordInfo( installedCoord ) ;
      // set js file
      rc = setJSFile( FILE_ROLLBACK_COORD ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to set js file[%s], rc = %d",
                      FILE_ROLLBACK_COORD, rc ) ;
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
                   "var V_COORD_HOSTNAME = \'%s\'; "
                   "var V_COORD_SERVICE = \'%s\'; "
                   "var INSTALLED_COORD = \'%S\' ",
                   _vCoordHostName.c_str(),
                   _vCoordSvcName.c_str(),
                   installedCoord.toString().c_str() ) ;

      PD_LOG ( PDDEBUG, "Create data node passes arguments: "
               "var COORD_HOSTNAME = %s; var COORD_SERVICE = %s;"
               "var INSTALLED_COORD = %s ",
               _vCoordHostName.c_str(), _vCoordSvcName.c_str(),
               installedCoord.toString().c_str() ) ;

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

   INT32 _omaRunRollbackCoordJob::doit ( BSONObj &retObj )
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
      // how to deal with this kind of error
      rc = omaGetObjElement( rval, "", subObj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", "", rc ) ;
      retObj = subObj.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _omaRunRollbackCoordJob::_getInstalledCoordInfo( BSONObj &obj )
   {
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      map< string, vector< InstalledNode > >::iterator it = _info.begin() ;

      for( ; it != _info.end(); it++ )
      {
         vector< InstalledNode > &nodes = it->second ;
         vector< InstalledNode >::iterator itr = nodes.begin() ;
         for( ; itr != nodes.end(); itr++ )
         {
            bab.append( BSON( OMA_FIELD_HOSTNAME << itr->_hostName <<
                              OMA_FIELD_SVCNAME <<  itr->_svcName ) ) ;
         }
      }
      bob.appendArray( OMA_FIELD_HOSTS, bab.arr() ) ;
      obj = bob.obj() ;
   }

   /*
      install db business task run rollback catalog job
   */
   _omaRunRollbackCatalogJob::_omaRunRollbackCatalogJob (
                                   string &vCoordHostName,
                                   string &vCoordSvcName,
                                   map< string, vector<InstalledNode> > &info )
   : _info( info )
   {
      _vCoordHostName = vCoordHostName ;
      _vCoordSvcName = vCoordSvcName ;
   }

   _omaRunRollbackCatalogJob::~_omaRunRollbackCatalogJob ()
   {
   }
   
   INT32 _omaRunRollbackCatalogJob::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      BSONObj catalogInstalledNodeInfo ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;

      // get installed catalog node info
      _getInstalledCatalogInfo( catalogInstalledNodeInfo ) ;
      // set js file
      rc = setJSFile( FILE_ROLLBACK_CATALOG ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to set js file[%s], rc = %d",
                      FILE_ROLLBACK_CATALOG, rc ) ;
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
                   "var COORD_HOSTNAME = \'%s\'; "
                   "var COORD_SERVICE = \'%s\'; " ,
                   _vCoordHostName.c_str(),
                   _vCoordSvcName.c_str() ) ;

      PD_LOG ( PDDEBUG, "Create data node passes arguments: "
               "var COORD_HOSTNAME = %s; var COORD_SERVICE = %s;",
               _vCoordHostName.c_str(), _vCoordSvcName.c_str() ) ;

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

   INT32 _omaRunRollbackCatalogJob::doit ( BSONObj &retObj )
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
      // how to deal with this kind of error
      rc = omaGetObjElement( rval, "", subObj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", "", rc ) ;
      retObj = subObj.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _omaRunRollbackCatalogJob::_getInstalledCatalogInfo ( BSONObj &obj )
   {
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      map< string, vector< InstalledNode > >::iterator it = _info.begin() ;

      for( ; it != _info.end(); it++ )
      {
         vector< InstalledNode > &nodes = it->second ;
         vector< InstalledNode >::iterator itr = nodes.begin() ;
         for( ; itr != nodes.end(); itr++ )
         {
            bab.append ( BSON( OMA_FIELD_HOSTNAME << itr->_hostName <<
                               OMA_FIELD_SVCNAME << itr->_svcName ) ) ;
         }
      }
      bob.appendArray( OMA_FIELD_HOSTS, bab.arr() ) ;
      obj = bob.obj() ;
   }

   /*
      install db business task run rollback data node job
   */

   _omaRunRollbackDataNodeJob::_omaRunRollbackDataNodeJob (
                                   string &vCoordHostName,
                                   string &vCoordSvcName,
                                   map< string, vector<InstalledNode> > &info )
   : _info( info )
   {
      _vCoordHostName = vCoordHostName ;
      _vCoordSvcName = vCoordSvcName ;
   }

   _omaRunRollbackDataNodeJob::~_omaRunRollbackDataNodeJob ()
   {
   }
   
   INT32 _omaRunRollbackDataNodeJob::init ( const CHAR *pInstallInfo )
   {
      INT32 rc = SDB_OK ;
      BSONObj dataGroupInfo ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;

      // get installed catalog node info
      _getInstalledDataGroupInfo( dataGroupInfo ) ;
      // set js file
      rc = setJSFile( FILE_ROLLBACK_DATANODE ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to set js file[%s], rc = %d",
                      FILE_ROLLBACK_DATANODE, rc ) ;
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
                   "var COORD_HOSTNAME = \'%s\'; "
                   "var COORD_SERVICE = \'%s\'; "
                   "var CREATED_DATA_GROUP = \'%s\'; " ,
                   _vCoordHostName.c_str(),
                   _vCoordSvcName.c_str(),
                   dataGroupInfo.toString().c_str() ) ;

      PD_LOG ( PDDEBUG, "Create data node passes arguments: "
               "var COORD_HOSTNAME = %s; var COORD_SERVICE = %s;"
               "var CREATED_DATA_GROUP = %s",
               _vCoordHostName.c_str(), _vCoordSvcName.c_str(),
               dataGroupInfo.toString().c_str() ) ;

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

   INT32 _omaRunRollbackDataNodeJob::doit ( BSONObj &retObj )
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
      // how to deal with this kind of error
      rc = omaGetObjElement( rval, "", subObj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", "", rc ) ;
      retObj = subObj.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _omaRunRollbackDataNodeJob::_getInstalledDataGroupInfo( BSONObj &obj )
   {
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      map< string, vector< InstalledNode > >::iterator it = _info.begin() ;

      for( ; it != _info.end(); it++ )
      {
         string groupname = it->first ;
         bab.append( groupname.c_str() ) ;
      }
      bob.appendArray( OMA_FIELD_GROUPNAME, bab.arr() ) ;
      obj = bob.obj() ;
   }

}


