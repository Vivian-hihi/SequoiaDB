/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = pmdLocalSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/04/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdLocalSession.hpp"
#include "charsetDef.hpp"
#include "charsetICUConvertor.hpp"
#include "msg.h"
#include "msgDef.h"
#include "msgMessage.hpp"
#include "coordRemoteSession.hpp"
#include "charsetConvertorInterface.hpp"
#include "charsetConvertorFactory.hpp"
#include "ossErr.h"
#include "ossSignal.hpp"
#include "ossTypes.h"
#include "ossTypes.hpp"
#include "pd.hpp"
#include "pmdEDU.hpp"
#include "pmdEnv.hpp"
#include "msgMessage.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
#include "pmdTrace.hpp"
#include "ossVer.hpp"
#include "rtnContext.hpp"
#include "msgConvertorImpl.hpp"
#include "../bson/lib/md5.hpp"
#include "auth.hpp"

using namespace bson ;

namespace engine
{
   #define PMD_RECV_DATA_AFTER_LENGTH_TIMEOUT         ( 120 * OSS_ONE_SEC )

   /*
      _pmdLocalSession implement
   */
   _pmdLocalSession::_pmdLocalSession( SOCKET fd )
   : pmdSession( fd ),
     _inMsgConvertor( NULL ),
     _outMsgConvertor( NULL )
   {
      ossMemset( (void*)&_replyHeader, 0, sizeof(_replyHeader) ) ;
      _needReply = TRUE ;
   }

   _pmdLocalSession::~_pmdLocalSession()
   {
      SAFE_OSS_DELETE( _inMsgConvertor ) ;
      SAFE_OSS_DELETE( _outMsgConvertor ) ;
   }

   INT32 _pmdLocalSession::getServiceType () const
   {
      return CMD_SPACE_SERVICE_LOCAL ;
   }

   SDB_SESSION_TYPE _pmdLocalSession::sessionType() const
   {
      return SDB_SESSION_LOCAL ;
   }

   void _pmdLocalSession::_onAttach ()
   {
   }

   void _pmdLocalSession::_onDetach ()
   {
   }

   INT32 _pmdLocalSession::run()
   {
      INT32 rc                = SDB_OK ;
      UINT32 msgSize          = 0 ;
      CHAR *pBuff             = NULL ;
      INT32 buffSize          = 0 ;
      pmdEDUMgr *pmdEDUMgr    = NULL ;
      monDBCB *mondbcb        = pmdGetKRCB()->getMonDBCB () ;
      // Compatibility handling. As client protocol version is unknown until the
      // first common message is received, set the minimum message length to the
      // length of old message header. Adjust it once the client version is
      // determined.
      UINT32 minMsgSize       = sizeof(MsgHeaderV1) ;
      MsgHeader *message      = NULL ;

      if ( !_pEDUCB )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      pmdEDUMgr = _pEDUCB->getEDUMgr() ;

      while ( !_pEDUCB->isDisconnected() && !_socket.isClosed() )
      {
         // clear interrupt flag
         _pEDUCB->resetInterrupt() ;
         _pEDUCB->resetInfo( EDU_INFO_ERROR ) ;
         _pEDUCB->resetLsn() ;
         pdClearLastError() ;

         // recv msg
         rc = recvData( (CHAR*)&msgSize, sizeof(UINT32) ) ;
         if ( rc )
         {
            if ( SDB_APP_FORCED != rc )
            {
               PD_LOG( PDERROR, "Session[%s] failed to recv msg size, "
                       "rc: %d", sessionName(), rc ) ;
            }
            break ;
         }
         /// update conf should here
         _pEDUCB->updateConf() ;

         // if system info msg
         if ( msgSize == (UINT32)MSG_SYSTEM_INFO_LEN )
         {
            rc = _recvSysInfoMsg( msgSize, &pBuff, buffSize ) ;
            if ( rc )
            {
               break ;
            }
            rc = _processSysInfoRequest( pBuff ) ;
            if ( rc )
            {
               break ;
            }

            _setHandshakeReceived() ;
         }
#ifdef SDB_ENTERPRISE

#ifdef SDB_SSL
         else if ( _isAwaitingHandshake() )
         {
            if ( pmdGetOptionCB()->useSSL() )
            {
               rc = _socket.doSSLHandshake ( (CHAR*)&msgSize, sizeof(UINT32) ) ;
               if ( rc )
               {
                  break ;
               }

               _setHandshakeReceived() ;
            }
            else
            {
               PD_LOG( PDERROR, "SSL handshake received but server is started "
                       "without SSL support" ) ;
               rc = SDB_NETWORK ;
               break ;
            }

            /*continue;

            PD_LOG( PDERROR, "SSL feature not available in this build" ) ;
            rc = SDB_NETWORK ;
            break ;*/
         }
#endif /* SDB_SSL */

#endif /* SDB_ENTERPRISE */
         // error msg
         else if ( msgSize < minMsgSize || msgSize > SDB_MAX_MSG_LENGTH )
         {
            PD_LOG( PDERROR, "Session[%s] recv msg size[%d] is less than "
                    "MsgHeader size[%d] or more than max msg size[%d]",
                    sessionName(), msgSize, minMsgSize,
                    SDB_MAX_MSG_LENGTH ) ;
            rc = SDB_INVALIDARG ;
            break ;
         }
         // other msg
         else
         {
            pBuff = getBuff( msgSize + 1 ) ;
            if ( !pBuff )
            {
               rc = SDB_OOM ;
               break ;
            }
            buffSize = getBuffLen() ;
            *(UINT32*)pBuff = msgSize ;
            INT32 hasReceived = 0 ;
            // recv the rest msg, need timeout
            rc = recvData( pBuff + sizeof(UINT32),
                           msgSize - sizeof(UINT32),
                           PMD_RECV_DATA_AFTER_LENGTH_TIMEOUT,
                           TRUE, &hasReceived ) ;
            if ( rc )
            {
               if ( SDB_APP_FORCED != rc )
               {
                  PD_LOG( PDERROR, "Session[%s] failed to recv msg[len: %u, "
                          "recieved: %d], rc: %d",
                          sessionName(), msgSize - sizeof(UINT32),
                          hasReceived, rc ) ;
               }
               break ;
            }

            // increase process event count
            _pEDUCB->incEventCount() ;
            mondbcb->addReceiveNum() ;
            pBuff[ msgSize ] = 0 ;
            // activate edu
            if ( SDB_OK != ( rc = pmdEDUMgr->activateEDU( _pEDUCB ) ) )
            {
               PD_LOG( PDERROR, "Session[%s] activate edu failed, rc: %d",
                       sessionName(), rc ) ;
               break ;
            }

            message = (MsgHeader *)pBuff ;
            rc = _preprocessMsg( message ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Session[%s] preprocess message failed, rc: %d",
                       sessionName(), rc ) ;
               break ;
            }
            // process msg
            rc = _processMsg( message ) ;
            if ( rc )
            {
               break ;
            }

            if ( sizeof(MsgHeader) != minMsgSize && _clientVersionMatch() )
            {
               minMsgSize = sizeof(MsgHeader) ;
            }

            // wait edu
            if ( SDB_OK != ( rc = pmdEDUMgr->waitEDU( _pEDUCB ) ) )
            {
               PD_LOG( PDERROR, "Session[%s] wait edu failed, rc: %d",
                       sessionName(), rc ) ;
               break ;
            }
         }
      } // end while

   done:
      disconnect() ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN_CHECK_PRIVS_CMD, "_pmdLocalSession::checkPrivilegesForCmd" )
   INT32 _pmdLocalSession::checkPrivilegesForCmd( const CHAR *cmdName,
                                                  const CHAR *pQuery,
                                                  const CHAR *pSelector,
                                                  const CHAR *pOrderby,
                                                  const CHAR *pHint )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN_CHECK_PRIVS_CMD );
      if ( !privilegeCheckEnabled() )
      {
         goto done;
      }

      try
      {
         boost::shared_ptr< const authAccessControlList > acl;
         rc = getACL( acl );
         PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to get ACL, rc: %d", sessionName(), rc );

         authRequiredPrivileges required;
         CMD_TAGS_ARRAY *tags = authGetCMDActionSetsTags( cmdName );
         if ( tags )
         {
            for ( UINT32 i = 0; i < tags->second; i++ )
            {
               AUTH_CMD_ACTION_SETS_TAG tag = tags->first[ i ];
               const authRequiredActionSets *actionSets = authGetCMDActionSetsByTag( tag );
               if ( !actionSets )
               {
                  rc = SDB_SYS;
                  PD_LOG( PDERROR, "Failed to get action sets for tag[%d]", tag );
                  goto error;
               }

               BSONObj obj;
               switch ( actionSets->getSource().obj )
               {
               case authRequiredActionSets::SOURCE_OBJ_NONE :
                  break;
               case authRequiredActionSets::SOURCE_OBJ_QUERY :
                  obj = BSONObj( pQuery );
                  break;
               case authRequiredActionSets::SOURCE_OBJ_SELECTOR :
                  obj = BSONObj( pSelector );
                  break;
               case authRequiredActionSets::SOURCE_OBJ_ORDERBY :
                  obj = BSONObj( pOrderby );
                  break;
               case authRequiredActionSets::SOURCE_OBJ_HINT :
                  obj = BSONObj( pHint );
                  break;
               }

               const CHAR *key = actionSets->getSource().key;
               if ( RESOURCE_TYPE_EXACT_COLLECTION == actionSets->getResourceType() )
               {
                  SDB_ASSERT( key, "The key must be configured for exact collection" );
                  BSONElement ele = obj.getField( key );
                  if ( String != ele.type() )
                  {
                     rc = SDB_SYS;
                     PD_LOG( PDERROR, "Failed to get collection full name from field[%s]", key );
                     goto error;
                  }
                  if ( !authResource::isExactName( ele.valuestr() ) )
                  {
                     rc = SDB_INVALIDARG;
                     PD_LOG_MSG( PDERROR,
                                 "Invalid format for collection name: %s, "
                                 "Expected format: <collectionspace>.<collectionname>",
                                 ele.valuestr() );
                     goto error;
                  }
                  boost::shared_ptr< authResource > r = authResource::forExact( ele.valuestr() );
                  required.addActionSetsOnResource( r, actionSets );
               }
               else if ( RESOURCE_TYPE_COLLECTION_SPACE == actionSets->getResourceType() )
               {
                  SDB_ASSERT( key, "The key must be configured for collection space" );
                  BSONElement ele = obj.getField( key );
                  if ( String != ele.type() )
                  {
                     rc = SDB_SYS;
                     PD_LOG( PDERROR, "Failed to get collection space name from field[%s]", key );
                     goto error;
                  }
                  const CHAR *pDot = ossStrchr( ele.valuestr(), '.' );
                  ossPoolString cs = pDot ? ossPoolString( ele.valuestr(), pDot - ele.valuestr() )
                                          : ossPoolString( ele.valuestr() );
                  boost::shared_ptr< authResource > r = authResource::forCS( cs );
                  required.addActionSetsOnResource( r, actionSets );
               }
               else if ( RESOURCE_TYPE_NON_SYSTEM == actionSets->getResourceType() ||
                         RESOURCE_TYPE_CLUSTER == actionSets->getResourceType() ||
                         RESOURCE_TYPE_ANY == actionSets->getResourceType() )
               {
                  required.addActionSetsOnSimpleType( actionSets );
               }
            }
         }

         rc = authMeetRequiredPrivileges( required, *acl );
         if ( SDB_NO_PRIVILEGES == rc )
         {
            rc = SDB_NO_PRIVILEGES;
            PD_LOG_MSG( PDERROR, "No privilege to execute command: %s", cmdName );
            goto error;
         }
         else if ( rc )
         {
            PD_LOG_MSG( PDERROR, "Failed to check privileges for command[%s], rc: %d", cmdName,
                        rc );
            goto error;
         }
      }
      catch ( std::exception &e )
      {
         rc = ossException2RC( &e );
         PD_RC_CHECK( rc, PDERROR, "Occur exception: %s. rc: %d", e.what(), rc );
         goto error;
      }
      catch ( boost::exception &e )
      {
         rc = SDB_SYS;
         PD_LOG( PDERROR, "Occured exception when check privileges: %s, rc: %d",
                 boost::diagnostic_information( e ).c_str(), rc );
         goto error;
      }

   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN_CHECK_PRIVS_CMD, rc );
      return rc;
   error:
      goto done;
   }
   
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN_CHECK_PRIVS_EXACT, "_pmdLocalSession::checkPrivilegesForActionsOnExact" )
   INT32 _pmdLocalSession::checkPrivilegesForActionsOnExact( const CHAR *pCollectionName,
                                                             const authActionSet &actions )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN_CHECK_PRIVS_EXACT ) ;
      if ( !privilegeCheckEnabled() )
      {
         goto done;
      }

      try
      {
         boost::shared_ptr< const authAccessControlList > acl;
         rc = getACL( acl );
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to get ACL, rc: %d", sessionName(), rc );
            goto error;
         }

         rc = authMeetActionsOnExact( pCollectionName, actions, *acl );
         if ( SDB_NO_PRIVILEGES == rc )
         {
            ossPoolStringStream ss;
            ss << actions;
            PD_LOG_MSG( PDERROR, "No privilege for actions %s on collection %s", ss.str().c_str(),
                        pCollectionName );
            goto error;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG(PDERROR, "Failed to check privileges, rc: %d");
            goto error;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG_MSG( PDERROR, "Occured exception: %s", e.what() );
         rc = SDB_INVALIDARG;
         goto error;
      }
      catch ( boost::exception &e )
      {
         rc = SDB_SYS;
         PD_LOG( PDERROR, "Occured exception: %s, rc: %d",
                 boost::diagnostic_information( e ).c_str(), rc );
         goto error;
      }

   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN_CHECK_PRIVS_EXACT, rc );
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN_CHECK_PRIVS_CLUSTER, "_pmdLocalSession::checkPrivilegesForActionsOnCluster" )
   INT32 _pmdLocalSession::checkPrivilegesForActionsOnCluster( const authActionSet &actions )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN_CHECK_PRIVS_CLUSTER );
      if ( !privilegeCheckEnabled() )
      {
         goto done;
      }

      try
      {
         boost::shared_ptr< const authAccessControlList > acl;
         rc = getACL( acl );
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to get ACL, rc: %d", sessionName(), rc );
            goto error;
         }
         rc = authMeetActionsOnCluster( actions, *acl );
         if ( SDB_NO_PRIVILEGES == rc )
         {
            ossPoolStringStream ss;
            ss << actions;
            PD_LOG_MSG( PDERROR, "No privilege for actions %s on cluster", ss.str().c_str() );
            goto error;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG(PDERROR, "Failed to check privileges, rc: %d");
            goto error;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG_MSG( PDERROR, "Occured exception: %s", e.what() );
         rc = SDB_INVALIDARG;
         goto error;
      }
      catch ( boost::exception &e )
      {
         rc = SDB_SYS;
         PD_LOG( PDERROR, "Occured exception: %s, rc: %d",
                 boost::diagnostic_information( e ).c_str(), rc );
         goto error;
      }

   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN_CHECK_PRIVS_CLUSTER, rc );
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN_CHECK_PRIVS_RESOURCE, "_pmdLocalSession::checkPrivilegesForActionsOnResource" )
   INT32 _pmdLocalSession::checkPrivilegesForActionsOnResource(
      const boost::shared_ptr< authResource > &resource,
      const authActionSet &actions )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN_CHECK_PRIVS_RESOURCE );
      if ( !privilegeCheckEnabled() )
      {
         goto done;
      }

      try
      {
         boost::shared_ptr< const authAccessControlList > acl;
         rc = getACL( acl );
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to get ACL, rc: %d", sessionName(), rc );
            goto error;
         }

         if ( !acl->isAuthorizedForActionsOnResource( *resource, actions))
         {
            BSONObjBuilder resBuilder;
            resource->toBSONObj(resBuilder);
            ossPoolStringStream ss;
            ss << actions;
            PD_LOG_MSG( PDERROR, "No privilege for actions %s on resource %s", ss.str().c_str(),
                        resBuilder.done().toString().c_str() );
            rc = SDB_NO_PRIVILEGES;
            goto error;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG_MSG( PDERROR, "Occured exception: %s", e.what() );
         rc = SDB_INVALIDARG;
         goto error;
      }
      catch ( boost::exception &e )
      {
         rc = SDB_SYS;
         PD_LOG( PDERROR, "Occured exception: %s, rc: %d",
                 boost::diagnostic_information( e ).c_str(), rc );
         goto error;
      }

   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN_CHECK_PRIVS_RESOURCE, rc );
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN_GETACL, "_pmdLocalSession::getACL" )
   INT32 _pmdLocalSession::getACL( boost::shared_ptr< const authAccessControlList > &acl )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN_GETACL );
      if ( !_acl )
      {
         rc = sdbGetRTNCB()->getUserCacheMgr()->getACL( _pEDUCB, getClient()->getUsername(), _acl );
         PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to get ACL from user cache manager, rc: %d",
                      sessionName(), rc );
      }
      acl = _acl;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN_GETACL, rc );
      return rc;
   error:
      goto done;
   }

   INT32 _pmdLocalSession::_recvSysInfoMsg( UINT32 msgSize,
                                            CHAR **ppBuff,
                                            INT32 &buffLen )
   {
      INT32 rc = SDB_OK ;
      UINT32 recvSize = sizeof(MsgSysInfoRequest) ;

      *ppBuff = getBuff( recvSize ) ;
      if ( !*ppBuff )
      {
         rc = SDB_OOM ;
         goto error ;
      }
      buffLen = getBuffLen() ;
      *(INT32*)(*ppBuff) = msgSize ;

      // recv recvSize1
      rc = recvData( *ppBuff + sizeof(UINT32), recvSize - sizeof( UINT32 ),
                     PMD_RECV_DATA_AFTER_LENGTH_TIMEOUT ) ;
      if ( rc )
      {
         if ( SDB_APP_FORCED != rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to recv sys info req rest "
                    "msg, rc: %d", sessionName(), rc ) ;
         }
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_processSysInfoRequest( const CHAR * msg )
   {
      INT32 rc = SDB_OK ;
      INT32 version = 0 ;
      INT32 subVersion = 0 ;
      INT32 fixVersion = 0 ;
      BOOLEAN endianConvert = FALSE ;
      md5::md5digest digest ;
      MsgGlobalID globalID = getOperator()->getGlobalID() ;
      MsgSysInfoReply reply ;

      reply.header.specialSysInfoLen      = MSG_SYSTEM_INFO_LEN ;
      reply.header.eyeCatcher             = MSG_SYSTEM_INFO_EYECATCHER ;
      reply.header.realMessageLength      = sizeof(MsgSysInfoReply) ;
      reply.osType                        = OSS_OSTYPE ;
      reply.authVersion                   = AUTH_SCRAM_SHA256 ;
      reply.dbStartTime                   = pmdGetStartTime() ;
      ossGetVersion( &version, &subVersion, &fixVersion, NULL, NULL, NULL ) ;
      reply.version                       = version ;
      reply.subVersion                    = subVersion ;
      reply.fixVersion                    = fixVersion ;
      reply.reserved                      = 0 ;

      globalID.incQueryID() ;
      reply.globalID = globalID ;
      getOperator()->updateGlobalID( globalID ) ;

      ossMemset( reply.pad, 0, sizeof( reply.pad ) ) ;

      md5::md5( (const void *)&reply,
                sizeof(MsgSysInfoReply) - sizeof(reply.fingerprint),
                digest ) ;
      ossMemcpy( reply.fingerprint, digest, sizeof(reply.fingerprint) ) ;

      rc = msgExtractSysInfoRequest ( (CHAR*)msg, endianConvert ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to extract sys info "
                    "request, rc = %d", sessionName(), rc ) ;

      rc = sendData ( (const CHAR*)&reply, sizeof( MsgSysInfoReply ) ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to send packet, rc = %d",
                    sessionName(), rc ) ;

   done :
      return rc ;
   error :
      disconnect() ;
      goto done ;
   }

   BOOLEAN _pmdLocalSession::_clientVersionMatch() const
   {
      return ( SDB_PROTOCOL_VER_2 == _client.getClientVersion() ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN__PREPROCESSMSG, "_pmdLocalSession::_preprocessMsg" )
   INT32 _pmdLocalSession::_preprocessMsg( MsgHeader *&msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN__PREPROCESSMSG ) ;
      CHAR *newMsg = NULL ;
      UINT32 msgLength = 0 ;

      SDB_ASSERT( msg, "Message pointer is NULL" ) ;

      // If the version of peer is unknown, we can determine it by the first
      // common message.
      if ( SDB_PROTOCOL_VER_INVALID == _client.getClientVersion() )
      {
         _client.setClientVersion(
               ( MSG_COMM_EYE_DEFAULT == msg->eye ) ?
               (SDB_PROTOCOL_VERSION)msg->version : SDB_PROTOCOL_VER_1 ) ;
      }

      if ( !_clientVersionMatch() )
      {
         if ( !_msgConvertorEnabled() )
         {
            rc = _enableMsgConvertor() ;
            PD_RC_CHECK( rc, PDERROR, "Session[%s] enables message convertor "
                         "failed[%d]", sessionName(), rc) ;
            PD_LOG( PDDEBUG, "Session[%s] enables message convertors",
                    sessionName() ) ;
         }
         else
         {
            _inMsgConvertor->reset( FALSE ) ;
         }

         rc = _inMsgConvertor->push((const CHAR *)msg, msg->messageLength ) ;
         PD_RC_CHECK( rc, PDERROR, "Push message into the message convertor "
                      "failed[%d]", rc ) ;
         rc = _inMsgConvertor->output( newMsg, msgLength ) ;
         PD_RC_CHECK( rc, PDERROR, "Get message from the message convertor "
                      "failed[%d]", rc ) ;

         SDB_ASSERT( (UINT32)((MsgHeader *)newMsg)->messageLength == msgLength,
                     "Message length after conversion is not as expected" ) ;

         msg = (MsgHeader *)newMsg ;
      }

   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN__PREPROCESSMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   void _pmdLocalSession::_saveOrSetMsgGlobalID( MsgHeader *pMsg )
   {
      SDB_ASSERT( pMsg, "msg can't be NULL" ) ;
      IOperator *pOperator = getOperator() ;
      MsgGlobalID globalID = pOperator->getGlobalID() ;

      if ( pMsg->globalID.getQueryID().getIdentifyID() != globalID.getQueryID().getIdentifyID() )
      {
         // The msg may be sent by the old version client
         // The msg's globalID of old version client is not initialized, so it's a random value
         globalID.incQueryID() ;
         pMsg->globalID = globalID ;
      }

      ((pmdOperator*)pOperator)->setMsg( pMsg ) ;

      return ;
   }

   INT32 _pmdLocalSession::_onMsgBegin( MsgHeader *pMsg )
   {
      INT32 rc = SDB_OK ;

      _pEDUCB->clearProcessInfo() ;

      _saveOrSetMsgGlobalID( pMsg ) ;

      // set reply header ( except flags, length )
      getClient()->registerInMsg( pMsg ) ;
      _replyHeader.contextID          = -1 ;
      _replyHeader.numReturned        = 0 ;
      _replyHeader.startFrom          = 0 ;
      _replyHeader.header.eye         = MSG_COMM_EYE_DEFAULT ;
      _replyHeader.header.opCode      = MAKE_REPLY_TYPE(pMsg->opCode) ;
      _replyHeader.header.requestID   = pMsg->requestID ;
      _replyHeader.header.TID         = pMsg->TID ;
      _replyHeader.header.routeID     = pmdGetNodeID() ;
      _replyHeader.header.version     = SDB_PROTOCOL_VER_2 ;
      _replyHeader.header.flags       = 0 ;
      _replyHeader.header.globalID    = pMsg->globalID ;
      ossMemset( _replyHeader.header.reserve, 0,
                 sizeof(_replyHeader.header.reserve) ) ;
      _replyHeader.returnMask         = 0 ;

      if ( isNoReplyMsg( pMsg->opCode ) )
      {
         _needReply = FALSE ;
      }
      else
      {
         _needReply = TRUE ;
      }

      // start operator
      MON_START_OP( _pEDUCB->getMonAppCB() ) ;
      _pEDUCB->getMonAppCB()->setLastOpType( pMsg->opCode ) ;

      return rc ;
   }

   void _pmdLocalSession::_onMsgEnd( INT32 result, MsgHeader *msg )
   {
      if ( result && SDB_DMS_EOC != result )
      {
         PD_LOG( PDWARNING, "Session[%s] process msg[opCode=%d, len: %d, "
                 "TID: %d, requestID: %llu] failed, rc: %d",
                 sessionName(), msg->opCode, msg->messageLength, msg->TID,
                 msg->requestID, result ) ;
      }

      if ( result != SDB_OK )
      {
         pmdIncErrNum( result ) ;
      }

      // end operator
      MON_END_OP( _pEDUCB->getMonAppCB() ) ;

      getClient()->unregisterInMsg() ;

      _pEDUCB->clearProcessInfo() ;

      ((pmdOperator*)getOperator())->clearMsg() ;

      if ( privilegeCheckEnabled() )
      {
         _acl.reset() ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN_PROMSG, "_pmdLocalSession::_processMsg" )
   INT32 _pmdLocalSession::_processMsg( MsgHeader * msg )
   {
      INT32 rc          = SDB_OK ;
      const CHAR *pBody = NULL ;
      INT32 bodyLen     = 0 ;
      rtnContextBuf contextBuff ;
      rtnContextBuf outReply ;
      INT32 opCode      = msg->opCode ;
      BOOLEAN hasException = FALSE ;
      MsgHeader *convertedMsg = NULL ;

      BOOLEAN needRollback = FALSE ;
      BOOLEAN isAutoCommit = FALSE ;
      BOOLEAN isDoCommit   = FALSE ;

      BSONObjBuilder retBuilder( PMD_RETBUILDER_DFT_SIZE ) ;

      PD_TRACE_ENTRY( SDB_PMDLOCALSN_PROMSG ) ;

      UINT64 bTime = ossGetCurrentMicroseconds() ;

      // convert message charset
      charsetConvertorInterface *inConvertor = NULL ;
      charsetConvertorInterface *outConvertor = NULL ;

      Charset clientCharset, resultsCharset ;
      bool replyConverted = FALSE ;
      static const Charset systemCharset = CHARSET_UTF8 ;

      // prepare input and output convertor
      {
         rc = _getClientCharset( clientCharset ) ;
         SDB_ASSERT(SDB_OK == rc, "ClientCharset is not valid" ) ;

         rc = _getResultsCharset( resultsCharset ) ;
         SDB_ASSERT(SDB_OK == rc, "ResultsCharset is not valid" ) ;

         inConvertor = charsetConvertorFactory::get( clientCharset,
                                                     systemCharset ) ;
         outConvertor = charsetConvertorFactory::get( systemCharset,
                                                      resultsCharset ) ;
      }
      // convert input message
      if ( inConvertor )
      {
         rc = _convertMsg( msg, &convertedMsg, inConvertor ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to convert charset "
                    "of input message , rc: %d", sessionName(), rc ) ;
         }
         if ( NULL != convertedMsg && SDB_OK == rc )
         {
            msg = convertedMsg ;
         }
      }

      // prepare
      _onMsgBegin( msg ) ;
      if ( SDB_OK == rc )
      {
         if ( MSG_BS_TRANS_COMMIT_REQ == msg->opCode )
         {
            isDoCommit = TRUE ;
         }

         try
         {
            rc = _processor->processMsg( msg, contextBuff,
                                         _replyHeader.contextID,
                                         _needReply,
                                         needRollback,
                                         retBuilder ) ;
            pBody     = contextBuff.data() ;
            bodyLen   = contextBuff.size() ;
            _replyHeader.numReturned = contextBuff.recordNum() ;
            _replyHeader.startFrom = (INT32)contextBuff.getStartFrom() ;

            if ( eduCB()->isAutoCommitTrans() &&
                 -1 == eduCB()->getCurAutoTransCtxID() )
            {
               isAutoCommit = TRUE ;
               if ( SDB_OK == rc || SDB_DMS_EOC == rc )
               {
                  INT32 rcTmp = _processor->doCommit() ;
                  rc = rcTmp ? rcTmp : rc ;
               }
            }

            if ( SDB_OK != rc && SDB_RTN_ALREADY_IN_AUTO_TRANS != rc &&
                 eduCB()->isTransaction() &&
                 ( isAutoCommit || isDoCommit ||
                   ( needRollback &&
                     eduCB()->getTransExecutor()->isTransAutoRollback() )
                 )
               )
            {
               PD_LOG( PDDEBUG, "Session[%s] rolling back operation "
                       "(opCode=%d, rc=%d)", sessionName(), msg->opCode, rc ) ;

               INT32 rcTmp = _processor->doRollback() ;
               if ( rcTmp )
               {
                  PD_LOG( PDERROR, "Session[%s] failed to rollback trans "
                          "info, rc: %d", sessionName(), rcTmp ) ;
               }
            }
         }
         catch( std::bad_alloc &e )
         {
            hasException = TRUE ;
            PD_LOG_MSG( PDERROR, "Occur exception: %s", e.what() ) ;
            rc = SDB_OOM ;
         }
         catch( std::exception &e )
         {
            hasException = TRUE ;
            PD_LOG_MSG( PDERROR, "Occur exception: %s", e.what() ) ;
            rc = SDB_SYS ;
         }
      }

      if ( _needReply )
      {
         if ( rc )
         {
            if ( SDB_APP_INTERRUPT == rc &&
                 SDB_OK != _pEDUCB->getInterruptRC() )
            {
               rc = _pEDUCB->getInterruptRC() ;
               PD_LOG ( PDDEBUG, "Interrupted EDU [%llu] with return code %d",
                        _pEDUCB->getID(), rc ) ;
            }

            if ( 0 == bodyLen )
            {
               utilBuildErrorBson( retBuilder, rc,
                                   _pEDUCB->getInfo( EDU_INFO_ERROR ) ) ;
               _errorInfo = retBuilder.obj() ;
               pBody = _errorInfo.objdata() ;
               bodyLen = (INT32)_errorInfo.objsize() ;
               BSONObj convertedErrObj ;
               if ( outConvertor && !replyConverted )
               {
                  INT32 tmpRC = outConvertor->convert( _errorInfo,
                                                       convertedErrObj ) ;
                  if ( tmpRC )
                  {
                     // Just report convert failed error message if rc is not SDB_OK
                     PD_LOG( PDERROR, "Session[%s] failed to convert "
                             "error message , rc: %d", sessionName(), rc ) ;
                  }
                  else
                  {
                     // Set converted error message
                     _errorInfo = convertedErrObj ;
                     pBody = _errorInfo.objdata() ;
                     bodyLen = (INT32)_errorInfo.objsize() ;
                  }
               }
               _replyHeader.numReturned = 1 ;
            }
            else
            {
               SDB_ASSERT( 1 == _replyHeader.numReturned,
                           "Record number must be 1" ) ;

               BSONObj errObj( pBody ), convertedErrObj ;
               if ( outConvertor && !replyConverted )
               {
                  INT32 tmpRC = outConvertor->convert( errObj,
                                                       convertedErrObj ) ;
                  if ( tmpRC )
                  {
                     // Just report convert failed error message if rc is not SDB_OK
                     PD_LOG( PDERROR, "Session[%s] failed to convert "
                             "error message , rc: %d", sessionName(), rc ) ;
                  }
                  else
                  {
                     retBuilder.appendElements( convertedErrObj ) ;
                  }
               }
               else
               {
                  retBuilder.appendElements( errObj ) ;
               }
               _errorInfo = retBuilder.obj() ;
               pBody = _errorInfo.objdata() ;
               bodyLen = (INT32)_errorInfo.objsize() ;
               _replyHeader.numReturned = 1 ;
            }
            replyConverted = TRUE ;
         }
         /// succeed and has result info
         else if ( !retBuilder.isEmpty() && 0 == bodyLen )
         {
            _errorInfo = retBuilder.obj() ;
            pBody = _errorInfo.objdata() ;
            bodyLen = (INT32)_errorInfo.objsize() ;
            _replyHeader.numReturned = 1 ;
         }

         // convert reply body
         if ( outConvertor && pBody && bodyLen && !replyConverted )
         {
            // do not convert OpenLob reply message
            // because it maybe not a BSON sequences
            if ( MSG_BS_LOB_OPEN_REQ != opCode &&
                 MSG_BS_LOB_READ_REQ != opCode )
            {
               rtnContextBuf inReply( pBody, bodyLen,
                                      _replyHeader.numReturned ) ;
               INT32 tmpRC = _convertReplyBody( inReply, outReply,
                                                outConvertor ) ;
               if ( tmpRC )
               {
                  PD_LOG( PDERROR, "Session[%s] failed to convert charset "
                        "of reply body, rc: %d, tmpRC: %d",
                        sessionName(), rc, tmpRC ) ;
                  rc = tmpRC ;
               }
               else if ( outReply.size() )
               {
                  pBody = outReply.data() ;
                  bodyLen = outReply.size() ;
               }
            }
         }

         // fill the return opCode
         _replyHeader.header.opCode = MAKE_REPLY_TYPE(opCode) ;
         _replyHeader.flags         = rc ;
         _replyHeader.header.messageLength = sizeof( _replyHeader ) +
                                             bodyLen ;

         // send response
         INT32 rcTmp = _reply( &_replyHeader, pBody, bodyLen ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Session[%s] failed to send response, rc: %d",
                    sessionName(), rcTmp ) ;
            disconnect() ;
         }
         else if ( hasException )
         {
            disconnect() ;
         }
      }

      // end
      _onMsgEnd( rc, msg ) ;

      // release converted msg buffer
      if ( inConvertor && convertedMsg )
      {
         msgReleaseBuffer( (CHAR *) convertedMsg, eduCB() );
      }

      UINT64 eTime = ossGetCurrentMicroseconds() ;
      if ( eTime > bTime )
      {
         monSvcTaskInfo *pInfo = NULL ;
         pInfo = eduCB()->getMonAppCB()->getSvcTaskInfo() ;
         if ( pInfo )
         {
            /// it doesn't matter wether type is MON_TOTAL_WRITE_TIME
            /// or MON_TOTAL_READ_TIME
            pInfo->monOperationTimeInc( MON_TOTAL_WRITE_TIME,
                                        eTime - bTime ) ;
         }
      }

      rc = SDB_OK ;
      PD_TRACE_EXITRC ( SDB_PMDLOCALSN_PROMSG, rc ) ;
      return rc ;
   }

   INT32 _pmdLocalSession::_reply( MsgOpReply* responseMsg, const CHAR *pBody,
                                   INT32 bodyLen )
   {
      INT32 rc = SDB_OK ;

      if ( _inMsgConvertor )
      {
         rc = _replyInCompatibleMode( responseMsg, pBody, bodyLen ) ;
         PD_RC_CHECK( rc, PDERROR, "Send reply message in compatible mode "
                      "failed[%d]. Message: %s", rc,
                      msg2String( &(responseMsg->header) ).c_str() ) ;
      }
      else
      {
         rc = _replyInNormalMode( responseMsg, pBody, bodyLen ) ;
         PD_RC_CHECK( rc, PDERROR, "Send reply message failed[%d]. Message: %s",
                      rc, msg2String( &(responseMsg->header) ).c_str() ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN__REPLYINCOMPATIBLEMODE, "_pmdLocalSession::_replyInCompatibleMode" )
   INT32 _pmdLocalSession::_replyInCompatibleMode( MsgOpReply *responseMsg,
                                                   const CHAR *data,
                                                   INT32 dataLen )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN__REPLYINCOMPATIBLEMODE ) ;
      CHAR *ptr = NULL ;
      UINT32 len = 0 ;

      SDB_ASSERT( _outMsgConvertor, "Message convertor is NULL" ) ;

      _outMsgConvertor->reset( FALSE ) ;
      rc = _outMsgConvertor->push( (const CHAR *)responseMsg,
                                   sizeof(MsgOpReply) ) ;
      PD_RC_CHECK( rc, PDERROR, "Push reply message header into message "
                   "convertor failed[%d]. Message: %s",
                   rc, msg2String( &(responseMsg->header) ).c_str() ) ;
      if ( data && dataLen > 0)
      {
         rc = _outMsgConvertor->push( data, dataLen ) ;
         PD_RC_CHECK( rc, PDERROR, "Push reply data into message convertor "
                      "failed[%d]. Message: %s",
                      rc, msg2String( &(responseMsg->header) ).c_str() ) ;
      }

      while ( TRUE )
      {
         rc = _outMsgConvertor->output( ptr, len ) ;
         PD_RC_CHECK( rc, PDERROR, "Get reply message from message convertor "
                      "failed[%d]", rc ) ;
         if ( !ptr )
         {
            // All data has been sent.
            break ;
         }

         rc = sendData( ptr, len ) ;
         PD_RC_CHECK( rc, PDERROR, "Send reply message failed[%d]", rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN__REPLYINCOMPATIBLEMODE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN__REPLYINNORMALMODE, "_pmdLocalSession::_replyInNormalMode" )
   INT32 _pmdLocalSession::_replyInNormalMode( MsgOpReply *responseMsg,
                                               const CHAR *data,
                                               INT32 dataLen )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN__REPLYINNORMALMODE ) ;

      // Step 1: Send the message header
      rc = sendData( (const CHAR *)responseMsg, sizeof(MsgOpReply) ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to send response header[%d]",
                   sessionName(), rc ) ;

      if ( data && dataLen > 0)
      {
         rc = sendData( data, dataLen ) ;
         PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to send data[%d]",
                      sessionName(), rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN__REPLYINNORMALMODE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_convertReplyBody(
      rtnContextBuf &inReply, rtnContextBuf &outReply,
      charsetConvertorInterface *convertor )
   {
      INT32 rc = SDB_OK;
      vector<BSONObj> replyBSONObjs;
      UINT32 bodyLength = 0, bodyOffset = 0;
      CHAR *buff = NULL;
      try
      {
         // convert body charset
         while ( !inReply.eof() )
         {
            BSONObj reply, convertedReply ;
            rc = inReply.nextObj( reply ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to get obj from obj buf, rc: %d",
                         rc ) ;

            rc = convertor->convert( reply, convertedReply ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to convert reply BSONObj, rc: %d",
                         rc ) ;

            replyBSONObjs.push_back( convertedReply ) ;
            bodyLength += ossAlign4( (UINT32)convertedReply.objsize() ) ;
         }

         // build new body
         buff = ( CHAR* ) SDB_THREAD_ALLOC( bodyLength ) ;
         PD_CHECK( buff, SDB_OOM, error, PDERROR,
                   " Out of memory while rebuilding reply body") ;

         ossMemset( buff, 0, bodyLength );
         for ( UINT32 i = 0 ; i < replyBSONObjs.size() ; i++ )
         {
            BSONObj obj = replyBSONObjs[i] ;
            ossMemcpy( (CHAR*)&(buff[bodyOffset]),
                       obj.objdata(),  obj.objsize() ) ;
            bodyOffset += ossAlign4( obj.objsize() ) ;
         }

         {
            rtnContextBuf tmpCtxBuf( buff, bodyLength, replyBSONObjs.size() ) ;
            outReply = tmpCtxBuf;
         }
      }
      catch ( exception &e )
      {
         rc = ossException2RC( &e ) ;
         PD_LOG( PDERROR, "Failed to convert reply object, occur exception %s, "
                 "rc: %d", e.what(), rc ) ;
         goto error ;
      }
   done:
     return rc ;
   error:
     goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_CONVERTMSG, "_pmdLocalSession::_convertMsg" )
   INT32 _pmdLocalSession::_convertMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_CONVERTMSG ) ;
      INT32 rc = SDB_OK;
      INT32 opCode = in->opCode ;
      pmdEDUCB *edu = eduCB() ;

      // shield to avoid calling check urgent event during
      // communicating messages with other nodes
      pmdUrgentEventShield _shield( edu ) ;
      switch ( opCode )
      {
         case MSG_BS_MSG_REQ:
            break;
         case MSG_BS_INSERT_REQ:
         {
            rc = _rebuildInsertMsg( in, out, convertor );
            PD_RC_CHECK( rc, PDERROR, "Failed to convert insert message "
                         "charset, rc: %d", rc ) ;
            break;
         }
         case MSG_BS_UPDATE_REQ:
         {
            rc = _rebuildUpdateMsg( in, out, convertor );
            PD_RC_CHECK( rc, PDERROR, "Failed to convert update message "
                         "charset, rc: %d", rc ) ;
            break;
         }
         case MSG_BS_SQL_REQ:
         {
            rc = _rebuildSQLMsg( in, out, convertor );
            PD_RC_CHECK( rc, PDERROR, "Failed to convert SQL message "
                         "charset, rc: %d", rc ) ;
            break;
         }
         case MSG_BS_DELETE_REQ:
         {
            rc = _rebuildDeleteMsg( in, out, convertor );
            PD_RC_CHECK( rc, PDERROR, "Failed to convert delete message "
                         "charset, rc: %d", rc ) ;
            break;
         }
         case MSG_BS_AGGREGATE_REQ:
         {
            rc = _rebuildAggrMsg( in, out, convertor );
            PD_RC_CHECK( rc, PDERROR, "Failed to convert aggrgate message "
                         "charset, rc: %d", rc ) ;
            break;
         }
         case MSG_BS_QUERY_REQ:
         {
            rc = _rebuildQueryMsg(in, out, convertor);
            PD_RC_CHECK( rc, PDERROR, "Failed to convert query message "
                         "charset, rc: %d", rc ) ;
            break;
         }
         case MSG_BS_TRANS_BEGIN_REQ:
         case MSG_BS_TRANS_COMMIT_REQ:
         case MSG_BS_TRANS_ROLLBACK_REQ:
         case MSG_BS_INTERRUPTE:
         case MSG_BS_LOB_WRITE_REQ:
         case MSG_BS_LOB_READ_REQ:
         case MSG_BS_LOB_LOCK_REQ:
         case MSG_BS_LOB_CLOSE_REQ:
         case MSG_BS_LOB_GETRTDETAIL_REQ:
         {
            break ;
         }
         case MSG_BS_LOB_OPEN_REQ:
         {
            rc = _rebuildOpenLobMsg( in, out, convertor ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of open Lob "
                         "message, rc: %d", rc ) ;
            break ;
         }
         case MSG_BS_LOB_REMOVE_REQ:
         {
            rc = _rebuildRemoveLobMsg( in, out, convertor ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of remove Lob "
                         "message, rc: %d", rc ) ;
            break ;
         }
         case MSG_BS_LOB_TRUNCATE_REQ:
         {
            rc = _rebuildTruncateLobMsg( in, out, convertor ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of truncate Lob "
                         "message, rc: %d", rc ) ;
            break ;
         }
         case MSG_BS_LOB_CREATELOBID_REQ:
         {
            rc = _rebuildCreateLobIDMsg( in, out, convertor ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of "
                         "create Lob ID message, rc: %d", rc ) ;
            break ;
         }
         case MSG_AUTH_CRTUSR_REQ:
         case MSG_AUTH_DELUSR_REQ:
            break ;
         case MSG_BS_SEQUENCE_FETCH_REQ:
         {
            rc = _rebuildFetchSeqMsg( in, out, convertor ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to convert fetch sequence "
                         "message charset, rc: %d", rc ) ;
            break;
         }
         default:
            break;
      }

      if ( out && (*out) )
      {
         // keep following fields same with original message
         (*out)->eye = in->eye ;
         (*out)->globalID = in->globalID ;
         (*out)->version = in->version ;
         (*out)->routeID = in->routeID ;
         (*out)->TID = in->TID ;
         (*out)->opCode = in->opCode ;
      }
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_CONVERTMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDINSERTEMSG, "_pmdLocalSession::_rebuildInsertMsg" )
   INT32 _pmdLocalSession::_rebuildInsertMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDINSERTEMSG ) ;
      INT32 rc = SDB_OK;
      INT32 flag = 0 ;
      const CHAR *pCollectionName = NULL ;
      const CHAR *pInsertor = NULL ;
      const CHAR *pHint = NULL ;
      INT32 count = 0 ;
      std::string convertedCollectionName ;

      // extract msg
      rc = msgExtractInsert( (CHAR*)in, &flag,
                             &pCollectionName, &pInsertor, count, &pHint ) ;
      PD_RC_CHECK(rc, PDERROR, "Failed to extract insert message, rc: %d", rc);

      // convert charset for collection name
      rc = convertor->convert( StringData(pCollectionName),
                               convertedCollectionName ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to convert charset for "
                   "collection name, rc: %d", rc ) ;

      // build new message with converted BSONObj and collection name
      for ( INT32 i = 0 ; i < count ; i++ )
      {
         INT32 bufLen = 0 ;
         BSONObj currentObj( pInsertor ) ;
         BSONObj convertedObj ;

         rc = convertor->convert( currentObj, convertedObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert charset for BSONObj "
                      "to be inserted ,rc: %d", rc ) ;

         if ( 0 == i )
         {
            rc = msgBuildInsertMsg( (CHAR **)out, &bufLen,
                                    convertedCollectionName.c_str(),
                                    flag, in->requestID,
                                    &convertedObj, eduCB() ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to append first converted "
                         "BSONObj to insert msg, rc: %d", rc ) ;
         }
         else
         {
             rc = msgAppendInsertMsg( (CHAR **)out, &bufLen,
                                      &convertedObj, eduCB() ) ;
             PD_RC_CHECK( rc, PDERROR, "Failed to append converted BSONObj to "
                          "insert msg, rc: %d", rc ) ;
         }
         pInsertor += ossAlign4( currentObj.objsize() ) ;
      }

      // append hint if exists
      if ( pHint )
      {
         INT32 bufLen = 0 ;
         BSONObj hint( pHint ) ;
         BSONObj convertedHint ;
         rc = convertor->convert( hint, convertedHint ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of hint "
                      "for insert message, rc: %d", rc ) ;

         rc = msgAppendHint2InsertMsg( (CHAR **)out, &bufLen,
                                       &convertedHint, eduCB() ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to append converted hint to "
                      "insert msg, rc: %d", rc ) ;
      }
   done:
     PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDINSERTEMSG, rc ) ;
     return rc ;
   error:
     goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDUPDATEMSG, "_pmdLocalSession::_rebuildUpdateMsg" )
   INT32 _pmdLocalSession::_rebuildUpdateMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDUPDATEMSG ) ;
      INT32 rc    = SDB_OK ;
      INT32 flag = 0 ;
      const CHAR *pCollectionName = NULL ;
      const CHAR *pSelectorBuffer = NULL ;
      const CHAR *pUpdatorBuffer  = NULL ;
      const CHAR *pHintBuffer     = NULL ;
      std::string convertedCollectionName ;
      BSONObj convertedSelector ;
      BSONObj convertedUpdator ;
      BSONObj convertedHint ;
      INT32 bufLen = 0 ;

      // extract msg
      rc = msgExtractUpdate( (const CHAR*)in, &flag, &pCollectionName,
                             &pSelectorBuffer, &pUpdatorBuffer,
                             &pHintBuffer );
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract update message failed, "
                   "rc: %d", sessionName(), rc ) ;

      // convert charset for strings in message
      {
         BSONObj selector( pSelectorBuffer ) ;
         BSONObj updator( pUpdatorBuffer ) ;
         BSONObj hint( pHintBuffer ) ;

         rc = convertor->convert( StringData(pCollectionName),
                                  convertedCollectionName ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert collection name, "
                      "rc: %d", rc ) ;

         rc = convertor->convert( selector, convertedSelector ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert selector charset, "
                     "rc: %d", rc ) ;

         rc = convertor->convert( updator, convertedUpdator ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert updator charset, "
                      "rc: %d", rc ) ;

         rc = convertor->convert( hint, convertedHint ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert hint charset, "
                     "rc: %d", rc ) ;
      }

      // rebuild message
      rc = msgBuildUpdateMsg( (CHAR **)out, &bufLen,
                              convertedCollectionName.c_str(),
                              flag, in->requestID, &convertedSelector,
                              &convertedUpdator,
                              &convertedHint, eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to rebuild update message, rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDUPDATEMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDSQLMSG, "_pmdLocalSession::_rebuildSQLMsg" )
   INT32 _pmdLocalSession::_rebuildSQLMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDSQLMSG ) ;
      INT32 rc          = SDB_OK ;
      const CHAR *sql   = NULL ;
      std::string convertedSQL ;
      INT32 bufLen = 0 ;

      rc = msgExtractSql( (const CHAR*)in, &sql ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to extract delete msg in "
                   "Session[%s], rc: %d", sessionName(), rc ) ;

      rc = convertor->convert( StringData(sql), convertedSQL );
      PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of SQL statement" ) ;

      rc = msgBuildSQL( (CHAR **) out, &bufLen,
                        convertedSQL.c_str(), in->requestID, eduCB() ) ;
      PD_RC_CHECK(rc, PDERROR, "Failed to build SQL, rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDSQLMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDDELMSG, "_pmdLocalSession::_rebuildDeleteMsg" )
   INT32 _pmdLocalSession::_rebuildDeleteMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDDELMSG ) ;
      INT32 rc    = SDB_OK ;
      INT32 flag = 0 ;
      const CHAR *pCollectionName = NULL ;
      const CHAR *pDeletorBuffer  = NULL ;
      const CHAR *pHintBuffer     = NULL ;
      BSONObj convertedDeletor, convertedHint ;
      std::string convertedCollectionName ;
      INT32 bufLen = 0 ;

      // extract msg
      rc = msgExtractDelete ( (const CHAR *)in , &flag, &pCollectionName,
                              &pDeletorBuffer, &pHintBuffer ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract delete msg failed, rc: %d",
                   sessionName(), rc ) ;

      // convert charset for strings in message
      {
         BSONObj deletor( pDeletorBuffer ) ;
         BSONObj hint( pHintBuffer ) ;

         rc = convertor->convert( StringData(pCollectionName),
                                  convertedCollectionName ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert charset for collection "
                     "name, rc: %d", rc ) ;

         rc = convertor->convert( deletor, convertedDeletor ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert charset for "
                      "deletor, rc: %d", rc ) ;

         rc = convertor->convert( hint, convertedHint ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert charset for "
                      "hint, rc: %d", rc ) ;
      }

      // rebuild message
      rc = msgBuildDeleteMsg( (CHAR **)out, &bufLen,
                              convertedCollectionName.c_str(),
                              flag, in->requestID, &convertedDeletor,
                              &convertedHint, eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to rebuild delete message, rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDDELMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDAGGRMSG, "_pmdLocalSession::_rebuildAggrMsg" )
   INT32 _pmdLocalSession::_rebuildAggrMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDAGGRMSG ) ;
      INT32 rc    = SDB_OK ;
      const CHAR *pObjs = NULL, *pCurr = NULL ;
      INT32 count = 0 ;
      INT32 flag = 0 ;
      const CHAR *pCollectionName = NULL ;
      std::string convertedCollectionName ;
      INT32 bufLen = 0 ;
      UINT32 offset = 0;

      // extract collection name and body in message
      rc = msgExtractAggrRequest( (const CHAR*)in, &pCollectionName,
                                  &pObjs, count, &flag ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract aggr msg failed, rc: %d",
                   sessionName(), rc ) ;

      // convert collection name
      rc = convertor->convert( StringData(pCollectionName),
                               convertedCollectionName ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to convert collection name, "
                   "rc: %d", rc ) ;

      // convert charset for strings in message
      for ( INT32 i = 0 ; i < count ; i++ )
      {
         pCurr = &pObjs[offset] ;
         BSONObj currObj(pCurr), convertedObj;

         // convert BSONObj in message
         rc = convertor->convert( currObj, convertedObj );
         PD_RC_CHECK( rc, PDERROR, "Failed to convert aggr message body, "
                      "rc: %d", rc ) ;
         if ( 0 == i )
         {
            rc = msgBuildAggrMsg( (CHAR **)out, &bufLen,
                                  convertedCollectionName.c_str(),
                                  flag, in->requestID, &convertedObj,
                                  eduCB() ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to rebuild aggr message, "
                         "rc: %d", rc ) ;
         }
         else
         {
            rc = msgAppendAggrMsg( (CHAR **)out, &bufLen,
                                   &convertedObj, eduCB() ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to append aggr object, "
                         "rc: %d", rc ) ;
         }
         offset += ossAlign4( currObj.objsize() );
      }
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDAGGRMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDQUERYRMSG, "_pmdLocalSession::_rebuildQueryMsg" )
   INT32 _pmdLocalSession::_rebuildQueryMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDQUERYRMSG ) ;
      INT32 rc                      = SDB_OK ;
      const CHAR *pCollectionName   = NULL ;
      const CHAR *pQueryBuff        = NULL ;
      const CHAR *pFieldSelector    = NULL ;
      const CHAR *pOrderByBuffer    = NULL ;
      const CHAR *pHintBuffer       = NULL ;
      INT32 flag              = 0 ;
      INT64 numToSkip         = -1 ;
      INT64 numToReturn       = -1 ;
      INT32 bufLen = 0 ;
      BSONObj convertedQuery, convertedSelector;
      BSONObj convertedOrder, convertedHint ;
      std::string convertedCollectionName ;

      // extract msg
      rc = msgExtractQuery ( (const CHAR *)in, &flag, &pCollectionName,
                             &numToSkip, &numToReturn, &pQueryBuff,
                             &pFieldSelector, &pOrderByBuffer, &pHintBuffer ) ;
      PD_RC_CHECK ( rc, PDERROR, "Extract query msg failed[rc:%d]", rc ) ;

      // convert charset for strings in message
      {
         BSONObj query( pQueryBuff) ;
         BSONObj selector( pFieldSelector ) ;
         BSONObj order( pOrderByBuffer ) ;
         BSONObj hint( pHintBuffer ) ;
         // convert collection name
         rc = convertor->convert( StringData(pCollectionName),
                                  convertedCollectionName ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert collection name, "
                      "rc: %d", rc ) ;

         // convert BSONObj in message
         rc = convertor->convert( query, convertedQuery ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert "
                      "query charset, rc: %d", rc ) ;

         rc = convertor->convert( selector, convertedSelector) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert selector "
                      "charset, rc: %d", rc ) ;

         rc = convertor->convert( order, convertedOrder ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert order "
                      "charset, rc: %d", rc ) ;

         rc = convertor->convert( hint, convertedHint ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert "
                      "hint charset, rc: %d", rc ) ;
      }

      // rebuild new message
      rc = msgBuildQueryMsg( (CHAR **)out, &bufLen,
                             convertedCollectionName.c_str(),
                             flag, in->requestID, numToSkip, numToReturn,
                             &convertedQuery, &convertedSelector,
                             &convertedOrder, &convertedHint, eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to rebuild charset for query message,"
                   "rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDQUERYRMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDOPENLOBMSG, "_pmdLocalSession::_rebuildOpenLobMsg" )
   INT32 _pmdLocalSession::_rebuildOpenLobMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDOPENLOBMSG ) ;
      INT32 rc = SDB_OK ;
      const MsgOpLob *header = NULL ;
      BSONObj obj, convertedObj ;
      INT32 bufLen = 0 ;

      rc = msgExtractOpenLobRequest( (const CHAR *)in, &header, obj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Failed to extract open lob request" ) ;

      rc = convertor->convert( obj, convertedObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of metadata in lob, "
                   "rc: %d", rc ) ;

      rc = msgBuildOpenLobMsg( (CHAR **)(out), &bufLen, &convertedObj,
                               header->flags, header->w, header->contextID,
                               header->header.requestID, eduCB() ) ;

      PD_RC_CHECK( rc, PDERROR, "Failed to rebuild open lob message, "
                   "rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDOPENLOBMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDREMOVELOBMSG, "_pmdLocalSession::_rebuildRemoveLobMsg" )
   INT32 _pmdLocalSession::_rebuildRemoveLobMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDREMOVELOBMSG ) ;
      INT32 rc = SDB_OK ;
      const MsgOpLob *header = NULL ;
      BSONObj obj, convertedObj ;
      INT32 bufLen = 0 ;

      rc = msgExtractRemoveLobRequest( (const CHAR *)in, &header, obj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Failed to extract remove lob request" ) ;

      rc = convertor->convert( obj, convertedObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of metadata in lob, "
                   "rc: %d", rc ) ;

      rc = msgBuildRemoveLobMsg( (CHAR **)(out), &bufLen, &convertedObj,
                                 header->flags, header->w, header->contextID,
                                 header->header.requestID, eduCB() ) ;

      PD_RC_CHECK( rc, PDERROR, "Failed to rebuild remove lob message, "
                   "rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDREMOVELOBMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDTRUNCATELOBMSG, "_pmdLocalSession::_rebuildTruncateLobMsg" )
   INT32 _pmdLocalSession::_rebuildTruncateLobMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDTRUNCATELOBMSG ) ;
      INT32 rc = SDB_OK ;
      const MsgOpLob *header = NULL ;
      BSONObj obj, convertedObj ;
      INT32 bufLen = 0 ;

      rc = msgExtractTruncateLobRequest( (const CHAR *)in, &header, obj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Failed to extract truncate lob request" ) ;

      rc = convertor->convert( obj, convertedObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of metadata in lob, "
                   "rc: %d", rc ) ;

      rc = msgBuildTruncateLobMsg( (CHAR **)(out), &bufLen, &convertedObj,
                                   header->flags, header->w, header->contextID,
                                   header->header.requestID, eduCB() ) ;

      PD_RC_CHECK( rc, PDERROR, "Failed to rebuild truncate lob message, "
                   "rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDTRUNCATELOBMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDCREATELOBIDMSG, "_pmdLocalSession::_rebuildCreateLobIDMsg" )
   INT32 _pmdLocalSession::_rebuildCreateLobIDMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDCREATELOBIDMSG ) ;
      INT32 rc = SDB_OK ;
      const MsgOpLob *header = NULL ;
      BSONObj obj, convertedObj ;
      INT32 bufLen = 0 ;

      rc = msgExtractCreateLobIDRequest( (const CHAR *)in, &header, obj ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Failed to extract create lob ID request" ) ;

      rc = convertor->convert( obj, convertedObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to convert charset of metadata in lob, "
                   "rc: %d", rc ) ;

      rc = msgBuildCreateLobIDMsg( (CHAR **)(out), &bufLen, &convertedObj,
                                   header->flags, header->w, header->contextID,
                                   header->header.requestID, eduCB() ) ;

      PD_RC_CHECK( rc, PDERROR, "Failed to rebuild create lob ID message, "
                   "rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDCREATELOBIDMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSESSION_REBUILDFETCHSEQMSG, "_pmdLocalSession::_rebuildFetchSeqMsg" )
   INT32 _pmdLocalSession::_rebuildFetchSeqMsg(
      const MsgHeader *in, MsgHeader **out,
      charsetConvertorInterface *convertor )
   {
      PD_TRACE_ENTRY( SDB_PMDLOCALSESSION_REBUILDFETCHSEQMSG ) ;
      INT32 rc                      = SDB_OK ;
      const CHAR *pCollectionName   = NULL ;
      const CHAR *pQueryBuff        = NULL ;
      const CHAR *pFieldSelector    = NULL ;
      const CHAR *pOrderByBuffer    = NULL ;
      const CHAR *pHintBuffer       = NULL ;
      INT32 flag              = 0 ;
      INT64 numToSkip         = -1 ;
      INT64 numToReturn       = -1 ;
      INT32 bufLen = 0 ;
      BSONObj convertedQuery ;

      // extract msg
      rc = msgExtractQuery ( (const CHAR *)in, &flag, &pCollectionName,
                             &numToSkip, &numToReturn, &pQueryBuff,
                             &pFieldSelector, &pOrderByBuffer, &pHintBuffer ) ;
      PD_RC_CHECK ( rc, PDERROR, "Extract query msg failed[rc:%d]", rc ) ;

      // convert charset for strings in message
      {
         BSONObj query( pQueryBuff) ;
         // convert BSONObj in message
         rc = convertor->convert( query, convertedQuery ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert "
                      "query charset, rc: %d", rc ) ;
      }

      // rebuild new message
      rc = msgBuildQueryMsg( (CHAR **)out, &bufLen,
                             "", flag, in->requestID, numToSkip, numToReturn,
                             &convertedQuery, NULL, NULL, NULL, eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to rebuild charset for "
                   "fetching sequence message, rc: %d", rc ) ;
   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSESSION_REBUILDFETCHSEQMSG, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_getClientCharset( Charset &clientCharset )
   {
      INT32 rc = SDB_OK ;
      coordSessionPropSite *pPropSite = NULL ;
      pmdRemoteSessionSite *pSite = NULL ;
      pmdEDUCB *cb = eduCB();

      clientCharset = getClient()->getClientCharset() ;
      // use session property if remote site exists
      pSite = ( pmdRemoteSessionSite* ) cb->getRemoteSite() ;
      if ( pSite )
      {
         pPropSite = ( coordSessionPropSite* ) pSite->getUserData() ;
         PD_CHECK( NULL != pPropSite, SDB_SYS, error, PDERROR,
                   "Session's prop site is NULL" ) ;
         clientCharset = pPropSite->getClientCharset();
      }
      PD_CHECK( clientCharset != CHARSET_UNKNOWN, SDB_SYS, error,
                PDERROR, "Unknown charset" ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_getResultsCharset( Charset &resultsCharset )
   {
      INT32 rc = SDB_OK ;
      coordSessionPropSite *pPropSite = NULL ;
      pmdRemoteSessionSite *pSite = NULL ;
      pmdEDUCB *cb = eduCB();

      resultsCharset = getClient()->getResultsCharset() ;
      pSite = ( pmdRemoteSessionSite* ) cb->getRemoteSite() ;
      // use session property if remote site exists
      if ( pSite )
      {
         pPropSite = ( coordSessionPropSite* ) pSite->getUserData() ;
         PD_CHECK( NULL != pPropSite, SDB_SYS, error, PDERROR,
                   "Session's prop site is NULL" ) ;
         resultsCharset = pPropSite->getResultsCharset() ;
      }
      PD_CHECK( resultsCharset != CHARSET_UNKNOWN, SDB_SYS, error,
                PDERROR, "Unknown charset" ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _pmdLocalSession::_msgConvertorEnabled() const
   {
      return ( NULL != _inMsgConvertor ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOCALSN__ENABLEMSGCONVERTOR, "_pmdLocalSession::_enableMsgConvertor" )
   INT32 _pmdLocalSession::_enableMsgConvertor()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_PMDLOCALSN__ENABLEMSGCONVERTOR ) ;

      SDB_ASSERT( !(_inMsgConvertor || _outMsgConvertor),
                  "Convertor is not NULL" ) ;

      _inMsgConvertor = SDB_OSS_NEW msgConvertorImpl ;
      _outMsgConvertor = SDB_OSS_NEW msgConvertorImpl ;
      if ( !_inMsgConvertor || !_outMsgConvertor )
      {
         rc = SDB_OOM ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB_PMDLOCALSN__ENABLEMSGCONVERTOR, rc ) ;
      return rc ;
   error:
      SAFE_OSS_DELETE( _inMsgConvertor ) ;
      SAFE_OSS_DELETE( _outMsgConvertor ) ;
      goto done ;
   }

}
