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

   Source File Name = omGetFileCommand.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/12/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#include "omGetFileCommand.hpp"
#include "omDef.hpp"
#include "rtn.hpp"
#include "omManager.hpp"
#include "omConfigGenerator.hpp"
#include "../bson/lib/md5.hpp"
#include "ossPath.hpp"
#include "ossProc.hpp"
#include <set>

using namespace bson;
using namespace boost::property_tree;


namespace engine
{
   // *****************omAuthCommand *****************************
   omAuthCommand::omAuthCommand( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession )
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
   }

   omAuthCommand::~omAuthCommand()
   {
   }

   void omAuthCommand::_sendErrorRes2Web( INT32 rc, const CHAR* detail )
   {
      BSONObj res = BSON( OM_REST_RES_RETCODE << rc 
                          << OM_REST_RES_DETAIL << detail ) ;

      _restAdaptor->setOPResult( _restSession, rc, res ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
   }

   void omAuthCommand::_sendErrorRes2Web( INT32 rc, const string &detail )
   {
      _sendErrorRes2Web( rc, detail.c_str() ) ;
   }

   void omAuthCommand::_decryptPasswd( const string encryptPasswd, string time,
                                       string &decryptPasswd )
   {
      decryptPasswd = encryptPasswd ;
   }

   INT32 omAuthCommand::doCommand()
   {
      const CHAR *pUserName        = NULL ;
      const CHAR *pPasswd          = NULL ;
      const CHAR *pTime            = NULL ;
      INT32 rc                     = SDB_OK ;
      ossSocket *socket            = NULL ;
      BSONObjBuilder authBuilder ;
      BSONObjBuilder resBuilder ;
      BSONObj bsonRes ;
      BSONObj bsonAuth ;
      md5::md5digest digest ;
      string realPasswd ;

      _restAdaptor->getQuery( _restSession, OM_REST_FIELD_LOGIN_NAME, 
                              &pUserName ) ;
      _restAdaptor->getQuery( _restSession, OM_REST_FIELD_LOGIN_PASSWD, 
                              &pPasswd ) ;
      _restAdaptor->getQuery( _restSession, OM_REST_FIELD_TIMESTAMP, &pTime ) ;
      if ( ( NULL == pUserName ) || ( NULL == pPasswd ) || ( NULL == pTime ) )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_REST_FIELD_LOGIN_NAME ) + " or " 
                        + OM_REST_FIELD_LOGIN_PASSWD + " or " 
                        + OM_REST_FIELD_TIMESTAMP + " is null" ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      //TODO: encrypt the passwd when through rest
      _decryptPasswd( pPasswd, pTime, realPasswd ) ;
      md5::md5( ( const void * )realPasswd.c_str(), realPasswd.length(), 
                digest) ;
      authBuilder.append( SDB_AUTH_USER, pUserName ) ;
      authBuilder.append( SDB_AUTH_PASSWD, md5::digestToString( digest ) ) ;
      bsonAuth = authBuilder.obj() ;
      rc = sdbGetOMManager()->authenticate( bsonAuth, _cb ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_AUTH_AUTHORITY_FORBIDDEN == rc )
         {
            _sendErrorRes2Web( rc, "username or passwd is wrong" ) ;
         }
         else
         {
            _sendErrorRes2Web( rc, "system error" ) ;
         }

         goto error ;
      }

      socket = _restSession->socket() ;
      if ( NULL == socket )
      {
         PD_LOG( PDERROR, "socket is null, impossible" ) ;
         _sendErrorRes2Web( SDB_SYS, "system error" ) ;
         goto error ;
      }

      rc = _restSession->doLogin( pUserName, socket->getLocalIP() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "do login failed:user=%s, ip=%u", pUserName,
                 socket->getLocalIP() ) ;
         _sendErrorRes2Web( SDB_SYS, "system error" ) ;
         goto error ;
      }

      resBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      resBuilder.append( OM_REST_RES_LOCAL, "/"OM_REST_INDEX_HTML ) ;
      _restAdaptor->appendHttpHeader( _restSession, FIELD_NAME_SESSIONID, 
                                      _restSession->getSessionID() ) ;
      _restAdaptor->setOPResult( _restSession, rc, resBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return SDB_OK ;
   error:
      goto done ;
   }

   // ********************onLogoutCommand********************************
   omLogoutCommand::omLogoutCommand( restAdaptor *pRestAdaptor, 
                                     pmdRestSession *pRestSession )
                   :omAuthCommand( pRestAdaptor, pRestSession )
   {
   }

   omLogoutCommand::~omLogoutCommand()
   {
   }

   INT32 omLogoutCommand::doCommand()
   {
      INT32 rc = SDB_OK ;

      _restSession->doLogout() ;

      BSONObjBuilder resBuilder ;
      resBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      resBuilder.append( OM_REST_RES_LOCAL, "/"OM_REST_LOGIN_HTML ) ;
      _restAdaptor->setOPResult( _restSession, rc, resBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

      return SDB_OK ;
   }

   // *****************omChangePasswdCommand *****************************
   omChangePasswdCommand::omChangePasswdCommand( restAdaptor *pRestAdaptor, 
                                                 pmdRestSession *pRestSession )
                         :omAuthCommand( pRestAdaptor, pRestSession )
   {
   }

   omChangePasswdCommand::~omChangePasswdCommand()
   {
   }

   INT32 omChangePasswdCommand::_getRestDetail( string &user, string &oldPasswd, 
                                                string &newPasswd, string &time )
   {
      INT32 rc               = SDB_OK ;
      const CHAR *pUser      = NULL ;
      const CHAR *pOldPasswd = NULL ;
      const CHAR *pNewPasswd = NULL ;
      const CHAR *pTime      = NULL ;
      _restAdaptor->getQuery( _restSession, OM_REST_FIELD_LOGIN_NAME, &pUser ) ;
      if ( NULL == pUser )
      {
         rc           = SDB_INVALIDARG ;
         _errorDetail = string( OM_REST_FIELD_LOGIN_NAME ) + " is null" ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      _restAdaptor->getQuery( _restSession, OM_REST_FIELD_LOGIN_PASSWD, 
                              &pOldPasswd ) ;
      if ( NULL == pOldPasswd )
      {
         rc           = SDB_INVALIDARG ;
         _errorDetail = string( OM_REST_FIELD_LOGIN_PASSWD ) + " is null" ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      _restAdaptor->getQuery( _restSession, OM_REST_FIELD_NEW_PASSWD, 
                              &pNewPasswd ) ;
      if ( NULL == pNewPasswd )
      {
         rc           = SDB_INVALIDARG ;
         _errorDetail = string( OM_REST_FIELD_NEW_PASSWD ) + " is null" ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      _restAdaptor->getQuery( _restSession, OM_REST_FIELD_TIMESTAMP, &pTime ) ;
      if ( NULL == pTime )
      {
         rc           = SDB_INVALIDARG ;
         _errorDetail = string( OM_REST_FIELD_TIMESTAMP ) + " is null" ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      user      = pUser ;
      newPasswd = pNewPasswd ;
      oldPasswd = pOldPasswd ;
      time      = pTime ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omChangePasswdCommand::doCommand()
   {
      INT32 rc = SDB_OK ;
      string user ;
      string oldPasswd ;
      string oldDecryptPasswd ;
      md5::md5digest oldDigest ;
      string newPasswd ;
      string newDecryptPasswd ;
      md5::md5digest newDigest ;
      string time ;

      BSONObj bsonAuth ;

      rc = _getRestDetail( user, oldPasswd, newPasswd, time ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get rest info failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      if ( user != _restSession->getLoginUserName() )
      {
         rc           = SDB_INVALIDARG ;
         _errorDetail = string( "can't not change other usr's passwd:" )
                        + "myusr=" + _restSession->getLoginUserName()
                        + ",usr=" + user ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _decryptPasswd( oldPasswd, time, oldDecryptPasswd ) ;
      md5::md5( ( const void * )oldDecryptPasswd.c_str(), 
                oldDecryptPasswd.length(), oldDigest) ;
      bsonAuth = BSON( SDB_AUTH_USER << user 
                      << SDB_AUTH_PASSWD << md5::digestToString( oldDigest ) ) ;
      rc = sdbGetOMManager()->authenticate( bsonAuth, _cb ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_AUTH_AUTHORITY_FORBIDDEN == rc )
         {
            PD_LOG( PDERROR, "username or passwd is wrong:rc=%d", rc ) ;
            _sendErrorRes2Web( rc, "username or passwd is wrong" ) ;
         }
         else
         {
            PD_LOG( PDERROR, "system error:rc=%d", rc ) ;
            _sendErrorRes2Web( rc, "system error" ) ;
         }

         goto error ;
      }

      _decryptPasswd( newPasswd, time, newDecryptPasswd ) ;
      md5::md5( ( const void * )newDecryptPasswd.c_str(), 
                newDecryptPasswd.length(), newDigest) ;
      rc = sdbGetOMManager()->authUpdatePasswd( user,
                                               md5::digestToString( oldDigest ), 
                                               md5::digestToString( newDigest ),
                                               _cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "change passwd failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "change passwd failed" ) ;
         goto error ;
      }

      {
         BSONObjBuilder resBuilder ;
         resBuilder.append( OM_REST_RES_RETCODE, rc ) ;
         _restAdaptor->setOPResult( _restSession, rc, resBuilder.obj() ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omCheckSessionCommand *****************************
   omCheckSessionCommand::omCheckSessionCommand( restAdaptor *pRestAdaptor, 
                                                 pmdRestSession *pRestSession )
                         :omAuthCommand( pRestAdaptor, pRestSession )
   {
   }

   omCheckSessionCommand::~omCheckSessionCommand()
   {
   }

   INT32 omCheckSessionCommand::doCommand()
   {
      BSONObjBuilder bsonBuilder ;
      const CHAR* sessionID = NULL ;
      _restAdaptor->getHttpHeader( _restSession, FIELD_NAME_SESSIONID, 
                                   &sessionID ) ;
      if ( NULL != sessionID && _restSession->isAuthOK() )
      {
         bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
         _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      }
      else
      {
         PD_LOG( PDEVENT, "OM: redirect to:%s", OM_REST_LOGIN_HTML ) ;
         bsonBuilder.append( OM_REST_RES_RETCODE, 
                             SDB_AUTH_AUTHORITY_FORBIDDEN ) ;
         bsonBuilder.append( OM_REST_RES_LOCAL, "/"OM_REST_LOGIN_HTML ) ;
         _restAdaptor->setOPResult( _restSession, SDB_AUTH_AUTHORITY_FORBIDDEN, 
                                    bsonBuilder.obj() ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      }

      return SDB_OK ;
   }

   // *****************omCreateClusterCommand *****************************
   omCreateClusterCommand::omCreateClusterCommand( restAdaptor *pRestAdaptor, 
                                                 pmdRestSession *pRestSession )
                          :omCheckSessionCommand( pRestAdaptor, 
                                                  pRestSession )
   {
   }

   omCreateClusterCommand::~omCreateClusterCommand()
   {
   }

   INT32 omCreateClusterCommand::_getClusterInfo( string &clusterName, 
                                                  string &desc,
                                                  string &sdbUsr, 
                                                  string &sdbPasswd,
                                                  string &sdbUsrGroup,
                                                  string &installPath )
   {
      const CHAR *pClusterInfo = NULL ;
      BSONObj clusterInfo ;
      INT32 rc                 = SDB_OK ;
      _restAdaptor->getQuery(_restSession, OM_REST_CLUSTER_INFO, 
                             &pClusterInfo ) ;
      if ( NULL == pClusterInfo )
      {
         _errorDetail = string("rest field ") + OM_REST_CLUSTER_INFO
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pClusterInfo, clusterInfo ) ;
      if ( rc )
      {
         _errorDetail = string("change rest field ") + OM_REST_CLUSTER_INFO
                        + " to BSONObj failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      clusterName = clusterInfo.getStringField( OM_BSON_FIELD_CLUSTER_NAME ) ;
      desc        = clusterInfo.getStringField( OM_BSON_FIELD_CLUSTER_DESC ) ;
      sdbUsr      = clusterInfo.getStringField( OM_BSON_FIELD_SDB_USER ) ;
      sdbPasswd   = clusterInfo.getStringField( OM_BSON_FIELD_SDB_PASSWD ) ;
      sdbUsrGroup = clusterInfo.getStringField( OM_BSON_FIELD_SDB_USERGROUP ) ;
      installPath = clusterInfo.getStringField( OM_BSON_FIELD_INSTALL_PATH ) ;
      if ( 0 == clusterName.length() || 0 == sdbUsr.length()
           || 0 == sdbPasswd.length() || 0 == sdbUsrGroup.length() )
      {
         _errorDetail = string( OM_BSON_FIELD_CLUSTER_NAME ) + " is null" 
                        + " or " + OM_BSON_FIELD_SDB_USER + " is null"
                        + " or " + OM_BSON_FIELD_SDB_PASSWD + " is null"
                        + " or " + OM_BSON_FIELD_SDB_USERGROUP + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      if ( 0 == installPath.length() )
      {
         installPath = OM_DEFAULT_INSTALL_PATH ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omCreateClusterCommand::doCommand()
   {
      string clusterName ;
      string desc ;
      string sdbUser ;
      string sdbPasswd ;
      string sdbUserGroup ;
      string sdbinstallPath ;
      BSONObjBuilder resBuilder ;
      BSONObj bsonCluster ;
      INT32 rc                 = SDB_OK ;

      rc = _getClusterInfo( clusterName, desc, sdbUser, sdbPasswd, 
                            sdbUserGroup, sdbinstallPath ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get cluster info failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      // duplicate check depends on the unique index of table(OM_CS_DEPLOY_CL_CLUSTERIDX1)
      bsonCluster = BSON( OM_CLUSTER_FIELD_NAME << clusterName
                          << OM_CLUSTER_FIELD_DESC << desc
                          << OM_CLUSTER_FIELD_SDBUSER << sdbUser
                          << OM_CLUSTER_FIELD_SDBPASSWD << sdbPasswd
                          << OM_CLUSTER_FIELD_SDBUSERGROUP << sdbUserGroup
                          << OM_CLUSTER_FIELD_INSTALLPATH << sdbinstallPath ) ;
      rc = rtnInsert( OM_CS_DEPLOY_CL_CLUSTER, bsonCluster, 1, 0, _cb );
      if ( rc )
      {
         if ( SDB_IXM_DUP_KEY == rc )
         {
            _errorDetail = clusterName + " is already exist" ;
            PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
         }
         else
         {
            _errorDetail = string("failed to insert cluster:") + clusterName ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
         }

         goto error ;
      }

      resBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      resBuilder.append( OM_BSON_FIELD_CLUSTER_NAME, clusterName.c_str() ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, resBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return SDB_OK ;
   error:
      goto done ;
   }

   // *****************omQueryClusterCommand *****************************
   omQueryClusterCommand::omQueryClusterCommand( restAdaptor *pRestAdaptor, 
                                                 pmdRestSession *pRestSession )
                         : omCreateClusterCommand( pRestAdaptor, pRestSession )
   {
   }

   omQueryClusterCommand::~omQueryClusterCommand()
   {
   }

   INT32 omQueryClusterCommand::doCommand()
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_CLUSTER ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      while ( TRUE )
      {
         BSONObj tmp ;
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_CLUSTER ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         tmp = BSON( OM_BSON_FIELD_CLUSTER_NAME 
                     << result.getStringField( OM_CLUSTER_FIELD_NAME )
                     << OM_BSON_FIELD_CLUSTER_DESC
                     << result.getStringField( OM_CLUSTER_FIELD_DESC )
                     << OM_BSON_FIELD_SDB_USER
                     << result.getStringField( OM_CLUSTER_FIELD_SDBUSER )
                     << OM_BSON_FIELD_SDB_USERGROUP
                     << result.getStringField( OM_CLUSTER_FIELD_SDBUSERGROUP )
                     << OM_BSON_FIELD_INSTALLPATH
                     << result.getStringField( OM_CLUSTER_FIELD_INSTALLPATH )) ;
         rc = _restAdaptor->appendHttpBody( _restSession, tmp.objdata(), 
                                            tmp.objsize(), 1 ) ;
         if ( rc )
         {
            _errorDetail = string( "falied to append http body" ) ;
            PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
            goto error ;
         }
      }

      bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      result = bsonBuilder.obj() ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, result ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return SDB_OK ;
   error:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      goto done ;
   }

   // ***************** omScanHostCommand *****************************
   omScanHostCommand::omScanHostCommand( restAdaptor *pRestAdaptor, 
                                         pmdRestSession *pRestSession, 
                                         string localAgentHost, 
                                         string localAgentService )
                     : omCreateClusterCommand( pRestAdaptor, pRestSession ),
                       _localAgentHost( localAgentHost ), 
                       _localAgentService( localAgentService )
   {
   }

   omScanHostCommand::~omScanHostCommand()
   {
   }

   // generate the bson array(result) for the list of hosts(hostInfoList), 
   // with array's keyname is (arrayKeyName)
   void omScanHostCommand::_generateArray( list<BSONObj> &hostInfoList, 
                                           string arrayKeyName, 
                                           BSONObj &result )
   {
      BSONObjBuilder builder ;
      BSONArrayBuilder arrayBuilder ;

      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         arrayBuilder.append( *ite ) ;
         ite++ ;
      }

      builder.appendArray( arrayKeyName.c_str(), arrayBuilder.arr() );
      result = builder.obj() ;

      return ;
   }

   void omScanHostCommand::_sendOkRes2Web( list<BSONObj> &hostResult )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bsonBuilder ;

      list<BSONObj>::iterator ite = hostResult.begin() ;
      while ( ite != hostResult.end() )
      {
         rc = _restAdaptor->appendHttpBody( _restSession, ite->objdata(), 
                                       ite->objsize(), 1 ) ;
         if ( rc )
         {
            PD_LOG(PDERROR, "Failed to appendHttpBody:rc=%d", rc ) ;
            _sendErrorRes2Web( rc, "Failed to appendHttpBody" ) ;
            goto error ;
         }

         ite++ ;
      }

      bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return ;
   error:
      goto done ;
   }

   // move the exist host to the hostResult
   void omScanHostCommand::_filterExistHost( list<BSONObj> &hostInfoList, 
                                             list<BSONObj> &hostResult )
   {
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         if ( _isHostExist( *ite ) )
         {
            BSONObj tmp = BSON( OM_BSON_FIELD_HOST_IP
                               << ite->getStringField( OM_BSON_FIELD_HOST_IP )
                               << OM_BSON_FIELD_HOST_NAME
                               << ite->getStringField( OM_BSON_FIELD_HOST_NAME )
                               << OM_REST_RES_RETCODE << SDB_IXM_DUP_KEY
                               << OM_REST_RES_DETAIL << "host is exist" ) ;
            hostResult.push_back( tmp ) ;

            hostInfoList.erase( ite++ ) ;
            continue ;
         }

         ite++ ;
      }
   }

   bool omScanHostCommand::_isHostExist( BSONObj &host )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID   = -1 ;
      INT32 rc           = SDB_OK ;
      rtnContextBuf buffObj ;
      SINT64 startingPos = 0 ;

      if ( host.hasField( OM_BSON_FIELD_HOST_NAME ) )
      {
         bsonBuilder.append( OM_HOST_FIELD_NAME, 
                             host.getStringField( OM_BSON_FIELD_HOST_NAME ) ) ;
      }

      if ( host.hasField( OM_BSON_FIELD_HOST_IP ) )
      {
         bsonBuilder.append( OM_HOST_FIELD_IP, 
                             host.getStringField( OM_BSON_FIELD_HOST_IP ) ) ;
      }

      matcher = bsonBuilder.obj() ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to query host:rc=%d,host=%s", rc, 
                 matcher.toString().c_str() ) ;
         return false ;
      }

      rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
      if ( rc )
      {
         if ( SDB_DMS_EOC != rc )
         {
            PD_LOG( PDERROR, "Failed to retreive record, rc = %d", rc ) ;
         }

         // notice: if rc != SDB_OK, contextID is deleted in rtnGetMore
         return false ;
      }

      _pRTNCB->contextDelete( contextID, _cb ) ;

      return true ;
   }

   INT32 omScanHostCommand::_getHostList( string &clusterName, 
                                          list<BSONObj> &hostInfo )
   {
      INT32 rc                     = SDB_OK ;
      const CHAR* pGlobalUser      = NULL ;
      const CHAR* pGlobalPasswd    = NULL ;
      const CHAR* pGlobalSshPort   = NULL ;
      const CHAR* pGlobalAgentPort = NULL ;
      const CHAR* pHostInfo        = NULL ;
      BSONObj bsonHostInfo ;
      BSONElement element ;


      _restAdaptor->getQuery( _restSession, OM_REST_FIELD_HOST_INFO, 
                              &pHostInfo ) ;
      if ( NULL == pHostInfo )
      {
         _errorDetail = string( OM_REST_FIELD_HOST_INFO ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pHostInfo, bsonHostInfo ) ;
      if ( rc )
      {
         _errorDetail = string( "change rest field " ) + OM_REST_FIELD_HOST_INFO
                        + " to BSONObj failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      pGlobalUser      = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_USER ) ;
      pGlobalPasswd    = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_PASSWD ) ;
      pGlobalSshPort   = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_SSHPORT ) ;
      pGlobalAgentPort = _localAgentService.c_str() ;
      clusterName      = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_CLUSTER_NAME ) ;
      if ( 0 == ossStrlen( pGlobalUser ) || 0 == ossStrlen( pGlobalPasswd )
           || 0 == ossStrlen( pGlobalSshPort ) || 0 == clusterName.length()
           || 0 == ossStrlen( pGlobalAgentPort ) )
      {
         _errorDetail = string( OM_BSON_FIELD_HOST_USER ) + " is null"
                        + " or " + OM_BSON_FIELD_HOST_PASSWD + " is null"
                        + " or " + OM_BSON_FIELD_HOST_SSHPORT + " is null"
                        + " or " + OM_BSON_FIELD_CLUSTER_NAME + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      element = bsonHostInfo.getField( OM_BSON_FIELD_HOST_INFO ) ;
      if ( element.isNull() || Array != element.type() )
      {
         _errorDetail = string( OM_BSON_FIELD_HOST_INFO ) 
                        + " is not array type" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s:type=%d", _errorDetail.c_str(), element.type() ) ;
         goto error ;
      }

      {
         BSONObjIterator i( element.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            BSONObj oneHost = ele.embeddedObject() ;
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_IP ) 
                    && !oneHost.hasField( OM_BSON_FIELD_HOST_NAME ) )
            {
               rc = SDB_INVALIDARG ;
               _errorDetail = string(OM_BSON_FIELD_HOST_IP) + " or " 
                              + OM_BSON_FIELD_HOST_NAME + " is not exist" ;
               PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
               goto error ;
            }

            builder.appendElements( oneHost ) ;
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_USER ) )
            {
               builder.append( OM_BSON_FIELD_HOST_USER, pGlobalUser ) ;
            }
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_PASSWD ) )
            {
               builder.append( OM_BSON_FIELD_HOST_PASSWD, pGlobalPasswd) ;
            }
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_SSHPORT ) )
            {
               builder.append( OM_BSON_FIELD_HOST_SSHPORT, pGlobalSshPort) ;
            }
            if ( !oneHost.hasField( OM_BSON_FIELD_AGENT_PORT ) )
            {
               builder.append( OM_BSON_FIELD_AGENT_PORT, pGlobalAgentPort) ;
            }

            tmp = builder.obj() ;
            hostInfo.push_back( tmp ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void omScanHostCommand::_clearSession( omManager *om, 
                                          pmdRemoteSession *remoteSession ) 
   {
      if ( NULL != remoteSession )
      {
         pmdSubSession *pSubSession = NULL ;
         pmdSubSessionItr itr       = remoteSession->getSubSessionItr() ;
         while ( itr.more() )
         {
            pSubSession = itr.next() ;
            MsgHeader *pMsg = pSubSession->getReqMsg() ;
            if ( NULL != pMsg )
            {
               SDB_OSS_FREE( pMsg ) ;
            }
         }

         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( remoteSession ) ;
      }
   }

   INT32 omScanHostCommand::_sendMsgToLocalAgent( omManager *om,
                                             pmdRemoteSession *remoteSession, 
                                             MsgHeader *pMsg )
   {
      MsgRouteID localAgentID ;
      INT32 rc = SDB_OK ;

      localAgentID = om->updateAgentInfo( _localAgentHost, 
                                          _localAgentService ) ;
      if ( NULL == remoteSession->addSubSession( localAgentID.value ) )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSubSession failed:id=%ld", localAgentID.value ) ;
         goto error ;
      }

      rc = remoteSession->sendMsg( pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send msg to localhost's agent failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omScanHostCommand::_receiveFromAgent( pmdRemoteSession *remoteSession,
                                               BSONObj &result )
   {
      VEC_SUB_SESSIONPTR subSessionVec ;
      INT32 rc           = SDB_OK ;
      MsgHeader *pRspMsg = NULL ;
      SINT32 flag        = 0 ;
      SINT64 contextID   = -1 ;
      SINT32 startFrom   = 0 ;
      SINT32 numReturned = 0 ;
      vector<BSONObj> objVec ;

      rc = remoteSession->waitReply( TRUE, &subSessionVec ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "wait reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( 1 != subSessionVec.size() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "unexpected session size:size=%d", 
                 subSessionVec.size() ) ;
         goto error ;
      }

      if ( subSessionVec[0]->isDisconnect() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG(PDERROR, "session disconected:id=%s,rc=%d", 
                routeID2String(subSessionVec[0]->getNodeID()).c_str(), rc ) ;
         goto error ;
      }

      pRspMsg = subSessionVec[0]->getRspMsg() ;
      if ( NULL == pRspMsg )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive null response:rc=%d", rc ) ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)pRspMsg, &flag, &contextID, &startFrom, 
                            &numReturned, objVec ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "extract reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( 1 != objVec.size() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "unexpected response size:rc=%d", rc ) ;
         goto error ;
      }

      result = objVec[0] ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omScanHostCommand::_parseResonpse( VEC_SUB_SESSIONPTR &subSessionVec, 
                                            BSONObj &response,
                                            list<BSONObj> &bsonResult )
   {
      INT32 rc           = SDB_OK ;

      BSONElement agentRCElement ;
      BSONElement hostElement ;

      agentRCElement = response.getField( OM_REST_RES_RETCODE ) ;
      if ( agentRCElement.eoo() || NumberInt != agentRCElement.type())
      {
         rc = SDB_UNEXPECTED_RESULT ;
         _errorDetail = string( "agent's response is unreconigzed" ) ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rc = response.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "agent process failed:detail=" ) 
                        + response.getStringField( OM_REST_RES_DETAIL ) ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      hostElement = response.getField( OM_BSON_FIELD_HOST_INFO ) ;
      if ( hostElement.isNull() || Array != hostElement.type() )
      {
         _errorDetail = string( "agent's response is unrecognized:" ) 
                        + OM_BSON_FIELD_HOST_INFO + " is not array type" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }
      {
         BSONObjIterator i( hostElement.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            BSONObj oneHost = ele.embeddedObject() ;
            bsonResult.push_back( oneHost ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omScanHostCommand::doCommand()
   {
      string clusterName ;
      list<BSONObj> hostInfoList ;
      INT32 rc                        = SDB_OK ;
      CHAR* pContent                  = NULL ;
      INT32 contentSize               = 0 ;
      MsgHeader *pMsg                 = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonRequest ;
      BSONObj bsonResponse ;
      list<BSONObj> bsonResult ;
      pmdEDUEvent eventData ;
      omManager *om                   = NULL ;
      VEC_SUB_SESSIONPTR subSessionVec ;

      rc = _getHostList( clusterName, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get host list failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _filterExistHost( hostInfoList, bsonResult ) ;
      if ( hostInfoList.size() == 0 )
      {
         _sendOkRes2Web( bsonResult ) ;
         goto done ;
      }

      // build request to agent
      _generateArray( hostInfoList, OM_REST_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_SCAN_HOST_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "build message failed:cmd=" ) 
                        + OM_SCAN_HOST_REQ ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      // send request to agent
      om   = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_WAIT_SCAN_RES_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         _errorDetail = string( "create remote session failed" ) ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         SDB_OSS_FREE( pContent ) ;
         _sendErrorRes2Web( SDB_OOM, _errorDetail ) ;
         goto error ;
      }

      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "send message to agent failed" ) ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _receiveFromAgent( remoteSession, bsonResponse ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "receive from agent failed" ) ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _parseResonpse( subSessionVec, bsonResponse, bsonResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_parseResonpse failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _sendOkRes2Web( bsonResult ) ;

   done:
      _clearSession( om, remoteSession ) ;
      return rc; 
   error:
      goto done ;
   }

   // *****************omCheckHostCommand *****************************
   omCheckHostCommand::omCheckHostCommand( restAdaptor *pRestAdaptor, 
                                           pmdRestSession *pRestSession,
                                           string localAgentHost, 
                                           string localAgentService )
                      : omScanHostCommand( pRestAdaptor, pRestSession, 
                                           localAgentHost, localAgentService )
   {
   }

   omCheckHostCommand::~omCheckHostCommand()
   {
   }

   void omCheckHostCommand::_eraseFromListByIP( list<BSONObj> &hostInfoList, 
                                                const string &ip )
   {
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         string tmpIP = ite->getStringField( OM_BSON_FIELD_HOST_IP ) ;
         if ( tmpIP.compare( ip ) == 0 )
         {
            hostInfoList.erase( ite ) ;
            return ;
         }
         ite++ ;
      }
   }

   void omCheckHostCommand::_eraseFromListByHost( list<BSONObj> &hostInfoList, 
                                                  const string &hostName )
   {
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         string tmpHost = ite->getStringField( OM_BSON_FIELD_HOST_NAME ) ;
         if ( tmpHost.compare( hostName ) == 0 )
         {
            hostInfoList.erase( ite ) ;
            return ;
         }
         ite++ ;
      }
   }

   void omCheckHostCommand::_eraseFromList( list<BSONObj> &hostInfoList, 
                                            BSONObj &oneHost )
   {
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      string hostName = oneHost.getStringField( OM_BSON_FIELD_HOST_NAME ) ;
      _eraseFromListByHost( hostInfoList, hostName ) ;
   }

   // check ping and ssh
   INT32 omCheckHostCommand::_doBasicCheck( list<BSONObj> &hostInfoList, 
                                            list<BSONObj> &hostResult )
   {
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      MsgHeader *pMsg   = NULL ;
      omManager *om     = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonRequest ;
      BSONObj result ;
      BSONElement rcElement ;
      BSONElement hostElement ;

      _generateArray(hostInfoList, OM_BSON_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_BASIC_CHECK_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "build query msg failed:cmd=" ) 
                        + OM_BASIC_CHECK_REQ ;
         PD_LOG(PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      // create remote session
      om            = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_BASICCHECK_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         _errorDetail = "add remote session failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "send message to agent failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      // receiving for agent's response
      rc = _receiveFromAgent( remoteSession, result ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "receive response from agent failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rcElement = result.getField( OM_REST_RES_RETCODE ) ;
      if ( rcElement.eoo() || NumberInt != rcElement.type())
      {
         rc = SDB_UNEXPECTED_RESULT ;
         _errorDetail = "agent's response is unreconigzed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rc = result.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = result.getStringField( OM_REST_RES_DETAIL ) ;
         PD_LOG( PDERROR, "agent process %s failed:detail=%s,rc=%d", 
                 OM_BASIC_CHECK_REQ, _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      hostElement = result.getField( OM_BSON_FIELD_HOST_INFO ) ;
      if ( hostElement.isNull() || Array != hostElement.type() )
      {
         _errorDetail = string( "agent's response is unrecognized:" ) 
                        + string(OM_BSON_FIELD_HOST_INFO) 
                        + " is not array type" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }
      {
         BSONObjIterator i( hostElement.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            BSONObj oneHost = ele.embeddedObject() ;
            rcElement       = oneHost.getField( OM_REST_RES_RETCODE ) ;
            if ( rcElement.eoo() || NumberInt != rcElement.type())
            {
               _errorDetail = string( "agent's response is unrecognized:host=" )
                              + oneHost.toString() ;
               rc = SDB_UNEXPECTED_RESULT ;
               PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
               goto error ;
            }
            // remove the basic check failure host to the hostResult 
            rc = result.getIntField( OM_REST_RES_RETCODE ) ;
            if ( SDB_OK != rc )
            {
               hostResult.push_back( oneHost ) ;
               _eraseFromList( hostInfoList, oneHost ) ;
            }
         }
      }

   done:
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 omCheckHostCommand::_installAgent( list<BSONObj> &hostInfoList, 
                                            list<BSONObj> &needUninstallHost )
   {
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      MsgHeader *pMsg   = NULL ;
      omManager *om     = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonRequest ;
      BSONObj result ;
      BSONObj hostResults ;
      BSONElement rcElement ;

      _generateArray( hostInfoList, OM_BSON_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_INSTALL_REMOTE_AGENT, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "build query msg failed:cmd=%s,rc=%d", 
                OM_INSTALL_REMOTE_AGENT, rc ) ;
         goto error ;
      }

      // create remote session
      om            = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_INSTALL_AGET_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      // receiving for agent's response
      rc = _receiveFromAgent( remoteSession, result ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "receive from agent failed:rc=%d", rc ) ;
         goto error ;
      }

      rcElement = result.getField( OM_REST_RES_RETCODE ) ;
      if ( rcElement.eoo() || NumberInt != rcElement.type())
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive unexpected response:rc=%d", rc ) ;
         goto error ;
      }

      rc = result.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      needUninstallHost.assign( hostInfoList.begin(), hostInfoList.end() ) ;
      hostResults = result.getObjectField( OM_BSON_FIELD_HOST_INFO ) ;
      {
         BSONObjIterator iter( hostResults ) ;
         while ( iter.more() )
         {
            BSONElement ele ;
            BSONObj oneResult ;
            string ip ;
            bool isNeedUninstall ;
            ele       = iter.next() ;
            oneResult = ele.embeddedObject() ;
            ip        = oneResult.getStringField( OM_BSON_FIELD_HOST_IP ) ;
            isNeedUninstall = oneResult.getBoolField( 
                                                 OM_BSON_FIELD_NEEDUNINSTALL ) ;
            if ( !isNeedUninstall )
            {
               _eraseFromListByIP( needUninstallHost, ip ) ;
            }
         }
      }

   done:
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   // create the requet for all the hosts in hostInfoList
   INT32 omCheckHostCommand::_addCheckHostReq( omManager *om,
                                               pmdRemoteSession *remoteSession,
                                               list<BSONObj> &hostInfoList ) 
   {
      INT32 rc = SDB_OK ;
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         MsgRouteID routeID ;
         string agentHost ;
         string agentIP ;
         string agentPort ;
         BSONObjBuilder reqBuilder ;
         BSONObj bsonRequest ;
         pmdSubSession *subSession = NULL ;
         CHAR *pContent            = NULL ;
         INT32 contentSize         = 0 ;
         agentIP   = ite->getStringField( OM_BSON_FIELD_HOST_IP ) ;
         agentHost = ite->getStringField( OM_BSON_FIELD_HOST_NAME ) ;
         agentPort = ite->getStringField( OM_BSON_FIELD_AGENT_PORT ) ;
         routeID   = om->updateAgentInfo( agentIP, agentPort ) ;
         subSession = remoteSession->addSubSession( routeID.value ) ;
         if ( NULL == subSession )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "addSubSessin failed" ) ;
            goto error ;
         }

         reqBuilder.append( OM_BSON_FIELD_HOST_IP, agentIP ) ;
         reqBuilder.append( OM_BSON_FIELD_HOST_NAME, agentHost ) ;
         reqBuilder.append( OM_BSON_FIELD_HOST_USER,
                            ite->getStringField( OM_BSON_FIELD_HOST_USER ) ) ;
         reqBuilder.append( OM_BSON_FIELD_HOST_PASSWD,
                            ite->getStringField( OM_BSON_FIELD_HOST_PASSWD ) ) ;
         bsonRequest = reqBuilder.obj() ;
         rc = msgBuildQueryMsg( &pContent, &contentSize, 
                                CMD_ADMIN_PREFIX OM_CHECK_HOST_REQ,
                                0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "msgBuildQueryMsg failed:rc=%d", rc ) ;
            goto error ;
         }

         subSession->setReqMsg( (MsgHeader *)pContent ) ;
         ite++;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void omCheckHostCommand::_updateDiskInfo( BSONObj &onehost )
   {
      BSONObj filter = BSON( OM_BSON_FIELD_DISK << "" ) ;
      BSONObj others ;
      BSONObj disk ;
      others = onehost.filterFieldsUndotted( filter, false ) ;
      disk   = onehost.filterFieldsUndotted( filter, true ) ;

      BSONArrayBuilder arrayBuilder ;

      BSONObj disks = disk.getObjectField( OM_BSON_FIELD_DISK ) ;
      BSONObjIterator iter( disks ) ;
      while ( iter.more() )
      {
         BSONElement ele ;
         BSONObj oneDisk ;
         ele     = iter.next() ;
         oneDisk = ele.embeddedObject() ;

         BSONObjBuilder builder ;
         builder.appendElements( oneDisk ) ;

         BSONElement sizeEle ;
         sizeEle = oneDisk.getField( OM_BSON_FIELD_DISK_FREE_SIZE ) ;
         INT64 freeSize = sizeEle.Long() ;
         if ( freeSize < OM_MIN_DISK_FREE_SIZE )
         {
            builder.append( OM_BSON_FIELD_DISK_CANUSE, false ) ;
         }
         else
         {
            builder.append( OM_BSON_FIELD_DISK_CANUSE, true ) ;
         }

         BSONObj tmp = builder.obj() ;
         arrayBuilder.append( tmp ) ;
      }

      disk = BSON( OM_BSON_FIELD_DISK << arrayBuilder.arr() ) ;

      BSONObjBuilder builder ;
      builder.appendElements( others ) ;
      builder.appendElements( disk ) ;
      onehost = builder.obj() ;
   }

   INT32 omCheckHostCommand::_checkHostEnv( list<BSONObj> &hostInfoList, 
                                            list<BSONObj> &hostResult )
   {
      INT32 rc          = SDB_OK ;
      omManager *om     = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      VEC_SUB_SESSIONPTR subSessionVec ;
      list<BSONObj>::iterator ite ;
      INT32 sucNum   = 0 ; 
      INT32 totalNum = 0 ;

      // create remote session
      om            = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_CHECK_HOST_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         goto error ;
      }
      rc = _addCheckHostReq( om, remoteSession, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "addCheckHostReq failed:rc=%d", rc ) ;
         goto done ;
      }

      remoteSession->sendMsg( &sucNum, &totalNum ) ;
      rc = remoteSession->waitReply( TRUE, &subSessionVec ) ;
      if ( SDB_OK != rc && SDB_TIMEOUT != rc )
      {
         PD_LOG( PDERROR, "wait replay failed:rc=%d", rc ) ;
         goto error ;
      }

      for ( UINT32 i = 0 ; i < subSessionVec.size() ; i++ )
      {
         vector<BSONObj> objVec ;
         SINT32 flag               = 0 ;
         SINT64 contextID          = -1 ;
         SINT32 startFrom          = 0 ;
         SINT32 numReturned        = 0 ;
         MsgHeader* pRspMsg        = NULL ;
         pmdSubSession *subSession = subSessionVec[i] ;
         if ( subSession->isDisconnect() )
         {
            rc = SDB_UNEXPECTED_RESULT ;
            PD_LOG(PDERROR, "session disconnected:id=%s,rc=%d", 
                   routeID2String(subSession->getNodeID()).c_str(), rc ) ;
            continue ;
         }

         pRspMsg = subSession->getRspMsg() ;
         if ( NULL == pRspMsg )
         {
            rc = SDB_UNEXPECTED_RESULT ;
            PD_LOG(PDERROR, "unexpected result:rc=%d", rc ) ;
            goto error ;
         }

         rc = msgExtractReply( (CHAR *)pRspMsg, &flag, &contextID, &startFrom, 
                               &numReturned, objVec ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "extract reply failed:rc=%d", rc ) ;
            goto error ;
         }

         if ( 1 != objVec.size() )
         {
            rc = SDB_UNEXPECTED_RESULT ;
            PD_LOG( PDERROR, "unexpected response size:rc=%d", rc ) ;
            goto error ;
         }

         BSONObj result = objVec[0] ;
         _updateDiskInfo( result ) ;
         hostResult.push_back( result ) ;
         //TODO: get /etc/hosts, and check 
         _eraseFromList( hostInfoList, result ) ;
      }

      ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         BSONObjBuilder builder ;
         BSONObj tmp ;
         builder.append( OM_BSON_FIELD_HOST_IP, 
                         ite->getStringField( OM_BSON_FIELD_HOST_IP ) ) ;
         builder.append( OM_BSON_FIELD_HOST_NAME,
                         ite->getStringField( OM_BSON_FIELD_HOST_NAME ) ) ;
         builder.append( OM_REST_RES_RETCODE, SDB_NETWORK ) ;
         builder.append( OM_REST_RES_DETAIL, "network error" ) ;

         tmp = builder.obj() ;
         hostResult.push_back( tmp ) ;
         hostInfoList.erase( ite++ ) ;
      }

   done:
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 omCheckHostCommand::_addAgentExitReq( omManager *om,
                                               pmdRemoteSession *remoteSession,
                                               list<BSONObj> &hostInfoList ) 
   {
      INT32 rc = SDB_OK ;
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         MsgRouteID routeID ;
         string agentHost ;
         string agentIP ;
         string agentPort ;
         pmdSubSession *subSession = NULL ;
         CHAR *pContent            = NULL ;
         INT32 contentSize         = 0 ;
         agentIP   = ite->getStringField( OM_BSON_FIELD_HOST_IP ) ;
         agentHost = ite->getStringField( OM_BSON_FIELD_HOST_NAME ) ;
         agentPort = ite->getStringField( OM_BSON_FIELD_AGENT_PORT ) ;
         routeID   = om->updateAgentInfo( agentIP, agentPort ) ;
         subSession = remoteSession->addSubSession( routeID.value ) ;
         if ( NULL == subSession )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "addSubSessin failed" ) ;
            goto error ;
         }

         rc = msgBuildQueryMsg( &pContent, &contentSize, 
                                CMD_ADMIN_PREFIX CMD_NAME_SHUTDOWN,
                                0, 0, 0, -1, NULL, NULL, NULL, NULL ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "msgBuildQueryMsg failed:rc=%d", rc ) ;
            goto error ;
         }

         subSession->setReqMsg( (MsgHeader *)pContent ) ;
         ite++;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omCheckHostCommand::_notifyAgentExit( list<BSONObj> &hostInfoList )
   {
      INT32 rc                        = SDB_OK ;
      omManager *om                   = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      INT32 sucNum                    = 0 ; 
      INT32 totalNum                  = 0 ;
      VEC_SUB_SESSIONPTR subSessionVec ;
      list<BSONObj>::iterator ite ;

      // create remote session
      om            = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                OM_WAIT_AGENT_EXIT_RES_INTERVAL,
                                                NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         goto error ;
      }
      rc = _addAgentExitReq( om, remoteSession, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_addAgentExitReq failed:rc=%d", rc ) ;
         goto done ;
      }

      remoteSession->sendMsg( &sucNum, &totalNum ) ;
      remoteSession->waitReply( TRUE, &subSessionVec ) ;
   done:
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 omCheckHostCommand::_uninstallAgent( list<BSONObj> &hostInfoList )
   {
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      MsgHeader *pMsg   = NULL ;
      omManager *om     = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonRequest ;
      VEC_SUB_SESSIONPTR subSessionVec ;

      if ( hostInfoList.size() == 0 )
      {
         goto done ;
      }

      _notifyAgentExit( hostInfoList ) ;

      _generateArray( hostInfoList, OM_REST_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_UNINSTALL_REMOTE_AGENT, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "build query msg failed:cmd=%s,rc=%d", 
                OM_UNINSTALL_REMOTE_AGENT, rc ) ;
         goto error ;
      }

      // create remote session
      om            = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                OM_WAIT_AGENT_UNISTALL_INTERVAL,
                                                NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      remoteSession->waitReply( TRUE, &subSessionVec ) ;

   done:
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   // check os/cpu/network etc (and get those infomations )
   INT32 omCheckHostCommand::_doCheck( list<BSONObj> &hostInfoList, 
                                       list<BSONObj> &hostResult )
   {
      INT32 rc = SDB_OK ;
      list<BSONObj> needUninstallHost ;

      PD_LOG( PDEVENT, "start to _installAgent" ) ;
      rc = _installAgent( hostInfoList, needUninstallHost ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "install agent failed" ;
         PD_LOG(PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      PD_LOG( PDEVENT, "start to _checkHostEnv" ) ;
      rc = _checkHostEnv( hostInfoList, hostResult ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "check host env failed" ;
         PD_LOG(PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

   done:
      PD_LOG( PDEVENT, "start to _uninstallAgent" ) ;
      _uninstallAgent( needUninstallHost ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 omCheckHostCommand::doCommand()
   {
      list<BSONObj> hostInfoList ;
      list<BSONObj> hostResult ;
      BSONObj bsonRequest ;
      string clusterName = "" ;
      INT32 rc           = SDB_OK ;

      rc = _getHostList( clusterName, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "fail to get host list:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      // move the exist host to hostResult
      _filterExistHost( hostInfoList, hostResult ) ;

      // move the check failed host to the hostResult
      PD_LOG( PDEVENT, "start to do BasicCheck" ) ;
      rc = _doBasicCheck( hostInfoList, hostResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "do basic check failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _doCheck( hostInfoList, hostResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "do check failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _sendOkRes2Web( hostResult ) ;

   done:
      return rc; 
   error:
      goto done ;
   }

   // *****************omAddHostCommand *****************************
   omAddHostCommand::omAddHostCommand( restAdaptor *pRestAdaptor, 
                                       pmdRestSession *pRestSession,
                                       string localAgentHost, 
                                       string localAgentService )
                    : omScanHostCommand( pRestAdaptor, pRestSession, 
                                         localAgentHost, localAgentService )
   {
   }

   omAddHostCommand::~omAddHostCommand()
   {
   }

   INT32 omAddHostCommand::_getHostList( string &clusterName, 
                                         list<BSONObj> &hostInfo )
   {
      INT32 rc                     = SDB_OK ;
      const CHAR* pGlobalUser      = NULL ;
      const CHAR* pGlobalPasswd    = NULL ;
      const CHAR* pGlobalSshPort   = NULL ;
      const CHAR* pGlobalAgentPort = NULL ;
      const CHAR* pHostInfo        = NULL ;
      string installPath ;
      BSONObj bsonHostInfo ;
      BSONElement element ;

      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_HOST_INFO, 
                             &pHostInfo ) ;
      if ( NULL == pHostInfo )
      {
         _errorDetail = "rest field:" + 
                        string( OM_REST_FIELD_HOST_INFO ) + " is null" ;
         rc           = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pHostInfo, bsonHostInfo ) ;
      if ( rc )
      {
         _errorDetail = string( "change rest field " ) + OM_REST_FIELD_HOST_INFO
                        + " to BSONObj failed";
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      pGlobalUser      = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_USER ) ;
      pGlobalPasswd    = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_PASSWD ) ;
      pGlobalSshPort   = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_SSHPORT ) ;
      pGlobalAgentPort = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_AGENT_PORT ) ;
      clusterName    = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_CLUSTER_NAME ) ;
      if ( 0 == ossStrlen( pGlobalUser ) || 0 == ossStrlen( pGlobalPasswd )
           || 0 == ossStrlen( pGlobalSshPort ) || 0 == clusterName.length()
           || 0 == ossStrlen( pGlobalAgentPort ) )
      {
         _errorDetail = string( OM_BSON_FIELD_HOST_USER ) + " is null"
                        + " or " + OM_BSON_FIELD_HOST_PASSWD + " is null"
                        + " or " + OM_BSON_FIELD_HOST_SSHPORT + " is null"
                        + " or " + OM_BSON_FIELD_AGENT_PORT + " is null"
                        + " or " + OM_BSON_FIELD_CLUSTER_NAME + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s:info=%s", _errorDetail.c_str(), pHostInfo ) ;
         goto error ;
      }

      rc = _getClusterInstallPath( clusterName, installPath ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s:info=%s", _errorDetail.c_str(), pHostInfo ) ;
         goto error ;
      }

      element = bsonHostInfo.getField( OM_BSON_FIELD_HOST_INFO ) ;
      {
         BSONObjIterator i( element.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            BSONObj oneHost = ele.embeddedObject() ;
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_IP ) 
                    || !oneHost.hasField( OM_BSON_FIELD_HOST_NAME ) )
            {
               rc = SDB_INVALIDARG ;
               _errorDetail = string(OM_BSON_FIELD_HOST_IP) + " or " 
                              + OM_BSON_FIELD_HOST_NAME 
                              + "have not exist" ;
               PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
               goto error ;
            }

            builder.appendElements( oneHost ) ;
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_USER ) )
            {
               builder.append( OM_BSON_FIELD_HOST_USER, pGlobalUser ) ;
            }

            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_PASSWD ) )
            {
               builder.append( OM_BSON_FIELD_HOST_PASSWD, pGlobalPasswd) ;
            }

            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_SSHPORT ) )
            {
               builder.append( OM_BSON_FIELD_HOST_SSHPORT, pGlobalSshPort) ;
            }

            if ( !oneHost.hasField( OM_BSON_FIELD_AGENT_PORT ) )
            {
               builder.append( OM_BSON_FIELD_AGENT_PORT, pGlobalAgentPort) ;
            }

            if ( !oneHost.hasField( OM_BSON_FIELD_INSTALL_PATH ) )
            {
               builder.append( OM_BSON_FIELD_INSTALL_PATH, installPath) ;
            }

            tmp = builder.obj() ;
            hostInfo.push_back( tmp ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omAddHostCommand::_generateAddHostReq( string clusterName,
                                               list<BSONObj> &hostInfoList, 
                                               BSONObj &bsonRequest )
   {
      BSONObjBuilder builder ;
      BSONArrayBuilder arrayBuilder ;
      list<BSONObj>::iterator ite ;

      INT32 rc = SDB_OK ;
      string sdbUser ;
      string sdbPasswd ;
      string sdbUserGroup ;
      CHAR packetPath[ OSS_MAX_PATHSIZE ] = "" ;
      rc = _getPacketFullPath( packetPath ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get packet path failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = _getSdbUsrInfo( clusterName, sdbUser, sdbPasswd, sdbUserGroup ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_getSdbUsrInfo failed:rc=%d", rc ) ;
         goto error ;
      }

      ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         BSONObjBuilder innerBuilder ;
         BSONObj temp ;
         innerBuilder.append( OM_BSON_FIELD_HOST_IP, 
                         ite->getStringField( OM_BSON_FIELD_HOST_IP ) ) ;
         innerBuilder.append( OM_BSON_FIELD_HOST_NAME, 
                         ite->getStringField( OM_BSON_FIELD_HOST_NAME ) ) ;
         innerBuilder.append( OM_BSON_FIELD_HOST_USER, 
                         ite->getStringField( OM_BSON_FIELD_HOST_USER ) ) ;
         innerBuilder.append( OM_BSON_FIELD_HOST_PASSWD, 
                         ite->getStringField( OM_BSON_FIELD_HOST_PASSWD ) ) ;
         innerBuilder.append( OM_BSON_FIELD_HOST_SSHPORT, 
                         ite->getStringField( OM_BSON_FIELD_HOST_SSHPORT ) ) ;
         innerBuilder.append( OM_BSON_FIELD_AGENT_PORT, 
                         ite->getStringField( OM_BSON_FIELD_AGENT_PORT ) ) ;
         innerBuilder.append( OM_BSON_FIELD_INSTALL_PATH, 
                         ite->getStringField( OM_BSON_FIELD_INSTALL_PATH ) ) ;
         temp = innerBuilder.obj() ;
         arrayBuilder.append( temp ) ;
         ite++ ;
      }

      builder.append( OM_BSON_FIELD_SDB_USER, sdbUser ) ;
      builder.append( OM_BSON_FIELD_SDB_PASSWD, sdbPasswd ) ;
      builder.append( OM_BSON_FIELD_SDB_USERGROUP, sdbUserGroup ) ;
      builder.append( OM_BSON_FIELD_PATCKET_PATH, packetPath ) ;
      builder.appendArray( OM_REST_FIELD_HOST_INFO, arrayBuilder.arr() );
      bsonRequest = builder.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omAddHostCommand::_getClusterInstallPath( string clusterName, 
                                                   string &installPath )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder resultBuilder ;
      BSONObj result ;

      BSONObjBuilder builder ;
      builder.append( OM_CLUSTER_FIELD_NAME, clusterName ) ;
      BSONObj matcher ;
      matcher = builder.obj() ;

      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID             = -1 ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "failed to query table:" ) 
                        + OM_CS_DEPLOY_CL_CLUSTER ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               _errorDetail = string( "cluster is not exist:cluster=" ) 
                              + clusterName ;
               PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
               goto error ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_CLUSTER ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj record( buffObj.data() ) ;
         installPath = record.getStringField( OM_CLUSTER_FIELD_INSTALLPATH ) ;
      }

   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omAddHostCommand::_getSdbUsrInfo( string clusterName, string &sdbUser, 
                                           string &sdbPasswd, 
                                           string &sdbUserGroup )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder resultBuilder ;
      BSONObj result ;

      BSONObjBuilder builder ;
      builder.append( OM_CLUSTER_FIELD_NAME, clusterName ) ;
      BSONObj matcher ;
      matcher = builder.obj() ;

      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID             = -1 ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "failed to query table:" ) 
                        + OM_CS_DEPLOY_CL_CLUSTER ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               _errorDetail = string( "cluster is not exist:cluster=" ) 
                              + clusterName ;
               PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
               goto error ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_CLUSTER ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj record( buffObj.data() ) ;
         sdbUser      = record.getStringField( OM_CLUSTER_FIELD_SDBUSER ) ;
         sdbPasswd    = record.getStringField( OM_CLUSTER_FIELD_SDBPASSWD ) ;
         sdbUserGroup = record.getStringField( OM_CLUSTER_FIELD_SDBUSERGROUP ) ;
      }

   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omAddHostCommand::_getPacketFullPath( char *path )
   {
      INT32 rc = SDB_OK ;
      CHAR tmpPath[ OSS_MAX_PATHSIZE + 1 ] = "" ;
      ossGetEWD( tmpPath, OSS_MAX_PATHSIZE ) ;
      utilCatPath( tmpPath, OSS_MAX_PATHSIZE, ".." ) ;
      utilCatPath( tmpPath, OSS_MAX_PATHSIZE, OM_PACKET_SUBPATH ) ;

      map< string, string> mapFiles ;      
      rc = ossEnumFiles( tmpPath, mapFiles ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "path is invalid:path=" ) + tmpPath ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      if ( mapFiles.size() != 1 )
      {
         rc = SDB_FNE ;
         PD_LOG_MSG( PDERROR, "path is invalid:path=%s,fileCount=%d", tmpPath, 
                     mapFiles.size() ) ;
         _errorDetail = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

      ossSnprintf( path, OSS_MAX_PATHSIZE, "%s", 
                   mapFiles.begin()->second.c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omAddHostCommand::_addHost( string clusterName, 
                                     list<BSONObj> &hostInfoList, 
                                     INT32 &transationID )
   {
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      MsgHeader *pMsg   = NULL ;
      omManager *om     = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonRequest ;
      BSONObj result ;
      BSONElement element ;
      BSONElement hostElement ;

      rc = _generateAddHostReq( clusterName, hostInfoList, bsonRequest ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_generateAddHostReq failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_ADD_HOST_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "build msg failed:cmd=" ) + OM_ADD_HOST_REQ ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      // create remote session
      om            = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_WAIT_SCAN_RES_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         _errorDetail = "create remote session failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "send message to agent failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      // receiving for agent's response
      rc = _receiveFromAgent( remoteSession, result ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "receive from agent failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rc = result.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = result.getStringField( OM_REST_RES_DETAIL ) ;
         PD_LOG( PDERROR, "agent process %s failed:detail=%s,rc=%d", 
                 OM_BASIC_CHECK_REQ, _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      element = result.getField( OM_BSON_FIELD_TRANSACTION_ID ) ;
      if ( element.eoo() || NumberInt != element.type())
      {
         rc = SDB_UNEXPECTED_RESULT ;
         _errorDetail = "agent's response is unrecognized" ;
         PD_LOG( PDERROR, "%s:type=%d", _errorDetail.c_str(), element.type() ) ;
         goto error ;
      }

      transationID = result.getIntField( OM_REST_RES_RETCODE ) ;

   done:
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   void omAddHostCommand::_generateTableField( BSONObjBuilder &builder, 
                                               string newFieldName,
                                               BSONObj &bsonOld,
                                               string oldFiledName ) 
   {  
      BSONElement element = bsonOld.getField( oldFiledName ) ;
      if ( !element.eoo() )
      {
         builder.appendAs( element, newFieldName ) ;
      }
   }

   INT32 omAddHostCommand::_storeHostInfo( string clusterName, 
                                           list<BSONObj> &hostInfoList )
   {
      INT32 rc = SDB_OK ;
      list<BSONObj>::iterator iteRollBack ;
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         BSONObjBuilder builder ;
         BSONObj tmp ;

         // TODO OM_HOST_FIELD_TIME
         _generateTableField( builder, OM_HOST_FIELD_NAME, *ite, 
                              OM_BSON_FIELD_HOST_NAME ) ;
         builder.append( OM_HOST_FIELD_CLUSTERNAME, clusterName ) ;
         _generateTableField( builder, OM_HOST_FIELD_IP, *ite, 
                              OM_BSON_FIELD_HOST_IP ) ;
         _generateTableField( builder, OM_HOST_FIELD_USER, *ite, 
                              OM_BSON_FIELD_HOST_USER ) ;
         _generateTableField( builder, OM_HOST_FIELD_PASSWORD, *ite, 
                              OM_BSON_FIELD_HOST_PASSWD ) ;
         _generateTableField( builder, OM_HOST_FIELD_OS, *ite, 
                              OM_BSON_FIELD_OS ) ;
         _generateTableField( builder, OM_HOST_FIELD_OM, *ite, 
                              OM_BSON_FIELD_OM ) ;
         _generateTableField( builder, OM_HOST_FIELD_MEMORY, *ite, 
                              OM_BSON_FIELD_MEMORY ) ;   
         _generateTableField( builder, OM_HOST_FIELD_DISK, *ite, 
                              OM_BSON_FIELD_DISK ) ;
         _generateTableField( builder, OM_HOST_FIELD_CPU, *ite, 
                              OM_BSON_FIELD_CPU ) ;
         _generateTableField( builder, OM_HOST_FIELD_NET, *ite, 
                              OM_BSON_FIELD_NET ) ;
         _generateTableField( builder, OM_HOST_FIELD_PORT, *ite, 
                              OM_BSON_FIELD_PORT ) ;
         _generateTableField( builder, OM_HOST_FIELD_SERVICE, *ite, 
                              OM_BSON_FIELD_SERVICE ) ;
         _generateTableField( builder, OM_HOST_FIELD_SAFETY, *ite, 
                              OM_BSON_FIELD_SAFETY ) ;
         _generateTableField( builder, OM_HOST_FIELD_INSTALLPATH, *ite, 
                              OM_BSON_FIELD_INSTALL_PATH ) ;
         _generateTableField( builder, OM_HOST_FIELD_AGENT_PORT, *ite, 
                              OM_BSON_FIELD_AGENT_PORT ) ;

         tmp = builder.obj() ;
         rc = rtnInsert( OM_CS_DEPLOY_CL_HOST, tmp, 1, 0, _cb ) ;
         {
            if ( rc )
            {
               _errorDetail = string("failed to store host's info into table:") 
                              + OM_CS_DEPLOY_CL_HOST ;
               PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
               goto error ;
            }
         }

         ite++ ;
      }

   done:
      return rc ;
   error:
      iteRollBack = hostInfoList.begin() ;
      while ( iteRollBack != ite )
      {
         BSONObjBuilder builder ;
         BSONObj tmp ;
         _generateTableField( builder, OM_HOST_FIELD_NAME, *iteRollBack, 
                              OM_BSON_FIELD_HOST_NAME ) ;
         tmp = builder.obj() ;
         rtnDelete( OM_CS_DEPLOY_CL_HOST, tmp, BSONObj(), 0, _cb ) ;
         iteRollBack++ ;
      }
      goto done ;
   }

   void omAddHostCommand::_transactionRollBack( string host, string service, 
                                                INT32 transactionID )
   {
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      MsgHeader *pMsg   = NULL ;
      omManager *om     = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonRequest ;
      BSONObjBuilder builder ;

      builder.append( OM_BSON_FIELD_TRANSACTION_ID, transactionID ) ;
      bsonRequest = builder.obj() ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_ROLLBACK_TRANSACTION_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "build query msg failed:rc=%d", rc ) ;
         goto error ;
      }

      // create remote session
      om            = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_WAIT_SCAN_RES_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      remoteSession->waitReply( TRUE ) ;

   done:
      _clearSession( om, remoteSession ) ;
      return ;
   error:
      goto done ;
   }

   INT32 omAddHostCommand::_checkHostExistence( list<BSONObj> &hostInfoList )
   {
      INT32 rc = SDB_OK ;
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         if ( _isHostExist( *ite ) )
         {
            rc = SDB_INVALIDARG ;
            string host  = ite->getStringField( OM_BSON_FIELD_HOST_NAME ) ;
            _errorDetail = string("host is exist:host=") + host ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         ite++ ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omAddHostCommand::doCommand()
   {
      list<BSONObj> hostInfoList ;
      string clusterName ;
      INT32 transactionID = -1 ;
      INT32 rc = SDB_OK ;
      BSONObjBuilder bsonBuilder ;

      rc = _getHostList( clusterName, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "fail to get host list:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _checkHostExistence( hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "fail to _checkHostExistence:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _addHost( clusterName, hostInfoList, transactionID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "fail to get host list:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _storeHostInfo( clusterName, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "fail to store host:rc=%d", rc ) ;
         _transactionRollBack( _localAgentHost, _localAgentService, 
                               transactionID ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      sdbGetOMManager()->updateClusterVersion( clusterName ) ;
      bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omQueryHostCommand *****************************
   omQueryHostCommand::omQueryHostCommand( restAdaptor *pRestAdaptor, 
                                           pmdRestSession *pRestSession )
                      : omCreateClusterCommand( pRestAdaptor, pRestSession )
   {
   }

   omQueryHostCommand::~omQueryHostCommand()
   {  
   }

   INT32 omQueryHostCommand::_queryHostInfoByCluster( string cluster, 
                                                      list<BSONObj> &hosts )
   {
      INT32 rc = SDB_OK ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID             = -1 ;
      
      matcher = BSON( OM_HOST_FIELD_CLUSTERNAME << cluster ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_HOST ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_HOST ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj record( buffObj.data() ) ;
         hosts.push_back( record.copy() ) ;
      }
   done:
      return rc ;
   error:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      goto done ;
   }

   INT32 omQueryHostCommand::_queryHostInfoByHost( string hostName, 
                                                   list<BSONObj> &hosts )
   {
      INT32 rc = SDB_OK ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID             = -1 ;
      
      matcher = BSON( OM_HOST_FIELD_NAME << hostName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_HOST ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_HOST ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj record( buffObj.data() ) ;
         hosts.push_back( record.copy() ) ;
      }
   done:
      return rc ;
   error:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      goto done ;
   }

   void omQueryHostCommand::_sendHostInfo2Web( list<BSONObj> &hosts )
   {
      BSONObjBuilder opBuilder ;
      list<BSONObj>::iterator iter = hosts.begin() ;
      while ( iter != hosts.end() )
      {
         _restAdaptor->appendHttpBody( _restSession, (*iter).objdata(), 
                                       (*iter).objsize(), 1 ) ;
         iter++ ;
      }

      opBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, opBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

      return ;
   }

   INT32 omQueryHostCommand::doCommand()
   {
      INT32 rc                 = SDB_OK ;
      const CHAR *pClusterName = NULL ;
      const CHAR *pHostName    = NULL ;
      list<BSONObj> hosts ;

      _restAdaptor->getQuery(_restSession, OM_REST_CLUSTER_NAME, 
                             &pClusterName ) ;
      _restAdaptor->getQuery(_restSession, OM_REST_HOST_NAME, 
                             &pHostName ) ;
      if ( ( NULL == pClusterName ) && ( NULL == pHostName ) )
      {
         _errorDetail = "rest field:" + string( OM_REST_CLUSTER_NAME ) + " and "
                        OM_REST_HOST_NAME + " must exist one field" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      if( NULL == pHostName )
      {
         rc = _queryHostInfoByCluster( pClusterName, hosts ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "_queryHostInfoByCluster failed:rc=%d", rc ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
            goto error ;
         }
      }
      else
      {
         rc = _queryHostInfoByHost( pHostName, hosts ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "_queryHostInfoByHost failed:rc=%d", rc ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
            goto error ;
         }
      }

      _sendHostInfo2Web( hosts ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omQueryBusinessTypeCommand *****************************
   omQueryBusinessTypeCommand::omQueryBusinessTypeCommand( 
                                                restAdaptor *pRestAdaptor, 
                                                pmdRestSession *pRestSession, 
                                                const CHAR *pRootPath, 
                                                const CHAR *pSubPath )
                               :omCreateClusterCommand( pRestAdaptor, 
                                                        pRestSession ),
                                _rootPath( pRootPath ), _subPath( pSubPath )
   {
   }

   omQueryBusinessTypeCommand::~omQueryBusinessTypeCommand()
   {
   }

   BOOLEAN omQueryBusinessTypeCommand::_isArray( ptree &pt )
   {
      BOOLEAN isArr = FALSE ;
      string type ;
      try
      {
         type = pt.get<string>( OM_XMLATTR_TYPE ) ;
      }
      catch( std::exception &e )
      {
         isArr = FALSE ;
         goto done ;
      }

      if ( ossStrcasecmp( type.c_str(), OM_XMLATTR_TYPE_ARRAY ) == 0 )
      {
         isArr = TRUE ;
         goto done ;
      }

   done:
      return isArr ;
   }

   BOOLEAN omQueryBusinessTypeCommand::_isStringValue( ptree &pt )
   {
      BOOLEAN isStringV = FALSE ;
      if ( _isArray( pt ) )
      {
         isStringV = FALSE ;
         goto done ;
      }

      if ( pt.size() == 0 )
      {
         isStringV = TRUE ;
         goto done ;
      }

      if ( pt.size() > 1 )
      {
         isStringV = FALSE ;
         goto done ;
      }

      // in this case pt.size() == 1
      {
         ptree::iterator ite = pt.begin() ;
         string key          = ite->first ;
         if ( ossStrcasecmp( key.c_str(), OM_XMLATTR_KEY ) == 0 )
         {
            isStringV = TRUE ;
            goto done ;
         }
      }

   done:
      return isStringV ;
   }

   void omQueryBusinessTypeCommand::_parseArray( ptree &pt, 
                                            BSONArrayBuilder &arrayBuilder )
   {
      ptree::iterator ite = pt.begin() ;
      for( ; ite != pt.end() ; ite++ )
      {
         string key    = ite->first ;
         if ( ossStrcasecmp( key.c_str(), OM_XMLATTR_KEY ) == 0 )
         {
            continue ;
         }

         BSONObj obj ;
         ptree child = ite->second ;
         _recurseParseObj( child, obj ) ;
         arrayBuilder.append( obj ) ;
      }
   }

   void omQueryBusinessTypeCommand::_recurseParseObj( ptree &pt, BSONObj &out )
   {
      BSONObjBuilder builder ;
      ptree::iterator ite = pt.begin() ;
      for( ; ite != pt.end() ; ite++ )
      {
         string key = ite->first ;
         if ( ossStrcasecmp( key.c_str(), OM_XMLATTR_KEY ) == 0 )
         {
            continue ;
         }

         ptree child = ite->second ;
         if ( _isArray( child ) )
         {
            BSONArrayBuilder arrayBuilder ;
            _parseArray( child, arrayBuilder ) ;
            builder.append( key, arrayBuilder.arr() ) ;
         }
         else if ( _isStringValue( child ) )
         {
            string value = ite->second.data() ;
            builder.append( key, value ) ;
         }
         else
         {
            // obj
            BSONObj obj ;
            _recurseParseObj( child, obj ) ;
            builder.append(key, obj ) ;
         }
      }

      out = builder.obj() ;
   }

   INT32 omQueryBusinessTypeCommand::_readConfigFile( string file, BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      try
      {
         ptree pt ;
         read_xml( file.c_str(), pt ) ;
         _recurseParseObj( pt, obj ) ;
      }
      catch( std::exception &e )
      {
         rc = SDB_INVALIDPATH ;
         _errorDetail = string( "parse file failed:file=" ) + file + ",err=" 
                        + e.what() ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omQueryBusinessTypeCommand::doCommand()
   {
      INT32 rc             = SDB_OK ;
      string businessFile  = _rootPath + OSS_FILE_SEP 
                             + OM_BUSINESS_CONFIG_SUBDIR
                             + OSS_FILE_SEP + OM_BUSINESS_FILE_NAME ;
      BSONObjBuilder opBuilder ;
      BSONObj bsonBusiness ;
      rc = _readConfigFile( businessFile, bsonBusiness ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read business file failed:file=%s", 
                 businessFile.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _restAdaptor->appendHttpBody( _restSession, bsonBusiness.objdata(), 
                                    bsonBusiness.objsize(), 1 ) ;
      opBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, opBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omQueryBusinessTemplateCommand *****************************
   omQueryBusinessTemplateCommand::omQueryBusinessTemplateCommand( 
                                                   restAdaptor *pRestAdaptor, 
                                                   pmdRestSession *pRestSession, 
                                                   const CHAR *pRootPath, 
                                                   const CHAR *pSubPath )
                                  :omQueryBusinessTypeCommand( pRestAdaptor, 
                                                               pRestSession, 
                                                               pRootPath,
                                                               pSubPath )
   {
   }

   omQueryBusinessTemplateCommand::~omQueryBusinessTemplateCommand()
   {
   }

   INT32 omQueryBusinessTemplateCommand::_readConfTemplate( 
                                                string businessType, 
                                                string file, 
                                                list<BSONObj> &deployModList ) 
   {
      INT32 rc = SDB_OK ;
      BSONObj deployModArray ;
      BSONObj deployMods ;
      rc = _readConfigFile( file, deployModArray ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read file failed:file=%s", file.c_str() ) ;
         goto error ;
      }

      deployMods = deployModArray.getObjectField( OM_BSON_DEPLOY_MOD_LIST ) ;
      {
         BSONObjIterator iter( deployMods ) ;
         while ( iter.more() )
         {
            BSONElement ele      = iter.next() ;
            BSONObj oneDeployMod = ele.embeddedObject() ;
            deployModList.push_back( oneDeployMod.copy()) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omQueryBusinessTemplateCommand::_readConfDetail( string file, 
                                                       BSONObj &bsonConfDetail )
   {
      INT32 rc = SDB_OK ;
      rc = _readConfigFile( file, bsonConfDetail ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read file failed:file=%s", file.c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omQueryBusinessTemplateCommand::doCommand()
   {
      INT32 rc                  = SDB_OK ;
      const CHAR* pBusinessType = NULL ;
      string templateFile       = "" ;
      BSONObjBuilder opBuilder ;
      list<BSONObj> deployModList ;
      list<BSONObj>::iterator iter ;

      _restAdaptor->getQuery(_restSession, OM_REST_BUSINESS_TYPE, 
                             &pBusinessType ) ;
      if ( NULL == pBusinessType )
      {
         _errorDetail = string( "rest field:" ) + OM_REST_BUSINESS_TYPE
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      templateFile = _rootPath + OSS_FILE_SEP + OM_BUSINESS_CONFIG_SUBDIR 
                     + OSS_FILE_SEP + pBusinessType + OM_TEMPLATE_FILE_NAME ;
      rc = _readConfTemplate( pBusinessType, templateFile, deployModList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read template file failed:file=%s:rc=%d", 
                 templateFile.c_str(), rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      iter = deployModList.begin() ;
      while ( iter != deployModList.end() )
      {
         _restAdaptor->appendHttpBody( _restSession, iter->objdata(),
                                       iter->objsize(), 1 ) ;
         iter++ ;
      }

      opBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, opBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // *********************omConfigBusinessCommand**************************
   omConfigBusinessCommand::omConfigBusinessCommand(  
                                                  restAdaptor *pRestAdaptor, 
                                                  pmdRestSession *pRestSession, 
                                                  const CHAR *pRootPath, 
                                                  const CHAR *pSubPath )
                           :omQueryBusinessTemplateCommand( pRestAdaptor, 
                                                            pRestSession, 
                                                            pRootPath,
                                                            pSubPath)
   {
   }

   omConfigBusinessCommand::~omConfigBusinessCommand()
   {
   }

   INT32 omConfigBusinessCommand::_getPropertyNameValue( BSONObj &bsonTemplate, 
                                                         string propertyName, 
                                                         string &value )
   {
      INT32 rc = SDB_OK ;
      BSONObj properties ;
      properties = bsonTemplate.getObjectField( OM_BSON_PROPERTY_ARRAY ) ;
      {
         BSONObjIterator iter( properties ) ;
         while ( iter.more() )
         {
            BSONElement ele     = iter.next() ;
            /*{ 
                 "Name": "replica_num", "Value": "2" 
              }*/
            BSONObj oneProperty = ele.embeddedObject() ;
            string name = oneProperty.getStringField( OM_BSON_PROPERTY_NAME ) ;
            if ( propertyName.compare( name ) == 0 )
            {
               if ( !oneProperty.hasField( OM_BSON_PROPERTY_VALUE ) )
               {
                  rc = SDB_INVALIDARG ;
                  PD_LOG( PDERROR, "propery have not value:name=%s", 
                          name.c_str() ) ;
                  goto error ;
               }

               value = oneProperty.getStringField( OM_BSON_PROPERTY_VALUE ) ;
               goto done ;
            }
         }
      }

      rc = SDB_INVALIDARG ;
      PD_LOG( PDERROR, "propery is not found:name=%s", propertyName.c_str() ) ;
      goto error ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigBusinessCommand::_fillTemplateInfo( BSONObj &bsonTemplate )
   {
      INT32 rc = SDB_OK ;
      string businessType ;
      string deployMod ;
      string businessName ;
      string file ;
      list<BSONObj> deployModList ;
      list<BSONObj>::iterator iterList ;
      BSONObj oneDeployMod ;
      BSONObjBuilder builder ;
      BSONArrayBuilder arrayBuilder ;
      BSONObj properties ;

      businessType = bsonTemplate.getStringField( OM_BSON_BUSINESS_TYPE ) ;
      deployMod    = bsonTemplate.getStringField( OM_BSON_DEPLOY_MOD ) ;
      businessName = bsonTemplate.getStringField( OM_BSON_BUSINESS_NAME ) ;

      file = _rootPath + OSS_FILE_SEP + OM_BUSINESS_CONFIG_SUBDIR 
             + OSS_FILE_SEP + businessType + OM_TEMPLATE_FILE_NAME ;
      rc = _readConfTemplate( businessType, file, deployModList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read template file failed:file=%s:rc=%d", 
                 file.c_str(), rc ) ;
         goto error ;
      }

      iterList = deployModList.begin() ;
      while ( iterList != deployModList.end() )
      {
         string tmpClusterType = iterList->getStringField(
                                                          OM_BSON_DEPLOY_MOD ) ;
         if ( tmpClusterType.compare( deployMod ) == 0 )
         {
            oneDeployMod = *iterList ;
            break ;
         }
         iterList++ ;
      }

      if ( iterList == deployModList.end() )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_DEPLOY_MOD ) + " is not exist:mode=" 
                        + deployMod ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      builder.append( OM_BSON_BUSINESS_TYPE, businessType ) ;
      builder.append( OM_BSON_DEPLOY_MOD, deployMod ) ;
      builder.append( OM_BSON_BUSINESS_NAME, businessName ) ;
      /*{
           "Property": [ { "Name": "replica_num", "Type": "int", "Default": "1", 
                           "Valid": "1", "Display": "edit box", "Edit": "false", 
                           "Desc": "", "WebName": "" 
                         }, ...
                       ]
        }*/
      properties = oneDeployMod.getObjectField( OM_BSON_PROPERTY_ARRAY ) ;
      {
         BSONObjIterator iter( properties ) ;
         while ( iter.more() )
         {
            BSONObjBuilder propertyBuilder ;
            BSONObj tmp ;
            BSONElement ele     = iter.next() ;
            BSONObj oneProperty = ele.embeddedObject() ;
            string propertyName ;
            string value ;
            /*{ 
                 "Name": "replica_num", "Type": "int", "Default": "1", 
                 "Valid": "1", "Display": "edit box", "Edit": "false", 
                 "Desc": "", "WebName": "" 
              }*/
            propertyBuilder.appendElements( oneProperty ) ;
            propertyName = oneProperty.getStringField( OM_BSON_PROPERTY_NAME) ;
            rc = _getPropertyNameValue( bsonTemplate, propertyName, value ) ;
            if ( SDB_OK != rc )
            {
               _errorDetail = string( "template miss property:" ) 
                              + propertyName ;
               PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
               goto error ;
            }
            propertyBuilder.append( OM_BSON_PROPERTY_VALUE, value ) ;
            tmp = propertyBuilder.obj() ;
            arrayBuilder.append( tmp ) ;
         }
      }

      builder.append( OM_BSON_PROPERTY_ARRAY, arrayBuilder.arr() ) ;
      bsonTemplate = builder.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigBusinessCommand::_getHostConfig( string hostName, 
                                                  string businessName,
                                                  BSONObj &config )
   {
      BSONObjBuilder confBuilder ;
      BSONArrayBuilder arrayBuilder ;
      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID        = -1 ;
      INT32 rc                = SDB_OK ;

      BSONObjBuilder condBuilder ;
      condBuilder.append( OM_CONFIGURE_FIELD_HOSTNAME, hostName ) ;
      //condBuilder.append( OM_CONFIGURE_FIELD_BUSINESSNAME, businessName ) ;
      // condBuilder.append( OM_CONFIGURE_FIELD_BUSINESSNAME, businessName ) ;
      BSONObj condition = condBuilder.obj() ;

      // query table
      rc = rtnQuery( OM_CS_DEPLOY_CL_CONFIGURE, selector, condition, order, 
                     hint, 0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_CONFIGURE ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         BSONObj tmpConf ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_CONFIGURE ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         /*
            {
               "HostName":"h1", "BusinessName":"b1", 
               "Config":
               [{"dbpath":"","svcname":"11810", ...}
                  , ...
               ]
            }
         */
         BSONObj result( buffObj.data() ) ;
         businessName = result.getStringField( 
                                             OM_CONFIGURE_FIELD_BUSINESSNAME ) ;
         tmpConf = result.getObjectField( OM_CONFIGURE_FIELD_CONFIG ) ;
         {
            BSONObjIterator iter( tmpConf ) ;
            while ( iter.more() )
            {  
               BSONObjBuilder innerBuilder ;
               BSONElement ele     = iter.next() ;

               innerBuilder.appendElements( ele.embeddedObject() ) ;
               innerBuilder.append( OM_BSON_BUSINESS_NAME, businessName ) ;
               arrayBuilder.append( innerBuilder.obj() ) ;
            }
         }
      }

      /*
         {
            "Config":
            [{"BusinessName":"b1","dbpath":"","svcname":"11810", ...}
               , ...
            ]
         }
      */
      confBuilder.append( OM_BSON_FIELD_CONFIG, arrayBuilder.arr() ) ;
      config = confBuilder.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigBusinessCommand::_checkBusiness( string businessName, 
                                                  const string &businessType,
                                                  const string &deployMod,
                                                  const string &clusterName )
   {
      INT32 rc = SDB_OK ;
      string existType ;
      string existDeployMod ;
      string existClusterName ;

      rc = _getExistBusiness( businessName, existType, existDeployMod, 
                              existClusterName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get exist business failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( existType == "" )
      {
         // no record
         goto done ;
      }

      if ( businessType != existType || deployMod != existDeployMod 
           || clusterName != existClusterName )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( "business conflict with same name:exist=[" ) 
                        + businessName + "," + existType + "," 
                        + existDeployMod + "," + existClusterName + "] new=["
                        + businessName + "," + businessType + "," 
                        + deployMod + "," + clusterName + "]" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }
   INT32 omConfigBusinessCommand::_getExistBusiness( const string &businessName, 
                                                     string &businessType,
                                                     string &deployMod,
                                                     string &clusterName )
   {
      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID  = -1 ;
      INT32 rc          = SDB_OK ;
      BSONObj condition = BSON( OM_BUSINESS_FIELD_NAME << businessName )  ;

      businessType = "" ;
      deployMod    = "" ;
      clusterName  = "" ;

      // query table
      rc = rtnQuery( OM_CS_DEPLOY_CL_BUSINESS, selector, condition, order, 
                     hint, 0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_BUSINESS ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         BSONObj tmpConf ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_BUSINESS ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         businessType = result.getStringField( OM_BUSINESS_FIELD_TYPE );
         deployMod    = result.getStringField( OM_BUSINESS_FIELD_DEPLOYMOD );
         clusterName  = result.getStringField( OM_BUSINESS_FIELD_CLUSTERNAME );
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*input parameter: 
     bsonHostInfo
     {
        "HostInfo":[{"HostName":"host1"}, ...]
     }

     output parameter:
     bsonHostInfo
     {
        "HostInfo":[{"HostName":"host1", "ClusterName":"c1", 
                     "Disk":[{"Name":"dev", "Mount":"/mnt", ... }, ...],
                     "Config":[{"dbpath":"","svcname":"11810", ...}, ...]
                    , ...
                   ]
     }

     */
   INT32 omConfigBusinessCommand::_fillHostInfo( string clusterName, 
                                                 string businessName,
                                                 BSONObj &bsonHostInfo )
   {
      BSONObj condition ;
      BSONObj result ;
      BSONArrayBuilder ArrBuilder ;
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      BOOLEAN isRecordFetched = false ;
      SINT64 contextID        = -1 ;
      INT32 rc                = SDB_OK ;

      BSONObjBuilder condBuilder ;
      condBuilder.append( OM_HOST_FIELD_CLUSTERNAME, clusterName ) ;
      if ( !bsonHostInfo.isEmpty() )
      {
         BSONArrayBuilder innArrBuilder ;
         BSONObj hosts ;
         hosts = bsonHostInfo.getObjectField( OM_BSON_FIELD_HOST_INFO ) ;
         {
            BSONObjIterator iter( hosts ) ;
            while ( iter.more() )
            {
               string hostName ;
               BSONObjBuilder builder ;
               BSONObj oneHost ;
               BSONObj tmp ;
               BSONElement ele ;

               ele      = iter.next() ;
               // {"HostName":"host1", ...}
               oneHost  = ele.embeddedObject() ;
               hostName = oneHost.getStringField( OM_BSON_FIELD_HOST_NAME ) ;
               builder.append( OM_HOST_FIELD_NAME, hostName ) ;
               tmp      = builder.obj() ;
               innArrBuilder.append( tmp ) ;
             }
          }

          condBuilder.append( "$or", innArrBuilder.arr() ) ;
      }

      condition = condBuilder.obj() ;
      // query table
      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, condition, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_HOST ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         BSONObjBuilder resultBuilder ;
         BSONObj config ;
         string hostName ;
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_HOST ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         /*
            result= {"HostName":"host1", "ClusterName":"c1", 
                     "Disk":[{"Name":"dev", "Mount":"/mnt", ... }, ...]
                     , ...  }
         */
         BSONObj result( buffObj.data() ) ;
         resultBuilder.appendElements( result ) ;
         hostName = result.getStringField( OM_HOST_FIELD_NAME ) ;
         rc       = _getHostConfig( hostName, businessName, config ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to get host config, rc=%d", rc ) ;
            goto error ;
         }

         resultBuilder.appendElements( config ) ;

         /*
            result= {"HostName":"host1", "ClusterName":"c1", 
                     "Disk":[{"Name":"dev", "Mount":"/mnt", ... }, ...],
                     "Config":[{"BusinessName":"b1","dbpath":"",
                                "svcname":"11810", ... }, ...]
                     , ...  }
         */
         result = resultBuilder.obj() ;
         ArrBuilder.append( result ) ;
         isRecordFetched = TRUE ;
      }

      if ( !isRecordFetched )
      {
         rc = SDB_DMS_RECORD_NOTEXIST ;
         _errorDetail = string( "host is not exist:host=" ) 
                        + condition.toString() ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      bsonBuilder.append( OM_BSON_FIELD_HOST_INFO, ArrBuilder.arr() ) ;
      bsonHostInfo = bsonBuilder.obj() ;

   done:
      return rc ;
   error:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      goto done ;
   }

   /*
     bsonTemplate(out)
       {
           "ClusterName":"c1","BusinessType":"sequoiadb", "BusinessName":"b1",
           "DeployMod": "standalone", 
           "Property": [ { "Name": "replica_num", "Type": "int", "Default": "1", 
                           "Valid": "1", "Display": "edit box", "Edit": "false", 
                           "Desc": "", "WebName": "" }
                           , ...
                       ] 
        }

     bsonHostInfo(out):    
        {
           "HostInfo":[{"HostName":"host1", "ClusterName":"c1", 
                        "Disk":[{"Name":"dev", "Mount":"/mnt", ... }, ...],
                        "Config":[{"BusinessName":"b1","dbpath":"","svcname":"11810", ...}, ...]
                       }
                       , ...
                      ]
        }*/
   INT32 omConfigBusinessCommand::_getTemplateInfo( BSONObj &bsonTemplate, 
                                                    BSONObj &bsonHostInfo )
   {
      INT32 rc          = SDB_OK ;
      const CHAR *pInfo = NULL ;
      BSONObj bsonInfo ;
      BSONObjBuilder templateBuilder ;
      BSONObj filter ;
      BSONObjBuilder hostInfoBuilder ;
      _restAdaptor->getQuery( _restSession, OM_REST_TEMPLATE_INFO, 
                             &pInfo ) ;
      if ( NULL == pInfo )
      {
         _errorDetail = "rest field:" + string( OM_REST_TEMPLATE_INFO ) 
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pInfo, bsonInfo ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "change rest field " + string( OM_REST_TEMPLATE_INFO )
                        + " to BSONObj failed" ;
         PD_LOG( PDERROR, "%s:rc=%d,src=%s", _errorDetail.c_str(), rc, pInfo ) ;
         goto error ;
      }

      filter       = BSON( OM_BSON_FIELD_HOST_INFO << "" ) ;
      /*{
           "ClusterName":"c1","BusinessType":"sequoiadb", "BusinessName":"b1",
           "ClusterType": "standalone", 
           "Property": [ { "Name": "replica_num", "Value":"" }, ...] 
        } */
      bsonTemplate = bsonInfo.filterFieldsUndotted( filter, false ) ;

      _clusterName = bsonTemplate.getStringField( OM_BSON_FIELD_CLUSTER_NAME ) ;
      if ( _clusterName == "" )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_FIELD_CLUSTER_NAME ) + " is empty" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      _businessType = bsonTemplate.getStringField( OM_BSON_BUSINESS_TYPE ) ;
      if ( _businessType == "" )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_BUSINESS_TYPE ) + " is empty" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      _businessName = bsonTemplate.getStringField( OM_BSON_BUSINESS_NAME ) ;
      if ( _businessName == "" )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_BUSINESS_NAME ) + " is empty" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      _deployMod = bsonTemplate.getStringField( OM_BSON_DEPLOY_MOD ) ;
      if ( _deployMod == "" )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_DEPLOY_MOD ) + " is empty" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rc = _fillTemplateInfo( bsonTemplate ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_fillTemplateInfo failed:rc=%d", rc ) ;
         goto error ;
      }
      // after fullFillTemplate, bsonTemplate add more details:
      /*{
           "ClusterName":"c1","BusinessType":"sequoiadb", "BusinessName":"b1",
           "ClusterType": "standalone", 
           "Property": [ { "Name": "replica_num", "Type": "int", "Default": "1", 
                           "Valid": "1", "Display": "edit box", "Edit": "false", 
                           "Desc": "", "WebName": "" }
                           , ...
                       ] 
        } */

      /*{
           "HostInfo":[{"HostName":"host1"}, ...]}
        }*/
      bsonHostInfo = bsonInfo.filterFieldsUndotted( filter, true ) ;
      /*{
           "HostInfo":[{"HostName":"host1", "ClusterName":"c1", 
                        "Disk":[{"Name":"dev", "Mount":"/mnt", ... }, ...],
                        "Config":[{"BusinessName":"b1","dbpath":"","svcname":"11810", ...}, ...]
                       }
                       , ...
                      ]
        }*/
      rc = _fillHostInfo( _clusterName, _businessName, bsonHostInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_fillHostInfo failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
   bsonConfDetail:
   {
      "Property":[{"Name":"dbpath", "Type":"path", "Default":"/opt/sequoiadb", 
                      "Valid":"1", "Display":"edit box", "Edit":"false", 
                      "Desc":"", "WebName":"" }
                      , ...
                 ] 
   }
   */
   INT32 omConfigBusinessCommand::_getConfigDetail( const BSONObj &bsonTemplate, 
                                                  BSONObj &bsonConfDetail )
   {
      INT32 rc = SDB_OK ;
      string confDetailFile = "" ;
      string businessType ;

      businessType = bsonTemplate.getStringField( OM_REST_BUSINESS_TYPE ) ;
      if ( businessType.length() == 0 )
      {
         _errorDetail = string( OM_REST_BUSINESS_TYPE ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }
      confDetailFile = _rootPath + OSS_FILE_SEP + OM_BUSINESS_CONFIG_SUBDIR 
                       + OSS_FILE_SEP + businessType 
                       + OM_CONFIG_ITEM_FILE_NAME ;

      rc = _readConfDetail(confDetailFile, bsonConfDetail ) ;
      if( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read configure failed:file=%s", 
                 confDetailFile.c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void omConfigBusinessCommand::_addProperties( BSONObjBuilder &builder, 
                                                 const BSONObj &bsonTemplate, 
                                                 const BSONObj &bsonConfDetail )
   {
      BSONObjBuilder valueCondBuilder ;
      BSONObj valueCondition ;
      valueCondBuilder.append( OM_BSON_PROPERTY_VALUE, "" ) ;
      valueCondition = valueCondBuilder.obj() ;

      BSONArrayBuilder arrayBuilder ;
      BSONObj tmp ;
      tmp = bsonConfDetail.getObjectField( OM_BSON_PROPERTY_ARRAY ) ;
      BSONObjIterator iter2( tmp ) ;
      while ( iter2.more() )
      {
         BSONElement ele ;
         BSONObj oneProperty ;
         string propertyName ;

         ele          = iter2.next() ;
         oneProperty  = ele.embeddedObject() ;
         arrayBuilder.append( oneProperty ) ;
      }

      builder.append( OM_BSON_PROPERTY_ARRAY, arrayBuilder.arr() ) ;
   }

   INT32 omConfigBusinessCommand::_generateConfig( 
                                                  const BSONObj &bsonTemplate, 
                                                  const BSONObj &bsonHostInfo, 
                                                  const BSONObj &bsonConfigItem, 
                                                  BSONObj &bsonConfig )
   {
      INT32 rc = SDB_OK ;
      omConfigGenerator confGenerator ;
      BSONObjBuilder builder ;
      BSONObj tmpConf ;
      /* tmpConf:
         {
            "Config":
            [
              {"HostName":"host1","DataGroupName":"",
               "DBPath": "/home/db2/coord/11830", "SvcName": "11830", ... 
              }, ...
            ]
         }
      */
      rc = confGenerator.generateSDBConfig( bsonTemplate, bsonConfigItem, 
                                            bsonHostInfo, tmpConf ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = confGenerator.getErrorDetail() ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      builder.appendElements( tmpConf ) ;
      builder.append( OM_BSON_FIELD_CLUSTER_NAME, _clusterName ) ;
      _addProperties( builder, bsonTemplate, bsonConfigItem ) ;
      bsonConfig = builder.obj() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigBusinessCommand::doCommand()
   {
      INT32 rc = SDB_OK ;
      BSONObj bsonTemplate ;
      BSONObj bsonHostInfo ;
      BSONObj bsonConfigDetail ;
      BSONObj bsonConfig ;
      BSONObjBuilder opBuilder ;
      rc = _getTemplateInfo( bsonTemplate, bsonHostInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get config info failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _checkBusiness( _businessName, _businessType, _deployMod, 
                           _clusterName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_checkBusiness failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _getConfigDetail( bsonTemplate, bsonConfigDetail ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get config item failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _generateConfig( bsonTemplate, bsonHostInfo, bsonConfigDetail, 
                            bsonConfig ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "generate config failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _restAdaptor->appendHttpBody( _restSession, bsonConfig.objdata(),
                                    bsonConfig.objsize(), 1 ) ;

      opBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, opBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************CheckBusinessConfigReq *****************************
   omInstallBusinessReq::omInstallBusinessReq( restAdaptor *pRestAdaptor, 
                                               pmdRestSession *pRestSession, 
                                               const CHAR *pRootPath, 
                                               const CHAR *pSubPath,
                                               string localAgentHost, 
                                               string localAgentService)
                        :omConfigBusinessCommand( pRestAdaptor, 
                                                  pRestSession, pRootPath,
                                                  pSubPath ), 
                         _localAgentHost( localAgentHost ),
                         _localAgentService( localAgentService )
   {
   }

   omInstallBusinessReq::~omInstallBusinessReq()
   {
   }

   /*
   bsonConfValue:
   {
      "BusinessType":"sequoiadb", "BusinessName":"b1", "DeployMod":"xxx", 
      "ClusterName":"c1", 
      "Config":
      [
         {"HostName": "host1", "datagroupname": "", 
          "dbpath": "/home/db2/standalone/11830", "svcname": "11830", ...}
         ,...
      ]
   }
   bsonHostInfo:
   {
     "HostInfo":[
                   {
                      "HostName":"host1", "ClusterName":"c1", 
                      "Disk":{"Name":"/dev/sdb", Size:"", Mount:"", Used:""},
                      "Config":[{"BusinessName":"b2","dbpath":"", svcname:"", 
                                 "role":"", ... }, ...]
                   }
                    , ... 
                ]
   }
   */
   INT32 omInstallBusinessReq::_extractHostInfo( BSONObj &bsonConfValue, 
                                                 BSONObj &bsonHostInfo )
   {
      INT32 rc = SDB_OK ;

      BSONObjBuilder conditionBuilder ;
      BSONObj condition ;
      BSONObjBuilder builder ;
      BSONArrayBuilder arrayBuilder ;
      BSONObj config ;
      set<string> tmpHost ;

      conditionBuilder.append( OM_BSON_FIELD_HOST_NAME, "" ) ;
      condition = conditionBuilder.obj() ;
      config    = bsonConfValue.getObjectField( OM_BSON_FIELD_CONFIG ) ;
      {
         BSONObjIterator iter( config ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            BSONObj oneNode = ele.embeddedObject() ;
            BSONObj host    = oneNode.filterFieldsUndotted(condition, true );
            tmpHost.insert( host.getStringField( OM_BSON_FIELD_HOST_NAME ) ) ;
         }
      }

      set<string>::iterator iterSet = tmpHost.begin() ;
      while( iterSet != tmpHost.end() )
      {
         BSONObj host = BSON( OM_BSON_FIELD_HOST_NAME << *iterSet ) ;
         arrayBuilder.append( host ) ;
         iterSet++ ;
      }

      builder.append( OM_BSON_FIELD_HOST_INFO, arrayBuilder.arr() ) ;
      bsonHostInfo = builder.obj() ;

      rc = _fillHostInfo( _clusterName, _businessName, bsonHostInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_fillHostInfo failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omInstallBusinessReq::_combineConfDetail( string businessType, 
                                                   string deployMod, 
                                                   BSONObj &bsonAllConf )
   {
      INT32 rc = SDB_OK ;
      string templateFile ;
      list <BSONObj> deployModList ;
      list <BSONObj>::iterator iterList ;
      BSONObj bsonDeployMod ;
      BSONObj bsonDetail ;
      string confDetailFile ;

      templateFile   = _rootPath + OSS_FILE_SEP + OM_BUSINESS_CONFIG_SUBDIR 
                       + OSS_FILE_SEP + businessType + OM_TEMPLATE_FILE_NAME ;
      rc = _readConfTemplate( businessType, templateFile, deployModList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read template file failed:file=%s,rc=%d", 
                 templateFile.c_str(), rc ) ;
         goto error ;
      }

      iterList = deployModList.begin() ;
      while ( iterList != deployModList.end() )
      {
         string tmpDeployMod = iterList->getStringField( OM_BSON_DEPLOY_MOD ) ;
         if ( deployMod == tmpDeployMod )
         {
            bsonDeployMod = *iterList ;
            break ;
         }
         iterList++ ;
      }

      if ( iterList == deployModList.end() )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_DEPLOY_MOD ) + " is not exist:"
                        + deployMod ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      confDetailFile = _rootPath + OSS_FILE_SEP + OM_BUSINESS_CONFIG_SUBDIR 
                       + OSS_FILE_SEP + businessType 
                       + OM_CONFIG_ITEM_FILE_NAME ;
      rc = _readConfDetail( confDetailFile, bsonDetail ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read config file failed:file=%s,rc=%d", 
                 confDetailFile.c_str(), rc ) ;
         goto error ;
      }

      {
         BSONArrayBuilder arrayBuilder ;
         BSONObj properties ;
         properties = bsonDeployMod.getObjectField( OM_BSON_PROPERTY_ARRAY ) ;
         BSONObjIterator iter1( properties ) ;
         while ( iter1.more() )
         {
            BSONElement ele  = iter1.next() ;
            BSONObj tmp      = ele.embeddedObject() ;
            arrayBuilder.append( tmp ) ;
         }

         properties = bsonDetail.getObjectField( OM_BSON_PROPERTY_ARRAY ) ;
         BSONObjIterator iter2( properties ) ;
         while ( iter2.more() )
         {
            BSONElement ele  = iter2.next() ;
            BSONObj tmp      = ele.embeddedObject() ;
            arrayBuilder.append( tmp ) ;
         }

         BSONObjBuilder builder ;
         builder.append( OM_BSON_PROPERTY_ARRAY, arrayBuilder.arr() ) ;
         bsonAllConf = builder.obj() ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omInstallBusinessReq::_receiveFromAgent( 
                                                pmdRemoteSession *remoteSession,
                                                BSONObj &result )
   {
      VEC_SUB_SESSIONPTR subSessionVec ;
      INT32 rc           = SDB_OK ;
      MsgHeader *pRspMsg = NULL ;
      SINT32 flag        = 0 ;
      SINT64 contextID   = -1 ;
      SINT32 startFrom   = 0 ;
      SINT32 numReturned = 0 ;
      vector<BSONObj> objVec ;

      rc = remoteSession->waitReply( TRUE, &subSessionVec ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "wait reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( 1 != subSessionVec.size() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "unexpected session size:size=%d", 
                 subSessionVec.size() ) ;
         goto error ;
      }

      if ( subSessionVec[0]->isDisconnect() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG(PDERROR, "session disconnected:id=%s,rc=%d", 
                routeID2String(subSessionVec[0]->getNodeID()).c_str(), rc ) ;
         goto error ;
      }

      pRspMsg = subSessionVec[0]->getRspMsg() ;
      if ( NULL == pRspMsg )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive null response:rc=%d", rc ) ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)pRspMsg, &flag, &contextID, &startFrom, 
                            &numReturned, objVec ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "extract reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( 1 != objVec.size() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "unexpected response size:rc=%d", rc ) ;
         goto error ;
      }

      result = objVec[0] ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omInstallBusinessReq::_sendMsgToLocalAgent( omManager *om,
                                                pmdRemoteSession *remoteSession, 
                                                MsgHeader *pMsg )
   {
      MsgRouteID localAgentID ;
      INT32 rc = SDB_OK ;

      localAgentID = om->updateAgentInfo( _localAgentHost, 
                                          _localAgentService ) ;
      if ( NULL == remoteSession->addSubSession( localAgentID.value ) )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSubSession failed:id=%ld", localAgentID.value ) ;
         goto error ;
      }

      rc = remoteSession->sendMsg( pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send msg to localhost's agent failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void omInstallBusinessReq::_clearSession( omManager *om, 
                                             pmdRemoteSession *remoteSession) 
   {
      if ( NULL != remoteSession )
      {
         pmdSubSession *pSubSession = NULL ;
         pmdSubSessionItr itr       = remoteSession->getSubSessionItr() ;
         while ( itr.more() )
         {
            pSubSession = itr.next() ;
            MsgHeader *pMsg = pSubSession->getReqMsg() ;
            if ( NULL != pMsg )
            {
               SDB_OSS_FREE( pMsg ) ;
            }
         }

         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( remoteSession ) ;
      }
   }

   INT32 omInstallBusinessReq::_applyInstallRequest( 
                                                  const BSONObj &bsonConfValue,
                                                  UINT64 &taskID )
   {
      INT32 rc          = SDB_OK ;
      BSONElement taskElement ;
      BSONObj result ;
      CHAR* pContent    = NULL ;
      INT32 contentSize = 0 ;
      omManager *om     = sdbGetOMManager() ;
      MsgHeader *pMsg   = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      string fakeTaskID = OM_TASKINFO_FAKE_TASKID ;

      rc = om->storeTaskInfo( fakeTaskID, OM_INSTALL_BUSINESS_REQ, 
                              _localAgentHost, _localAgentService, 
                              bsonConfValue, OM_TASK_STATUS_DOING ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = _cb->getInfo( EDU_INFO_ERROR ) ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_INSTALL_BUSINESS_REQ, 
                             0, 0, 0, -1, &bsonConfValue, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "build msg failed:cmd=" ) 
                        + OM_INSTALL_BUSINESS_REQ ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      // create remote session
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_WAIT_SCAN_RES_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         _errorDetail = "create remote session failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "send message to agent failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      // receiving for agent's response
      rc = _receiveFromAgent( remoteSession, result ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = "receive from agent failed" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rc = result.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = result.getStringField( OM_REST_RES_DETAIL ) ;
         PD_LOG( PDERROR, "agent process failed:detail=%s,rc=%d", 
                 _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      taskElement = result.getField( OM_BSON_TASKID ) ;
      if ( taskElement.type() != NumberLong )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( "agent's response format error:res=" )
                        + result.toString( false, true ) ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      //TODO: we assume thas this should not fail;
      taskID = taskElement.numberLong() ;
      om->updateTaskID( fakeTaskID, taskElement.numberLong() ) ;
      rc = om->saveInstallTask( _localAgentHost, _localAgentService, result, 
                                bsonConfValue ) ;
      SDB_ASSERT( ( SDB_OK == rc ), "" ) ;
   done:
      om->removeTask( fakeTaskID ) ;
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
   bsonConfValue:
   {
      "BusinessType":"sequoiadb", "BusinessName":"b1", "deployMod":"xxx", 
      "ClusterName":"c1", 
      "Config":
      [
         {"HostName": "host1", "datagroupname": "", 
          "dbpath": "/home/db2/standalone/11830", "svcname": "11830", ...}
         ,...
      ]
   }
   bsonHostInfo:
   {
     "HostInfo":[
                   {
                      "HostName":"host1", "ClusterName":"c1", 
                      "Disk":{"Name":"/dev/sdb", Size:"", Mount:"", Used:""},
                      "Config":[{"BusinessName":"b2","dbpath":"", svcname:"", 
                                 "role":"", ... }, ...]
                   }
                    , ... 
                ]
   }
   */
   void omInstallBusinessReq::_compeleteConfValue( const BSONObj &bsonHostInfo, 
                                                   BSONObj &bsonConfValue )
   {
      map< string, simpleHostInfo > hostMap ;
      map< string, simpleHostInfo >::iterator iterMap ;
      BSONObj hostInfos ;
      hostInfos = bsonHostInfo.getObjectField( OM_BSON_FIELD_HOST_INFO ) ;
      BSONObjIterator iter( hostInfos ) ;
      while ( iter.more() )
      {
         simpleHostInfo host ;
         BSONElement ele  = iter.next() ;
         BSONObj oneHost  = ele.embeddedObject() ;

         host.hostName    = oneHost.getStringField( OM_HOST_FIELD_NAME ) ;

         host.user        = oneHost.getStringField( OM_HOST_FIELD_USER ) ;
         host.passwd      = oneHost.getStringField( 
                                                OM_HOST_FIELD_PASSWORD ) ;
         host.clusterName = oneHost.getStringField( 
                                                OM_HOST_FIELD_CLUSTERNAME ) ;
         hostMap[ host.hostName ] = host ;
      }

      BSONObj condition ;
      BSONObjBuilder conditionBuilder ;
      conditionBuilder.append( OM_BSON_FIELD_CONFIG, "" ) ;
      condition = conditionBuilder.obj() ;

      BSONArrayBuilder arrayBuilder ;
      BSONObj configs = bsonConfValue.filterFieldsUndotted( condition, true ) ;
      BSONObj commons = bsonConfValue.filterFieldsUndotted( condition, false ) ;
      BSONObj nodes ;
      nodes           = configs.getObjectField( OM_BSON_FIELD_CONFIG ) ;
      {
         BSONObjIterator iterBson( nodes ) ;
         while ( iterBson.more() )
         {
            BSONObjBuilder tmpBuilder ;
            BSONElement ele = iterBson.next() ;
            BSONObj oneNode = ele.embeddedObject() ;
            string hostName = oneNode.getStringField( 
                                                     OM_BSON_FIELD_HOST_NAME ) ;
            iterMap = hostMap.find( hostName ) ;
            SDB_ASSERT( iterMap != hostMap.end(), "" ) ;

            tmpBuilder.appendElements( oneNode ) ;
            tmpBuilder.append( OM_BSON_FIELD_HOST_USER, iterMap->second.user ) ;
            tmpBuilder.append( OM_BSON_FIELD_HOST_PASSWD, 
                               iterMap->second.passwd ) ;
            BSONObj tmp = tmpBuilder.obj() ;
            arrayBuilder.append( tmp ) ;
         }
      }

      BSONObjBuilder valueBuilder ;
      valueBuilder.append( OM_BSON_FIELD_CONFIG, arrayBuilder.arr() ) ;
      valueBuilder.appendElements( commons ) ;

      bsonConfValue = valueBuilder.obj() ;
   }

   INT32 omInstallBusinessReq::_getRestInfo( BSONObj &bsonConfValue )
   {
      INT32 rc          = SDB_OK ;
      const CHAR *pInfo = NULL ;

      _restAdaptor->getQuery( _restSession, OM_REST_CONFIG_INFO, &pInfo ) ;
      if ( NULL == pInfo )
      {
         _errorDetail = "rest field:" + string( OM_REST_CONFIG_INFO )
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pInfo, bsonConfValue ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "change rest field " ) + OM_REST_TEMPLATE_INFO
                        + " to BSONObj failed" ;
         PD_LOG( PDERROR, "%s:rc=%d,src=%s", _errorDetail.c_str(), rc, pInfo ) ;
         goto error ;
      }

      _clusterName = bsonConfValue.getStringField( 
                                                  OM_BSON_FIELD_CLUSTER_NAME ) ;
      if ( _clusterName == "" )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_FIELD_CLUSTER_NAME ) + " is empty" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      _businessType = bsonConfValue.getStringField( OM_BSON_BUSINESS_TYPE ) ;
      if ( _businessType == "" )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_BUSINESS_TYPE ) + " is empty" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      _businessName = bsonConfValue.getStringField( OM_BSON_BUSINESS_NAME ) ;
      if ( _businessName == "" )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_BUSINESS_NAME ) + " is empty" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      _deployMod = bsonConfValue.getStringField( OM_BSON_DEPLOY_MOD ) ;
      if ( _deployMod == "" )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( OM_BSON_DEPLOY_MOD ) + " is empty" ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omInstallBusinessReq::doCommand()
   {
      INT32 rc = SDB_OK ;
      omConfigGenerator confGenerator ;
      BSONObjBuilder opBuilder ;
      BSONObj bsonConfValue ;
      BSONObj bsonHostInfo ;
      BSONObj bsonAllConf ;
      UINT64 taskID ;

      rc = _getRestInfo( bsonConfValue ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_getRestInfo failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _checkBusiness( _businessName, _businessType, _deployMod, 
                           _clusterName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_checkBusiness failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _combineConfDetail( _businessType, _deployMod, bsonAllConf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_combineConfDetail failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _extractHostInfo( bsonConfValue, bsonHostInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get host info failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = confGenerator.checkSDBConfig( bsonConfValue, bsonAllConf, 
                                         bsonHostInfo ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = confGenerator.getErrorDetail() ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _compeleteConfValue( bsonHostInfo, bsonConfValue ) ;

      sdbGetOMManager()->getTaskWriteLock() ;
      if ( sdbGetOMManager()->isInstallTaskExist() )
      {
         rc = SDB_INVALIDARG ;
         INT32 status ;
         bool isAllFinshed ;
         string detail ;
         BSONObj progress ;
         string tmpID ;
         sdbGetOMManager()->getInstallTask( status, tmpID, isAllFinshed, 
                                            detail, progress ) ;
         _errorDetail = "exist install task:taskid=" + tmpID ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;

         sdbGetOMManager()->releaseTaskWriteLock() ;
         goto error ;
      }

      rc = _applyInstallRequest( bsonConfValue, taskID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_applyInstallRequest failed:rc=%d", rc ) ;
         sdbGetOMManager()->releaseTaskWriteLock() ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }
      sdbGetOMManager()->releaseTaskWriteLock() ;

      opBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      opBuilder.append( OM_BSON_TASKID, (long long)taskID ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, opBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // **************omQueryInstallProgress*************************
   omQueryInstallProgress::omQueryInstallProgress( restAdaptor *pRestAdaptor, 
                                                  pmdRestSession *pRestSession )
                          :omCreateClusterCommand( pRestAdaptor, pRestSession )
   {
   }

   omQueryInstallProgress::~omQueryInstallProgress()
   {
   }

   void omQueryInstallProgress::_testSaveTask()
   {
      BSONObjBuilder tmpTestBuilder ;
      BSONObj tmpTest ;
      tmpTestBuilder.append( OM_REST_RES_RETCODE, 0 ) ;
      tmpTestBuilder.append( OM_REST_RES_DETAIL, "haha" ) ;
      tmpTestBuilder.append( OM_BSON_TASKID, 12345LL ) ;
      tmpTestBuilder.append( OM_BSON_ISFINISHED, false ) ;
      {
         BSONObjBuilder haha ;
         haha.append( OM_BSON_ITEM_NAME, "coord" ) ;
         haha.append( OM_BSON_TOTAL_COUNT, 4 ) ;
         haha.append( OM_BSON_INSTALLED_COUNT, 2 ) ;
         haha.append( OM_BSON_ITEM_DESC, "xx" ) ;
         BSONObj bsonHaha = haha.obj() ;
         BSONArrayBuilder arrayBuilder ;
         arrayBuilder.append( bsonHaha ) ;
         tmpTestBuilder.appendArray( OM_BSON_TASK_PROGRESS, arrayBuilder.arr() ) ;
      }

      sdbGetOMManager()->storeTaskInfo( OM_TASKINFO_FAKE_TASKID, 
                                        OM_INSTALL_BUSINESS_REQ, 
                                        OM_DEFAULT_LOCAL_HOST, "11790", 
                                        BSONObj(), OM_TASK_STATUS_DOING ) ;
      tmpTest = tmpTestBuilder.obj() ;
      sdbGetOMManager()->saveInstallTask( OM_DEFAULT_LOCAL_HOST, "11790", 
                                          tmpTest, BSONObj() ) ;
   }

   void omQueryInstallProgress::_testUpdateTask()
   {
      BSONObjBuilder tmpTestBuilder ;
      BSONObj tmpTest ;
      tmpTestBuilder.append( OM_REST_RES_RETCODE, 0 ) ;
      tmpTestBuilder.append( OM_REST_RES_DETAIL, "haha" ) ;
      tmpTestBuilder.append( OM_BSON_TASKID, "ad" ) ;
      tmpTestBuilder.append( OM_BSON_ISFINISHED, false ) ;
      {
         BSONObjBuilder haha ;
         haha.append( OM_BSON_ITEM_NAME, "coord" ) ;
         haha.append( OM_BSON_TOTAL_COUNT, 4 ) ;
         haha.append( OM_BSON_INSTALLED_COUNT, 3 ) ;
         haha.append( OM_BSON_ITEM_DESC, "xx" ) ;
         BSONObj bsonHaha = haha.obj() ;
         BSONArrayBuilder arrayBuilder ;
         arrayBuilder.append( bsonHaha ) ;
         tmpTestBuilder.appendArray( OM_BSON_TASK_PROGRESS, arrayBuilder.arr() ) ;
      }
      tmpTest = tmpTestBuilder.obj() ;
      sdbGetOMManager()->updateInstallTask( tmpTest ) ;
   }

   void omQueryInstallProgress::_testFinishTask()
   {
      BSONObjBuilder tmpTestBuilder ;
      BSONObj tmpTest ;
      tmpTestBuilder.append( OM_REST_RES_RETCODE, 0 ) ;
      tmpTestBuilder.append( OM_REST_RES_DETAIL, "haha" ) ;
      tmpTestBuilder.append( OM_BSON_TASKID, "ad" ) ;
      tmpTestBuilder.append( OM_BSON_ISFINISHED, true ) ;
      {
         BSONObjBuilder haha ;
         haha.append( OM_BSON_ITEM_NAME, "coord" ) ;
         haha.append( OM_BSON_TOTAL_COUNT, 4 ) ;
         haha.append( OM_BSON_INSTALLED_COUNT, 4 ) ;
         haha.append( OM_BSON_ITEM_DESC, "xx" ) ;
         BSONObj bsonHaha = haha.obj() ;
         BSONArrayBuilder arrayBuilder ;
         arrayBuilder.append( bsonHaha ) ;
         tmpTestBuilder.appendArray( OM_BSON_TASK_PROGRESS, arrayBuilder.arr() ) ;
      }
      tmpTest = tmpTestBuilder.obj() ;
      sdbGetOMManager()->finishInstallTask( tmpTest ) ;
   }

   INT32 omQueryInstallProgress::doCommand()
   {
      INT32 status ;
      string taskID ;
      bool isAllFinished ;
      string detail ;
      BSONObj progress ;

      BSONObj restTask ;
      string restTaskID ;
      INT32 rc          = SDB_OK ;
      const CHAR *pTask = NULL ;
      _restAdaptor->getQuery( _restSession, OM_REST_TASK_INFO, &pTask ) ;
      if ( NULL == pTask )
      {
         _errorDetail = "rest field:" + string( OM_REST_TASK_INFO )
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = fromjson( pTask, restTask ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "change rest field " ) + OM_REST_TASK_INFO
                        + " to BSONObj failed" ;
         PD_LOG( PDERROR, "%s:rc=%d,src=%s", _errorDetail.c_str(), rc, pTask ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      sdbGetOMManager()->getTaskWriteLock() ;
//      _testSaveTask() ;
//      _testUpdateTask() ;
      //_testFinishTask() ;
      sdbGetOMManager()->getInstallTask( status, taskID, isAllFinished, detail,
                                         progress ) ;
      sdbGetOMManager()->releaseTaskWriteLock() ;

      restTaskID = restTask.getStringField( OM_BSON_TASKID ) ;
      if ( OM_TASK_STATUS_IDLE == status )
      {
         rc = SDB_OM_TASK_NOT_EXIST ;
         _errorDetail = "task is not exist:task=" + restTaskID ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
      }
      else
      {
         BSONObjBuilder opBuilder ;
         BSONObj op ;
         INT32 restRC = SDB_OK ;
         if ( taskID.compare( restTaskID ) != 0 )
         {
            rc = SDB_OM_TASK_NOT_EXIST ;
            _errorDetail = "task is not exist:task=" + restTaskID ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
            goto error ;
         }

         if ( OM_TASK_STATUS_DOING == status )
         {
            restRC = SDB_OK ;
         }
         else if ( OM_TASK_STATUS_ERROR_ROLLBACK == status )
         {
            restRC = SDB_OM_TASK_ROLLBACK ;
         }
         else
         {
            //OM_TASK_STATUS_ERROR_FINISH/OM_TASK_STATUS_FINISH
            SDB_ASSERT( isAllFinished, "" ) ;
            restRC = SDB_OK ;
         }

         opBuilder.append( OM_REST_RES_RETCODE, restRC ) ;
         opBuilder.append( OM_REST_RES_DETAIL, detail ) ;
         opBuilder.append( OM_BSON_TASKID, taskID ) ;
         opBuilder.append( OM_BSON_ISFINISHED, isAllFinished ) ;
         opBuilder.appendArray( OM_BSON_TASK_PROGRESS, progress ) ;
         op = opBuilder.obj() ;
         _restAdaptor->setOPResult( _restSession, SDB_OK, op ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omQueryNodeConfCommand *****************************
   omQueryNodeConfCommand::omQueryNodeConfCommand( restAdaptor *pRestAdaptor, 
                                                  pmdRestSession *pRestSession )
                          :omAuthCommand( pRestAdaptor, pRestSession )
   {
   }

   omQueryNodeConfCommand::~omQueryNodeConfCommand()
   {
   }

   INT32 omQueryNodeConfCommand::_getNodeInfo( string businessName, 
                                           map<string, BSONObj> &mapHostConf )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_CONFIGURE_FIELD_BUSINESSNAME << businessName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CONFIGURE, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_CONFIGURE ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         BSONObjBuilder innerBuilder ;
         BSONObj tmp ;
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_CONFIGURE ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         string hostName ;
         hostName = result.getStringField( OM_CONFIGURE_FIELD_HOSTNAME ) ;
         mapHostConf.insert( map<string, BSONObj>::value_type( hostName, 
                                                             result.copy() ) ) ;
      }
   done:
      return SDB_OK ;
   error:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      goto done ;
   }

   void omQueryNodeConfCommand::_sendNodeInfo2Web( 
                                             map<string, BSONObj> &mapHostConf )
   {
      BSONObjBuilder opBuilder ;
      map<string, BSONObj>::iterator iter = mapHostConf.begin() ;
      while ( iter != mapHostConf.end() )
      {
         string hostName = iter->first ;
         BSONObj config  = iter->second ;
         BSONObj confs   = config.getObjectField( OM_BSON_FIELD_CONFIG ) ;
         {
            BSONObjIterator iterBson( confs ) ;
            while ( iterBson.more() )
            {
               BSONObjBuilder builder ;
               BSONElement ele = iterBson.next() ;
               BSONObj oneNode = ele.embeddedObject() ;
               builder.appendElements( oneNode ) ;
               builder.append( OM_BSON_FIELD_HOST_NAME, hostName ) ;

               BSONObj tmp = builder.obj() ;
               _restAdaptor->appendHttpBody( _restSession, tmp.objdata(), 
                                             tmp.objsize(), 1 ) ;
            }
         }
         iter++ ;
      }

      opBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, opBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

      return ;
   }

   INT32 omQueryNodeConfCommand::_test()
   {
      string business = "b1" ;
      string host     = "h1" ;
      if ( sdbGetOMManager()->_isHostConfExist( host, business ) )
      {
         BSONObj conf = BSON( OM_CONFIGURE_FIELD_HOSTNAME << "hh" ) ;
         sdbGetOMManager()->_appendConfigure( host, business, conf ) ;
      }
      else
      {
         BSONObj conf = BSON( OM_CONFIGURE_FIELD_HOSTNAME << "hh" ) ;
         sdbGetOMManager()->_insertConfigure( host, business, conf ) ;
      }

      if ( sdbGetOMManager()->_isHostConfExist( host, business ) )
      {
         BSONObj conf = BSON( OM_CONFIGURE_FIELD_HOSTNAME << "kk" ) ;
         sdbGetOMManager()->_appendConfigure( host, business, conf ) ;
      }
      else
      {
         BSONObj conf = BSON( OM_CONFIGURE_FIELD_HOSTNAME << "kk" ) ;
         sdbGetOMManager()->_insertConfigure( host, business, conf ) ;
      }

      return SDB_OK ;
   }

   INT32 omQueryNodeConfCommand::doCommand()
   {
      INT32 rc                 = SDB_OK ;
      const CHAR *businessName = NULL ;
      map<string, BSONObj> mapHostConf ;


      _restAdaptor->getQuery( _restSession, OM_REST_BUSINESS_NAME, 
                              &businessName ) ;
      if ( NULL == businessName )
      {
         _errorDetail = "rest field:" + string( OM_REST_BUSINESS_NAME )
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _getNodeInfo( businessName, mapHostConf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _sendNodeInfo2Web( mapHostConf ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omQueryBusinessCommand *****************************
   omQueryBusinessCommand::omQueryBusinessCommand( restAdaptor *pRestAdaptor, 
                                                  pmdRestSession *pRestSession )
                          :omAuthCommand( pRestAdaptor, pRestSession )
   {
   }

   omQueryBusinessCommand::~omQueryBusinessCommand()
   {
   }

   void omQueryBusinessCommand::_sendBusinessInfo2Web( 
                                                   list<BSONObj> &listBusiness )
   {
      BSONObjBuilder opBuilder ;
      list<BSONObj>::iterator iter = listBusiness.begin() ;
      while ( iter != listBusiness.end() )
      {
         _restAdaptor->appendHttpBody( _restSession, (*iter).objdata(), 
                                            (*iter).objsize(), 1 ) ;
         iter++ ;
      }

      opBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, opBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

      return ;
   }

   INT32 omQueryBusinessCommand::_getBusinessInfoByCluster( string clusterName, 
                                                   list<BSONObj> &listBusiness )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_BUSINESS_FIELD_CLUSTERNAME << clusterName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_BUSINESS, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_BUSINESS ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         BSONObjBuilder innerBuilder ;
         BSONObj tmp ;
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_BUSINESS ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         listBusiness.push_back( result.copy() ) ;
      }
   done:
      return SDB_OK ;
   error:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      goto done ;
   }

   INT32 omQueryBusinessCommand::_getBusinessInfoByBusiness( string business, 
                                                   list<BSONObj> &listBusiness )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_BUSINESS_FIELD_NAME << business ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_BUSINESS, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_BUSINESS ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         BSONObjBuilder innerBuilder ;
         BSONObj tmp ;
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_BUSINESS ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         listBusiness.push_back( result.copy() ) ;
         break ;
      }
   done:
      return SDB_OK ;
   error:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      goto done ;
   }

   INT32 omQueryBusinessCommand::doCommand()
   {
      INT32 rc                 = SDB_OK ;
      const CHAR *clusterName  = NULL ;
      const CHAR *business     = NULL ;
      list<BSONObj> listBusiness ;

      _restAdaptor->getQuery( _restSession, OM_REST_CLUSTER_NAME, 
                              &clusterName ) ;
      _restAdaptor->getQuery( _restSession, OM_REST_BUSINESS_NAME, 
                              &business ) ;
      if ( ( NULL == clusterName ) && ( NULL == business ) )
      {
         _errorDetail = "rest field:" + string( OM_REST_CLUSTER_NAME ) + " and "
                        OM_REST_BUSINESS_NAME + " must exist one field" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      if ( NULL == business )
      {
         rc = _getBusinessInfoByCluster( clusterName, listBusiness ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
            goto error ;
         }
      }
      else
      {
         rc = _getBusinessInfoByBusiness( business, listBusiness ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
            _sendErrorRes2Web( rc, _errorDetail ) ;
            goto error ;
         }
      }

      _sendBusinessInfo2Web( listBusiness ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omStartBusinessCommand *****************************
   omStartBusinessCommand::omStartBusinessCommand( restAdaptor *pRestAdaptor, 
                                                   pmdRestSession *pRestSession,
                                                   string localAgentHost, 
                                                   string localAgentService )
                          :omScanHostCommand( pRestAdaptor, pRestSession,
                                             localAgentHost, localAgentService ) 
   {
   }

   omStartBusinessCommand::~omStartBusinessCommand()
   {
   }

   INT32 omStartBusinessCommand::_getNodeInfo( const string &businessName, 
                                               BSONObj &nodeInfos,
                                               BOOLEAN &isExistFlag )
   {
      BSONArrayBuilder arrayBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      isExistFlag      = FALSE ;

      matcher = BSON( OM_CONFIGURE_FIELD_BUSINESSNAME << businessName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CONFIGURE, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_CONFIGURE ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         string hostName ;
         BSONObjBuilder builder ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc          = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_CONFIGURE ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         isExistFlag = TRUE ;
         BSONObj result( buffObj.data() ) ;
         rc = _expandNodeInfoToBuilder( result, arrayBuilder ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "_expandNodeInfoToBuilder failed:rc=%d", rc ) ;
            goto error ;
         }
      }

      nodeInfos = BSON( OM_BSON_FIELD_CONFIG << arrayBuilder.arr() ) ;
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /*
     record:
     {
       BusinessName:"b1", Hostname:"h1"
       Config:
       [
         {
           dbpath:"", role:"", logfilesz:"", ...
         }, ...
       ]
     }

     this function transfer record to a BSON like below, and add to arrayBuilder

     {
       HostName:"", User:"", Passwd:"", dbpath:"", role:"", logfilesz:"", ...
     }
   */
   INT32 omStartBusinessCommand::_expandNodeInfoToBuilder( 
                                                const BSONObj &record, 
                                                BSONArrayBuilder &arrayBuilder )
   {
      INT32 rc = SDB_OK ;
      string hostName ;
      simpleHostInfo hostInfo ;
      BOOLEAN isHostExist = FALSE ;
      BSONObj confs ;
      hostName = record.getStringField( OM_CONFIGURE_FIELD_HOSTNAME ) ;
      rc = _getHostInfo( hostName, hostInfo, isHostExist ) ;
      if ( SDB_OK != rc || !isHostExist )
      {
         _errorDetail = string( "failed to get record from table:" )
                        + OM_CS_DEPLOY_CL_HOST + ",host=" + hostName ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      confs = record.getObjectField( OM_CONFIGURE_FIELD_CONFIG ) ;
      {
         BSONObjIterator iter( confs ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            BSONObjBuilder builder ;
            builder.append( ele ) ;
            builder.append( OM_BSON_FIELD_HOST_NAME, hostInfo.hostName ) ;
            builder.append( OM_BSON_FIELD_HOST_USER, hostInfo.user ) ;
            builder.append( OM_BSON_FIELD_HOST_PASSWD, hostInfo.passwd ) ;

            BSONObj oneNode = builder.obj() ;
            arrayBuilder.append( oneNode ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omStartBusinessCommand::_getHostInfo( const string &hostName,
                                               simpleHostInfo &hostInfo,
                                               BOOLEAN &isExistFlag )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_HOST_FIELD_NAME << hostName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_HOST ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc          = SDB_OK ;
               isExistFlag = FALSE ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_HOST ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         hostInfo.hostName    = result.getStringField( OM_HOST_FIELD_NAME ) ;
         hostInfo.clusterName = result.getStringField( 
                                                   OM_HOST_FIELD_CLUSTERNAME ) ;
         hostInfo.ip          = result.getStringField( OM_HOST_FIELD_IP ) ;
         hostInfo.user        = result.getStringField( OM_HOST_FIELD_USER ) ;
         hostInfo.passwd      = result.getStringField( 
                                                   OM_HOST_FIELD_PASSWORD ) ;
         hostInfo.installPath = result.getStringField( 
                                                   OM_HOST_FIELD_INSTALLPATH ) ;
         isExistFlag = TRUE ;
         break ;
      }
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omStartBusinessCommand::doCommand()
   {
      INT32 rc = SDB_OK ;
      return rc ;
   }

   // *****************omStopBusinessCommand *****************************
   omStopBusinessCommand::omStopBusinessCommand( restAdaptor *pRestAdaptor, 
                                                 pmdRestSession *pRestSession,
                                                 string localAgentHost, 
                                                 string localAgentService )
                         :omScanHostCommand( pRestAdaptor, pRestSession,
                                             localAgentHost, localAgentService ) 
   {
   }

   omStopBusinessCommand::~omStopBusinessCommand()
   {
   }

   INT32 omStopBusinessCommand::doCommand()
   {
      INT32 rc = SDB_OK ;
      return rc ;
   }

   // *****************omRemoveClusterCommand *****************************
   omRemoveClusterCommand::omRemoveClusterCommand( restAdaptor *pRestAdaptor, 
                                                  pmdRestSession *pRestSession )
                          :omAuthCommand( pRestAdaptor, pRestSession )
   {
   }

   omRemoveClusterCommand::~omRemoveClusterCommand()
   {
   }

   INT32 omRemoveClusterCommand::_getClusterExistFlag( 
                                                      const string &clusterName, 
                                                      BOOLEAN &flag )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_CLUSTER_FIELD_NAME << clusterName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_CLUSTER ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               flag = FALSE ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_CLUSTER ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         flag = TRUE ;
         break ;
      }
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveClusterCommand::_getClusterExistHostFlag( 
                                                      const string &clusterName, 
                                                      BOOLEAN &flag )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_HOST_FIELD_CLUSTERNAME << clusterName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_HOST ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               flag = FALSE ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_HOST ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         flag = TRUE ;
         break ;
      }
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveClusterCommand::_removeCluster( const string &clusterName )
   {
      INT32 rc          = SDB_OK ;
      BSONObj condition = BSON( OM_CLUSTER_FIELD_NAME << clusterName ) ;
      BSONObj hint ;

      rc = rtnDelete( OM_CS_DEPLOY_CL_CLUSTER, condition, hint, 0, _cb );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to delete taskinfo from table:%s,"
                     "%s=%s,rc=%d", OM_CS_DEPLOY_CL_CLUSTER, 
                     OM_CLUSTER_FIELD_NAME, clusterName.c_str(), rc ) ;
         _errorDetail = _cb->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveClusterCommand::doCommand()
   {
      INT32 rc                   = SDB_OK ;
      const CHAR *clusterName    = NULL ;
      BOOLEAN isClusterExist     = FALSE ;
      BSONObj result ;
      BOOLEAN isClusterExistHost = FALSE ;

      _restAdaptor->getQuery( _restSession, OM_REST_CLUSTER_NAME, 
                              &clusterName ) ;
      if ( NULL == clusterName )
      {
         _errorDetail = "rest field:" + string( OM_REST_CLUSTER_NAME )
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _getClusterExistFlag( clusterName, isClusterExist ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      if ( !isClusterExist )
      {
         result = BSON( OM_REST_RES_RETCODE << SDB_DMS_RECORD_NOTEXIST 
                        << OM_REST_RES_DETAIL 
                        << ( string( clusterName ) + " is not exist" ) ) ;
         _restAdaptor->setOPResult( _restSession, SDB_OK, result ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
         goto done ;
      }

      rc = _getClusterExistHostFlag( clusterName, isClusterExistHost ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      if ( isClusterExistHost )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( "host exist in cluster, host should be "
                                "removed first:cluster=" ) + clusterName ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _removeCluster( clusterName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      result = BSON( OM_REST_RES_RETCODE << SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, result ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omRemoveHostCommand *****************************
   omRemoveHostCommand::omRemoveHostCommand( restAdaptor *pRestAdaptor, 
                                             pmdRestSession *pRestSession,
                                             string localAgentHost, 
                                             string localAgentService )
                       :omStartBusinessCommand( pRestAdaptor, pRestSession, 
                                                localAgentHost, 
                                                localAgentService )
   {
   }

   omRemoveHostCommand::~omRemoveHostCommand()
   {
   }

   INT32 omRemoveHostCommand::_getHostName( string &hostName, 
                                            BOOLEAN &isForced )
   {
      INT32 rc              = SDB_OK ;
      const CHAR *pHostName = NULL ;
      const CHAR *pForce    = NULL ;
      _restAdaptor->getQuery( _restSession, OM_REST_HOST_NAME, 
                              &pHostName ) ;
      if ( NULL == pHostName )
      {
         _errorDetail = "rest field:" + string( OM_REST_HOST_NAME )
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      hostName = pHostName ;
      isForced = FALSE ;
      _restAdaptor->getQuery( _restSession, OM_REST_ISFORCE, 
                              &pForce ) ;
      if ( ( NULL != pForce ) && ( ossStrcasecmp( pForce, "1" ) == 0 ) )
      {
         isForced = TRUE ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveHostCommand::_getHostExistBusinessFlag( const string &hostName, 
                                                         BOOLEAN &flag )
   {
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_CONFIGURE_FIELD_HOSTNAME << hostName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CONFIGURE, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_CONFIGURE ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               flag = FALSE ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_CONFIGURE ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         flag = TRUE ;
         break ;
      }
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveHostCommand::_deleteHostRecord( const string &hostName )
   {
      INT32 rc          = SDB_OK ;
      BSONObj condition = BSON( OM_HOST_FIELD_NAME << hostName ) ;
      BSONObj hint ;

      rc = rtnDelete( OM_CS_DEPLOY_CL_HOST, condition, hint, 0, _cb );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to delete record from table:%s,"
                     "%s=%s,rc=%d", OM_CS_DEPLOY_CL_HOST, 
                     OM_HOST_FIELD_NAME, hostName.c_str(), rc ) ;
         _errorDetail = _cb->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveHostCommand::_removeHostByAgent( 
                                                const simpleHostInfo &hostInfo )
   {
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      omManager *om     = NULL ;
      MsgHeader *pMsg   = NULL ;

      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonResponse ;
      BSONObj bsonRequest ;
      bsonRequest = BSON( OM_BSON_FIELD_HOST_NAME << hostInfo.hostName 
                       << OM_BSON_FIELD_HOST_IP << hostInfo.ip 
                       << OM_BSON_FIELD_HOST_USER << hostInfo.user 
                       << OM_BSON_FIELD_HOST_PASSWD << hostInfo.passwd 
                       << OM_BSON_FIELD_INSTALLPATH << hostInfo.installPath ) ;

      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_REMOVE_HOST_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "build message failed:cmd=" ) 
                        + OM_REMOVE_HOST_REQ ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      // send request to agent
      om   = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_MSG_TIMEOUT_TWO_HOUR,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         _errorDetail = string( "create remote session failed" ) ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "send message to agent failed" ) ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      rc = _receiveFromAgent( remoteSession, bsonResponse ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "receive from agent failed" ) ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rc = bsonResponse.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "agent's response error:res=" )
                        + bsonResponse.toString( false, true ) ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }
   done:
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveHostCommand::_removeHost( const simpleHostInfo &hostInfo, 
                                           BOOLEAN isForced )
   {
      INT32 rc = SDB_OK ;
      rc = _removeHostByAgent( hostInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "agent remove host failed:rc=%d", rc ) ;
         if ( !isForced )
         {
            goto error ;
         }
      }

      _errorDetail = "" ;
      rc = _deleteHostRecord( hostInfo.hostName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "delete host's record failed:host=%s,rc=%d", 
                 hostInfo.hostName.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveHostCommand::doCommand()
   {
      INT32 rc = SDB_OK ;
      string hostName ;
      BSONObj result ;
      simpleHostInfo hostInfo ;
      BOOLEAN isForced            = FALSE ;
      BOOLEAN isHostExist         = FALSE ;
      BOOLEAN isHostExistBusiness = FALSE ;

      rc = _getHostName( hostName, isForced ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _getHostInfo( hostName, hostInfo, isHostExist ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      if ( !isHostExist )
      {
         result = BSON( OM_REST_RES_RETCODE << SDB_DMS_RECORD_NOTEXIST 
                        << OM_REST_RES_DETAIL 
                        << ( hostName + " is not exist" ) ) ;
         _restAdaptor->setOPResult( _restSession, SDB_OK, result ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
         goto done ;
      }

      rc = _getHostExistBusinessFlag( hostName, isHostExistBusiness ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      if ( isHostExistBusiness )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( "business exist in host, business should be "
                                "removed first:host=" ) + hostName.c_str() ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _removeHost( hostInfo, isForced ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      result = BSON( OM_REST_RES_RETCODE << SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, result ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omRemoveBusinessCommand *****************************
   omRemoveBusinessCommand::omRemoveBusinessCommand( restAdaptor *pRestAdaptor, 
                                                   pmdRestSession *pRestSession,
                                                   string localAgentHost, 
                                                   string localAgentService )
                           :omStartBusinessCommand( pRestAdaptor, pRestSession, 
                                                    localAgentHost, 
                                                    localAgentService )
   {
   }

   omRemoveBusinessCommand::~omRemoveBusinessCommand()
   {
   }

   INT32 omRemoveBusinessCommand::_getBusinessExistFlag( 
                                     const string &businessName, BOOLEAN &flag )
   {
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_BUSINESS_FIELD_NAME << businessName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_BUSINESS, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         _errorDetail = string( "fail to query table:" ) 
                        + OM_CS_DEPLOY_CL_BUSINESS ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               flag = FALSE ;
               break ;
            }

            contextID = -1 ;
            _errorDetail = string( "failed to get record from table:" )
                           + OM_CS_DEPLOY_CL_BUSINESS ;
            PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
            goto error ;
         }

         flag = TRUE ;
         break ;
      }
   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveBusinessCommand::_removeBusinessByAgent( 
                                             const BSONObj &nodeInfos )
   {
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      omManager *om     = NULL ;
      MsgHeader *pMsg   = NULL ;

      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonResponse ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_REMOVE_BUSINESS_REQ, 
                             0, 0, 0, -1, &nodeInfos, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "build message failed:cmd=" ) 
                        + OM_REMOVE_BUSINESS_REQ ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      // send request to agent
      om   = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_MSG_TIMEOUT_TWO_HOUR,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         _errorDetail = string( "create remote session failed" ) ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "send message to agent failed" ) ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         SDB_OSS_FREE( pContent ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      rc = _receiveFromAgent( remoteSession, bsonResponse ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "receive from agent failed" ) ;
         PD_LOG( PDERROR, "%s:rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }

      rc = bsonResponse.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "agent's response error:res=" )
                        + bsonResponse.toString( false, true ) ;
         PD_LOG( PDERROR, "%s,rc=%d", _errorDetail.c_str(), rc ) ;
         goto error ;
      }
   done:
      _clearSession( om, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveBusinessCommand::_deleteConfigureRecord( 
                                             const string &businessName )
   {
      INT32 rc = SDB_OK ;
      BSONObj condition ;
      BSONObj hint ;

      condition = BSON( OM_CONFIGURE_FIELD_BUSINESSNAME << businessName ) ;
      rc = rtnDelete( OM_CS_DEPLOY_CL_CONFIGURE, condition, hint, 0, _cb );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to delete record from table:%s,"
                     "%s=%s,rc=%d", OM_CS_DEPLOY_CL_CONFIGURE, 
                     OM_CONFIGURE_FIELD_BUSINESSNAME, businessName.c_str(), 
                     rc ) ;
         _errorDetail = _cb->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveBusinessCommand::_deleteBusinessRecord( 
                                             const string &businessName )
   {
      INT32 rc = SDB_OK ;
      BSONObj condition ;
      BSONObj hint ;

      condition = BSON( OM_BUSINESS_FIELD_NAME << businessName ) ;
      rc = rtnDelete( OM_CS_DEPLOY_CL_BUSINESS, condition, hint, 0, _cb );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to delete record from table:%s,"
                     "%s=%s,rc=%d", OM_CS_DEPLOY_CL_BUSINESS, 
                     OM_BUSINESS_FIELD_NAME, businessName.c_str(), rc ) ;
         _errorDetail = _cb->getInfo( EDU_INFO_ERROR ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveBusinessCommand::_removeBusiness( const string &businessName,
                                                   const BSONObj &nodeInfos, 
                                                   BOOLEAN isExistNode,
                                                   BOOLEAN isForced )
   {
      INT32 rc = SDB_OK ;
      if ( isExistNode )
      {
         rc = _removeBusinessByAgent( nodeInfos ) ;
         if ( SDB_OK != rc )
         {
            if ( !isForced )
            {
               PD_LOG( PDERROR, "agent remove business failed:business=%s,rc=%d",
                       businessName.c_str(), rc ) ;
               goto error ;
            }
         }
      }

      rc = _deleteConfigureRecord( businessName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "delete configure's record failed:business=%s,rc=%d", 
                 businessName.c_str(), rc ) ;
         goto error ;
      }

      rc = _deleteBusinessRecord( businessName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "delete business's record failed:business=%s,rc=%d", 
                 businessName.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omRemoveBusinessCommand::doCommand()
   {
      BSONObj nodeInfos ;
      BSONObj result ;
      BOOLEAN isForced            = FALSE ;
      BOOLEAN isBusinessExist     = FALSE ;
      BOOLEAN isBusinessExistNode = FALSE ;
      INT32 rc                    = SDB_OK ;
      const CHAR *pBusinessName   = NULL ;
      const CHAR *pForce          = NULL ;
      _restAdaptor->getQuery( _restSession, OM_REST_BUSINESS_NAME, 
                              &pBusinessName ) ;
      if ( NULL == pBusinessName )
      {
         _errorDetail = "rest field:" + string( OM_REST_BUSINESS_NAME )
                        + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      _restAdaptor->getQuery( _restSession, OM_REST_ISFORCE, &pForce ) ;
      if ( ( NULL != pForce ) && ( ossStrcasecmp( pForce, "1" ) == 0 ) )
      {
         isForced = TRUE ;
      }

      rc = _getBusinessExistFlag( pBusinessName, isBusinessExist ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      if ( !isBusinessExist )
      {
         BSONObj result = BSON( OM_REST_RES_RETCODE << SDB_DMS_RECORD_NOTEXIST 
                            << OM_REST_RES_DETAIL 
                            << ( string( pBusinessName ) + " is not exist" ) ) ;
         _restAdaptor->setOPResult( _restSession, SDB_OK, result ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
         goto done ;
      }

      rc = _getNodeInfo( pBusinessName, nodeInfos, isBusinessExistNode ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get node info failed:business=%s,rc=%d", 
                 pBusinessName, rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      rc = _removeBusiness( pBusinessName, nodeInfos, isBusinessExistNode, 
                            isForced ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "remove business failed:business=%s,rc=%d", 
                 pBusinessName, rc ) ;
         _sendErrorRes2Web( rc, _errorDetail ) ;
         goto error ;
      }

      result = BSON( OM_REST_RES_RETCODE << SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, result ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // *****************omGetFileCommand *****************************
   omGetFileCommand::omGetFileCommand( restAdaptor *pRestAdaptor, 
                                       pmdRestSession *pRestSession, 
                                       const CHAR *pRootPath,
                                       const CHAR *pSubPath )
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
      _rootPath    = pRootPath ;
      _subPath     = pSubPath ;
   }

   omGetFileCommand::~omGetFileCommand()
   {
   }

   INT32 omGetFileCommand::doCommand() 
   {
      INT32 rc                      = SDB_OK ;
      CHAR *pContent                = NULL ;
      INT32 contentLength           = 0 ;
      restFileController* transfer = NULL ;
      string realSubPath            = _subPath ;

      transfer = restFileController::getTransferInstance() ;
      transfer->getTransferedPath( _subPath.c_str(), realSubPath ) ;

      rc = _getFileContent( _rootPath + realSubPath, &pContent, 
                            contentLength ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_FNE == rc )
         {
            PD_LOG( PDEVENT, "OM: file no found:%s, rc=%d", 
                    realSubPath.c_str(), rc ) ;
            _restAdaptor->sendResponse( _restSession, HTTP_NOTFOUND ) ;
         }
         else
         {
            PD_LOG( PDEVENT, "OM: open file failed:%s, rc=%d", 
                    realSubPath.c_str(), rc ) ;
            _restAdaptor->sendResponse( _restSession, HTTP_SERVICUNAVA ) ;
         }

         goto error ;
      }

      _restAdaptor->appendHttpBody( _restSession, pContent, contentLength ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

      _restSession->releaseBuff( pContent, contentLength ) ;

   done:
      return SDB_OK ;
   error:
      goto done ;
   }

   INT32 omGetFileCommand::undoCommand()
   {
      return SDB_OK ;
   }

   INT32 omGetFileCommand::_getFileContent( string filePath, 
                                            CHAR **pFileContent, 
                                            INT32 &fileContentLen )
   {
      OSSFILE file ;
      INT32 rc               = SDB_OK ;
      INT64 fileSize         = 0 ;
      SINT64 realFileSize    = 0 ;
      INT32 buffSize         = 0 ;
      bool isFileOpened      = false ;
      bool isAllocBuff       = false ;

      rc = ossOpen( filePath.c_str(), OSS_READONLY, OSS_RWXU, file ) ;
      PD_RC_CHECK ( rc, PDERROR, 
                    "Failed to open file:file=%s, rc = %d", 
                    filePath.c_str(), rc ) ;

      isFileOpened = true ;
      rc           = ossGetFileSize( &file, &fileSize ) ;
      PD_RC_CHECK ( rc, PDERROR, 
                    "Failed to get file size:file=%s, rc = %d", 
                    filePath.c_str(), rc ) ;

      rc = _restSession->allocBuff( fileSize, pFileContent, buffSize ) ;
      PD_RC_CHECK ( rc, PDERROR, 
                    "Failed to alloc buff:buff_size=%I64d, rc = %d", 
                    fileSize, rc ) ;

      isAllocBuff = true ;
      rc          = ossRead( &file, *pFileContent, fileSize, &realFileSize ) ;
      PD_RC_CHECK ( rc, PDERROR, 
                    "Failed to read file content:file=%s, rc = %d", 
                    filePath.c_str(), rc ) ;

      fileContentLen = realFileSize ;

   done:
      if ( isFileOpened )
      {
         ossClose( file ) ;
      }

      if ( isAllocBuff )
      {
         _restSession->releaseBuff( *pFileContent, buffSize ) ;
      }

      return rc ;
   error:
      goto done ;
   }


   restFileController::restFileController()
   {
      INT32 pairCount   = 0 ;
      INT32 i           = 0 ;
      INT32 publicCount = 0 ;

      static char* filePathTable[][2] = {
         { OSS_FILE_SEP,    OSS_FILE_SEP OM_REST_INDEX_HTML }
      } ;

      // only html file, other file is all public now(from jiawen)
      static char *fileAuthorityPublic[] = {
         OSS_FILE_SEP OM_REST_LOGIN_HTML ,
      } ;

      pairCount = sizeof( filePathTable ) / ( 2 * sizeof ( char * ) ) ;

      for ( i = 0 ; i < pairCount ; i++ )
      {
         _transfer.insert( mapValueType( filePathTable[i][0], 
                                                    filePathTable[i][1])) ; 
      }

      publicCount = sizeof( fileAuthorityPublic ) / ( sizeof ( char * ) ) ;
      for ( i = 0 ; i < publicCount ; i++ )
      {
         _publicAccessFiles.insert( mapValueType( fileAuthorityPublic[i], 
                                                  fileAuthorityPublic[i])) ; 
      }
   }

   restFileController* restFileController::getTransferInstance()
   {
      static restFileController instance ;
      return &instance ;
   }

   INT32 restFileController::getTransferedPath( const char *src, 
                                                   string &transfered )
   {
      INT32 rc = SDB_OK ;
      mapIteratorType ite = _transfer.find( src ) ;
      if ( ite == _transfer.end() )
      {
         goto error ;
      }

      transfered = ite->second ;

   done:
      return rc ; 
   error:
      rc = -1 ;
      goto done ;
   }

   bool restFileController::isFileAuthorPublic( const char *file ) 
   {
      mapIteratorType ite = _publicAccessFiles.find( file ) ;
      if ( ite == _publicAccessFiles.end() )
      {
         return false ;
      }

      return true ;
   }
}

