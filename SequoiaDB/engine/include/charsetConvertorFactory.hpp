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
      static boost::movelib::unique_ptr<charsetConvertorInterface>
         get( Charset inCharset, Charset outCharset ) ;

   protected:
      static charsetConvertorInterface*
         _create( Charset inCharset, Charset outCharset ) ;
   } ;

}  // namespace engine

#endif