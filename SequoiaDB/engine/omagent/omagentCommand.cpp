#include "omagentCommand.hpp"
#include "omagentUtil.hpp"
#include "omagentHelper.hpp"
#include "sptContainer.hpp"

using namespace engine ;
using namespace bson ;

//static INT32 checkRemoteAgentProcess ( BSONObj &result ) ;

namespace CLSMGR
{
   // _omagentCommand
   _omagentCommand::_omagentCommand ()
   {
/*
      if ( NULL == scope )
      {
         _sptContainer container ;
         scope = container.newScope( SPT_SCOPE_TYPE_SP ) ;
         if ( NULL == scope )
            ossPrintf( "Failed to get scope"OSS_NEWLINE ) ;
      }
*/
   }

   _omagentCommand::~_omagentCommand ()
   {
/*
      if ( NULL != scope )
      {
         scope->shutdown() ;
         delete scope ;
         scope = NULL ;
      }
*/
   }

//   _sptScope *_omagentCommand::scope = NULL ;

   // _omagentCmdAssit
   _omagentCmdAssit::_omagentCmdAssit ( OA_NEW_FUNC pFunc )
   {
      if ( pFunc )
      {
         _omagentCommand *pCommand = (*pFunc)() ;
         if ( pCommand )
         {
            getOmagentCmdBuilder()->_register ( pCommand->name(), pFunc ) ;
            SDB_OSS_DEL pCommand ;
            pCommand = NULL ;
         }
      }
   }

   _omagentCmdAssit::~_omagentCmdAssit ()
   {
   }

   // _omagentCmdBuilder
   _omagentCmdBuilder::_omagentCmdBuilder ()
   {
   }

   _omagentCmdBuilder::~_omagentCmdBuilder ()
   {
      // TODO: do i need to release memory in map ?
   }

   _omagentCommand* _omagentCmdBuilder::create ( const CHAR *command )
   {
      OA_NEW_FUNC pFunc = _find ( command ) ;
      if ( pFunc )
      {
         return (*pFunc)() ;
      }
      return NULL ;
   }

   void _omagentCmdBuilder::release ( const _omagentCommand *pCommand )
   {
      if ( pCommand )
      {
         SDB_OSS_DEL pCommand ;
      }
   }

   INT32 _omagentCmdBuilder::_register ( const CHAR *name, OA_NEW_FUNC pFunc )
   {
      INT32 rc = SDB_OK ;

      std::pair<MAP_OACMD_IT, BOOLEAN> ret ;
      ret = _cmdMap.insert( std::pair<const CHAR*, OA_NEW_FUNC>(name, pFunc) ) ;
      if ( FALSE == ret.second )
      {
         PD_LOG ( PDERROR, "Failed to register omagent command %s, \
                  already exist", name ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto error ;
   }

   OA_NEW_FUNC _omagentCmdBuilder::_find ( const CHAR *name )
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

   // get omagent command builder
   _omagentCmdBuilder* getOmagentCmdBuilder()
   {
      static _omagentCmdBuilder cmdBuilder ;
      return &cmdBuilder ;
   }

   // command list:
   IMPLEMENT_OACMD_AUTO_REGISTER( _omagentAddHost )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omagentScanHost )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omagentInstallRemoteAgent )
   IMPLEMENT_OACMD_AUTO_REGISTER( _omagentInstallAgentProcess )

   // _omagentAddHost
   _omagentAddHost::_omagentAddHost()
   {
      _scope = NULL ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
   }

   _omagentAddHost::~_omagentAddHost()
   {
   }

   INT32 _omagentAddHost::init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                                 const CHAR *pMatcherBuff,
                                 const CHAR *pSelectBuff,
                                 const CHAR *pOrderByBuff,
                                 const CHAR *pHintBuff )
   {
      INT32 rc = SDB_OK ;
   done:
      return rc ;
   error:
     goto done ;
   }

// _sptScope* getSptScope
   INT32 _omagentAddHost::doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;
   done:
      return rc ;
   error:
     goto done ;
   }

   // _omagentScanHost
   _omagentScanHost::_omagentScanHost()
   {
//      ossPrintf ( "In scan host constructor."OSS_NEWLINE ) ;
      _scope = NULL ;
      _jsFileName = "scanHost.js" ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
   }

   _omagentScanHost::~_omagentScanHost()
   {
//      ossPrintf ( "In scan host destructor."OSS_NEWLINE ) ;
   }

   INT32 _omagentScanHost::init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                                 const CHAR *pMatcherBuff,
                                 const CHAR *pSelectBuff,
                                 const CHAR *pOrderByBuff,
                                 const CHAR *pHintBuff )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pMatcherBuff ) ;
      ele = arg.getField ( OMA_FIELD_NAME_HOSTS ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
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

      // 
   done:
      return rc ;
   error:
     goto done ;
   }

//   INT32 _omagentScanHost::doit (  omagentObjBuff &objBuff )
   INT32 _omagentScanHost::doit ( CHAR **ppBody, INT32 &bodyLen,
                                  INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONObj retObj ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;
      while ( itr != _hosts.end() )
      {
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         // BSONObj pattern ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;

         CHAR tempBuff[ 1024 ] = { 0 } ;
         const CHAR *pIp = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassWord = NULL ;

         _content.clear() ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_IP, &pIp ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_IP, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_USERNAME, &pUserName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_USERNAME, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_PASSWORD, &pPassWord ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_PASSWORD, rc ) ;

         ossSnprintf( tempBuff, 1024,
                      "var IP = \"%s\"; var USERNAME = \"%s\"; var PASSWORD = \"%s\";",
                      pIp, pUserName, pPassWord ) ;

         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(), _jsFileName, 1, 1, rval, detail ) ;

         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                     _jsFileName, rc, detail.toString().c_str() ) ;
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_NAME_PING, false ) ;
            bob.append( OMA_FIELD_NAME_SSH, false ) ;
            bob.appendNull( OMA_FIELD_NAME_HOSTNAME ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         rc = omagentGetObjElement( rval, "", subObj ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", "", rc ) ;
/*
         pattern = BSON( OMA_FIELD_NAME_PING << 1 << OMA_FIELD_NAME_SSH << 1 <<
                         OMA_FIELE_NAME_HOSTNAME << 1 ) ;
         obj =  subObj.extractFields( pattern, true );
         bob.appendElement( obj ) ;
*/
         bob.append( OMA_FIELD_NAME_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;

printf ( "reval is: %s\n", rval.toString(false, true).c_str() ) ;

      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_NAME_SCAN_HOST_RET, bab.arr() ) ;
      retObj = bob.obj() ;
std::cout << "retObj's size: " << retObj.objsize() << std::endl ;
std::cout << "retObj is: " << retObj.toString().c_str() << std::endl ;
      // build return body
      rc = omagentBuildReplyMsgBody( ppBody, &bodyLen, 1, &retObj ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Omagent failed to build reply msg, rc: %d", rc ) ;
         goto error;
      }

   done:
      return rc ;
   error:
     goto done ;
   }


   // _omagentInstallRemoteAgent
   _omagentInstallRemoteAgent::_omagentInstallRemoteAgent ()
   {
      _scope = NULL ;
      _jsFileName = "installAgentProcess.js" ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;
   }

   _omagentInstallRemoteAgent::~_omagentInstallRemoteAgent ()
   {

   }

   INT32 _omagentInstallRemoteAgent::init ( INT32 flags, INT64 numToSkip,
                                            INT64 numToReturn,
                                            const CHAR *pMatcherBuff,
                                            const CHAR *pSelectBuff,
                                            const CHAR *pOrderByBuff,
                                            const CHAR *pHintBuff )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pMatcherBuff ) ;
      ele = arg.getField ( OMA_FIELD_NAME_HOSTS ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
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

   INT32 _omagentInstallRemoteAgent::doit ( CHAR **ppBody, INT32 &bodyLen,
                                            INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONObj retObj ;
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

         CHAR tempBuff[ 1024 ] = { 0 } ;
         const CHAR *pIp = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;
         const CHAR *pLocalPath = NULL ;
         const CHAR *pRemotePath = NULL ;
         const CHAR *pVersion   = NULL ;
         BOOLEAN isRunning      = FALSE ;

         _content.clear() ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_IP, &pIp ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_IP, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_USERNAME, &pUserName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_USERNAME, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_PASSWORD, &pPassword ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_PASSWORD, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_LOCAL_PACKET_PATH,
                                       &pLocalPath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d",
                   OMA_FIELD_NAME_LOCAL_PACKET_PATH, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_REMOTE_PACKET_PATH,
                                       &pRemotePath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d",
                   OMA_FIELD_NAME_REMOTE_PACKET_PATH, rc ) ;

         // check whether the remote machine has install omagent or not
         rc = getRemoteAgentStatus ( pIp, pUserName, pPassword, status ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Failed to get remote mechine's status, rc = %d", rc ) ;
         rc = omagentGetBooleanElement( status, OMA_FIELD_NAME_AGENT_IS_RUNNING,
                                        isRunning ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d",
                   OMA_FIELD_NAME_AGENT_IS_RUNNING, rc ) ;
         if ( isRunning )
         {
            const CHAR *ver = getVersion() ;
            rc = omagentGetStringElement( status, OMA_FIELD_NAME_AGENT_VERSION,
                                          &pVersion ) ;
            PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                      "Get field[%s] failed, rc: %d",
                      OMA_FIELD_NAME_AGENT_VERSION, rc ) ;
            if ( 0 != ossStrncmp( pVersion, ver, ossStrlen( ver ) ) )
            {
               PD_LOG( PDDEBUG, "Remote omagent's version is: %s, \
                       and we are going to instll version %s",
                       pVersion, ver ) ;
               BSONObj errObj ;
               BSONObjBuilder bob ;
               bob.append( OMA_FIELD_NAME_IP, pIp ) ;
               bob.append( OMA_FIELD_NAME_RC, SDB_OMA_DIFF_VER_AGT_IS_RUNNING ) ;
               bob.append( OMA_FIELD_NAME_DETAIL,
                           getErrDesp( SDB_OMA_DIFF_VER_AGT_IS_RUNNING ) ) ;
               errObj = bob.obj() ;
               result.push_back( errObj ) ;
               continue ;
            }
         }
         // build js file's argument
         ossSnprintf( tempBuff, 1024,
                      " var IP = \"%s\"; var USERNAME = \"%s\"; \
                      var PASSWORD = \"%s\"; var LOCAL_PACKET_PATH = \"%s\"; \
                      var REMOTE_PACKET_PATH = \"%s\" ",
                      pIp, pUserName, pPassword, pLocalPath, pRemotePath ) ;
         PD_LOG ( PDDEBUG, "Install remote agent passes arguments: %s",
                  tempBuff ) ;
         _content += tempBuff ;
         _content += OSS_NEWLINE ;
         _content += _fileBuff ;

         // execute js
         rc = _scope->eval( _content.c_str(), _content.size(), _jsFileName, 1, 1, rval, detail ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to eval js file: %s, rc = %d, errmsg = %s",
                     _jsFileName, rc, detail.toString().c_str() ) ;
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_NAME_IP, pIp ) ;
            bob.append( OMA_FIELD_NAME_RC, rc ) ;
            bob.append( OMA_FIELD_NAME_DETAIL, detail.toString().c_str() ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         // extract the result from the return value
         rc = omagentGetObjElement( rval, "", subObj ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", "", rc ) ;
         bob.append( OMA_FIELD_NAME_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;

printf ( "reval is: %s\n", rval.toString(false, true).c_str() ) ;

      } // while
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_NAME_INSATLL_REMOTE_AGENT_RET, bab.arr() ) ;
      retObj = bob.obj() ;
std::cout << "retObj's size: " << retObj.objsize() << std::endl ;
std::cout << "retObj is: " << retObj.toString().c_str() << std::endl ;
      // build return body
      rc = omagentBuildReplyMsgBody( ppBody, &bodyLen, 1, &retObj ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Omagent failed to build reply msg, rc: %d", rc ) ;
         goto error;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omagentInstallRemoteAgent::getRemoteAgentStatus ( const CHAR *pIp,
                                                            const CHAR *pUserName,
                                                            const CHAR *pPassword,
                                                            BSONObj &result )
   {
      INT32 rc = SDB_OK ;
      _omagentCheckRemoteAgentProcess checkRemote ;
      rc = checkRemote.check( pIp, pUserName, pPassword, result ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Faled to check remote mechine's status, rc = %d",
                  rc ) ;
      }
      return rc ;
   }


   // _omagentCheckRemoteAgentProcess
   _omagentCheckRemoteAgentProcess::_omagentCheckRemoteAgentProcess ()
   {
      _scope = NULL ;
      _jsFileName = "checkRemoteAgentProcess.js" ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;

   }

   _omagentCheckRemoteAgentProcess::~_omagentCheckRemoteAgentProcess ()
   {

   }

   INT32 _omagentCheckRemoteAgentProcess::init( INT32 flags, INT64 numToSkip,
                                                INT64 numToReturn,
                                                const CHAR *pMatcherBuff,
                                                const CHAR *pSelectBuff,
                                                const CHAR *pOrderByBuff,
                                                const CHAR *pHintBuff )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pMatcherBuff ) ;
      ele = arg.getField ( OMA_FIELD_NAME_HOSTS ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
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
   error :
      goto done ;
   }


   INT32 _omagentCheckRemoteAgentProcess::doit( CHAR **ppBody, INT32 &bodyLen,
                                                INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONObj retObj ;
      std::vector<BSONObj>::iterator it ;
      std::vector<BSONObj>::iterator itr = _hosts.begin() ;
      while ( itr != _hosts.end() )
      {
         BSONObj detail ;
         BSONObj rval ;
         BSONObj host = *itr++ ;
         // BSONObj pattern ;
         BSONObj subObj ;
         BSONObj temp ;
         BSONObjBuilder bob ;

         CHAR tempBuff[ 1024 ] = { 0 } ;
         const CHAR *pIp = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;

         _content.clear() ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_IP, &pIp ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_IP, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_USERNAME, &pUserName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_USERNAME, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_PASSWORD, &pPassword ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_PASSWORD, rc ) ;

         ossSnprintf( tempBuff, 1024,
                      "var IP = \"%s\"; var USERNAME = \"%s\"; var PASSWORD = \"%s\";",
                      pIp, pUserName, pPassword ) ;
         PD_LOG( PDDEBUG, "Arguments for checkRemoteAgentProcess.js is: %s",
                 tempBuff ) ;

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
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_NAME_IP, pIp ) ;
            bob.append( OMA_FIELD_NAME_RC, rc ) ;
            bob.append( OMA_FIELD_NAME_DETAIL, detail.toString().c_str() ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         rc = omagentGetObjElement( rval, "", subObj ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", "", rc ) ;
         bob.append( OMA_FIELD_NAME_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;

printf ( "reval is: %s\n", rval.toString(false, true).c_str() ) ;

      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_NAME_CHECK_REMOTE_AGENT_PROCESS_RET,
                       bab.arr() ) ;
      retObj = bob.obj() ;
std::cout << "retObj's size: " << retObj.objsize() << std::endl ;
std::cout << "retObj is: " << retObj.toString().c_str() << std::endl ;
      // build return body
      rc = omagentBuildReplyMsgBody( ppBody, &bodyLen, 1, &retObj ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Omagent failed to build reply msg, rc: %d", rc ) ;
         goto error;
      }

   done:
      return rc ;
   error:
      goto error ;
   }


   INT32 _omagentCheckRemoteAgentProcess::check ( const CHAR *pIp,
                                                  const CHAR *pUserName,
                                                  const CHAR *pPassword,
                                                  BSONObj &result )
   {
      INT32 rc = SDB_OK ;
      CHAR *pBuffer = NULL ;
      INT32 bufLen  = 0 ;
      INT32 returnNum = 0 ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONObj host ;
      BSONObj subObj ;
      if ( !pIp || !pUserName || !pPassword )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Invalid argument" ) ;
         goto error ;
      }
      subObj = BSON( OMA_FIELD_NAME_IP << pIp <<
                     OMA_FIELD_NAME_USERNAME << pUserName <<
                     OMA_FIELD_NAME_PASSWORD << pPassword ) ;
      bab.append( subObj ) ;
      bob.appendArray( OMA_FIELD_NAME_HOSTS, bab.arr() ) ;
      host = bob.obj() ;
      // init
      rc = init ( 0, 0, 0, host.objdata(), NULL, NULL, NULL ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to init check remote agent process" ) ;
         goto error ;
      }
      // doit
      rc = doit ( &pBuffer, bufLen, returnNum ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to execute check remote agent process" ) ;
         goto error ;
      }
      // build return obj
      try
      {
         result = BSONObj( pBuffer ) ;
      }
      catch ( std::exception &e )
      {
         PD_CHECK( FALSE, SDB_INVALIDARG, error, PDERROR,
                  "Failed to build bson:%s", e.what() ) ;
      }

      done:
         return rc ;
      error:
         goto done ;
   }

   // _omagentInstallAgentProcess
   _omagentInstallAgentProcess::_omagentInstallAgentProcess ()
   {
      _scope = NULL ;
      _jsFileName = "installAgentProcess.js" ;
      _fileBuff = NULL ;
      _buffSize = 0 ;
      _readSize = 0 ;

   }

   _omagentInstallAgentProcess::~_omagentInstallAgentProcess ()
   {

   }

   INT32 _omagentInstallAgentProcess::init( INT32 flags, INT64 numToSkip,
                                                INT64 numToReturn,
                                                const CHAR *pMatcherBuff,
                                                const CHAR *pSelectBuff,
                                                const CHAR *pOrderByBuff,
                                                const CHAR *pHintBuff )
   {
      INT32 rc = SDB_OK ;
      // parse bson and get arguments info for js file
      BSONElement ele ;
      BSONObj arg( pMatcherBuff ) ;
      ele = arg.getField ( OMA_FIELD_NAME_HOSTS ) ;
      if ( Array == ele.type() )
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG ( PDERROR, "Invalid argument" ) ;
               goto error ;
            }
            BSONObj temp = ele.embeddedObject() ;
            _hosts.push_back ( temp ) ;
         }
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
   error :
      goto done ;
   }


   INT32 _omagentInstallAgentProcess::doit( CHAR **ppBody, INT32 &bodyLen,
                                                INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;
      std::vector<BSONObj> result ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONObj retObj ;
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

         CHAR tempBuff[ 1024 ] = { 0 } ;
         const CHAR *pIp = NULL ;
         const CHAR *pUserName = NULL ;
         const CHAR *pPassword = NULL ;
         const CHAR *pLocalPath = NULL ;
         const CHAR *pRemotePath = NULL ;

         _content.clear() ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_IP, &pIp ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_IP, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_USERNAME, &pUserName ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_USERNAME, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_PASSWORD, &pPassword ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", OMA_FIELD_NAME_PASSWORD, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_LOCAL_PACKET_PATH,
                                       &pLocalPath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d",
                   OMA_FIELD_NAME_LOCAL_PACKET_PATH, rc ) ;
         rc = omagentGetStringElement( host, OMA_FIELD_NAME_REMOTE_PACKET_PATH,
                                       &pRemotePath ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d",
                   OMA_FIELD_NAME_REMOTE_PACKET_PATH, rc ) ;

         ossSnprintf( tempBuff, 1024,
                      " var IP = \"%s\"; var USERNAME = \"%s\"; \
                      var PASSWORD = \"%s\"; var LOCAL_PACKET_PATH = \"%s\"; \
                      var REMOTE_PACKET_PATH = \"%s\" ",
                      pIp, pUserName, pPassword, pLocalPath, pRemotePath ) ;
         PD_LOG ( PDDEBUG, "Install remote agent passes arguments: %s",
                  tempBuff ) ;

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
            // TODO:what's in detail ?
            BSONObj errObj ;
            BSONObjBuilder bob ;
            bob.append( OMA_FIELD_NAME_IP, pIp ) ;
            bob.append( OMA_FIELD_NAME_RC, rc ) ;
            bob.append( OMA_FIELD_NAME_DETAIL, detail.toString().c_str() ) ;
            errObj = bob.obj() ;
            result.push_back( errObj ) ;
            continue ;
         }
         rc = omagentGetObjElement( rval, "", subObj ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d", "", rc ) ;
         bob.append( OMA_FIELD_NAME_IP, pIp ) ;
         bob.appendElements( subObj ) ;
         temp = bob.obj() ;
         result.push_back( temp ) ;

printf ( "reval is: %s\n", rval.toString(false, true).c_str() ) ;

      }
      // build return bson obj
      it = result.begin() ;
      while ( it != result.end() )
         bab.append( *it++ ) ;
      bob.appendArray( OMA_FIELD_NAME_INSTALL_AGENT_PROCESS_RET,
                       bab.arr() ) ;
      retObj = bob.obj() ;
std::cout << "retObj's size: " << retObj.objsize() << std::endl ;
std::cout << "retObj is: " << retObj.toString().c_str() << std::endl ;
      // build return body
      rc = omagentBuildReplyMsgBody( ppBody, &bodyLen, 1, &retObj ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Omagent failed to build reply msg, rc: %d", rc ) ;
         goto error;
      }

   done:
      return rc ;
   error:
      goto error ;
   }

}
