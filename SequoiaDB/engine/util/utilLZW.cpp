#include "pd.hpp"
#include "utilCompressor.hpp"
#include "utilLZW.hpp"

namespace engine
{
   using namespace std;

   INT32 _lzwBuildDict( _utilLZWContext *ctx, const CHAR *source,
                     UINT32 sourceLen, CHAR *destBuf,
                     UINT32 &destLen )
   {
      LZW_CODE   code;
      unsigned isize = 0;
      unsigned strlen = 0;
      LZW_CODE nc ;
      LZW_CODE tmp ;
      UINT8    c;

      // TBD: Need to handle destLen
      //lzw_init(ctx, destBuf);
      ctx->stream = destBuf ;

      c = source[0] ;
      code = c;
      isize++;
      strlen++;

      for ( ; isize < sourceLen; ++isize )
      {
         c = source[ isize ] ;
         nc = _utilLZWFindStr(ctx, code, c);
         if (nc == NODE_NULL)
         {
            // the string was not found - write <prefix>
            _utilLZWWrite(ctx, code);

            // add <prefix>+<current symbol> to the dictionary
            tmp = _utilLZWAddStr(ctx, code, c);
            if (tmp == NODE_NULL)
            {
               /* Dictionary is full */
               break;
            }

            code = c;
            strlen = 1;
         }
         else
         {
            code = nc;
            strlen++;
         }
      }

      // write last code
      _utilLZWWrite(ctx, code);
      _utilLZWFlushBits(ctx);
      _utilLZWWriteBuf(ctx->stream + ctx->total_out - ctx->lzwn,
                   ctx->buff, ctx->lzwn);
      destLen = ctx->total_out ;

      return 0;
   }

   /******************************************************************************
   **  lzw_encode
   **  --------------------------------------------------------------------------
   **  Encodes input byte stream into LZW code stream.
   **
   **  Arguments:
   **      ctx  - LZW context;
   **      fin  - input file;
   **      fout - output file;
   **
   **  Return: error code
   ******************************************************************************/
   INT32 utilLZWEncode( _utilLZWContext *ctx, const CHAR *source, UINT32 sourceLen,
                      CHAR *destBuf, UINT32 &destLen )
   {
      LZW_CODE   code;
      unsigned isize = 0;
      unsigned strlen = 0;
      LZW_CODE nc ;
      LZW_CODE tmp ;
      UINT8    c;

      // TBD: Need to handle destLen
      //lzw_init(ctx, destBuf);
      ctx->stream = destBuf ;

      c = source[0] ;
      code = c;
      isize++;
      strlen++;

      for ( ; isize < sourceLen; ++isize )
      {
         c = source[ isize ] ;
         nc = _utilLZWFindStr(ctx, code, c);
         if (nc == NODE_NULL)
         {
            // the string was not found - write <prefix>
            _utilLZWWrite(ctx, code);
            code = c;
            strlen = 1;
         }
         else
         {
            code = nc;
            strlen++;
         }
      }

      // write last code
      _utilLZWWrite(ctx, code);
      _utilLZWFlushBits(ctx);
      _utilLZWWriteBuf(ctx->stream + ctx->total_out - ctx->lzwn,
                   ctx->buff, ctx->lzwn);
      destLen = ctx->total_out ;

      return 0;
   }

   #define MAX_TMP_BUFF_SIZE  ( 32 * 1024 )
   UINT8 buff[ MAX_TMP_BUFF_SIZE ] ;

   /******************************************************************************
   **  lzw_decode
   **  --------------------------------------------------------------------------
   **  Decodes input LZW code stream into byte stream.
   **
   **  Arguments:
   **      ctx  - LZW context;
   **      fin  - input file;
   **      fout - output file;
   **
   **  Return: error code
   ******************************************************************************/
   INT32 utilLZWDecode( _utilLZWContext *ctx,  const CHAR *source, UINT32 sourceLen,
                   CHAR *destBuf, UINT32 &destLen )
   {
      unsigned      isize = 0;
      LZW_CODE        code;
      UINT8 c;
      UINT32  destLength = 0 ;

      ctx->stream = (CHAR *)source ;
      ctx->streamRdPos = ctx->stream ;
      ctx->streamLen = sourceLen ;
      ctx->total_out = 0 ;

      for(;;)
      {
         unsigned      strlen;
         LZW_CODE        nc;

         nc = _utilLZWRead(ctx);

         // check input strean for EOF (lzwm == 0)
         if (!ctx->lzwm)
            break;

         // unknown code
         if (nc > ctx->max)
         {
            if (nc-1 == ctx->max)
            {
               code = NODE_NULL;
            }
            else
            {
               fprintf(stderr, "ERROR: wrong code %d, input %d\n", nc, isize);
               break;
            }
         }

         // get string for the new code from dictionary
         strlen = _utilLZWGetStr(ctx, nc, buff, sizeof(buff));
         // remember the first sybmol of this string
         c = buff[sizeof(buff) - strlen];
         memcpy( destBuf + destLength, buff + (sizeof(buff) - strlen ), strlen ) ;
         destLength += strlen ;

         code = nc;
         isize++;
      }

      destLen = destLength ;

      return 0;
   }

   void utilLZWSaveDict( const _utilLZWContext *ctx, CHAR *dictBuf, UINT32 &maxDictLen )
   {
      /*
       * Please refer to structure _utilLZWContext. The 'dict', 'max' and 'codesize'
       * members should be stored and used by compressor/decompressor later.
       * Besides that, store dictionary node number at the beginning.
       */
      UINT32 dictFinalLen = 0 ;
      UINT32 dictNodesLen = sizeof( utilLZWNode ) * ctx->dictNodeNum ;
      SDB_ASSERT( ( dictNodesLen + sizeof( UINT32 ) * 3 ) <= maxDictLen,
                  "Buffer for dictionary is too small" ) ;

      *( UINT32 * )dictBuf = ctx->dictNodeNum ;
      dictFinalLen = sizeof( UINT32 ) ;
      ossMemcpy( dictBuf + dictFinalLen, ctx->dict, dictNodesLen ) ;
      dictFinalLen += dictNodesLen ;
      *( UINT32 * )(dictBuf + dictFinalLen) = ctx->max ;
      dictFinalLen += sizeof( UINT32 ) ;
      *( UINT32 * )(dictBuf + dictFinalLen) = ctx->codesize ;
      dictFinalLen += sizeof( UINT32 ) ;

      maxDictLen = dictFinalLen ;
   }

   INT32 utilLZWLoadDict( _utilLZWContext *ctx, const CHAR *dict, UINT32 &dictLen )
   {
      INT32 rc = SDB_OK ;
      UINT32 readPos = 0 ;
      ctx->dictNodeNum = *( UINT32 *)dict ;
      readPos += sizeof( UINT32 ) ;
      ctx->dict = (utilLZWNode *)( dict + readPos ) ;
      readPos += sizeof( utilLZWNode ) * ctx->dictNodeNum ;
      ctx->max = *( UINT32 * )( dict + readPos ) ;
      readPos += sizeof( UINT32 ) ;
      ctx->codesize = *( UINT32 * )( dict + readPos ) ;

   done:
      return rc ;
   }

   INT32 utilLZWBuildDict( const CHAR *src, UINT32 srcLen,
                           CHAR *dict, UINT32 &maxDictLen )
   {
      INT32 rc = SDB_OK ;
      UINT32 dictNodeNum = 0 ;
      UINT32 tmpBufLen = srcLen ;
      _utilLZWContext * lzw_ctx = NULL ;
      CHAR *tmpBuf = ( CHAR * )SDB_OSS_MALLOC( tmpBufLen ) ;
      PD_CHECK( tmpBuf, SDB_OOM, error, PDERROR,
                "Failed to allocate temporary memory to build dictionary, "
                "requested size: %d", tmpBufLen ) ;

      dictNodeNum = ( maxDictLen - sizeof( UINT32 ) ) / ( sizeof( _utilLZWNode ) ) ;
      SDB_ASSERT( dictNodeNum > 0,
                  "max dictioinary length provided is invalid" ) ;
      lzw_ctx = ( _utilLZWContext * )SDB_OSS_MALLOC( sizeof(_utilLZWContext) ) ;
      PD_CHECK( lzw_ctx, SDB_OOM, error, PDERROR,
                "Failed to allocate memory to build dictionary, "
                "requested size: %d", sizeof(_utilLZWContext) ) ;

      utilLZWInit( lzw_ctx, dictNodeNum ) ;
      lzw_ctx->dictNodeNum = dictNodeNum ;
      rc = _lzwBuildDict( lzw_ctx, src, srcLen, tmpBuf, tmpBufLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to encode sample data to build "
                   "dictionary, rc: %d", rc ) ;
      utilLZWSaveDict( lzw_ctx, dict, maxDictLen ) ;

   done:
      if ( tmpBuf )
      {
         SDB_OSS_FREE( tmpBuf ) ;
      }
      if ( lzw_ctx )
      {
         SDB_OSS_FREE( lzw_ctx ) ;
      }
      return rc ;
   error:
      goto done ;
   }
}

