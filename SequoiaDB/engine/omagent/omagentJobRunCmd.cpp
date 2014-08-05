#include "omagentUtil.hpp"
#include "omagentJobRunCmd.hpp"


namespace engine
{
   /*
      _omaJobRunInstallCatalogCmd
   */
   _omaJobRunInstallCatalogCmd::_omaJobRunInstallCatalogCmd()
   {
      _scope = NULL ;
      _jsFileName = "createCatalog.js" ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
   }

   _omaJobRunInstallCatalogCmd::~_omaJobRunInstallCatalogCmd() {}

   INT32 _omaJobRunInstallCatalogCmd::init ( std::vector<BSONObj> &objs )
   {
      INT32 rc = SDB_OK ;
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
         rc = omaGetStringElement( *it, OMA_FIELD_HOSTNAME1,
                                       &info._hostName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME1, rc ) ;
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
         // _confPath
         rc = omaGetStringElement( *it, OMA_OPTION_CONFPATH,
                                       &info._confPath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_OPTION_CONFPATH, rc ) ;
         // _conf
         pattern = BSON( OMA_FIELD_HOSTNAME1 << 1 <<
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
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff, &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read js file: %s, rc = %d",
                  _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = getSptScope () ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaJobRunInstallCatalogCmd::doit ( InstallJobResult &result )
   {
      INT32 rc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      std::string errMsg = "" ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
      std::vector<InstallInfo>::iterator it ;

      if ( 0 == _installInfos.size() )
      {
         rc = SDB_INVALIDARG ;
         errMsg = "No catalog info for install" ;
         PD_LOG( PDERROR, errMsg.c_str() ) ;
         goto done ;
      }
      it = _installInfos.begin() ;
      while( it != _installInfos.end() )
      {
         const CHAR* conf = (*it)._conf.toString().c_str() ;
         BSONObj subObj ;

         // build js arguments
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var INSTALL_HOSTNAME = \"%s\"; var INSTALL_SERVICE = \"%s\"; var INSTALL_PATH = \"%s\"; var CONFIG = \"%s\"; ",
                      (*it)._hostName, (*it)._svcName, (*it)._dbPath, conf ) ;

         PD_LOG ( PDDEBUG, "Create catalog passes arguments:  var INSTALL_HOSTNAME = %s; var INSTALL_SERVICE = %s; var INSTALL_PATH = %s; var CONFIG = %s;",
                  (*it)._hostName, (*it)._svcName, (*it)._dbPath, conf ) ;
         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                     _jsFileName, rc, detail.toString().c_str() ) ;
            errMsg = errMsg + "Install catalog " +
                     (*it)._hostName + ":" +(*it)._svcName + "failed" ;
            goto error ;
         }
         // extract subObj
         rc = omaGetObjElement( rval, "", subObj ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", "", rc ) ;
         // extract return rc
         {
         INT32 retRc = SDB_OK ;
         rc = omaGetIntElement ( subObj, OMA_FIELD_RC, retRc ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Get field[%s] failed, rc: %d",
                     OMA_FIELD_RC, rc ) ;
            errMsg += "system error" ;
            goto error ;
         }
         if ( retRc )
         {
            rc = retRc ;
            errMsg = errMsg + "Install catalog [" +
                     (*it)._hostName + ":" +(*it)._svcName + "] failed" ;
            PD_LOG( PDERROR, (errMsg + ", rc = %d").c_str(), retRc ) ;
            goto error;
         }
         }
         // record successful node for rollback when install error happen
         result._finishNode.push_back( *it ) ;
         // go to next
         it++ ;
      }
   done:
      return rc ;
   error:
      result._rc = rc ;
      result._errMsg = errMsg ;
      goto done ;
   }


   /*
      _omaJobRunInstallCoordCmd
   */
   _omaJobRunInstallCoordCmd::_omaJobRunInstallCoordCmd()
   {
      _scope = NULL ;
      _jsFileName = "createCoord.js" ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
   }

   _omaJobRunInstallCoordCmd::~_omaJobRunInstallCoordCmd() {}

   INT32 _omaJobRunInstallCoordCmd::init ( std::vector<BSONObj> &objs )
   {
      INT32 rc = SDB_OK ;
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
         rc = omaGetStringElement( *it, OMA_FIELD_HOSTNAME1,
                                       &info._hostName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME1, rc ) ;
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
         // _confPath
         rc = omaGetStringElement( *it, OMA_OPTION_CONFPATH,
                                       &info._confPath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_OPTION_CONFPATH, rc ) ;
         // _conf
         pattern = BSON( OMA_FIELD_HOSTNAME1 << 1 <<
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
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff, &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read js file: %s, rc = %d",
                  _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = getSptScope () ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaJobRunInstallCoordCmd::doit ( InstallJobResult &result )
   {
      INT32 rc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      std::string errMsg = "" ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
      std::vector<InstallInfo>::iterator it ;

      if ( 0 == _installInfos.size() )
      {
         rc = SDB_INVALIDARG ;
         errMsg = "No coord info for install" ;
         PD_LOG( PDERROR, errMsg.c_str() ) ;
         goto done ;
      }
      it = _installInfos.begin() ;
      while( it != _installInfos.end() )
      {
         const CHAR* conf = (*it)._conf.toString().c_str() ;
         BSONObj subObj ;

         // build js arguments
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var INSTALL_HOSTNAME = \"%s\"; var INSTALL_SERVICE = \"%s\"; var INSTALL_PATH = \"%s\"; var CONFIG = \"%s\"; ",
                      (*it)._hostName, (*it)._svcName, (*it)._dbPath, conf ) ;

         PD_LOG ( PDDEBUG, "Create coord passes arguments: var INSTALL_HOSTNAME = %s; var INSTALL_SERVICE = %s; var INSTALL_PATH = %s; var CONFIG = %s;",
                           (*it)._hostName, (*it)._svcName, (*it)._dbPath, conf ) ;
         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                     _jsFileName, rc, detail.toString().c_str() ) ;
            errMsg = errMsg + "Install coord " +
                     (*it)._hostName + ":" +(*it)._svcName + "failed" ;
            goto error ;
         }
         // extract subObj
         rc = omaGetObjElement( rval, "", subObj ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", "", rc ) ;
         // extract return rc
         {
         INT32 retRc = SDB_OK ;
         rc = omaGetIntElement ( subObj, OMA_FIELD_RC, retRc ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Get field[%s] failed, rc: %d",
                     OMA_FIELD_RC, rc ) ;
            errMsg += "system error" ;
            goto error ;
         }
         if ( retRc )
         {
            rc = retRc ;
            errMsg = errMsg + "Install coord [" +
                     (*it)._hostName + ":" +(*it)._svcName + "] failed" ;
            PD_LOG( PDERROR, (errMsg + ", rc = %d").c_str(), retRc ) ;
            goto error;
         }
         }
         // record successful node for rollback when install error happen
         result._finishNode.push_back( *it ) ;
         // go to next
         it++ ;
      }
   done:
      return rc ;
   error:
      result._rc = rc ;
      result._errMsg = errMsg ;
      goto done ;
   }


   /*
      _omaJobRunInstallDataCmd
   */
   _omaJobRunInstallDataCmd::_omaJobRunInstallDataCmd()
   {
      _scope = NULL ;
      _jsFileName = "createData.js" ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
   }

   _omaJobRunInstallDataCmd::~_omaJobRunInstallDataCmd() {}

   INT32 _omaJobRunInstallDataCmd::init ( std::vector<BSONObj> &objs )
   {
      INT32 rc = SDB_OK ;
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
         rc = omaGetStringElement( *it, OMA_FIELD_HOSTNAME1,
                                       &info._hostName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME1, rc ) ;
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
         // _confPath
         rc = omaGetStringElement( *it, OMA_OPTION_CONFPATH,
                                       &info._confPath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_OPTION_CONFPATH, rc ) ;
         // _conf
         pattern = BSON( OMA_FIELD_HOSTNAME1 << 1 <<
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
      // read js from file
      rc = readFile ( _jsFileName, &_fileBuff, &_buffSize, &_readSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read js file: %s, rc = %d",
                  _jsFileName, rc ) ;
         goto error ;
      }
      // get scope
      _scope = getSptScope () ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaJobRunInstallDataCmd::doit ( InstallJobResult &result )
   {
      INT32 rc = SDB_OK ;
      BSONObj rval ;
      BSONObj detail ;
      std::string errMsg = "" ;
      CHAR tempBuff[ JS_ARG_LEN ] = { 0 } ;
      std::vector<InstallInfo>::iterator it ;

      if ( 0 == _installInfos.size() )
      {
         rc = SDB_INVALIDARG ;
         errMsg = "No data node infos for install" ;
         PD_LOG( PDERROR, errMsg.c_str() ) ;
         goto done ;
      }
      it = _installInfos.begin() ;
      while( it != _installInfos.end() )
      {
         const CHAR* conf = (*it)._conf.toString().c_str() ;
         BSONObj subObj ;

         // build js arguments
         ossSnprintf( tempBuff, JS_ARG_LEN,
                      " var GROUPNAME = \"%s\"; var INSTALL_HOSTNAME = \"%s\"; var INSTALL_SERVICE = \"%s\"; var INSTALL_PATH = \"%s\"; var CONFIG = \"%s\"; ",
                      (*it)._dataGroupName, (*it)._hostName, (*it)._svcName, (*it)._dbPath, conf ) ;

         PD_LOG ( PDDEBUG, "Create data node passes arguments: groupname = %s; hostname = %s; svcname = %s; dbpath = %s; config = %s;",
                           (*it)._dataGroupName, (*it)._hostName,
                           (*it)._svcName, (*it)._dbPath, conf ) ;
         _content.clear() ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(),
                            _jsFileName, 1, 1, rval, detail ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                     _jsFileName, rc, detail.toString().c_str() ) ;
            errMsg = errMsg + "Install data node " +
                     (*it)._hostName + ":" +(*it)._svcName + "failed" ;
            goto error ;
         }
         // extract subObj
         rc = omaGetObjElement( rval, "", subObj ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", "", rc ) ;
         // extract return rc
         {
         INT32 retRc = SDB_OK ;
         rc = omaGetIntElement ( subObj, OMA_FIELD_RC, retRc ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Get field[%s] failed, rc: %d",
                     OMA_FIELD_RC, rc ) ;
            errMsg += "system error" ;
            goto error ;
         }
         if ( retRc )
         {
            rc = retRc ;
            errMsg = errMsg + "Install data node [" +
                     (*it)._hostName + ":" +(*it)._svcName + "] failed" ;
            PD_LOG( PDERROR, (errMsg + ", rc = %d").c_str(), retRc ) ;
            goto error;
         }
         }
         // record successful node for rollback when install error happen
         result._finishNode.push_back( *it ) ;
         // go to next
         it++ ;
      }
   done:
      return rc ;
   error:
      result._rc = rc ;
      result._errMsg = errMsg ;
      goto done ;
   }



}


