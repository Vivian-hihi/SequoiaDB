
#include "core.hpp"
#include "pd.hpp"


namespace engine
{
   // called by non-coord modes
   INT32 rtnMsg ( CHAR *pMsg )
   {
      INT32 rc = SDB_OK ;
      if ( pMsg )
      {
         PD_LOG ( PDEVENT, "Received message : %s", pMsg ) ;
      }
      return rc ;
   }
}

