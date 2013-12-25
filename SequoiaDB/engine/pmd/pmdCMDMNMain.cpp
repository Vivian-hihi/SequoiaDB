/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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
   cPmdDaemon daemon( "sdbcmDMN");
   rc = svc.init();
   PD_RC_CHECK( rc, PDERROR,
               "failed to init cm(rc=%d)", rc );
   daemon.addChildrenProcess( &svc );
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