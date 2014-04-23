
#include "core.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pd.hpp"
#include "catNodeManager.hpp"
#include "catDef.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{

   PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCATNODEMGRENTPNT, "pmdCatNodeManagerEntryPoint" )
   INT32 pmdCatNodeManagerEntryPoint( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_PMDCATNODEMGRENTPNT );
      pmdEDUMgr *eduMgr   = pmdGetKRCB()->getEDUMgr();
      catNodeManager nodeMgr;
      rc = nodeMgr.init( cb );
      if ( rc != SDB_OK )
      {
         PD_LOG( PDEVENT,
                     "Cat-node-manager init failed(rc = %d)", rc );
         return rc;
      }
      eduMgr->regSystemEDU ( EDU_TYPE_CATNODEMANAGER, cb->getID()) ;

      //loop:process event
      while ( !cb->isDisconnected() )
      {
         pmdEDUEvent event;
         if ( cb->waitEvent(event, MILLSEC_1000) )
         {
            if ( PMD_EDU_EVENT_TERM == event._eventType )
            {
               PD_LOG ( PDEVENT, "EDU[%lld] is terminated", cb->getID() ) ;
               rc = SDB_APP_DISCONNECT ;
               break;
            }

            rc = nodeMgr.processEvent( event );
            if ( event._Data != NULL )
            {
               EvntCatalogInternalEvent *pEvent =
                  (EvntCatalogInternalEvent *)event._Data;
               if ( pEvent->data != NULL )
               {
                  SDB_OSS_FREE( pEvent->data );
               }
               SDB_OSS_FREE( pEvent );
            }
         }
      }

      PD_TRACE_EXITRC ( SDB_PMDCATNODEMGRENTPNT, rc );
      return rc;
   }
}