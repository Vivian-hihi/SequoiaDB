/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = pmdDaemon.cpp

   Descriptive Name = pmdDaemon

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/17/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#include "pmdDaemon.hpp"
#include "rtnCM.hpp"
#include "pd.hpp"
#include "ossUtil.h"

using namespace engine;
using namespace CLSMGR;
INT32 main( INT32 argc, CHAR** argv )
{
   INT32 rc = SDB_OK;
   cCMService svc;
   cPmdDaemon daemon( "sdbcmd");
   rc = svc.init();
   PD_RC_CHECK( rc, PDERROR,
               "failed to init cm(rc=%d)", rc );
   rc = daemon.addChildrenProcess( &svc );
   PD_RC_CHECK( rc, PDERROR,
               "failed to add childrenProcess(rc=%d)",
               rc );
   rc = daemon.init();
   if ( rc != SDB_OK )
   {
      ossPrintf( "failed to init daemon process(rc=%d)",
               rc );
      goto error;
   }
   PD_LOG( PDEVENT, "start program" );
   rc = daemon.run( argc, argv );
   PD_RC_CHECK( rc, PDERROR,
               "execute failed(rc=%d)", rc );
done:
   daemon.stop();
   PD_LOG( PDEVENT, "stop program" );
   return rc;
error:
   goto done;
}