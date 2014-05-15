
#include "core.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pd.hpp"
#include "ossMem.hpp"
#include "catDef.hpp"
#include "catCatalogManager.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCATALOGMGRENTPNT, "pmdCatCatalogManagerEntryPoint" )
   INT32 pmdCatCatalogManagerEntryPoint(pmdEDUCB *cb, void *pData)
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_PMDCATALOGMGRENTPNT ) ;
      pmdEDUMgr *eduMgr   = pmdGetKRCB()->getEDUMgr() ;
      catCatalogueManager* pCatlogMgr = ( catCatalogueManager* )pData ;

      pCatlogMgr->attachCB( cb ) ;

      rc = eduMgr->activateEDU( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU" ) ;
         goto error ;
      }

      eduMgr->regSystemEDU ( EDU_TYPE_CATCATALOGUEMANAGER, cb->getID()) ;

      // loop:process event
      while ( ! cb->isDisconnected() )
      {
         pmdEDUEvent event;
         if ( cb->waitEvent(event, MILLSEC_1000) )
         {
            if ( PMD_EDU_EVENT_TERM == event._eventType )
            {
               PD_LOG ( PDEVENT, "EDU[%lld] is terminated", cb->getID() ) ;
               rc = SDB_APP_FORCED ;
               break;
            }
            rc = pCatlogMgr->processEvent( event ) ;
            if ( event._Data != NULL )
            {
               EvntCatalogInternalEvent *pEvent =
                  (EvntCatalogInternalEvent *)event._Data;
               if ( pEvent->data != NULL )
               {
                  SDB_OSS_FREE( pEvent->data );
               }
               SDB_OSS_FREE(event._Data) ;
            }
         }
      }

   done:
      pCatlogMgr->detachCB( cb ) ;
      PD_TRACE_EXITRC ( SDB_PMDCATALOGMGRENTPNT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

}

