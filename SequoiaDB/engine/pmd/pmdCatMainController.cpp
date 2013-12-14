
#include "core.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pd.hpp"
#include "catalogueCB.hpp"
#include "catDef.hpp"
#include "catMainController.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCATMAINCTRLENTPNT, "pmdCatMainControllerEntryPoint" )
   INT32 pmdCatMainControllerEntryPoint( pmdEDUCB *cb, void *pData )
   {
      SDB_ASSERT ( cb, "cb can't be NULL" )
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_PMDCATMAINCTRLENTPNT );

      pmdKRCB   *krcb     = pmdGetKRCB () ;
      clsCB *pclsCB = krcb->getClsCB();
      pmdEDUMgr *eduMgr   = krcb->getEDUMgr () ;
      BOOLEAN isPrimary = FALSE;

      // free by end of the function
      catEventProcessor *pCatMainCtr = SDB_OSS_NEW catMainController( cb );
      if ( !pCatMainCtr )
      {
         PD_LOG ( PDERROR, "Failed to allocate catMainControler" ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      do
      {
         rc = pCatMainCtr->init();
         if ( rc != SDB_OK )
         {
            PD_LOG (PDERROR, "CatlogueMainController init error(rc = %d)", rc );
            break;
         }
         eduMgr->regSystemEDU ( EDU_TYPE_CATMAINCONTROLLER, cb->getID()) ;

         //loop: process event(switch between primary and slave)
         while ( ! cb->isDisconnected() )
         {
            pmdEDUEvent event;
            if ( ! cb->waitEvent(event, MILLSEC_1000) )
            {
               BOOLEAN isTmp = pclsCB->isPrimary ();
               if ( isTmp != isPrimary )
               {
                  if ( TRUE == isTmp )
                  {
                     rc = pCatMainCtr->active() ;
                  }
                  else
                  {
                     rc = pCatMainCtr->deactive() ;
                  }

                  if ( SDB_OK != rc )
                  {
                     break ;
                  }
                  isPrimary = isTmp ;
               }
            }
            else
            {
               if (PMD_EDU_EVENT_TERM == event._eventType)
               {
                  PD_LOG ( PDEVENT, "EDU[%lld] is terminated", cb->getID() ) ;
                  rc = SDB_APP_FORCED;
                  break;
               }

               rc = pCatMainCtr->processEvent(event);
               if (rc != SDB_OK)
               {
                  PD_LOG ( PDWARNING,
                          "MainController process event failed(EventType = %d)",
                          event._eventType );
               }
               if ( event._Data != NULL )
               {
                  SDB_OSS_FREE(event._Data);
               }
            }
         }
      }while(FALSE);

   done :
      if ( pCatMainCtr )
      {
         SDB_OSS_DEL pCatMainCtr ;
      }
      PD_TRACE_EXITRC ( SDB_PMDCATMAINCTRLENTPNT, rc );
      return rc ;
   error :
      goto done ;
   }
}

