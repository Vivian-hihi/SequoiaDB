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
