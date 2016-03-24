#include "pd.hpp"
#include "utilCompressorLZW.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW_CONSTRUCTOR, "_utilCompressorLZW::_utilCompressorLZW" )
   _utilCompressorLZW::_utilCompressorLZW()
      : _utilCompressor( UTIL_COMPRESSOR_LZW )
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW_DESTRUCTOR, "_utilCompressorLZW::~_utilCompressorLZW" )
   _utilCompressorLZW::~_utilCompressorLZW()
   {
   }

   INT32 _utilCompressorLZW::compressBound( UINT32 srcLen,
                                            UINT32 &maxCompressedLen,
                                            const CHAR *dictionary )
   {
      /*
       * In the worst scenario, no string in the source with length greater than
       * 1 can be found in the dictionary. In this case, each character in the
       * source should be represented by one dictionary code separately. If the
       * code size is greater than 8, the data will expand after encoding...
       * 4 more bytes are reserved at the beginning to store the original length
       */
      INT32 rc = SDB_OK ;

      if ( !dictionary )
      {
         PD_LOG( PDERROR, "Dictionary is required for lzw" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      maxCompressedLen =
         ( (( utilLZWDictHead *)dictionary)->_codeSize * srcLen  + 7 ) / 8
         + sizeof( UINT32 ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCompressorLZW::compress( const CHAR *source, UINT32 sourceLen,
                                       CHAR *dest, UINT32 &destLen,
                                       const CHAR *dictionary )
   {
      INT32 rc = SDB_OK ;
      utilLZWContext context ;
      utilLZWDictionary dict ;

      dict.attach( dictionary ) ;
      context.setDictionary( &dict ) ;

      rc = _lzw.encode( &context, source, sourceLen, dest, destLen ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to encode data using LZW, rc = %d", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCompressorLZW::getUncompressedLen( const CHAR *source,
                                                 UINT32 sourceLen,
                                                 UINT32 &length )
   {
      length = *(UINT32 *)source ;

      return SDB_OK ;
   }

   INT32 _utilCompressorLZW::decompress( const CHAR *source, UINT32 sourceLen,
                                         CHAR *dest, UINT32 &destLen,
                                         const CHAR *dictionary )
   {
      INT32 rc = SDB_OK ;
      utilLZWContext context ;
      utilLZWDictionary dict ;

      dict.attach( dictionary ) ;
      context.setDictionary( &dict ) ;

      rc = _lzw.decode( &context, source, sourceLen, dest, destLen ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to decode data using LZW, rc = %d", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}

