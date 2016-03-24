#ifndef UTIL_COMPRESSOR_LZW__
#define UTIL_COMPRESSOR_LZW__

#include "utilCompressor.hpp"
#include "utilLZW.hpp"

namespace engine
{
   class _utilCompressorLZW : public utilCompressor
   {
      public:
         _utilCompressorLZW();
         ~_utilCompressorLZW();
      public:
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

      private:
         _utilLZW _lzw ;
   };
   typedef _utilCompressorLZW utilCompressorLZW ;
}

#endif /* UTIL_COMPRESSOR_LZW__ */

