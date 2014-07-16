#include "core.h"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "ossSocket.hpp"
#include "omagentSession.hpp"

using namespace CLSMGR ;

INT32 main( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   const CHAR *dialogFile =
      "omagent.log" ;
//      "/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent/omagent.log" ;

   // pd
   sdbEnablePD( dialogFile ) ;
   setPDLevel( PDDEBUG ) ;
   PD_LOG( PDEVENT, "Start omagent..." ) ;

   omagentSession oas( (SOCKET)10 ) ;
   rc = oas.run() ;

   ossPrintf("in omagent main, rc is: %d\n", rc ) ;
   // close spidermoneky
   return rc ;
}
