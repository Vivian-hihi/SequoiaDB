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


using namespace bson;

namespace engine
{
   // *****************omAuthCommand *****************************
   omAuthCommand::omAuthCommand( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession, 
                                 const CHAR *pRootPath )
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
      _rootPath    = pRootPath ;
   }

   omAuthCommand::~omAuthCommand()
   {
   }

   void omAuthCommand::_sendErrorRes2Web( INT32 rc, const CHAR* detail )
   {
      BSONObjBuilder bsonBuilder ;
      
      bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      if ( NULL != detail )
      {
         bsonBuilder.append( OM_REST_RES_DETAIL, detail ) ;
      }

      _restAdaptor->setOPResult( _restSession, rc, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
   }

   INT32 omAuthCommand::doCommand()
   {
      const CHAR *pUserName        = NULL ;
      const CHAR *pPasswd          = NULL ;
      INT32 rc                     = SDB_OK ;
      ossSocket *socket            = NULL ;
      restSessionInfo *sessionInfo = NULL ;
      BSONObjBuilder authBuilder ;
      BSONObjBuilder resBuilder ;
      BSONObj bsonRes ;
      BSONObj bsonAuth ;

      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_LOGIN_NAME, &pUserName ) ;
      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_LOGIN_PASSWD, &pPasswd ) ;

      if ( ( NULL == pUserName ) || ( NULL == pPasswd ) )
      {
         _sendErrorRes2Web( SDB_INVALIDARG, "username or passwd is null" ) ;
         goto error ;
      }

      authBuilder.append( SDB_AUTH_USER, pUserName ) ;
      authBuilder.append( SDB_AUTH_PASSWD, pPasswd ) ;
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

      if ( _restSession->isAuthOK() )
      {
         sdbGetOMManager()->releaseSessionInfo( _restSession->getSessionID() ) ;
      }
      
      sessionInfo = sdbGetOMManager()->newSessionInfo(pUserName, 
                                                      socket->getLocalIP() ) ;
      if ( NULL == sessionInfo )
      {
         PD_LOG( PDERROR, "new session failed:user=%s, ip=%u", pUserName,
                 socket->getLocalIP() ) ;
         _sendErrorRes2Web( SDB_SYS, "system error" ) ;
      }

      sessionInfo->_authOK = TRUE ;
      resBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      resBuilder.append( OM_REST_RES_LOCAL, "/"OM_REST_INDEX_HTML ) ;
      _restAdaptor->appendHttpHeader( _restSession, FIELD_NAME_SESSIONID, 
                                      sessionInfo->_id.c_str() ) ;
      _restAdaptor->setOPResult( _restSession, rc, resBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      
   done:
      return SDB_OK ;

   error:
      goto done ;
   }

   // *****************omCheckSessionCommand *****************************
   omCheckSessionCommand::omCheckSessionCommand( restAdaptor *pRestAdaptor, 
                                                 pmdRestSession *pRestSession )
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
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
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
   }

   omCreateClusterCommand::~omCreateClusterCommand()
   {
   }

   INT32 omCreateClusterCommand::_getClusterInfo( const CHAR **pClusterName, 
                                                  const CHAR **pDesc )
   {
      const CHAR *pClusterInfo = NULL ;
      BSONObj bsonClusterInfo ;
      INT32 rc                 = SDB_OK ;
      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_CLUSTER_INFO, 
                             &pClusterInfo ) ;
      if ( NULL == pClusterInfo )
      {
         rc = SDB_INVALIDARG ;
         _sendErrorRes2Web( SDB_INVALIDARG, "cluster info is null" ) ;
         goto error ;
      }

      rc = fromjson( pClusterInfo, bsonClusterInfo ) ;
      if ( rc )
      {
         _sendErrorRes2Web( rc, "change to BSONObj failed" ) ;
         goto error ;
      }

      *pClusterName = bsonClusterInfo.getStringField( 
                                         OM_BSON_FIELD_CLUSTER_NAME ) ;
      *pDesc        = bsonClusterInfo.getStringField( 
                                         OM_BSON_FIELD_CLUSTER_DESC ) ;
      if ( 0 == ossStrlen( *pClusterName ) )
      {
         rc = SDB_INVALIDARG ;
         _sendErrorRes2Web( rc, "cluster info is invalid" ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omCreateClusterCommand::doCommand()
   {
      const CHAR *pClusterName = NULL ;
      const CHAR *pDesc        = NULL ;
      BSONObjBuilder bsonBuilder ;
      BSONObj bsonCluster ;
      INT32 rc                 = SDB_OK ;

      rc = _getClusterInfo( &pClusterName, &pDesc ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      bsonBuilder.append( OM_CLUSTER_FIELD_NAME, pClusterName ) ;
      bsonBuilder.append( OM_CLUSTER_FIELD_DESC, pDesc ) ;
      // duplicate check depends on the unique index of table(OM_CS_DEPLOY_CL_CLUSTERIDX1)
      bsonCluster = bsonBuilder.obj() ;
      rc = rtnInsert( OM_CS_DEPLOY_CL_CLUSTER, bsonCluster, 1, 0, _cb );
      if ( rc )
      {
         if ( SDB_IXM_DUP_KEY == rc )
         {
            string errorInfo = string(pClusterName) + " is already exist" ;
            _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         }
         else
         {
            string errorInfo = string("failed to insert cluster:") 
                               + pClusterName ;
            PD_LOG( PDERROR, "OM: failed to insert cluster:%s", pClusterName ) ;
            _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         }

         goto error ;
      }

      bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      bsonBuilder.append( OM_BSON_FIELD_CLUSTER_NAME, pClusterName ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      
   done:
      return SDB_OK ;
   error:
      goto done ;
   }

   void omCreateClusterCommand::_sendErrorRes2Web( INT32 rc, const CHAR* detail )
   {
      BSONObjBuilder bsonBuilder ;
      
      bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      if ( NULL != detail )
      {
         bsonBuilder.append( OM_REST_RES_DETAIL, detail ) ;
      }

      _restAdaptor->setOPResult( _restSession, rc, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
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
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         string errorInfo = string( "fail to query table:" ) 
                               + OM_CS_DEPLOY_CL_CLUSTER ;
         PD_LOG( PDERROR, "fail to query table:%s, rc = %d", 
                 OM_CS_DEPLOY_CL_CLUSTER, rc ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }
      
      while ( TRUE )
      {
         BSONObjBuilder innerBuilder ;
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
            PD_LOG( PDERROR, "Failed to retreive record, rc = %d", rc ) ;
            //TODO clear pre_http_body
            _sendErrorRes2Web( rc, "Failed to retreive record" ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         innerBuilder.append( OM_BSON_FIELD_CLUSTER_NAME, 
                               result.getStringField( OM_CLUSTER_FIELD_NAME )) ;
         innerBuilder.append( OM_BSON_FIELD_CLUSTER_DESC, 
                               result.getStringField( OM_CLUSTER_FIELD_DESC )) ;
         rc = _restAdaptor->appendHttpBody( _restSession, 
                                            innerBuilder.obj().objdata(), 
                                            innerBuilder.obj().objsize(), 1 ) ;
         if ( rc )
         {
            //TODO clear pre_http_body
            _sendErrorRes2Web( rc, "Failed to appendHttpBody" ) ;
            goto error ;
         }
      }
      
      bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
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
                                         pmdRestSession *pRestSession )
                     : omCreateClusterCommand( pRestAdaptor, pRestSession )
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

   // move the exist host to the hostResult
   void omScanHostCommand::_checkHostExistence(list<BSONObj> &hostInfoList, 
                                               list<BSONObj> &hostResult )
   {
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         if ( _isHostExist( *ite ) )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            builder.append( OM_BSON_FIELD_HOST_IP, 
                            ite->getStringField( OM_BSON_FIELD_HOST_IP ) ) ;
            builder.append( OM_BSON_FIELD_HOST_NAME, 
                            ite->getStringField( OM_BSON_FIELD_HOST_NAME ) ) ;
            builder.append( OM_REST_RES_RETCODE, SDB_IXM_DUP_KEY ) ;
            tmp = builder.obj() ;
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

   INT32 omScanHostCommand::_getHostList( list<BSONObj> &hostInfo )
   {
      INT32 rc                   = SDB_OK ;
      const CHAR* pGlobalUser    = NULL ;
      const CHAR* pGlobalPasswd  = NULL ;
      const CHAR* pGlobalSshPort = NULL ;
      const CHAR* pHostInfo      = NULL ;
      BSONObj bsonHostInfo ;
      BSONElement element ;

      
      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_HOST_INFO, 
                             &pHostInfo ) ;
      if ( NULL == pHostInfo )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is null" ) ;
         _sendErrorRes2Web( rc, "hostinfo is null" ) ;
         goto error ;
      }

      rc = fromjson( pHostInfo, bsonHostInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "fromjson failed:rc=%d,json=%s", rc, pHostInfo ) ;
         _sendErrorRes2Web( rc, "change to BSONObj failed" ) ;
         goto error ;
      }

      pGlobalUser    = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_USER ) ;
      pGlobalPasswd  = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_PASSWD ) ;
      pGlobalSshPort = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_SSHPORT ) ;
      if ( 0 == ossStrlen( pGlobalUser ) || 0 == ossStrlen( pGlobalPasswd )
           || 0 == ossStrlen( pGlobalSshPort ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is invalid:info=%s", pHostInfo ) ;
         _sendErrorRes2Web( rc, "hostinfo is invalid" ) ;
         goto error ;
      }

      element = bsonHostInfo.getField( OM_REST_FIELD_HOST_INFO ) ;
      if ( element.isNull() || Array != element.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is not array type" ) ;
         _sendErrorRes2Web( rc, "hostinfo is not array type" ) ;
         goto error ;
      }

      {
         BSONObjIterator i( element.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:element=%s", 
                       ele.String().c_str() ) ;
               _sendErrorRes2Web( rc, "array's element is invalid" ) ;
               goto error ;
            }
            
            BSONObj oneHost = ele.embeddedObject() ;
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_IP ) 
                    && !oneHost.hasField( OM_BSON_FIELD_HOST_NAME ) )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "ip or hostname have not been set:element=%s",
                       ele.String().c_str() ) ;
               _sendErrorRes2Web( rc, "ip or hostname have not been set" ) ;
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

            tmp = builder.obj() ;
            hostInfo.push_back( tmp ) ;
         }
      }
      
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omScanHostCommand::doCommand()
   {
      list<BSONObj> hostInfoList ;
      INT32 rc          = SDB_OK ;
      CHAR* pContent    = NULL ;
      INT32 contentSize = 0 ;
      MsgHeader *pMsg   = NULL ;
      BSONObj bsonRequest ;
      list<BSONObj> bsonResult ;
      pmdEDUEvent eventData ;

      rc = _getHostList( hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      _checkHostExistence( hostInfoList, bsonResult ) ;
      _generateArray( hostInfoList, OM_REST_FIELD_HOST_INFO, bsonRequest ) ;

      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_SCAN_HOST_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "build message failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "build message failed" ) ;
         goto error ;
      }

      pMsg = (MsgHeader *)pContent ;
      rc = sdbGetOMManager()->sendMsgToAgent( "localhost", pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send msg to localhost's agent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "send msg to localhost's agent failed" ) ;
         goto error ;
      }

      //TODO 1 receive agent's response
      //     2 check response return code
      //     3 send response to web
      return SDB_OK ;

   done:
      if ( NULL != pContent )
      {
         SDB_OSS_DEL pContent ;
      }
      
      return rc; 
   error:
      goto done ;
   }

   // *****************omCheckHostCommand *****************************
   omCheckHostCommand::omCheckHostCommand( restAdaptor *pRestAdaptor, 
                                           pmdRestSession *pRestSession )
                      : omScanHostCommand( pRestAdaptor, pRestSession )
   {
   }

   omCheckHostCommand::~omCheckHostCommand()
   {
   }

   INT32 omCheckHostCommand::_getHostList( string &clusterName, 
                                           list<BSONObj> &hostInfo )
   {
      INT32 rc                   = SDB_OK ;
      const CHAR* pGlobalUser    = NULL ;
      const CHAR* pGlobalPasswd  = NULL ;
      const CHAR* pGlobalSshPort = NULL ;
      const CHAR* pHostInfo      = NULL ;
      BSONObj bsonHostInfo ;
      BSONElement element ;

      
      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_HOST_INFO, 
                             &pHostInfo ) ;
      if ( NULL == pHostInfo )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is null" ) ;
         _sendErrorRes2Web( rc, "hostinfo is null" ) ;
         goto error ;
      }

      rc = fromjson( pHostInfo, bsonHostInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "fromjson failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "change to BSONObj failed" ) ;
         goto error ;
      }

      pGlobalUser    = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_USER ) ;
      pGlobalPasswd  = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_PASSWD ) ;
      pGlobalSshPort = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_SSHPORT ) ;
      clusterName    = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_CLUSTER_NAME ) ;
      if ( 0 == ossStrlen( pGlobalUser ) || 0 == ossStrlen( pGlobalPasswd )
           || 0 == ossStrlen( pGlobalSshPort ) || clusterName.length() == 0 )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is invalid:info=%s", pHostInfo ) ;
         _sendErrorRes2Web( rc, "hostinfo is invalid" ) ;
         goto error ;
      }

      element = bsonHostInfo.getField( OM_REST_FIELD_HOST_INFO ) ;
      if ( element.isNull() || Array != element.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is not array type:type=%d", 
                 element.type() ) ;
         _sendErrorRes2Web( rc, "hostinfo is not array type" ) ;
         goto error ;
      }

      {
         BSONObjIterator i( element.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:type=%d", 
                       ele.type() ) ;
               _sendErrorRes2Web( rc, "array's element is invalid" ) ;
               goto error ;
            }
            
            BSONObj oneHost = ele.embeddedObject() ;
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_IP ) 
                    || !oneHost.hasField( OM_BSON_FIELD_HOST_NAME ) )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "ip or hostname have not been set" ) ;
               _sendErrorRes2Web( rc, "ip or hostname have not been set" ) ;
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

            tmp = builder.obj() ;
            hostInfo.push_back( tmp ) ;
         }
      }
      
   done:
      return rc ;
   error:
      goto done ;
   }

   

   void omCheckHostCommand::_sendOkRes2Web( list<BSONObj> &hostResult )
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
            PD_LOG(PDERROR, "fail to appendHttpBody:rc=%d", rc ) ;
            _sendErrorRes2Web( rc, "Failed to appendHttpBody" ) ;
            goto error ;
         }
      }

      bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

   done:
      return ;
   error:
      goto done ;
   }

   // check ping and ssh
   INT32 omCheckHostCommand::_doBasicCheck( list<BSONObj> &hostInfoList, 
                                            list<BSONObj> &hostResult )
   {
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      BSONObj bsonRequest ;
      MsgHeader *pMsg   = NULL ;

      _generateArray(hostInfoList, OM_REST_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_BASIC_CHECK_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "build query msg failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "build message failed" ) ;
         goto error ;
      }

      pMsg = (MsgHeader *)pContent ;
      rc = sdbGetOMManager()->sendMsgToAgent( "localhost", pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "send msg to agent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "send msg to agent failed" ) ;
         goto error ;
      }

      //TODO: get response

   done:
      return rc ;
   error:
      goto done ;
   }

   // check os/cpu/network etc (and get those infomations )
   INT32 omCheckHostCommand::_doCheck( list<BSONObj> &hostInfoList, 
                                       list<BSONObj> &hostResult )
   {
      // send agent to remote host
      // send check command to remote host independent
      // receive response
      // move all to hostResult
      INT32 rc          = SDB_OK ;
      CHAR *pContent    = NULL ;
      INT32 contentSize = 0 ;
      BSONObj bsonRequest ;
      MsgHeader *pMsg   = NULL ;
      list<BSONObj>::iterator ite ;

      _generateArray(hostInfoList, OM_REST_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_INSTALL_REMOTE_AGENT, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "build query msg failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "build message failed" ) ;
         goto error ;
      }

      pMsg = (MsgHeader *)pContent ;
      rc = sdbGetOMManager()->sendMsgToAgent( "localhost", pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "send msg to agent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "send msg to agent failed" ) ;
         goto error ;
      }
      //TODO: receive agent's res

      ite = hostInfoList.begin() ;
      while ( ite != hostInfoList.end() )
      {
         rc = SDB_OK ;
      }

   done:
      if ( NULL != pContent )
      {
         SDB_OSS_FREE( pContent ) ;
      }
      
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
         PD_LOG(PDERROR, "fail to get host list:rc=%d", rc ) ;
         goto error ;
      }

      // move the exist host to hostResult
      _checkHostExistence( hostInfoList, hostResult ) ;

      // move the check failed host to the hostResult
      rc = _doBasicCheck( hostInfoList, hostResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "do basic check failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = _doCheck( hostInfoList, hostResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "do check failed:rc=%d", rc ) ;
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
                                       pmdRestSession *pRestSession )
                       :omCreateClusterCommand( pRestAdaptor, pRestSession )
   {
   }
   
   omAddHostCommand::~omAddHostCommand()
   {
   }

   INT32 omAddHostCommand::_getHostDetialList( string &clusterName, 
                                               list<BSONObj> &hostInfo )
   {
      INT32 rc                   = SDB_OK ;
      const CHAR* pGlobalUser    = NULL ;
      const CHAR* pGlobalPasswd  = NULL ;
      const CHAR* pGlobalSshPort = NULL ;
      const CHAR* pHostInfo      = NULL ;
      BSONObj bsonHostInfo ;
      BSONElement element ;

      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_HOST_INFO, 
                             &pHostInfo ) ;
      if ( NULL == pHostInfo )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is null" ) ;
         _sendErrorRes2Web( rc, "hostinfo is null" ) ;
         goto error ;
      }

      rc = fromjson( pHostInfo, bsonHostInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "fromjson failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "change to BSONObj failed" ) ;
         goto error ;
      }

      pGlobalUser    = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_USER ) ;
      pGlobalPasswd  = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_PASSWD ) ;
      pGlobalSshPort = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_SSHPORT ) ;
      clusterName    = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_CLUSTER_NAME ) ;
      if ( 0 == ossStrlen( pGlobalUser ) || 0 == ossStrlen( pGlobalPasswd )
           || 0 == ossStrlen( pGlobalSshPort ) || clusterName.length() == 0 )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is invalid:info=%s", pHostInfo ) ;
         _sendErrorRes2Web( rc, "hostinfo is invalid" ) ;
         goto error ;
      }

      element = bsonHostInfo.getField( OM_REST_FIELD_HOST_INFO ) ;
      if ( element.isNull() || Array != element.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "hostinfo is not array type:type=%d", 
                 element.type() ) ;
         _sendErrorRes2Web( rc, "hostinfo is not array type" ) ;
         goto error ;
      }

      {
         BSONObjIterator i( element.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:type=%d", 
                       ele.type() ) ;
               _sendErrorRes2Web( rc, "array's element is invalid" ) ;
               goto error ;
            }
            
            BSONObj oneHost = ele.embeddedObject() ;
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_IP ) 
                    || !oneHost.hasField( OM_BSON_FIELD_HOST_NAME ) )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "ip or hostname have not been set" ) ;
               _sendErrorRes2Web( rc, "ip or hostname have not been set" ) ;
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

            tmp = builder.obj() ;
            hostInfo.push_back( tmp ) ;
         }
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
      INT32 rc = SDB_OK ;

      rc = _getHostDetialList( clusterName, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }



   // *****************omGetFileCommand *****************************
   omGetFileCommand::omGetFileCommand( restAdaptor *pRestAdaptor, 
                                       pmdRestSession *pRestSession, 
                                       const CHAR *pRootPath,
                                       const CHAR *pSubPath)
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
      transfer->getTransferedPath(_subPath.c_str(), realSubPath) ;
      
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
         {"/",    "/"OM_REST_INDEX_HTML}
      } ;

      // only html file, other file is all public now(from jiawen)
      static char *fileAuthorityPublic[] = {
         "/"OM_REST_LOGIN_HTML ,
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

