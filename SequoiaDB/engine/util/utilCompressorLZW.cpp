#include "pd.hpp"
#include "utilCompressorLZW.hpp"

namespace engine
{
   _utilCompressorLZW::_utilCompressorLZW()
      : _utilCompressor( UTIL_COMPRESSOR_LZW )
   {
      ossMemset( &_ctx, 0, sizeof(_utilLZWContext) ) ;
   }

   _utilCompressorLZW::~_utilCompressorLZW()
   {
   }

   INT32 _utilCompressorLZW::prepare( UTIL_COMPRESSOR_WORK workType )
   {
      INT32 rc = SDB_OK ;

   done:
      return rc ;
   }

   INT32 _utilCompressorLZW::setDictionary( const CHAR * dict, UINT32 dict_size,
                                            BOOLEAN copy )
   {
      INT32 rc = SDB_OK ;

      utilLzwReset( &_ctx, FALSE ) ;
      rc = utilLZWLoadDict( &_ctx, dict, dict_size ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to load dictionary for lzw, rc: %d", rc ) ;
   done:
      return rc;
   error:
      goto done ;
   }

   size_t _utilCompressorLZW::compressBound( size_t srcLen )
   {
      // TBD:
      return ( srcLen * 2 ) ;
   }

   INT32 _utilCompressorLZW::compress( const CHAR* source, UINT32 sourceSize,
                                       CHAR* dest, UINT32 &destSize )
   {
      INT32 rc = SDB_OK ;

      utilLzwReset( &_ctx, TRUE ) ;
      rc = utilLZWEncode( &_ctx, source, sourceSize, dest, destSize ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to encode data using LZW, rc: %d", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCompressorLZW::decompress( const CHAR* source, UINT32 sourceSize,
                                         CHAR* dest, UINT32 &destSize )
   {
      INT32 rc = SDB_OK ;

      utilLzwReset( &_ctx, TRUE ) ;
      rc = utilLZWDecode( &_ctx, source, sourceSize, dest, destSize ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to decode data using LZW, rc: %d", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}

