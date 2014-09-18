
#include "fmpController.hpp"
#include <string>
#include "pmdOptions.h"
#include "pmdDef.hpp"
#include "utilParam.hpp"

using namespace std ;

#define COMMANDS_OPTIONS \
    ( PMD_COMMANDS_STRING (PMD_OPTION_HELP, ",h"), "help" ) \
    ( PMD_OPTION_VERSION, "version" )

// initialize options
void init ( po::options_description &desc )
{
   PMD_ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   PMD_ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << "Usage:  sdbfmp [OPTION]" <<std::endl;
   std::cout << desc << std::endl ;
}

INT32 main( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   fmpController controller ;
   po::options_description desc ( "Command options" ) ;
   po::variables_map vm ;

   init ( desc ) ;

   // validate arguments
   rc = engine::utilReadCommandLine( argc, argv, desc, vm, FALSE ) ;
   if ( rc )
   {
      PD_LOG( PDERROR, "Invalid arguments, rc: %d", rc ) ;
      displayArg ( desc ) ;
      goto done ;
   }
   /// read cmd first
   if ( vm.count( PMD_OPTION_HELP ) )
   {
      displayArg( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }
   if ( vm.count( PMD_OPTION_VERSION ) )
   {
      ossPrintVersion( "Sdb fmp version" ) ;
      rc = SDB_PMD_VERSION_ONLY ;
      goto done ;
   }

   rc = controller.run() ;

done:
   return SDB_OK == rc ? 0 : 127 ;
}

