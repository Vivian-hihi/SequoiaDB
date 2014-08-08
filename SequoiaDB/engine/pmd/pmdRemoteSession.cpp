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
#include "msgMessage.hpp"

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
      _parent           = NULL ;
      _nodeID.value     = MSG_INVALID_ROUTEID ;
      _reqID            = 0 ;
      _pReqMsg          = NULL ;
      _isSend           = FALSE ;
      _isDisconnect     = TRUE ;
      _memType          = PMD_EDU_MEM_NONE ;

      _isProcessed      = FALSE ;
      _processResult    = SDB_OK ;
      _userData         = 0 ;
      _needToDel        = FALSE ;
   }

   _pmdSubSession::~_pmdSubSession()
   {
      _parent = NULL ;
   }

   void _pmdSubSession::clearReplyInfo()
   {
      _isDisconnect  = FALSE ;

      if ( _event._Data && PMD_EDU_MEM_NONE != _event._eventType )
      {
         pmdEduEventRelase( _event, _parent->getEDUCB() ) ;
      }
      _event.reset() ;
   }

   void _pmdSubSession::clearRequestInfo()
   {
      _ioDatas.clear() ;

      if ( _pReqMsg )
      {
         if ( PMD_EDU_MEM_ALLOC == _memType )
         {
            SDB_OSS_FREE( (void*)_pReqMsg ) ;
         }
         else if ( PMD_EDU_MEM_SELF == _memType )
         {
            _parent->getEDUCB()->releaseBuff( (CHAR*)_pReqMsg ) ;
         }
         _pReqMsg = NULL ;
      }
      _memType = PMD_EDU_MEM_NONE ;
   }

   void _pmdSubSession::addIODatas( const netIOVec &ioVec )
   {
      for ( UINT32 i = 0 ; i < ioVec.size() ; ++i )
      {
         _ioDatas.push_back( ioVec[ i ] ) ;
      }
   }

   void _pmdSubSession::addIOData( const netIOV &io )
   {
      _ioDatas.push_back( io ) ;
   }

   UINT32 _pmdSubSession::getIODataLen()
   {
      UINT32 len = 0 ;
      for ( UINT32 i = 0 ; i < _ioDatas.size() ; ++i )
      {
         len += _ioDatas[ i ].iovLen ;
      }
      return len ;
   }

   void _pmdSubSession::setReqMsg( MsgHeader *pReqMsg,
                                   pmdEDUMemTypes memType )
   {
      clearRequestInfo() ;
      _pReqMsg = pReqMsg ;
      _memType = memType ;
   }

   MsgHeader* _pmdSubSession::getRspMsg( BOOLEAN owned )
   {
      if ( owned )
      {
         _event._dataMemType = PMD_EDU_MEM_NONE ;
      }
      return (MsgHeader*)_event._Data ;
   }

   void _pmdSubSession::setProcessInfo( INT32 processResult )
   {
      _processResult = processResult ;
      _isProcessed   = TRUE ;
   }

   void _pmdSubSession::clearProcessInfo()
   {
      _isProcessed   = FALSE ;
      _processResult = SDB_OK ;
   }

   void _pmdSubSession::setSendResult( BOOLEAN isSend )
   {
      _isSend = isSend ;
      if ( isSend )
      {
         _isDisconnect = FALSE ;
         _needToDel    = TRUE ;
      }
   }

   void _pmdSubSession::processEvent( pmdEDUEvent &event )
   {
      MsgHeader *pRsp = ( MsgHeader* )event._Data ;

      clearReplyInfo() ;

      if ( MSG_BS_DISCONNECT == pRsp->opCode )
      {
         _isDisconnect = TRUE ;
      }

      _event = event ;
      if ( PMD_EDU_MEM_NONE == event._dataMemType )
      {
         _event._Data = SDB_OSS_MALLOC( pRsp->messageLength ) ;
         if ( _event._Data )
         {
            ossMemcpy( _event._Data, pRsp, pRsp->messageLength ) ;
            _event._dataMemType = PMD_EDU_MEM_ALLOC ;
         }
         else
         {
            PD_LOG( PDERROR, "Failed to alloc memory[size: %d]",
                    pRsp->messageLength ) ;
         }
      }
      else
      {
         event._dataMemType = PMD_EDU_MEM_NONE ;
      }
   }

   /*
      _pmdSubSessionItr implement
   */
   _pmdSubSessionItr::_pmdSubSessionItr( MAP_SUB_SESSION *pSessions,
                                         PMD_SSITR_FILTER filter )
   :_pSessions( pSessions ), _filter( filter )
   {
      if ( _pSessions )
      {
         _curPos = _pSessions->begin() ;
         _findPos() ;
      }
   }

   _pmdSubSessionItr::~_pmdSubSessionItr()
   {
      _pSessions = NULL ;
   }

   void _pmdSubSessionItr::_findPos()
   {
      if ( PMD_SSITR_ALL != _filter )
      {
         pmdSubSession *pSub = NULL ;
         while ( _curPos != _pSessions->end() )
         {
            pSub = &(_curPos->second) ;
            if ( PMD_SSITR_UNSENT == _filter && !pSub->isSend() )
            {
               break ;
            }
            else if ( PMD_SSITR_SENT == _filter && pSub->isSend() )
            {
               break ;
            }
            else if ( PMD_SSITR_UNREPLY == _filter && pSub->isSend() &&
                      !pSub->hasReply() )
            {
               break ;
            }
            else if ( PMD_SSITR_REPLY == _filter && pSub->hasReply() )
            {
               break ;
            }
            else if ( PMD_SSITR_UNPROCESSED == _filter && pSub->hasReply() &&
                      !pSub->isProcessed() )
            {
               break ;
            }
            else if ( PMD_SSITR_PROCESSED == _filter && pSub->isProcessed() )
            {
               break ;
            }
            else if ( PMD_SSITR_PROCESS_SUC == _filter &&
                      pSub->isProcessed() &&
                      SDB_OK == pSub->getProcessRet() )
            {
               break ;
            }
            else if ( PMD_SSITR_PROCESS_FAIL == _filter &&
                      pSub->isProcessed() &&
                      SDB_OK != pSub->getProcessRet() )
            {
               break ;
            }
            else if ( PMD_SSITR_DISCONNECT == _filter &&
                      pSub->isDisconnect() )
            {
               break ;
            }
            ++_curPos ;
         }
      }
   }

   BOOLEAN _pmdSubSessionItr::more()
   {
      if ( !_pSessions || _curPos == _pSessions->end() )
      {
         return FALSE ;
      }
      return TRUE ;
   }

   pmdSubSession* _pmdSubSessionItr::next()
   {
      pmdSubSession *pSubSession = &(_curPos->second) ;
      ++_curPos ;
      _findPos() ;
      return pSubSession ;
   }

   /*
      _pmdRemoteSession implement
   */
   _pmdRemoteSession::_pmdRemoteSession( netRouteAgent *pAgent,
                                         UINT64 sessionID,
                                         _pmdRemoteSessionSite *pSite,
                                         INT64 timeout,
                                         IRemoteSessionHandler *pHandle )
   {
      _pAgent        = pAgent ;
      _pHandle       = pHandle ;
      _pEDUCB        = NULL ;
      _sessionID     = sessionID ;
      _pSite         = pSite ;

      setTimeout( timeout ) ;
   }

   _pmdRemoteSession::~_pmdRemoteSession()
   {
      _pAgent        = NULL ;
      _pHandle       = NULL ;
      _pSite         = NULL ;
   }

   void _pmdRemoteSession::clear()
   {
      _mapPendingSubSession.clear() ;
      _mapSubSession.clear() ;
      _pHandle          = NULL ;
      _pSite            = NULL ;
   }

   void _pmdRemoteSession::reset( UINT64 sessionID,
                                  _pmdRemoteSessionSite *pSite,
                                  INT64 timeout ,
                                  IRemoteSessionHandler *pHandle )
   {
      _sessionID        = sessionID ;
      _pSite            = pSite ;
      _pHandle          = pHandle ;

      setTimeout( timeout ) ;
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
      _mapPendingSubSession.erase( nodeID ) ;
      MAP_SUB_SESSION_IT it = _mapSubSession.find( nodeID ) ;
      if ( it != _mapSubSession.end() )
      {
         if ( it->second.isNeedToDel() )
         {
            _pSite->delSubSession( it->second.getReqID() ) ;
         }
         _mapSubSession.erase( it ) ;
      }
   }

   void _pmdRemoteSession::clearSubSession()
   {
      _mapPendingSubSession.clear() ;

      MAP_SUB_SESSION_IT it = _mapSubSession.begin() ;
      while ( it != _mapSubSession.end() )
      {
         if ( it->second.isNeedToDel() )
         {
            _pSite->delSubSession( it->second.getReqID() ) ;
         }
         ++it ;
      }
      _mapSubSession.clear() ;
   }

   UINT32 _pmdRemoteSession::getSubSessionCount( PMD_SSITR_FILTER filter )
   {
      UINT32 count = 0 ;
      if ( PMD_SSITR_ALL == filter )
      {
         count = _mapSubSession.size() ;
      }
      else
      {
         pmdSubSessionItr itr = getSubSessionItr( filter ) ;
         
         while ( itr.more() )
         {
            itr.next() ;
            ++count ;
         }
      }
      return count ;
   }

   BOOLEAN _pmdRemoteSession::isTimeout() const
   {
      return _milliTimeout <= 0 ? TRUE : FALSE ;
   }

   BOOLEAN _pmdRemoteSession::isAllReply()
   {
      BOOLEAN ret = TRUE ;
      pmdSubSession *pSub = NULL ;
      MAP_SUB_SESSION_IT it = _mapSubSession.begin() ;
      while ( it != _mapSubSession.end() )
      {
         pSub = &(it->second) ;
         ++it ;
         // send msg, but not reply
         if ( pSub->isSend() && !pSub->hasReply() )
         {
            ret = FALSE ;
            break ;
         }
      }

      return ret ;
   }

   void _pmdRemoteSession::setTimeout( INT64 timeout )
   {
      if ( timeout <= 0 )
      {
         _milliTimeout = 0x7FFFFFFFFFFFFFFF ;
      }
      else
      {
         _milliTimeout = timeout ;
      }
   }

   pmdSubSessionItr _pmdRemoteSession::getSubSessionItr( PMD_SSITR_FILTER filter )
   {
      return pmdSubSessionItr( &_mapSubSession, filter ) ;
   }

   pmdSubSession* _pmdRemoteSession::addSubSession( UINT64 nodeID )
   {
      pmdSubSession &subSession = _mapSubSession[ nodeID ] ;
      if ( subSession.getNodeIDUInt() != nodeID )
      {
         subSession.setNodeID( nodeID ) ;
         subSession.setParent( this ) ;
         _pSite->addNodeID( nodeID ) ;
      }
      return &subSession ;
   }

   INT32 _pmdRemoteSession::sendMsg( MsgHeader * pSrcMsg,
                                     pmdEDUMemTypes memType,
                                     INT32 *pSucNum,
                                     INT32 *pTotalNum )
   {
      INT32 rc = SDB_OK ;
      VEC_SUB_SESSIONPTR vecFailedSession ;
      vector< INT32 > vecFailedFlag ;
      pmdSubSession *pSub = NULL ;

      if ( pTotalNum )
      {
         *pTotalNum = 0 ;
      }
      if ( pSucNum )
      {
         *pSucNum = 0 ;
      }

      pmdSubSessionItr itr = getSubSessionItr( PMD_SSITR_UNSENT ) ;
      while ( itr.more() )
      {
         pSub = itr.next() ;
         if ( pSrcMsg )
         {
            pSub->setReqMsg( pSrcMsg ) ;
         }
         rc = sendMsg( pSub ) ;

         if ( pTotalNum )
         {
            ++(*pTotalNum) ;
         }
         if ( SDB_OK == rc )
         {
            if ( pSucNum )
            {
               ++(*pSucNum) ;
            }
            if ( pSrcMsg && PMD_EDU_MEM_NONE != memType )
            {
               pSub->setReqMsgMemType( memType ) ;
               memType = PMD_EDU_MEM_NONE ;
            }
         }
         else
         {
            vecFailedSession.push_back( pSub ) ;
            vecFailedFlag.push_back( rc ) ;

            if ( !pSucNum && !_pHandle )
            {
               goto error ;
            }
         }
      }

      if ( !_pHandle )
      {
         if ( vecFailedFlag.size() > 0 )
         {
            rc = vecFailedFlag[ 0 ] ;
            goto error ;
         }
         goto done ;
      }
      else
      {
         INT32 rcTmp = SDB_OK ;
         UINT64 nodeID = 0 ;
         // process failed
         for ( UINT32 i = 0 ; i < vecFailedSession.size() ; ++i )
         {
            pSub = vecFailedSession[ i ] ;
            nodeID = pSub->getNodeIDUInt() ;
            rcTmp = vecFailedFlag[ i ] ;
            rc = _pHandle->onSendFailed( this, &pSub, rcTmp ) ;
            PD_RC_CHECK( rc, PDERROR, "Session[%s] send msg to node[%s] "
                         "failed[rc: %d] and processed failed[rc: %d]",
                         _pEDUCB->toString().c_str(),
                         routeID2String(nodeID).c_str(),
                         rcTmp, rc ) ;
            if ( pSucNum )
            {
               ++(*pSucNum) ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRemoteSession::sendMsg( MsgHeader *pSrcMsg,
                                     SET_NODEID &subs,
                                     pmdEDUMemTypes memType,
                                     INT32 *pSucNum,
                                     INT32 *pTotalNum )
   {
      INT32 rc = SDB_OK ;
      pmdSubSession *pSub = NULL ;
      VEC_SUB_SESSIONPTR vecFailedSession ;
      vector< INT32 > vecFailedFlag ;
      SET_NODEID::iterator it = subs.begin() ;
      UINT64 nodeID = 0 ;

      if ( pTotalNum )
      {
         *pTotalNum = 0 ;
      }
      if ( pSucNum )
      {
         *pSucNum = 0 ;
      }

      while ( it != subs.end() )
      {
         nodeID = *it ;
         ++it ;
         pSub = addSubSession( nodeID ) ;
         if ( pSub )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Session[%s] failed to add sub session[%s]",
                    _pEDUCB->toString().c_str(),
                    routeID2String(nodeID).c_str() ) ;
            goto error ;
         }
         if ( pSub->isSend() )
         {
            continue ;
         }
         pSub->setReqMsg( pSrcMsg ) ;
         rc = sendMsg( pSub ) ;

         if ( pTotalNum )
         {
            ++(*pTotalNum) ;
         }
         if ( SDB_OK == rc )
         {
            if ( pSucNum )
            {
               ++(*pSucNum) ;
            }
            if ( pSrcMsg && memType != PMD_EDU_MEM_NONE )
            {
               pSub->setReqMsgMemType( memType ) ;
               memType = PMD_EDU_MEM_NONE ;
            }
         }
         else if ( SDB_OK != rc )
         {
            vecFailedSession.push_back( pSub ) ;
            vecFailedFlag.push_back( rc ) ;

            if ( !pSucNum && !_pHandle )
            {
               goto error ;
            }
         }
      }

      if ( !_pHandle )
      {
         if ( vecFailedFlag.size() > 0 )
         {
            rc = vecFailedFlag[ 0 ] ;
            goto error ;
         }
         goto done ;
      }
      else
      {
         INT32 rcTmp = SDB_OK ;
         // process failed
         for ( UINT32 i = 0 ; i < vecFailedSession.size() ; ++i )
         {
            pSub = vecFailedSession[ i ] ;
            nodeID = pSub->getNodeIDUInt() ;
            rcTmp = vecFailedFlag[ i ] ;
            rc = _pHandle->onSendFailed( this, &pSub, rcTmp ) ;
            PD_RC_CHECK( rc, PDERROR, "Session[%s] send msg to node[%s] "
                         "failed[rc: %d] and processed failed[rc: %d]",
                         _pEDUCB->toString().c_str(),
                         routeID2String(nodeID).c_str(),
                         rcTmp, rc ) ;
            if ( pSucNum )
            {
               ++(*pSucNum) ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRemoteSession::sendMsg( INT32 *pSucNum, INT32 *pTotalNum )
   {
      return sendMsg( NULL, PMD_EDU_MEM_NONE, pSucNum, pTotalNum ) ;
   }

   INT32 _pmdRemoteSession::sendMsg( UINT64 nodeID )
   {
      INT32 rc = SDB_OK ;
      pmdSubSession *pSub = getSubSession( nodeID ) ;
      if ( !pSub )
      {
         PD_LOG( PDERROR, "Session[%s] can't find sub session[%s]",
                 _pEDUCB->toString().c_str(), routeID2String(nodeID).c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      rc = sendMsg( pSub ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRemoteSession::sendMsg( pmdSubSession *pSub )
   {
      INT32 rc = SDB_OK ;
      UINT64 oldReqID = 0 ;

      if ( !pSub )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !pSub->getReqMsg() )
      {
         PD_LOG( PDERROR, "Session[%s] request msg header is NULL",
                 _pEDUCB->toString().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // has send msg
      if ( pSub->isSend() )
      {
         PD_LOG( PDDEBUG, "Session[%s] has already send msg to "
                 "node[%s]", _pEDUCB->toString().c_str(),
                 routeID2String(pSub->getNodeID()).c_str() ) ;
         goto done ;
      }

      oldReqID = pSub->getReqID() ;
      pSub->setReqID( _pEDUCB->incCurRequestID() ) ;
      pSub->getReqMsg()->requestID = pSub->getReqID() ;
      pSub->getReqMsg()->routeID.value = MSG_INVALID_ROUTEID ;
      pSub->getReqMsg()->TID = _pEDUCB->getTID() ;

      // prepare send
      if ( pSub->getIODatas()->size() > 0 )
      {
         pSub->getReqMsg()->messageLength = sizeof( MsgHeader ) +
                                            pSub->getIODataLen() ;
         rc = _pAgent->syncSendv( pSub->getNodeID(), pSub->getReqMsg(),
                                  *(pSub->getIODatas()) ) ;
      }
      else
      {
         rc = _pAgent->syncSend( pSub->getNodeID(),
                                 (void*)pSub->getReqMsg() ) ;
      }

      if ( SDB_OK == rc )
      {
         if ( pSub->isNeedToDel() )
         {
            _pSite->delSubSession( oldReqID ) ;
         }
         pSub->setSendResult( TRUE ) ;
         // add to request map
         _pSite->addSubSession( pSub ) ;
      }
      else
      {
         PD_RC_CHECK( rc, PDERROR, "Session[%s] send msg to node[%s] failed, "
                      "rc: %d", _pEDUCB->toString().c_str(),
                      routeID2String(pSub->getNodeID()).c_str(), rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRemoteSession::waitReply( BOOLEAN waitAll,
                                       VEC_SUB_SESSIONPTR *pSubs )
   {
      INT32 rc = SDB_OK ;

      if ( !pSubs )
      {
         rc = waitReply1( waitAll, NULL ) ;
      }
      else
      {
         MAP_SUB_SESSIONPTR mapSessionPtrs ;
         INT32 rc = waitReply1( waitAll, &mapSessionPtrs ) ;
         if ( SDB_OK == rc )
         {
            MAP_SUB_SESSIONPTR_IT it = mapSessionPtrs.begin() ;
            while ( it != mapSessionPtrs.end() )
            {
               pSubs->push_back( it->second ) ;
               ++it ;
            }
         }
      }

      return rc ;
   }

   INT32 _pmdRemoteSession::waitReply1( BOOLEAN waitAll,
                                        MAP_SUB_SESSIONPTR *pSubs )
   {
      INT32 rc                      = SDB_OK ;
      pmdEDUEvent event ;
      INT64 timeout                 = OSS_ONE_SEC ;
      UINT32 totalUnReplyNum        = 0 ;
      UINT32 replyNum               = 0 ;
      pmdSubSession *pSubSession    = NULL ;

      // if pending sessions is not empty
      if ( _mapPendingSubSession.size() > 0 )
      {
         if ( pSubs )
         {
            MAP_SUB_SESSIONPTR_IT itPending = _mapPendingSubSession.begin() ;
            while ( itPending != _mapPendingSubSession.end() )
            {
               (*pSubs)[ itPending->first ] = itPending->second ;
               ++itPending ;
               ++replyNum ;
            }
         }
         else
         {
            replyNum = _mapPendingSubSession.size() ;
         }
         _mapPendingSubSession.clear() ;
      }

      totalUnReplyNum = getSubSessionCount( PMD_SSITR_UNREPLY ) ;
      while ( totalUnReplyNum > 0 )
      {
         if ( _pEDUCB->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         if ( !waitAll && replyNum > 0 )
         {
            timeout = 1 ;
         }
         else
         {
            timeout = _milliTimeout < OSS_ONE_SEC ?
                      _milliTimeout : OSS_ONE_SEC ;
         }

         // wait event
         if ( !_pEDUCB->waitEvent( event, timeout ) )
         {
            _milliTimeout -= timeout ;
            if ( 0 == replyNum || waitAll )
            {
               if ( _milliTimeout <= 0 )
               {
                  rc = SDB_TIMEOUT ;
                  goto error ;
               }
            }
            else
            {
               if ( _milliTimeout <= 0 )
               {
                  _milliTimeout = 1 ;
               }
               goto done ;
            }
            continue ;
         }

         if ( PMD_EDU_EVENT_MSG != event._eventType )
         {
            PD_LOG( PDWARNING, "Session[%s] recv unknonw event[type: %d]",
                    _pEDUCB->toString().c_str(), event._eventType ) ;
            pmdEduEventRelase( event, _pEDUCB ) ;
            event.reset() ;
            continue ;
         }

         pSubSession = NULL ;
         rc = _pSite->processEvent( event, _mapSubSession, &pSubSession ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to process event, rc: %d", rc ) ;

         if ( pSubSession )
         {
            ++replyNum ;
            --totalUnReplyNum ;
            if ( pSubs )
            {
               (*pSubs)[ pSubSession->getNodeIDUInt() ] = pSubSession ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _pmdRemoteSession::addPending( pmdSubSession *pSubSession )
   {
      _mapPendingSubSession[ pSubSession->getNodeIDUInt() ] = pSubSession ;
   }

   /*
      _pmdRemoteSessionSite implement
   */
   _pmdRemoteSessionSite::_pmdRemoteSessionSite()
   {
      _pEDUCB = NULL ;
   }

   _pmdRemoteSessionSite::~_pmdRemoteSessionSite()
   {
      _pEDUCB = NULL ;
   }

   void _pmdRemoteSessionSite::addSubSession( pmdSubSession *pSub )
   {
      _mapReq2SubSession[ pSub->getReqID() ] = pSub ;
   }

   void _pmdRemoteSessionSite::delSubSession( UINT64 reqID )
   {
      _mapReq2SubSession.erase( reqID ) ;
   }

   INT32 _pmdRemoteSessionSite::processEvent( pmdEDUEvent &event,
                                              MAP_SUB_SESSION &mapSessions,
                                              pmdSubSession **ppSub )
   {
      INT32 rc = SDB_OK ;
      MAP_SUB_SESSION_IT it ;
      MAP_SUB_SESSIONPTR_IT itPtr ;
      MsgHeader *pReply = NULL ;
      UINT64 nodeID = 0 ;

      SDB_ASSERT( ppSub, "ppSub can't be NULL" ) ;

      if ( !event._Data )
      {
         PD_LOG( PDWARNING, "Session[%s] msg event data is NULL",
                 _pEDUCB->toString().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      pReply = ( MsgHeader* )event._Data ;
      nodeID = pReply->routeID.value ;

      // if is MSG_BS_DISCONNECT, the remote node is disconnect
      if ( MSG_BS_DISCONNECT == pReply->opCode )
      {
         itPtr = _mapReq2SubSession.begin() ;
         while ( itPtr != _mapReq2SubSession.end() )
         {
            if ( pReply->requestID < itPtr->first )
            {
               break ;
            }
            else if ( itPtr->second->getNodeIDUInt() == nodeID )
            {
               itPtr->second->processEvent( event ) ;
               itPtr->second->setNeedToDel( FALSE ) ;
               if ( !*ppSub && ( it = mapSessions.find( nodeID ) ) !=
                    mapSessions.end() &&
                    &(it->second) == itPtr->second )
               {
                  *ppSub = &(it->second) ;
               }
               else
               {
                  itPtr->second->parent()->addPending( itPtr->second ) ;
               }
               // remove from request id map
               _mapReq2SubSession.erase( itPtr++ ) ;
               continue ;
            }
            ++itPtr ;
         }
      }
      else
      {
         itPtr = _mapReq2SubSession.find( pReply->requestID ) ;
         if ( itPtr != _mapReq2SubSession.end() )
         {
            itPtr->second->processEvent( event ) ;
            itPtr->second->setNeedToDel( FALSE ) ;
            nodeID = itPtr->second->getNodeIDUInt() ;
            if ( !*ppSub && ( it = mapSessions.find( nodeID ) ) !=
                 mapSessions.end() &&
                 &(it->second) == itPtr->second )
            {
               *ppSub = &(it->second) ;
            }
            else
            {
               itPtr->second->parent()->addPending( itPtr->second ) ;
            }
            // remove from request id map
            _mapReq2SubSession.erase( itPtr ) ;
         }
         else
         {
            PD_LOG( PDWARNING, "Session[%s] recv expired msg[opCode: (%d)%u, "
                    "ReqID: %lld, Len: %d, NodeID: %s]",
                    _pEDUCB->toString().c_str(), IS_REPLY_TYPE(pReply->opCode),
                    GET_REQUEST_TYPE(pReply->opCode), pReply->requestID,
                    pReply->messageLength,
                    routeID2String(pReply->routeID).c_str() ) ;
         }
      }

   done:
      pmdEduEventRelase( event, _pEDUCB ) ;
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
      pmdRemoteSessionSite &site = _mapTID2EDU[ cb->getTID() ] ;
      site.setEduCB( cb ) ;
   }

   void _pmdRemoteSessionMgr::unregEUD( _pmdEDUCB * cb )
   {
      ossScopedLock lock( &_edusLatch, EXCLUSIVE ) ;
      _mapTID2EDU.erase( cb->getTID() ) ;
   }

   pmdRemoteSessionSite* _pmdRemoteSessionMgr::getSite( _pmdEDUCB * cb )
   {
      pmdRemoteSessionSite *pSite = NULL ;
      ossScopedLock lock( &_edusLatch, SHARED ) ;
      MAP_TID_2_EDU_IT it = _mapTID2EDU.find( cb->getTID() ) ;
      if ( it != _mapTID2EDU.end() )
      {
         pSite = &(it->second) ;
      }
      return pSite ;
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
         pEDUCB = it->second.eduCB() ;

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
                    "opCode: (%d)%u, TID: %d, ReqID: %llu, NodeID: %s]",
                    pMsg->messageLength, IS_REPLY_TYPE(pMsg->opCode),
                    GET_REQUEST_TYPE(pMsg->opCode), pMsg->TID,
                    pMsg->requestID, routeID2String(pMsg->routeID).c_str() ) ;
         }
      }
      else
      {
         PD_LOG( PDWARNING, "Can't find remote session[TID=%d] for msg["
                 "opCode: (%d)%u, ReqID: %llu, NodeID: %s, Len: %u]",
                 pMsg->TID, IS_REPLY_TYPE(pMsg->opCode),
                 GET_REQUEST_TYPE(pMsg->opCode), pMsg->requestID,
                 routeID2String(pMsg->routeID).c_str(), pMsg->messageLength ) ;
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
            cb = it->second.eduCB() ;
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

   void _pmdRemoteSessionMgr::handleConnect( const NET_HANDLE &handle,
                                             _MsgRouteID id,
                                             BOOLEAN isPositive )
   {
      // TODO:XUJIANHUI
      // CHECK REMOTE ID, AND SEND HOST+SERVCIE TO PEER
   }

   pmdRemoteSession* _pmdRemoteSessionMgr::addSession( _pmdEDUCB * cb,
                                                        INT64 timeout,
                                                        IRemoteSessionHandler *pHandle )
   {
      pmdRemoteSession *pSession = NULL ;
      UINT64 sessionID = 0 ;
      pmdRemoteSessionSite *pSite = getSite( cb ) ;
      if ( !pSite )
      {
         PD_LOG( PDERROR, "Session[%s] is not registered for remote session",
                 cb->toString().c_str() ) ;
         return NULL ;
      }

      ossScopedLock lock( &_mapLatch, EXCLUSIVE ) ;

      sessionID = _sessionHWNum++ ;

      // if idle session not empty
      if ( !_idleSessions.empty() )
      {
         pSession = _idleSessions.back() ;
         pSession->reset( sessionID, pSite, timeout, pHandle ) ;
         _idleSessions.pop_back() ;
      }
      else
      {
         pSession = SDB_OSS_NEW _pmdRemoteSession( _pAgent, sessionID, pSite,
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

