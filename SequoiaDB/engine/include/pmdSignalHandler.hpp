/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdSignalHandler.hpp

   Descriptive Name = Process MoDel Signal Handler Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains function used for signal
   handler.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDSIGNALHANDLER_HPP_
#define PMDSIGNALHANDLER_HPP_
#include "core.hpp"
#include "ossSignal.hpp"
#include "ossStackDump.hpp"
#include "ossPrimitiveFileOp.hpp"
namespace engine
{
   void ossSignalSigsegv ( OSS_HANDPARMS ) ;
}

#endif
