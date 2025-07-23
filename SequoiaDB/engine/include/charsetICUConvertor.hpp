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

      charsetConvertorInterface* make_clone() ;

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