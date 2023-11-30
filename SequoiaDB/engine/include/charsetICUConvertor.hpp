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

   Source File Name = charsetICUConvertor.hpp

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
#ifndef CHARSET_ICU_CONVERTOR__
#define CHARSET_ICU_CONVERTOR__

#include "charsetDef.hpp"
#include "charsetConvertorInterface.hpp"
#include "ossTypes.h"
#include <memory>

struct UConverter ;

namespace engine
{

   // An ICU encoder for encoding objects with character set.

   class charsetICUConvertor : public charsetConvertorInterface
   {
      charsetICUConvertor( Charset inCharset,
                           Charset outCharset,
                           UConverter* inConv,
                           UConverter* outConv ) ;

   public:
      static charsetConvertorInterface*
         make( Charset inCharset, Charset outCharset ) ;

      virtual ~charsetICUConvertor() ;
      virtual INT32 convertToBuffer( const char* inBuffer,
                                     int inSize,
                                     char* outBuffer,
                                     int outSize,
                                     int& convertedInSize,
                                     int& convertedOutSize ) const ;

      virtual INT32 convertToString( const StringData &inString,
                                     std::string& outString ) const
      {
          return convert(inString, outString) ;
      }

      virtual INT32 estimateConvertedSize(int inSize) const
      {
          return _estimateMaxSize * inSize ;
      }

      virtual INT32 convert( const std::string& inString,
                             std::string& outString ) const ;
      virtual INT32 convert( const StringData& inString,
                             std::string& outString ) const ;
      virtual INT32 convert( const BSONObj& inObject,
                             BSONObj& outObject ) const ;

   protected:
      INT32 _convert( const BSONObj& inObject,
                      BSONObjBuilder& outBuilder ) const ;
      INT32 _convert( const BSONObj& inObject,
                      BSONArrayBuilder& outBuilder ) const ;

   protected:
      int _estimateMaxSize ;
      UConverter* _inConverter ;
      UConverter* _outConverter ;
   } ;

}  // namespace engine

#endif