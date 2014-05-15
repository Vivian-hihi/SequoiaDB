
#include "core.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pd.hpp"
#include "catNodeManager.hpp"
#include "catDef.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCATNODEMGRENTPNT, "pmdCatNodeManagerEntryPoint" )
   INT32 pmdCatNodeManagerEntryPoint( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_PMDCATNODEMGRENTPNT );
      pmdEDUMgr *eduMgr   = pmdGetKRCB()->getEDUMgr();
      catNodeManager* pNodeMgr = ( catNodeManager* )pData ;

      pNodeMgr->attachCB( cb ) ;

      rc = eduMgr->activateEDU( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU" ) ;
         goto error ;
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
               PD_LOG ( PDDEBUG, "EDU[%lld, %s] is terminated", cb->getID(),
                        getEDUName( cb->getType() ) ) ;
               rc = SDB_APP_DISCONNECT ;
               break;
            }

            rc = pNodeMgr->processEvent( event );
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

   done:
      pNodeMgr->detachCB( cb ) ;
      PD_TRACE_EXITRC ( SDB_PMDCATNODEMGRENTPNT, rc );
      return rc ;
   error:
      goto done ;
   }
}