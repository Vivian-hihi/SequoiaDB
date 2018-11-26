#include "ossTypes.h"
#include "ossVer.h"
#include "ossUtil.hpp"
#include "passwdOptions.hpp"
#include "utilPasswdTool.hpp"
#include "utilCipherFile.hpp"
#include "utilCipherMgr.hpp"
#include "pd.hpp"
#include <iostream>

using namespace engine;
using namespace passwd;
using namespace std;

#define SDBPASSWD_LOG "sdbpasswd.log"

INT32 main(INT32 argc, char* argv[])
{
   INT32 rc = SDB_OK ;
   passwdOptions options ;

   sdbEnablePD( SDBPASSWD_LOG ) ;
   setPDLevel( PDINFO ) ;
   cipherMgr mgr ;
   cipherFile cipherfile ;

   try
   {
      rc = options.parseCmd( argc, argv ) ;
      if ( rc )
      {
         options.printHelpInfo();
         if ( SDB_PMD_HELP_ONLY == rc )
         {
            rc = SDB_OK;
            goto done;
         }
         goto error;
      }

      cipherfile.initFile( options.file(), cipherFile::WRole ) ;
      if ( SDB_OK != rc )
      {
         std::cerr << "init cipherfile failed" << std::endl ;
         PD_LOG( PDERROR, "cipherfFile initialize error [%d]", rc ) ;
         goto error;
      }

      rc = mgr.init( &cipherfile );
      if ( SDB_OK != rc )
      {
         std::cerr << "init cipherMgr failed" << std::endl ;
         PD_LOG( PDERROR, "cipherMgr initialize error [%d]", rc ) ;
         goto error;
      }
      if ( passwdOptions::OpAddUser == options.mode() )
      {
         string password ;
         if ( !options.hasPassword() )
         {
            password = passwordTool::interactivePasswdInput() ;
         }
         else
         {
            password = options.password() ;
         }

         rc = mgr.addUser( options.user(), options.token(), password );
         if ( SDB_OK != rc )
         {
            std::cerr << "adduser failed" << std::endl ;
            PD_LOG( PDINFO, "cipherMgr adduser [%s], error [%d]",
                    options.user().c_str(), rc ) ;
         }
      }
      else if ( passwdOptions::OpRemoveUser == options.mode() )
      {
         rc = mgr.removeUser( options.user() );
         if ( SDB_OK != rc )
         {
            std::cerr << "removeuser failed" << std::endl ;
            PD_LOG( PDINFO, "cipherMgr removeuser [%s], error [%d]",
                    options.user().c_str(), rc ) ;
         }
      }
      else
      {
         rc = SDB_SYS ;
         std::cerr << "unexpected error" << std::endl ;
         PD_LOG( PDERROR, "cipherMgr illegal method" ) ;
         goto error;
      }
   }
   catch( exception &e )
   {
      std::cerr << "unexpected error" << endl ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}
