/*******************************************************************************


   Copyright (C) 2011-2023 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = charsetDef.hpp

   Descriptive Name = N/A

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          25/10/2023  ZYS Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef CHARSET_DEF__
#define CHARSET_DEF__

#include "ossTypes.h"

namespace engine 
{

#define CHARSET_NAME_UTF8 "UTF8"
#define CHARSET_NAME_GB18030 "GB18030"

   enum Charset
   {
      CHARSET_UNKNOWN = -1 ,
      CHARSET_UTF8 ,
      CHARSET_GB18030 ,
   } ;
   
   const INT16 CHARSET_SUPPORTED_CHARSET_NUM = 2 ;

}  // namespace engine

#endif