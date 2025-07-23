/*******************************************************************************

   Copyright (C) 2011-Present SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

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