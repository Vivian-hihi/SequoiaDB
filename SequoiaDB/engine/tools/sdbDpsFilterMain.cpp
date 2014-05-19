#include <iostream>
#include "sdbDpsLogFilter.hpp"
#include "sdbDpsFilter.hpp"
#include "sdbDpsOption.hpp"
#include "ossVer.h"

INT32 main(INT32 argc, CHAR** argv)
{
   INT32 rc                = SDB_OK ;
   
   dpsLogFilter *logFilter = NULL ;
   iFilter *filter         = NULL ;
   dpsFilterOption op ;

   po::options_description desc( "Command options" ) ;
   po::variables_map vm ;
   rc = op.init( argc, argv, desc, vm, filter ) ;
   if( rc )
   {
      op.displayArgs( desc ) ;
      goto error ;
   }

   if( vm.count( DPS_LOG_FILTER_HELP ) )
   {
      op.displayArgs( desc ) ;
      rc = SDB_OK ;
      goto done ;
   }

   if( vm.count( DPS_LOG_FILTER_VER ) )
   {
      ossPrintVersion( "SequoiaDB version" ) ;
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
