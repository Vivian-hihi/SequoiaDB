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

using namespace bson ;

namespace engine
{
   const UINT32 SESSION_SOCKET_DFT_TIMEOUT = 10 ;
   const UINT32 SESSION_MEM_ALIGMENT_SIZE  = 1024 ;
   const UINT32 SESSION_MAX_CATCH_SIZE     = 16*1024*1024 ;

   /*
      _pmdSession implement
   */
   _pmdSession::_pmdSession( SOCKET fd )
   :_socket( &fd, SESSION_SOCKET_DFT_TIMEOUT )
   {
      _pEDUCB  = NULL ;
      _eduID   = PMD_INVALID_EDUID ;
      _pBuff   = NULL ;
      _buffLen = 0 ;

      _totalCatchSize   = 0 ;
      _totalMemSize     = 0 ;

      _socket.disableNagle() ;

      // make session name
      if ( SOCKET_INVALIDSOCKET != fd )
      {
         CHAR tmpName [ 128 ] = {0} ;
         _socket.getPeerAddress( tmpName, sizeof( tmpName ) -1 ) ;
         _sessionName = tmpName ;
         _sessionName += ":" ;
         ossSnprintf( tmpName, sizeof( tmpName ) -1, "%d",
                      _socket.getPeerPort() ) ;
         _sessionName += tmpName ;
      }
   }

   _pmdSession::~_pmdSession()
   {
      clear() ;
   }

   void _pmdSession::clear ()
   {
      // release buff
      if ( _pBuff )
      {
         releaseBuff( _pBuff, _buffLen ) ;
         _pBuff = NULL ;
      }
      _buffLen = 0 ;

      // clean catch
      CATCH_MAP_IT it = _catchMap.begin() ;
      while ( it != _catchMap.end() )
      {
         SDB_OSS_FREE( it->second ) ;
         _totalCatchSize -= it->first ;
         _totalMemSize -= it->first ;
         ++it ;
      }
      _catchMap.clear() ;

      // clean alloc memory
      ALLOC_MAP_IT itAlloc = _allocMap.begin() ;
      while ( itAlloc != _allocMap.end() )
      {
         SDB_OSS_FREE( itAlloc->first ) ;
         _totalMemSize -= itAlloc->second ;
         ++itAlloc ;
      }
      _allocMap.clear() ;

      SDB_ASSERT( _totalCatchSize == 0 , "Catch size is error" ) ;
      SDB_ASSERT( _totalMemSize == 0, "Memory size is error" ) ;
   }

   void _pmdSession::attach( _pmdEDUCB * cb )
   {
      SDB_ASSERT( cb, "cb can't be NULL" ) ;

      PD_LOG( PDINFO, "Session[%s] attach edu[%d]", sessionName(),
              cb->getID() ) ;

      _pEDUCB = cb ;
      _eduID  = cb->getID() ;
      _pEDUCB->attachSession( this ) ;
      _pEDUCB->setName( sessionName() ) ;
      _pEDUCB->setClientSock( socket() ) ;

      _onAttach() ;
   }

   void _pmdSession::detach ()
   {
      PD_LOG( PDINFO, "Session[%s] detach edu[%d]", sessionName(),
              eduID() ) ;

      _onDetach() ;
      _pEDUCB->detachSession() ;
      _pEDUCB = NULL ;
   }

   const CHAR* _pmdSession::sessionName () const
   {
      return _sessionName.c_str() ;
   }

   INT32 _pmdSession::allocBuff( INT32 len, CHAR **ppBuff, INT32 &buffLen )
   {
      INT32 rc = SDB_OK ;

      // first alloc from catch
      if ( _totalCatchSize >= len && _allocFromCatch( len, ppBuff, buffLen ) )
      {
         goto done ;
      }

      // malloc
      len = ossRoundUpToMultipleX( len, SESSION_MEM_ALIGMENT_SIZE ) ;
      *ppBuff = ( CHAR* )SDB_OSS_MALLOC( len ) ;
      if( !*ppBuff )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Session[%s] malloc memory[size: %d] failed",
                 sessionName(), len ) ;
         goto error ;
      }
      buffLen = len ;

      // update meta info
      _totalMemSize += buffLen ;
      _allocMap[ *ppBuff ] = buffLen ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _pmdSession::releaseBuff( CHAR *pBuff, INT32 buffLen )
   {
      ALLOC_MAP_IT itAlloc = _allocMap.find( pBuff ) ;
      if ( itAlloc == _allocMap.end() )
      {
         SDB_OSS_FREE( pBuff ) ;
         return ;
      }
      buffLen = itAlloc->second ;
      _allocMap.erase( itAlloc ) ;

      if ( (UINT32)buffLen > SESSION_MAX_CATCH_SIZE )
      {
         SDB_OSS_FREE( pBuff ) ;
         _totalMemSize -= buffLen ;
      }
      else
      {
         // add to catch
         _catchMap.insert( std::make_pair( buffLen, pBuff ) ) ;
         _totalCatchSize += buffLen ;

         // re-org catch
         while ( _totalCatchSize > SESSION_MAX_CATCH_SIZE )
         {
            CATCH_MAP_IT it = _catchMap.begin() ;
            SDB_OSS_FREE( it->second ) ;
            _totalMemSize -= it->first ;
            _totalCatchSize -= it->first ;
            _catchMap.erase( it ) ;
         }
      }
   }

   INT32 _pmdSession::reallocBuff( INT32 len, CHAR **ppBuff, INT32 &buffLen )
   {
      INT32 rc = SDB_OK ;
      CHAR *pOld = *ppBuff ;
      INT32 oldLen = buffLen ;

      ALLOC_MAP_IT itAlloc = _allocMap.find( *ppBuff ) ;
      if ( itAlloc != _allocMap.end() )
      {
         buffLen = itAlloc->second ;
         oldLen = buffLen ;
      }
      else if ( *ppBuff != NULL || buffLen != 0 )
      {
         PD_LOG( PDERROR, "Session[%s] realloc input buffer error",
                 sessionName() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( buffLen >= len )
      {
         goto done ;
      }
      len = ossRoundUpToMultipleX( len, SESSION_MEM_ALIGMENT_SIZE ) ;
      *ppBuff = ( CHAR* )SDB_OSS_REALLOC( *ppBuff, len ) ;
      if ( !*ppBuff )
      {
         PD_LOG( PDERROR, "Failed to realloc memory, size: %d", len ) ;
         goto error ;
      }

      buffLen = len ;

      // update meta info
      _totalMemSize += ( len - oldLen ) ;

      _allocMap[ *ppBuff ] = buffLen ;

   done:
      return rc ;
   error:
      if ( pOld )
      {
         releaseBuff( pOld, oldLen ) ;
         *ppBuff = NULL ;
         buffLen = 0 ;
      }
      goto done ;
   }

   CHAR* _pmdSession::getBuff( INT32 len )
   {
      if ( _buffLen < len )
      {
         if ( _pBuff )
         {
            releaseBuff( _pBuff, _buffLen ) ;
            _pBuff = NULL ;
         }
         _buffLen = 0 ;

         allocBuff( len, &_pBuff, _buffLen ) ;
      }

      return _pBuff ;
   }

   BOOLEAN _pmdSession::_allocFromCatch( INT32 len, CHAR **ppBuff,
                                         INT32 &buffLen )
   {
      CATCH_MAP_IT it = _catchMap.lower_bound( len ) ;
      if ( it != _catchMap.end() )
      {
         *ppBuff = it->second ;
         buffLen = it->first ;
         _catchMap.erase( it ) ;
         _allocMap[ *ppBuff ] = buffLen ;
         _totalCatchSize -= buffLen ;
         return TRUE ;
      }
      return FALSE ;
   }

   void _pmdSession::disconnect()
   {
      _socket.close() ;
   }

   INT32 _pmdSession::sendData( const CHAR * pData, INT32 size,
                                INT32 timeout, BOOLEAN block,
                                INT32 *pSentLen, INT32 flags )
   {
      INT32 rc = SDB_OK ;
      INT32 sentSize = 0 ;
      INT32 totalSentSize = 0 ;
      INT32 realTimeout = timeout < 0 ? OSS_SOCKET_DFT_TIMEOUT : timeout ;

      while ( TRUE )
      {
         if ( _pEDUCB && _pEDUCB->isForced () )
         {
            rc = SDB_APP_FORCED ;
            goto done ;
         }
         rc = _socket.send ( &pData[totalSentSize], size-totalSentSize,
                             sentSize, realTimeout, flags, block ) ;
         totalSentSize += sentSize ;
         if ( timeout < 0 && SDB_TIMEOUT == rc )
         {
            continue ;
         }
         break ;
      }

   done :
      if ( totalSentSize > 0 )
      {
         pmdGetKRCB()->getMonDBCB()->svcNetOutAdd( totalSentSize ) ;
      }
      if ( pSentLen )
      {
         *pSentLen = totalSentSize ;
      }
      return rc ;
   }

   INT32 _pmdSession::recvData( CHAR * pData, INT32 size, INT32 timeout,
                                BOOLEAN block, INT32 *pRecvLen, INT32 flags )
   {
      INT32 rc = SDB_OK ;
      INT32 receivedSize = 0 ;
      INT32 totalReceivedSize = 0 ;
      INT32 realTimeout = timeout < 0 ? OSS_SOCKET_DFT_TIMEOUT : timeout ;

      while ( TRUE )
      {
         if ( _pEDUCB && _pEDUCB->isForced () )
         {
            rc = SDB_APP_FORCED ;
            goto done ;
         }
         rc = _socket.recv ( &pData[totalReceivedSize], size-totalReceivedSize,
                             receivedSize, realTimeout, flags, block ) ;
         totalReceivedSize += receivedSize ;
         if ( timeout < 0 && SDB_TIMEOUT == rc )
         {
            continue ;
         }
         break ;
      }

   done :
      if ( totalReceivedSize > 0 )
      {
         pmdGetKRCB()->getMonDBCB()->svcNetInAdd( totalReceivedSize ) ;
      }
      if ( pRecvLen )
      {
         *pRecvLen = totalReceivedSize ;
      }
      return rc ;
   }

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

      if ( !_pEDUCB )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      pmdEDUMgr               = _pEDUCB->getEDUMgr() ;

      while ( !_pEDUCB->isDisconnected() && !_socket.isClosed() )
      {
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
                    msgSize, sizeof(MsgHeader), SDB_MAX_MSG_LENGTH ) ;
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

      disconnect() ;

   done:
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
      // recv rest
      rc = recvData( *ppBuff + sizeof(UINT32), recvSize - sizeof(UINT32) ) ;
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

      if ( SDB_ROLE_STANDALONE == pmdGetDBRole() ) // not auth
      {
         _authOK = TRUE ;
         goto done ;
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

      if ( SDB_DMS_EOC != result )
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
         else
         {
            rc = _processOPMsg( msg, _replyHeader.contextID, &pBody,
                                bodyLen, _replyHeader.numReturned,
                                _replyHeader.startFrom ) ;
         }
      }

      if ( _needReply )
      {
         if ( rc && bodyLen == 0 )
         {
            _errorInfo = pmdGetErrorBson( rc, _pEDUCB->getInfo(
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
            rc = _onQueryReqMsg( msg, contextID ) ;
            break ;
         case MSG_BS_DELETE_REQ :
            rc = _onDelReqMsg( msg ) ;
            break ;
         case MSG_BS_GETMORE_REQ :
            rc = _onGetMoreReqMsg( msg, _contextBuff, startPos, contextID ) ;
            if ( SDB_OK == rc )
            {
               *ppBody     = _contextBuff.data() ;
               bodyLen     = _contextBuff.size() ;
               returnNum   = _contextBuff.recordNum() ;
            }
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

   INT32 _pmdLocalSession::_onQueryReqMsg( MsgHeader * msg, INT64 &contextID )
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
                           _pDMSCB, _pRTNCB, contextID, NULL, TRUE ) ;
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

}


