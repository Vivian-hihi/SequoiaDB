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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>


using namespace bson;
using namespace boost::property_tree;

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

   INT32 omCreateClusterCommand::_getClusterInfo( string &clusterName, 
                                                  string &desc )
   {
      const CHAR *pClusterInfo = NULL ;
      BSONObj bsonClusterInfo ;
      INT32 rc                 = SDB_OK ;
      _restAdaptor->getQuery(_restSession, OM_REST_CLUSTER_INFO, 
                             &pClusterInfo ) ;
      if ( NULL == pClusterInfo )
      {
         string errorInfo = string( OM_REST_CLUSTER_INFO ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( SDB_INVALIDARG, errorInfo.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pClusterInfo, bsonClusterInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "change to BSONObj failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "change to BSONObj failed" ) ;
         goto error ;
      }

      clusterName = bsonClusterInfo.getStringField( 
                                         OM_BSON_FIELD_CLUSTER_NAME ) ;
      desc        = bsonClusterInfo.getStringField( 
                                         OM_BSON_FIELD_CLUSTER_DESC ) ;
      if ( 0 == clusterName.length() )
      {
         string errorInfo = string( OM_BSON_FIELD_CLUSTER_NAME ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
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
      BSONObjBuilder bsonBuilder ;
      BSONObjBuilder resBuilder ;
      BSONObj bsonCluster ;
      INT32 rc                 = SDB_OK ;

      rc = _getClusterInfo( clusterName, desc ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      bsonBuilder.append( OM_CLUSTER_FIELD_NAME, clusterName.c_str() ) ;
      bsonBuilder.append( OM_CLUSTER_FIELD_DESC, desc.c_str() ) ;
      // duplicate check depends on the unique index of table(OM_CS_DEPLOY_CL_CLUSTERIDX1)
      bsonCluster = bsonBuilder.obj() ;
      rc = rtnInsert( OM_CS_DEPLOY_CL_CLUSTER, bsonCluster, 1, 0, _cb );
      if ( rc )
      {
         if ( SDB_IXM_DUP_KEY == rc )
         {
            string errorInfo = clusterName + " is already exist" ;
            _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         }
         else
         {
            string errorInfo = string("failed to insert cluster:") 
                               + clusterName ;
            PD_LOG( PDERROR, "OM: failed to insert cluster:%s=%s,rc=%d", 
                    OM_CLUSTER_FIELD_NAME, clusterName.c_str(), rc ) ;
            _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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

   void omCreateClusterCommand::_sendErrorRes2Web( INT32 rc, const CHAR* detail )
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj tmp ;
      
      bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      if ( NULL != detail )
      {
         bsonBuilder.append( OM_REST_RES_DETAIL, detail ) ;
      }

      tmp = bsonBuilder.obj() ;
      _restAdaptor->setOPResult( _restSession, rc, tmp ) ;
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
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         string errorInfo = string( "fail to query table:" ) 
                               + OM_CS_DEPLOY_CL_CLUSTER ;
         PD_LOG( PDERROR, "fail to query table:%s, rc=%d", 
                 OM_CS_DEPLOY_CL_CLUSTER, rc ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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
            PD_LOG( PDERROR, "Failed to retreive record, rc=%d", rc ) ;
            //TODO clear pre_http_body
            _sendErrorRes2Web( rc, "Failed to retreive record" ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         innerBuilder.append( OM_BSON_FIELD_CLUSTER_NAME, 
                              result.getStringField( OM_CLUSTER_FIELD_NAME )) ;
         innerBuilder.append( OM_BSON_FIELD_CLUSTER_DESC, 
                              result.getStringField( OM_CLUSTER_FIELD_DESC )) ;
         tmp = innerBuilder.obj() ;
         rc = _restAdaptor->appendHttpBody( _restSession, tmp.objdata(), 
                                            tmp.objsize(), 1 ) ;
         if ( rc )
         {
            //TODO clear pre_http_body
            PD_LOG( PDERROR, "faile to appendHttpBody:rc=%d", rc ) ;
            _sendErrorRes2Web( rc, "Failed to appendHttpBody" ) ;
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

      
      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_HOST_INFO, 
                             &pHostInfo ) ;
      if ( NULL == pHostInfo )
      {
         string errorInfo = string( OM_REST_FIELD_HOST_INFO ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pHostInfo, bsonHostInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "change to BSONObj failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "change to BSONObj failed" ) ;
         goto error ;
      }

      pGlobalUser      = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_USER ) ;
      pGlobalPasswd    = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_PASSWD ) ;
      pGlobalSshPort   = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_SSHPORT ) ;
      pGlobalAgentPort = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_AGENTPORT ) ;
      clusterName    = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_CLUSTER_NAME ) ;
      if ( 0 == ossStrlen( pGlobalUser ) || 0 == ossStrlen( pGlobalPasswd )
           || 0 == ossStrlen( pGlobalSshPort ) || 0 == clusterName.length()
           || 0 == ossStrlen( pGlobalAgentPort ) )
      {
         string errorInfo = string( OM_REST_FIELD_HOST_INFO )
                            + " is invalid" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      element = bsonHostInfo.getField( OM_BSON_FIELD_HOST_INFO ) ;
      if ( element.isNull() || Array != element.type() )
      {
         string errorInfo = string( OM_BSON_FIELD_HOST_INFO )
                            + " is not array type" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s is not array type:type=%d", 
                 OM_BSON_FIELD_HOST_INFO, element.type() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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
               string errorInfo = string(OM_BSON_FIELD_HOST_IP) + " or " 
                                  + OM_BSON_FIELD_HOST_NAME 
                                  + "have not been set" ;
               PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
               _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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
            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_AGENTPORT ) )
            {
               builder.append( OM_BSON_FIELD_HOST_AGENTPORT, pGlobalAgentPort) ;
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

   INT32 omScanHostCommand::_sendMsgToLocalAgent( omManager *om,
                                             pmdRemoteSession *remoteSession, 
                                             MsgHeader *pMsg )
   {
      MsgRouteID localAgentID ;
      INT32 rc = SDB_OK ;

      localAgentID = om->updateAgentInfo( _localAgentHost.c_str(), 
                                          _localAgentService.c_str() ) ;
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

      rc = remoteSession->waitReply( TRUE, &subSessionVec, -1 ) ;
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
         PD_LOG( PDERROR, "receive error response:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "receive error response" ) ;
         goto error ;
      }

      rc = response.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "agent process scan command failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "agent process scan command failed" ) ;
         goto error ;
      }

      hostElement = response.getField( OM_BSON_FIELD_HOST_INFO ) ;
      if ( hostElement.isNull() || Array != hostElement.type() )
      {
         string errorInfo = string( OM_BSON_FIELD_HOST_INFO ) 
                            + " is not array type" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }
      {
         BSONObjIterator i( hostElement.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:element=%s", 
                       ele.toString().c_str() ) ;
               _sendErrorRes2Web( rc, "array's element is invalid" ) ;
               goto error ;
            }
            
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
         goto error ;
      }

      _checkHostExistence( hostInfoList, bsonResult ) ;
      if ( hostInfoList.size() == 0 )
      {
         _sendOkRes2Web( bsonResult ) ;
         goto done ;
      }

      // build request to agent
      _generateArray( hostInfoList, OM_REST_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_SCAN_HOST_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "build message failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "build message failed" ) ;
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
         PD_LOG( PDERROR, "addSession failed" ) ;
         _sendErrorRes2Web( SDB_OOM, "addSession failed" ) ;
         goto error ;
      }

      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed" ) ;
         _sendErrorRes2Web( rc, "send message to agent failed" ) ;
         goto error ;
      }

      rc = _receiveFromAgent( remoteSession, bsonResponse ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "receiveFromAgent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "receiveFromAgent failed" ) ;
         goto error ;
      }

      rc = _parseResonpse( subSessionVec, bsonResponse, bsonResult ) ;
      if ( SDB_OK != rc )
      {
         // if error, _sendErrorRes2Web is called in _parseResonpse
         PD_LOG( PDERROR, "_parseResonpse failed:rc=%d", rc ) ;
         goto error ;
      }

      _sendOkRes2Web( bsonResult ) ;

   done:
      if ( NULL != pContent )
      {
         SDB_OSS_FREE( pContent ) ;
      }

      if ( NULL != remoteSession )
      {
         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( _cb->getTID() ) ;
      }
      
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

   void omCheckHostCommand::_eraseFromList( list<BSONObj> &hostInfoList, 
                                            BSONObj &oneHost )
   {
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
      string hostName = oneHost.getStringField( OM_BSON_FIELD_HOST_NAME ) ;
      while ( ite != hostInfoList.end() )
      {
         string tmpHostName = ite->getStringField( OM_BSON_FIELD_HOST_NAME ) ;
         if ( tmpHostName.compare( hostName ) == 0 )
         {
            hostInfoList.erase( ite ) ;
            return ;
         }
         ite++ ;
      }
      
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
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_BASIC_CHECK_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "build query msg failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "build message failed" ) ;
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
         _sendErrorRes2Web( rc, "addSession failed" ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "send message to agent failed" ) ;
         goto error ;
      }

      // receiving for agent's response
      rc = _receiveFromAgent( remoteSession, result ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "receive from agent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "receive from agent failed" ) ;
         goto error ;
      }

      rcElement = result.getField( OM_REST_RES_RETCODE ) ;
      if ( rcElement.eoo() || NumberInt != rcElement.type())
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive unexpected response:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "receive unexpected response" ) ;
         goto error ;
      }

      rc = result.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         string errorInfo = string("agent process ") + OM_BASIC_CHECK_REQ
                            + " failed" ;
         PD_LOG( PDERROR, "agent process %s failed:rc=%d", 
                 OM_BASIC_CHECK_REQ, rc ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      hostElement = result.getField( OM_BSON_FIELD_HOST_INFO ) ;
      if ( hostElement.isNull() || Array != hostElement.type() )
      {
         string errorInfo = string(OM_BSON_FIELD_HOST_INFO) 
                            + " is not array type" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }
      {
         BSONObjIterator i( hostElement.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONObjBuilder builder ;
            BSONObj tmp ;
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:element=%s", 
                       ele.toString().c_str() ) ;
               _sendErrorRes2Web( rc, "array's element is invalid" ) ;
               goto error ;
            }
            
            BSONObj oneHost = ele.embeddedObject() ;
            rcElement = oneHost.getField( OM_REST_RES_RETCODE ) ;
            if ( rcElement.eoo() || NumberInt != rcElement.type())
            {
               string errorInfo = string( "receive unexpected response:host=" )
                                  + oneHost.toString() ;
               rc = SDB_UNEXPECTED_RESULT ;
               PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
               _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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
      if ( NULL != pContent )
      {
         SDB_OSS_FREE( pContent ) ;
      }

      if ( NULL != remoteSession )
      {
         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( _cb->getTID() ) ;
      }

      return rc ;
   error:
      goto done ;
   }

   INT32 omCheckHostCommand::_installAgent( list<BSONObj> &hostInfoList )
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

      _generateArray( hostInfoList, OM_BSON_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_INSTALL_REMOTE_AGENT, 
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
                                                      OM_WAIT_SCAN_RES_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
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

   done:
      if ( NULL != pContent )
      {
         SDB_OSS_FREE( pContent ) ;
      }

      if ( NULL != remoteSession )
      {
         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( _cb->getTID() ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   // create the requet for all the hosts in hostInfoList
   INT32 omCheckHostCommand::_addCheckHostReq( omManager *om,
                                               pmdRemoteSession *remoteSession,
                                               list<BSONObj> &hostInfoList,
                                               list<BSONObj> &hostResult ) 
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
         agentPort = ite->getStringField( OM_BSON_FIELD_HOST_AGENTPORT ) ;
         routeID   = om->updateAgentInfo( agentHost.c_str(), 
                                          agentPort.c_str() ) ;
         subSession = remoteSession->addSubSession( routeID.value ) ;
         if ( NULL == subSession )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "addSubSessin failed" ) ;
            goto error ;
         }

         reqBuilder.append(OM_BSON_FIELD_HOST_USER, 
                           ite->getStringField( OM_BSON_FIELD_HOST_USER ) ) ;
         reqBuilder.append(OM_BSON_FIELD_HOST_PASSWD, 
                           ite->getStringField( OM_BSON_FIELD_HOST_PASSWD ) ) ;
         bsonRequest = reqBuilder.obj() ;
         rc = msgBuildQueryMsg( &pContent, &contentSize, OM_CHECK_REMOTE_HOST, 
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

   INT32 omCheckHostCommand::_checkHostEnv( list<BSONObj> &hostInfoList, 
                                            list<BSONObj> &hostResult )
   {
      INT32 rc          = SDB_OK ;
      omManager *om     = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      VEC_SUB_SESSIONPTR subSessionVec ;
      list<BSONObj>::iterator ite ;
      
      // create remote session
      om            = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb, 
                                                      OM_WAIT_SCAN_RES_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         goto error ;
      }
      rc = _addCheckHostReq( om, remoteSession, hostInfoList, hostResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "addCheckHostReq failed:rc=%d", rc ) ;
         goto done ;
      }

      remoteSession->sendMsg() ;
      rc = remoteSession->waitReply( true, &subSessionVec, -1 ) ;
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
         hostResult.push_back( result ) ;
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
         builder.append( OM_REST_RES_RETCODE, SDB_TIMEOUT ) ;

         tmp = builder.obj() ;
         hostResult.push_back( tmp ) ;
         hostInfoList.erase( ite++ ) ;
      }

   done:
      if ( NULL != remoteSession )
      {
         MAP_SUB_SESSION *subSessions = remoteSession->getSubSessions() ;
         if ( NULL != subSessions )
         {
            MAP_SUB_SESSION_IT it = subSessions->begin() ;
            while ( it != subSessions->end() )
            {
               MsgHeader *pMsg = it->second.getReqMsg() ;
               if ( NULL != pMsg )
               {
                  SDB_OSS_FREE( pMsg ) ;
               }

               it++ ;
            }
         }

         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( _cb->getTID() ) ;
      }

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
      
      if ( hostInfoList.size() == 0 )
      {
         goto done ;
      }
      
      _generateArray( hostInfoList, OM_REST_FIELD_HOST_INFO, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_UNINSTALL_REMOTE_AGENT, 
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
                                                      OM_WAIT_SCAN_RES_INTERVAL,
                                                      NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      if ( NULL != pContent )
      {
         SDB_OSS_FREE( pContent ) ;
      }

      if ( NULL != remoteSession )
      {
         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( _cb->getTID() ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   // check os/cpu/network etc (and get those infomations )
   INT32 omCheckHostCommand::_doCheck( list<BSONObj> &hostInfoList, 
                                       list<BSONObj> &hostResult )
   {
      INT32 rc = SDB_OK ;
      list<BSONObj> agentInstalledHost ;

      rc = _installAgent( hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "install agent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "install agent failed" ) ;
         goto error ;
      }

      agentInstalledHost.assign( hostInfoList.begin(), hostInfoList.end() ) ;

      rc = _checkHostEnv( hostInfoList, hostResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG(PDERROR, "check host env failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "check host env failed" ) ;
         goto error ;
      }

   done:
      _uninstallAgent( agentInstalledHost ) ;
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
      const CHAR* pInstallPath     = NULL ;
      BSONObj bsonHostInfo ;
      BSONElement element ;

      
      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_HOST_INFO, 
                             &pHostInfo ) ;
      if ( NULL == pHostInfo )
      {
         string errorInfo = "rest field:" + 
                            string( OM_REST_FIELD_HOST_INFO ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pHostInfo, bsonHostInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "fromjson failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "change to BSONObj failed" ) ;
         goto error ;
      }

      pGlobalUser      = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_USER ) ;
      pGlobalPasswd    = bsonHostInfo.getStringField( OM_BSON_FIELD_HOST_PASSWD ) ;
      pGlobalSshPort   = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_SSHPORT ) ;
      pGlobalAgentPort = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_HOST_AGENTPORT ) ;
      clusterName    = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_CLUSTER_NAME ) ;
      pInstallPath   = bsonHostInfo.getStringField( 
                                       OM_BSON_FIELD_INSTALL_PATH ) ;
      if ( 0 == ossStrlen( pGlobalUser ) || 0 == ossStrlen( pGlobalPasswd )
           || 0 == ossStrlen( pGlobalSshPort ) || 0 == clusterName.length()
           || 0 == ossStrlen( pGlobalAgentPort ) 
           || 0 == ossStrlen( pInstallPath) )
      {
         string errorInfo = string( OM_REST_FIELD_HOST_INFO )
                            + " is invalid" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s is invalid:info=%s", OM_REST_FIELD_HOST_INFO, 
                 pHostInfo ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      element = bsonHostInfo.getField( OM_BSON_FIELD_HOST_INFO ) ;
      if ( element.isNull() || Array != element.type() )
      {
         string errorInfo = string( OM_BSON_FIELD_HOST_INFO )
                            + " is not array type" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s is not array type:type=%d", 
                 OM_BSON_FIELD_HOST_INFO, element.type() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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
               string errorInfo = string(OM_BSON_FIELD_HOST_IP) + " or " 
                                  + OM_BSON_FIELD_HOST_NAME 
                                  + "have not been set" ;
               PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
               _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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

            if ( !oneHost.hasField( OM_BSON_FIELD_HOST_AGENTPORT ) )
            {
               builder.append( OM_BSON_FIELD_HOST_AGENTPORT, pGlobalAgentPort) ;
            }

            if ( !oneHost.hasField( OM_BSON_FIELD_INSTALL_PATH ) )
            {
               builder.append( OM_BSON_FIELD_INSTALL_PATH, pInstallPath ) ;
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

   void omAddHostCommand::_generateAddHostReq( list<BSONObj> &hostInfoList, 
                                               BSONObj &bsonRequest )
   {
      BSONObjBuilder builder ;
      BSONArrayBuilder arrayBuilder ;
      
      list<BSONObj>::iterator ite = hostInfoList.begin() ;
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
         innerBuilder.append( OM_BSON_FIELD_HOST_AGENTPORT, 
                         ite->getStringField( OM_BSON_FIELD_HOST_AGENTPORT ) ) ;
         innerBuilder.append( OM_BSON_FIELD_HOST_AGENTPORT, 
                         ite->getStringField( OM_BSON_FIELD_HOST_AGENTPORT ) ) ;
         innerBuilder.append( OM_BSON_FIELD_INSTALL_PATH, 
                         ite->getStringField( OM_BSON_FIELD_INSTALL_PATH ) ) ;
         temp = innerBuilder.obj() ;
         arrayBuilder.append( temp ) ;
         ite++ ;
      }
      
      builder.appendArray( OM_REST_FIELD_HOST_INFO, arrayBuilder.arr() );
      bsonRequest = builder.obj() ;

      return ;
   }
   
   INT32 omAddHostCommand::_addHost( list<BSONObj> &hostInfoList, 
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

      _generateAddHostReq( hostInfoList, bsonRequest ) ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_ADD_HOST_REQ, 
                             0, 0, 0, -1, &bsonRequest, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "build query msg failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "build message failed" ) ;
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
         _sendErrorRes2Web( rc, "addSession failed" ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "send message to agent failed" ) ;
         goto error ;
      }

      // receiving for agent's response
      rc = _receiveFromAgent( remoteSession, result ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "receive from agent failed:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "receive from agent failed" ) ;
         goto error ;
      }

      element = result.getField( OM_REST_RES_RETCODE ) ;
      if ( element.eoo() || NumberInt != element.type())
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive unexpected response:rc=%d", rc ) ;
         _sendErrorRes2Web( rc, "receive unexpected response" ) ;
         goto error ;
      }

      rc = result.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         string errorInfo = string("agent process ") + OM_BASIC_CHECK_REQ
                            + " failed" ;
         PD_LOG( PDERROR, "agent process %s failed:rc=%d", 
                 OM_BASIC_CHECK_REQ, rc ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      element = result.getField( OM_BSON_FIELD_TRANSACTION_ID ) ;
      if ( element.eoo() || NumberInt != element.type())
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive unexpected response:type=%d", 
                 element.type() ) ;
         _sendErrorRes2Web( rc, "receive unexpected response" ) ;
         goto error ;
      }
      
      transationID = result.getIntField( OM_REST_RES_RETCODE ) ;
      
   done:
      if ( NULL != pContent )
      {
         SDB_OSS_FREE( pContent ) ;
      }

      if ( NULL != remoteSession )
      {
         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( _cb->getTID() ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   void omAddHostCommand::_generateTableField( BSONObjBuilder &builder, 
                                               string newFieldName,
                                               BSONObj &bsonOld,
                                               string oldFiledName ) 
   {  
      BSONElement element = bsonOld.getField( OM_BSON_FIELD_HOST_NAME ) ;
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

         tmp = builder.obj() ;
         rc = rtnInsert( OM_CS_DEPLOY_CL_HOST, tmp, 1, 0, _cb );
         {
            if ( rc )
            {
                  string errorInfo = string("failed to insert table[") 
                                     + OM_CS_DEPLOY_CL_HOST + "]";
                  PD_LOG( PDERROR, "%s:rc=%d", errorInfo.c_str(), rc ) ;
                  _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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
         rtnDelete( OM_CS_DEPLOY_CL_HOST, tmp, BSONObj(), 0, _cb );
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
                             OM_ROLLBACK_TRANSACTION_REQ, 
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
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      if ( NULL != pContent )
      {
         SDB_OSS_FREE( pContent ) ;
      }

      if ( NULL != remoteSession )
      {
         remoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( _cb->getTID() ) ;
      }
      return ;
   error:
      goto done ;
   }

   INT32 omAddHostCommand::doCommand()
   {
      list<BSONObj> hostInfoList ;
      string clusterName ;
      INT32 transactionID = -1 ;
      INT32 rc = SDB_OK ;

      rc = _getHostList( clusterName, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "fail to get host list:rc=%d", rc ) ;
         goto error ;
      }

      rc = _addHost( hostInfoList, transactionID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "fail to get host list:rc=%d", rc ) ;
         goto error ;
      }

      rc = _storeHostInfo( clusterName, hostInfoList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "fail to store host:rc=%d", rc ) ;
         _transactionRollBack( _localAgentHost, _localAgentService, 
                               transactionID ) ;
         goto error ;
      }

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

   INT32 omQueryHostCommand::doCommand()
   {
      INT32 rc                     = SDB_OK ;
      const CHAR* pClusterName     = NULL ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObjBuilder builder ;
      SINT64 contextID             = -1 ;

      BSONObjBuilder resultBuilder ;
      BSONObj result ;

      _restAdaptor->getQuery(_restSession, OM_BSON_FIELD_CLUSTER_NAME, 
                             &pClusterName ) ;
      if ( NULL == pClusterName )
      {
         string errorInfo = "rest field:" + 
                            string( OM_BSON_FIELD_CLUSTER_NAME ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      builder.append( OM_HOST_FIELD_CLUSTERNAME, pClusterName ) ;
      matcher = builder.obj() ;

      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         string errorInfo = string( "fail to query table:" ) 
                               + OM_CS_DEPLOY_CL_HOST ;
         PD_LOG( PDERROR, "fail to query table:%s, rc = %d", 
                 OM_CS_DEPLOY_CL_HOST, rc ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
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
            PD_LOG( PDERROR, "Failed to retreive record, rc = %d", rc ) ;
            //TODO clear pre_http_body
            _sendErrorRes2Web( rc, "Failed to retreive record" ) ;
            goto error ;
         }

         BSONObj record( buffObj.data() ) ;
         //TODO change keys
         rc = _restAdaptor->appendHttpBody( _restSession, record.objdata(), 
                                            record.objsize(), 1 ) ;
         if ( rc )
         {
            //TODO clear pre_http_body
            _sendErrorRes2Web( rc, "Failed to appendHttpBody" ) ;
            goto error ;
         }
      }

      resultBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      result = resultBuilder.obj() ;
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

   // *****************omQueryBusinessCommand *****************************
   omQueryBusinessCommand::omQueryBusinessCommand( restAdaptor *pRestAdaptor, 
                                                pmdRestSession *pRestSession, 
                                                const CHAR *pRootPath, 
                                                const CHAR *pSubPath )
                          :omCreateClusterCommand( pRestAdaptor, pRestSession ),
                           _rootPath( pRootPath ), _subPath( pSubPath )
   {
   }

   omQueryBusinessCommand::~omQueryBusinessCommand()
   {
   }

   INT32 omQueryBusinessCommand::doCommand()
   {
      INT32 rc             = SDB_OK ;
      string businessFile  = _rootPath + "/" + OM_BUSINESS_CONFIG_SUBDIR
                             + "/" + OM_BUSINESS_FILE_NAME ;
      BSONObjBuilder opBuilder ;
      BSONArrayBuilder arrayBuilder ;

      BSONObjBuilder businessBuilder ;
      BSONObj bsonBusiness ;
      ptree pt ;
      try
      {
         read_xml( businessFile.c_str(), pt ) ;
         ptree ptList = pt.get_child( OM_XML_BUSINESS_LIST ) ;
         ptree::iterator ite = ptList.begin() ;
         for( ; ite != ptList.end() ; ite++ )
         {
            BSONObjBuilder builder ;
            BSONObj temp ;
            ptree business = ite->second ;
            string name = business.get<string>( OM_XMLATTR_BUSINESS_NAME ) ;
            string desc = business.get<string>( OM_XMLATTR_BUSINESS_DESC ) ;
            builder.append( OM_BSON_BUSINESS_TYPE, name ) ;
            builder.append( OM_BSON_BUSINESS_DESC, desc ) ;
            temp = builder.obj() ;
            arrayBuilder.append( temp ) ;
         }
      }
      catch( std::exception &e )
      {
         rc = SDB_DMS_RECORD_NOTEXIST ;
         _sendErrorRes2Web( SDB_DMS_RECORD_NOTEXIST, e.what() ) ;
         goto error ;
      }

      businessBuilder.appendArray( OM_BSON_BUSINESS_LIST, arrayBuilder.arr() );
      bsonBusiness = businessBuilder.obj() ;
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
                                  :omQueryBusinessCommand( pRestAdaptor, 
                                                           pRestSession, 
                                                           pRootPath,
                                                           pSubPath)
   {
   }

   omQueryBusinessTemplateCommand::~omQueryBusinessTemplateCommand()
   {
   }

   INT32 omQueryBusinessTemplateCommand::doCommand()
   {
      INT32 rc                  = SDB_OK ;
      const CHAR* pBusinessType = NULL ;
      string templateFile       = "" ;
      BSONObjBuilder opBuilder ;

      _restAdaptor->getQuery(_restSession, OM_REST_BUSINESS_TYPE, 
                             &pBusinessType ) ;
      if ( NULL == pBusinessType )
      {
         string errorInfo = "rest field:" + 
                            string( OM_REST_BUSINESS_TYPE ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      templateFile = _rootPath + "/" + OM_BUSINESS_CONFIG_SUBDIR + "/" 
                     + pBusinessType + OM_TEMPLATE_FILE_NAME ;
      try
      {
         ptree pt ;
         read_xml( templateFile.c_str(), pt ) ;
         ptree ptList = pt.get_child( OM_XML_CLUSTER_TYPE_LIST ) ;
         ptree::iterator ite = ptList.begin() ;
         for( ; ite != ptList.end() ; ite++ )
         {
            BSONObjBuilder oneClusterTypeBulider ;
            BSONObj oneClusterType ;
            BSONArrayBuilder arrayBuilder ;
            ptree clusterType = ite->second ;
            string name = clusterType.get<string>( OM_XMLATTR_PROPERTY_NAME ) ;
            oneClusterTypeBulider.append( OM_BSON_CLUSTER_TYPE, name ) ;
            oneClusterTypeBulider.append( OM_BSON_BUSINESS_TYPE, 
                                          pBusinessType ) ;
            ptree::iterator propertyIte = clusterType.begin() ;
            /* xml sample:
            <cluster_type_list>
               <cluster_type name="standalone">
                  <property name="replica_num"    type="int" />
                  <property name="data_group_num"    type="int" />
               </cluster_type>
            </cluster_type_list>
            */

            /* in this case, @clusterType have 3 elements:
               1.  <cluster_type name="standalone">
               2.  <property name="replica_num"    type="int" />
               3.  <property name="data_group_num"    type="int" />

               and the @propertyIte point to the first element. we just want to 
               read the "property", so we ignore the first element to read 
               the next elements
            */
            propertyIte++ ;
            for( ; propertyIte != clusterType.end() ; propertyIte++ )
            {
               BSONObjBuilder builder ;
               BSONObj temp ;
               string name         = "" ;
               string type         = "" ;
               string defaultValue = "" ;
               string valid        = "" ;
               string display      = "" ;
               string edit         = "" ;
               string desc         = "" ;
               string level        = "" ;
               name         = propertyIte->second.get<string>( 
                                                OM_XMLATTR_PROPERTY_NAME ) ;
               type         = propertyIte->second.get<string>( 
                                                OM_XMLATTR_PROPERTY_TYPE ) ;
               defaultValue = propertyIte->second.get<string>( 
                                                OM_XMLATTR_PROPERTY_DEFAULT ) ;
               valid        = propertyIte->second.get<string>( 
                                                OM_XMLATTR_PROPERTY_VALID ) ;
               display      = propertyIte->second.get<string>( 
                                                OM_XMLATTR_PROPERTY_DISPLAY ) ;
               edit         = propertyIte->second.get<string>( 
                                                OM_XMLATTR_PROPERTY_EDIT ) ;
               desc         = propertyIte->second.get<string>( 
                                                OM_XMLATTR_PROPERTY_DESC ) ;
               level        = propertyIte->second.get<string>( 
                                                OM_XMLATTR_PROPERTY_LEVEL ) ;
               builder.append( OM_BSON_PROPERTY_NAME, name ) ;
               builder.append( OM_BSON_PROPERTY_TYPE, type ) ;
               builder.append( OM_BSON_PROPERTY_DEFAULT, defaultValue ) ;
               builder.append( OM_BSON_PROPERTY_VALID, valid ) ;
               builder.append( OM_BSON_PROPERTY_DISPLAY, display ) ;
               builder.append( OM_BSON_PROPERTY_EDIT, edit ) ;
               builder.append( OM_BSON_PROPERTY_DESC, desc ) ;
               builder.append( OM_BSON_PROPERTY_LEVEL, level ) ;
               temp = builder.obj() ;
               arrayBuilder.append( temp ) ;
            }

            oneClusterTypeBulider.append( OM_BSON_PROPERTY_ARRAY, 
                                          arrayBuilder.arr() ) ;
            oneClusterType = oneClusterTypeBulider.obj() ;
            _restAdaptor->appendHttpBody( _restSession, oneClusterType.objdata(),
                                         oneClusterType.objsize(), 1 ) ;
         }
      }
      catch( std::exception &e )
      {
         rc = SDB_DMS_RECORD_NOTEXIST ;
         _sendErrorRes2Web( SDB_DMS_RECORD_NOTEXIST, e.what() ) ;
         goto error ;
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
                           :omQueryBusinessCommand( pRestAdaptor, 
                                                    pRestSession, 
                                                    pRootPath,
                                                    pSubPath)
   {
   }

   omConfigBusinessCommand::~omConfigBusinessCommand()
   {
   }

   INT32 omConfigBusinessCommand::_getTemplateInfo( BSONObj &bsonTemplate, 
                                                    BSONObj &bsonHostInfo )
   {
      INT32 rc          = SDB_OK ;
      const CHAR *pInfo = NULL ;
      BSONObj bsonInfo ;
      BSONObjBuilder templateBuilder ;
      BSONObjBuilder hostInfoBuilder ;
      _restAdaptor->getQuery(_restSession, OM_REST_TEMPLATE_INFO, 
                             &pInfo ) ;
      if ( NULL == pInfo )
      {
         string errorInfo = "rest field:" + string( OM_REST_TEMPLATE_INFO ) 
                            + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      rc = fromjson( pInfo, bsonInfo ) ;
      if ( SDB_OK != rc )
      {
         string errorInfo = "rest field:" + string( OM_REST_TEMPLATE_INFO )
                            + "is invalid" ;
         PD_LOG( PDERROR, "fromjson failed:rc=%d,src=%s", rc, pInfo ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

      {
         BSONObjIterator iter( bsonInfo ) ;
         while ( iter.more() )
         {
            BSONElement beTmp = iter.next() ;
            const CHAR *pFieldName = beTmp.fieldName();
            if ( ossStrcasecmp( OM_BSON_FIELD_HOST_INFO, pFieldName) == 0 )
            {
               hostInfoBuilder.append( beTmp ) ;
            }
            else
            {
               hostInfoBuilder.append( beTmp ) ;
            }
         }
      }

      bsonTemplate = templateBuilder.obj() ;
      // OM_BSON_FIELD_HOST_INFO can be null 
      bsonHostInfo = hostInfoBuilder.obj() ;

      if ( bsonTemplate.isEmpty() )
      {
         string errorInfo = string( OM_REST_TEMPLATE_INFO ) + " have not "
                            + OM_BSON_FIELD_HOST_INFO ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigBusinessCommand::_getConfigDetail( const BSONObj &bsonTemplate, 
                                                  BSONObj &bsonConfigItem )
   {
      INT32 rc = SDB_OK ;
      string configItemFile = "" ;
      string businessType ;
      BSONObjBuilder configItemBuilder ;
      BSONArrayBuilder arrayBuilder ;

      businessType = bsonTemplate.getStringField( OM_BSON_CLUSTER_TYPE ) ;
      if ( businessType.length() == 0 )
      {
         string errorInfo = string( OM_BSON_CLUSTER_TYPE ) + " is null" ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str() ) ;
         _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         goto error ;
      }
      configItemFile = _rootPath + "/" + OM_BUSINESS_CONFIG_SUBDIR + "/" 
                       + businessType + OM_CONFIG_ITEM_FILE_NAME ;
      try
      {
         ptree pt ;
         read_xml( configItemFile.c_str(), pt ) ;
         ptree ptList = pt.get_child( OM_XML_CONFIG ) ;
         ptree::iterator ite = ptList.begin() ;
         for( ; ite != ptList.end() ; ite++ )
         {
            BSONObjBuilder builder ;
            BSONObj temp ;
            string name         = "" ;
            string type         = "" ;
            string defaultValue = "" ;
            string valid        = "" ;
            string display      = "" ;
            string edit         = "" ;
            string desc         = "" ;
            string level        = "" ;
            name         = ite->second.get<string>( 
                                             OM_XMLATTR_PROPERTY_NAME ) ;
            type         = ite->second.get<string>( 
                                             OM_XMLATTR_PROPERTY_TYPE ) ;
            defaultValue = ite->second.get<string>( 
                                             OM_XMLATTR_PROPERTY_DEFAULT ) ;
            valid        = ite->second.get<string>( 
                                             OM_XMLATTR_PROPERTY_VALID ) ;
            display      = ite->second.get<string>( 
                                             OM_XMLATTR_PROPERTY_DISPLAY ) ;
            edit         = ite->second.get<string>( 
                                             OM_XMLATTR_PROPERTY_EDIT ) ;
            desc         = ite->second.get<string>( 
                                             OM_XMLATTR_PROPERTY_DESC ) ;
            level        = ite->second.get<string>( 
                                             OM_XMLATTR_PROPERTY_LEVEL ) ;
            builder.append( OM_BSON_PROPERTY_NAME, name ) ;
            builder.append( OM_BSON_PROPERTY_TYPE, type ) ;
            builder.append( OM_BSON_PROPERTY_DEFAULT, defaultValue ) ;
            builder.append( OM_BSON_PROPERTY_VALID, valid ) ;
            builder.append( OM_BSON_PROPERTY_DISPLAY, display ) ;
            builder.append( OM_BSON_PROPERTY_EDIT, edit ) ;
            builder.append( OM_BSON_PROPERTY_DESC, desc ) ;
            builder.append( OM_BSON_PROPERTY_LEVEL, level ) ;
            temp = builder.obj() ;
            arrayBuilder.append( temp ) ;
         }
      }
      catch( std::exception &e )
      {
         rc = SDB_DMS_RECORD_NOTEXIST ;
         _sendErrorRes2Web( SDB_DMS_RECORD_NOTEXIST, e.what() ) ;
         goto error ;
      }

      configItemBuilder.append( OM_BSON_PROPERTY_ARRAY, arrayBuilder.arr() ) ;
      bsonConfigItem = configItemBuilder.obj() ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigBusinessCommand::_generateConfig( 
                                                const BSONObj &bsonTemplate, 
                                                const BSONObj &bsonHostInfo, 
                                                const BSONObj &bsonConfigItem, 
                                                BSONObj &bsonConfig )
   {
      INT32 rc = SDB_OK ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
      
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
      //TODO: bsonTemplate 再load本地配置表
      rc = _getTemplateInfo( bsonTemplate, bsonHostInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get config info failed" ) ;
         goto error ;
      }
      
      rc = _getConfigDetail( bsonTemplate, bsonConfigDetail ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get config item failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = _generateConfig( bsonTemplate, bsonHostInfo, bsonConfigDetail, 
                            bsonConfig ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "generate config failed" ) ;
         _sendErrorRes2Web( rc, "generate config failed" ) ;
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

