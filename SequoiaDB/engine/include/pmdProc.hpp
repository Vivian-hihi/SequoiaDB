/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdProc.hpp

   Descriptive Name = pmdProc

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/26/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDPROC_HPP_
#define PMDPROC_HPP_

#include "ossTypes.h"
#include "oss.hpp"

namespace engine
{
   class iPmdProc : public SDBObject
   {
   public:
      iPmdProc();
      virtual ~iPmdProc();
      virtual INT32 regSignalHandler();
      virtual INT32 run( INT32 argc, CHAR **argv ) = 0;
      static BOOLEAN isRunning();

   public:
      static void stop();

   private:
      static BOOLEAN                _isRunning ;
   };
}

#endif