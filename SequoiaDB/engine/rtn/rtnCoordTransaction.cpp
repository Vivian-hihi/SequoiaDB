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

   Source File Name = rtnCoordTransaction.cpp

   Descriptive Name = Runtime Coord Transaction

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   transaction management on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "rtnCoordTransaction.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "msgMessage.hpp"

namespace engine
{
   INT32 rtnCoordTransBegin::execute( CHAR * pReceiveBuffer,
                                      SINT32 packSize,
                                      pmdEDUCB * cb,
                                      MsgOpReply & replyHeader,
                                      rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;

      MsgHeader *pHeader               = (MsgHeader *)pReceiveBuffer ;
      replyHeader.header.messageLength = sizeof( MsgOpReply ) ;
      replyHeader.header.opCode        = MSG_BS_TRANS_BEGIN_RSP ;
      replyHeader.header.requestID     = pHeader->requestID ;
      replyHeader.header.routeID.value = 0 ;
      replyHeader.header.TID           = pHeader->TID ;
      replyHeader.contextID            = -1 ;
      replyHeader.flags                = SDB_OK ;
      replyHeader.numReturned          = 0 ;
      replyHeader.startFrom            = 0 ;
      rc = cb->createTransaction() ;
      PD_RC_CHECK( rc, PDERROR,
                  "create transaction failed(rc=%d)",
                  rc );
   done:
      return rc;
   error:
      replyHeader.flags = rc;
      goto done;
   }

   INT32 rtnCoord2PhaseCommit::execute( CHAR * pReceiveBuffer,
                                        SINT32 packSize,
                                        pmdEDUCB * cb,
                                        MsgOpReply &replyHeader,
                                        rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK;
      INT32 rcTmp = SDB_OK;

      MsgHeader *pHeader               = (MsgHeader *)pReceiveBuffer;
      replyHeader.header.messageLength = sizeof( MsgOpReply );
      replyHeader.header.opCode        = MSG_BS_TRANS_COMMIT_RSP;
      replyHeader.header.requestID     = pHeader->requestID;
      replyHeader.header.routeID.value = 0;
      replyHeader.header.TID           = pHeader->TID;
      replyHeader.contextID            = -1;
      replyHeader.flags                = SDB_OK;
      replyHeader.numReturned          = 0;
      replyHeader.startFrom            = 0;

      if ( !cb->isTransaction() )
      {
         rc = SDB_DPS_TRANS_NO_TRANS ;
         goto error;
      }

      rc = doPhase1( pReceiveBuffer, packSize, cb, replyHeader );
      PD_CHECK( SDB_OK == rc, rc, errorcancel, PDERROR,
                "Execute failed on phase1(rc=%d)", rc ) ;

      rc = doPhase2( pReceiveBuffer, packSize, cb, replyHeader );
      PD_RC_CHECK( rc, PDERROR,
                   "Execute failed on phase2(rc=%d)", rc ) ;

   done:
      return rc ;
   errorcancel:
      rcTmp = cancelOp( pReceiveBuffer, packSize, cb, replyHeader );
      if ( rcTmp )
      {
         PD_LOG ( PDERROR, "Failed to cancel the operate, rc: %d",
                  rcTmp );
      }
   error:
      replyHeader.flags = rc ;
      goto done ;
   }

   INT32 rtnCoord2PhaseCommit::doPhase1( CHAR * pReceiveBuffer,
                                         SINT32 packSize,
                                         pmdEDUCB * cb,
                                         MsgOpReply & replyHeader )
   {
      INT32 rc = SDB_OK;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      CHAR *pMsgReq                    = NULL;
      MsgHeader *pMsgHead              = NULL;
      CoordGroupList groupLst;
      CoordGroupList sendGroupLst;

      rc = buildPhase1Msg( pReceiveBuffer, &pMsgReq );
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to build the message on phase1(rc=%d)",
                   rc );

      pMsgHead = (MsgHeader *)pMsgReq;
      pMsgHead->TID = cb->getTID();

      // execute on data nodes
      rc = executeOnDataGroup( pMsgReq, pRouteAgent, cb ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to execute on data-group on phase1(rc=%d)",
                   rc ) ;

   done:
      if ( pMsgReq )
      {
         SDB_OSS_FREE( pMsgReq );
         pMsgReq = NULL;
      }
      return rc;
   error:
      goto done;
   }

   INT32 rtnCoord2PhaseCommit::doPhase2( CHAR * pReceiveBuffer,
                                         SINT32 packSize,
                                         pmdEDUCB * cb,
                                         MsgOpReply & replyHeader )
   {
      INT32 rc = SDB_OK;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      CHAR *pMsgReq                    = NULL;
      MsgHeader *pMsgHead              = NULL;
      CoordGroupList groupLst;
      CoordGroupList sendGroupLst;

      rc = buildPhase2Msg( pReceiveBuffer, &pMsgReq );
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to build the message on phase1(rc=%d)",
                   rc ) ;

      pMsgHead = (MsgHeader *)pMsgReq;
      pMsgHead->TID = cb->getTID();

      // execute on data nodes
      rc = executeOnDataGroup( pMsgReq, pRouteAgent, cb );
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to execute on data-group on phase1(rc=%d)",
                   rc ) ;

   done:
      if ( pMsgReq )
      {
         SDB_OSS_FREE( pMsgReq );
         pMsgReq = NULL;
      }
      return rc;
   error:
      goto done;
   }

   INT32 rtnCoord2PhaseCommit::cancelOp( CHAR * pReceiveBuffer,
                                         SINT32 packSize,
                                         pmdEDUCB * cb,
                                         MsgOpReply & replyHeader )
   {
      // do nothing, rollback will do in session
      return SDB_OK ;
   }

   INT32 rtnCoordTransCommit::executeOnDataGroup( CHAR * pMsg,
                                                  netMultiRouteAgent * pRouteAgent,
                                                  pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      INT32 rcTmp = SDB_OK ;
      REQUESTID_MAP requestIdMap ;
      REPLY_QUE replyQue;
      MsgHeader *pMsgHead = (MsgHeader *)pMsg;
      DpsTransNodeMap *pNodeMap = cb->getTransNodeLst();
      DpsTransNodeMap::iterator iterMap = pNodeMap->begin();

      while( iterMap != pNodeMap->end() )
      {
         rcTmp = rtnCoordSendRequestToNode( (void *)pMsg, iterMap->second,
                                            pRouteAgent, cb, requestIdMap ) ;
         if ( rcTmp )
         {
            rc = rc ? rc : rcTmp ;
            PD_LOG ( PDWARNING, "Failed to send commit request to the "
                     "node[%s], rc: %d",
                     routeID2String( iterMap->second ).c_str(),
                     rcTmp ) ;
         }
         ++iterMap ;
      }

      rcTmp = rtnCoordGetReply( cb, requestIdMap, replyQue,
                                MAKE_REPLY_TYPE( pMsgHead->opCode ) ) ;
      if ( rcTmp )
      {
         rc = rc ? rc : rcTmp ;
         PD_LOG( PDERROR, "Failed to get the reply, rc: %d", rcTmp ) ;
      }

      while ( !replyQue.empty() )
      {
         MsgOpReply *pReply = NULL;
         pReply = (MsgOpReply *)(replyQue.front());
         replyQue.pop();
         rcTmp = pReply->flags ;

         if ( rcTmp != SDB_OK )
         {
            rc = rc ? rc : rcTmp;
            PD_LOG( PDERROR, "Data node[%s] commit transaction failed, rc: %d",
                    routeID2String( pReply->header.routeID ).c_str(),
                    rcTmp ) ;
         }
         SDB_OSS_FREE( pReply ) ;
      }

      if ( rc )
      {
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 rtnCoordTransCommit::buildPhase1Msg( CHAR * pReceiveBuffer, CHAR **pMsg )
   {
      SDB_ASSERT( pMsg, "pMsg can't be NULL" ) ;
      INT32 bufferSize = 0;
      if ( *pMsg != NULL )
      {
         SDB_OSS_FREE( pMsg );
         *pMsg = NULL;
      }
      return msgBuildTransCommitPreMsg( pMsg, &bufferSize );
   }

   INT32 rtnCoordTransCommit::buildPhase2Msg( CHAR * pReceiveBuffer, CHAR **pMsg )
   {
      SDB_ASSERT( pMsg, "pMsg can't be NULL" ) ;
      INT32 bufferSize = 0;
      if ( *pMsg != NULL )
      {
         SDB_OSS_FREE( pMsg );
         *pMsg = NULL;
      }
      return msgBuildTransCommitMsg( pMsg, &bufferSize );
   }

   INT32 rtnCoordTransCommit::execute( CHAR * pReceiveBuffer,
                                       SINT32 packSize,
                                       pmdEDUCB * cb,
                                       MsgOpReply & replyHeader,
                                       rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;

      rc = rtnCoord2PhaseCommit::execute( pReceiveBuffer, packSize, 
                                          cb, replyHeader, NULL );
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to commit the transaction(rc=%d)",
                   rc ) ;
      // complete, delete transaction
      cb->delTransaction() ;
   done:
      return rc;
   error:
      // rollback in session
      goto done;
   }

   BOOLEAN rtnCoordTransCommit::needRollback() const
   {
      return TRUE ;
   }

   INT32 rtnCoordTransRollback::execute( CHAR * pReceiveBuffer,
                                         SINT32 packSize,
                                         pmdEDUCB * cb,
                                         MsgOpReply & replyHeader,
                                         rtnContextBuf *buf )
   {
      INT32 rc                         = SDB_OK;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();

      MsgHeader *pHeader               = (MsgHeader *)pReceiveBuffer ;
      replyHeader.header.messageLength = sizeof( MsgOpReply ) ;
      replyHeader.header.opCode        = MSG_BS_TRANS_ROLLBACK_RSP ;
      replyHeader.header.routeID.value = 0 ;
      replyHeader.contextID            = -1 ;
      replyHeader.flags                = SDB_OK ;
      replyHeader.numReturned          = 0 ;
      replyHeader.startFrom            = 0 ;

      if ( pHeader )
      {
         replyHeader.header.requestID     = pHeader->requestID;
         replyHeader.header.TID           = pHeader->TID;
      }

      if ( !cb->isTransaction() )
      {
         rc = SDB_DPS_TRANS_NO_TRANS ;
         goto error;
      }

      rc = rtnCoordTransOperator::rollBack( cb, pRouteAgent ) ;
      PD_RC_CHECK( rc, PDERROR, "Rollback transaction failed, rc: %d", rc ) ;

   done:
      return rc ;
   error:
      replyHeader.flags = rc ;
      goto done;
   }

   void rtnCoordTransRollback::_prepareForTrans( pmdEDUCB *cb,
                                                 MsgHeader *pMsg )
   {
   }

   BOOLEAN rtnCoordTransRollback::needRollback() const
   {
      return FALSE ;
   }

}
