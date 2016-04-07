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

   Source File Name = utilCompressor.hpp

   Descriptive Name = Compressor for data compression and decompression.

   When/how to use: this program may be used to compress/decompress data. This
   file contains the interfaces provided by compressors.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2015  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTIL_COMPRESSOR__
#define UTIL_COMPRESSOR__

#include "core.hpp"
#include "oss.hpp"

namespace engine
{
   enum UTIL_COMPRESSOR_TYPE
   {
      UTIL_COMPRESSOR_INVALID = -1,
      UTIL_COMPRESSOR_SNAPPY = 0,
      UTIL_COMPRESSOR_LZW = 1
   } ;

   #define UTIL_INVALID_DICT           NULL
   typedef void * utilDictHandle ;

   /* This class provides compressor interfaces. */
   class _utilCompressor : public SDBObject
   {
   public:
      _utilCompressor( UTIL_COMPRESSOR_TYPE type )
      : _type( type )
      {
      }

      virtual ~_utilCompressor() {}

      virtual INT32 compressBound( UINT32 srcLen, UINT32 &maxCompressedLen,
                                 const utilDictHandle dictionary = NULL ) = 0 ;

      virtual INT32 compress( const CHAR *source, UINT32 sourceLen,
                              CHAR *dest, UINT32 &destLen,
                              const utilDictHandle dictionary = NULL ) = 0 ;

      virtual INT32 getUncompressedLen( const CHAR *source, UINT32 sourceLen,
                                        UINT32 &length) = 0 ;

      virtual INT32 decompress( const CHAR *source, UINT32 sourceLen,
                                CHAR *dest, UINT32 &destLen,
                                const utilDictHandle dictionary = NULL ) = 0 ;
   private:
      UTIL_COMPRESSOR_TYPE _type ;
   } ;
   typedef _utilCompressor utilCompressor ;

   /*
    * Get the global compressor pointer. When that is done, you can use the
    * compressor to compress/decompress data.
    */
   utilCompressor* getCompressorByType( UTIL_COMPRESSOR_TYPE type ) ;

   /* Get the name of the compressor in string format. */
   const CHAR *utilCompressType2String( UINT8 type ) ;
}

#endif /* UTIL_COMPRESSOR__ */

