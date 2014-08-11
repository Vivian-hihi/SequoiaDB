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

   Source File Name = omagentSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentSession.hpp"
#include "omagentMgr.hpp"
#include "pmdCommon.hpp"
#include "msgMessage.hpp"
#include "omagentHelper.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   /*
      Local define
   */
   #define OMAGENT_SESESSION_TIMEOUT         ( 120 )

   /*
      _omaSession implement
   */
   BEGIN_OBJ_MSG_MAP( _omaSession, _clsSession )
      // msg map or event map
      ON_MSG( MSG_CM_REMOTE, _onNodeMgrReq )
      ON_MSG( MSG_BS_QUERY_REQ, _onOMAgentReq )
   END_OBJ_MSG_MAP()

   _omaSession::_omaSession( UINT64 sessionID )
   :_clsSession( sessionID )
   {
      ossMemset( (void*)&_replyHeader, 0, sizeof(_replyHeader) ) ;
      _pAgent = NULL ;
      _pNodeMgr = NULL ;
      _pBody = NULL ;
      _bodyLen = 0 ;
   }

   _omaSession::~_omaSession()
   {
      if ( _pBody )
      {
         SAFE_OSS_DELETE ( _pBody ) ;
         _bodyLen = 0 ;
      }
   }

   SDB_SESSION_TYPE _omaSession::sessionType() const
   {
      return SDB_SESSION_OMAGENT ;
   }

   EDU_TYPES _omaSession::eduType() const
   {
      return EDU_TYPE_AGENT ;
   }

   void _omaSession::onRecieve( const NET_HANDLE netHandle, MsgHeader * msg )
   {
      ossGetCurrentTime( _lastRecvTime ) ;
   }

   BOOLEAN _omaSession::timeout ( UINT32 interval )
   {
      BOOLEAN ret = FALSE ;
      ossTimestamp curTime ;
      ossGetCurrentTime ( curTime ) ;

      if ( curTime.time - _lastRecvTime.time > OMAGENT_SESESSION_TIMEOUT )
      {
         // will be release
         ret = TRUE ;
         goto done ;
      }

   done :
      return ret ;
   }

   void _omaSession::onTimer( UINT64 timerID, UINT32 interval )
   {
   }

   void _omaSession::_onDetach()
   {
   }

   void _omaSession::_onAttach()
   {
      _pNodeMgr = sdbGetOMAgentMgr()->getNodeMgr() ;
      _pAgent = sdbGetOMAgentMgr()->getRouteAgent() ;
   }

   INT32 _omaSession::_defaultMsgFunc( NET_HANDLE handle, MsgHeader * msg )
   {
      PD_LOG( PDWARNING, "Session[%s] Recieve unknow msg[type:[%d]%u, len:%u]",
              sessionName(), IS_REPLY_TYPE( msg->opCode ) ? 1 : 0,
              GET_REQUEST_TYPE( msg->opCode ), msg->messageLength ) ;

      return _reply( SDB_CLS_UNKNOW_MSG, msg ) ;
   }

   INT32 _omaSession::_reply( MsgOpReply *header, const CHAR *pBody,
                              INT32 bodyLen )
   {
      INT32 rc = SDB_OK ;

      if ( (UINT32)(header->header.messageLength) !=
           sizeof (MsgOpReply) + bodyLen )
      {
         PD_LOG ( PDERROR, "Session[%s] reply message length error[%u != %u]",
                  sessionName() ,header->header.messageLength,
                  sizeof ( MsgOpReply ) + bodyLen ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      //Send message
      if ( bodyLen > 0 )
      {
         rc = _pAgent->syncSend ( _netHandle, (MsgHeader *)header, 
                                  (void*)pBody, bodyLen ) ;
      }
      else
      {
         rc = _pAgent->syncSend ( _netHandle, (void *)header ) ;
      }

      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Session[%s] send reply message failed[rc:%d]",
                  sessionName(), rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaSession::_reply( INT32 flags, MsgHeader * pSrcReqMsg )
   {
      const CHAR *pBody = NULL ;
      INT32 bodyLen     = 0 ;

      //Build reply message
      _replyHeader.header.opCode = MAKE_REPLY_TYPE( pSrcReqMsg->opCode ) ;
      _replyHeader.header.messageLength = sizeof ( MsgOpReply ) ;
      _replyHeader.header.requestID = pSrcReqMsg->requestID ;
      _replyHeader.header.TID = pSrcReqMsg->TID ;
      _replyHeader.header.routeID.value = 0 ;
      _replyHeader.flags = flags ;
      _replyHeader.contextID = -1 ;
      _replyHeader.numReturned = 0 ;
      _replyHeader.startFrom = 0 ;

      if ( flags )
      {
         _errorInfo = pmdGetErrorBson( flags, _pEDUCB->getInfo(
                                       EDU_INFO_ERROR ) ) ;
         bodyLen  = _errorInfo.objsize() ;
         pBody    = _errorInfo.objdata() ;
         _replyHeader.header.messageLength += bodyLen ;
         _replyHeader.numReturned = 1 ;
      }

      return _reply( &_replyHeader, pBody, bodyLen ) ;
   }

   INT32 _omaSession::_onNodeMgrReq( const NET_HANDLE & handle,
                                     MsgHeader * pMsg )
   {
      INT32 rc = SDB_OK ;
      MsgCMRequest *pCMReq = ( MsgCMRequest* )pMsg ;
      INT32 remoteCode = 0 ;
      CHAR *arg1 = NULL ;
      CHAR *arg2 = NULL ;
      CHAR *arg3 = NULL ;
      CHAR *arg4 = NULL ;

      if ( pMsg->messageLength < (SINT32)sizeof (MsgCMRequest) )
      {
         PD_LOG( PDERROR, "Session[%s] recieve invalid msg[opCode: %d, "
                 "len: %d]", sessionName(), pMsg->opCode,
                 pMsg->messageLength ) ;
         goto done ;
      }

      rc = msgExtractCMRequest ( ( CHAR*)pMsg, &remoteCode, &arg1, &arg2,
                                 &arg3, &arg4 ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Session[%s]failed to extract cm request, rc: %d",
                  sessionName(), rc ) ;
         goto done ;
      }

      switch( pCMReq->remoCode )
      {
         case SDBSTART :
            rc = _pNodeMgr->startANode( arg1 ) ;
            break ;
         case SDBSTOP :
            rc = _pNodeMgr->stopANode( arg1 ) ;
            break ;
         case SDBADD :
            rc = _pNodeMgr->addANode( arg1, arg2 ) ;
            break ;
         case SDBMODIFY :
            rc = _pNodeMgr->mdyANode( arg1 ) ;
            break ;
         case SDBRM :
            rc = _pNodeMgr->rmANode( arg1, arg2 ) ;
            break ;
         case SDBSTARTALL :
            rc = _pNodeMgr->startAllNodes( NODE_START_CLIENT ) ;
            break ;
         case SDBSTOPALL :
            rc = _pNodeMgr->stopAllNodes() ;
            break ;
         default :
            PD_LOG( PDERROR, "Unknow remote code[%d] in session[%s]",
                    pCMReq->remoCode, sessionName() ) ;
            rc = SDB_INVALIDARG ;
            break ;
      }

      if ( rc )
      {
         PD_LOG( PDERROR, "Session[%s] process remote code[%d] failed, rc: %d",
                 sessionName(), pCMReq->remoCode, rc ) ;
      }

   done:
      return _reply( rc, pMsg ) ;
   }

   INT32 _omaSession::_onOMAgentReq( const NET_HANDLE &handle,
                                     MsgHeader &pMsg )
   {
      INT32 rc = SDB_OK ;
      INT32 flags               = 0 ;
      CHAR *pCollectionName     = NULL ;
      CHAR *pQuery              = NULL ;
      CHAR *pFieldSelector      = NULL ;
      CHAR *pOrderByBuffer      = NULL ;
      CHAR *pHintBuffer         = NULL ;
      SINT64 numToSkip          = -1 ;
      SINT64 numToReturn        = -1 ;
      _omaCommand *pCommand = NULL ;

      PD_LOG ( PDEVENT, "Omagent receive requset from omsvc" ) ;
      // build reply massage header
      _buildReplyHeader( (MsgHeader *)(&pMsg) ) ;
      // extract command
      rc = msgExtractQuery ( (CHAR *)(&pMsg), &flags, &pCollectionName,
                             &numToSkip, &numToReturn, &pQuery,
                             &pFieldSelector, &pOrderByBuffer,
                             &pHintBuffer ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR,
                  "Session[%s] extract omsvc's command msg failed, rc: %d",
                  sessionName(), rc ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // handle command
      if ( omaIsCommand ( pCollectionName ) )
      {
         PD_LOG( PDEVENT, "Omagent receive command: %s", pCollectionName ) ;
         rc = omaParseCommand ( pCollectionName, &pCommand ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to parse omsvc's command[%s] [rc:%d]",
                    pCollectionName, rc ) ;
            goto error ;
         }
         rc = omaInitCommand( pCommand, flags, numToSkip, numToReturn,
                                  pQuery, pFieldSelector, pOrderByBuffer,
                                  pHintBuffer ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR,
                    "Failed to init omsvc's command for omagent, rc = %d",
                    rc ) ;
            goto error ;
         }
         rc = omaRunCommand( pCommand, &_pBody, _bodyLen ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to run omsvc's command, rc = %d", rc ) ;
            goto error ;
         }
         _replyHeader.numReturned = 1 ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Omsvc's request is not a command" ) ;
         goto error ;
      }
   done:
      if ( pCommand )
      {
         omaReleaseCommand( &pCommand ) ;
      }
      return _reply( &_replyHeader, _pBody, _bodyLen ) ;
   error:
      _replyHeader.flags = rc ;
      goto done ;
   }

   INT32 _omaSession::_buildReplyHeader( MsgHeader *pMsg )
   {
      _replyHeader.header.messageLength = sizeof( MsgOpReply ) ;
      _replyHeader.header.opCode        = MAKE_REPLY_TYPE(pMsg->opCode) ;
      _replyHeader.header.TID           = pMsg->TID ;
      _replyHeader.header.routeID.value = 0 ;
      _replyHeader.header.requestID     = pMsg->requestID ;

      _replyHeader.contextID            = -1 ;
      _replyHeader.flags                = 0 ;
      _replyHeader.numReturned          = 0 ;
      _replyHeader.startFrom            = 0 ;
      _replyHeader.numReturned          = 0 ;

      return SDB_OK ;
   }


/*
   INT32 _omaSession::_processMsg( MsgHeader *msg )
   {
      INT32 rc          = SDB_OK ;
      CHAR* pBody = NULL ;
      INT32 bodyLen     = 0 ;

      // build reply msg header
      rc = _buildReplyHeader( msg ) ;
      if ( rc )
      {

      }
      // process msg
      rc = _processOPMsg( msg, &pBody, bodyLen,
                          _replyHeader.numReturned ) ;
      if ( rc )
      {
         ossPrintf("rc is : %d\n", rc) ;
      }
      while( 1 )
      {
         ossSleepsecs ( 1 ) ;
      }

      BSONObj temp( pBody ) ;
      ossPrintf( "result :\n" ) ;
      ossPrintf( "pBody is : %s\n", temp.toString().c_str() ) ;
      ossPrintf( "bodyLen is : %d\n", bodyLen ) ;
      ossPrintf( "numReturned is : %d\n", _replyHeader.numReturned ) ;

      */
      /*
      if ( _needReply )
      {
         if ( rc && bodyLen == 0 )
         {
            _errorInfo = BSON( "errmsg" << "something wrong for process omsvc reques"
                               << "errcode" << rc ) ;
            _replyHeader.numReturned = -1 ;
         }
         _replyHeader.flags = rc ;
         _replayHeader.header.messageLength = sizeof( _replyHeader ) + bodyLen ;

         // send response
         INT32 rcTmp = _reply( &_replyHeader, pBody, bodyLen ) ;
         if ( rcTmp )
         {
            PD_LOG ( PDERROR, "Session[%s] failed to send response for omsvc, rc: %d",
                     sessionName(), rcTmp ) ;
         }
      }*/
      /*

   done:
      if ( NULL  != pBody )
      {
         SDB_OSS_FREE( pBody ) ;
         pBody = NULL ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaSession::_processOPMsg( MsgHeader *msg, CHAR **ppBody,
                                     INT32 &bodyLen, INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;

      if ( NULL == ppBody )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      switch( msg->opCode )
      {
         case MSG_BS_QUERY_REQ :
            rc = _onQueryReqMsg( msg, ppBody, bodyLen, returnNum ) ;
            break ;
         default :
            PD_LOG( PDERROR, "Session[%s] recv unknow msg from omsvc[type:[%d]%d, "
                    "len: %d, tid: %d, routeID: %d.%d.%d, reqID: %lld]",
                    sessionName(), IS_REPLY_TYPE(msg->opCode),
                    GET_REQUEST_TYPE(msg->opCode), msg->messageLength, msg->TID,
                    msg->routeID.columns.groupID, msg->routeID.columns.nodeID,
                    msg->routeID.columns.serviceID, msg->requestID ) ;
            rc = SDB_INVALIDARG ;
            break ;
      }

      if  ( rc )
      {
         goto error ;
      }

      bodyLen = objBuff.size() ;
      returnNum = objBuff.recordNum() ;
      if ( bodyLen > 0 )
         ossMemcpy( *ppBody, objBuff.data(), bodyLen ) ;
   done:
      return rc ;
   error:
      if ( _needRollback )
      {
         INT32 rcTmp = _omaRollbak() ;
         PD_LOG( PDEVENT, "Something wrong, need to rollback" ) ;
         _needRollback = FALSE ;
      }
      goto done ;
   }

   INT32 _omaSession::_buildReplyHeader( MsgHeader *pMsg )
   {
      _replyHeader.header.messageLength = sizeof( MsgOpReply ) ;
      _replyHeader.header.opCode        = MAKE_REPLY_TYPE(pMsg->opCode) ;
      _replyHeader.header.TID           = pMsg->TID ;
      _replyHeader.header.routeID.value = 0 ;
      _replyHeader.header.requestID     = pMsg->requestID ;

      _replyHeader.contextID            = -1 ;
      _replyHeader.flags                = 0 ;
      _replyHeader.numReturned          = 0 ;
      _replyHeader.startFrom            = 0 ;
      _replyHeader.numReturned          = 0 ;

      return SDB_OK ;
   }

   INT32 _omaSession::_onQueryReqMsg( MsgHeader *msg, CHAR **ppBody,
                                      INT32 &bodyLen, INT32 &returnNum )
   {
      INT32 rc                  = SDB_OK ;
      INT32 flags               = 0 ;
      CHAR *pCollectionName     = NULL ;
      CHAR *pQuery              = NULL ;
      CHAR *pFieldSelector      = NULL ;
      CHAR *pOrderByBuffer      = NULL ;
      CHAR *pHintBuffer         = NULL ;
      SINT64 numToSkip          = -1 ;
      SINT64 numToReturn        = -1 ;
      _omaCommand *pCommand = NULL ;

      PD_LOG ( PDEVENT, "omsvc request received" ) ;
      // extract command
      rc = msgExtractQuery ( (CHAR *)msg, &flags, &pCollectionName,
                             &numToSkip, &numToReturn, &pQuery,
                             &pFieldSelector, &pOrderByBuffer,
                             &pHintBuffer ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Session[%s] extract omsvc's command msg failed, rc: %d",
                  sessionName(), rc ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // is command
      if ( omaIsCommand ( pCollectionName ) )
      {
         PD_LOG( PDEVENT, "omagent receive command: %s", pCollectionName ) ;
         rc = omaParseCommand ( pCollectionName, &pCommand ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to parse omsvc's command[%s] [rc:%d]",
                    pCollectionName, rc ) ;
            goto error ;
         }
         rc = omaInitCommand( pCommand, flags, numToSkip, numToReturn,
                                  pQuery, pFieldSelector, pOrderByBuffer,
                                  pHintBuffer ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to init omsvc's command for omagent, rc = %d", rc ) ;
            goto error ;
         }
         rc = omaRunCommand( pCommand, ppBody, bodyLen, returnNum ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to run omsvc's command, rc = %d", rc ) ;
            goto error ;
         }
      }
   done:
      if ( pCommand )
      {
         omaReleaseCommand( &pCommand ) ;
      }
      return rc ;
   error:
    goto done ;
   }
   */

} // namespace engine

