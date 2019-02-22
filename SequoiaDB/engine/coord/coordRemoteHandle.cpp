/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = coordRemoteHandle.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/17/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "coordRemoteHandle.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "msgMessageFormat.hpp"
#include "schedDef.hpp"
#include "coordCB.hpp"
#include "coordResource.hpp"
#include "msgMessage.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{

   #define COORD_EXPIRED_KILLCONTEXT_TIMEOUT       ( 30000 )      /// 30s

   /*
      _coordRemoteHandlerBase implement
   */
   _coordRemoteHandlerBase::_coordRemoteHandlerBase()
   {
      _initType      = INIT_V1 ;
      _initFinished  = TRUE ;
      _nodeVer       = 0 ;
      _nodeID        = 0 ;
   }

   _coordRemoteHandlerBase::~_coordRemoteHandlerBase()
   {
   }

   INT32 _coordRemoteHandlerBase::onSendFailed( _pmdRemoteSession *pSession,
                                                _pmdSubSession **ppSub,
                                                INT32 flag )
   {
      return flag ;
   }

   void _coordRemoteHandlerBase::onReply( _pmdRemoteSession *pSession,
                                          _pmdSubSession **ppSub,
                                          const MsgHeader *pReply,
                                          BOOLEAN isPending )
   {
      if ( !_initFinished || 0 != _nodeID )
      {
         MsgOpReply *pOpReply = ( MsgOpReply* )pReply ;
         INT32 orgRspOpCode = (*ppSub)->getOrgRspOpCode() ;
         pmdEDUCB *cb = pSession->getEDUCB() ;
         pmdRemoteSessionSite* pSite = NULL ;
         pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;

         if ( !_initFinished &&
              ( SDB_INVALID_ROUTEID == pOpReply->flags ||
                ( SDB_OK != pOpReply->flags &&
                  ( MSG_COM_SESSION_INIT_RSP == orgRspOpCode ||
                    MSG_PACKET_RES == orgRspOpCode )
                 )
               )
             )
         {
            pSession->reConnectSubSession( pReply->routeID.value ) ;
         }

         _initFinished = TRUE ;

         /// update node version
         if ( 0 != _nodeID &&
              ( SDB_OK == pOpReply->flags ||
                ( MSG_COM_SESSION_INIT_RSP != orgRspOpCode &&
                  MSG_PACKET_RES != orgRspOpCode )
              )
            )
         {
            pSite->setNodeVer( _nodeID, _nodeVer ) ;
         }

         _nodeID  = 0 ;
         _nodeVer = 0 ;
      }
   }

   INT32 _coordRemoteHandlerBase::_onSendConnectOld( _pmdSubSession *pSub )
   {
      INT32 rc = SDB_OK ;
      pmdEDUCB *cb = NULL ;
      pmdRemoteSessionSite* pSite = NULL ;
      pmdRemoteSession *pInitSession = NULL ;

      cb = pSub->parent()->getEDUCB() ;
      pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;

      pInitSession = pSite->addSession( -1, NULL ) ;
      if ( !pInitSession )
      {
         PD_LOG( PDERROR, "Create init remote session failed" ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      rc = _sessionInit( pInitSession, pSub->getNodeID(), cb ) ;
      if ( rc )
      {
         goto error ;
      }

   done:
      if ( pInitSession )
      {
         pSite->removeSession( pInitSession ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordRemoteHandlerBase::onSendConnect( _pmdSubSession *pSub,
                                                 const MsgHeader *pReq,
                                                 BOOLEAN isFirst )
   {
      INT32 rc = SDB_OK ;

      if ( INIT_V0 == _initType )
      {
         rc = _onSendConnectOld( pSub ) ;
      }
      else
      {
         rc = _buildPacketWithSessionInit( pSub->parent(), pSub, FALSE ) ;
         if ( SDB_OK == rc )
         {
            _initFinished = FALSE ;
         }
      }
      return rc ;
   }

   INT32 _coordRemoteHandlerBase::_buildPacketWithSessionInit( _pmdRemoteSession *pSession,
                                                               _pmdSubSession *pSub,
                                                               BOOLEAN isUpdate )
   {
      INT32 rc = SDB_OK ;
      BSONObj objInfo ;
      CHAR *pBuff = NULL ;
      MsgComSessionInitReq *pInitReq = NULL ;
      UINT32 msgLength = sizeof( MsgComSessionInitReq ) ;
      pmdEDUCB *cb = pSession->getEDUCB() ;

      /// construct info
      try
      {
         objInfo = _buildSessionInitObj( cb ) ;
         msgLength += objInfo.objsize() ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      /// allocate memory
      rc = cb->allocBuff( msgLength, &pBuff, NULL ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Alloc memory failed, size: %u, rc: %d",
                 msgLength, rc ) ;
         goto error ;
      }
      pInitReq = (MsgComSessionInitReq*)pBuff ;

      /// init message
      pInitReq->header.messageLength = msgLength ;
      pInitReq->header.opCode = MSG_COM_SESSION_INIT_REQ ;
      pInitReq->header.requestID = 0 ;
      pInitReq->header.routeID.value = 0 ;
      pInitReq->header.TID = cb->getTID() ;
      pInitReq->dstRouteID.value = pSub->getNodeIDUInt() ;
      pInitReq->srcRouteID.value = pmdGetNodeID().value ;
      pInitReq->localIP = _netFrame::getLocalAddress() ;
      pInitReq->peerIP = 0 ;
      pInitReq->localPort = pmdGetLocalPort() ;
      pInitReq->peerPort = 0 ;
      pInitReq->localTID = cb->getTID() ;
      pInitReq->localSessionID = cb->getID() ;
      pInitReq->isUpdate = isUpdate ? 1 : 0 ;
      ossMemset( pInitReq->reserved, 0, sizeof( pInitReq->reserved ) ) ;
      ossMemcpy( pInitReq->data, objInfo.objdata(), objInfo.objsize() ) ;

      rc = _buildPacket( pSession, pSub, ( MsgHeader*)pBuff ) ;
      if ( rc )
      {
         goto error ;
      }

   done:
      if ( pBuff )
      {
         cb->releaseBuff( pBuff ) ;
         pBuff = NULL ;
      }
      return rc ;
   error:
      goto done ;
   }

   BSONObj _coordRemoteHandlerBase::_buildSessionInitObj( _pmdEDUCB *cb )
   {
      const CHAR *pRemoteIP = "" ;
      UINT16 remotePort = 0 ;
      UINT32 mask = 0 ;
      UINT32 configMask = 0 ;
      BSONObjBuilder builder( 256 ) ;

      if ( cb->getSession() )
      {
         IClient *pClient = cb->getSession()->getClient() ;
         if ( pClient )
         {
            pRemoteIP = pClient->getFromIPAddr() ;
            remotePort = pClient->getFromPort() ;
         }
      }

      builder.append( SDB_AUTH_USER, cb->getUserName() ) ;
      builder.append( SDB_AUTH_PASSWD, cb->getPassword() ) ;
      builder.append( FIELD_NAME_HOST, pmdGetKRCB()->getHostName() ) ;
      builder.append( PMD_OPTION_SVCNAME, pmdGetOptionCB()->getServiceAddr() ) ;
      builder.append( FIELD_NAME_REMOTE_IP, pRemoteIP ) ;
      builder.append( FIELD_NAME_REMOTE_PORT, (INT32)remotePort ) ;

      pdGetCurAuditMask( AUDIT_LEVEL_USER, mask, configMask ) ;
      builder.append( FIELD_NAME_AUDIT_MASK, (INT32)mask ) ;
      builder.append( FIELD_NAME_AUDIT_CONFIG_MASK, (INT32)configMask ) ;

      return builder.obj() ;
   }

   INT32 _coordRemoteHandlerBase::_sessionInit( _pmdRemoteSession *pSession,
                                                const MsgRouteID &nodeID,
                                                _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      pmdSubSession *pSub = NULL ;
      MsgOpReply *pReply = NULL ;

      BSONObj objInfo ;
      CHAR *pBuff = NULL ;
      MsgComSessionInitReq *pInitReq = NULL ;
      UINT32 msgLength = sizeof( MsgComSessionInitReq ) ;

      /// construct info
      try
      {
         objInfo = _buildSessionInitObj( cb ) ;
         msgLength += objInfo.objsize() ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      /// allocate memory
      rc = cb->allocBuff( msgLength, &pBuff, NULL ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Alloc memory failed, size: %u, rc: %d",
                 msgLength, rc ) ;
         goto error ;
      }
      pInitReq = (MsgComSessionInitReq*)pBuff ;

      /// init message
      pInitReq->header.messageLength = msgLength ;
      pInitReq->header.opCode = MSG_COM_SESSION_INIT_REQ ;
      pInitReq->header.requestID = 0 ;
      pInitReq->header.routeID.value = 0 ;
      pInitReq->header.TID = cb->getTID() ;
      pInitReq->dstRouteID.value = nodeID.value ;
      pInitReq->srcRouteID.value = pmdGetNodeID().value ;
      pInitReq->localIP = _netFrame::getLocalAddress() ;
      pInitReq->peerIP = 0 ;
      pInitReq->localPort = pmdGetLocalPort() ;
      pInitReq->peerPort = 0 ;
      pInitReq->localTID = cb->getTID() ;
      pInitReq->localSessionID = cb->getID() ;
      ossMemset( pInitReq->reserved, 0, sizeof( pInitReq->reserved ) ) ;
      ossMemcpy( pInitReq->data, objInfo.objdata(), objInfo.objsize() ) ;

      /// send message to peer
      pSub = pSession->addSubSession( nodeID.value ) ;
      pSub->setReqMsg( ( MsgHeader* )pInitReq, PMD_EDU_MEM_NONE ) ;

      rc = pSession->sendMsg( pSub ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Send session init request to node[%s] failed, "
                 "rc: %d", routeID2String( nodeID ).c_str(), rc ) ;
         goto error ;
      }

      /// get reply
      rc = pSession->waitReply1( TRUE ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Wait session init response from node[%s] failed, "
                 "rc: %d", routeID2String( nodeID ).c_str(), rc ) ;
         goto error ;
      }

      /// process result
      pReply = ( MsgOpReply* )pSub->getRspMsg( FALSE ) ;
      if ( !pReply )
      {
         PD_LOG( PDERROR, "Session init reply message is null in node[%s]",
                 routeID2String( nodeID ).c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      rc = pReply->flags ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Session init with node[%s] failed, rc: %d",
                 routeID2String( nodeID ).c_str(), rc ) ;
         goto error ;
      }

   done:
      if ( pBuff )
      {
         cb->releaseBuff( pBuff ) ;
         pBuff = NULL ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordRemoteHandlerBase::_checkSessionSchedInfo( _pmdRemoteSession *pSession,
                                                          _pmdSubSession *pSub,
                                                          _pmdEDUCB *cb,
                                                          UINT32 &nodeSiteVer )
   {
      INT32 rc = SDB_OK ;
      coordResource *pResource = pmdGetKRCB()->getCoordCB()->getResource() ;
      schedItem *pItem = NULL ;
      schedInfo *pInfo = NULL ;

      pItem = (schedItem*)cb->getSession()->getSchedItemPtr() ;
      if ( !pItem )
      {
         goto done ;
      }
      pInfo = &(pItem->_info) ;

      if ( pResource->getOmGroupInfo()->nodeCount() > 0 &&
           ( SCHED_UNKNWON_VERSION == pInfo->getVersion() ||
             pInfo->isNew() ) )
      {
         /// update task info
         coordOmStrategyAgent *pAgent = pResource->getOmStrategyAgent() ;
         omTaskStrategyInfoPtr ptr ;

         rc = pAgent->getTaskStrategy( pInfo->getTaskName(),
                                       pInfo->getUserName(),
                                       pInfo->getIP(),
                                       ptr,
                                       FALSE ) ;
         if ( SDB_OK == rc )
         {
            pInfo->fromBSON( ptr->toBSON( OM_STRATEGY_MASK_BASEINFO ), FALSE ) ;
   
            if ( pItem->_ptr.get() &&
                 ( pItem->_ptr->getTaskID() != (UINT64)pInfo->getTaskID() ||
                   0 != ossStrcmp( pItem->_ptr->getTaskName(),
                                   pInfo->getTaskName() ) ) )
            {
               schedTaskMgr *pSvcTaskMgr = pmdGetKRCB()->getSvcTaskMgr() ;
               /// update task info
               pItem->_ptr = pSvcTaskMgr->getTaskInfoPtr(
                                                pItem->_info.getTaskID(),
                                                pItem->_info.getTaskName() ) ;
               /// update monApp's info
               cb->getMonAppCB()->setSvcTaskInfo( pItem->_ptr.get() ) ;
            }
         }
      }

      if ( pInfo->getVersion() != (INT32)nodeSiteVer )
      {
         /// rebuild message
         rc = _buildPacketWithUpdateSched( pSession, pSub, pInfo->toBSON() ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Build packet message with update-sched failed, "
                    "rc: %d", rc ) ;
            goto error ;
         }

         /// update version
         nodeSiteVer = pInfo->getVersion() ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordRemoteHandlerBase::_checkSessionAttr( _pmdRemoteSession *pSession,
                                                     _pmdSubSession *pSub,
                                                     _pmdEDUCB *cb,
                                                     UINT32 &nodeSiteVer )
   {
      INT32 rc = SDB_OK ;
      UINT32 curVersion = pdGetCurAuditVersion() ;

      if ( 0 != curVersion && curVersion != nodeSiteVer )
      {
         /// when net handle is invalid, the info will
         /// stored in session-init message
         if ( NET_INVALID_HANDLE != pSub->getHandle() )
         {
            rc = _buildPacketWithSessionInit( pSession, pSub, TRUE ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Build packet message with session-update "
                       "failed, rc: %d", rc ) ;
               goto error ;
            }
         }

         /// update version
         nodeSiteVer = curVersion ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordRemoteHandlerBase::onSend( _pmdRemoteSession *pSession,
                                          _pmdSubSession *pSub )
   {
      INT32 rc = SDB_OK ;
      pmdEDUCB *cb = pSession->getEDUCB() ;
      pmdRemoteSessionSite* pSite = NULL ;
      UINT32 nodeSiteSchedVer = (UINT32)SCHED_INVALID_VERSION ;
      UINT32 nodeSiteSessionVer = 0 ;
      UINT64 nodeSiteVer = 0 ;
      MsgRouteID nodeID = pSub->getNodeID() ;

      /// is not data node, ignored
      if ( nodeID.columns.groupID < DATA_GROUP_ID_BEGIN ||
           nodeID.columns.groupID > DATA_GROUP_ID_END )
      {
         goto done ;
      }

      pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;
      if ( pSite->getNodeVer( nodeID.value, nodeSiteVer ) )
      {
         ossUnpack32From64( nodeSiteVer, nodeSiteSchedVer, 
                            nodeSiteSessionVer ) ;
      }

      rc = _checkSessionSchedInfo( pSession, pSub, cb, nodeSiteSchedVer ) ;
      if ( rc )
      {
         goto error ;
      }

      rc = _checkSessionAttr( pSession, pSub, cb, nodeSiteSessionVer ) ;
      if ( rc )
      {
         goto error ;
      }

      _nodeVer = ossPack32To64( nodeSiteSchedVer, nodeSiteSessionVer ) ;

      /// has changed
      if ( nodeSiteVer != _nodeVer )
      {
         _nodeID = nodeID.value ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _coordRemoteHandlerBase::setUserData( UINT64 data )
   {
      if ( (INT32)data == INIT_V0 )
      {
         _initType = INIT_V0 ;
      }
      else
      {
         _initType = INIT_V1 ;
      }
   }

   INT32 _coordRemoteHandlerBase::_buildPacket( _pmdRemoteSession *pSession,
                                                _pmdSubSession *pSub,
                                                MsgHeader *pHeader )
   {
      INT32 rc = SDB_OK ;
      UINT32 totalLen = 0 ;
      UINT32 pos = 0 ;
      MsgHeader *pOldHeader = pSub->getReqMsg() ;
      pmdEDUCB *cb = pSession->getEDUCB() ;
      CHAR *pBuff = NULL ;

      if ( pSub->getIODatas()->size() > 0 )
      {
         pOldHeader->messageLength = sizeof( MsgHeader ) +
                                     pSub->getIODataLen() ;
      }

      totalLen = pHeader->messageLength + pOldHeader->messageLength ;

      if ( MSG_PACKET != pOldHeader->opCode )
      {
         totalLen += sizeof( MsgHeader ) ;
      }

      rc = cb->allocBuff( totalLen, &pBuff, NULL ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Allocate memory[%u] failed, rc: %d",
                 totalLen, rc ) ;
         goto error ;
      }
      else
      {
         MsgHeader *pMsgPacket = NULL ;
         /// packet
         pMsgPacket = ( MsgHeader* )( pBuff + pos ) ;
         pos += sizeof( MsgHeader ) ;

         /// new add
         ossMemcpy( pBuff + pos, (void*)pHeader, pHeader->messageLength ) ;
         MsgHeader *pNewAdd = ( MsgHeader* )( pBuff + pos ) ;
         pNewAdd->requestID = pOldHeader->requestID ;
         pNewAdd->routeID.value = pOldHeader->routeID.value ;
         pNewAdd->TID = pOldHeader->TID ;
         pos += pHeader->messageLength ;

         if ( MSG_PACKET == pOldHeader->opCode )
         {
            ossMemcpy( (void*)pMsgPacket, (void*)pOldHeader,
                       sizeof( MsgHeader ) ) ;
         }
         else
         {
            pMsgPacket->opCode = MSG_PACKET ;
            pMsgPacket->requestID = pOldHeader->requestID ;
            pMsgPacket->routeID.value = pOldHeader->routeID.value ;
            pMsgPacket->TID = pOldHeader->TID ;
         }
         pMsgPacket->messageLength = totalLen ;

         /// old
         if ( pSub->getIODatas()->size() > 0 )
         {
            if ( MSG_PACKET != pOldHeader->opCode )
            {
               ossMemcpy( pBuff + pos, (void*)pOldHeader,
                          sizeof( MsgHeader ) ) ;
               pos += sizeof( MsgHeader ) ;
            }

            netIOVec *pIOVec = pSub->getIODatas() ;
            for ( UINT32 i = 0 ; i < pIOVec->size() ; ++i )
            {
               netIOV &ioItem = (*pIOVec)[i] ;
               ossMemcpy( pBuff + pos, ioItem.iovBase, ioItem.iovLen ) ;
               pos += ioItem.iovLen ;
            }
         }
         else
         {
            CHAR *pCopyData = ( CHAR* )pOldHeader ;
            UINT32 copyLen = pOldHeader->messageLength ;

            if ( MSG_PACKET == pOldHeader->opCode )
            {
               pCopyData += sizeof( MsgHeader ) ;
               copyLen -= sizeof( MsgHeader ) ;
            }
            ossMemcpy( pBuff + pos, pCopyData,copyLen ) ;
            pos += copyLen ;
         }

         /// set sub session
         pSub->clearIODatas() ;
         pSub->setReqMsg( (MsgHeader*)pBuff, PMD_EDU_MEM_SELF ) ;
         pBuff = NULL ;
      }

   done:
      if ( pBuff )
      {
         cb->releaseBuff( pBuff ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordRemoteHandlerBase::_buildPacketWithUpdateSched( _pmdRemoteSession *pSession,
                                                               _pmdSubSession *pSub,
                                                               const BSONObj &objSched )
   {
      INT32 rc = SDB_OK ;
      pmdEDUCB *cb = pSession->getEDUCB() ;
      CHAR *pBuff = NULL ;
      INT32 buffSize = 0 ;

      rc = msgBuildUpdateMsg( &pBuff, &buffSize,
                              CMD_ADMIN_PREFIX SYS_CL_SESSION_INFO,
                              0, 0, NULL,  &objSched, NULL, cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Build update message failed, rc: %d", rc ) ;
         goto error ;
      }

      rc = _buildPacket( pSession, pSub, ( MsgHeader* )pBuff ) ;
      if ( rc )
      {
         goto error ;
      }

   done:
      if ( pBuff )
      {
         msgReleaseBuffer( pBuff, cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordNoSessionInitHandler implement
   */
   _coordNoSessionInitHandler::_coordNoSessionInitHandler()
   {
   }

   _coordNoSessionInitHandler::~_coordNoSessionInitHandler()
   {
   }

   INT32 _coordNoSessionInitHandler::onSendConnect( _pmdSubSession *pSub,
                                                    const MsgHeader *pReq,
                                                    BOOLEAN isFirst )
   {
      /// already disconnect
      return SDB_COORD_REMOTE_DISC ;
   }

   /*
      _coordRemoteHandler implement
   */
   _coordRemoteHandler::_coordRemoteHandler()
   {
      _interruptWhenFailed = FALSE ;
   }

   _coordRemoteHandler::~_coordRemoteHandler()
   {
   }

   void _coordRemoteHandler::enableInterruptWhenFailed( BOOLEAN enable,
                                                        const SET_RC *pIgnoreRC )
   {
      _interruptWhenFailed = enable ;
      if ( pIgnoreRC )
      {
         _ignoreRC = *pIgnoreRC ;
      }
      else
      {
         _ignoreRC.clear() ;
      }
   }

   void _coordRemoteHandler::onReply( _pmdRemoteSession *pSession,
                                      _pmdSubSession **ppSub,
                                      const MsgHeader *pReply,
                                      BOOLEAN isPending )
   {
      _coordRemoteHandlerBase::onReply( pSession, ppSub, pReply, isPending ) ;

      if ( _interruptWhenFailed )
      {
         MsgOpReply *pOpReply = ( MsgOpReply* )pReply ;
         /// When not ok and not in ignored rc set
         if ( SDB_OK != pOpReply->flags &&
              _ignoreRC.find( pOpReply->flags ) == _ignoreRC.end() )
         {
            PD_LOG( PDWARNING, "Session[%s]: Sub-session[%s] recieved "
                    "invalid reply with flag[%d], stop other sub-sessions",
                    pSession->getEDUCB()->toString().c_str(),
                    routeID2String( pReply->routeID ).c_str(),
                    pOpReply->flags ) ;
            pSession->stopSubSession() ;
         }
      }
   }

   INT32 _coordRemoteHandler::onExpiredReply ( pmdRemoteSessionSite *pSite,
                                               const MsgHeader *pReply )
   {
      INT32 rc = SDB_OK ;
      MsgOpReply *pOpReply = NULL ;
      pmdRemoteSession *pSession = NULL ;
      MsgOpKillContexts msgKillContext ;

      if ( NULL == pReply || !IS_REPLY_TYPE( pReply->opCode ) )
      {
         goto done ;
      }

      pOpReply = (MsgOpReply *)pReply ;
      if ( -1 == pOpReply->contextID )
      {
         goto done ;
      }

      PD_LOG( PDWARNING, "Received expired context[%lld] from node[%s]",
              pOpReply->contextID,
              routeID2String( pReply->routeID ).c_str() ) ;

      pSession = pSite->addSession( COORD_EXPIRED_KILLCONTEXT_TIMEOUT ) ;
      pSession->addSubSession( pReply->routeID.value ) ;

      /// send kill context
      msgKillContext.contextIDs[ 0 ] = pOpReply->contextID ;
      msgKillContext.numContexts = 1 ;
      msgKillContext.ZERO = 0 ;
      msgKillContext.header.messageLength = sizeof( MsgOpKillContexts ) ;
      msgKillContext.header.opCode = MSG_BS_KILL_CONTEXT_REQ ;
      msgKillContext.header.requestID = 0 ;
      msgKillContext.header.routeID.value = 0 ;
      msgKillContext.header.TID = 0 ;

      /// Ignore sendMsg failed and waitReply failed
      rc = pSession->sendMsg( (MsgHeader*)&msgKillContext,
                              PMD_EDU_MEM_NONE ) ;
      if ( SDB_OK == rc )
      {
         pSession->waitReply1() ;
      }

   done:
      if ( pSession )
      {
         pSite->removeSession( pSession ) ;
      }
      return rc ;
   }

}

