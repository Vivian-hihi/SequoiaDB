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

   /*
      HTML File Name Define
   */
   #define HTML_FILE_LOGIN                   "login.html"
   #define HTML_FILE_INDEX                   "index.html"

   /*
      OM Field Define
   */

}

#endif // OM_DEF_HPP__

