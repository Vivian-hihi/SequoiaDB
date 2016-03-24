// This header file is to define the compressor type and interfaces.

#ifndef UTIL_COMPRESSOR__
#define UTIL_COMPRESSOR__

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"

namespace engine
{
   typedef void * utilCompressorContext ;
   #define UTIL_INVALID_COMP_CTX    NULL

   enum UTIL_COMPRESSOR_TYPE
   {
      UTIL_COMPRESSOR_INVALID = -1,
      UTIL_COMPRESSOR_SNAPPY = 0,
      UTIL_COMPRESSOR_LZW = 1
   } ;

   /* This class provides compressor interfaces. */
   class _utilCompressor : public SDBObject
   {
   public:
      _utilCompressor( UTIL_COMPRESSOR_TYPE type )
      : _type( type )
      {
      }

      virtual ~_utilCompressor() {}
      virtual INT32 compressBound( UINT32 srcLen,
                                 UINT32 &maxCompressedLen,
                                 const CHAR *dictionary = NULL ) = 0 ;

      virtual INT32 compress( const CHAR *source, UINT32 sourceLen,
                              CHAR *dest, UINT32 &destLen,
                              const CHAR *dictionary = NULL ) = 0 ;
      virtual INT32 getUncompressedLen( const CHAR *source, UINT32 sourceLen,
                                        UINT32 &length) = 0 ;
      virtual INT32 decompress( const CHAR *source, UINT32 sourceLen,
                                CHAR *dest, UINT32 &destLen,
                                const CHAR *dictionary = NULL ) = 0 ;
   private:
      UTIL_COMPRESSOR_TYPE _type ;
   } ;
   typedef _utilCompressor utilCompressor ;

   utilCompressor* getCompressorByType( UTIL_COMPRESSOR_TYPE type ) ;
   const CHAR *utilCompressType2String( UINT8 type ) ;
}

#endif /* UTIL_COMPRESSOR__ */

