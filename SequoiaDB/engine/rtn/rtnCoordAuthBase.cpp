/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnCoordAuthBase.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnCoordAuthBase.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "msgMessage.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "rtnCoordCommon.hpp"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_CLEARQ, "clearQ" )
   static void clearQ( REPLY_QUE &queue )
   {
      PD_TRACE_ENTRY ( SDB_CLEARQ ) ;
      while ( !queue.empty() )
      {
         MsgInternalReplyHeader *tmp = ( MsgInternalReplyHeader * )
                                       (queue.front()) ;
         queue.pop();
         SDB_OSS_FREE ( tmp );
      }
      PD_TRACE_EXIT ( SDB_CLEARQ ) ;
      return ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOAUTHBASE_FORWARD, "rtnCoordAuthBase::forward" )
   INT32 rtnCoordAuthBase::forward( CHAR *pReceiveBuffer, SINT32 packSize,
                                CHAR **ppResultBuffer, pmdEDUCB *cb,
                                MsgOpReply &replyHeader, INT32 msgType,
                                BOOLEAN sWhenNoPrimary )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOAUTHBASE_FORWARD ) ;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      MsgHeader *header = (MsgHeader *)pReceiveBuffer ;
      header->routeID.value = 0 ;
      header->TID = cb->getTID() ;
      CoordGroupInfoPtr cata ;
      REQUESTID_MAP nodes ;
      REPLY_QUE replyQue ;
      NodeID curNodeID = pKrcb->getClsCB()->getNodeID() ;
      rc = rtnCoordGetCatGroupInfo( cb, FALSE, cata ) ;
      PD_RC_CHECK ( rc, PDWARNING,
                    "Failed to get catalog group info, rc = %d", rc  ) ;
      rc = rtnCoordSendRequestToPrimary( pReceiveBuffer,
                                 cata, nodes,
                                 pRouteAgent,
                                 MSG_ROUTE_CAT_SERVICE,
                                 cb ) ;
      if ( SDB_OK != rc )
      {
         rc = rtnCoordGetCatGroupInfo( cb, TRUE, cata ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "failed to get catagroup info, rc = %d", rc ) ;
         nodes.clear() ;
         rc = rtnCoordSendRequestToPrimary( pReceiveBuffer,
                                    cata, nodes,
                                    pRouteAgent,
                                    MSG_ROUTE_CAT_SERVICE,
                                    cb ) ;
         if ( SDB_OK != rc )
         {
            if ( sWhenNoPrimary )
            {
               rc = rtnCoordSendRequestToOne( pReceiveBuffer, cata,
                                      nodes, pRouteAgent,
                                      MSG_ROUTE_CAT_SERVICE,
                                      cb ) ;
               PD_RC_CHECK ( rc, PDERROR,
                             "can not find a available cata node, rc = %d",
                             rc ) ;
            }
            else
            {
               PD_RC_CHECK ( rc, PDERROR,
                             "cannot find the priamry, rc = %d", rc ) ;
            }
         }
      }

      rc = rtnCoordGetReply( cb, nodes, replyQue, msgType );
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to get reply from catalog for auth, rc = %d", rc ) ;

      if ( !replyQue.empty() )
      {
         MsgInternalReplyHeader *res = ( MsgInternalReplyHeader * )
                                       (replyQue.front()) ;
         if ( SDB_CLS_NOT_PRIMARY == rc )
         {
            clearQ( replyQue ) ;
            nodes.clear() ;
            rc = rtnCoordGetCatGroupInfo( cb, TRUE, cata ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to get catalog group info, rc = %d", rc ) ;
            rc = rtnCoordSendRequestToPrimary( pReceiveBuffer,
                                       cata, nodes,
                                       pRouteAgent,
                                       MSG_ROUTE_CAT_SERVICE,
                                       cb ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to send request to primary, rc = %d", rc ) ;
            rc = rtnCoordGetReply( cb, nodes, replyQue, msgType );
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to get reply from primary, rc = %d", rc ) ;
            if ( !replyQue.empty() )
            {
               MsgInternalReplyHeader *r = ( MsgInternalReplyHeader * )
                                          (replyQue.front()) ;
               rc = r->res ;
            }
            else
            {
               PD_RC_CHECK ( SDB_SYS, PDERROR, "Empty reply is received" ) ;
            }
         }
         else
         {
            rc = res->res ;
         }
      }
      else
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR, "Empty reply is received" ) ;
      }
      if ( SDB_OK != rc )
      {
         goto error ;
      }
    done:
      msgBuildReplyMsgHeader( replyHeader,
                              sizeof(replyHeader),
                              header->opCode,
                              rc,
                              -1, 0, 0,
                              curNodeID,
                              header->requestID ) ;
      clearQ( replyQue ) ;
      PD_TRACE_EXITRC ( SDB_RTNCOAUTHBASE_FORWARD, rc ) ;
      return rc ;
   error:
      rtnCoordClearRequest( cb, nodes );
      goto done ;
   }
}
