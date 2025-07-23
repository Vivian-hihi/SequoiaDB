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

   Source File Name = netCommon.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-motionatted
   versions of PD component. This file contains declare of PD functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who         Description
   ====== =========== =========== ==============================================
          04/01/2024  fangjiabin  Initial Draft

   Last Changed =

*******************************************************************************/
#include "netCommon.hpp"

namespace engine
{
   static UTIL_COMPRESSOR_TYPE& _netGetCompressor()
   {
      static UTIL_COMPRESSOR_TYPE s_netCompressor = UTIL_COMPRESSOR_INVALID ;
      return s_netCompressor ;
   }

   void netUpdateNetcompressor( UTIL_COMPRESSOR_TYPE compressor )
   {
      _netGetCompressor() = compressor ;
   }

   UTIL_COMPRESSOR_TYPE netGetNetcompressor()
   {
      return _netGetCompressor() ;
   }
}