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

   Source File Name = charsetConvertotInterface.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          25/10/2023  ZYS  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef CHARSET_CONVERTOR_INTERFACE__
#define CHARSET_CONVERTOR_INTERFACE__

#include "charsetDef.hpp"
#include <memory>
#include <string>
#include "ossTypes.h"
#include "strConvertorInterface.hpp"
#include "../bson/bsonobj.h"

using namespace bson ;

namespace engine
{

   // An interface for encoding objects with character set.
   class charsetConvertorInterface : public convertorInterface 
   {
   public:
      charsetConvertorInterface( Charset inCharset, Charset outCharset )
          : _inCharset(inCharset), _outCharset(outCharset) {}
      
      virtual ~charsetConvertorInterface() {}

      virtual charsetConvertorInterface* make_clone()
      { 
         return NULL ; 
      }

      virtual INT32 convert( const std::string& inString, 
                             std::string& outString ) const = 0 ;
      virtual INT32 convert( const StringData& inString,
                             std::string& outString ) const = 0 ;
      virtual INT32 convert( const BSONObj& inObject,
                             BSONObj& outObject ) const = 0 ;

      Charset getInCharset() const
      {
          return _inCharset ;
      }

      Charset getOutCharset() const
      {
          return _outCharset ;
      }

   private:
      const Charset _inCharset ;
      const Charset _outCharset ;
   } ;

}  // namespace engine

#endif
