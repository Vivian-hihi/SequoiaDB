#include "ossMem.hpp"
#include "pd.hpp"
#include "utilCompressorFactory.hpp"
#include "utilCompressorLZW.hpp"
#include "utilCompressorLZ4.hpp"
#include "utilCompressorZlib.hpp"

namespace engine
{
   INT32 _utilCompressorFactory::createCompressor( UTIL_COMPRESSOR_TYPE type,
                                                   utilCompressor *&compressor )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( UTIL_COMPRESSOR_LZW == type || UTIL_COMPRESSOR_LZ4 == type
                  || UTIL_COMPRESSOR_ZLIB == type,
                  "Compressor type not supported" ) ;

      if ( UTIL_COMPRESSOR_LZW == type )
      {
         compressor = SDB_OSS_NEW utilCompressorLZW ;
      }
      else if ( UTIL_COMPRESSOR_LZ4 == type )
      {
         compressor = SDB_OSS_NEW utilCompressorLZ4 ;
      }
      else
      {
         compressor = SDB_OSS_NEW utilCompressorZlib ;
      }

      PD_CHECK( compressor, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for compressor" ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   void utilCompressorFactory::destroyCompressor( utilCompressor* compressor )
   {
      if ( NULL != compressor )
      {
         SDB_OSS_DEL compressor;
      }
   }
}

