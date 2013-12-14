/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossUtil.cpp

   Descriptive Name = Operating System Services Utilities

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains wrappers for basic System Calls
   or C APIs.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include <iostream>
#include <string>

#include "barBkupLogger.hpp"

INT32 main( INT32 argc, CHAR **argv )
{
   /*INT32 rc = SDB_OK;
   engine::_barBkupLogger logger;

   rc = logger.restore( argc, argv );
   if ( SDB_PMD_HELP_ONLY == rc )
   {
      goto done;
   }
   else if ( SDB_OK != rc )
   {
      std::cerr << "failed to restore, error number:" << rc << std::endl ;
   }
   else
   {
      std::cout << "success!" << std::endl;
   }
done:*/
   return SDB_OK;
}
