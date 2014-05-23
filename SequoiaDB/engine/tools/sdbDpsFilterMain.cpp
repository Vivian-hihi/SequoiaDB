#include <iostream>
#include "sdbDpsLogFilter.hpp"
#include "sdbDpsFilter.hpp"
#include "sdbDpsOption.hpp"

INT32 main(INT32 argc, CHAR** argv)
{
   INT32 rc                = SDB_OK ;

   dpsLogFilter *logFilter = NULL ;
   iFilter *filter         = NULL ;
   dpsFilterOption op ;

   po::options_description desc( "Command options" ) ;
   po::variables_map vm ;
   rc = op.init( argc, argv, desc, vm ) ;
   if( rc )
   {
      op.displayArgs( desc ) ;
      goto error ;
   }

   rc = op.handle( desc, vm, filter ) ;
   if( rc )
   {
      goto error ;
   }
   if( SDB_DPS_DUMP_HELP == rc || SDB_DPS_DUMP_VER == rc )
   {
      goto done ;
   }

   logFilter = SDB_OSS_NEW dpsLogFilter( op.getCmdData() ) ;
   if( NULL == logFilter )
   {
      printf( "Failed to allocate dpsLogFilter" ) ;
      rc = SDB_OOM ;
      goto error ;
   }

   logFilter->setFilter( filter ) ;
   rc = logFilter->doParse() ;
   if( rc )
   {
      goto error ;
   }

done:
   if ( logFilter )
   {
      SDB_OSS_DEL( logFilter ) ;
      logFilter = NULL ;
   }
   return rc  ;

error :
   goto done  ;
}
