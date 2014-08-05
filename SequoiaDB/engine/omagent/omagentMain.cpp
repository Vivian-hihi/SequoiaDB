#include "core.h"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "ossSocket.hpp"
#include "utilPath.hpp"
#include "omagentSession.hpp"

using namespace engine ;

INT32 main( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   const CHAR *dialogFile =
      "omagent.log" ;
//      "/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent/omagent.log" ;

   // record program path
   setProgramName( argv[0] ) ;
   // pd
   sdbEnablePD( dialogFile ) ;
   setPDLevel( PDDEBUG ) ;
   PD_LOG( PDEVENT, "Start omagent..." ) ;
   omaSession oas( (SOCKET)10 ) ;
   rc = oas.run() ;

   ossPrintf("in omagent main, rc is: %d\n", rc ) ;
   // close spidermoneky
   return rc ;
}
