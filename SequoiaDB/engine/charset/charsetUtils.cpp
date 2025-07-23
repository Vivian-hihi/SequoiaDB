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

   Source File Name = charsetUtils.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/11/2023  ZYS  Initial Draft

   Last Changed =

*******************************************************************************/
#include "charsetDef.hpp"
#include "charsetUtils.hpp"
#include "../bson/bson.h"
#include "pd.hpp"

namespace engine 
{
   using namespace bson ;
   Charset charsetParse( StringData value )
   {
      if ( 0 == strcasecmp( value.data(), CHARSET_NAME_UTF8) )
      {
         return CHARSET_UTF8 ;
      }

      if ( 0 == strcasecmp( value.data(), CHARSET_NAME_GB18030) )
      {
         return CHARSET_GB18030 ;
      }

      return CHARSET_UNKNOWN ;
   }

   StringData charsetSerializer( Charset value )
   {
      if ( value == CHARSET_UTF8 )
      {
         return StringData( CHARSET_NAME_UTF8 ) ;
      }

      if ( value == CHARSET_GB18030 )
      {
         return StringData( CHARSET_NAME_GB18030 ) ;
      }
      return StringData( "" ) ;
   }
}  // namespace engine
