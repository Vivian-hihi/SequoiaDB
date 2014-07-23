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

   Source File Name = pmdRemoteSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/05/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdRemoteSession.hpp"
#include "pmdEDU.hpp"

#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   /*
      DEFINES
   */
   #define PMD_MAX_IDLE_REMOTE_SESSIONS         ( 200 )

   /*
      pmdSubSession implement
   */
   _pmdSubSession::_pmdSubSession()
   {
      _nodeID           = 0 ;
      _reqID            = 0 ;
      _pReqMsg          = NULL ;
      _pRspMsg          = NULL ;
      _isSend           = FALSE ;
      _isDisconnect     = TRUE ;

      _isProcessed      = FALSE ;
      _processResult    = SDB_OK ;
   }

   _pmdSubSession::~_pmdSubSession()
   {
   }

   void _pmdSubSession::clearReplyInfo()
   {
      _pRspMsg       = NULL ;
      _isProcessed   = FALSE ;
      _processResult = SDB_OK ;
      _isDisconnect  = FALSE ;
   }

   /*
      _pmdRemoteSession implement
   */
   _pmdRemoteSession::_pmdRemoteSession( netRouteAgent *pAgent,
                                         UINT64 sessionID,
                                         INT64 timeout,
                                         IRemoteSessionHandler *pHandle )
   {
      _pAgent        = pAgent ;
      _pHandle       = pHandle ;
      _pEDUCB        = NULL ;
      _sessionID     = sessionID ;
   }

   _pmdRemoteSession::~_pmdRemoteSession()
   {
      _pAgent        = NULL ;
      _pHandle       = NULL ;
   }

   void _pmdRemoteSession::clear()
   {
      // TODO:XUJIANHUI
   }

   void _pmdRemoteSession::reset( UINT64 sessionID,
                                  INT64 timeout ,
                                  IRemoteSessionHandler *pHandle )
   {
      // TODO:XUJIANHUI
   }

   pmdSubSession* _pmdRemoteSession::getSubSession( UINT64 nodeID )
   {
      MAP_SUB_SESSION_IT it = _mapSubSession.find( nodeID ) ;
      if ( it == _mapSubSession.end() )
      {
         return NULL ;
      }
      return &( it->second ) ;
   }

   void _pmdRemoteSession::delSubSession( UINT64 nodeID )
   {
      _mapSubSession.erase( nodeID ) ;
   }

   INT32 _pmdRemoteSession::disconnSubSession( UINT64 nodeID )
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   void _pmdRemoteSession::clearSubSession()
   {
      _mapSubSession.clear() ;
   }

   UINT32 _pmdRemoteSession::getSubSessionCount()
   {
      return 0 ;
   }

   UINT32 _pmdRemoteSession::getReplyCount( BOOLEAN exceptProcessed )
   {
      return 0 ;
   }

   UINT32 _pmdRemoteSession::getSucReplyCount()
   {
      return 0 ;
   }

   BOOLEAN _pmdRemoteSession::isTimeout()
   {
      return FALSE ;
   }

   BOOLEAN _pmdRemoteSession::isAllReply()
   {
      return FALSE ;
   }

   MAP_SUB_SESSION* _pmdRemoteSession::getSubSessions()
   {
      return &_mapSubSession ;
   }

   pmdSubSession* _pmdRemoteSession::addSubSession( UINT64 nodeID )
   {
      pmdSubSession &subSession = _mapSubSession[ nodeID ] ;
      if ( subSession.getNodeID() != nodeID )
      {
         subSession.setNodeID( nodeID ) ;
      }
      return &subSession ;
   }

   INT32 _pmdRemoteSession::sendMsg( MsgHeader * pSrcMsg, INT32 *pSucNum,
                                     INT32 *pTotalNum )
   {
      INT32 rc = SDB_OK ;

      MAP_SUB_SESSION_IT it = _mapSubSession.begin() ;
      while ( it != _mapSubSession.end() )
      {
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRemoteSession::sendMsg( MsgHeader *pSrcMsg,
                                     SET_SUB_SESSIONID &subs,
                                     INT32 *pSucNum,
                                     INT32 *pTotalNUm )
   {
      INT32 rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRemoteSession::sendMsg( INT32 *pSucNum, INT32 *pTotalNum )
   {
      INT32 rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRemoteSession::sendMsg( UINT64 nodeID )
   {
      INT32 rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRemoteSession::waitReply( BOOLEAN waitAll,
                                       VEC_SUB_SESSIONPTR *pVecSubs,
                                       INT64 millisec )
   {
      INT32 rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _pmdRemoteSessionMgr implement
   */
   _pmdRemoteSessionMgr::_pmdRemoteSessionMgr()
   {
      _pAgent = NULL ;
      _sessionHWNum = 1 ;
   }

   _pmdRemoteSessionMgr::~_pmdRemoteSessionMgr()
   {
      SDB_ASSERT( _mapSessions.size() == 0, "Session must be Zero" ) ;

      // clear euds
      _mapTID2EDU.clear() ;

      // release idle sessions
      for ( UINT32 i = 0 ; i < _idleSessions.size() ; ++i )
      {
         SDB_OSS_DEL _idleSessions[ i ] ;
      }
      _idleSessions.clear() ;

      // release session
      MAP_REMOTE_SESSION_IT it = _mapSessions.begin() ;
      while ( it != _mapSessions.end() )
      {
         SDB_OSS_DEL it->second ;
         ++it ;
      }
      _mapSessions.clear() ;

      _pAgent = NULL ;
   }

   INT32 _pmdRemoteSessionMgr::init( netRouteAgent * pAgent )
   {
      if ( !pAgent )
      {
         return SDB_INVALIDARG ;
      }
      _pAgent = pAgent ;

      return SDB_OK ;
   }

   INT32 _pmdRemoteSessionMgr::fini()
   {
      SDB_ASSERT( _mapSessions.size() == 0, "Session must be Zero" ) ;
      SDB_ASSERT( _mapTID2EDU.size() == 0, "EDU must be Zero" ) ;

      return SDB_OK ;
   }

   void _pmdRemoteSessionMgr::registerEDU( _pmdEDUCB * cb )
   {
      ossScopedLock lock( &_edusLatch, EXCLUSIVE ) ;
      _mapTID2EDU[ cb->getTID() ] = cb ;
   }

   void _pmdRemoteSessionMgr::unregEUD( _pmdEDUCB * cb )
   {
      ossScopedLock lock( &_edusLatch, EXCLUSIVE ) ;
      _mapTID2EDU.erase( cb->getTID() ) ;
   }

   INT32 _pmdRemoteSessionMgr::pushMessage( const NET_HANDLE &handle,
                                            const MsgHeader *pMsg )
   {
      INT32 rc = SDB_OK ;
      pmdEDUCB *pEDUCB = NULL ;
      MAP_TID_2_EDU_IT it ;

      ossScopedLock lock( &_edusLatch, SHARED ) ;

      it = _mapTID2EDU.find( pMsg->TID ) ;
      if ( it != _mapTID2EDU.end() )
      {
         CHAR *pNewBuff = NULL ;
         pEDUCB = it->second ;

         // assign memory
         pNewBuff = ( CHAR* )SDB_OSS_MALLOC( pMsg->messageLength + 1 ) ;
         if ( pNewBuff )
         {
            // copy data
            ossMemcpy( pNewBuff, pMsg, pMsg->messageLength ) ;
            pNewBuff[ pMsg->messageLength ] = 0 ;
            // push to edu queue
            pEDUCB->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                            PMD_EDU_MEM_ALLOC,
                                            pNewBuff, (UINT64)handle ) ) ;
         }
         else
         {
            PD_LOG( PDERROR, "Failed to alloc memory[size: %d] for msg["
                    "opCode: (%d)%u, TID: %d, ReqID: %llu, NodeID: %u.%u.%u]",
                    pMsg->messageLength, IS_REPLY_TYPE(pMsg->opCode),
                    GET_REQUEST_TYPE(pMsg->opCode), pMsg->TID,
                    pMsg->requestID, pMsg->routeID.columns.groupID,
                    pMsg->routeID.columns.nodeID,
                    pMsg->routeID.columns.serviceID ) ;
         }
      }
      else
      {
         PD_LOG( PDWARNING, "Can't find remote session[TID=%d] for msg["
                 "opCode: (%d)%u, ReqID: %llu, NodeID: %u.%u.%u, Len: %u]",
                 pMsg->TID, IS_REPLY_TYPE(pMsg->opCode),
                 GET_REQUEST_TYPE(pMsg->opCode), pMsg->requestID,
                 pMsg->routeID.columns.groupID, pMsg->routeID.columns.nodeID,
                 pMsg->routeID.columns.serviceID, pMsg->messageLength ) ;
         rc = SDB_INVALIDARG ;
      }

      return rc ;
   }

   void _pmdRemoteSessionMgr::handleClose( const NET_HANDLE &handle,
                                           const _MsgRouteID &id )
   {
      pmdEDUCB *cb = NULL ;
      MsgOpReply *pMsg = NULL ;

      if ( sessionCount() > 0 )
      {
         // push disconnect reply to each edu
         ossScopedLock lock( &_edusLatch, SHARED ) ;
         MAP_TID_2_EDU_IT it = _mapTID2EDU.begin() ;
         while ( it != _mapTID2EDU.end() )
         {
            cb = it->second ;
            pMsg = ( MsgOpReply* )SDB_OSS_MALLOC( sizeof( MsgOpReply ) ) ;
            if ( pMsg )
            {
               pMsg->header.messageLength = sizeof( MsgOpReply ) ;
               pMsg->header.opCode = MSG_BS_DISCONNECT ;
               pMsg->header.requestID = cb->incCurRequestID() ;
               pMsg->header.TID = cb->getTID() ;
               pMsg->header.routeID.value = id.value ;
               pMsg->contextID = -1 ;
               pMsg->flags = SDB_COORD_REMOTE_DISC ;
               pMsg->numReturned = 0 ;
               pMsg->startFrom = 0 ;

               cb->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG, PMD_EDU_MEM_ALLOC,
                                           (CHAR*)pMsg, (UINT64)handle ) ) ;
            }
            ++it ;
         }
      }
   }

   pmdRemoteSession* _pmdRemoteSessionMgr::addSession( _pmdEDUCB * cb,
                                                        INT64 timeout,
                                                        IRemoteSessionHandler *pHandle )
   {
      pmdRemoteSession *pSession = NULL ;
      UINT64 sessionID = 0 ;

      ossScopedLock lock( &_mapLatch, EXCLUSIVE ) ;

      sessionID = _sessionHWNum++ ;

      // if idle session not empty
      if ( !_idleSessions.empty() )
      {
         pSession = _idleSessions.back() ;
         pSession->reset( sessionID, timeout, pHandle ) ;
         _idleSessions.pop_back() ;
      }
      else
      {
         pSession = SDB_OSS_NEW _pmdRemoteSession( _pAgent, sessionID,
                                                   timeout, pHandle ) ;
         if ( !pSession )
         {
            --_sessionHWNum ;
            PD_LOG( PDERROR, "Failed to alloc remote session" ) ;
            goto error ;
         }
      }

      pSession->attachCB( cb ) ;
      _mapSessions[ sessionID ] = pSession ;

   done:
      return pSession ;
   error:
      goto done ;
   }

   pmdRemoteSession* _pmdRemoteSessionMgr::getSession( UINT64 sessionID )
   {
      pmdRemoteSession *pSession = NULL ;
      MAP_REMOTE_SESSION_IT it ;

      ossScopedLock lock( &_mapLatch, SHARED ) ;
      it = _mapSessions.find( sessionID ) ;
      if ( it != _mapSessions.end() )
      {
         pSession = it->second ;
      }

      return pSession ;
   }

   void _pmdRemoteSessionMgr::removeSession( UINT64 sessionID )
   {
      ossScopedLock lock( &_mapLatch, EXCLUSIVE ) ;
      MAP_REMOTE_SESSION_IT it = _mapSessions.find( sessionID ) ;
      if ( it != _mapSessions.end() )
      {
         it->second->detachCB() ;
         it->second->clear() ;

         if ( _idleSessions.size() < PMD_MAX_IDLE_REMOTE_SESSIONS )
         {
            // add to idle vector
            _idleSessions.push_back( it->second ) ;
         }
         else
         {
            SDB_OSS_DEL it->second ;
         }
         _mapSessions.erase( it ) ;
      }
   }

   void _pmdRemoteSessionMgr::removeSession( pmdRemoteSession *pSession )
   {
      return removeSession( pSession->sessionID() ) ;
   }

   UINT32 _pmdRemoteSessionMgr::sessionCount()
   {
      ossScopedLock lock( &_mapLatch, SHARED ) ;
      return (UINT32)_mapSessions.size() ;
   }

}

