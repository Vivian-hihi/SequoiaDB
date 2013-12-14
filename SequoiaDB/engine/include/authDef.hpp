/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = authDef.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef AUTHDEF_HPP_
#define AUTHDEF_HPP_

#include "core.hpp"

namespace engine
{
   #define AUTH_SPACE "SYSAUTH"
   #define AUTH_USR_COLLECTION AUTH_SPACE".SYSUSRS"
   /// AUTH_USR_COLLECTION SCHEMA
   /// {usr:"", passwd:"", fromusr:"", frompasswd:"", crttime:}

   #define AUTH_USR_INDEX_NAME "usrindex"

}

#endif

