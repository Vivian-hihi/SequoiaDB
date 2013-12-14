
#include "rtnCoordInterrupt.hpp"
#include "coordSession.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOINTERRUPT_EXECUTE, "rtnCoordInterrupt::execute" )
   INT32 rtnCoordInterrupt::execute( CHAR * pReceiveBuffer, SINT32 packSize,
                        CHAR * * ppResultBuffer, pmdEDUCB * cb,
                        MsgOpReply & replyHeader,
                        BSONObj **ppErrorObj )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOINTERRUPT_EXECUTE ) ;
      MsgHeader *pInterrupt = (MsgHeader *)pReceiveBuffer;
      replyHeader.header.messageLength = sizeof( MsgOpReply );
      replyHeader.header.opCode = MSG_BS_INTERRUPTE;
      replyHeader.header.TID = pInterrupt->TID;
      replyHeader.header.routeID.value = 0;
      replyHeader.header.requestID = pInterrupt->requestID;
      replyHeader.flags = SDB_OK ;
      replyHeader.startFrom = 0;
      replyHeader.numReturned = 0;

      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();

      // delete all opened contexts when received the interrupt message
      {
         SINT64 contextID = -1 ;
         while ( -1 != (contextID = cb->contextPeek() ))
            pRtncb->contextDelete( contextID, NULL ) ;
      }

      // send interrut message to all sub-session
      CoordSession *pSession = cb->getCoordSession();
      if ( pSession )
      {
         ROUTE_SET routeSet;
         pSession->getAllSessionRoute( routeSet );
         SendInterrupt( cb, routeSet );
      }
      PD_TRACE_EXITRC ( SDB_RTNCOINTERRUPT_EXECUTE, rc ) ;
      return rc;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOINTERRUPT_SENDINTER, "rtnCoordInterrupt::SendInterrupt" )
   void rtnCoordInterrupt::SendInterrupt( pmdEDUCB *cb, ROUTE_SET &routeSet )
   {
      PD_TRACE_ENTRY ( SDB_RTNCOINTERRUPT_SENDINTER ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      netMultiRouteAgent *pRouteAgent = krcb->getCoordCB()->getRouteAgent();

      MsgHeader interruptMsg;
      interruptMsg.messageLength = sizeof( MsgHeader );
      interruptMsg.opCode = MSG_BS_INTERRUPTE;
      interruptMsg.TID = cb->getTID();
      interruptMsg.routeID.value = 0;

      rtnCoordSendRequestToNodesWithOutReply((void *)(&interruptMsg), routeSet,
                                             pRouteAgent );
      PD_TRACE_EXIT ( SDB_RTNCOINTERRUPT_SENDINTER ) ;
   }
}
