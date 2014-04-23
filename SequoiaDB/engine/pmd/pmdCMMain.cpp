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