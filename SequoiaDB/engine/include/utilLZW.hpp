#ifndef UTIL_LZW__
#define UTIL_LZW__
#include "core.hpp"
#include "oss.hpp"
#include "ossMem.hpp"
#include "ossUtil.h"
#include "pd.hpp"

namespace engine
{
   #define NODE_NULL            (-1)
   #define MAX_STREAM_BUFF_SIZE 256

   typedef UINT32 LZW_CODE;
   /* Used to perform byte(bits) reading and writting of compressed stream data. */
   struct _utilBitBuffer
   {
      UINT32 buf;
      UINT32 n;
   } ;
   typedef _utilBitBuffer utilBitBuffer ;

   /* LZW node, represents a string */
   struct _utilLZWNode
   {
      LZW_CODE  prev;
      LZW_CODE  first;
      LZW_CODE  next;
      UINT8     ch;
   } ;
   typedef _utilLZWNode utilLZWNode ;

   /* LZW context, used to maintain information during the compression and decompression */
   struct _utilLZWContext
   {
      utilLZWNode      *dict;             // Dictionary builded by provided data.
      UINT32            max;              // maximal code in the dictionary
      UINT32            codesize;         // number of bits in code
      _utilBitBuffer    bb;
      CHAR             *stream;           // pointer to the stream for reading or writting
      UINT32            streamLen;        // Stream buffer length
      CHAR             *streamRdPos;      // Position for reading in the stream buffer.
      UINT32            total_out;        // total size write to stream(encode only)
      UINT32            lzwn;             // buffer byte counter
      UINT32            lzwm;             // buffer size (decoder only)
      UINT8             buff[MAX_STREAM_BUFF_SIZE];        // stream buffer, adjust its size if you need
      UINT32            dictNodeNum ;
   } ;
   typedef _utilLZWContext utillzwContext ;

   OSS_INLINE void _utilLZWWriteBuf(void *stream, UINT8 *buf, UINT32 size)
   {
      ossMemcpy( stream, buf, size ) ;
   }

   OSS_INLINE void _utilLZWReadBuf(void *stream, UINT8 *buf, UINT32 size)
   {
      ossMemcpy( buf, stream, size ) ;
   }

   static OSS_INLINE void _utilLZWWriteByte(_utilLZWContext *ctx, const UINT8 b)
   {
      ctx->buff[ctx->lzwn++] = b ;
      ctx->total_out++ ;

      if (ctx->lzwn == sizeof(ctx->buff)) {
         ctx->lzwn = 0;
         _utilLZWWriteBuf( ctx->stream + ctx->total_out - sizeof(ctx->buff),
                           ctx->buff, sizeof(ctx->buff));
      }
   }

   static OSS_INLINE UINT8 _utilLZWReadByte(_utilLZWContext *ctx)
   {
      UINT32 streamRemainLen = 0 ;

      if (ctx->lzwn == ctx->lzwm)
      {
         streamRemainLen = ctx->streamLen - ( ctx->streamRdPos - ctx->stream ) ;
         if ( 0 == streamRemainLen )
         {
            ctx->lzwm = 0 ;
            ctx->lzwn = 0 ;
         }
         else
         {
            ctx->lzwm = streamRemainLen > sizeof(ctx->buff) ?
                        sizeof(ctx->buff) : streamRemainLen ;
            _utilLZWReadBuf( ctx->streamRdPos, ctx->buff, ctx->lzwm ) ;
            ctx->streamRdPos += ctx->lzwm ;
            ctx->lzwn = 0;
         }
      }

      return ctx->buff[ctx->lzwn++] ;
   }

   /******************************************************************************
   **  _utilLZWWriteBits
   **  --------------------------------------------------------------------------
   **  Write bits into bit-buffer.
   **  The number of bits should not exceed 24.
   **
   **  Arguments:
   **      ctx     - pointer to LZW context;
   **      bits    - bits to write;
   **      nbits   - number of bits to write, 0-24;
   **
   **  Return: -
   ******************************************************************************/
   static OSS_INLINE void _utilLZWWriteBits(_utilLZWContext *ctx, UINT32 bits, UINT32 nbits)
   {
      // shift old bits to the left, add new to the right
      ctx->bb.buf = (ctx->bb.buf << nbits) | (bits & ((1 << nbits)-1));

      nbits += ctx->bb.n;

      // flush whole bytes
      while (nbits >= 8) {
         UINT8 b;

         nbits -= 8;
         b = ctx->bb.buf >> nbits;

         _utilLZWWriteByte(ctx, b);
      }

      ctx->bb.n = nbits;
   }

   /******************************************************************************
   **  _utilLZWReadBits
   **  --------------------------------------------------------------------------
   **  Read bits from bit-buffer.
   **  The number of bits should not exceed 24.
   **
   **  Arguments:
   **      ctx     - pointer to LZW context;
   **      nbits   - number of bits to read, 0-24;
   **
   **  Return: bits
   ******************************************************************************/
   static OSS_INLINE UINT32 _utilLZWReadBits(_utilLZWContext *ctx, UINT32 nbits)
   {
      UINT32 bits;

      // read bytes
      while (ctx->bb.n < nbits) {
             UINT8 b = _utilLZWReadByte(ctx);

             // shift old bits to the left, add new to the right
             ctx->bb.buf = (ctx->bb.buf << 8) | b;
             ctx->bb.n += 8;
      }

      ctx->bb.n -= nbits;
      bits = (ctx->bb.buf >> ctx->bb.n) & ((1 << nbits)-1);

      return bits;
   }

   /******************************************************************************
   **  _utilLZWFlushBits
   **  --------------------------------------------------------------------------
   **  Flush bits into bit-buffer.
   **  If there is not an integer number of bytes in bit-buffer - add zero bits
   **  and write these bytes.
   **
   **  Arguments:
   **      pbb     - pointer to bit-buffer context;
   **
   **  Return: -
   ******************************************************************************/
   static OSS_INLINE void _utilLZWFlushBits(_utilLZWContext *ctx)
   {
      if ( ctx->bb.n )
      {
         _utilLZWWriteBits(ctx, 0, 8-ctx->bb.n);
      }
   }


   /******************************************************************************
   **  lzw_init
   **  --------------------------------------------------------------------------
   **  Initializes LZW context.
   **
   **  Arguments:
   **      ctx     - LZW context;
   **      stream  - Pointer to Input/Output stream object;
   **
   **  RETURN: -
   ******************************************************************************/
   OSS_INLINE INT32 utilLZWInit( _utilLZWContext *ctx, UINT32 dictNodeNum )
   {
      UINT32 rc = SDB_OK ;
      UINT32 i;

      ossMemset(ctx, 0, sizeof(*ctx));

      ctx->dict = ( utilLZWNode * )SDB_OSS_MALLOC( sizeof( utilLZWNode ) * dictNodeNum ) ;
      PD_CHECK( ctx->dict, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for dictionary, requested size: %d",
                sizeof( utilLZWNode ) * dictNodeNum ) ;

      for (i = 0; i < dictNodeNum; i++)
      {
         ctx->dict[i].prev = NODE_NULL;
         ctx->dict[i].first = NODE_NULL;
         ctx->dict[i].next = NODE_NULL;
         if ( i < 256 )
         {
         ctx->dict[i].ch = i;
         }
      }

      ctx->max = 255;
      ctx->codesize = 8;
   done:
      return rc ;
   error:
      goto done ;
   }

   /******************************************************************************
   **  _utilLZWFindStr
   **  --------------------------------------------------------------------------
   **  Searches a string in LZW dictionaly. It is used only in encoder.
   **
   **  Arguments:
   **      ctx  - LZW context;
   **      code - code for the string beginning (already in dictionary);
   **      c    - last symbol;
   **
   **  RETURN: code representing the string or NODE_NULL.
   ******************************************************************************/
   static OSS_INLINE LZW_CODE _utilLZWFindStr(_utilLZWContext *ctx, LZW_CODE code, UINT8 ch )
   {
      LZW_CODE nc;

      for (nc = ctx->dict[code].first; nc != NODE_NULL; nc = ctx->dict[nc].next)
      {
         if ( ch < (UINT8)ctx->dict[nc].ch )
         {
            continue ;
         }
         else
         {
            if (code == ctx->dict[nc].prev && ch == ctx->dict[nc].ch)
            {
               return nc;
            }
            else
            {
               break ;
            }
         }
      }

      return NODE_NULL;
   }

   /******************************************************************************
   **  _utilLZWGetStr
   **  --------------------------------------------------------------------------
   **  Reads string from the LZW dictionaly. Because of particular dictionaty
   **  structure the buffer is filled from the end so the offset from the
   **  beginning of the buffer will be <buffer size> - <string size>.
   **
   **  Arguments:
   **      ctx  - LZW context;
   **      code - code of the string (already in dictionary);
   **      buff - buffer for the string;
   **      size - the buffer size;
   **
   **  Return: the number of bytes in the string
   ******************************************************************************/
   static OSS_INLINE UINT32 _utilLZWGetStr( _utilLZWContext *ctx, LZW_CODE code,
                                           UINT8 buff[], UINT32 size )
   {
      UINT32 i = size;

      while (code != NODE_NULL && i)
      {
         buff[--i] = ctx->dict[code].ch;
         code = ctx->dict[code].prev;
      }

      return size - i;
   }

   /******************************************************************************
   **  _utilLZWAddStr
   **  --------------------------------------------------------------------------
   **  Adds string to the LZW dictionaly.
   **  It is important that codesize is increased after the code was sent into
   **  the output stream.
   **
   **  Arguments:
   **      ctx  - LZW context;
   **      code - code for the string beginning (already in dictionary);
   **      c    - last symbol;
   **
   **  RETURN: code representing the string or NODE_NULL if dictionary is full.
   ******************************************************************************/
   static OSS_INLINE LZW_CODE _utilLZWAddStr(_utilLZWContext *ctx, LZW_CODE code, UINT8 ch )
   {
      LZW_CODE currCode ;
      LZW_CODE nextCode ;

      ctx->max++;

      if (ctx->max >= ctx->dictNodeNum)
           return NODE_NULL;

      ctx->dict[ctx->max].prev = code;
      ctx->dict[ctx->max].first = NODE_NULL;

      if ( NODE_NULL == ctx->dict[code].first )
      {
         ctx->dict[code].first = ctx->max ;
      }
      else
      {
         currCode = nextCode = ctx->dict[code].first;
         while ( ( NODE_NULL != nextCode ) && ( ch < ctx->dict[nextCode].ch ) )
         {
            currCode = nextCode ;
            nextCode = ctx->dict[currCode].next ;
         }

         if ( currCode == ctx->dict[code].first )
         {
            /* currCode not moved, then add the new node to the head.*/
            ctx->dict[ctx->max].next = ctx->dict[code].first ;
            ctx->dict[code].first = ctx->max ;
         }
         else
         {
            /* Add the new node at the middle or end of the chain. */
            ctx->dict[ctx->max].next = ctx->dict[currCode].next ;
            ctx->dict[currCode].next = ctx->max ;
         }
      }

      ctx->dict[ctx->max].ch = ch ;

      return ctx->max;
   }

   /******************************************************************************
   **  _utilLZWWrite
   **  --------------------------------------------------------------------------
   **  Writes an output code into the stream.
   **  This function is used only in encoder.
   **
   **  Arguments:
   **      ctx  - LZW context;
   **      code - code for the string;
   **
   **  RETURN: -
   ******************************************************************************/
   static OSS_INLINE void _utilLZWWrite(_utilLZWContext *ctx, LZW_CODE code)
   {
      // increase the code size (number of bits) if needed
      if (ctx->max == (1 << ctx->codesize))
             ctx->codesize++;

      _utilLZWWriteBits(ctx, code, ctx->codesize);
   }

   /******************************************************************************
   **  _utilLZWRead
   **  --------------------------------------------------------------------------
   **  Reads a code from the input stream. Be careful about where you put its
   **  call because this function changes the codesize.
   **  This function is used only in decoder.
   **
   **  Arguments:
   **      ctx  - LZW context;
   **
   **  RETURN: code
   ******************************************************************************/
   static OSS_INLINE LZW_CODE _utilLZWRead(_utilLZWContext *ctx)
   {
      // increase the code size (number of bits) if needed
      if (ctx->max+1 == (1 << ctx->codesize))
             ctx->codesize++;

      return _utilLZWReadBits(ctx, ctx->codesize);
   }

   OSS_INLINE void utilLzwReset( _utilLZWContext *ctx, BOOLEAN keepDict )
   {
      if ( keepDict )
      {
         memset( (CHAR *)&(ctx->bb), 0, sizeof(_utilLZWContext) -
                 ((CHAR *)&(ctx->bb) - (CHAR*)ctx));
      }
      else
      {
         if ( ctx->dict )
         {
            SDB_OSS_FREE( ctx->dict ) ;
            ctx->dict = NULL ;
         }
         memset( ctx, 0, sizeof( _utilLZWContext ) ) ;
      }
   }

   INT32 utilLZWBuildDict( const CHAR *src, UINT32 srcLen,
                       CHAR *dict, UINT32 &maxDictLen ) ;
   void utilLZWSaveDict( const _utilLZWContext *ctx, CHAR *dictBuf, UINT32 &maxDictLen ) ;
   INT32 utilLZWLoadDict( _utilLZWContext *ctx, const CHAR *dict, UINT32 &dictLen ) ;
   int utilLZWEncode( _utilLZWContext *ctx, const CHAR *source, UINT32 sourceLen,
                      CHAR *destBuf, UINT32 &destLen ) ;
   int utilLZWDecode( _utilLZWContext *ctx,  const CHAR *source, UINT32 sourceLen,
                      CHAR *destBuf, UINT32 &destLen );

}

#endif /* UTIL_LZW__ */

