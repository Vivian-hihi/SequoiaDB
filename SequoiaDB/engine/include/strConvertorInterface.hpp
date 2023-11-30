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