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
         INT32 prepare( UTIL_COMPRESSOR_WORK workType ) ;
         INT32 setDictionary( const CHAR* dict, UINT32 dict_size,
                              BOOLEAN copy = FALSE ) ;
         size_t compressBound( size_t srcLen ) ;
         INT32 compress( const CHAR* source, UINT32 sourceSize,
                        CHAR* dest, UINT32 &destSize ) ;
         INT32 decompress( const CHAR* source, UINT32 sourceSize,
                          CHAR* dest, UINT32 &destSize ) ;
      private:
         _utilLZWContext _ctx ;
   };
   typedef _utilCompressorLZW utilCompressorLZW ;
}

#endif /* UTIL_COMPRESSOR_LZW__ */


