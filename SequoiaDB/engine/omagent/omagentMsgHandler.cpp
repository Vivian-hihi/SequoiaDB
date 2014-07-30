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

   Source File Name = omagentMsgHandler.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/06/2012  Tan Zhaobo  Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentMsgHandler.hpp"
#include "omagentMgr.hpp"
#include "pmdEDU.hpp"

namespace engine
{
   _omagentMsgHandler::_omagentMsgHandler ()
   {
      _pMainCB       = NULL ;
      _pOMAgentMgr   = sdbGetOMAgentMgr() ;
   }

   _omagentMsgHandler::~_omagentMsgHandler ()
   {
      _pMainCB       = NULL ;
      _pOMAgentMgr   = NULL ;
   }

   void _omagentMsgHandler::attach( _pmdEDUCB * cb )
   {
      _pMainCB = cb ;
   }

   void _omagentMsgHandler::detach()
   {
      _pMainCB = NULL ;
   }

   INT32 _omagentMsgHandler::handleMsg ( const NET_HANDLE &handle,
                                         const _MsgHeader *header,
                                         const CHAR *msg )
   {
      INT32 rc = SDB_OK ;

      // main cb msg
      if ( header->TID == 0 )
      {
         CHAR *pNewMsg = NULL ;
         SDB_ASSERT( _pMainCB, "Main cb can't be NULL" ) ;
         if ( !_pMainCB )
         {
            PD_LOG( PDERROR, "Main cb handler is null when recv "
                    "msg[opCode:%d]", header->opCode ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         pNewMsg = (CHAR*)SDB_OSS_MALLOC( header->messageLength + 1 ) ;
         if ( !pNewMsg )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Failed to alloc memory for msg[opCode: %d, "
                    "len: %d], rc: %d", header->opCode, header->messageLength,
                    rc ) ;
            goto error ;
         }

         // copy msg
         ossMemcpy( pNewMsg, msg, header->messageLength ) ;
         pNewMsg[ header->messageLength ] = 0 ;
         // push event
         _pMainCB->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                           PMD_EDU_MEM_ALLOC,
                                           pNewMsg, (UINT64)handle ) ) ;
      }
      // session msg
      else
      {
         rc = _handleSessionMsg( handle, header, msg ) ;
      }

   done:
      return rc ;
   error:
      goto done;
   }

   INT32 _omagentMsgHandler::_handleSessionMsg( const NET_HANDLE &handle,
                                                const _MsgHeader *header,
                                                const CHAR *msg )
   {
      INT32 rc = SDB_OK;
      void *newMsg = NULL ;
      BOOLEAN bCreate = TRUE ;

      UINT64 sessionID = ossPack32To64( handle, header->TID ) ;

      if ( MSG_BS_DISCONNECT == header->opCode ||
           MSG_BS_INTERRUPTE == header->opCode )
      {
         bCreate = FALSE ;
      }

      //Find the session, if not exist, need create (need start edu)
      _clsSession *pSession = _pOMAgentMgr->getSession( sessionID ,
                                                        CLS_SESSION_PASSIVE,
                                                        handle, bCreate, NULL ) ;
      if ( NULL == pSession )
      {
         if ( !bCreate )
         {
            goto done ;
         }

         PD_LOG ( PDERROR, "Failed to create session[ID:%lld, type:%d]",
                  sessionID, type () ) ;
         rc = SDB_OOM ;
         goto error;
      }

      //On recieve
      pSession->onRecieve ( handle, (_MsgHeader*)header ) ;

      if ( MSG_BS_DISCONNECT == header->opCode )
      {
         PD_LOG ( PDEVENT, "Session[%s] recieved disconnect message", 
            pSession->sessionName() ) ;
         rc = _pClsMgr->_releaseSession( pSession, TRUE ) ;
         goto done ;
      }
      else if ( MSG_BS_INTERRUPTE == header->opCode )
      {
         PD_LOG ( PDEVENT, "Session[%s] recieved interrupt message", 
            pSession->sessionName() ) ;
         pSession->eduCB()->interrupt () ;
      }

      rc = _pClsMgr->assignMemory ( pSession, header->messageLength ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      //Post the msg to the edu queue, need create new msg in clsMemPool
      newMsg = pSession->copyMsg( msg, header->messageLength );
      if ( NULL == newMsg )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory for new msg" );
         rc = SDB_OOM;
         goto error;
      }

      pSession->eduCB()->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                                 PMD_EDU_MEM_NONE,
                                                 newMsg ) ) ;

   done:
      PD_TRACE_EXITRC ( SDB__CLSMSGHND_HNDSNMSG, rc );
      return rc;
   error:
      // need to response error
      if ( CLS_SHARD == type() && MSG_INVALID_ROUTEID == header->routeID.value )
      {
         MsgOpReply reply ;
         reply.header.opCode = MAKE_REPLY_TYPE(header->opCode) ;
         reply.header.requestID = header->requestID ;
         reply.header.routeID.value = 0 ;
         reply.header.TID  = header->TID ;
         reply.header.messageLength = sizeof ( MsgOpReply ) ;
         reply.flags = rc ;
         reply.contextID = -1 ;
         reply.numReturned = 1 ;
         reply.startFrom = 0 ;

         BSONObjBuilder bb ;
         bb.append ( OP_ERRNOFIELD, rc ) ;
         bb.append ( OP_ERRDESP_FIELD, getErrDesp ( rc ) ) ;
         bb.append ( OP_ERR_DETAIL, "can't create session" ) ;
         BSONObj errorObj = bb.obj () ;
         reply.header.messageLength += errorObj.objsize() ;

         _pClsMgr->getShardRouteAgent()->syncSend ( handle, ( MsgHeader*)&reply,
                                                    (void*)errorObj.objdata(),
                                                    errorObj.objsize() ) ;
      }
      goto done;
   }

   INT32 _omagentMsgHandler::handleClose ( const NET_HANDLE *handle,
                                           _MsgRouteID id )
   {
      INT32 rc = SDB_OK ;
      // TODO:XUJIANHUI
   done:
      return rc ;
   error:
      goto done;
   }

}

