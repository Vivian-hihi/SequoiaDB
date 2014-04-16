/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = omDef.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_DEF_HPP__
#define OM_DEF_HPP__

#include "core.hpp"
#include "oss.hpp"

namespace engine
{

   /*
      param define
   */
   #define OM_FIX_BUFF_CATCH_NUMBER          ( 100 )

   #define OM_FIX_PTR_SIZE(x)                ( x + sizeof(INT32) )
   #define OM_FIX_PTR_HEADER(ptr)            (*(INT32*)(ptr))
   #define OM_FIX_BUFF_TO_PTR(buff)          ((CHAR*)(buff)-sizeof(INT32))
   #define OM_FIX_PTR_TO_BUFF(ptr)           ((CHAR*)(ptr)+sizeof(INT32))
   #define OM_FIX_BUFF_HEADER(buff)          (*(INT32*)((CHAR*)(buff)-sizeof(INT32)))

   // max rest body size
   #define OM_REST_MAX_BODY_SIZE             ( 64 * 1024 * 1024 )

}

#endif // OM_DEF_HPP__

