#include "pmdEDU.hpp"
#include "rtn.hpp"
#include "ossUtil.h"

namespace engine
{
   INT32 pmdDpsTransRollbackTaskEntryPoint( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;

      while( !cb->isDisconnected() )
      {
         pmdEDUEvent event;
         if ( cb->waitEvent( event, OSS_ONE_SEC ))
         {
            if ( PMD_EDU_EVENT_TERM == event._eventType )
            {
               PD_LOG ( PDEVENT, "EDU[%lld] is terminated",
                        cb->getID() );
               rc = SDB_APP_DISCONNECT;
               break;
            }
            else if ( PMD_EDU_EVENT_ACTIVE == event._eventType )
            {
               rc = rtnTransRollbackAll( cb );
            }
            if ( event._Data != NULL && event._release )
            {
               SDB_OSS_FREE( event._Data );
            }
         }
      }
      rc = SDB_OK ;
      return rc ;
   }
}

