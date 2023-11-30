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
