/*******************************************************************************


   Copyright (C) 2011-2023 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY ; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = catLocation.cpp

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

#ifndef CHARSET_CONVERTOR_FACTORY__
#define CHARSET_CONVERTOR_FACTORY__

#include "charsetDef.hpp"
#include "charsetConvertorInterface.hpp"
#include "ossTypes.h"
#include "boost/move/unique_ptr.hpp"

namespace engine 
{
   // Charset convertor factory
   class charsetConvertorFactory
   {
   public:
      static void init() ;
      static void deinit() ;
      static charsetConvertorInterface* get( Charset inCharset,
                                             Charset outCharset ) ;

   protected:
      static charsetConvertorInterface*
         _create( Charset inCharset, Charset outCharset ) ;
   } ;

}  // namespace engine

#endif