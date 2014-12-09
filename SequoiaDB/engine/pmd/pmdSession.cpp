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

   Source File Name = pmdSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/04/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdSession.hpp"
#include "pmdEDU.hpp"
#include "pmdEnv.hpp"
#include "msgMessage.hpp"
#include "rtn.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "msgAuth.hpp"
#include "rtnLob.hpp"
#include "pmdProcessor.hpp"

using namespace bson ;

namespace engine
{
   /*
      _pmdLocalSession implement
   */
   _pmdLocalSession::_pmdLocalSession( SOCKET fd )
   :pmdSession( fd )
   {
      _authOK  = FALSE ;
      ossMemset( (void*)&_replyHeader, 0, sizeof(_replyHeader) ) ;
      _needReply = TRUE ;
      _needRollback = FALSE ;
      _pDMSCB = NULL ;
      _pDPSCB = NULL ;
      _pRTNCB = NULL ;
   }

   _pmdLocalSession::~_pmdLocalSession()
   {
   }

   UINT64 _pmdLocalSession::identifyID()
   {
      return ossPack32To64( _socket.getLocalIP(), _socket.getLocalPort() ) ;
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
      pmdKRCB *krcb = pmdGetKRCB() ;
      _pDMSCB = krcb->getDMSCB() ;
      _pDPSCB = krcb->getDPSCB() ;
      _pRTNCB = krcb->getRTNCB() ;

      if ( _pDPSCB && !_pDPSCB->isLogLocal() )
      {
         _pDPSCB = NULL ;
      }
   }

   INT32 _pmdLocalSession::_attachProcessor( _IProcessor *processor )
   {
      SDB_ASSERT( NULL != processor, "" ) ;
      _processor = processor ;
      return _processor->attachSession( this ) ;
   }

   void _pmdLocalSession::_detachProcessor()
   {
      SDB_ASSERT( NULL != _processor, "" ) ;
      _processor->detachSession() ;
      _processor = NULL ;
   }

   void _pmdLocalSession::_onDetach ()
   {
      // rollback transaction
      if ( DPS_INVALID_TRANS_ID != eduCB()->getTransID() )
      {
         INT32 rc = rtnTransRollback( eduCB(), _pDPSCB ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] rollback trans info failed, rc: %d",
                    sessionName(), rc ) ;
         }
      }

      // delete all context
      INT64 contextID = -1 ;
      while ( -1 != ( contextID = eduCB()->contextPeek() ) )
      {
         _pRTNCB->contextDelete( contextID, NULL ) ;
      }

      eduCB()->setClientSock( NULL ) ;
   }

   INT32 _pmdLocalSession::run()
   {
      INT32 rc                = SDB_OK ;
      UINT32 msgSize          = 0 ;
      CHAR *pBuff             = NULL ;
      INT32 buffSize          = 0 ;
      pmdEDUMgr *pmdEDUMgr    = NULL ;
      _DataProcessor dataProcessor ;

      if ( !_pEDUCB )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      pmdEDUMgr               = _pEDUCB->getEDUMgr() ;
      _attachProcessor( &dataProcessor ) ;

      while ( !_pEDUCB->isDisconnected() && !_socket.isClosed() )
      {
         // clear interrupt flag
         _pEDUCB->resetInterrupt() ;
         _pEDUCB->resetInfo( EDU_INFO_ERROR ) ;

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
         }
         // error msg
         else if ( msgSize < sizeof(MsgHeader) || msgSize > SDB_MAX_MSG_LENGTH )
         {
            PD_LOG( PDERROR, "Session[%s] recv msg size[%d] is less than "
                    "MsgHeader size[%d] or more than max msg size[%d]",
                    sessionName(), msgSize, sizeof(MsgHeader),
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
            // recv the rest msg
            rc = recvData( pBuff + sizeof(UINT32), msgSize - sizeof(UINT32) ) ;
            if ( rc )
            {
               if ( SDB_APP_FORCED != rc )
               {
                  PD_LOG( PDERROR, "Session failed to recv rest msg, rc: %d",
                          sessionName(), rc ) ;
               }
               break ;
            }
 
            // increase process event count
            _pEDUCB->incEventCount() ;
            pBuff[ msgSize ] = 0 ;
            // activate edu
            if ( SDB_OK != ( rc = pmdEDUMgr->activateEDU( _pEDUCB ) ) )
            {
               PD_LOG( PDERROR, "Session[%s] activate edu failed, rc: %d",
                       sessionName(), rc ) ;
               break ;
            }
            // process msg
            rc = _processMsg( (MsgHeader*)pBuff ) ;
            if ( rc )
            {
               break ;
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
      _detachProcessor() ;
      disconnect() ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_recvSysInfoMsg( UINT32 msgSize,
                                            CHAR **ppBuff,
                                            INT32 &buffLen )
   {
      INT32 rc = SDB_OK ;
      INT32 recvSize = (INT32)sizeof(MsgSysInfoRequest) ;

      *ppBuff = getBuff( recvSize ) ;
      if ( !*ppBuff )
      {
         rc = SDB_OOM ;
         goto error ;
      }
      buffLen = getBuffLen() ;
      *(INT32*)(*ppBuff) = msgSize ;

      // recv recvSize1
      rc = recvData( *ppBuff + sizeof(UINT32), recvSize - sizeof( UINT32 ) ) ;
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

   INT32 _pmdLocalSession::_onAuth( MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      BSONObj authObj ;
      BSONElement user, pass ;
      rc = extractAuthMsg( msg, authObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to extrace auth msg, "
                   "rc: %d", sessionName(), rc ) ;
      user = authObj.getField( SDB_AUTH_USER ) ;
      pass = authObj.getField( SDB_AUTH_PASSWD ) ;

      if ( SDB_ROLE_STANDALONE == pmdGetDBRole() ) // not auth
      {
         _authOK = TRUE ;
         goto done ;
      }
      else if ( SDB_ROLE_OM == pmdGetDBRole() )
      {
         rc = sdbGetOMManager()->authenticate( authObj, eduCB() ) ;
         PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to authenticate, "
                      "rc: %d", sessionName(), rc ) ;
         _authOK = TRUE ;
         eduCB()->setUserInfo( user.valuestrsafe(), pass.valuestrsafe() ) ;
      }
      else
      {
         MsgHeader *pAuthRes = NULL ;
         shardCB *pShard = sdbGetShardCB() ;
         BOOLEAN hasRetry = FALSE ;

         while ( TRUE )
         {
            rc = pShard->syncSend( msg, CATALOG_GROUPID, TRUE, &pAuthRes ) ;
            if ( SDB_OK != rc )
            {
               rc = pShard->syncSend( msg, CATALOG_GROUPID, FALSE, &pAuthRes ) ;
               PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to send auth "
                            "req to catalog, rc=%d", sessionName(), rc ) ;
            }
            if ( NULL == pAuthRes )
            {
               rc = SDB_SYS ;
               PD_LOG( PDERROR, "syncsend return ok but res is NULL" ) ;
               goto error ;
            }
            rc = (( MsgInternalReplyHeader *)pAuthRes)->res ;
            SDB_OSS_FREE( (BYTE*)pAuthRes ) ;
            pAuthRes = NULL ;

            if ( SDB_CLS_NOT_PRIMARY == rc && !hasRetry )
            {
               hasRetry = TRUE ;
               pShard->updateCatGroup( TRUE, CLS_SHARD_TIMEOUT ) ;
               continue ;
            }
            else if ( rc )
            {
               PD_LOG( PDERROR, "Session[%s] auth failed, rc: %d",
                       sessionName(), rc ) ;
               goto error ;
            }
            else
            {
               _authOK = TRUE ;
               eduCB()->setUserInfo( user.valuestrsafe(),
                                     pass.valuestrsafe() ) ;
            }
            break ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_processSysInfoRequest( const CHAR * msg )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN endianConvert = FALSE ;
      MsgSysInfoReply reply ;

      MsgSysInfoReply *pReply = &reply ;
      INT32 replySize = sizeof(reply) ;

      rc = msgExtractSysInfoRequest ( (CHAR*)msg, endianConvert ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to extract sys info "
                    "request, rc = %d", sessionName(), rc ) ;

      // reply
      rc = msgBuildSysInfoReply ( (CHAR**)&pReply, &replySize ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to build sys info reply, "
                    "rc = %d", sessionName(), rc ) ;

      rc = sendData ( (const CHAR*)pReply, replySize ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to send packet, rc = %d",
                    sessionName(), rc ) ;

   done :
      return rc ;
   error :
      disconnect() ;
      goto done ;
   }

   INT32 _pmdLocalSession::_onMsgBegin( MsgHeader *msg )
   {
      // set reply header ( except flags, length )
      _replyHeader.contextID          = -1 ;
      _replyHeader.numReturned        = 0 ;
      _replyHeader.startFrom          = 0 ;
      _replyHeader.header.opCode      = MAKE_REPLY_TYPE(msg->opCode) ;
      _replyHeader.header.requestID   = msg->requestID ;
      _replyHeader.header.TID         = msg->TID ;
      _replyHeader.header.routeID     = pmdGetNodeID() ;

      if ( MSG_BS_INTERRUPTE == msg->opCode ||
           MSG_BS_DISCONNECT == msg->opCode )
      {
         _needReply = FALSE ;
      }
      else
      {
         _needReply = TRUE ;
      }

      if ( MSG_BS_UPDATE_REQ == msg->opCode ||
           MSG_BS_INSERT_REQ == msg->opCode ||
           MSG_BS_DELETE_REQ == msg->opCode ||
           MSG_BS_TRANS_COMMIT_REQ == msg->opCode )
      {
         _needRollback = TRUE ;
      }
      else
      {
         _needRollback = FALSE ;
      }

      // start operator
      MON_START_OP( _pEDUCB->getMonAppCB() ) ;

      return SDB_OK ;
   }

   void _pmdLocalSession::_onMsgEnd( INT32 result, MsgHeader *msg )
   {
      // release buff context
      _contextBuff.release() ;

      if ( result && SDB_DMS_EOC != result )
      {
         PD_LOG( PDWARNING, "Session[%s] process msg[opCode=%d, len: %d, "
                 "TID: %d, requestID: %llu] failed, rc: %d",
                 sessionName(), msg->opCode, msg->messageLength, msg->TID,
                 msg->requestID, result ) ;
      }

      // end operator
      MON_END_OP( _pEDUCB->getMonAppCB() ) ;
   }

   INT32 _pmdLocalSession::_processMsg( MsgHeader * msg )
   {
      INT32 rc          = SDB_OK ;
      const CHAR *pBody = NULL ;
      INT32 bodyLen     = 0 ;
      rtnContextBuf contextBuff ;

      // prepare
      rc = _onMsgBegin( msg ) ;
      if ( SDB_OK == rc )
      {
         if ( MSG_AUTH_VERIFY_REQ == msg->opCode )
         {
            rc = _onAuth( msg ) ;
         }
         else if ( !_authOK )
         {
            rc = SDB_AUTH_AUTHORITY_FORBIDDEN ;
         }
         else if ( MSG_BS_INTERRUPTE == msg->opCode )
         {
            rc = _onInterruptMsg( msg ) ;
         }
         else if ( MSG_BS_DISCONNECT == msg->opCode )
         {
            PD_LOG( PDEVENT, "Session[%s, %d] recv disconnect msg",
                    sessionName(), eduID() ) ;
            disconnect() ;
         }
         else
         {
            rc = _processor->processMsg( msg, _pDPSCB, contextBuff, 
                                         _replyHeader.contextID, 
                                         _replyHeader.startFrom ) ;
            pBody     = contextBuff.data() ;
            bodyLen   = contextBuff.size() ;
            _replyHeader.numReturned = contextBuff.recordNum() ;
            if ( SDB_OK != rc )
            {
               if ( _needRollback )
               {
                  INT32 rcTmp = rtnTransRollback( eduCB(), _pDPSCB ) ;
                  if ( rcTmp )
                  {
                     PD_LOG( PDERROR, "Session[%s] failed to rollback trans "
                             "info, rc: %d", sessionName(), rcTmp ) ;
                  }
                  _needRollback = FALSE ;
               }
            }
//            rc = _processOPMsg( msg, _replyHeader.contextID, &pBody,
//                                bodyLen, _replyHeader.numReturned,
//                                _replyHeader.startFrom ) ;
         }
      }

      if ( _needReply )
      {
         if ( rc && bodyLen == 0 )
         {
            _errorInfo = utilGetErrorBson( rc, _pEDUCB->getInfo(
                                           EDU_INFO_ERROR ) ) ;
            pBody = _errorInfo.objdata() ;
            bodyLen = (INT32)_errorInfo.objsize() ;
            _replyHeader.numReturned = 1 ;
         }
         _replyHeader.flags = rc ;
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
      }

      // end
      _onMsgEnd( rc, msg ) ;
      rc = SDB_OK ;

      return rc ;
   }

   INT32 _pmdLocalSession::_reply( MsgOpReply *responseMsg,
                                   const CHAR *pBody,
                                   INT32 bodyLen )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( responseMsg->header.messageLength ==
                  (SINT32)(sizeof(MsgOpReply) + bodyLen),
                  "Invalid msg" ) ;

      // response header
      rc = sendData( (const CHAR*)responseMsg, sizeof(MsgOpReply) ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Session[%s] failed to send response header, rc: %d",
                 sessionName(), rc ) ;
         goto error ;
      }
      // response body
      if ( pBody )
      {
         rc = sendData( pBody, bodyLen ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to send response body, rc: %d",
                    sessionName(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_processOPMsg( MsgHeader *msg, INT64 &contextID,
                                          const CHAR **ppBody, INT32 &bodyLen,
                                          INT32 &returnNum, INT32 &startPos )
   {
      INT32 rc = SDB_OK ;

      switch( msg->opCode )
      {
         case MSG_BS_INTERRUPTE :
            rc = _onInterruptMsg( msg ) ;
            break ;
         case MSG_BS_MSG_REQ :
            rc = _onMsgReqMsg( msg ) ;
            break ;
         case MSG_BS_UPDATE_REQ :
            rc = _onUpdateReqMsg( msg ) ;
            break ;
         case MSG_BS_INSERT_REQ :
            rc = _onInsertReqMsg( msg ) ;
            break ;
         case MSG_BS_QUERY_REQ :
            rc = _onQueryReqMsg( msg, _contextBuff, startPos, contextID ) ;
            break ;
         case MSG_BS_DELETE_REQ :
            rc = _onDelReqMsg( msg ) ;
            break ;
         case MSG_BS_GETMORE_REQ :
            rc = _onGetMoreReqMsg( msg, _contextBuff, startPos, contextID ) ;
            break ;
         case MSG_BS_KILL_CONTEXT_REQ :
            rc = _onKillContextsReqMsg( msg ) ;
            break ;
         case MSG_BS_DISCONNECT :
            PD_LOG( PDEVENT, "Session[%s, %d] recv disconnect msg",
                    sessionName(), eduID() ) ;
            disconnect() ;
            break ;
         case MSG_BS_SQL_REQ :
            rc = _onSQLMsg( msg, contextID ) ;
            break ;
         case MSG_BS_TRANS_BEGIN_REQ :
            rc = _onTransBeginMsg() ;
            break ;
         case MSG_BS_TRANS_COMMIT_REQ :
            rc = _onTransCommitMsg() ;
            break ;
         case MSG_BS_TRANS_ROLLBACK_REQ :
            rc = _onTransRollbackMsg() ;
            break ;
         case MSG_BS_AGGREGATE_REQ :
            rc = _onAggrReqMsg( msg, contextID ) ;
            break ;
         case MSG_BS_LOB_OPEN_REQ :
            rc = _onOpenLobMsg( msg, contextID, _contextBuff ) ;
            break ;
         case MSG_BS_LOB_WRITE_REQ:
            rc = _onWriteLobMsg( msg ) ;
            break ;
         case MSG_BS_LOB_READ_REQ:
            rc = _onReadLobMsg( msg, _contextBuff ) ;
            break ;
         case MSG_BS_LOB_CLOSE_REQ:
            rc = _onCloseLobMsg( msg ) ;
            break ;
         case MSG_BS_LOB_REMOVE_REQ:
            rc = _onRemoveLobMsg( msg ) ;
            break ;
         default :
            PD_LOG( PDWARNING, "Session[%s] recv unknow msg[type:[%d]%d, "
                    "len: %d, tid: %d, routeID: %d.%d.%d, reqID: %lld]",
                    sessionName(), IS_REPLY_TYPE(msg->opCode),
                    GET_REQUEST_TYPE(msg->opCode), msg->messageLength, msg->TID,
                    msg->routeID.columns.groupID, msg->routeID.columns.nodeID,
                    msg->routeID.columns.serviceID, msg->requestID ) ;
            rc = SDB_INVALIDARG ;
            break ;
      }

      // set return body
      *ppBody     = _contextBuff.data() ;
      bodyLen     = _contextBuff.size() ;
      returnNum   = _contextBuff.recordNum() ;

      if ( rc )
      {
         goto error ;
      }

   done:
      return rc ;
   error:
      if ( _needRollback )
      {
         INT32 rcTmp = rtnTransRollback( eduCB(), _pDPSCB ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Session[%s] failed to rollback trans info, "
                    "rc: %d", sessionName(), rcTmp ) ;
         }
         _needRollback = FALSE ;
      }
      goto done ;
   }

   INT32 _pmdLocalSession::_onInsertReqMsg( MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pInsertor = NULL ;
      INT32 count = 0 ;

      rc = msgExtractInsert( (CHAR *)msg, &flag, &pCollectionName,
                             &pInsertor, count ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extrace insert msg failed, rc: %d",
                   sessionName(), rc ) ;

      try
      {
         BSONObj insertor( pInsertor ) ;
         // add list op info
         MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                             "CL:%s, Insertors:%s, count: %d",
                             pCollectionName,
                             insertor.toString().c_str(),
                             count ) ;

         PD_LOG ( PDDEBUG, "Session[%s] insert objs: %s\ncount: %d\n"
                  "collection: %s", sessionName(), insertor.toString().c_str(),
                  count, pCollectionName ) ;
 
         rc = rtnInsert( pCollectionName, insertor, count, flag, _pEDUCB ) ;
         PD_RC_CHECK( rc, PDERROR, "Session[%s] insert objs[%s, count:%d, "
                      "collection: %s] failed, rc: %d", sessionName(),
                      insertor.toString().c_str(), count, pCollectionName,
                      rc ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Session[%s] insert objs occur exception: %s",
                 sessionName(), e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onUpdateReqMsg( MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flags = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pSelectorBuffer = NULL ;
      CHAR *pUpdatorBuffer = NULL ;
      CHAR *pHintBuffer = NULL ;

      rc = msgExtractUpdate( (CHAR*)msg, &flags, &pCollectionName,
                             &pSelectorBuffer, &pUpdatorBuffer,
                             &pHintBuffer );
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract update message failed, "
                   "rc: %d", sessionName(), rc ) ;

      try
      {
         BSONObj selector( pSelectorBuffer );
         BSONObj updator( pUpdatorBuffer );
         BSONObj hint( pHintBuffer );
         // add last op info
         MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                             "CL:%s, Match:%s, Updator:%s, Hint:%s",
                             pCollectionName,
                             selector.toString().c_str(),
                             updator.toString().c_str(),
                             hint.toString().c_str() ) ;

         PD_LOG ( PDDEBUG, "Session[%s] Update: selctor: %s\nupdator: %s\n"
                  "hint: %s", sessionName(), selector.toString().c_str(),
                  updator.toString().c_str(), hint.toString().c_str() ) ;

         rc = rtnUpdate( pCollectionName, selector, updator, hint,
                         flags, _pEDUCB, _pDMSCB, _pDPSCB ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Session[%s] Failed to create selector and updator "
                  "for update: %s", sessionName(), e.what () ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onDelReqMsg( MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flags = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pDeletorBuffer = NULL ;
      CHAR *pHintBuffer = NULL ;

      rc = msgExtractDelete ( (CHAR *)msg , &flags, &pCollectionName, 
                              &pDeletorBuffer, &pHintBuffer ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract delete msg failed, rc: %d",
                   sessionName(), rc ) ;

      try
      {
         BSONObj deletor ( pDeletorBuffer ) ;
         BSONObj hint ( pHintBuffer ) ;
         // add last op info
         MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                            "CL:%s, Deletor:%s, Hint:%s",
                            pCollectionName,
                            deletor.toString().c_str(),
                            hint.toString().c_str() ) ;

         PD_LOG ( PDDEBUG, "Session[%s] Delete: deletor: %s\nhint: %s",
                  sessionName(), deletor.toString().c_str(), 
                  hint.toString().c_str() ) ;

         rc = rtnDelete( pCollectionName, deletor, hint, flags, _pEDUCB, 
                         _pDMSCB, _pDPSCB ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Session[%s] Failed to create deletor for "
                  "DELETE: %s", sessionName(), e.what () ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onInterruptMsg( MsgHeader * msg )
   {
      PD_LOG ( PDEVENT, "Session[%s, %d] recieved interrupt msg",
               sessionName(), eduID() ) ;

      // delete all contextID, rollback transaction
      if ( _pEDUCB )
      {
         INT64 contextID = -1 ;
         while ( -1 != ( contextID = _pEDUCB->contextPeek() ) )
         {
            _pRTNCB->contextDelete ( contextID, NULL ) ;
         }

         INT32 rcTmp = rtnTransRollback( _pEDUCB, _pDPSCB );
         if ( rcTmp )
         {
            PD_LOG ( PDERROR, "Failed to rollback(rc=%d)", rcTmp );
         }
         _pEDUCB->clearTransInfo() ;
      }

      return SDB_OK ;
   }

   INT32 _pmdLocalSession::_onMsgReqMsg( MsgHeader * msg )
   {
      return rtnMsg( (MsgOpMsg*)msg ) ;
   }

   INT32 _pmdLocalSession::_onQueryReqMsg( MsgHeader * msg,
                                           _rtnContextBuf &buffObj,
                                           INT32 &startingPos,
                                           INT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      INT32 flags = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pQueryBuff = NULL ;
      CHAR *pFieldSelector = NULL ;
      CHAR *pOrderByBuffer = NULL ;
      CHAR *pHintBuffer = NULL ;
      INT64 numToSkip = -1 ;
      INT64 numToReturn = -1 ;
      _rtnCommand *pCommand = NULL ;

      rc = msgExtractQuery ( (CHAR *)msg, &flags, &pCollectionName,
                             &numToSkip, &numToReturn, &pQueryBuff,
                             &pFieldSelector, &pOrderByBuffer, &pHintBuffer ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract query msg failed, rc: %d",
                   sessionName(), rc ) ;

      if ( !rtnIsCommand ( pCollectionName ) )
      {
         rtnContextBase *pContext = NULL ;
         try
         {
            BSONObj matcher ( pQueryBuff ) ;
            BSONObj selector ( pFieldSelector ) ;
            BSONObj orderBy ( pOrderByBuffer ) ;
            BSONObj hint ( pHintBuffer ) ;
            // add last op info
            MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                               "CL:%s, Match:%s, Selector:%s, OrderBy:%s, "
                               "Hint:%s", pCollectionName,
                               matcher.toString().c_str(),
                               selector.toString().c_str(),
                               orderBy.toString().c_str(),
                               hint.toString().c_str() ) ;

            PD_LOG ( PDDEBUG, "Session[%s] Query: matcher: %s\nselector: "
                     "%s\norderBy: %s\nhint:%s", sessionName(),
                     matcher.toString().c_str(), selector.toString().c_str(),
                     orderBy.toString().c_str(), hint.toString().c_str() ) ;

            rc = rtnQuery( pCollectionName, selector, matcher, orderBy,
                           hint, flags, _pEDUCB, numToSkip, numToReturn,
                           _pDMSCB, _pRTNCB, contextID, &pContext, TRUE ) ;
            if ( rc )
            {
               goto error ;
            }

            if ( ( flags & FLG_QUERY_WITH_RETURNDATA ) && NULL != pContext )
            {
               INT64 startPos64 = 0 ;
               rc = pContext->getMore( -1, buffObj, startPos64, _pEDUCB ) ;
               if ( rc || pContext->eof() )
               {
                  _pRTNCB->contextDelete( contextID, _pEDUCB ) ;
                  contextID = -1 ;
               }
               startingPos = ( INT32 )startPos64 ;

               if ( SDB_DMS_EOC == rc )
               {
                  rc = SDB_OK ;
               }
               else if ( rc )
               {
                  PD_LOG( PDERROR, "Session[%s] failed to query with return "
                          "data, rc: %d", sessionName(), rc ) ;
                  goto error ;
               }
            }
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR, "Session[%s] Failed to create matcher and "
                     "selector for QUERY: %s", sessionName(), e.what () ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      else
      {
         rc = rtnParserCommand( pCollectionName, &pCommand ) ;

         if ( SDB_OK != rc )
         {
            PD_LOG ( PDERROR, "Parse command[%s] failed[rc:%d]",
                     pCollectionName, rc ) ;
            goto error ;
         }

         rc = rtnInitCommand( pCommand , flags, numToSkip, numToReturn,
                              pQueryBuff, pFieldSelector, pOrderByBuffer,
                              pHintBuffer ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         PD_LOG ( PDDEBUG, "Command: %s", pCommand->name () ) ;

         //run command
         rc = rtnRunCommand( pCommand, getServiceType(),
                             _pEDUCB, _pDMSCB, _pRTNCB,
                             _pDPSCB, 1, &contextID ) ;
         if ( rc )
         {
            goto error ;
         }
      }

   done:
      if ( pCommand )
      {
         rtnReleaseCommand( &pCommand ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onGetMoreReqMsg( MsgHeader * msg,
                                             rtnContextBuf &buffObj,
                                             INT32 &startingPos,
                                             INT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      INT32 numToRead = 0 ;
      INT64 startPos64 = 0 ;

      rc = msgExtractGetMore ( (CHAR*)msg, &numToRead, &contextID ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract get more msg failed, "
                   "rc: %d", sessionName(), rc ) ;

      // add last op info
      MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                          "ContextID:%lld, NumToRead:%d",
                          contextID, numToRead ) ;

      PD_LOG ( PDDEBUG, "GetMore: contextID:%lld\nnumToRead: %d", contextID,
               numToRead ) ;

      rc = rtnGetMore ( contextID, numToRead, buffObj, startPos64,
                        _pEDUCB, _pRTNCB ) ;

      startingPos = ( INT32 )startPos64 ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onKillContextsReqMsg( MsgHeader *msg )
   {
      PD_LOG ( PDDEBUG, "session[%s] _onKillContextsReqMsg", sessionName() ) ;

      INT32 rc = SDB_OK ;
      INT32 contextNum = 0 ;
      INT64 *pContextIDs = NULL ;

      rc = msgExtractKillContexts ( (CHAR*)msg, &contextNum, &pContextIDs ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract kill contexts msg failed, "
                   "rc: %d", sessionName(), rc ) ;

      if ( contextNum > 0 )
      {
         PD_LOG ( PDDEBUG, "KillContext: contextNum:%d\ncontextID: %lld",
                  contextNum, pContextIDs[0] ) ;
      }

      rc = rtnKillContexts ( contextNum, pContextIDs, _pEDUCB, _pRTNCB ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onSQLMsg( MsgHeader *msg, INT64 &contextID )
   {
      CHAR *sql = NULL ;
      INT32 rc = SDB_OK ;
      SQL_CB *sqlcb = pmdGetKRCB()->getSqlCB() ;

      rc = msgExtractSql( (CHAR*)msg, &sql ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract sql msg failed, rc: %d",
                   sessionName(), rc ) ;

      rc = sqlcb->exec( sql, _pEDUCB, contextID ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onTransBeginMsg ()
   {
      INT32 rc = SDB_OK ;
      if ( pmdGetDBRole() != SDB_ROLE_STANDALONE )
      {
         rc = SDB_PERM ;
         PD_LOG( PDERROR, "In sharding mode, couldn't execute "
                 "transaction operation from local service" ) ;
         goto error ;
      }
      else
      {
         rc = rtnTransBegin( _pEDUCB ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onTransCommitMsg ()
   {
      INT32 rc = SDB_OK ;
      if ( pmdGetDBRole() != SDB_ROLE_STANDALONE )
      {
         rc = SDB_PERM ;
         PD_LOG( PDERROR, "In sharding mode, couldn't execute "
                 "transaction operation from local service" ) ;
         goto error ;
      }
      else
      {
         rc = rtnTransCommit( _pEDUCB, _pDPSCB ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onTransRollbackMsg ()
   {
      INT32 rc = SDB_OK ;
      if ( pmdGetDBRole() != SDB_ROLE_STANDALONE )
      {
         rc = SDB_PERM ;
         PD_LOG( PDERROR, "In sharding mode, couldn't execute "
                 "transaction operation from local service" ) ;
         goto error ;
      }
      else
      {
         rc = rtnTransRollback( _pEDUCB, _pDPSCB ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onAggrReqMsg( MsgHeader *msg, INT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      CHAR *pCollectionName = NULL ;
      CHAR *pObjs = NULL ;
      INT32 count = 0 ;
      INT32 flags = 0 ;

      rc = msgExtractAggrRequest( (CHAR*)msg, &pCollectionName,
                                  &pObjs, count, &flags ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extrace aggr msg failed, rc: %d",
                   sessionName(), rc ) ;

      try
      {
         BSONObj objs( pObjs ) ;
         rc = rtnAggregate( pCollectionName, objs, count, flags, _pEDUCB,
                            _pDMSCB, contextID ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Session[%s] occurred exception in aggr: %s",
                 sessionName(), e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onOpenLobMsg( MsgHeader *msg,
                                          SINT64 &contextID,
                                          rtnContextBuf &buffObj )
   {
      INT32 rc = SDB_OK ;
      const MsgOpLob *header = NULL ;
      BSONObj lob ;
      BSONObj meta ;
      rc = msgExtractOpenLobRequest( ( const CHAR * )msg, &header, lob ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract open msg:%d", rc ) ;
         goto error ;
      }

      rc = rtnOpenLob( lob, header->flags, TRUE, _pEDUCB,
                       _pDPSCB, header->w, contextID, meta ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open lob:%d", rc ) ;
         goto error ;
      }

      buffObj = rtnContextBuf( meta.objdata(), meta.objsize(), 1 ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onWriteLobMsg( MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      const MsgOpLob *header = NULL ;
      UINT32 len = 0 ;
      SINT64 offset = -1 ;
      const CHAR *data = NULL ;
      rc = msgExtractWriteLobRequest( ( const CHAR * )msg, &header,
                                        &len, &offset, &data ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract write msg:%d", rc ) ;
         goto error ;
      }

      rc = rtnWriteLob( header->contextID, _pEDUCB,
                        len, data ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to write lob:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onReadLobMsg( MsgHeader *msg,
                                          rtnContextBuf &buffObj )
   {
      INT32 rc = SDB_OK ;
      const MsgOpLob *header = NULL ;
      SINT64 offset = -1 ;
      UINT32 readLen = 0 ;
      UINT32 length = 0 ;
      const CHAR *data = NULL ;

      rc = msgExtractReadLobRequest( ( const CHAR * )msg, &header,
                                      &readLen, &offset ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract read msg:%d", rc ) ;
         goto error ;
      }

      rc = rtnReadLob( header->contextID, _pEDUCB,
                       readLen, offset, &data, length ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read lob:%d", rc ) ;
         goto error ;
      }

      buffObj = rtnContextBuf( data, length, 0 ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onCloseLobMsg( MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      const MsgOpLob *header = NULL ;
      rc = msgExtractCloseLobRequest( ( const CHAR * )msg, &header ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract close msg:%d", rc ) ;
         goto error ;
      }

      rc = rtnCloseLob( header->contextID, _pEDUCB ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to close lob:%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onRemoveLobMsg( MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      BSONObj meta ;
      const MsgOpLob *header = NULL ;
      rc = msgExtractRemoveLobRequest( ( const CHAR * )msg, &header,
                                        meta ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract remove msg:%d", rc ) ;
         goto error ;
      }

      rc = rtnRemoveLob( meta, header->w,
                         _pEDUCB, _pDPSCB ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to remove lob:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onGetLobMeta( MsgHeader *msg,
                                          rtnContextBuf &buffObj )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      const MsgOpLob *header = NULL ;

      rc = msgExtractLobRequest( ( const CHAR * )msg, &header,
                                 obj, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract get meat msg:%d", rc ) ;
         goto error ;
      }

      rc = rtnGetLobMetaData( header->contextID, _pEDUCB, obj ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get meta data:%d", rc ) ;
         goto error ;
      }

      buffObj = rtnContextBuf( obj.objdata(), obj.objsize(), 1 ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}


