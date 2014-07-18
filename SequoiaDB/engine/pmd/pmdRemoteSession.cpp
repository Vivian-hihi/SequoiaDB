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
                                         INT64 timeout,
                                         IRemoteSessionHandler *pHandle )
   {
      _pAgent        = pAgent ;
      _pHandle       = pHandle ;
      _pEDUCB        = NULL ;
   }

   _pmdRemoteSession::~_pmdRemoteSession()
   {
      _pAgent        = NULL ;
      _pHandle       = NULL ;
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
   }

   _pmdRemoteSessionMgr::~_pmdRemoteSessionMgr()
   {
      SDB_ASSERT( _mapSessions.size() == 0, "Session must be Zero" ) ;

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
      return SDB_OK ;
   }

   INT32 _pmdRemoteSessionMgr::pushMessage( const NET_HANDLE &handle,
                                            const MsgHeader *pMsg )
   {
      INT32 rc = SDB_OK ;
      pmdEDUCB *pEDUCB = NULL ;
      MAP_REMOTE_SESSION_IT it ;

      ossScopedLock lock( &_mapLatch, SHARED ) ;

      it = _mapSessions.find( pMsg->TID ) ;
      if ( it != _mapSessions.end() )
      {
         CHAR *pNewBuff = NULL ;
         INT32 buffLen = 0 ;

         pEDUCB = it->second->getEDUCB() ;

         // assign memory
         rc = pEDUCB->allocBuff( pMsg->messageLength, &pNewBuff, buffLen ) ;
         if ( SDB_OK == rc )
         {
            // copy data
            ossMemcpy( pNewBuff, pMsg, pMsg->messageLength ) ;
            // push to edu queue
            pEDUCB->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                            PMD_EDU_MEM_SELF,
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
   }

   pmdRemoteSession* _pmdRemoteSessionMgr::addSession( _pmdEDUCB * cb,
                                                       INT64 timeout,
                                                       IRemoteSessionHandler *pHandle )
   {
      pmdRemoteSession *pSession = NULL ;
      MAP_REMOTE_SESSION_IT it ;

      ossScopedLock lock( &_mapLatch, EXCLUSIVE ) ;
      it = _mapSessions.find( cb->getTID() ) ;
      if ( it != _mapSessions.end() )
      {
         // already exist
         pSession = it->second ;
         goto done ;
      }

      pSession = SDB_OSS_NEW _pmdRemoteSession( _pAgent, timeout, pHandle ) ;
      if ( pSession )
      {
         pSession->attachCB( cb ) ;
         _mapSessions[ cb->getTID() ] = pSession ;
      }

   done:
      return pSession ;
   }

   pmdRemoteSession* _pmdRemoteSessionMgr::getSession( UINT32 tid )
   {
      pmdRemoteSession *pSession = NULL ;
      MAP_REMOTE_SESSION_IT it ;

      ossScopedLock lock( &_mapLatch, SHARED ) ;
      it = _mapSessions.find( tid ) ;
      if ( it != _mapSessions.end() )
      {
         pSession = it->second ;
      }

      return pSession ;
   }

   void _pmdRemoteSessionMgr::removeSession( UINT32 tid )
   {
      ossScopedLock lock( &_mapLatch, EXCLUSIVE ) ;
      MAP_REMOTE_SESSION_IT it = _mapSessions.find( tid ) ;
      if ( it != _mapSessions.end() )
      {
         SDB_OSS_DEL it->second ;
         _mapSessions.erase( it ) ;
      }
   }

   UINT32 _pmdRemoteSessionMgr::sessionCount()
   {
      ossScopedLock lock( &_mapLatch, SHARED ) ;
      return (UINT32)_mapSessions.size() ;
   }

}

