/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdSignalHandler.cpp

   Descriptive Name = Process MoDel Signal Handler

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains function to handle signals.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "pmdSignalHandler.hpp"
#include "ossStackDump.hpp"
#include "ossEDU.hpp"
#include "pmd.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
#if defined (_LINUX)
   PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSIGNALSIGSEGV, "ossSignalSigsegv" )
   void ossSignalSigsegv ( OSS_HANDPARMS )
   {
      PD_TRACE_ENTRY ( SDB_OSSSIGNALSIGSEGV );
      ossStackTrace ( OSS_HANDARGS, pmdGetKRCB()->getDiagLogPath() ) ;
      ossRestoreSystemSignal ( signum, true, pmdGetKRCB()->getDiagLogPath() ) ;
      PD_TRACE_EXIT ( SDB_OSSSIGNALSIGSEGV );
      return ;
   }
#endif
}
