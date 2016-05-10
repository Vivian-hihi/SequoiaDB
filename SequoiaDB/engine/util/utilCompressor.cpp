/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = utilCompressor.cpp

   Descriptive Name = Compressor for data compression and decompression.

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2015  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#include "utilCompressorSnappy.hpp"
#include "utilCompressorLZW.hpp"
#include "utilCompressorLZ4.hpp"
#include "utilCompressorZlib.hpp"
#include "msgDef.hpp"

namespace engine
{
   utilCompressor* getCompressorByType( UTIL_COMPRESSOR_TYPE type )
   {
      utilCompressor *compressor = NULL ;

      static utilCompressorSnappy snappyCompressor ;
      static utilCompressorLZW lzwCompressor ;
      static utilCompressorLZ4 lz4Compressor ;
      static utilCompressorZlib zlibCompressor ;
      switch ( type )
      {
         case UTIL_COMPRESSOR_LZW:
            compressor = &lzwCompressor ;
            break ;
         case UTIL_COMPRESSOR_SNAPPY:
            compressor = &snappyCompressor ;
            break;
         case UTIL_COMPRESSOR_LZ4:
            compressor = &lz4Compressor ;
            break;
         case UTIL_COMPRESSOR_ZLIB:
            compressor = &zlibCompressor ;
            break;
         default:
            compressor = NULL ;
      }

      return compressor ;
   }

   const CHAR *utilCompressType2String( UINT8 type )
   {
      switch( type )
      {
         case UTIL_COMPRESSOR_SNAPPY :
            return VALUE_NAME_SNAPPY ;
            break ;
         case UTIL_COMPRESSOR_LZW :
            return VALUE_NAME_LZW ;
            break ;
         default :
            return "Invalid" ;
      }
   }
} /* engine */

