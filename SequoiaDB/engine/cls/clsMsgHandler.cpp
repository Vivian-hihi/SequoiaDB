/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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
#include "clsMgr.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"


namespace engine
{
   _clsMsgHandler::_clsMsgHandler( _clsMgr *pClsMgr )
   {
      _pClsMgr = pClsMgr ;
      _pMgrEDUCB = NULL ;
      _pShardCB  = NULL ;
   }

   _clsMsgHandler::~_clsMsgHandler()
   {
      _pClsMgr = NULL ;
      _pMgrEDUCB = NULL ;
      _pShardCB  = NULL ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMSGHND_CPMSG, "_clsMsgHandler::copyMsg" )
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
   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMSGHND_HNDMSG, "_clsMsgHandler::handleMsg" )
   INT32 _clsMsgHandler::handleMsg( const NET_HANDLE & handle, 
                                    const _MsgHeader * header, 
                                    const CHAR * msg)
   {
      //If TID not Zero, implicate external business require form client
      //or repl sync messages
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSMSGHND_HNDMSG );
      if ( header->TID != 0 )
      {
         rc = handleSessionMsg ( handle, header, msg ) ;
      }
      //Other msg will push to cls queue
      else
      {
         rc = handleClsMsg( handle, header, msg ) ;
      }
      PD_TRACE_EXITRC ( SDB__CLSMSGHND_HNDMSG, rc );
      return rc ;
   }

   //In this function, there is a singal thread at the same time,
   //So, can not use mutex
   void _clsMsgHandler::handleClose ( const NET_HANDLE & handle, _MsgRouteID id )
   {
      PD_LOG ( PDINFO, "connection[handle:%d] closed", handle ) ;
      _pClsMgr->handleSessionClose( type(), handle ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMSGHND_HNDSNMSG, "_clsMsgHandler::handleSessionMsg" )
   INT32 _clsMsgHandler::handleSessionMsg (  const NET_HANDLE &handle,
                                             const _MsgHeader *header,
                                             const CHAR *msg )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__CLSMSGHND_HNDSNMSG );
      void *newMsg = NULL ;
      BOOLEAN bCreate = TRUE ;

      UINT64 sessionID = ossPack32To64( header->routeID.columns.nodeID, 
                                        header->TID );
      if ( CLS_SHARD == type () &&
           MSG_INVALID_ROUTEID == header->routeID.value )
      {
         sessionID = ossPack32To64 ( CLS_BASE_HANDLE_ID + handle, header->TID ) ;
      }

      if ( MSG_BS_DISCONNECT == header->opCode ||
           MSG_BS_INTERRUPTE == header->opCode )
      {
         bCreate = FALSE ;
      }

      //Find the session, if not exist, need create (need start edu)
      _clsSession *pSession = _pClsMgr->getSession( type() , sessionID ,
                                                    CLS_SESSION_PASSIVE,
                                                    handle, bCreate, NULL );
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

      pSession->eduCB()->postEvent( pmdEDUEvent(PMD_EDU_EVENT_MSG, FALSE, newMsg) ) ;

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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMSGHND_HNDCLSMSG, "_clsMsgHandler::handleClsMsg" )
   INT32 _clsMsgHandler::handleClsMsg( const NET_HANDLE &handle,
                                       const _MsgHeader *header,
                                       const CHAR *msg )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__CLSMSGHND_HNDCLSMSG );
      _MsgHeader *newHeader = NULL ;

      void *newMsg = copyMsg ( msg, header->messageLength ) ;
      if ( NULL == newMsg )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory for new msg" ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      newHeader = ( MsgHeader * )newMsg ;
      //store handle
      _pClsMgr->pushMsgHandle ( newMsg, handle ) ;

      if ( _pShardCB && ( MSG_CAT_NODEGRP_RES == (UINT32)newHeader->opCode ||
           MSG_CAT_QUERY_CATALOG_RSP == (UINT32)newHeader->opCode ||
           MSG_CAT_QUERY_SPACEINFO_RSP == (UINT32)newHeader->opCode ||
           ( MSG_CAT_CATGRP_RES == (UINT32)newHeader->opCode &&
             _pShardCB->getTID() != (UINT32)newHeader->requestID ) ) )
      {
         _pShardCB->postEvent( pmdEDUEvent(PMD_EDU_EVENT_MSG, TRUE, newMsg) );
      }
      else
      {
         //store type to TID and dispatch restore
         newHeader->TID = (UINT32)type() ;
         _pMgrEDUCB->postEvent( pmdEDUEvent(PMD_EDU_EVENT_MSG, TRUE, newMsg) );
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSMSGHND_HNDCLSMSG, rc );
      return rc;
   error:
      goto done;
   }

   _shdMsgHandler::_shdMsgHandler ( _clsMgr *pClsMgr )
      : _clsMsgHandler ( pClsMgr )
   {
   }

   _shdMsgHandler::~_shdMsgHandler ()
   {
   }

   INT32 _shdMsgHandler::type () const
   {
      return CLS_SHARD ;
   }

   _replMsgHandler::_replMsgHandler ( _clsMgr *pClsMgr )
      :_clsMsgHandler ( pClsMgr )
   {
   }

   _replMsgHandler::~_replMsgHandler ()
   {
   }

   INT32 _replMsgHandler::type () const
   {
      return CLS_REPL ;
   }

}


