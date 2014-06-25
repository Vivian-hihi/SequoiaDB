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
                                         IRemoteSessionHandler *pHandle )
   {
      _pAgent        = pAgent ;
      _pHandle       = pHandle ;
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
   _pmdRemoteSessionMgr::_pmdRemoteSessionMgr( netRouteAgent *pAgent )
   {
   }

   _pmdRemoteSessionMgr::~_pmdRemoteSessionMgr()
   {
   }

   INT32 _pmdRemoteSessionMgr::pushMessage( const NET_HANDLE &handle,
                                            const MsgHeader *pMsg )
   {
      return SDB_OK ;
   }

   void _pmdRemoteSessionMgr::handleClose( const NET_HANDLE &handle,
                                           const _MsgRouteID &id )
   {
   }

   pmdRemoteSession* _pmdRemoteSessionMgr::addSession( _pmdEDUCB * cb,
                                                       INT64 timeout,
                                                       IRemoteSessionHandler *pHandle )
   {
      return NULL ;
   }

   pmdRemoteSession* _pmdRemoteSessionMgr::getSession( UINT32 tid )
   {
      return NULL ;
   }

   void _pmdRemoteSessionMgr::removeSession( UINT32 tid )
   {
   }

}

