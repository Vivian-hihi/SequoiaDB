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
