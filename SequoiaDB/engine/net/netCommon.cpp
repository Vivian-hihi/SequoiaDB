/******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

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
#include "ossUtil.h"

namespace engine
{
   static NET_COMPRESSOR& _netGetCompressor()
   {
      static NET_COMPRESSOR  s_netCompressor = NONE_COMPRESSOR ;
      return s_netCompressor ;
   }

   void netUpdateNetcompressor( NET_COMPRESSOR compressor )
   {
      _netGetCompressor() = compressor ;
   }

   NET_COMPRESSOR netGetNetcompressor()
   {
      return _netGetCompressor() ;
   }

   const CHAR* netCompressorNum2Str( NET_COMPRESSOR compressor )
   {
      if ( LZ4_COMPRESSOR == compressor )
      {
         return NET_COMPRESSOR_STR_LZ4 ;
      }
      else
      {
         return "" ;
      }
   }

   NET_COMPRESSOR netCompressorStr2Num( const CHAR* compressor )
   {
      if ( compressor != NULL &&
           0 == ossStrcasecmp( compressor, NET_COMPRESSOR_STR_LZ4 ) )
      {
         return LZ4_COMPRESSOR ;
      }
      else
      {
         return NONE_COMPRESSOR ;
      }
   }
}