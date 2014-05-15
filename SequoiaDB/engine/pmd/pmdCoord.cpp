
#include "pmd.hpp"
#include "pmdEDU.hpp"
#include "pmdEDUMgr.hpp"
#include "netRouteAgent.hpp"
#include "pd.hpp"
#include "netTimer.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   class coordTimeoutHandler : public _netTimeoutHandler
   {
      public:
         void handleTimeout( const UINT32 &millisec,
                             const UINT32 &id )
         {
            return ;
         }
   } ;

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCOORDNETWKENTPNT, "pmdCoordNetWorkEntryPoint" )
   INT32 pmdCoordNetWorkEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      PD_TRACE_ENTRY ( SDB_PMDCOORDNETWKENTPNT ) ;
      pmdEDUMgr *pEduMgr = cb->getEDUMgr() ;
      _netRouteAgent *pNetWork = ( _netRouteAgent* )pData ;
      MsgRouteID coordRouteID ;
      coordTimeoutHandler timeHandler ;
      UINT32 timerid = 0 ;

      INT32 rc = pEduMgr->activateEDU( cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Active edu[%s] failed, rc: %d",
                   getEDUName( cb->getType() ), rc ) ;

      coordRouteID.value = 0 ;
      pNetWork->setLocalID( coordRouteID ) ;

      rc = pNetWork->addTimer( 60000, &timeHandler, timerid ) ;
      PD_RC_CHECK( rc, PDERROR, "Add coord timer failed, rc: %d", rc ) ;

      PD_LOG ( PDEVENT, "Runing coord-network..." ) ;

      pEduMgr->addIOService( pNetWork->ioservice() );
      pNetWork->run();
      pNetWork->removeTimer( timerid ) ;
      pEduMgr->deleteIOService( pNetWork->ioservice() );

      PD_LOG ( PDEVENT, "Stop coord-network" ) ;

   done:
      PD_TRACE_EXIT ( SDB_PMDCOORDNETWKENTPNT );
      return SDB_OK ;
   error:
      goto done ;
   }

}

