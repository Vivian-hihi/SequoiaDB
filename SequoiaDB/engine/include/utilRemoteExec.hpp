/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = utilRemoteExec.hpp

   Descriptive Name = Remote Excuting Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declares for process op.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          2/27/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTILREMOTEEXEC_HPP__
#define UTILREMOTEEXEC_HPP__

#include "core.hpp"
#include "../bson/bson.h"

INT32 utilRemoteExec ( SINT32 remoCode,
                      const CHAR * hostname,
                      SINT32 *retCode,
                      bson::BSONObj *arg1 = NULL,
                      bson::BSONObj *arg2 = NULL,
                      bson::BSONObj *arg3 = NULL,
                      bson::BSONObj *arg4 = NULL ) ;

#endif /* UTILREMOTEEXEC_HPP__ */