
#include "core.hpp"
#include "pd.hpp"
#include "pmdEDUMgr.hpp"
#include "netRouteAgent.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDSHARDRENTPNT, "pmdShardREntryPoint" )
   INT32 pmdShardREntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_PMDSHARDRENTPNT );
      pmdEDUMgr *pEDUMgr = cb->getEDUMgr () ;
      _netRouteAgent *pRouteAgent = (_netRouteAgent *)pData;

      rc = pEDUMgr->activateEDU( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU[type:%d,ID:%lld]",
                  cb->getType(), cb->getID() ) ;
         goto error ;
      }

      //start run
      PD_LOG ( PDEVENT, "Run sharding listen..." ) ;
      try
      {
         pEDUMgr->addIOService( pRouteAgent->ioservice() ) ;
         pRouteAgent->run() ;
         pEDUMgr->deleteIOService ( pRouteAgent->ioservice() ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception during start shard session: %s",
                  e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "Stop sharding listen" ) ;

   done:
      PD_TRACE_EXITRC ( SDB_PMDSHARDRENTPNT, rc );
      return rc;
   error:
      goto done;
   }
}

