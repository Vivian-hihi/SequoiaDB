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

   Source File Name = clsSession.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "clsSession.hpp"
#include "ossMem.hpp"
#include "pmd.hpp"
#include "rtnCommand.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

#include "../bson/bson.h"

using namespace bson ;

namespace engine
{

   /*
      _clsSessionMeta implement
   */
   _clsSessionMeta::_clsSessionMeta( const NET_HANDLE handle )
   :_basedHandleNum( 0 )
   {
      _netHandle = handle ;
   }

   _clsSessionMeta::~_clsSessionMeta()
   {
   }

   /*
      _clsSession implement
   */
   BEGIN_OBJ_MSG_MAP( _clsSession, _clsObjBase )
      //ON_MSG
   END_OBJ_MSG_MAP()

   _clsSession::_clsSession( UINT64 sessionID )
   {
      _lockFlag = FALSE ;
      _startType = CLS_SESSION_PASSIVE ;

      clear() ;

      _sessionID = sessionID ;
      _makeName () ;
      _latchIn.try_get () ;
   }

   _clsSession::~_clsSession()
   {
      clear() ;
      _latchIn.release () ;
   }

   UINT64 _clsSession::identifyID()
   {
      // TODO:XUJIANHUI
      // BY COORD SESSION INFO
      return 0 ;
   }

   INT32 _clsSession::getServiceType() const
   {
      return CMD_SPACE_SERVICE_SHARD ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_ATHIN, "_clsSession::attachIn" )
   INT32 _clsSession::attachIn ( pmdEDUCB *cb )
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_ATHIN );

      SDB_ASSERT( cb, "cb can't be NULL" ) ;

      PD_LOG( PDINFO, "Session[%s] attach edu[%d]", sessionName(),
              cb->getID() ) ;

      _pEDUCB = cb ;
      _eduID  = cb->getID() ;
      _pEDUCB->setName( sessionName() ) ;
      _pEDUCB->attachSession( this ) ;

      _latchOut.try_get () ;
      _latchIn.release () ;

      _onAttach () ;

      PD_TRACE_EXIT ( SDB__CLSSN_ATHIN );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_ATHOUT, "_clsSession::attachOut" )
   INT32 _clsSession::attachOut ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_ATHOUT );

      PD_LOG( PDINFO, "Session[%s] detach edu[%d]", sessionName(),
              eduID() ) ;

      _onDetach () ;

      _pEDUCB->detachSession() ;
      _latchOut.release () ;
      _pEDUCB = NULL ;
      PD_TRACE_EXIT ( SDB__CLSSN_ATHOUT );
      return SDB_OK ;
   }

   BOOLEAN _clsSession::isDetached () const
   {
      return _pEDUCB ? FALSE : TRUE ;
   }

   BOOLEAN _clsSession::isAttached () const
   {
      return _pEDUCB ? TRUE : FALSE ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_CLEAR, "_clsSession::clear" )
   void _clsSession::clear()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_CLEAR );
      if ( _lockFlag )
      {
         _unlock () ;
      }

      _sessionID = INVLIAD_SESSION_ID;
      _pEDUCB = NULL ;
      _eduID = PMD_INVALID_EDUID ;
      _netHandle = NET_INVALID_HANDLE ;
      _name [0] = 0 ;
      _pMeta = NULL ;

      for ( UINT32 index = 0 ; index < MAX_BUFFER_ARRAY_SIZE; ++index )
      {
         _buffArray[index].pBuffer = NULL ;
         _buffArray[index].size = 0 ;
         _buffArray[index].useFlag = CLS_BUFF_INVALID ;
         _buffArray[index].addTime = 0 ;
      }
      _buffBegin = 0 ;
      _buffEnd = 0 ;
      _buffCount = 0 ;
      PD_TRACE_EXIT ( SDB__CLSSN_CLEAR );
   }

   void _clsSession::onRecieve ( const NET_HANDLE netHandle, MsgHeader * msg )
   {
   }

   BOOLEAN _clsSession::timeout ( UINT32 interval )
   {
      return FALSE ;
   }

   void _clsSession::_onAttach ()
   {
   }

   void _clsSession::_onDetach ()
   {
   }

   UINT64 _clsSession::sessionID () const
   {
      return _sessionID ;
   }

   void _clsSession::sessionID ( UINT64 sessionID )
   {
      _sessionID = sessionID ;
      _makeName () ;
   }

   EDUID _clsSession::eduID () const
   {
      return _eduID ;
   }

   pmdEDUCB *_clsSession::eduCB () const
   {
      return _pEDUCB ;
   }

   NET_HANDLE _clsSession::netHandle () const
   {
      return _netHandle ;
   }

   void _clsSession::meta ( clsSessionMeta * pMeta )
   {
      _pMeta = pMeta ;
      if ( _pMeta )
      {
         _netHandle = _pMeta->getHandle() ;
      }
      else
      {
         _netHandle = NET_INVALID_HANDLE ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN__MKNAME, "_clsSession::_makeName" )
   void _clsSession::_makeName ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN__MKNAME );
      UINT32 nodeID, TID ;
      ossUnpack32From64 ( _sessionID, nodeID, TID ) ;
      if ( nodeID > CLS_BASE_HANDLE_ID )
      {
         ossSnprintf( _name , SESSION_NAME_LEN, "NetID:%u,TID:%u",
                      nodeID - CLS_BASE_HANDLE_ID, TID ) ;
      }
      else
      {
         ossSnprintf( _name , SESSION_NAME_LEN, "NodeID:%u,TID:%u,Start:%s",
                      nodeID, TID, isStartActive() ? "active" : "passive" ) ;
      }
      _name [SESSION_NAME_LEN] = 0 ;
      PD_TRACE_EXIT ( SDB__CLSSN__MKNAME );
   }

   BOOLEAN _clsSession::isStartActive ()
   {
      return _startType == CLS_SESSION_ACTIVE ? TRUE : FALSE ;
   }

   void _clsSession::startType ( INT32 startType )
   {
      _startType = startType ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN__LOCK, "_clsSession::_lock" )
   INT32 _clsSession::_lock ()
   {
      INT32 rc = SDB_SYS ;
      PD_TRACE_ENTRY ( SDB__CLSSN__LOCK );
      if ( _pMeta && !_lockFlag )
      {
         _pMeta->getLatch()->get() ;
         _lockFlag = TRUE ;
         rc = SDB_OK ;
      }
      PD_TRACE_EXITRC ( SDB__CLSSN__LOCK, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN__UNLOCK, "_clsSession::_unlock" )
   INT32 _clsSession::_unlock ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN__UNLOCK );
      if ( _pMeta && _lockFlag )
      {
         _pMeta->getLatch()->release () ;
         _lockFlag = FALSE ;
      }
      PD_TRACE_EXIT ( SDB__CLSSN__UNLOCK );
      return SDB_OK ;
   }

   const CHAR *_clsSession::sessionName () const
   {
      return _name ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_WTATH, "_clsSession::waitAttach" )
   INT32 _clsSession::waitAttach ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_WTATH );
      _latchIn.get () ;
      PD_TRACE_EXIT ( SDB__CLSSN_WTATH );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_WTDTH, "_clsSession::waitDetach" )
   INT32 _clsSession::waitDetach ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_WTDTH );
      _latchOut.get () ;
      _latchOut.release () ;
      PD_TRACE_EXIT ( SDB__CLSSN_WTDTH );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_CPMSG, "_clsSession::copyMsg" )
   void * _clsSession::copyMsg( const CHAR *msg, UINT32 length )
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_CPMSG );
      void *p = NULL ;
      UINT32 buffPos = _decBuffPos ( _buffEnd ) ;
      if ( _buffArray[buffPos].isAlloc() &&
           _buffArray[buffPos].size >= length )
      {
         ossMemcpy( _buffArray[buffPos].pBuffer, msg, length ) ;
         _buffArray[buffPos].useFlag = CLS_BUFF_USING ;
         p = (void*)&_buffArray[buffPos] ;
         goto done ;
      }

      PD_LOG ( PDERROR, "Session[%s] copy msg failed[buffindex:%d, size:%d, "
               "flag:%d, message length:%d", sessionName(), buffPos,
               _buffArray[buffPos].size, _buffArray[buffPos].useFlag, length ) ;

   done :
      PD_TRACE_EXIT ( SDB__CLSSN_CPMSG );
      return p ;
   }

   BOOLEAN _clsSession::isBufferFull() const
   {
      return _buffCount >= MAX_BUFFER_ARRAY_SIZE ? TRUE : FALSE ;
   }

   BOOLEAN _clsSession::isBufferEmpty() const
   {
      return _buffCount == 0 ? TRUE : FALSE ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_FRNBUF, "_clsSession::frontBuffer" )
   clsBuffInfo *_clsSession::frontBuffer ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_FRNBUF );
      clsBuffInfo *p = NULL ;
      if ( _buffArray[_buffBegin].isInvalid() )
      {
         goto done ;
      }
      SDB_ASSERT ( _buffCount > 0 , "impossible" ) ;

      p = &_buffArray[_buffBegin] ;
   done :
      PD_TRACE_EXIT ( SDB__CLSSN_FRNBUF );
      return p ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_POPBUF, "_clsSession::popBuffer" )
   void _clsSession::popBuffer ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_POPBUF );
      SDB_ASSERT ( _buffCount > 0 , "impossible" ) ;

      _buffArray[_buffBegin].pBuffer = NULL ;
      _buffArray[_buffBegin].size = 0 ;
      _buffArray[_buffBegin].useFlag = CLS_BUFF_INVALID ;
      _buffArray[_buffBegin].addTime = 0 ;

      --_buffCount ;
      _buffBegin = _incBuffPos( _buffBegin ) ;
      PD_TRACE_EXIT ( SDB__CLSSN_POPBUF );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_PSHBUF, "_clsSession::pushBuffer" )
   INT32 _clsSession::pushBuffer ( CHAR * pBuffer, UINT32 size )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSN_PSHBUF );
      if ( _buffCount >= MAX_BUFFER_ARRAY_SIZE )
      {
         rc = SDB_CLS_BUFFER_FULL ;
         goto done ;
      }

      SDB_ASSERT ( _buffArray[_buffEnd].isInvalid (), "impossilbe" ) ;

      ++_buffCount ;
      _buffArray[_buffEnd].pBuffer = pBuffer ;
      _buffArray[_buffEnd].size = size ;
      _buffArray[_buffEnd].useFlag = CLS_BUFF_ALLOC ;
      _buffArray[_buffEnd].addTime = time( NULL ) ;

      _buffEnd = _incBuffPos( _buffEnd ) ;

   done :
      PD_TRACE_EXITRC ( SDB__CLSSN_PSHBUF, rc );
      return rc ;
   }

   UINT32 _clsSession::_incBuffPos ( UINT32 pos )
   {
      ++pos ;
      if ( pos < MAX_BUFFER_ARRAY_SIZE )
      {
         return pos ;
      }

      return 0 ;
   }

   UINT32 _clsSession::_decBuffPos ( UINT32 pos )
   {
      return pos ? pos - 1 : MAX_BUFFER_ARRAY_SIZE - 1 ;
   }

   /*
      _clsSessionMgr implement
   */
   _clsSessionMgr::_clsSessionMgr()
   {
      _force                  = FALSE ;
      _pRTAgent               = NULL ;
      _pTimerHandle           = NULL ;
      _handleCloseTimerID     = NET_INVALID_TIMER_ID ;
      _sessionTimerID         = NET_INVALID_TIMER_ID ;
      _timerInterval          = OSS_ONE_SEC ;
   }

   _clsSessionMgr::~_clsSessionMgr()
   {
      _pRTAgent               = NULL ;
      _pTimerHandle           = NULL ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_INIT, "_clsSessionMgr::init" )
   INT32 _clsSessionMgr::init( netRouteAgent *pRTAgent,
                               _netTimeoutHandler *pTimerHandle,
                               UINT32 timerInterval )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( CLS_SESSMGR_INIT ) ;

      if ( !pRTAgent || !pTimerHandle )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      _pRTAgent      = pRTAgent ;
      _pTimerHandle  = pTimerHandle ;
      _timerInterval = timerInterval ;

      // init mem pool
      rc = _memPool.initialize() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to init mem pool, rc: %d", rc ) ;
         goto error ;
      }

      // set timer
      rc = _pRTAgent->addTimer( _timerInterval, _pTimerHandle,
                                _sessionTimerID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Add session timer failed, rc: %d", rc ) ;
      }

   done:
      PD_TRACE_EXITRC ( CLS_SESSMGR_INIT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_FINI, "_clsSessionMgr::fini" )
   INT32 _clsSessionMgr::fini()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( CLS_SESSMGR_FINI ) ;

      _force = TRUE ;

      // kill timer
      if ( _pRTAgent )
      {
         if ( NET_INVALID_TIMER_ID != _sessionTimerID )
         {
            _pRTAgent->removeTimer( _sessionTimerID ) ;
            _sessionTimerID = NET_INVALID_TIMER_ID ;
         }
         if ( NET_INVALID_TIMER_ID != _handleCloseTimerID )
         {
            _pRTAgent->removeTimer( _handleCloseTimerID ) ;
            _handleCloseTimerID = NET_INVALID_TIMER_ID ;
         }
      }

      // release session and meta
      MAPSESSION_IT it = _mapSession.begin () ;
      while ( it != _mapSession.end() )
      {
         _releaseSession_i( it->second, FALSE, FALSE ) ;
         ++it ;
      }
      _mapSession.clear () ;

      while ( _deqCatchSessions.size () > 0 )
      {
         _releaseSession_i( _deqCatchSessions.front (), FALSE, FALSE ) ;
         _deqCatchSessions.pop_front () ;
      }

      while ( _deqDeletingSessions.size() > 0 )
      {
         _releaseSession_i ( _deqDeletingSessions.front(), FALSE, FALSE ) ;
         _deqDeletingSessions.pop_front() ;
      }

      //Clear latch
      MAPMETA_IT itMeta = _mapMeta.begin() ;
      while ( itMeta != _mapMeta.end() )
      {
         SDB_OSS_DEL itMeta->second ;
         ++itMeta ;
      }
      _mapMeta.clear() ;

      // mem pool fini
      rc = _memPool.final() ;

      PD_TRACE_EXITRC ( CLS_SESSMGR_FINI, rc ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_ONTIMER, "_clsSessionMgr::onTimer" )
   void _clsSessionMgr::onTimer( UINT32 interval )
   {
      PD_TRACE_ENTRY( CLS_SESSMGR_ONTIMER ) ;

      //Check _deqShdDeletingSessions
      ossScopedLock lock ( &_deqDeletingMutex ) ;
      clsSession *pSession = NULL ;
      DEQSESSION::iterator it = _deqDeletingSessions.begin() ;
      while ( it != _deqDeletingSessions.end() )
      {
         pSession = *it ;
         if ( !pSession->isDetached() )
         {
            ++it ;
            continue ;
         }
         it = _deqDeletingSessions.erase( it ) ;
         _releaseSession_i( pSession, FALSE, FALSE ) ;
      }

      PD_TRACE_EXIT( CLS_SESSMGR_ONTIMER ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_PUSHMSG, "_clsSessionMgr::assignMemory" )
   INT32 _clsSessionMgr::pushMessage( clsSession *pSession,
                                      const MsgHeader *header,
                                      const NET_HANDLE &handle )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( CLS_SESSMGR_PUSHMSG ) ;
      CHAR *pNewBuff = NULL ;
      UINT32 buffSize = 0 ;
      UINT64 userData = 0 ; // 0: memPool, 1: alloc
      pmdEDUMemTypes memType = PMD_EDU_MEM_NONE ;

      clsBuffInfo * pBuffInfo = pSession->frontBuffer () ;
      while ( pBuffInfo && pBuffInfo->isFree() )
      {
         if ( !pNewBuff && pBuffInfo->size >= (UINT32)header->messageLength )
         {
            pNewBuff = pBuffInfo->pBuffer ;
            buffSize = pBuffInfo->size ;
         }
         else //release memory to pool
         {
            _memPool.release( pBuffInfo->pBuffer, pBuffInfo->size ) ;
         }
         pSession->popBuffer () ;
         pBuffInfo = pSession->frontBuffer () ;
      }

      if ( !pNewBuff && !pSession->isBufferFull() )
      {
         pNewBuff = _memPool.alloc ( header->messageLength, buffSize ) ;
         if ( !pNewBuff )
         {
            PD_LOG ( PDERROR, "Memory pool assign memory failed[size:%d]",
                     header->messageLength ) ;
         }
      }

      if ( pNewBuff )
      {
         rc = pSession->pushBuffer ( pNewBuff, buffSize ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG ( PDERROR, "push buffer failed in session[%s, rc:%d]", 
                     pSession->sessionName(), rc ) ;
            _memPool.release ( pNewBuff, buffSize ) ;
            SDB_ASSERT ( 0, "why the buffer is full??? check" ) ;
            goto error ;
         }

         pNewBuff = (CHAR*)pSession->copyMsg( (const CHAR*)header,
                                              header->messageLength ) ;
         if ( NULL == pNewBuff )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for new msg" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
      }
      else
      {
         // alloc msg
         pNewBuff = ( CHAR* )SDB_OSS_MALLOC( header->messageLength ) ;
         if ( !pNewBuff )
         {
            PD_LOG( PDERROR, "Failed to alloc msg[size: %d] in session[%s]",
                    header->messageLength, pSession->sessionName() ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         ossMemcpy( pNewBuff, (void*)header, header->messageLength ) ;
         userData = 1 ;
         memType  = PMD_EDU_MEM_ALLOC ;
      }

      // post edu event
      pSession->eduCB()->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                                 memType, pNewBuff,
                                                 userData ) ) ;
   done:
      PD_TRACE_EXITRC ( CLS_SESSMGR_PUSHMSG, rc ) ;
      return rc ;
   error:
      _onPushMsgFailed( rc, header, handle, pSession ) ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_GETSESSION, "_clsSessionMgr::getSession" )
   clsSession* _clsSessionMgr::getSession( UINT64 sessionID, INT32 startType,
                                           const NET_HANDLE handle,
                                           BOOLEAN bCreate, INT32 opCode,
                                           void *data )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( CLS_SESSMGR_GETSESSION );
      clsSession *pSession = NULL ;
      SDB_SESSION_TYPE sessionType = SDB_SESSION_MAX ;

      MAPSESSION_IT it = _mapSession.find( sessionID ) ;
      if ( it != _mapSession.end() )
      {
         pSession = it->second ;

         // need to attach meta
         if ( !pSession->getMeta() && pSession->canAttachMeta() &&
              NET_INVALID_HANDLE != handle )
         {
            _attachSessionMeta( pSession, handle ) ;
         }
         goto done ;
      }

      if ( !bCreate )
      {
         goto done ;
      }

      // parse session type
      sessionType = _prepareCreate( sessionID, startType, opCode ) ;
      if ( SDB_SESSION_MAX == sessionType )
      {
         PD_LOG( PDERROR, "Failed to parse session type by info[sessionID: "
                 "%lld, startType: %d, opCode: (%d)%d ]", sessionID,
                 startType, IS_REPLY_TYPE(opCode), GET_REQUEST_TYPE(opCode) ) ;
         goto error ;
      }

      // get from catch
      if ( _canReuse( sessionType ) && _deqCatchSessions.size() > 0 )
      {
         DEQSESSION::iterator itDeq = _deqCatchSessions.begin() ;
         while ( itDeq != _deqCatchSessions.end() )
         {
            if ( (*itDeq)->sessionType() == sessionType )
            {
               pSession = *itDeq ;
               _deqCatchSessions.erase( itDeq ) ;
               break ;
            }
            ++itDeq ;
         }
      }

      if ( !pSession )
      {
         pSession = _createSession( sessionType, startType, sessionID, data ) ;
         if ( !pSession )
         {
            PD_LOG( PDERROR, "Failed to create session[sessionType: %d, "
                    "startType: %d, sessionID: %lld ]", sessionType,
                    startType, sessionID ) ;
            goto error ;
        }
      }

      // set session info
      _mapSession[ sessionID ] = pSession ;
      pSession->startType( startType ) ;
      pSession->sessionID( sessionID ) ;

      PD_LOG ( PDEVENT, "Create session[Name: %s, StartType: %d]",
               pSession->sessionName(), startType ) ;

      // attach meta
      if ( !pSession->getMeta() && pSession->canAttachMeta() &&
           NET_INVALID_HANDLE != handle )
      {
         rc = _attachSessionMeta( pSession, handle ) ;
         if ( rc )
         {
            goto error ;
         }
      }

      //Start session EDU
      rc = _startSessionEDU( pSession ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start session EDU, rc = %d", rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXIT ( CLS_SESSMGR_GETSESSION );
      return pSession ;
   error:
      if ( pSession )
      {
         releaseSession ( pSession ) ;
         pSession = NULL ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_ATCHMETA, "_clsSessionMgr::_attachSessionMeta" )
   INT32 _clsSessionMgr::_attachSessionMeta( clsSession *pSession,
                                             const NET_HANDLE handle )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( CLS_SESSMGR_ATCHMETA ) ;
      clsSessionMeta * pMeta = NULL ;
      MAPMETA_IT itMeta = _mapMeta.find ( handle ) ;
      if ( itMeta == _mapMeta.end() )
      {
         pMeta = SDB_OSS_NEW clsSessionMeta ( handle ) ;
         if ( NULL == pMeta )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for meta" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         _mapMeta[handle] = pMeta ;
      }
      else
      {
         pMeta = itMeta->second ;
      }
      pMeta->incBaseHandleNum() ;
      pSession->meta ( pMeta ) ;

   done:
      PD_TRACE_EXITRC ( CLS_SESSMGR_ATCHMETA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_STARTEDU, "_clsSessionMgr::_startSessionEDU" )
   INT32 _clsSessionMgr::_startSessionEDU( clsSession *pSession )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( CLS_SESSMGR_STARTEDU ) ;
      pmdKRCB *pKRCB = pmdGetKRCB() ;
      pmdEDUMgr *pEDUMgr = pKRCB->getEDUMgr() ;
      EDUID eduID = PMD_INVALID_EDUID ;
      pmdEDUCB *cb = NULL ;

      rc = pEDUMgr->startEDU( pSession->eduType(), (void *)pSession, &eduID ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_QUIESCED == rc )
         {
            PD_LOG ( PDWARNING, "Reject new connection due to quiesced "
                     "database" ) ;
         }
         else
         {
            PD_LOG ( PDERROR, "Failed to create subagent thread, rc: %d",
                     rc ) ;
         }
         goto error ;
      }

      //Wait the EDUCB is in the session
      pSession->waitAttach () ;

   done:
      PD_TRACE_EXITRC ( CLS_SESSMGR_STARTEDU, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_RLSSS, "_clsSessionMgr::releaseSession" )
   INT32 _clsSessionMgr::releaseSession( clsSession * pSession, BOOLEAN delay )
   {
      PD_TRACE_ENTRY ( CLS_SESSMGR_RLSSS ) ;
      if ( !_force )
      {
         MAPSESSION_IT it = _mapSession.find( pSession->sessionID() ) ;
         if ( it != _mapSession.end() )
         {
            _mapSession.erase( it ) ;
         }
      }

      INT32 rc = _releaseSession_i( pSession, TRUE, delay ) ;
      PD_TRACE_EXITRC ( CLS_SESSMGR_RLSSS, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_RLSSS_I, "_clsSessionMgr::_releaseSession_i" )
   INT32 _clsSessionMgr::_releaseSession_i ( clsSession *pSession,
                                             BOOLEAN postQuit,
                                             BOOLEAN delay )
   {
      PD_TRACE_ENTRY ( CLS_SESSMGR_RLSSS_I ) ;
      clsBuffInfo *pBuffInfo = NULL ;

      SDB_ASSERT ( pSession, "pSession can't be NULL" ) ;
      if ( !_force && postQuit && pSession->eduCB() )
      {
         // Notify the edu quit
         pSession->eduCB()->disconnect () ;
      }

      if ( delay )
      {
         ossScopedLock lock ( &_deqDeletingMutex ) ;
         _deqDeletingSessions.push_back ( pSession ) ;
         goto done ;
      }

      // Wait the EDUCB is out the session
      pSession->waitDetach () ;

      // dec based handle number
      if ( pSession->getMeta() )
      {
         pSession->getMeta()->decBaseHandleNum() ;
      }

      // Release Memory to pool
      pBuffInfo = pSession->frontBuffer() ;
      while ( pBuffInfo )
      {
         _memPool.release ( pBuffInfo->pBuffer, pBuffInfo->size ) ;
         pSession->popBuffer () ;
         pBuffInfo = pSession->frontBuffer() ;
      }
      pSession->clear() ;

      if ( !_force && _canReuse( pSession->sessionType() ) &&
           _deqCatchSessions.size() < _maxCatchSize() )
      {
         _deqCatchSessions.push_back( pSession ) ;
         goto done ;
      }

      SDB_OSS_DEL pSession ;

   done:
      PD_TRACE_EXIT ( CLS_SESSMGR_RLSSS_I );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_REPLY, "_clsSessionMgr::_reply" )
   INT32 _clsSessionMgr::_reply( const NET_HANDLE &handle, INT32 rc,
                                 const MsgHeader *pReqMsg )
   {
      INT32 ret = SDB_OK ;
      PD_TRACE_ENTRY ( CLS_SESSMGR_REPLY ) ;

      MsgOpReply reply ;
      BSONObj obj = pmdGetErrorBson( rc, "can't create session" ) ;

      if ( !_pRTAgent )
      {
         rc = SDB_INVALIDARG ;
      }

      reply.header.opCode = MAKE_REPLY_TYPE( pReqMsg->opCode ) ;
      reply.header.requestID = pReqMsg->requestID ;
      reply.header.routeID.value = 0 ;
      reply.header.TID  = pReqMsg->TID ;
      reply.header.messageLength = sizeof ( MsgOpReply ) ;
      reply.flags = rc ;
      reply.contextID = -1 ;
      reply.numReturned = 1 ;
      reply.startFrom = 0 ;

      reply.header.messageLength += obj.objsize() ;

      ret = _pRTAgent->syncSend ( handle, ( MsgHeader*)&reply,
                                  (void*)obj.objdata(),
                                  obj.objsize() ) ;

   done:
      PD_TRACE_EXITRC ( CLS_SESSMGR_REPLY, rc );
      return ret ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_HDLSNCLOSE, "_clsSessionMgr::handleSessionClose" )
   INT32 _clsSessionMgr::handleSessionClose( const NET_HANDLE handle )
   {
      PD_TRACE_ENTRY ( CLS_SESSMGR_HDLSNCLOSE ) ;
      clsSession *pSession = NULL ;
      MAPSESSION_IT it = _mapSession.begin() ;
      while ( it != _mapSession.end() )
      {
         pSession = it->second ;
         if ( pSession->netHandle() == handle )
         {
            PD_LOG ( PDEVENT, "Session[%s, handle:%d] closed",
                     pSession->sessionName(), pSession->netHandle() ) ;
            _releaseSession_i( pSession, TRUE, TRUE ) ;
            _mapSession.erase( it++ ) ;
            continue ;
         }
         ++it ;
      }

      if ( NET_INVALID_TIMER_ID == _handleCloseTimerID )
      {
         _pRTAgent->addTimer( 30 * OSS_ONE_SEC, _pTimerHandle,
                              _handleCloseTimerID ) ;
      }

      PD_TRACE_EXIT ( CLS_SESSMGR_HDLSNCLOSE ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_HDLSNTM, "_clsSessionMgr::handleSessionTimeout" )
   INT32 _clsSessionMgr::handleSessionTimeout( UINT32 timerID,
                                               UINT32 interval )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( CLS_SESSMGR_HDLSNTM ) ;

      if ( _sessionTimerID == timerID )
      {
         _checkSession( interval ) ;
      }
      else if ( _handleCloseTimerID == timerID )
      {
         _checkSessionMeta( interval ) ;
         _pRTAgent->removeTimer( _handleCloseTimerID ) ;
         _handleCloseTimerID = NET_INVALID_TIMER_ID ;
         goto done ;
      }
      else
      {
         //return not zero, the timer will dispath to main cb
         rc = -1 ;
      }

   done :
      PD_TRACE_EXITRC ( CLS_SESSMGR_HDLSNTM, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_CHKSNMETA, "_clsSessionMgr::_checkSessionMeta" )
   void _clsSessionMgr::_checkSessionMeta( UINT32 interval )
   {
      PD_TRACE_ENTRY ( CLS_SESSMGR_CHKSNMETA ) ;

      MAPMETA_IT it = _mapMeta.begin() ;
      while ( it != _mapMeta.end() )
      {
         clsSessionMeta *pMeta = it->second ;
         if ( 0 == pMeta->getBasedHandleNum() )
         {
            SDB_OSS_DEL pMeta ;
            _mapMeta.erase( it++ ) ;
            continue ;
         }
         ++it ;
      }

      PD_TRACE_EXIT ( CLS_SESSMGR_CHKSNMETA ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( CLS_SESSMGR_CHKSN, "_clsSessionMgr::_checkSession" )
   void _clsSessionMgr::_checkSession( UINT32 interval )
   {
      PD_TRACE_ENTRY ( CLS_SESSMGR_CHKSN ) ;

      clsSession *pSession = NULL ;
      MAPSESSION_IT it = _mapSession.begin() ;
      while ( it != _mapSession.end() )
      {
         pSession = it->second ;

         if ( !pSession->isProcess() && pSession->timeout( interval ) )
         {
            PD_LOG ( PDEVENT, "Session[%s] timeout", pSession->sessionName() ) ;
            _releaseSession_i ( pSession, TRUE, TRUE ) ;
            _mapSession.erase ( it++ ) ;
            continue ;
         }
         ++it ;
      }

      PD_TRACE_EXIT ( CLS_SESSMGR_CHKSN ) ;
   }

}


