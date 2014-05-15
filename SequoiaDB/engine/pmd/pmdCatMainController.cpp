
#include "core.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pd.hpp"
#include "catDef.hpp"
#include "catMainController.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCATMAINCTRLENTPNT, "pmdCatMainControllerEntryPoint" )
   INT32 pmdCatMainControllerEntryPoint( pmdEDUCB *cb, void *pData )
   {
      SDB_ASSERT ( cb, "cb can't be NULL" )
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_PMDCATMAINCTRLENTPNT );

      pmdKRCB   *krcb      = pmdGetKRCB () ;
      pmdEDUMgr *eduMgr    = krcb->getEDUMgr () ;

      // free by end of the function
      catMainController *pCatMainCtr = ( catMainController* )pData ;

      pCatMainCtr->attachCB( cb ) ;

      rc = eduMgr->activateEDU( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU" ) ;
         goto error ;
      }
      eduMgr->regSystemEDU ( EDU_TYPE_CATMAINCONTROLLER, cb->getID() ) ;

      //loop: process event(switch between primary and slave)
      while ( ! cb->isDisconnected() )
      {
         pmdEDUEvent event ;
         if ( cb->waitEvent(event, MILLSEC_1000) )
         {
            if (PMD_EDU_EVENT_TERM == event._eventType)
            {
               PD_LOG ( PDEVENT, "EDU[%lld] is terminated", cb->getID() ) ;
               rc = SDB_APP_FORCED ;
               break;
            }

            rc = pCatMainCtr->processEvent( event ) ;
            if (rc != SDB_OK)
            {
               PD_LOG ( PDWARNING, "MainController process event failed"
                        "(EventType = %d), rc: %d", event._eventType,
                        rc ) ;
            }
            if ( event._Data != NULL )
            {
               SDB_OSS_FREE(event._Data);
            }
         }
      }

   done :
      pCatMainCtr->detachCB( cb ) ;
      PD_TRACE_EXITRC ( SDB_PMDCATMAINCTRLENTPNT, rc );
      return rc ;
   error :
      goto done ;
   }

}

