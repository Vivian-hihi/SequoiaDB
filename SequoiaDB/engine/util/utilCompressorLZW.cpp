#include "pd.hpp"
#include "utilCompressorLZW.hpp"

namespace engine
{
   #define MAX_DICT_CTX_NUM      32

   _utilCompressorLZW::_utilCompressorLZW()
      : _utilCompressor( UTIL_COMPRESSOR_LZW ),
        _dictionary( NULL )
   {
   }

   _utilCompressorLZW::~_utilCompressorLZW()
   {
      utilLZWContext *context = NULL ;

      while ( _vecContext.size() > 0 )
      {
         context = _vecContext.back() ;
         SDB_OSS_DEL context ;
         _vecContext.pop_back() ;
      }

      if ( _dictionary )
      {
         SDB_OSS_DEL _dictionary ;
      }
   }

   /* Get a compressor context ready. The dictionary will be set. */
   INT32 _utilCompressorLZW::prepare( utilCompressorContext &ctx )
   {
      INT32 rc = SDB_OK ;
      _utilLZWContext *context = NULL ;

      _vecCtxLatch.get() ;
      if ( _vecContext.size() > 0 )
      {
         context = _vecContext.back() ;
         _vecContext.pop_back() ;
      }
      _vecCtxLatch.release() ;

      if ( !context )
      {
         context = SDB_OSS_NEW _utilLZWContext ;
         PD_CHECK( context, SDB_OOM, error, PDERROR,
                   "Failed to allocate context for LZW, requested size: %d",
                   sizeof( _utilLZWContext ) ) ;
         context->setDictionary( _dictionary ) ;
      }

      ctx = ( utilCompressorContext )context ;
      _prepared  = TRUE ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCompressorLZW::prepareExt( utilCompressorContext context )
   {
      SDB_ASSERT( context, "Invalid argument" ) ;

      ((_utilLZWContext *)context)->setDictionary( _dictionary ) ;
      _prepared  = TRUE ;

      return SDB_OK ;
   }

   /*
    * Reset everything in the context except the dictionary. Then it can be used
    * directly to compress or decompress.
    */
   INT32 _utilCompressorLZW::rePrepare( utilCompressorContext ctx )
   {
      _utilLZWContext *context = ( _utilLZWContext * )ctx ;

      SDB_ASSERT( context, "Compressor context is invalid" ) ;
      SDB_ASSERT( context->isReady(), "Order of interface invoking is wrong" ) ;

      context->reset( TRUE ) ;

      return SDB_OK ;
   }

   INT32 _utilCompressorLZW::setDictionary( const CHAR * dict, UINT32 dictLen )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( dict && dictLen > 0, "Dictionary information is invalid" ) ;

      _dictionary = SDB_OSS_NEW _utilLZWDictionary ;
      PD_CHECK( _dictionary, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for compressor dictionary, "
                "requested size: %d", sizeof( _utilLZWDictionary ) ) ;

      rc = _dictionary->loadFromStream( dict, dictLen ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to set format dictionary for LZW, rc: %d", rc ) ;
   done:
      return rc ;
   error:
      if ( _dictionary )
      {
         SDB_OSS_DEL _dictionary ;
      }
      goto done ;
   }

   void _utilCompressorLZW::_freeDictionary()
   {
      if ( _dictionary )
      {
         SDB_OSS_DEL _dictionary ;
         _dictionary = NULL ;
      }
   }

   size_t _utilCompressorLZW::compressBound( size_t srcLen )
   {
      return ( _dictionary->getCodeSize() * srcLen ) / 8 + 1 ;
   }

   INT32 _utilCompressorLZW::compress( utilCompressorContext ctx,
                                       const CHAR* source, UINT32 sourceLen,
                                       CHAR* dest, UINT32 &destLen )
   {
      INT32 rc = SDB_OK ;

      rc = _lzw.encode( ( _utilLZWContext * )ctx, source,
                          sourceLen, dest, destLen ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to encode data using LZW, rc: %d", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCompressorLZW::decompress( utilCompressorContext ctx,
                                         const CHAR* source, UINT32 sourceLen,
                                         CHAR* dest, UINT32 &destLen )
   {
      INT32 rc = SDB_OK ;

      rc = _lzw.decode( ( _utilLZWContext * )ctx, source,
                          sourceLen, dest, destLen ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to decode data using LZW, rc: %d", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCompressorLZW::done( utilCompressorContext &ctx )
   {
      _utilLZWContext *context = ( _utilLZWContext * )ctx ;
      SDB_ASSERT( context, "Invalid compressor context" ) ;

      _vecCtxLatch.get() ;
      if ( _vecContext.size() < MAX_DICT_CTX_NUM )
      {
         context->reset( TRUE ) ;
         _vecContext.push_back( context ) ;
      }
      else
      {
         SDB_OSS_DEL context ;
      }
      _vecCtxLatch.release() ;

      ctx = UTIL_INVALID_COMP_CTX ;

      _prepared = FALSE ;

      return SDB_OK ;
   }
}

