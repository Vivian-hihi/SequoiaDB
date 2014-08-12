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

   Source File Name = pmdAsyncHandler.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          1/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "core.hpp"
#include "pmdAsyncHandler.hpp"
#include "pmdAsyncSession.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   /*
      _pmdAsyncTimerHandler implement
   */
   _pmdAsyncTimerHandler::_pmdAsyncTimerHandler( _pmdAsycSessionMgr * pSessionMgr )
   {
      _pMgrCB = NULL ;
      _pSessionMgr = pSessionMgr ;
   }

   _pmdAsyncTimerHandler::~_pmdAsyncTimerHandler()
   {
      _pMgrCB = NULL ;
      _pSessionMgr = NULL ;
   }

   UINT64 _pmdAsyncTimerHandler::_makeTimerID( UINT32 timerID )
   {
      return ( UINT64 )timerID ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDTMHD_HDTMOUT, "_pmdAsyncTimerHandler::handleTimeout" )
   void _pmdAsyncTimerHandler::handleTimeout( const UINT32 &millisec,
                                              const UINT32 &id )
   {
      PD_TRACE_ENTRY ( SDB__PMDTMHD_HDTMOUT ) ;
      UINT64 timerID = _makeTimerID ( id ) ;

      if ( _pSessionMgr->handleSessionTimeout( timerID , millisec ) != SDB_OK )
      {
         PMD_EVENT_MESSAGES *eventMsg = (PMD_EVENT_MESSAGES *)
            SDB_OSS_MALLOC( sizeof (PMD_EVENT_MESSAGES ) ) ;

         if ( NULL == eventMsg )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for PDM "
                     "timeout Event" ) ;
         }
         else
         {
            ossTimestamp ts;
            ossGetCurrentTime(ts);

            eventMsg->timeoutMsg.interval = millisec ;
            eventMsg->timeoutMsg.occurTime = ts.time ;
            eventMsg->timeoutMsg.timerID = timerID ;

            _pMgrCB->postEvent( pmdEDUEvent ( PMD_EDU_EVENT_TIMEOUT, 
                                              PMD_EDU_MEM_ALLOC,
                                              (void*)eventMsg ) ) ;
         }
      }
      PD_TRACE_EXIT ( SDB__PMDTMHD_HDTMOUT ) ;
   }

   /*
      _pmdAsyncMsgHandler implement
   */
   _pmdAsyncMsgHandler::_pmdAsyncMsgHandler( _pmdAsycSessionMgr *pSessionMgr )
   {
      _pSessionMgr   = pSessionMgr ;
      _pMgrEDUCB     = NULL ;
   }

   _pmdAsyncMsgHandler::~_pmdAsyncMsgHandler()
   {
      _pSessionMgr   = NULL ;
      _pMgrEDUCB     = NULL ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDMSGHND_CPMSG, "_pmdAsyncMsgHandler::copyMsg" )
   void * _pmdAsyncMsgHandler::copyMsg ( const CHAR* msg, UINT32 length )
   {
      PD_TRACE_ENTRY ( SDB__PMDMSGHND_CPMSG );
      CHAR *pBuffer = (CHAR * )SDB_OSS_MALLOC ( length ) ;
      if ( pBuffer )
      {
         ossMemcpy( pBuffer, msg, length ) ;
      }

      PD_TRACE_EXIT ( SDB__PMDMSGHND_CPMSG );
      return pBuffer ;
   }

   //In this function, there is a singal thread at the same time,
   //So, can not use mutex
   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDMSGHND_HNDMSG, "_pmdAsyncMsgHandler::handleMsg" )
   INT32 _pmdAsyncMsgHandler::handleMsg( const NET_HANDLE & handle, 
                                         const _MsgHeader * header, 
                                         const CHAR * msg)
   {
      //If TID not Zero, implicate external business require form client
      //or repl sync messages
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDMSGHND_HNDMSG ) ;
      if ( header->TID != 0 )
      {
         rc = handleSessionMsg ( handle, header, msg ) ;
      }
      //Other msg will push to cb queue
      else
      {
         rc = handleMainMsg( handle, header, msg ) ;
      }
      PD_TRACE_EXITRC ( SDB__PMDMSGHND_HNDMSG, rc ) ;
      return rc ;
   }

   //In this function, there is a singal thread at the same time,
   //So, can not use mutex
   void _pmdAsyncMsgHandler::handleClose ( const NET_HANDLE & handle,
                                           _MsgRouteID id )
   {
      PD_LOG ( PDINFO, "connection[handle:%d] closed", handle ) ;
      _pSessionMgr->handleSessionClose( handle ) ;
   }

   void _pmdAsyncMsgHandler::onStop()
   {
      _pSessionMgr->handleStop() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDMSGHND_HNDSNMSG, "_pmdAsyncMsgHandler::handleSessionMsg" )
   INT32 _pmdAsyncMsgHandler::handleSessionMsg ( const NET_HANDLE &handle,
                                                 const _MsgHeader *header,
                                                 const CHAR *msg )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__PMDMSGHND_HNDSNMSG );
      void *newMsg = NULL ;
      BOOLEAN bCreate = TRUE ;

      UINT64 sessionID = _pSessionMgr->makeSessionID( handle, header ) ;

      if ( MSG_BS_DISCONNECT == header->opCode ||
           MSG_BS_INTERRUPTE == header->opCode )
      {
         bCreate = FALSE ;
      }

      //Find the session, if not exist, need create (need start edu)
      _pmdAsyncSession *pSession = _pSessionMgr->getSession( sessionID ,
                                                             PMD_SESSION_PASSIVE,
                                                             handle, bCreate,
                                                             header->opCode,
                                                             NULL ) ;
      if ( NULL == pSession )
      {
         if ( !bCreate )
         {
            goto done ;
         }

         PD_LOG ( PDERROR, "Failed to create session[ID:%lld]",
                  sessionID ) ;
         rc = SDB_OOM ;
         goto error;
      }

      //On recieve
      pSession->onRecieve ( handle, (_MsgHeader*)header ) ;

      if ( MSG_BS_DISCONNECT == header->opCode )
      {
         PD_LOG ( PDEVENT, "Session[%s] recieved disconnect message", 
                  pSession->sessionName() ) ;
         rc = _pSessionMgr->releaseSession( pSession, TRUE ) ;
         goto done ;
      }
      else if ( MSG_BS_INTERRUPTE == header->opCode )
      {
         PD_LOG ( PDEVENT, "Session[%s] recieved interrupt message", 
                  pSession->sessionName() ) ;
         pSession->eduCB()->interrupt() ;
      }

      rc = _pSessionMgr->pushMessage( pSession, header, handle ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__PMDMSGHND_HNDSNMSG, rc ) ;
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDMSGHND_HNDMAINMSG, "_pmdAsyncMsgHandler::handleMainMsg" )
   INT32 _pmdAsyncMsgHandler::handleMainMsg( const NET_HANDLE &handle,
                                             const _MsgHeader *header,
                                             const CHAR *msg )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__PMDMSGHND_HNDMAINMSG );
      _MsgHeader *newHeader = NULL ;
      void *newMsg = NULL ;

      SDB_ASSERT( _pMgrEDUCB, "Main edu can't be NULL" ) ;
      if ( !_pMgrEDUCB )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      newMsg = copyMsg ( msg, header->messageLength ) ;
      if ( NULL == newMsg )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory for new msg" ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      newHeader = ( MsgHeader * )newMsg ;

      _postMainMsg( handle, newHeader ) ;

   done:
      PD_TRACE_EXITRC ( SDB__PMDMSGHND_HNDMAINMSG, rc );
      return rc;
   error:
      goto done;
   }

   void _pmdAsyncMsgHandler::_postMainMsg( const NET_HANDLE &handle,
                                           MsgHeader *pNewMsg )
   {
      _pMgrEDUCB->postEvent( pmdEDUEvent ( PMD_EDU_EVENT_MSG,
                                           PMD_EDU_MEM_ALLOC,
                                           pNewMsg, (UINT64)handle ) ) ;
   }


}

