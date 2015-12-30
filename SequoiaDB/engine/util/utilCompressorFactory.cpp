#include "ossMem.hpp"
#include "pd.hpp"
#include "utilCompressorFactory.hpp"
#include "utilCompressorLZW.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"
//#include "utilCompressorLZ4.hpp"
//#include "utilCompressorZlib.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORFACTORY_CREATECOMPRESSOR, "_utilCompressorFactory::createCompressor" )
   INT32 _utilCompressorFactory::createCompressor( UTIL_COMPRESSOR_TYPE type,
                                                   utilCompressor *&compressor )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORFACTORY_CREATECOMPRESSOR ) ;
      SDB_ASSERT( UTIL_COMPRESSOR_LZW == type,
                  "Compressor type not supported" ) ;

       /*
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
      */

      compressor = SDB_OSS_NEW utilCompressorLZW ;
      PD_CHECK( compressor, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for compressor" ) ;
   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORFACTORY_CREATECOMPRESSOR, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORFACTORY_DESTROYCOMPRESSOR, "_utilCompressorFactory::destroyCompressor" )
   void utilCompressorFactory::destroyCompressor( utilCompressor* compressor )
   {
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORFACTORY_DESTROYCOMPRESSOR ) ;
      if ( NULL != compressor )
      {
         SDB_OSS_DEL compressor;
      }
      PD_TRACE_EXIT( SDB__UTILCOMPRESSORFACTORY_DESTROYCOMPRESSOR ) ;
   }
}

