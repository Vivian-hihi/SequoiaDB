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

   Source File Name = clsMsgHandler.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "clsMsgHandler.hpp"
#include "clsSession.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"


namespace engine
{
   /*
      _clsMsgHandler implement
   */
   _clsMsgHandler::_clsMsgHandler( _clsSessionMgr *pSessionMgr )
   {
      _pSessionMgr   = pSessionMgr ;
      _pMgrEDUCB     = NULL ;
   }

   _clsMsgHandler::~_clsMsgHandler()
   {
      _pSessionMgr   = NULL ;
      _pMgrEDUCB     = NULL ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMSGHND_CPMSG, "_clsMsgHandler::copyMsg" )
   void * _clsMsgHandler::copyMsg ( const CHAR* msg, UINT32 length )
   {
      PD_TRACE_ENTRY ( SDB__CLSMSGHND_CPMSG );
      CHAR *pBuffer = (CHAR * )SDB_OSS_MALLOC ( length ) ;
      if ( pBuffer )
      {
         ossMemcpy( pBuffer, msg, length ) ;
      }

      PD_TRACE_EXIT ( SDB__CLSMSGHND_CPMSG );
      return pBuffer ;
   }

   //In this function, there is a singal thread at the same time,
   //So, can not use mutex
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMSGHND_HNDMSG, "_clsMsgHandler::handleMsg" )
   INT32 _clsMsgHandler::handleMsg( const NET_HANDLE & handle, 
                                    const _MsgHeader * header, 
                                    const CHAR * msg)
   {
      //If TID not Zero, implicate external business require form client
      //or repl sync messages
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMSGHND_HNDMSG ) ;
      if ( header->TID != 0 )
      {
         rc = handleSessionMsg ( handle, header, msg ) ;
      }
      //Other msg will push to cb queue
      else
      {
         rc = handleMainMsg( handle, header, msg ) ;
      }
      PD_TRACE_EXITRC ( SDB__CLSMSGHND_HNDMSG, rc ) ;
      return rc ;
   }

   //In this function, there is a singal thread at the same time,
   //So, can not use mutex
   void _clsMsgHandler::handleClose ( const NET_HANDLE & handle,
                                      _MsgRouteID id )
   {
      PD_LOG ( PDINFO, "connection[handle:%d] closed", handle ) ;
      _pSessionMgr->handleSessionClose( handle ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMSGHND_HNDSNMSG, "_clsMsgHandler::handleSessionMsg" )
   INT32 _clsMsgHandler::handleSessionMsg (  const NET_HANDLE &handle,
                                             const _MsgHeader *header,
                                             const CHAR *msg )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__CLSMSGHND_HNDSNMSG );
      void *newMsg = NULL ;
      BOOLEAN bCreate = TRUE ;

      UINT64 sessionID = _pSessionMgr->makeSessionID( handle, header ) ;

      if ( MSG_BS_DISCONNECT == header->opCode ||
           MSG_BS_INTERRUPTE == header->opCode )
      {
         bCreate = FALSE ;
      }

      //Find the session, if not exist, need create (need start edu)
      _clsSession *pSession = _pSessionMgr->getSession( sessionID ,
                                                        CLS_SESSION_PASSIVE,
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
      PD_TRACE_EXITRC ( SDB__CLSMSGHND_HNDSNMSG, rc ) ;
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMSGHND_HNDMAINMSG, "_clsMsgHandler::handleMainMsg" )
   INT32 _clsMsgHandler::handleMainMsg( const NET_HANDLE &handle,
                                        const _MsgHeader *header,
                                        const CHAR *msg )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__CLSMSGHND_HNDMAINMSG );
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
      PD_TRACE_EXITRC ( SDB__CLSMSGHND_HNDMAINMSG, rc );
      return rc;
   error:
      goto done;
   }

   void _clsMsgHandler::_postMainMsg( const NET_HANDLE &handle,
                                      MsgHeader *pNewMsg )
   {
      _pMgrEDUCB->postEvent( pmdEDUEvent ( PMD_EDU_EVENT_MSG,
                                           PMD_EDU_MEM_ALLOC,
                                           pNewMsg, (UINT64)handle ) ) ;
   }

   /*
      _shdMsgHandler implement
   */
   _shdMsgHandler::_shdMsgHandler ( _clsSessionMgr *pSessionMgr )
      : _clsMsgHandler ( pSessionMgr )
   {
      _pShardCB = NULL ;
   }

   _shdMsgHandler::~_shdMsgHandler ()
   {
      _pShardCB = NULL ;
   }

   void _shdMsgHandler::_postMainMsg( const NET_HANDLE & handle,
                                      MsgHeader * pNewMsg )
   {
      if ( _pShardCB && ( MSG_CAT_NODEGRP_RES == (UINT32)pNewMsg->opCode ||
           MSG_CAT_QUERY_CATALOG_RSP == (UINT32)pNewMsg->opCode ||
           MSG_CAT_QUERY_SPACEINFO_RSP == (UINT32)pNewMsg->opCode ||
           ( MSG_CAT_CATGRP_RES == (UINT32)pNewMsg->opCode &&
             _pShardCB->getTID() != (UINT32)pNewMsg->requestID ) ) )
      {
         _pShardCB->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                            PMD_EDU_MEM_ALLOC,
                                            pNewMsg, (UINT64)handle ) ) ;
      }
      else
      {
         //store type to TID and dispatch restore
         pNewMsg->TID = (UINT32)CLS_SHARD ;
         _pMgrEDUCB->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                             PMD_EDU_MEM_ALLOC,
                                             pNewMsg, (UINT64)handle ) );
      }
   }

   /*
      _replMsgHandler implement
   */
   _replMsgHandler::_replMsgHandler ( _clsSessionMgr *pSessionMgr )
      :_clsMsgHandler ( pSessionMgr )
   {
   }

   _replMsgHandler::~_replMsgHandler ()
   {
   }

   void _replMsgHandler::_postMainMsg( const NET_HANDLE &handle,
                                       MsgHeader *pNewMsg )
   {
      //store type to TID and dispatch restore
      pNewMsg->TID = (UINT32)CLS_REPL ;
      _pMgrEDUCB->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                          PMD_EDU_MEM_ALLOC,
                                          pNewMsg, (UINT64)handle ) ) ;
   }

}


