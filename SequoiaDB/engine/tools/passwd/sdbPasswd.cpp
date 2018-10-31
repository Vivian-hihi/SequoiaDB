#include "ossTypes.h"
#include "ossVer.h"
#include "ossUtil.hpp"
#include "passwdOptions.hpp"
#include "utilCipher.hpp"
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
   cipherMgr mgr;

   try
   {
      rc = options.parseCmd( argc, argv ) ;
      if ( rc )
      {
         if ( SDB_PMD_HELP_ONLY == rc )
         {
            rc = SDB_OK;
            goto done;
         }
         options.printHelpInfo();
         goto error;
      }

      rc = mgr.initialize( options.file(), cipherMgr::WRole );
      if ( SDB_OK != rc )
      {
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
         PD_LOG( PDINFO, "cipherMgr adduser [%s], error [%d]",
                 options.user().c_str(), rc ) ;
      }
      else if ( passwdOptions::OpRemoveUser == options.mode() )
      {
         rc = mgr.removeUser( options.user() );
         PD_LOG( PDINFO, "cipherMgr removeuser [%s], error [%d]",
                 options.user().c_str(), rc ) ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "cipherMgr illegal method" ) ;
         goto error;
      }
   }
   catch( exception &e )
   {
      std::cerr << "unexpect error happend" << endl ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}
