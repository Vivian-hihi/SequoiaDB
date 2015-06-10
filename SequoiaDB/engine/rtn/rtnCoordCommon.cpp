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

   Source File Name = rtnCoordCommon.cpp

   Descriptive Name = Runtime Coord Common

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "rtnCoord.hpp"
#include "rtnCoordCommon.hpp"
#include "msg.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "rtnCoordQuery.hpp"
#include "msgMessage.hpp"
#include "coordCB.hpp"
#include "rtnContext.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "coordSession.hpp"
#include "rtnCoordCommands.hpp"
#include "rtn.hpp"

using namespace bson;

namespace engine
{
   /*
      Local function define
   */

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCOSENDREQUESTTOONE, "_rtnCoordSendRequestToOne" )
   static INT32 _rtnCoordSendRequestToOne( MsgHeader *pBuffer,
                                           CoordGroupInfoPtr &groupInfo,
                                           REQUESTID_MAP &sendNodes, // out
                                           netMultiRouteAgent *pRouteAgent,
                                           MSG_ROUTE_SERVICE_TYPE type,
                                           pmdEDUCB *cb,
                                           const netIOVec *pIOVec,
                                           BOOLEAN isResend )
   {
      INT32 rc                = SDB_OK ;
      CoordSession *pSession  = cb->getCoordSession() ;
      MsgRouteID routeID ;
      clsGroupItem *groupItem = NULL ;
      UINT32 nodeNum          = 0 ;
      UINT32 beginPos         = 0 ;
      UINT32 selTimes         = 0 ;

      BOOLEAN hasRetry        = FALSE ;
      UINT64 reqID            = 0 ;
      INT32 preferReplicaType = PREFER_REPL_ANYONE ;

      PD_TRACE_ENTRY ( SDB__RTNCOSENDREQUESTTOONE ) ;

      if ( pSession )
      {
         preferReplicaType = pSession->getPreferReplType() ;
         if ( PREFER_REPL_MASTER == preferReplicaType )
         {
            if ( MSG_BS_QUERY_REQ == pBuffer->opCode )
            {
               MsgOpQuery *pQuery = ( MsgOpQuery* )pBuffer ;
               if ( FALSE == isResend )
               {
                  pQuery->flags |= FLG_QUERY_PRIMARY ;
               }
               else
               {
                  pQuery->flags &= ~FLG_QUERY_PRIMARY ;
               }
            }
            else if ( pBuffer->opCode > ( SINT32 )MSG_LOB_BEGIN &&
                      pBuffer->opCode < ( SINT32 )MSG_LOB_END )
            {
               MsgOpLob *pLobMsg = ( MsgOpLob* )pBuffer ;
               if ( FALSE == isResend )
               {
                  pLobMsg->flags |= FLG_LOBREAD_PRIMARY ;
               }
               else
               {
                  pLobMsg->flags &= ~FLG_LOBREAD_PRIMARY ;
               }
            }
         }
      }

      /*******************************
      // send to last node
      ********************************/
      if ( NULL != pSession )
      {
         routeID = pSession->getLastNode( groupInfo->getGroupID() ) ;
         // last node is valid and in group info( when group or node 
         // is remove )
         if ( routeID.value != 0 &&
              groupInfo->getGroupItem()->nodePos( routeID.columns.nodeID ) >= 0 )
         {
            if ( pIOVec && pIOVec->size() > 0 )
            {
               rc = pRouteAgent->syncSend( routeID, pBuffer, *pIOVec,
                                           reqID, cb ) ;
            }
            else
            {
               rc = pRouteAgent->syncSend( routeID, (void *)pBuffer,
                                           reqID, cb, NULL, 0 ) ;
            }
            if ( SDB_OK == rc )
            {
               sendNodes[reqID] = routeID ;
               goto done ;
            }
            else
            {
               rtnCoordUpdateNodeStatByRC( cb, routeID, groupInfo, rc ) ;

               PD_LOG( PDWARNING, "Send msg[opCode: %d, TID: %u] to group[%u] 's "
                       "last node[%s] failed, rc: %d", pBuffer->opCode,
                       pBuffer->TID, groupInfo->getGroupID(),
                       routeID2String( routeID ).c_str(), rc ) ;
            }
         }
      }

   retry:
      /*******************************
      // send to new node
      ********************************/
      groupItem = groupInfo->getGroupItem() ;
      nodeNum = groupInfo->getGroupSize() ;
      if ( nodeNum <= 0 )
      {
         if ( !hasRetry && CATALOG_GROUPID != groupInfo->getGroupID() )
         {
            hasRetry = TRUE ;
            rc = rtnCoordGetGroupInfo( cb, groupInfo->getGroupID(),
                                       TRUE, groupInfo ) ; 
            PD_RC_CHECK( rc, PDERROR, "Get group info[%u] failed, rc: %d",
                         groupInfo->getGroupID(), rc ) ;
            goto retry ;
         }
         rc = SDB_CLS_EMPTY_GROUP ;
         PD_LOG ( PDERROR, "Couldn't send the request to empty group" ) ;
         goto error ;
      }

      routeID.value = MSG_INVALID_ROUTEID ;
      rtnCoordGetNodePos( preferReplicaType, groupItem, ossRand(), beginPos ) ;

      selTimes = 0 ;
      while( selTimes < nodeNum )
      {
         INT32 status = NET_NODE_STAT_NORMAL ;
         rtnCoordGetNextNode( preferReplicaType, groupItem,
                              selTimes, beginPos ) ;

         rc = groupItem->getNodeID( beginPos, routeID, type ) ;
         if ( rc )
         {
            break ;
         }
         rc = groupInfo->getNodeInfo( beginPos, status ) ;
         if ( rc )
         {
            break ;
         }
         if ( NET_NODE_STAT_NORMAL == status )
         {
            if ( pIOVec )
            {
               rc = pRouteAgent->syncSend( routeID, pBuffer, *pIOVec,
                                           reqID, cb ) ;
            }
            else
            {
               rc = pRouteAgent->syncSend( routeID, (void *)pBuffer,
                                           reqID, cb, NULL, 0 ) ;
            }
            if ( SDB_OK == rc )
            {
               sendNodes[ reqID ] = routeID ;

               if ( pSession )
               {
                  pSession->addLastNode( routeID ) ;
               }
               break ;
            }
            else
            {
               rtnCoordUpdateNodeStatByRC( cb, routeID, groupInfo, rc ) ;
            }
         }
         else
         {
            rc = SDB_CLS_NODE_BSFAULT ;
         }
      }

      if ( rc && !hasRetry )
      {
         hasRetry = TRUE ;
         if ( CATALOG_GROUPID != groupInfo->getGroupID() )
         {
            rc = rtnCoordGetGroupInfo( cb, groupInfo->getGroupID(),
                                       TRUE, groupInfo ) ;
            PD_RC_CHECK( rc, PDERROR, "Get group info[%u] failed, rc: %d",
                         groupInfo->getGroupID(), rc ) ;
         }
         else
         {
            groupInfo->clearNodesStat() ;
            rc = SDB_OK ;
         }
         goto retry ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__RTNCOSENDREQUESTTOONE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCOSENDREQUESTTOPRIMARY, "_rtnCoordSendRequestToPrimary" )
   static INT32 _rtnCoordSendRequestToPrimary( MsgHeader *pBuffer,
                                               CoordGroupInfoPtr &groupInfo,
                                               netMultiRouteAgent *pRouteAgent,
                                               MSG_ROUTE_SERVICE_TYPE type,
                                               pmdEDUCB *cb,
                                               REQUESTID_MAP &sendNodes, // out
                                               const netIOVec *pIOVec )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__RTNCOSENDREQUESTTOPRIMARY ) ;

      MsgRouteID primaryRouteID ;
      UINT64 reqID = 0 ;
      BOOLEAN hasRetry = FALSE ;

   retry:
      primaryRouteID = groupInfo->getPrimary( type ) ;
      if ( primaryRouteID.value != 0 )
      {
         if ( pIOVec && pIOVec->size() > 0 )
         {
            rc = pRouteAgent->syncSend( primaryRouteID, pBuffer, *pIOVec,
                                        reqID, cb ) ;
         }
         else
         {
            rc = pRouteAgent->syncSend( primaryRouteID, (void *)pBuffer,
                                        reqID, cb, NULL, 0 ) ;
         }
         if ( SDB_OK == rc )
         {
            sendNodes[ reqID ] = primaryRouteID ;
            goto done ;
         }
         else
         {
            rtnCoordUpdateNodeStatByRC( cb, primaryRouteID, groupInfo, rc ) ;

            // update and retry
            if ( !hasRetry )
            {
               goto update ;
            }

            PD_LOG( PDWARNING, "Send msg[opCode: %d, TID: %u] to group[%u] 's "
                    "primary node[%s] failed, rc: %d", pBuffer->opCode,
                    pBuffer->TID, groupInfo->getGroupID(),
                    routeID2String( primaryRouteID ).c_str(), rc ) ;
            // not go to error, send to any one node
         }
      }

      // send to any one node
      rc = SDB_RTN_NO_PRIMARY_FOUND ;
      if ( !hasRetry )
      {
         goto update ;
      }
      // send to any one node, so the group has no primary, will wait some
      // time at data node
      rc = _rtnCoordSendRequestToOne( pBuffer, groupInfo, sendNodes,
                                      pRouteAgent, type, cb,
                                      pIOVec, TRUE ) ;
      goto done ;

   update:
      if ( rc && !hasRetry )
      {
         hasRetry = TRUE ;
         rc = rtnCoordGetGroupInfo( cb, groupInfo->getGroupID(),
                                    TRUE, groupInfo ) ;
         PD_RC_CHECK( rc, PDERROR, "Get group info[%u] failed, rc: %d",
                      groupInfo->getGroupID(), rc ) ;
         goto retry ;
      }
   done:
      PD_TRACE_EXITRC ( SDB__RTNCOSENDREQUESTTOPRIMARY, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCOSENDREQUESTTONODEGROUP, "_rtnCoordSendRequestToNodeGroup" )
   static INT32 _rtnCoordSendRequestToNodeGroup( MsgHeader *pBuffer,
                                                 CoordGroupInfoPtr &groupInfo,
                                                 BOOLEAN isSendPrimary,
                                                 netMultiRouteAgent *pRouteAgent,
                                                 MSG_ROUTE_SERVICE_TYPE type,
                                                 pmdEDUCB *cb,
                                                 REQUESTID_MAP &sendNodes, // out
                                                 const netIOVec *pIOVec,
                                                 BOOLEAN isResend )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__RTNCOSENDREQUESTTONODEGROUP ) ;

      MsgRouteID routeID ;
      UINT64 reqID = 0 ;
      UINT32 groupID = groupInfo->getGroupID() ;

      // if trans valid, send to trans node
      cb->getTransNodeRouteID( groupID, routeID ) ;
      if ( routeID.value != 0 )
      {
         if ( pIOVec && pIOVec->size() > 0 )
         {
            rc = pRouteAgent->syncSend( routeID, pBuffer, *pIOVec,
                                        reqID, cb ) ;
         }
         else
         {
            rc = pRouteAgent->syncSend( routeID, (void *)pBuffer,
                                        reqID, cb, NULL, 0 ) ;
         }
         if ( SDB_OK == rc )
         {
            sendNodes[ reqID ] = routeID ;
         }
         else
         {
            PD_LOG( PDERROR, "Send msg[opCode: %d, TID: %u] to group[%u] 's "
                    "transaction node[%s] failed, rc: %d", pBuffer->opCode,
                    pBuffer->TID, groupID, routeID2String( routeID ).c_str(),
                    rc ) ;
            rtnCoordUpdateNodeStatByRC( cb, routeID, groupInfo, rc ) ;
            goto error ;
         }
      }
      // not trans, send to node by isSendPrimary
      else
      {
         if ( isSendPrimary )
         {
            rc = _rtnCoordSendRequestToPrimary( pBuffer, groupInfo,
                                                pRouteAgent, type, cb,
                                                sendNodes, pIOVec ) ;
         }
         else
         {
            rc = _rtnCoordSendRequestToOne( pBuffer, groupInfo, sendNodes,
                                            pRouteAgent, type, cb,
                                            pIOVec, isResend ) ;
         }
         PD_RC_CHECK( rc, PDERROR, "Send msg[opCode: %d, TID: %u] to "
                      "group[%u]'s %s node failed, rc: %d",
                      pBuffer->opCode, pBuffer->TID, groupID,
                      isSendPrimary ? "primary" : "one", rc ) ; 
      }

   done:
      PD_TRACE_EXITRC ( SDB__RTNCOSENDREQUESTTONODEGROUP, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      End local function define
   */

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOCATAQUERY, "rtnCoordCataQuery" )
   INT32 rtnCoordCataQuery ( const CHAR *pCollectionName,
                             const BSONObj &selector,
                             const BSONObj &matcher,
                             const BSONObj &orderBy,
                             const BSONObj &hint,
                             INT32 flag,
                             pmdEDUCB *cb,
                             SINT64 numToSkip,
                             SINT64 numToReturn,
                             SINT64 &contextID )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOCATAQUERY ) ;
      pmdKRCB *pKrcb    = pmdGetKRCB();
      CoordCB *pCoordcb = pKrcb->getCoordCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();
      netMultiRouteAgent *pRouteAgent = pCoordcb->getRouteAgent();
      CoordGroupInfoPtr cataGroupInfo;
      CHAR *pBuf = NULL;
      MsgOpQuery *pQueryMsg = NULL;
      INT32 bufferSize = 0;
      rtnContextCoord *pContext = NULL ;
      CoordGroupList groupLst;
      REPLY_QUE replyQue;
      REQUESTID_MAP sendNodes;
      BOOLEAN hasRetry = FALSE;
      MsgOpReply *pReply = NULL ;
      MsgRouteID nodeID ;
      BOOLEAN takeOver = FALSE ;

      contextID = -1 ;

      rc = msgBuildQueryMsg( &pBuf, &bufferSize, pCollectionName, flag, 0,
                             numToSkip, numToReturn, &matcher, &selector,
                             &orderBy, &hint ) ;
      PD_RC_CHECK( rc, PDERROR, "failed to build the query-msg(rc=%d)", rc );

      pQueryMsg = (MsgOpQuery *)pBuf;
      pQueryMsg->header.TID = cb->getTID();

      rc = rtnCoordGetCatGroupInfo( cb, FALSE, cataGroupInfo ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get catalog group info(rc=%d)",
                   rc );

      rc = pRtncb->contextNew( RTN_CONTEXT_COORD, (rtnContext**)&pContext,
                               contextID, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "failed to allocate context(rc=%d)", rc );
      rc = pContext->open( BSONObj(), BSONObj(), -1, 0 ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context failed, rc: %d", rc ) ;

   retry:
      rc = rtnCoordSendRequestToPrimary( pBuf, cataGroupInfo, sendNodes,
                                         pRouteAgent, MSG_ROUTE_CAT_SERVICE,
                                         cb ) ;
      PD_RC_CHECK( rc, PDERROR, "failed to send the message to catalog "
                   "node(rc=%d)", rc ) ;

      rc = rtnCoordGetReply( cb, sendNodes, replyQue, MSG_BS_QUERY_RES ) ;
      PD_RC_CHECK( rc, PDERROR, "failed to get the reply(rc=%d)", rc );

      SDB_ASSERT( replyQue.size() == 1, "The replyQue size must be 1" );
      while ( !replyQue.empty() )
      {
         if ( pReply )
         {
            SDB_OSS_FREE( pReply ) ;
         }
         pReply = (MsgOpReply *)(replyQue.front()) ;
         replyQue.pop() ;
         rc = pReply->flags ;
         nodeID.value = pReply->header.routeID.value ;
      }
      if ( rc )
      {
         if ( SDB_DMS_EOC == rc )
         {
            goto done;
         }
         else if ( rtnCoordGroupReplyCheck( cb, rc, !hasRetry, nodeID,
                                            cataGroupInfo, NULL, TRUE,
                                            pReply->startFrom ) )
         {
            hasRetry = TRUE ;
            goto retry ;
         }
         PD_LOG ( PDERROR, "failed to query on catalog(rc=%d)", rc ) ;
         goto error ;
      }

      rc = pContext->addSubContext( pReply, takeOver );
      PD_RC_CHECK( rc, PDERROR, "failed to add sub-context(rc=%d)", rc ) ;
      pContext->addSubDone( cb ) ;

   done:
      if ( pBuf )
      {
         SDB_OSS_FREE( pBuf );
      }
      if ( pReply && !takeOver )
      {
         SDB_OSS_FREE( pReply ) ;
      }
      PD_TRACE_EXITRC ( SDB_RTNCOCATAQUERY, rc ) ;
      return rc ;
   error:
      rtnCoordClearRequest( cb, sendNodes );
      if ( contextID >= 0 )
      {
         pRtncb->contextDelete( contextID, cb );
         contextID = -1 ;
      }
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCONODEQUERY, "rtnCoordNodeQuery" )
   INT32 rtnCoordNodeQuery ( const CHAR *pCollectionName,
                             const BSONObj &condition,
                             const BSONObj &selector,
                             const BSONObj &orderBy,
                             const BSONObj &hint,
                             INT64 numToSkip, INT64 numToReturn,
                             CoordGroupList &groupLst,
                             pmdEDUCB *cb,
                             rtnContext **ppContext,
                             const CHAR *realCLName,
                             INT32 flag )
   {
      INT32 rc                        = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNCONODEQUERY ) ;
      CHAR *pBuffer                   = NULL ;
      INT32 bufferSize                = 0 ;

      pmdKRCB *pKrcb                  = pmdGetKRCB() ;
      SDB_RTNCB *pRtnCB               = pKrcb->getRTNCB() ;
      CoordCB *pCoordcb               = pKrcb->getCoordCB() ;
      netMultiRouteAgent *pRouteAgent = pCoordcb->getRouteAgent();

      SDB_ASSERT ( ppContext, "ppContext can't be NULL" ) ;
      SDB_ASSERT ( cb, "cb can't be NULL" ) ;

      rtnCoordQuery newQuery ;
      rtnQueryConf queryConf ;
      rtnSendOptions sendOpt ;

      BOOLEAN needReset = FALSE ;
      BOOLEAN onlyOneNode = groupLst.size() == 1 ? TRUE : FALSE ;
      INT64 tmpSkip = numToSkip ;
      INT64 tmpReturn = numToReturn ;

      if ( !onlyOneNode )
      {
         if ( numToReturn > 0 && numToSkip > 0 )
         {
            tmpReturn = numToReturn + numToSkip ;
         }
         tmpSkip = 0 ;

         rtnNeedResetSelector( selector, orderBy, needReset ) ;
      }
      else
      {
         numToSkip = 0 ;
      }

      const CHAR *realCLFullName = realCLName ? realCLName : pCollectionName ;
      rtnContextCoord *pTmpContext = NULL ;
      INT64 contextID = -1 ;

      if ( groupLst.size() == 0 )
      {
         PD_LOG( PDERROR, "Send group list is empty" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = pRtnCB->contextNew ( RTN_CONTEXT_COORD, (rtnContext**)&pTmpContext,
                                contextID, cb ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to allocate new context, rc = %d",
                  rc ) ;
         goto error ;
      }
      rc = pTmpContext->open( orderBy,
                              needReset ? selector : BSONObj(),
                              numToReturn, numToSkip ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Open context failed, rc: %d", rc ) ;
         goto error ;
      }

      // build a query request based on the provided information
      rc = msgBuildQueryMsg ( &pBuffer, &bufferSize, pCollectionName,
                              flag, 0, tmpSkip, tmpReturn,
                              condition.isEmpty()?NULL:&condition,
                              selector.isEmpty()?NULL:&selector,
                              orderBy.isEmpty()?NULL:&orderBy,
                              hint.isEmpty()?NULL:&hint ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build query message, rc = %d",
                  rc ) ;
         goto error ;
      }

      queryConf._realCLName = realCLFullName ;
      sendOpt._groupLst = groupLst ;
      sendOpt._useSpecialGrp = TRUE ;

      rc = newQuery.queryOrDoOnCL( (MsgHeader*)pBuffer, pRouteAgent, cb,
                                   &pTmpContext, sendOpt, &queryConf ) ;
      PD_RC_CHECK( rc, PDERROR, "Query collection[%s] from data node "
                   "failed, rc = %d", realCLFullName, rc ) ;

   done :
      if ( pBuffer )
      {
         SDB_OSS_FREE ( pBuffer ) ;
      }
      if ( pTmpContext )
      {
         *ppContext = pTmpContext ;
      }
      PD_TRACE_EXITRC ( SDB_RTNCONODEQUERY, rc ) ;
      return rc ;
   error :
      if ( contextID >= 0 )
      {
         pRtnCB->contextDelete ( contextID, cb ) ;
         pTmpContext = NULL ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETREPLY, "rtnCoordGetReply" )
   INT32 rtnCoordGetReply ( pmdEDUCB *cb,  REQUESTID_MAP &requestIdMap,
                            REPLY_QUE &replyQue, const SINT32 opCode,
                            BOOLEAN isWaitAll, BOOLEAN clearReplyIfFailed )
   {
      INT32 rc = SDB_OK;
      ossQueue<pmdEDUEvent> tmpQue;
      PD_TRACE_ENTRY ( SDB_RTNCOGETREPLY ) ;
      REQUESTID_MAP::iterator iterMap;
      INT64 waitTime = RTN_COORD_RSP_WAIT_TIME;
      while ( requestIdMap.size() > 0 )
      {
         pmdEDUEvent pmdEvent;
         BOOLEAN isGotMsg = cb->waitEvent( pmdEvent, waitTime ) ;
         // if we hit interrupt, let's just get out of here. Don't need to worry
         // about cb queue, pmdEDUCB::clear() is going to clean it up.
         PD_CHECK( !cb->isInterrupted() && !cb->isForced(),
                   SDB_APP_INTERRUPT, error, PDERROR,
                   "Interrupt! stop receiving reply!" ) ;

         // if we didn't receive anything
         if ( FALSE == isGotMsg )
         {
            if ( !isWaitAll && !replyQue.empty() )
            {
               break;
            }
            else
            {
               continue;
            }
         }

         if ( pmdEvent._eventType != PMD_EDU_EVENT_MSG )
         {
            PD_LOG ( PDWARNING, "received unknown event(eventType:%d)",
                     pmdEvent._eventType ) ;
            pmdEduEventRelase( pmdEvent, cb ) ;
            pmdEvent.reset () ;
            continue;
         }
         MsgHeader *pMsg = (MsgHeader *)(pmdEvent._Data);
         if ( NULL == pMsg )
         {
            PD_LOG ( PDWARNING, "received invalid event(data is null)" );
            continue;
         }

         if ( MSG_COOR_REMOTE_DISC == pMsg->opCode )
         {
            // check if transaction-node
            MsgRouteID routeID;
            routeID.value = pMsg->routeID.value;
            if ( cb->isTransNode( routeID ))
            {
               cb->setTransRC( SDB_COORD_REMOTE_DISC );
               PD_LOG ( PDERROR,
                        "transaction operation interrupt, "
                        "remote-node disconnected:"
                        "groupID=%u, nodeID=%u, serviceID=%u",
                        routeID.columns.groupID,
                        routeID.columns.nodeID,
                        routeID.columns.serviceID );
            }

            iterMap = requestIdMap.begin();
            while ( iterMap != requestIdMap.end() )
            {
               if ( iterMap->second.value == routeID.value )
               {
                  break;
               }
               ++iterMap;
            }
            if ( iterMap != requestIdMap.end() &&
                 iterMap->first <= pMsg->requestID )
            {
               // remote node disconnected,
               // go on receive all reply then clear all

               // don't return the rc,
               // the error will be detected while process the reply
               // rc = rc ? rc : SDB_COORD_REMOTE_DISC;
               PD_LOG ( PDERROR,
                        "get reply failed, remote-node disconnected:"
                        "groupID=%u, nodeID=%u, serviceID=%u",
                        iterMap->second.columns.groupID,
                        iterMap->second.columns.nodeID,
                        iterMap->second.columns.serviceID ) ;
               MsgOpReply *pDiscSrc = ( MsgOpReply *)(pmdEvent._Data);
               MsgOpReply *pDiscMsg = NULL;
               pDiscMsg = (MsgOpReply *)SDB_OSS_MALLOC(
                  pDiscSrc->header.messageLength ) ;
               if ( NULL == pDiscMsg )
               {
                  rc = rc ? rc : SDB_OOM ;
                  PD_LOG( PDERROR, "malloc failed(size=%d)",
                          sizeof(MsgOpReply) ) ;
                  goto error ;
               }
               else
               {
                  ossMemcpy( (CHAR *)pDiscMsg, (CHAR *)pDiscSrc,
                              pDiscSrc->header.messageLength );
                  replyQue.push( (CHAR *)pDiscMsg ) ;
               }
               cb->getCoordSession()->delRequest( iterMap->first ) ;
               requestIdMap.erase( iterMap ) ;
            }
            if ( cb->getCoordSession()->isValidResponse( routeID,
                                                         pMsg->requestID ) )
            {
               tmpQue.push( pmdEvent );
            }
            else
            {
               SDB_OSS_FREE ( pmdEvent._Data ) ;
               pmdEvent.reset () ;
            }
            continue ;
         }
         MsgHeader *pReply = ( MsgHeader *)(pmdEvent._Data) ;

         if ( opCode != pReply->opCode ||
              ( iterMap = requestIdMap.find( pReply->requestID ) )
               == requestIdMap.end() )
         {
            if ( cb->getCoordSession()->isValidResponse( pReply->requestID ) )
            {
               tmpQue.push( pmdEvent ) ;
            }
            else
            {
               PD_LOG ( PDWARNING, 
                        "received unexpected msg(opCode=[%d]%d,"
                        "expectOpCode=[%d]%d,"
                        "groupID=%u, nodeID=%u, serviceID=%u)",
                        IS_REPLY_TYPE( pReply->opCode ),
                        GET_REQUEST_TYPE( pReply->opCode ),
                        IS_REPLY_TYPE( opCode ),
                        GET_REQUEST_TYPE( opCode ),
                        pReply->routeID.columns.groupID,
                        pReply->routeID.columns.nodeID,
                        pReply->routeID.columns.serviceID );
               SDB_OSS_FREE( pReply ) ;
            }
         }
         else
         {
            PD_LOG ( PDDEBUG, "received the reply("
                     " opCode=[%d]%d, requestID=%llu, TID=%u, "
                     "groupID=%u, nodeID=%u, serviceID=%u )",
                     IS_REPLY_TYPE( pReply->opCode ),
                     GET_REQUEST_TYPE( pReply->opCode ),
                     pReply->requestID, pReply->TID,
                     pReply->routeID.columns.groupID,
                     pReply->routeID.columns.nodeID,
                     pReply->routeID.columns.serviceID ) ;

            requestIdMap.erase( iterMap ) ;
            cb->getCoordSession()->delRequest( pReply->requestID ) ;
            replyQue.push( (CHAR *)( pmdEvent._Data ) ) ;
            pmdEvent.reset () ;
            if ( !isWaitAll )
            {
               waitTime = RTN_COORD_RSP_WAIT_TIME_QUICK;
            }
         }
      }
      if ( rc )
      {
         goto error;
      }

   done:
      while( !tmpQue.empty() )
      {
         pmdEDUEvent otherEvent;
         tmpQue.wait_and_pop( otherEvent );
         cb->postEvent( otherEvent );
      }
      PD_TRACE_EXITRC ( SDB_RTNCOGETREPLY, rc ) ;
      return rc;
   error:
      //clear the incomplete reply-queue
      if ( clearReplyIfFailed )
      {
         while ( !replyQue.empty() )
         {
            CHAR *pData = replyQue.front();
            replyQue.pop();
            SDB_OSS_FREE( pData );
         }
      }
      rtnCoordClearRequest( cb, requestIdMap ) ;
      goto done;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_GETSERVNAME, "getServiceName" )
   INT32 getServiceName ( BSONElement &beService,
                                 INT32 serviceType,
                                 std::string &strServiceName )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_GETSERVNAME ) ;
      strServiceName = "";
      do
      {
         if ( beService.type() != Array )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                     "failed to get the service-name(type=%d),\
                     the service field is invalid", serviceType );
            break;
         }
         try
         {
            BSONObjIterator i( beService.embeddedObject() );
            while ( i.more() )
            {
               BSONElement beTmp = i.next();
               BSONObj boTmp = beTmp.embeddedObject();
               BSONElement beServiceType = boTmp.getField(
                  CAT_SERVICE_TYPE_FIELD_NAME );
               if ( beServiceType.eoo() || !beServiceType.isNumber() )
               {
                  rc = SDB_INVALIDARG;
                  PD_LOG ( PDERROR, "Failed to get the service-name(type=%d),"
                           "get the field(%s) failed", serviceType,
                           CAT_SERVICE_TYPE_FIELD_NAME );
                  break;
               }
               if ( beServiceType.numberInt() == serviceType )
               {
                  BSONElement beServiceName = boTmp.getField(
                     CAT_SERVICE_NAME_FIELD_NAME );
                  if ( beServiceType.eoo() || beServiceName.type() != String )
                  {
                     rc = SDB_INVALIDARG;
                     PD_LOG ( PDERROR, "Failed to get the service-name"
                              "(type=%d), get the field(%s) failed",
                              serviceType, CAT_SERVICE_NAME_FIELD_NAME ) ;
                     break;
                  }
                  strServiceName = beServiceName.String();
                  break;
               }
            }
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR, "unexpected exception: %s", e.what() ) ;
            rc = SDB_INVALIDARG ;
         }
      }while ( FALSE );
      PD_TRACE_EXITRC ( SDB_GETSERVNAME, rc ) ;
      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETCATAINFO, "rtnCoordGetCataInfo" )
   INT32 rtnCoordGetCataInfo( pmdEDUCB *cb,
                              const CHAR *pCollectionName,
                              BOOLEAN isNeedRefreshCata,
                              CoordCataInfoPtr &cataInfo,
                              BOOLEAN *pHasUpdate )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETCATAINFO ) ;
      while( TRUE )
      {
         if ( isNeedRefreshCata )
         {
            rc = rtnCoordGetRemoteCata( cb, pCollectionName, cataInfo ) ;
            if ( SDB_OK == rc && pHasUpdate )
            {
               *pHasUpdate = TRUE ;
            }
         }
         else
         {
            rc = rtnCoordGetLocalCata ( pCollectionName, cataInfo );
            if ( SDB_CAT_NO_MATCH_CATALOG == rc )
            {
               // couldn't find the match catalogue,
               // then get from catalogue-node
               isNeedRefreshCata = TRUE ;
               continue;
            }
         }
         break;
      }
      PD_TRACE_EXITRC ( SDB_RTNCOGETCATAINFO, rc ) ;
      return rc ;
   }

   void rtnCoordRemoveGroup( UINT32 group )
   {
      pmdGetKRCB()->getCoordCB()->removeGroupInfo( group ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETLOCALCATA, "rtnCoordGetLocalCata" )
   INT32 rtnCoordGetLocalCata( const CHAR *pCollectionName,
                               CoordCataInfoPtr &cataInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETLOCALCATA ) ;
      pmdKRCB *pKrcb          = pmdGetKRCB();
      CoordCB *pCoordcb       = pKrcb->getCoordCB();
      rc = pCoordcb->getCataInfo( pCollectionName, cataInfo ) ;
      PD_TRACE_EXITRC ( SDB_RTNCOGETLOCALCATA, rc ) ;
      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETREMOTECATA, "rtnCoordGetRemoteCata" )
   INT32 rtnCoordGetRemoteCata( pmdEDUCB *cb,
                                const CHAR *pCollectionName,
                                CoordCataInfoPtr &cataInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETREMOTECATA ) ;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      BOOLEAN isNeedRefresh            = FALSE ;
      CoordGroupInfoPtr cataGroupInfo ;
      MsgRouteID nodeID ;
      UINT32 primaryID = 0 ;

      rc = rtnCoordGetCatGroupInfo( cb, FALSE, cataGroupInfo, NULL ) ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Failed to get the catalogue-node-group "
                  "info, rc: %d", rc ) ;
         goto error ;
      }

      do
      {
         BSONObj boQuery;
         BSONObj boFieldSelector;
         BSONObj boOrderBy;
         BSONObj boHint;
         try
         {
            boQuery = BSON( CAT_CATALOGNAME_NAME << pCollectionName ) ;
         }
         catch ( std::exception &e )
         {
            rc = SDB_SYS;
            PD_LOG ( PDERROR, "Get reomte catalogue failed, while "
                     "build query-obj received unexception error:%s",
                     e.what() );
            break ;
         }
         CHAR *pBuffer = NULL;
         INT32 bufferSize = 0;
         // the buffer will be free after call sendRequestToPrimary
         rc = msgBuildQueryCatalogReqMsg ( &pBuffer, &bufferSize,
                                           0, 0, 0, 1, cb->getTID(),
                                           &boQuery, &boFieldSelector,
                                           &boOrderBy, &boHint );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR, "Failed to build query-catalog-message, rc: %d",
                     rc );
            break ;
         }

         REQUESTID_MAP sendNodes;
         rc = rtnCoordSendRequestToPrimary( pBuffer, cataGroupInfo, sendNodes,
                                            pRouteAgent, MSG_ROUTE_CAT_SERVICE,
                                            cb ) ;
         if ( pBuffer != NULL )
         {
            SDB_OSS_FREE( pBuffer );
         }
         if ( rc != SDB_OK )
         {
            rtnCoordClearRequest( cb, sendNodes ) ;

            PD_LOG ( PDERROR, "Failed to send catalog-query msg to "
                     "catalogue-group, rc: %d", rc ) ;
            break ;
         }

         REPLY_QUE replyQue;
         rc = rtnCoordGetReply( cb, sendNodes, replyQue,
                                MSG_CAT_QUERY_CATALOG_RSP ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get reply from catalogue-node, "
                    "rc: %d", rc ) ;
            break ;
         }

         // process reply
         BOOLEAN isGetExpectReply = FALSE ;
         while ( !replyQue.empty() )
         {
            MsgCatQueryCatRsp *pReply = NULL ;
            pReply = (MsgCatQueryCatRsp *)(replyQue.front());
            replyQue.pop();

            if ( FALSE == isGetExpectReply )
            {
               nodeID.value = pReply->header.routeID.value ;
               primaryID = pReply->startFrom ;
               rc = rtnCoordProcessQueryCatReply( pReply, cataInfo ) ;
               if( SDB_OK == rc )
               {
                  isGetExpectReply = TRUE ;
                  pCoordcb->updateCataInfo( pCollectionName, cataInfo ) ;
               }
            }
            if ( NULL != pReply )
            {
               SDB_OSS_FREE( pReply ) ;
            }
         }

         // catalogue-node reply no primary
         // and the catalogue-group info have not be refreshed,
         // maybe the catalogue-group-info is expired and refresh it
         if ( rc )
         {
            if ( rtnCoordGroupReplyCheck( cb, rc, !isNeedRefresh, nodeID,
                                          cataGroupInfo, NULL, TRUE,
                                          primaryID ) )
            {
               isNeedRefresh = TRUE ;
               continue ;
            }

            PD_LOG( PDERROR, "Get catalog info[%s] from remote failed, rc: %d",
                    pCollectionName, rc ) ;

            if ( ( SDB_DMS_NOTEXIST == rc || SDB_DMS_EOC == rc ) &&
                 pCoordcb->isSubCollection( pCollectionName ) )
            {
               /// change the error
               rc = SDB_CLS_COORD_NODE_CAT_VER_OLD ;
            }
         }
         break ;
      }while ( TRUE ) ;

      if ( SDB_OK == rc && cataInfo->isMainCL() )
      {
         vector< string > subCLLst ;
         cataInfo->getSubCLList( subCLLst ) ;

         vector< string >::iterator iterLst = subCLLst.begin() ;
         while ( iterLst != subCLLst.end() )
         {
            CoordCataInfoPtr subCataInfoTmp ;
            rc = rtnCoordGetRemoteCata( cb, iterLst->c_str(),
                                        subCataInfoTmp ) ;
            if( rc )
            {
               PD_LOG( PDERROR, "Get main collection[%s]'s sub collection[%s] "
                       "failed, rc: %d", pCollectionName, iterLst->c_str(),
                       rc ) ;
               // remove main catalog info
               pCoordcb->delCataInfo( pCollectionName ) ;
               break ;
            }
            ++iterLst ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB_RTNCOGETREMOTECATA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETGROUPINFO, "rtnCoordGetGroupInfo" )
   INT32 rtnCoordGetGroupInfo ( pmdEDUCB *cb,
                                UINT32 groupID,
                                BOOLEAN isNeedRefresh,
                                CoordGroupInfoPtr &groupInfo,
                                BOOLEAN *pHasUpdate )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETGROUPINFO ) ;

      while( TRUE )
      {
         if ( isNeedRefresh )
         {
            rc = rtnCoordGetRemoteGroupInfo( cb, groupID, NULL, groupInfo ) ;
            if ( SDB_OK == rc && pHasUpdate )
            {
               *pHasUpdate = TRUE ;
            }
         }
         else
         {
            rc = rtnCoordGetLocalGroupInfo ( groupID, groupInfo );
            if ( SDB_COOR_NO_NODEGROUP_INFO == rc )
            {
               // couldn't find the match catalogue,
               // then get from catalogue-node
               isNeedRefresh = TRUE ;
               continue;
            }
         }
         break ;
      }

      PD_TRACE_EXITRC ( SDB_RTNCOGETGROUPINFO, rc ) ;

      return rc;
   }

   INT32 rtnCoordGetGroupInfo( pmdEDUCB *cb,
                               const CHAR *groupName,
                               BOOLEAN isNeedRefresh,
                               CoordGroupInfoPtr &groupInfo,
                               BOOLEAN *pHasUpdate )
   {
      INT32 rc = SDB_OK;

      while( TRUE )
      {
         if ( isNeedRefresh )
         {
            rc = rtnCoordGetRemoteGroupInfo( cb, 0, groupName, groupInfo ) ;
            if ( SDB_OK == rc && pHasUpdate )
            {
               *pHasUpdate = TRUE ;
            }
         }
         else
         {
            rc = rtnCoordGetLocalGroupInfo ( groupName, groupInfo ) ;
            if ( SDB_COOR_NO_NODEGROUP_INFO == rc )
            {
               // couldn't find the match catalogue,
               // then get from catalogue-node
               isNeedRefresh = TRUE ;
               continue ;
            }
         }
         break ;
      }

      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETLOCALGROUPINFO, "rtnCoordGetLocalGroupInfo" )
   INT32 rtnCoordGetLocalGroupInfo ( UINT32 groupID,
                                     CoordGroupInfoPtr &groupInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETLOCALGROUPINFO ) ;
      pmdKRCB *pKrcb          = pmdGetKRCB();
      CoordCB *pCoordcb       = pKrcb->getCoordCB();

      if ( CATALOG_GROUPID == groupID )
      {
         groupInfo = pCoordcb->getCatGroupInfo() ;
      }
      else
      {
         rc = pCoordcb->getGroupInfo( groupID, groupInfo ) ;
      }
      PD_TRACE_EXITRC ( SDB_RTNCOGETLOCALGROUPINFO, rc ) ;
      return rc ;
   }

   INT32 rtnCoordGetLocalGroupInfo ( const CHAR *groupName,
                                     CoordGroupInfoPtr &groupInfo )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *pKrcb          = pmdGetKRCB() ;
      CoordCB *pCoordcb       = pKrcb->getCoordCB() ;
      if ( 0 == ossStrcmp( CATALOG_GROUPNAME, groupName ) )
      {
         groupInfo = pCoordcb->getCatGroupInfo() ;
      }
      else
      {
         rc = pCoordcb->getGroupInfo( groupName, groupInfo ) ;
      }
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETREMOTEGROUPINFO, "rtnCoordGetRemoteGroupInfo" )
   INT32 rtnCoordGetRemoteGroupInfo ( pmdEDUCB *cb,
                                      UINT32 groupID,
                                      const CHAR *groupName,
                                      CoordGroupInfoPtr &groupInfo,
                                      BOOLEAN addToLocal )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNCOGETREMOTEGROUPINFO ) ;
      pmdKRCB *pKrcb          = pmdGetKRCB();
      CoordCB *pCoordcb       = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      BOOLEAN isNeedRefresh = FALSE;
      CHAR *buf = NULL ;
      MsgCatGroupReq *msg = NULL ;
      MsgCatGroupReq msgGroupReq ;
      CoordGroupInfoPtr cataGroupInfo ;
      MsgRouteID nodeID ;

      // if catalogure group
      if ( CATALOG_GROUPID == groupID ||
           ( groupName && 0 == ossStrcmp( groupName, CATALOG_GROUPNAME ) ) )
      {
         rc = rtnCoordGetRemoteCatGroupInfo( cb, groupInfo ) ;
         goto done ;
      }

      rc = rtnCoordGetCatGroupInfo( cb, FALSE, cataGroupInfo, NULL );
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Failed to get cata-group-info,"
                  "no catalogue-node-group info" );
        goto error ;
      }

      if ( NULL == groupName )
      {
         msgGroupReq.id.columns.groupID = groupID;
         msgGroupReq.id.columns.nodeID = 0;
         msgGroupReq.id.columns.serviceID = 0;
         msgGroupReq.header.messageLength = sizeof( MsgCatGroupReq );
         msgGroupReq.header.opCode = MSG_CAT_GRP_REQ;
         msgGroupReq.header.routeID.value = 0;
         msgGroupReq.header.TID = cb->getTID();
         msg = &msgGroupReq ;
      }
      else
      {
         UINT32 nameLen = ossStrlen( groupName ) + 1 ;
         UINT32 msgLen = nameLen +  sizeof(MsgCatGroupReq) ;
         buf = ( CHAR * )SDB_OSS_MALLOC( msgLen ) ;
         if ( NULL == buf )
         {
            PD_LOG( PDERROR, "failed to allocate mem." ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         msg = ( MsgCatGroupReq * )buf ;
         msg->id.value = 0 ;
         msg->header.messageLength = msgLen ;
         msg->header.opCode = MSG_CAT_GRP_REQ;
         msg->header.routeID.value = 0;
         msg->header.TID = cb->getTID();
         ossMemcpy( buf + sizeof(MsgCatGroupReq),
                    groupName, nameLen ) ;
      }

      do
      {
         REQUESTID_MAP sendNodes ;
         rc = rtnCoordSendRequestToPrimary( (CHAR *)(msg),
                                            cataGroupInfo, sendNodes,
                                            pRouteAgent,
                                            MSG_ROUTE_CAT_SERVICE,
                                            cb );
         if ( rc != SDB_OK )
         {
            rtnCoordClearRequest( cb, sendNodes ) ;

            if ( groupName )
            {
               PD_LOG ( PDERROR, "Failed to get group info[%s], because of "
                        "send group-info-request failed, rc: %d",
                        groupName, rc ) ;
            }
            else
            {
               PD_LOG ( PDERROR, "Failed to get group info[%u], because of "
                        "send group-info-request failed, rc: %d",
                        groupID, rc ) ;
            }
            break ;
         }

         REPLY_QUE replyQue;
         rc = rtnCoordGetReply( cb, sendNodes, replyQue,
                                MSG_CAT_GRP_RES ) ;
         if ( rc != SDB_OK )
         {
            if ( groupName )
            {
               PD_LOG ( PDWARNING, "Failed to get group info[%s], because of "
                        "get reply failed, rc: %d", groupName, rc ) ;
            }
            else
            {
               PD_LOG ( PDWARNING, "Failed to get group info[%u], because of "
                        "get reply failed, rc: %d", groupID, rc ) ;
            }
            break ;
         }

         // process reply
         BOOLEAN getExpected = FALSE ;
         UINT32 primaryID = 0 ;
         while ( !replyQue.empty() )
         {
            MsgHeader *pReply = NULL;
            pReply = (MsgHeader *)(replyQue.front());
            replyQue.pop() ;

            if ( FALSE == getExpected )
            {
               nodeID.value = pReply->routeID.value ;
               primaryID = MSG_GET_INNER_REPLY_STARTFROM( pReply ) ;
               rc = rtnCoordProcessGetGroupReply( pReply, groupInfo ) ;
               if ( SDB_OK == rc )
               {
                  getExpected = TRUE ;
                  if ( addToLocal )
                  {
                     pCoordcb->addGroupInfo( groupInfo ) ;
                     rc = rtnCoordUpdateRoute( groupInfo, pRouteAgent,
                                               MSG_ROUTE_SHARD_SERVCIE ) ;
                  }
               }
            }
            if ( NULL != pReply )
            {
               SDB_OSS_FREE( pReply );
            }
         }

         if ( rc != SDB_OK )
         {
            if ( rtnCoordGroupReplyCheck( cb, rc, !isNeedRefresh, nodeID,
                                          cataGroupInfo, NULL, TRUE,
                                          primaryID ) )
            {
               isNeedRefresh = TRUE ;
               continue;
            }

            if ( groupName )
            {
               PD_LOG ( PDERROR, "Failed to get group info[%s], because of "
                        "reply error, flag: %d", groupName, rc ) ;
            }
            else
            {
               PD_LOG ( PDERROR, "Failed to get group info[%u], because of "
                        "reply error, flag: %d", groupID, rc ) ;
            }
         }
         break ;
      }while ( TRUE ) ;

   done:
      if ( NULL != buf )
      {
         SDB_OSS_FREE( buf ) ;
         buf = NULL ;
      }
      PD_TRACE_EXITRC ( SDB_RTNCOGETREMOTEGROUPINFO, rc ) ;
      return rc;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETCATGROUPINFO, "rtnCoordGetCatGroupInfo" )
   INT32 rtnCoordGetCatGroupInfo ( pmdEDUCB *cb,
                                   BOOLEAN isNeedRefresh,
                                   CoordGroupInfoPtr &groupInfo,
                                   BOOLEAN *pHasUpdate )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETCATGROUPINFO ) ;
      UINT32 retryCount = 0 ;

      while( TRUE )
      {
         if ( isNeedRefresh )
         {
            rc = rtnCoordGetRemoteCatGroupInfo( cb, groupInfo ) ;
            if ( SDB_CLS_GRP_NOT_EXIST == rc && retryCount < 30 )
            {
               ++retryCount ;
               ossSleep( 100 ) ;
               continue ;
            }
            if ( SDB_OK == rc && pHasUpdate )
            {
               *pHasUpdate = TRUE ;
            }
         }
         else
         {
            rc = rtnCoordGetLocalCatGroupInfo ( groupInfo ) ;
            if ( ( SDB_OK == rc && groupInfo->getGroupSize() == 0 ) ||
                   SDB_COOR_NO_NODEGROUP_INFO == rc )
            {
               // couldn't find the match group-info,
               // then get from catalogue-node
               isNeedRefresh = TRUE;
               continue ;
            }
         }
         break;
      }
      PD_TRACE_EXITRC ( SDB_RTNCOGETCATGROUPINFO, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETLOCALCATGROUPINFO, "rtnCoordGetLocalCatGroupInfo" )
   INT32 rtnCoordGetLocalCatGroupInfo ( CoordGroupInfoPtr &groupInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETLOCALCATGROUPINFO ) ;
      pmdKRCB *pKrcb          = pmdGetKRCB();
      CoordCB *pCoordcb       = pKrcb->getCoordCB();
      groupInfo = pCoordcb->getCatGroupInfo();
      PD_TRACE_EXITRC ( SDB_RTNCOGETLOCALCATGROUPINFO, rc ) ;
      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETREMOTECATAGROUPINFOBYADDR, "rtnCoordGetRemoteCataGroupInfoByAddr" )
   INT32 rtnCoordGetRemoteCataGroupInfoByAddr ( pmdEDUCB *cb,
                                                CoordGroupInfoPtr &groupInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETREMOTECATAGROUPINFOBYADDR ) ;
      UINT64 reqID;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      MsgHeader *pReply                = NULL;
      ossQueue<pmdEDUEvent> tmpQue ;
      UINT32 sendPos                   = 0 ;

      MsgCatCatGroupReq msgGroupReq;
      msgGroupReq.id.columns.groupID = CAT_CATALOG_GROUPID ;
      msgGroupReq.id.columns.nodeID = 0;
      msgGroupReq.id.columns.serviceID = 0;
      msgGroupReq.header.messageLength = sizeof( MsgCatGroupReq );
      msgGroupReq.header.opCode = MSG_CAT_GRP_REQ ;
      msgGroupReq.header.routeID.value = 0;
      msgGroupReq.header.TID = cb->getTID();

      CoordVecNodeInfo cataNodeAddrList ;
      pCoordcb->getCatNodeAddrList( cataNodeAddrList ) ;

      if ( cataNodeAddrList.size() == 0 )
      {
         rc = SDB_CAT_NO_ADDR_LIST ;
         PD_LOG ( PDERROR, "no catalog node info" );
         goto error ;
      }

      while( sendPos < cataNodeAddrList.size() )
      {
         for ( ; sendPos < cataNodeAddrList.size() ; ++sendPos )
         {
            rc = pRouteAgent->syncSendWithoutCheck( cataNodeAddrList[sendPos]._id,
                                                    (MsgHeader*)&msgGroupReq,
                                                    reqID, cb ) ;
            if ( SDB_OK == rc )
            {
               break ;
            }
         }
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR, "Failed to send the request to catalog-node"
                     "(rc=%d)", rc );
            break ;
         }
         while ( TRUE )
         {
            pmdEDUEvent pmdEvent;
            BOOLEAN isGotMsg = cb->waitEvent( pmdEvent,
                                              RTN_COORD_RSP_WAIT_TIME ) ;
            if ( cb->isForced() ||
                 ( cb->isInterrupted() && !( cb->isDisconnected() ) )
                )
            {
               rc = SDB_APP_INTERRUPT ;
               break ;
            }
            if ( FALSE == isGotMsg )
            {
               continue ;
            }
            if ( PMD_EDU_EVENT_MSG != pmdEvent._eventType )
            {
               PD_LOG ( PDWARNING, "Received unknown event(eventType=%d)",
                        pmdEvent._eventType ) ;
               continue ;
            }
            MsgHeader *pMsg = (MsgHeader *)(pmdEvent._Data) ;
            if ( NULL == pMsg )
            {
               PD_LOG ( PDWARNING, "Received invalid msg-event(data is null)" );
               continue ;
            }
            if ( (UINT32)pMsg->opCode != MSG_CAT_GRP_RES ||
                 pMsg->requestID != reqID )
            {
               if ( cb->getCoordSession()->isValidResponse( reqID ) )
               {
                  tmpQue.push( pmdEvent );
               }
               else
               {
                  PD_LOG( PDWARNING, "Received unexpected message"
                        "(opCode=[%d]%d, requestID=%llu, TID=%u, "
                        "groupID=%u, nodeID=%u, serviceID=%u )",
                        IS_REPLY_TYPE( pMsg->opCode ),
                        GET_REQUEST_TYPE( pMsg->opCode ),
                        pMsg->requestID, pMsg->TID,
                        pMsg->routeID.columns.groupID,
                        pMsg->routeID.columns.nodeID,
                        pMsg->routeID.columns.serviceID );
                  SDB_OSS_FREE( pMsg ) ;
               }
               continue ;
            }
            // recv the reply msg
            cb->getCoordSession()->delRequest( reqID ) ;
            pReply = (MsgHeader *)pMsg ;
            break ;
         }

         if ( rc != SDB_OK || NULL == pReply )
         {
            PD_LOG ( PDERROR, "Failed to get reply(rc=%d)", rc );
            break ;
         }
         rc = rtnCoordProcessGetGroupReply( pReply, groupInfo ) ;
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "Failed to get catalog-group info, reply error(rc=%d)",
                     rc ) ;
            continue ;
         }
         rc = rtnCoordUpdateRoute( groupInfo, pRouteAgent,
                                   MSG_ROUTE_CAT_SERVICE ) ;
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR, "Failed to get catalog-group info,"
                     "update route failed(rc=%d)", rc );
            ++sendPos ;
            SDB_OSS_FREE( pReply ) ;
            pReply = NULL ;
            continue ;
         }

         // update shard sevice also
         rtnCoordUpdateRoute( groupInfo, pRouteAgent,
                              MSG_ROUTE_SHARD_SERVCIE ) ;

         pCoordcb->updateCatGroupInfo( groupInfo ) ;
         break ;
      }

   done:
      if ( NULL != pReply )
      {
         SDB_OSS_FREE( pReply ) ;
      }
      while ( !tmpQue.empty() )
      {
         pmdEDUEvent otherEvent;
         tmpQue.wait_and_pop( otherEvent );
         cb->postEvent( otherEvent );
      }
      PD_TRACE_EXITRC ( SDB_RTNCOGETREMOTECATAGROUPINFOBYADDR, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETREMOTECATGROUPINFO, "rtnCoordGetRemoteCatGroupInfo" )
   INT32 rtnCoordGetRemoteCatGroupInfo ( pmdEDUCB *cb,
                                         CoordGroupInfoPtr &groupInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETREMOTECATGROUPINFO ) ;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();

      BOOLEAN hasRetry                 = FALSE ;
      MsgRouteID nodeID ;
      CoordGroupInfoPtr cataGroupInfo ;
      rtnCoordGetLocalCatGroupInfo( cataGroupInfo ) ;

      MsgCatCatGroupReq msgGroupReq;
      msgGroupReq.id.columns.groupID = CAT_CATALOG_GROUPID;
      msgGroupReq.id.columns.nodeID = 0;
      msgGroupReq.id.columns.serviceID = 0;
      msgGroupReq.header.messageLength = sizeof( MsgCatGroupReq );
      msgGroupReq.header.opCode = MSG_CAT_GRP_REQ ;

      if ( cataGroupInfo->getGroupSize() == 0 )
      {
         rc = rtnCoordGetRemoteCataGroupInfoByAddr( cb, groupInfo ) ;
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR, "Failed to get cata-group-info by addr, "
                     "rc: %d", rc ) ;
            goto error ;
         }
         goto done ;
      }

      do
      {
         msgGroupReq.header.routeID.value = 0;
         msgGroupReq.header.TID = cb->getTID();
         REQUESTID_MAP sendNodes ;
         rc = rtnCoordSendRequestToOne( (CHAR *)(&msgGroupReq),
                                        cataGroupInfo, sendNodes,
                                        pRouteAgent, MSG_ROUTE_CAT_SERVICE,
                                        cb, FALSE ) ;
         if ( rc != SDB_OK )
         {
            rtnCoordClearRequest( cb, sendNodes );
            PD_LOG ( PDERROR, "Failed to get cata-group-info from "
                     "catalogue-node,send group-info-request failed, rc: %d",
                     rc ) ;
            break ;
         }

         REPLY_QUE replyQue;
         rc = rtnCoordGetReply( cb, sendNodes, replyQue,
                                MSG_CAT_GRP_RES ) ;
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDWARNING, "Failed to get cata-group-info from "
                     "catalogue-node, get reply failed, rc: %d", rc );
            break ;
         }

         BOOLEAN getExpected = FALSE ;
         UINT32 primaryID = 0 ;
         while ( !replyQue.empty() )
         {
            MsgHeader *pReply = NULL;
            pReply = (MsgHeader *)(replyQue.front());
            replyQue.pop();

            if ( FALSE == getExpected )
            {
               nodeID.value = pReply->routeID.value ;
               primaryID = MSG_GET_INNER_REPLY_STARTFROM(pReply) ;
               rc = rtnCoordProcessGetGroupReply( pReply, groupInfo ) ;
               if ( SDB_OK == rc )
               {
                  getExpected = TRUE ;
                  pCoordcb->updateCatGroupInfo( groupInfo );
                  rc = rtnCoordUpdateRoute( groupInfo,  pRouteAgent,
                                            MSG_ROUTE_CAT_SERVICE ) ;
                  // update shard service also
                  rtnCoordUpdateRoute( groupInfo, pRouteAgent,
                                       MSG_ROUTE_SHARD_SERVCIE ) ;
               }
            }
            if ( NULL != pReply )
            {
               SDB_OSS_FREE( pReply );
            }
         }

         if ( rc != SDB_OK )
         {
            if ( rtnCoordGroupReplyCheck( cb, rc, !hasRetry, nodeID,
                                          cataGroupInfo, NULL,
                                          TRUE, primaryID ) )
            {
               hasRetry = TRUE ;
               continue;
            }
            PD_LOG( PDERROR, "Get catalog group info from remote failed, "
                    "rc: %d", rc ) ;
         }
         break ;
      }while ( TRUE ) ;

   done:
      PD_TRACE_EXITRC ( SDB_RTNCOGETREMOTECATGROUPINFO, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOUPDATEROUTE, "rtnCoordUpdateRoute" )
   INT32 rtnCoordUpdateRoute ( CoordGroupInfoPtr &groupInfo,
                               netMultiRouteAgent *pRouteAgent,
                               MSG_ROUTE_SERVICE_TYPE type )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOUPDATEROUTE ) ;

      string host ;
      string service ;
      MsgRouteID routeID ;
      routeID.value = MSG_INVALID_ROUTEID ;

      UINT32 index = 0 ;
      clsGroupItem *groupItem = groupInfo->getGroupItem() ;
      while ( SDB_OK == groupItem->getNodeInfo( index++, routeID, host,
                                                service, type ) )
      {
         rc = pRouteAgent->updateRoute( routeID, host.c_str(),
                                        service.c_str() ) ;
         if ( rc != SDB_OK )
         {
            if ( SDB_NET_UPDATE_EXISTING_NODE == rc )
            {
               rc = SDB_OK;
            }
            else
            {
               PD_LOG ( PDERROR, "update route failed (rc=%d)", rc ) ;
               break;
            }
         }
      }
      PD_TRACE_EXITRC ( SDB_RTNCOUPDATEROUTE, rc ) ;
      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETGROUPSBYCATAINFO, "rtnCoordGetGroupsByCataInfo" )
   INT32 rtnCoordGetGroupsByCataInfo( CoordCataInfoPtr &cataInfo,
                                      CoordGroupList &sendGroupLst,
                                      CoordGroupList &groupLst,
                                      pmdEDUCB *cb,
                                      BOOLEAN *pHasUpdate,
                                      const BSONObj *pQuery )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNCOGETGROUPSBYCATAINFO ) ;
      if ( !cataInfo->isMainCL() )
      {
         // normal collection or sub-collection
         if ( NULL == pQuery || pQuery->isEmpty() )
         {
            cataInfo->getGroupLst( groupLst ) ;
         }
         else
         {
            cataInfo->getGroupByMatcher( *pQuery, groupLst ) ;
         }

         if ( groupLst.size() <= 0 )
         {
            if ( pQuery )
            {
               PD_LOG( PDERROR, "Failed to get groups for obj[%s] from "
                       "catalog info[%s]", pQuery->toString().c_str(),
                       cataInfo->getCatalogSet()->toCataInfoBson(
                       ).toString().c_str() ) ;
            }
            SDB_ASSERT( FALSE, "Catalog can't be empty" ) ;
            rc = SDB_CAT_NO_MATCH_CATALOG ;
         }
         else
         {
            //don't resend to the node which reply ok
            CoordGroupList::iterator iter = sendGroupLst.begin();
            while( iter != sendGroupLst.end() )
            {
               groupLst.erase( iter->first );
               ++iter;
            }
         }
      }
      else
      {
         // main-collection
         vector< string > subCLLst ;

         if ( NULL == pQuery || pQuery->isEmpty() )
         {
            cataInfo->getSubCLList( subCLLst ) ;
         }
         else
         {
            cataInfo->getMatchSubCLs( *pQuery, subCLLst ) ;
         }

         if ( 0 == subCLLst.size() &&
              ( !pHasUpdate ||
                ( pHasUpdate && !(*pHasUpdate) ) ) )
         {
            if ( SDB_OK == rtnCoordGetRemoteCata( cb, cataInfo->getName(),
                                                  cataInfo ) )
            {
               if ( pHasUpdate )
               {
                  *pHasUpdate = TRUE ;
               }
               if ( NULL == pQuery || pQuery->isEmpty() )
               {
                  cataInfo->getSubCLList( subCLLst ) ;
               }
               else
               {
                  cataInfo->getMatchSubCLs( *pQuery, subCLLst ) ;
               }
            }
         }

         vector< string >::iterator iterCL = subCLLst.begin() ;
         while( iterCL != subCLLst.end() )
         {
            CoordCataInfoPtr subCataInfo ;
            CoordGroupList groupLstTmp ;
            CoordGroupList::iterator iterGrp ;
            rc = rtnCoordGetCataInfo( cb, (*iterCL).c_str(), FALSE,
                                      subCataInfo, NULL ) ;
            PD_RC_CHECK( rc, PDWARNING,
                         "Failed to get sub-collection catalog-info(rc=%d)",
                         rc ) ;
            rc = rtnCoordGetGroupsByCataInfo( subCataInfo,
                                              sendGroupLst,
                                              groupLstTmp,
                                              cb,
                                              NULL,
                                              pQuery ) ;
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to get sub-collection group-info(rc=%d)",
                         rc ) ;

            iterGrp = groupLstTmp.begin() ;
            while ( iterGrp != groupLstTmp.end() )
            {
               groupLst[ iterGrp->first ] = iterGrp->second ;
               ++iterGrp ;
            }
            ++iterCL ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB_RTNCOGETGROUPSBYCATAINFO, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOSENDREQUESTTONODESWITHOUTREPLY, "rtnCoordSendRequestToNodesWithOutReply" )
   void rtnCoordSendRequestToNodesWithOutReply( void *pBuffer,
                                                ROUTE_SET &nodes,
                                                netMultiRouteAgent *pRouteAgent )
   {
      PD_TRACE_ENTRY ( SDB_RTNCOSENDREQUESTTONODESWITHOUTREPLY ) ;
      pRouteAgent->multiSyncSend( nodes, pBuffer ) ;
      PD_TRACE_EXIT ( SDB_RTNCOSENDREQUESTTONODESWITHOUTREPLY ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOSENDREQUESTTONODEWITHOUTCHECK, "rtnCoordSendRequestToNodeWithoutCheck" )
   INT32 rtnCoordSendRequestToNodeWithoutCheck( void *pBuffer,
                                                const MsgRouteID &routeID,
                                                netMultiRouteAgent *pRouteAgent,
                                                pmdEDUCB *cb,
                                                REQUESTID_MAP &sendNodes )
   {
      INT32 rc = SDB_OK;
      UINT64 reqID = 0;
      PD_TRACE_ENTRY ( SDB_RTNCOSENDREQUESTTONODEWITHOUTCHECK ) ;
      rc = pRouteAgent->syncSendWithoutCheck( routeID, pBuffer, reqID, cb );
      PD_RC_CHECK ( rc, PDERROR, "Failed to send the request to node"
                    "(groupID=%u, nodeID=%u, serviceID=%u, rc=%d)",
                    routeID.columns.groupID,
                    routeID.columns.nodeID,
                    routeID.columns.serviceID,
                    rc ) ;
      sendNodes[ reqID ] = routeID ;

   done:
      PD_TRACE_EXITRC ( SDB_RTNCOSENDREQUESTTONODEWITHOUTCHECK, rc ) ;
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOSENDREQUESTTONODEWITHOUTREPLY, "rtnCoordSendRequestToNodeWithoutReply" )
   INT32 rtnCoordSendRequestToNodeWithoutReply( void *pBuffer,
                                                MsgRouteID &routeID,
                                                netMultiRouteAgent *pRouteAgent )
   {
      INT32 rc = SDB_OK;
      UINT64 reqID = 0;
      PD_TRACE_ENTRY ( SDB_RTNCOSENDREQUESTTONODEWITHOUTREPLY ) ;
      rc = pRouteAgent->syncSend( routeID, pBuffer, reqID, NULL );
      PD_RC_CHECK ( rc, PDERROR, "Failed to send the request to node"
                    "(groupID=%u, nodeID=%u, serviceID=%u, rc=%d)",
                    routeID.columns.groupID,
                    routeID.columns.nodeID,
                    routeID.columns.serviceID,
                    rc ) ;

   done:
      PD_TRACE_EXITRC ( SDB_RTNCOSENDREQUESTTONODEWITHOUTREPLY, rc ) ;
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOSENDREQUESTTONODE, "rtnCoordSendRequestToNode" )
   INT32 rtnCoordSendRequestToNode( void *pBuffer,
                                    MsgRouteID routeID,
                                    netMultiRouteAgent *pRouteAgent,
                                    pmdEDUCB *cb,
                                    REQUESTID_MAP &sendNodes )
   {
      INT32 rc = SDB_OK;
      UINT64 reqID = 0;
      PD_TRACE_ENTRY ( SDB_RTNCOSENDREQUESTTONODE ) ;
      rc = pRouteAgent->syncSend( routeID, pBuffer, reqID, cb );
      PD_RC_CHECK ( rc, PDERROR, "Failed to send the request to node"
                    "(groupID=%u, nodeID=%u, serviceID=%u, rc=%d)",
                    routeID.columns.groupID,
                    routeID.columns.nodeID,
                    routeID.columns.serviceID,
                    rc ) ;
      sendNodes[ reqID ] = routeID ;

   done:
      PD_TRACE_EXITRC ( SDB_RTNCOSENDREQUESTTONODE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOSENDREQUESTTONODE2, "rtnCoordSendRequestToNode" )
   INT32 rtnCoordSendRequestToNode( void *pBuffer,
                                    MsgRouteID routeID,
                                    netMultiRouteAgent *pRouteAgent,
                                    pmdEDUCB *cb,
                                    const netIOVec &iov,
                                    REQUESTID_MAP &sendNodes )
   {
      INT32 rc = SDB_OK;
      UINT64 reqID = 0;
      PD_TRACE_ENTRY ( SDB_RTNCOSENDREQUESTTONODE2 ) ;
      rc = pRouteAgent->syncSend( routeID, ( MsgHeader *)pBuffer, iov, reqID, cb );
      PD_RC_CHECK ( rc, PDERROR,
                  "failed to send the request to node"
                  "(groupID=%u, nodeID=%u, serviceID=%u, rc=%d)",
                  routeID.columns.groupID,
                  routeID.columns.nodeID,
                  routeID.columns.serviceID,
                  rc );
      sendNodes[ reqID ] = routeID;
   done:
      PD_TRACE_EXITRC ( SDB_RTNCOSENDREQUESTTONODE2, rc ) ;
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOSENDREQUESTTONODEGROUPS, "rtnCoordSendRequestToNodeGroups" )
   INT32 rtnCoordSendRequestToNodeGroups( CHAR *pBuffer,
                                          CoordGroupList &groupLst,
                                          CoordGroupMap &mapGroupInfo,
                                          BOOLEAN isSendPrimary,
                                          netMultiRouteAgent *pRouteAgent,
                                          pmdEDUCB *cb,
                                          REQUESTID_MAP &sendNodes,
                                          BOOLEAN isResend,
                                          MSG_ROUTE_SERVICE_TYPE type )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOSENDREQUESTTONODEGROUPS ) ;
      CoordGroupList::iterator iter ;

      rc = rtnGroupList2GroupPtr( cb, groupLst, mapGroupInfo, FALSE ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "Get groups info failed, rc: %d", rc ) ;
         goto error ;
      }

      iter = groupLst.begin() ;
      while ( iter != groupLst.end() )
      {
         rc = rtnCoordSendRequestToNodeGroup( pBuffer,
                                              mapGroupInfo[ iter->first ],
                                              isSendPrimary, pRouteAgent,
                                              cb, sendNodes, isResend,
                                              type ) ;
         if ( SDB_OK != rc )
         {
            MsgHeader *pMsg = ( MsgHeader* )pBuffer ;
            PD_LOG( PDERROR, "Send msg[opCode: %d, TID: %u] to group[%u] "
                    "failed, rc:%d", pMsg->opCode, pMsg->TID,
                    iter->first, rc ) ;
            goto error ;
         }
         ++iter;
      }

   done:
      PD_TRACE_EXITRC ( SDB_RTNCOSENDREQUESTTONODEGROUPS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOSENDREQUESTTONODEGROUPS2, "rtnCoordSendRequestToNodeGroups" )
   INT32 rtnCoordSendRequestToNodeGroups( MsgHeader *pBuffer,
                                          CoordGroupList &groupLst,
                                          CoordGroupMap &mapGroupInfo,
                                          BOOLEAN isSendPrimary,
                                          netMultiRouteAgent *pRouteAgent,
                                          pmdEDUCB *cb,
                                          const netIOVec &iov,
                                          REQUESTID_MAP &sendNodes, // out
                                          BOOLEAN isResend,
                                          MSG_ROUTE_SERVICE_TYPE type )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOSENDREQUESTTONODEGROUPS2 ) ;
      CoordGroupList::iterator iter ;

      rc = rtnGroupList2GroupPtr( cb, groupLst, mapGroupInfo, FALSE ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "Get groups info failed, rc: %d" ) ;
         goto error ;
      }

      iter = groupLst.begin() ;
      while ( iter != groupLst.end() )
      {
         rc = rtnCoordSendRequestToNodeGroup( pBuffer,
                                              mapGroupInfo[ iter->first ],
                                              isSendPrimary, pRouteAgent,
                                              cb, iov, sendNodes,
                                              isResend, type ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Send msg[opCode: %d, TID: %u] to group[%u] "
                    "failed, rc:%d", pBuffer->opCode, pBuffer->TID,
                    iter->first, rc ) ;
            goto error ;
         }
         ++iter ;
      }

   done:
      PD_TRACE_EXITRC( SDB_RTNCOSENDREQUESTTONODEGROUPS2, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnCoordSendRequestToNodeGroups( MsgHeader *pBuffer,
                                          CoordGroupList &groupLst,
                                          CoordGroupMap &mapGroupInfo,
                                          BOOLEAN isSendPrimary,
                                          netMultiRouteAgent *pRouteAgent,
                                          pmdEDUCB *cb,
                                          GROUP_2_IOVEC &iov,
                                          REQUESTID_MAP &sendNodes, // out
                                          BOOLEAN isResend,
                                          MSG_ROUTE_SERVICE_TYPE type )
   {
      INT32 rc = SDB_OK ;
      CoordGroupList::iterator iter ;
      GROUP_2_IOVEC::iterator itIO ;
      netIOVec *pCommonIO = NULL ;
      netIOVec *pIOVec = NULL ;

      rc = rtnGroupList2GroupPtr( cb, groupLst, mapGroupInfo, FALSE ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "Get groups info failed, rc: %d" ) ;
         goto error ;
      }

      // find common iovec
      itIO = iov.find( 0 ) ; // group id is 0 for common iovec
      if ( iov.end() != itIO )
      {
         pCommonIO = &( itIO->second ) ;
      }

      iter = groupLst.begin() ;
      while ( iter != groupLst.end() )
      {
         // find groups iovec
         itIO = iov.find( iter->first ) ;
         if ( itIO != iov.end() )
         {
            pIOVec = &( itIO->second ) ;
         }
         else if ( pCommonIO )
         {
            pIOVec = pCommonIO ;
         }
         else
         {
            // error, not find the io datas
            PD_LOG( PDERROR, "Can't find the group[%d]'s iovec datas",
                    iter->first ) ;
            rc = SDB_SYS ;
            SDB_ASSERT( FALSE, "Group iovec is null" ) ;
            goto error ;
         }

         rc = rtnCoordSendRequestToNodeGroup( pBuffer,
                                              mapGroupInfo[ iter->first ],
                                              isSendPrimary, pRouteAgent,
                                              cb, *pIOVec, sendNodes,
                                              isResend, type ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Send msg[opCode: %d, TID: %u] to group[%u] "
                    "failed, rc:%d", pBuffer->opCode, pBuffer->TID,
                    iter->first, rc ) ;
            goto error ;
         }
         ++iter ;
      }

   done:
      PD_TRACE_EXITRC( SDB_RTNCOSENDREQUESTTONODEGROUPS2, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnCoordSendRequestToNodeGroup( MsgHeader *pBuffer,
                                         CoordGroupInfoPtr &groupInfo,
                                         BOOLEAN isSendPrimary,
                                         netMultiRouteAgent *pRouteAgent,
                                         pmdEDUCB *cb,
                                         const netIOVec &iov,
                                         REQUESTID_MAP &sendNodes,
                                         BOOLEAN isResend,
                                         MSG_ROUTE_SERVICE_TYPE type )
   {
      return _rtnCoordSendRequestToNodeGroup( pBuffer, groupInfo, isSendPrimary,
                                              pRouteAgent, type, cb,
                                              sendNodes, &iov, isResend ) ;
   }

   INT32 rtnCoordSendRequestToNodeGroup( CHAR *pBuffer,
                                         CoordGroupInfoPtr &groupInfo,
                                         BOOLEAN isSendPrimary,
                                         netMultiRouteAgent *pRouteAgent,
                                         pmdEDUCB *cb,
                                         REQUESTID_MAP &sendNodes,
                                         BOOLEAN isResend,
                                         MSG_ROUTE_SERVICE_TYPE type )
   {
      return _rtnCoordSendRequestToNodeGroup( (MsgHeader*)pBuffer, groupInfo,
                                              isSendPrimary, pRouteAgent,
                                              type, cb, sendNodes, NULL,
                                              isResend ) ;
   }

   INT32 rtnCoordSendRequestToPrimary( CHAR *pBuffer,
                                       CoordGroupInfoPtr &groupInfo,
                                       REQUESTID_MAP &sendNodes,
                                       netMultiRouteAgent *pRouteAgent,
                                       MSG_ROUTE_SERVICE_TYPE type,
                                       pmdEDUCB *cb )
   {
      return _rtnCoordSendRequestToPrimary( (MsgHeader*)pBuffer, groupInfo,
                                            pRouteAgent, type, cb,
                                            sendNodes, NULL ) ;
   }

   INT32 rtnCoordSendRequestToPrimary( MsgHeader *pBuffer,
                                       CoordGroupInfoPtr &groupInfo,
                                       REQUESTID_MAP &sendNodes,
                                       netMultiRouteAgent *pRouteAgent,
                                       const netIOVec &iov,
                                       MSG_ROUTE_SERVICE_TYPE type,
                                       pmdEDUCB *cb )
   {
      return _rtnCoordSendRequestToPrimary( pBuffer, groupInfo, pRouteAgent,
                                            type, cb, sendNodes, &iov ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETNODEPOS, "rtnCoordGetNodePos" )
   void rtnCoordGetNodePos( INT32 preferReplicaType,
                            clsGroupItem *groupItem,
                            UINT32 random,
                            UINT32 &pos )
   {
      PD_TRACE_ENTRY ( SDB_RTNCOGETNODEPOS ) ;
      UINT32 posTmp = 0 ;

      switch( preferReplicaType )
      {
         case PREFER_REPL_NODE_1:
         case PREFER_REPL_NODE_2:
         case PREFER_REPL_NODE_3:
         case PREFER_REPL_NODE_4:
         case PREFER_REPL_NODE_5:
         case PREFER_REPL_NODE_6:
         case PREFER_REPL_NODE_7:
            {
               posTmp = preferReplicaType - 1 ;
               break;
            }
         case PREFER_REPL_MASTER:
            {
               posTmp = groupItem->getPrimaryPos() ;
               // if there is no primary,
               // then do not break and go on to
               // get random node
               if ( CLS_RG_NODE_POS_INVALID != posTmp )
               {
                  break ;
               }
            }
         case PREFER_REPL_ANYONE:
         case PREFER_REPL_SLAVE:
         default:
            {
               posTmp = random ;
               break;
            }
      }

      if( groupItem->nodeCount() > 0 )
      {
         pos = posTmp % groupItem->nodeCount() ;
      }
   }

   void rtnCoordGetNextNode( INT32 preferReplicaType,
                             clsGroupItem *groupItem,
                             UINT32 &selTimes,
                             UINT32 &curPos )
   {
      if( selTimes >= groupItem->nodeCount() )
      {
         curPos = CLS_RG_NODE_POS_INVALID ;
      }
      else
      {
         UINT32 tmpPos = curPos ;
         if ( selTimes > 0 )
         {
            tmpPos = ( curPos + 1 ) % groupItem->nodeCount() ;
         }

         if ( PREFER_REPL_ANYONE != preferReplicaType &&
              PREFER_REPL_MASTER != preferReplicaType )
         {
            UINT32 pimaryPos = groupItem->getPrimaryPos() ;

            if ( CLS_RG_NODE_POS_INVALID != pimaryPos &&
                 selTimes + 1 == groupItem->nodeCount() )
            {
               tmpPos = pimaryPos ;
            }
            else if ( tmpPos == pimaryPos )
            {
               tmpPos = ( tmpPos + 1 ) % groupItem->nodeCount() ;
            }
         }

         curPos = tmpPos ;
         ++selTimes ;
      }
   }

   INT32 rtnCoordSendRequestToOne( CHAR *pBuffer,
                                   CoordGroupInfoPtr &groupInfo,
                                   REQUESTID_MAP &sendNodes,
                                   netMultiRouteAgent *pRouteAgent,
                                   MSG_ROUTE_SERVICE_TYPE type,
                                   pmdEDUCB *cb,
                                   BOOLEAN isResend )
   {
      return _rtnCoordSendRequestToOne( ( MsgHeader*) pBuffer, groupInfo,
                                         sendNodes, pRouteAgent, type,
                                         cb, NULL, isResend ) ;
   }

   INT32 rtnCoordSendRequestToOne( MsgHeader *pBuffer,
                                   CoordGroupInfoPtr &groupInfo,
                                   REQUESTID_MAP &sendNodes,
                                   netMultiRouteAgent *pRouteAgent,
                                   const netIOVec &iov,
                                   MSG_ROUTE_SERVICE_TYPE type,
                                   pmdEDUCB *cb,
                                   BOOLEAN isResend )
   {
      return _rtnCoordSendRequestToOne( pBuffer, groupInfo, sendNodes,
                                        pRouteAgent, type, cb,
                                        &iov, isResend ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOPROCESSGETGROUPREPLY, "rtnCoordProcessGetGroupReply" )
   INT32 rtnCoordProcessGetGroupReply ( MsgHeader *pReply,
                                        CoordGroupInfoPtr &groupInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOPROCESSGETGROUPREPLY ) ;
      UINT32 headerLen = MSG_GET_INNER_REPLY_HEADER_LEN(pReply) ;

      do
      {
         if ( SDB_OK == MSG_GET_INNER_REPLY_RC(pReply) &&
              (UINT32)pReply->messageLength >= headerLen + 5 )
         {
            try
            {
               BSONObj boGroupInfo( MSG_GET_INNER_REPLY_DATA(pReply) );
               BSONElement beGroupID = boGroupInfo.getField( CAT_GROUPID_NAME );
               if ( beGroupID.eoo() || !beGroupID.isNumber() )
               {
                  rc = SDB_INVALIDARG;
                  PD_LOG ( PDERROR,
                           "Process get-group-info-reply failed,"
                           "failed to get the field(%s)", CAT_GROUPID_NAME );
                  break;
               }
               CoordGroupInfo *pGroupInfo
                           = SDB_OSS_NEW CoordGroupInfo( beGroupID.number() );
               if ( NULL == pGroupInfo )
               {
                  rc = SDB_OOM;
                  PD_LOG ( PDERROR, "Process get-group-info-reply failed,"
                           "new failed ");
                  break;
               }
               CoordGroupInfoPtr groupInfoTmp( pGroupInfo );
               rc = groupInfoTmp->fromBSONObj( boGroupInfo );
               if ( rc != SDB_OK )
               {
                  PD_LOG ( PDERROR, "Process get-group-info-reply failed,"
                           "failed to parse the groupInfo(rc=%d)", rc );
                  break;
               }
               groupInfo = groupInfoTmp;
            }
            catch ( std::exception &e )
            {
               rc = SDB_INVALIDARG;
               PD_LOG ( PDERROR, "Process get-group-info-reply failed,"
                        "received unexpected error:%s", e.what() );
               break;
            }
         }
         else
         {
            rc = MSG_GET_INNER_REPLY_RC(pReply) ;
         }
      }while ( FALSE );
      PD_TRACE_EXITRC ( SDB_RTNCOPROCESSGETGROUPREPLY, rc ) ;
      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOPROCESSQUERYCATREPLY, "rtnCoordProcessQueryCatReply" )
   INT32 rtnCoordProcessQueryCatReply ( MsgCatQueryCatRsp *pReply,
                                        CoordCataInfoPtr &cataInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOPROCESSQUERYCATREPLY ) ;

      do
      {
         if ( 0 == pReply->flags )
         {
            try
            {
               BSONObj boCataInfo( (CHAR *)pReply + sizeof(MsgCatQueryCatRsp) );
               BSONElement beName = boCataInfo.getField( CAT_CATALOGNAME_NAME );
               if ( beName.eoo() || beName.type() != String )
               {
                  rc = SDB_INVALIDARG;
                  PD_LOG ( PDERROR, "Failed to parse query-catalogue-reply,"
                           "failed to get the field(%s)",
                           CAT_CATALOGNAME_NAME );
                  break;
               }
               BSONElement beVersion = boCataInfo.getField( CAT_CATALOGVERSION_NAME );
               if ( beVersion.eoo() || !beVersion.isNumber() )
               {
                  rc = SDB_INVALIDARG;
                  PD_LOG ( PDERROR, "Failed to parse query-catalogue-reply, "
                           "failed to get the field(%s)",
                           CAT_CATALOGVERSION_NAME );
                  break;
               }

               // the pCataInfoTmp will be deleted by smart-point automatically
               CoordCataInfo *pCataInfoTmp = NULL;
               pCataInfoTmp = SDB_OSS_NEW CoordCataInfo( beVersion.number(),
                                                         beName.str().c_str() );
               if ( NULL == pCataInfoTmp )
               {
                  rc = SDB_OOM;
                  PD_LOG ( PDERROR,
                           "Failed to parse query-catalogue-reply, new failed");
                  break;
               }
               CoordCataInfoPtr cataInfoPtr( pCataInfoTmp );
               rc = cataInfoPtr->fromBSONObj( boCataInfo );
               if ( rc != SDB_OK )
               {
                  PD_LOG ( PDERROR, "Failed to parse query-catalogue-reply, "
                           "parse catalogue info from bson-obj failed(rc=%d)",
                           rc );
                  break;
               }
               PD_LOG ( PDDEBUG, "new catalog info: %s",
                        boCataInfo.toString().c_str() );
               cataInfo = cataInfoPtr ;
            }
            catch ( std::exception &e )
            {
               rc = SDB_INVALIDARG;
               PD_LOG ( PDERROR, "Failed to parse query-catalogue-reply,"
                        "received unexcepted error:%s", e.what() );
               break;
            }
         }
         else
         {
            
            PD_LOG ( PDWARNING, "Recieve unexcepted reply while query "
                     "catalogue(flag=%d)", pReply->flags ) ;
         }
         rc= pReply->flags ;
      }while ( FALSE ) ;

      PD_TRACE_EXITRC ( SDB_RTNCOPROCESSQUERYCATREPLY, rc ) ;
      return rc;
   }

   INT32 rtnCoordGetAllGroupList( pmdEDUCB * cb, CoordGroupList &groupList,
                                  const BSONObj *query, BOOLEAN exceptCata,
                                  BOOLEAN exceptCoord,
                                  BOOLEAN useLocalWhenFailed )
   {
      INT32 rc = SDB_OK ;
      GROUP_VEC vecGrpPtr ;

      rc = rtnCoordGetAllGroupList( cb, vecGrpPtr, query, exceptCata,
                                    exceptCoord, useLocalWhenFailed ) ;
      if ( rc )
      {
         goto error ;
      }
      else
      {
         GROUP_VEC::iterator it = vecGrpPtr.begin() ;
         while ( it != vecGrpPtr.end() )
         {
            CoordGroupInfoPtr &ptr = *it ;
            groupList[ ptr->getGroupID() ] = ptr->getGroupID() ;
            ++it ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnCoordGetAllGroupList( pmdEDUCB * cb, GROUP_VEC &groupLst,
                                  const BSONObj *query, BOOLEAN exceptCata,
                                  BOOLEAN exceptCoord,
                                  BOOLEAN useLocalWhenFailed )
   {
      INT32 rc = SDB_OK;
      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();
      CoordCB *pCoordcb = pKrcb->getCoordCB();
      INT32 bufferSize = 0;
      rtnCoordCommand *pCmdProcesser = NULL;
      CHAR *pListReq = NULL;
      SINT64 contextID = -1;
      rtnContextBuf buffObj ;

      rtnCoordProcesserFactory *pProcesserFactory
               = pCoordcb->getProcesserFactory();
      pCmdProcesser = pProcesserFactory->getCommandProcesser(
         COORD_CMD_LISTGROUPS ) ;
      SDB_ASSERT( pCmdProcesser, "pCmdProcesser can't be NULL!" );
      rc = msgBuildQueryMsg( &pListReq, &bufferSize, COORD_CMD_LISTGROUPS,
                             0, 0, 0, -1, query ) ;
      PD_RC_CHECK( rc, PDERROR, "failed to build list groups request(rc=%d)",
                   rc );
      rc = pCmdProcesser->execute( (MsgHeader*)pListReq, cb,
                                   contextID, &buffObj ) ;
      PD_RC_CHECK( rc, PDERROR, "failed to list groups(rc=%d)", rc ) ;

      while ( TRUE )
      {
         rc = rtnGetMore( contextID, 1, buffObj, cb, pRtncb ) ;
         if ( rc )
         {
            if ( rc != SDB_DMS_EOC )
            {
               PD_RC_CHECK( rc, PDERROR, "failed to execute getmore(rc=%d)",
                            rc );
            }
            contextID = -1 ;
            rc = SDB_OK ;
            break ;
         }

         try
         {
            CoordGroupInfo *pGroupInfo = NULL ;
            CoordGroupInfoPtr groupInfoTmp ;
            BSONObj boGroupInfo( buffObj.data() ) ;
            BSONElement beGroupID = boGroupInfo.getField( CAT_GROUPID_NAME );
            PD_CHECK( beGroupID.isNumber(), SDB_INVALIDARG, error, PDERROR,
                      "failed to process group info, failed to get the field"
                      "(%s)", CAT_GROUPID_NAME ) ;
            pGroupInfo = SDB_OSS_NEW CoordGroupInfo( beGroupID.number() );
            PD_CHECK( pGroupInfo != NULL, SDB_OOM, error, PDERROR,
                      "malloc failed!" );
            groupInfoTmp = CoordGroupInfoPtr( pGroupInfo );
            rc = groupInfoTmp->fromBSONObj( boGroupInfo );
            PD_RC_CHECK( rc, PDERROR, "failed to parse the group info(rc=%d)",
                         rc ) ;

            if ( groupInfoTmp->getGroupID() == CATALOG_GROUPID )
            {
               if ( !exceptCata )
               {
                  groupLst.push_back( groupInfoTmp ) ;
               }
            }
            else if ( groupInfoTmp->getGroupID() == COORD_GROUPID )
            {
               if ( !exceptCoord )
               {
                  groupLst.push_back( groupInfoTmp ) ;
               }
            }
            else
            {
               groupLst.push_back( groupInfoTmp );
            }
            pCoordcb->addGroupInfo( groupInfoTmp );
            rc = rtnCoordUpdateRoute( groupInfoTmp, pCoordcb->getRouteAgent(),
                                      MSG_ROUTE_SHARD_SERVCIE ) ;
            // update cata service also
            if ( groupInfoTmp->getGroupID() == CATALOG_GROUPID )
            {
               rtnCoordUpdateRoute( groupInfoTmp, pCoordcb->getRouteAgent(),
                                    MSG_ROUTE_CAT_SERVICE ) ;
               pCoordcb->updateCatGroupInfo( groupInfoTmp ) ;
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_SYS ;
            PD_RC_CHECK( rc, PDERROR, "Failed to process group info, received "
                         "unexpected error:%s", e.what() ) ;
         }
      }

   done:
      if ( -1 != contextID )
      {
         pRtncb->contextDelete( contextID, cb );
      }
      SAFE_OSS_FREE( pListReq ) ;
      return rc;
   error:
      if ( useLocalWhenFailed )
      {
         pCoordcb->getLocalGroupList( groupLst, exceptCata, exceptCoord ) ;
         rc = SDB_OK ;
      }
      goto done;
   }

   INT32 rtnCoordSendRequestToNodes( void *pBuffer,
                                     ROUTE_SET &nodes,
                                     netMultiRouteAgent *pRouteAgent,
                                     pmdEDUCB *cb,
                                     REQUESTID_MAP &sendNodes,
                                     ROUTE_RC_MAP &failedNodes )
   {
      INT32 rc = SDB_OK;
      ROUTE_SET::iterator iter = nodes.begin();
      while( iter != nodes.end() )
      {
         MsgRouteID routeID;
         routeID.value = *iter;
         rc = rtnCoordSendRequestToNode( pBuffer, routeID,
                                         pRouteAgent, cb,
                                         sendNodes );
         if ( rc != SDB_OK )
         {
            failedNodes[ *iter ] = rc ;
         }
         ++iter;
      }
      return SDB_OK ;
   }

   INT32 rtnCoordReadALine( const CHAR *&pInput,
                            CHAR *pOutput )
   {
      INT32 rc = SDB_OK;
      while( *pInput != 0x0D && *pInput != 0x0A && *pInput != '\0' )
      {
         *pOutput = *pInput;
         ++pInput;
         ++pOutput;
      }
      *pOutput = '\0';
      return rc;
   }

   void rtnCoordClearRequest( pmdEDUCB *cb, REQUESTID_MAP &sendNodes )
   {
      REQUESTID_MAP::iterator iterMap = sendNodes.begin();
      while( iterMap != sendNodes.end() )
      {
         cb->getCoordSession()->delRequest( iterMap->first );
         sendNodes.erase( iterMap++ );
      }
   }

   INT32 rtnCoordGetSubCLsByGroups( const CoordSubCLlist &subCLList,
                                    const CoordGroupList &sendGroupList,
                                    pmdEDUCB *cb,
                                    CoordGroupSubCLMap &groupSubCLMap,
                                    const BSONObj *query )
   {
      INT32 rc = SDB_OK;
      CoordGroupList::const_iterator iterSend;
      CoordSubCLlist::const_iterator iterCL = subCLList.begin();
      while( iterCL != subCLList.end() )
      {
         CoordCataInfoPtr cataInfo;
         CoordGroupList groupList;
         CoordGroupList::iterator iterGroup;
         rc = rtnCoordGetCataInfo( cb, (*iterCL).c_str(),
                                   FALSE, cataInfo );
         PD_RC_CHECK( rc, PDWARNING, "Failed to get catalog info of "
                      "sub-collection[%s], rc: %d", (*iterCL).c_str(),
                      rc ) ;
         if ( NULL == query || query->isEmpty() )
         {
            cataInfo->getGroupLst( groupList );
         }
         else
         {
            cataInfo->getGroupByMatcher( *query, groupList ) ;
         }
         // SDB_ASSERT( groupList.size() > 0, "group list can't be empty!" );
         iterGroup = groupList.begin();
         while( iterGroup != groupList.end() )
         {
            groupSubCLMap[ iterGroup->first ].push_back( (*iterCL) );
            ++iterGroup;
         }
         ++iterCL;
      }

      iterSend = sendGroupList.begin();
      while( iterSend != sendGroupList.end() )
      {
         groupSubCLMap.erase( iterSend->first ) ;
         ++iterSend;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 rtnCoordParseGroupList( pmdEDUCB *cb,
                                 const BSONObj &obj,
                                 CoordGroupList &groupList,
                                 BSONObj *pNewObj )
   {
      INT32 rc = SDB_OK ;
      CoordGroupInfoPtr grpPtr ;
      BSONObjBuilder builder ;
      BOOLEAN isModify = FALSE ;

      BSONObjIterator it( obj ) ;
      while( it.more() )
      {
         BSONElement ele = it.next() ;

         // group id
         if ( 0 == ossStrcasecmp( ele.fieldName(), CAT_GROUPID_NAME ) )
         {
            isModify = TRUE ;
            if ( ele.type() == NumberInt )
            {
               groupList[(UINT32)ele.numberInt()] = (UINT32)ele.numberInt() ;
            }
            else if ( ele.type() == Array )
            {
               BSONObjIterator tmpItr( ele.embeddedObject() ) ;
               while ( tmpItr.more() )
               {
                  BSONElement e1 = tmpItr.next () ;
                  if ( e1.type() != NumberInt )
                  {
                     rc = SDB_INVALIDARG ;
                     goto error ;
                  }
                  groupList[(UINT32)e1.numberInt()] = (UINT32)e1.numberInt() ;
               }
            }
            else
            {
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
         // group name
         else if ( 0 == ossStrcasecmp( ele.fieldName(),
                                       FIELD_NAME_GROUPNAME ) ||
                   0 == ossStrcasecmp( ele.fieldName(),
                                       FIELD_NAME_GROUPS ) )
         {
            isModify = TRUE ;
            if ( ele.type() == String )
            {
               rc = rtnCoordGetGroupInfo( cb, ele.str().c_str(),
                                          FALSE, grpPtr ) ;
               PD_RC_CHECK( rc, PDERROR, "Get group[%s] failed, rc: %d",
                            ele.str().c_str(), rc ) ;
               groupList[ grpPtr->getGroupID() ] = grpPtr->getGroupID() ;
            }
            else if ( ele.type() == Array )
            {
               BSONObjIterator tmpItr( ele.embeddedObject() ) ;
               while ( tmpItr.more() )
               {
                  BSONElement e1 = tmpItr.next() ;
                  if ( e1.type() != String )
                  {
                     rc = SDB_INVALIDARG ;
                     goto error ;
                  }
                  rc = rtnCoordGetGroupInfo( cb, e1.str().c_str(),
                                             FALSE, grpPtr ) ;
                  PD_RC_CHECK( rc, PDERROR, "Get group[%s] failed, rc: %d",
                               e1.str().c_str(), rc ) ;
                  groupList[ grpPtr->getGroupID() ] = grpPtr->getGroupID() ;
               }
            }
            else
            {
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
         else if ( pNewObj )
         {
            builder.append( ele ) ;
         }
      }

      if ( pNewObj )
      {
         if ( isModify )
         {
            *pNewObj = builder.obj() ;
         }
         else
         {
            *pNewObj = obj ;
         }
      }

   done:
      return rc ;
   error:
      PD_LOG( PDERROR, "Failed to parse group list from bson[%s], rc: %d",
              obj.toString().c_str(), rc ) ;
      goto done ;
   }

   BSONObj* rtnCoordGetFilterByID( FILTER_BSON_ID filterID,
                                   rtnQueryOptions &queryOption )
   {
      BSONObj *pFilter = NULL ;
      switch ( filterID )
      {
         case FILTER_ID_SELECTOR:
            pFilter = &queryOption._selector ;
            break ;
         case FILTER_ID_ORDERBY:
            pFilter = &queryOption._orderBy ;
            break ;
         case FILTER_ID_HINT:
            pFilter = &queryOption._hint ;
            break ;
         default:
            pFilter = &queryOption._query ;
            break ;
      }
      return pFilter ;
   }

   INT32 rtnCoordParseGroupList( pmdEDUCB *cb, MsgOpQuery *pMsg,
                                 FILTER_BSON_ID filterObjID,
                                 CoordGroupList &groupList )
   {
      INT32 rc = SDB_OK ;
      rtnQueryOptions queryOption ;
      BSONObj *pFilterObj = NULL ;

      rc = queryOption.fromQueryMsg( (CHAR *)pMsg ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract query msg failed, rc: %d", rc ) ;

      pFilterObj = rtnCoordGetFilterByID( filterObjID, queryOption ) ;
      try
      {
         if ( !pFilterObj->isEmpty() )
         {
            rc = rtnCoordParseGroupList( cb, *pFilterObj, groupList ) ;
            if ( rc )
            {
               goto error ;
            }
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnGroupList2GroupPtr( pmdEDUCB *cb, CoordGroupList &groupList,
                                CoordGroupMap &groupMap,
                                BOOLEAN reNew )
   {
      INT32 rc = SDB_OK ;
      CoordGroupInfoPtr ptr ;

      if ( reNew )
      {
         groupMap.clear() ;
      }

      CoordGroupList::iterator it = groupList.begin() ;
      while ( it != groupList.end() )
      {
         if ( !reNew && groupMap.end() != groupMap.find( it->second ) )
         {
            // alredy exist, don't update group info
            ++it ;
            continue ;
         }
         rc = rtnCoordGetGroupInfo( cb, it->second, FALSE, ptr ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get group[%d] info, rc: %d",
                      it->second, rc ) ;
         groupMap[ it->second ] = ptr ;
         ++it ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnGroupList2GroupPtr( pmdEDUCB * cb, CoordGroupList & groupList,
                                GROUP_VEC & groupPtrs )
   {
      INT32 rc = SDB_OK ;
      groupPtrs.clear() ;
      CoordGroupInfoPtr ptr ;

      CoordGroupList::iterator it = groupList.begin() ;
      while ( it != groupList.end() )
      {
         rc = rtnCoordGetGroupInfo( cb, it->second, FALSE, ptr ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get group[%d] info, rc: %d",
                      it->second, rc ) ;
         groupPtrs.push_back( ptr ) ;
         ++it ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnGroupPtr2GroupList( pmdEDUCB * cb, GROUP_VEC & groupPtrs,
                                CoordGroupList & groupList )
   {
      groupList.clear() ;

      GROUP_VEC::iterator it = groupPtrs.begin() ;
      while ( it != groupPtrs.end() )
      {
         CoordGroupInfoPtr &ptr = *it ;
         groupList[ ptr->getGroupID() ] = ptr->getGroupID() ;
         ++it ;
      }

      return SDB_OK ;
   }

   INT32 rtnCoordGetGroupNodes( pmdEDUCB *cb, const BSONObj &filterObj,
                                NODE_SEL_STY emptyFilterSel,
                                CoordGroupList &groupList, ROUTE_SET &nodes,
                                BSONObj *pNewObj )
   {
      INT32 rc = SDB_OK ;
      CoordGroupInfoPtr ptr ;
      MsgRouteID routeID ;
      GROUP_VEC groupPtrs ;
      GROUP_VEC::iterator it ;

      vector< UINT16 > vecNodeID ;
      vector< string > vecHostName ;
      vector< string > vecSvcName ;
      BOOLEAN emptyFilter = TRUE ;

      BSONObjBuilder builder ;
      BOOLEAN isModify = FALSE ;

      nodes.clear() ;

      rc = rtnGroupList2GroupPtr( cb, groupList, groupPtrs ) ;
      PD_RC_CHECK( rc, PDERROR, "Group ids to group info failed, rc: %d", rc ) ;

      /// parse obj
      {
         BSONObjIterator itr( filterObj ) ;
         while( itr.more() )
         {
            BSONElement ele = itr.next() ;

            if ( 0 == ossStrcasecmp( ele.fieldName(), CAT_NODEID_NAME ) )
            {
               if ( ele.type() == NumberInt )
               {
                  vecNodeID.push_back( ele.numberInt() ) ;
                  emptyFilter = FALSE ;
               }
               else if ( ele.type() == Array )
               {
                  BSONObjIterator tmpItr( ele.embeddedObject() ) ;
                  while( tmpItr.more() )
                  {
                     BSONElement e1 = tmpItr.next() ;
                     if ( NumberInt != e1.type() )
                     {
                        rc = SDB_INVALIDARG ;
                        goto error ;
                     }
                     vecNodeID.push_back( e1.numberInt() ) ;
                     emptyFilter = FALSE ;
                  }
               }
               else
               {
                  PD_LOG( PDWARNING, "Field[%s] type[%d] error",
                          CAT_NODEID_NAME, ele.type() ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
            }
            else if ( 0 == ossStrcasecmp( ele.fieldName(), FIELD_NAME_HOST ) )
            {
               if ( ele.type() == String )
               {
                  vecHostName.push_back( ele.str() ) ;
                  emptyFilter = FALSE ;
               }
               else if ( ele.type() == Array )
               {
                  BSONObjIterator tmpItr( ele.embeddedObject() ) ;
                  while( tmpItr.more() )
                  {
                     BSONElement e1 = tmpItr.next() ;
                     if ( String != e1.type() )
                     {
                        rc = SDB_INVALIDARG ;
                        goto error ;
                     }
                     vecHostName.push_back( e1.str() ) ;
                     emptyFilter = FALSE ;
                  }
               }
               else
               {
                  PD_LOG( PDWARNING, "Field[%s] type[%d] error",
                          FIELD_NAME_HOST, ele.type() ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
            }
            else if ( 0 == ossStrcasecmp( ele.fieldName(),
                                          FIELD_NAME_SERVICE_NAME ) ||
                      0 == ossStrcasecmp( ele.fieldName(),
                                          PMD_OPTION_SVCNAME ) )
            {
               if ( ele.type() == String )
               {
                  vecSvcName.push_back( ele.str() ) ;
                  emptyFilter = FALSE ;
               }
               else if ( ele.type() == Array )
               {
                  BSONObjIterator tmpItr( ele.embeddedObject() ) ;
                  while( tmpItr.more() )
                  {
                     BSONElement e1 = tmpItr.next() ;
                     if ( String != e1.type() )
                     {
                        rc = SDB_INVALIDARG ;
                        goto error ;
                     }
                     vecSvcName.push_back( e1.str() ) ;
                     emptyFilter = FALSE ;
                  }
               }
               else
               {
                  PD_LOG( PDWARNING, "Field[%s] type[%d] error",
                          PMD_OPTION_SVCNAME, ele.type() ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
            }
            else if ( pNewObj )
            {
               builder.append( ele ) ;
            } // end if
         } // end while
      }

      it = groupPtrs.begin() ;
      while ( it != groupPtrs.end() )
      {
         ptr = *it ;

         routeID.value = MSG_INVALID_ROUTEID ;
         clsGroupItem *grp = ptr->getGroupItem() ;
         routeID.columns.groupID = grp->groupID() ;
         const VEC_NODE_INFO *nodesInfo = grp->getNodes() ;
         for ( VEC_NODE_INFO::const_iterator itrn = nodesInfo->begin() ;
               itrn != nodesInfo->end();
               ++itrn )
         {
            if ( FALSE == emptyFilter )
            {
               BOOLEAN findNode = FALSE ;
               UINT32 index = 0 ;
               /// check node id
               for ( index = 0 ; index < vecNodeID.size() ; ++index )
               {
                  if ( vecNodeID[ index ] == itrn->_id.columns.nodeID )
                  {
                     findNode = TRUE ;
                     break ;
                  }
               }
               if ( index > 0 && !findNode )
               {
                  continue ;
               }

               findNode = FALSE ;
               /// check host name
               for ( index = 0 ; index < vecHostName.size() ; ++index )
               {
                  if ( 0 == vecHostName[ index ].compare(
                            itrn->_service[ MSG_ROUTE_LOCAL_SERVICE ] ) )
                  {
                     findNode = TRUE ;
                     break ;
                  }
               }
               if ( index > 0 && !findNode )
               {
                  continue ;
               }

               findNode = FALSE ;
               /// check svcname
               for ( index = 0 ; index < vecSvcName.size() ; ++index )
               {
                  if ( 0 == vecSvcName[ index ].compare(
                            itrn->_service[MSG_ROUTE_LOCAL_SERVICE] ) )
                  {
                     findNode = TRUE ;
                     break ;
                  }
               }
               if ( index > 0 && !findNode )
               {
                  continue ;
               }
            }
            else
            {
               if ( NODE_SEL_PRIMARY == emptyFilterSel )
               {
                  MsgRouteID primaryNode = ptr->getPrimary() ;
                  if ( 0 == primaryNode.value )
                  {
                     PD_LOG( PDERROR, "Group[%u] has no primary node",
                             grp->groupID() ) ;
                     rc = SDB_RTN_NO_PRIMARY_FOUND ;
                     goto error ;
                  }

                  if ( itrn->_id.columns.nodeID != primaryNode.columns.nodeID )
                  {
                     continue ;
                  }
               }
            }
            routeID.columns.nodeID = itrn->_id.columns.nodeID ;
            routeID.columns.serviceID = MSG_ROUTE_SHARD_SERVCIE ;
            nodes.insert( routeID.value ) ;
         }
         ++it ;
      }

      if ( pNewObj )
      {
         if ( emptyFilter )
         {
            *pNewObj = filterObj ;
         }
         else
         {
            *pNewObj = builder.obj() ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOUPNODESTATBYRC, "rtnCoordUpdateNodeStatByRC" )
   void rtnCoordUpdateNodeStatByRC( pmdEDUCB *cb,
                                    const MsgRouteID &routeID,
                                    CoordGroupInfoPtr &groupInfo,
                                    INT32 retCode )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOUPNODESTATBYRC ) ;

      CoordSession *pSession = cb->getCoordSession() ;
      NET_NODE_STATUS status = NET_NODE_STAT_NORMAL;
      if ( MSG_INVALID_ROUTEID == routeID.value )
      {
         goto done;
      }
      else if ( SDB_OK != rc && pSession )
      {
         pSession->removeLastNode( routeID.columns.groupID,
                                   routeID ) ;
      }

      switch ( retCode )
      {
         case SDB_CLS_FULL_SYNC:
            status = NET_NODE_STAT_FULLSYNC ;
            break ;
         case SDB_NETWORK:
         case SDB_NETWORK_CLOSE:
         case SDB_NET_CANNOT_CONNECT:
         case SDB_COORD_REMOTE_DISC:
            status = NET_NODE_STAT_OFFLINE ;
            break ;
         default:
            status = NET_NODE_STAT_NORMAL ;
            break ;
      }

      if( groupInfo.get() )
      {
         groupInfo->updateNodeStat( routeID.columns.nodeID,
                                    status ) ;
      }

   done:
      PD_TRACE_EXIT ( SDB_RTNCOUPNODESTATBYRC ) ;
      return ;
   }

   BOOLEAN rtnCoordGroupReplyCheck( pmdEDUCB *cb, INT32 flag,
                                    BOOLEAN canRetry,
                                    const MsgRouteID &nodeID,
                                    CoordGroupInfoPtr &groupInfo,
                                    BOOLEAN *pUpdate,
                                    BOOLEAN canUpdate,
                                    UINT32 primaryID )
   {
      BOOLEAN primaryExist = FALSE ;
      if ( SDB_CLS_NOT_PRIMARY == flag && 0 != primaryID )
      {
         primaryExist = TRUE ;
         // update primary
         if ( groupInfo.get() )
         {
            MsgRouteID primaryNodeID ;
            primaryNodeID.value = nodeID.value ;
            primaryNodeID.columns.nodeID = primaryID ;
            if ( SDB_OK == groupInfo->setPrimary( primaryNodeID ) )
            {
               return TRUE ;
            }
         }
      }
      else if ( !canRetry )
      {
         return FALSE ;
      }

      if ( SDB_CLS_NOT_PRIMARY == flag || primaryExist )
      {
         if ( groupInfo.get() )
         {
            groupInfo->setSlave( nodeID ) ;
         }

         if ( canUpdate )
         {
            UINT32 groupID = nodeID.columns.groupID ;
            INT32 rc = rtnCoordGetRemoteGroupInfo( cb, groupID, NULL,
                                                   groupInfo, TRUE ) ;
            if ( SDB_OK == rc )
            {
               if( pUpdate )
               {
                  *pUpdate = TRUE ;
               }
            }
            else
            {
               PD_LOG( PDWARNING, "Get group info[%u] from remote failed, "
                       "rc: %d", groupID, rc ) ;
               return FALSE ;
            }
         }
         return TRUE ;
      }
      // [SDB_COORD_REMOTE_DISC] can't use, because when some insert/update
      // opr do partibal, if retry, data will repeat. If we can't do from
      // break, we can use the flag for retry
      else if ( SDB_CLS_FULL_SYNC == flag )
      {
         // don't update group info
         rtnCoordUpdateNodeStatByRC( cb, nodeID, groupInfo, flag ) ;
         return TRUE ;
      }
      else if ( SDB_CLS_NODE_NOT_ENOUGH == flag )
      {
         // do nothing, retry once
         return TRUE ;
      }

      return FALSE ;
   }

   BOOLEAN rtnCoordCataReplyCheck( pmdEDUCB *cb, INT32 flag,
                                   BOOLEAN canRetry,
                                   CoordCataInfoPtr &cataInfo,
                                   BOOLEAN *pUpdate,
                                   BOOLEAN canUpdate )
   {
      if ( canRetry && (
           SDB_CLS_COORD_NODE_CAT_VER_OLD == flag ||
           SDB_CLS_NO_CATALOG_INFO == flag ||
           SDB_CLS_GRP_NOT_EXIST == flag ||
           SDB_CLS_NODE_NOT_EXIST == flag ||
           SDB_CAT_NO_MATCH_CATALOG == flag )
          )
      {
         if ( canUpdate && cataInfo.get() )
         {
            CoordCataInfoPtr newCataInfo ;
            const CHAR *collectionName = cataInfo->getName() ;
            INT32 rc = rtnCoordGetRemoteCata( cb, collectionName,
                                              newCataInfo ) ;
            if ( SDB_OK == rc )
            {
               cataInfo = newCataInfo ;
               if( pUpdate )
               {
                  *pUpdate = TRUE ;
               }
            }
            else
            {
               PD_LOG( PDWARNING, "Get catalog info[%s] from remote failed, "
                       "rc: %d", collectionName, rc ) ;
               return FALSE ;
            }
         }
         return TRUE ;
      }

      return FALSE ;
   }

   INT32 rtnCataChangeNtyToAllNodes( pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      netMultiRouteAgent *pRouteAgent = sdbGetCoordCB()->getRouteAgent() ;
      MsgHeader ntyMsg ;
      ntyMsg.messageLength = sizeof( MsgHeader ) ;
      ntyMsg.opCode = MSG_CAT_GRP_CHANGE_NTY ;
      ntyMsg.requestID = 0 ;
      ntyMsg.routeID.value = 0 ;
      ntyMsg.TID = cb->getTID() ;

      CoordGroupList groupLst ;
      ROUTE_SET sendNodes ;
      REQUESTID_MAP successNodes ;
      ROUTE_RC_MAP failedNodes ;

      // list all groups
      rc = rtnCoordGetAllGroupList( cb, groupLst, NULL, FALSE ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get all group list, rc: %d", rc ) ;

      // get nodes
      rc = rtnCoordGetGroupNodes( cb, BSONObj(), NODE_SEL_ALL,
                                  groupLst, sendNodes ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get nodes, rc: %d", rc ) ;
      if ( sendNodes.size() == 0 )
      {
         PD_LOG( PDWARNING, "Not found any node" ) ;
         rc = SDB_CLS_NODE_NOT_EXIST ;
         goto error ;
      }

      // send msg, no response
      rtnCoordSendRequestToNodes( (void*)&ntyMsg, sendNodes, 
                                  pRouteAgent, cb, successNodes,
                                  failedNodes ) ;
      if ( failedNodes.size() != 0 )
      {
         rc = failedNodes.begin()->second ;
      }
      rtnCoordClearRequest( cb, successNodes ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnCoordParseControlParam( const BSONObj &obj,
                                    rtnCoordCtrlParam &param,
                                    UINT32 mask,
                                    BSONObj *pNewObj )
   {
      INT32 rc = SDB_OK ;

      BOOLEAN modify = FALSE ;
      BSONObjBuilder builder ;
      BSONObjIterator it( obj ) ;
      while( it.more() )
      {
         BSONElement e = it.next() ;
         if ( ( mask & RTN_CTRL_MASK_GLOBAL ) &&
              0 == ossStrcasecmp( e.fieldName(), FIELD_NAME_GLOBAL ) )
         {
            modify = TRUE ;
            param._parseMask |= RTN_CTRL_MASK_GLOBAL ;
            if ( !e.isBoolean() )
            {
               PD_LOG( PDERROR, "Field[%s] is invalid in obj[%s]",
                       FIELD_NAME_GLOBAL, obj.toString().c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            param._isGlobal = e.boolean() ? TRUE : FALSE ;
         }
         else if ( ( mask & RTN_CTRL_MASK_GLOBAL ) &&
                   e.isNull() &&
                   ( 0 == ossStrcasecmp( e.fieldName(),
                                        FIELD_NAME_GROUPS ) ||
                     0 == ossStrcasecmp( e.fieldName(),
                                        FIELD_NAME_GROUPNAME ) ||
                     0 == ossStrcasecmp( e.fieldName(),
                                        FIELD_NAME_GROUPID ) )
                 )
         {
            param._isGlobal = FALSE ;
            modify = TRUE ;
            param._parseMask |= RTN_CTRL_MASK_GLOBAL ;
         }
         else if ( ( mask & RTN_CTRL_MASK_NODE_SELECT ) &&
                   0 == ossStrcasecmp( e.fieldName(),
                                       FIELD_NAME_NODE_SELECT ) )
         {
            modify = TRUE ;
            param._parseMask |= RTN_CTRL_MASK_NODE_SELECT ;
            if ( String != e.type() ||
                 ( 0 != ossStrcasecmp( e.valuestr(), "all" ) &&
                   0 != ossStrcasecmp( e.valuestr(), "primary" ) ) )
            {
               PD_LOG( PDERROR, "Field[%s] is invalid in obj[%s]",
                       FIELD_NAME_NODE_SELECT, obj.toString().c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            else if ( 0 == ossStrcasecmp( e.valuestr(), "all" ) )
            {
               param._emptyFilterSel = NODE_SEL_ALL ;
            }
            else
            {
               param._emptyFilterSel = NODE_SEL_PRIMARY ;
            }
         }
         else if ( ( mask & RTN_CTRL_MASK_ROLE ) &&
                   0 == ossStrcasecmp( e.fieldName(),
                                       FIELD_NAME_ROLE ) )
         {
            ossMemset( &param._role, 0, sizeof( param._role ) ) ;
            modify = TRUE ;
            param._parseMask |= RTN_CTRL_MASK_NODE_SELECT ;
            if ( String == e.type() &&
                 SDB_ROLE_MAX != utilGetRoleEnum( e.valuestr() ) )
            {
               param._role[ utilGetRoleEnum( e.valuestr() ) ] = 1 ;
            }
            else if ( Array == e.type() )
            {
               BSONObjIterator tmpItr( e.embeddedObject() ) ;
               while( tmpItr.more() )
               {
                  BSONElement tmpE = tmpItr.next() ;
                  if ( String == e.type() &&
                       SDB_ROLE_MAX != utilGetRoleEnum( tmpE.valuestr() ) )
                  {
                     param._role[ utilGetRoleEnum( tmpE.valuestr() ) ] = 1 ;
                  }
                  else
                  {
                     PD_LOG( PDERROR, "Field[%s] is invalid in obj[%s]",
                             FIELD_NAME_ROLE, obj.toString().c_str() ) ;
                     rc = SDB_INVALIDARG ;
                     goto error ;
                  }
               }
            }
            else
            {
               PD_LOG( PDERROR, "Field[%s] is invalid in obj[%s]",
                       FIELD_NAME_ROLE, obj.toString().c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
         else if ( ( mask & RTN_CTRL_MASK_RAWDATA ) &&
                   0 == ossStrcasecmp( e.fieldName(), FIELD_NAME_RAWDATA ) )
         {
            modify = TRUE ;
            param._parseMask |= RTN_CTRL_MASK_RAWDATA ;
            if ( !e.isBoolean() )
            {
               PD_LOG( PDERROR, "Field[%s] is invalid in obj[%s]",
                       FIELD_NAME_RAWDATA, obj.toString().c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            param._rawData = e.Bool() ? TRUE : FALSE ;
         }
         else if ( pNewObj )
         {
            builder.append( e ) ;
         }
      }

      if ( pNewObj )
      {
         if ( modify )
         {
            *pNewObj = builder.obj() ;
         }
         else
         {
            *pNewObj = obj ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

}

