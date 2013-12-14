
#include "core.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCATNETWKENTPNT, "pmdCatNetWorkEntryPoint" )
   INT32 pmdCatNetWorkEntryPoint( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDCATNETWKENTPNT );
      pmdEDUMgr *pEduMgr = cb->getEDUMgr() ;
      _netRouteAgent *pNetWork = pmdGetKRCB()->getCATLOGUECB()->netWork();
      if ( !pNetWork )
      {
         PD_LOG ( PDERROR, "Failed to alloc net work" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = pEduMgr->activateEDU( cb->getID() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU[type:%d,ID:%lld]",
                  cb->getType() , cb->getID() ) ;
         goto error ;
      }

      pEduMgr->addIOService( pNetWork->ioservice() );
      pNetWork->run();
      pEduMgr->deleteIOService( pNetWork->ioservice() );

   done:
      PD_TRACE_EXITRC ( SDB_PMDCATNETWKENTPNT, rc );
      return rc ;
   error:
      goto done ;
   }
}


