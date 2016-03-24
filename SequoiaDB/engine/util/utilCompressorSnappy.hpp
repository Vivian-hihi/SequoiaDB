#ifndef UTIL_COMPRESSOR_SNAPPY__
#define UTIL_COMPRESSOR_SNAPPY__

#include "utilCompressor.hpp"
#include <../snappy/snappy.h>

namespace engine
{
   class _utilCompressorSnappy : public utilCompressor
   {
   public:
      _utilCompressorSnappy()
      : _utilCompressor( UTIL_COMPRESSOR_SNAPPY )
      {
      }

      INT32 compressBound( UINT32 srcLen,
                            UINT32 &maxCompressedLen,
                            const CHAR *dictionary = NULL ) ;
      INT32 compress( const CHAR *source, UINT32 sourceLen,
                      CHAR *dest, UINT32 &destLen,
                      const CHAR *dictionary = NULL ) ;
      INT32 getUncompressedLen( const CHAR *source, UINT32 sourceLen,
                                UINT32 &length) ;
      INT32 decompress( const CHAR *source, UINT32 sourceLen,
                        CHAR *dest, UINT32 &destLen,
                        const CHAR *dictionary = NULL ) ;
   } ;
   typedef _utilCompressorSnappy utilCompressorSnappy ;

}

#endif /*¡¡UTIL_COMPRESSOR_SNAPPY__ */

