/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdCMMain.cpp

   Descriptive Name = pmdCMMain

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
#include "rtnCM.hpp"
#include "ossErr.h"
#include "pd.hpp"

using namespace CLSMGR;
INT32 main( INT32 argc, CHAR** argv )
{
   INT32 rc = SDB_OK;
   cCMService svc;
   rc = svc.init();
   PD_RC_CHECK( rc, PDERROR,
               "failed to init cm(rc=%d)",
               rc );
   PD_LOG( PDEVENT, "start cm" );
   rc = svc.run( argc, argv );
   if ( rc )
   {
      PD_LOG( PDERROR,
            "execute failed(rc=%d)", rc );
   }
   PD_RC_CHECK( rc, PDERROR,
               "execute failed(rc=%d)", rc );
done:
   return rc;
error:
   goto done;
}