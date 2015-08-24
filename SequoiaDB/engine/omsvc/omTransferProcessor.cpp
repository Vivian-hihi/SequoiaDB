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

   Source File Name = omTransferProcessor.cpp

   Descriptive Name = om transfer message Processor source file

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/09/2015  Lin YouBin  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdStartup.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "omManager.hpp"
#include "msgMessage.hpp"
#include "omDef.hpp"
#include "omContextTransfer.hpp"
#include "omTransferProcessor.hpp"

namespace engine
{

   _omTransferProcessor::_omTransferProcessor( list< _omNodeInfo > &nodeList )
   {
      _nodeList = nodeList ;
   }

   _omTransferProcessor::~_omTransferProcessor()
   {
   }

   void _omTransferProcessor::_clearRemoteSession( 
                                             pmdRemoteSessionMgr *rsManager,
                                             pmdRemoteSession *remoteSession )
   {
      if ( NULL != remoteSession && NULL != rsManager )
      {
         remoteSession->clearSubSession() ;
         rsManager->removeSession( remoteSession ) ;
      }
   }

   INT32 _omTransferProcessor::_sendMsg2Target( pmdRemoteSessionMgr *rsManager,
                                                MsgRouteID &id, MsgHeader *msg, 
                                                MsgHeader **result )
   {
      INT32 rc = SDB_OK ;
      pmdRemoteSession *remoteSession = NULL ;
      MsgHeader *reply = NULL ;
      VEC_SUB_SESSIONPTR subSessionVec ;

      remoteSession = rsManager->addSession( pmdGetThreadEDUCB(), 
                                             OM_WAIT_SCAN_RES_INTERVAL, NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "create remote session failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( NULL == remoteSession->addSubSession( id.value ) )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSubSession failed:id=%ld", id.value ) ;
         goto error ;
      }

      rc = remoteSession->sendMsg( msg ) ;
      if ( SDB_OK != rc )
      {  
         PD_LOG( PDERROR, "send msg to target failed:id=%ld,rc=%d", 
                 id.value, rc ) ;
         goto error ;
      }

      rc = remoteSession->waitReply( TRUE, &subSessionVec ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "wait reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( 1 != subSessionVec.size() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "unexpected session size:size=%d", 
                 subSessionVec.size() ) ;
         goto error ;
      }

      if ( subSessionVec[0]->isDisconnect() )
      {
         rc = SDB_NETWORK_CLOSE ;
         PD_LOG(PDERROR, "session disconnected:id=%s,rc=%d", 
                routeID2String( subSessionVec[0]->getNodeID()).c_str(), rc ) ;
         goto error ;
      }

      reply = subSessionVec[0]->getRspMsg() ;
      if ( NULL == reply )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive null response:rc=%d", rc ) ;
         goto error ;
      }

      *result = reply ;

   done:
      _clearRemoteSession( rsManager, remoteSession ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _omTransferProcessor::processMsg( MsgHeader *msg,
                                           rtnContextBuf &contextBuff,
                                           INT64 &contextID,
                                           BOOLEAN &needReply )
   {
      pmdKRCB *pKrcb    = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();
      INT32 rc          = SDB_OK ;
      MsgHeader *result = NULL ;
      rtnContext *pContext = NULL ;
      omManager *om = sdbGetOMManager() ;
      MsgRouteID routeID ;
      _omContextTransfer *pTmpContext = NULL ;
      list< omNodeInfo >::iterator iter = _nodeList.begin() ;

      if ( _nodeList.size() <= 0 )
      {
         SDB_ASSERT( _nodeList.size() > 0, "nodelist can't be 0" ) ;
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "nodeList is 0" ) ;
         goto error ;
      }

      while ( iter != _nodeList.end() )
      {
         contextID = -1 ;
         routeID = om->updateAgentInfo( iter->hostName, iter->service ) ;
         rc = _sendMsg2Target( om->getRSManager(), routeID, msg, &result ) ;
         if ( SDB_OK == rc )
         {
            break ;
         }

         PD_LOG( PDERROR, "send message to target failed:host=%s,port=%s",
                 iter->hostName.c_str(), iter->service.c_str() ) ;
         iter++ ;
      }

      if ( SDB_OK != rc )
      {
         //this sugguest all node is failure.
         PD_LOG( PDERROR, "all nodes is failure." ) ;
         goto error ;
      }

      // create context
      rc = pRtncb->contextNew( RTN_CONTEXT_OM_TRANSFER,
                               &pContext, contextID, eduCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to allocate context(rc=%d)",
                   rc ) ;

      pTmpContext = ( _omContextTransfer *)pContext ;
      rc = pTmpContext->open( om->getRSManager(), routeID, result ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "open context failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      if ( -1 != contextID )
      {
         pRtncb->contextDelete( contextID, eduCB() ) ;
         contextID = -1 ;
      }
      goto done ;
   }

   SDB_PROCESSOR_TYPE _omTransferProcessor::processorType() const
   {
      return SDB_PROCESSOR_OM ;
   }

   const CHAR* _omTransferProcessor::processorName() const
   {
      return "transferProcessor" ;
   }

   void _omTransferProcessor::attach( pmdSession *session )
   {
      attachSession( session ) ;
   }

   void _omTransferProcessor::detach()
   {
      detachSession() ;
   }

   void _omTransferProcessor::_onAttach()
   {
   }

   void _omTransferProcessor::_onDetach()
   {
   }
}

