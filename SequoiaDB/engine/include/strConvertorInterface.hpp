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
#ifndef STR_CONVERTOR_INTERFACE__
#define STR_CONVERTOR_INTERFACE__

#include <string>
#include "../bson/bsonobj.h"
#include "ossTypes.h"

namespace engine 
{
    using namespace bson ;

    // A convertorInterface is an abstract class for converting
    // character set of StringData objects.
    class convertorInterface 
    {
    public:
        convertorInterface() { }

        virtual ~convertorInterface() { }

        // Convert character set of input StringData.
        virtual INT32 convertToBuffer( const char* inBuffer,
                                       int inSize,
                                       char* outBuffer,
                                       int outSize,
                                       int& convertedInSize,
                                       int& convertedOutSize ) const = 0 ;
        virtual INT32 convertToString( const StringData &inString,
                                       std::string& outString ) const = 0 ;

        virtual INT32 estimateConvertedSize(int inSize) const = 0 ;
    } ;

}  // namespace engine

#endif