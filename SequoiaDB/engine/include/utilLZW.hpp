#ifndef UTIL_LZW__
#define UTIL_LZW__

#include "core.hpp"
#include "oss.hpp"
#include "ossMem.hpp"
#include "ossUtil.h"
#include "pd.hpp"

namespace engine
{
   #define DICT_INVALID_NODE          4294967295
   #define DICT_NODE_VALID( code )    ( DICT_INVALID_NODE != code )
   #define DICT_BASE_NODE_CODE        256

   /*
    * Internally we use a 32bit buffer to do continuous bits reading and
    * writting, so the maximum node code should be less than (2^25(32M) - 1).
    */
   #define DICT_MAX_NODE_CODE        ( ( 1 << 25 ) - 1 )
   #define DICT_MAX_SIZE             ( 4 << 20 )

   /*
    * 0~255 represent 256 diffrent symbols(initial state of the dictionary),
    * duplicated strings can be handled only when more are added to the
    * dictionary.
    */
   #define MIN_NODE_NUM              ( 256 + 1 )
   #define MAX_STREAM_BUFF_SIZE      256

   typedef UINT32 LZW_CODE ;

   /*
    * Used to perform byte(bits) reading and writting of compressed stream data.
    * As code of each dictionary node will be stored as part of the dictionary,
    * we use as least bits to represent a node as possible in order to save
    * space( both in memory and on disk). The number of bits may be not multiple
    * of 8, but we store the stream byte by byte. So the bits of one code may
    * be handled in more than one shot. This buffer is used to handle the
    * combination of the code bits.
    */
   struct _utilBitBuffer
   {
      UINT32 _buf ;   /* Buffer for splitting and concencate bits of codes by
                       * shifting the bits in this buffer. */
      UINT32 _n ;     /* Number of bits remain in the buffer. */
   } ;
   typedef _utilBitBuffer utilBitBuffer ;

   /*
    * Dictionary node in LZW. Each node represents a string, which linked by
    * prev of each node. This node contains the last character of the string.
    * All nodes which mark this node as 'prev' are linked by their 'next', and
    * this parent node points to the first child using 'first'.
    */
   struct _utilLZWNode
   {
      LZW_CODE  _prev;   /* Code of the previous part of the string
                          * (the last character excluded) */
      LZW_CODE  _first;  /* first 'child' */
      LZW_CODE  _next;   /* first 'brother' */
      UINT32    _len ;   /* Length of the string. This is for getting acurate
                          * dest buffer when decoding. */
      UINT8     _ch;     /* Last character of the string this node represents.*/
      UINT8     _pad[3] ;

      void reset()
      {
         _prev = DICT_INVALID_NODE ;
         _first = DICT_INVALID_NODE ;
         _next = DICT_INVALID_NODE ;
         _len = 0 ;
      }
   } ;
   typedef _utilLZWNode utilLZWNode ;

   /* Dictionary head. */
   struct _utilLZWDictHead
   {
      UINT32 _codeSize;       /* Bit number to represent a code. */
      UINT32 _maxCode;        /* Maximum code in the dictionary. */
   } ;
   typedef _utilLZWDictHead utilLZWDictHead ;

   class _utilLZWDictionary : public SDBObject
   {
      typedef std::pair<UINT32, UINT32> NODE_REF_ITEM ;
      typedef std::vector<NODE_REF_ITEM> NODE_REF_NUM_VEC ;
      typedef NODE_REF_NUM_VEC::iterator NODE_REF_NUM_VEC_ITR ;

   public:
      _utilLZWDictionary()
      {
         _head._codeSize = 0 ;
         _head._maxCode = 0 ;
         _nodes = NULL ;
      }

      ~_utilLZWDictionary()
      {
         if ( _nodes )
         {
            SDB_OSS_FREE( _nodes ) ;
         }
      }

      /*
       * Initialize the dictionary. maxSize tells the maximum space the
       * dictionary is able to use, including dictionary head and all of its
       * nodes. dictOptimize specifies the stratage of building the dictionary.
       * If it's set to TRUE, internal dictionary rebuild will happen, which
       * takes a little more time, but may results in better compression ratio.
       */
      INT32 init( UINT32 maxSize, BOOLEAN dictOptimize = TRUE ) ;
      void reset() ;

      /*
       * Shrink the dictionary to the target size if its current size is greater
       * than that. It will kick out the 'not so good' dictionary items and keep
       * the better ones. This will only happen when dictOptimize for init is
       * TRUE.
       */
      INT32 shrink( UINT32 maxSize ) ;

      UINT32 getMaxNodeNum() { return _maxNodeNum ; }
      UINT32 getCodeSize() { return _head._codeSize ; }
      UINT32 getMaxCode() { return _head._maxCode ; }

      UINT32 getDictSize() ;

      /*
       * The dictionary is able to format to a stream, as well as building from
       * a correct format stream.
       * This is useful when you want to store the dictionary some where, and
       * use it again in future.
       */
      INT32 dumpToStream( CHAR *stream, UINT32 &length ) ;
      INT32 loadFromStream( const CHAR *stream, UINT32 len ) ;

      OSS_INLINE LZW_CODE addStr( LZW_CODE preCode, UINT8 ch ) ;
      OSS_INLINE LZW_CODE findStr( LZW_CODE preCode, UINT8 ch ) ;
      OSS_INLINE LZW_CODE findStrExt( LZW_CODE preCode, UINT8 ch ) ;
      OSS_INLINE UINT32 getStr( LZW_CODE code, UINT8 *buff, UINT32 bufSize ) ;

   private:
      OSS_INLINE void _removeStr( LZW_CODE code ) ;

      /*
       * Move the string from the current position to a new one. It will gain
       * a new code.
       */
      OSS_INLINE void _moveStr( LZW_CODE code, LZW_CODE newCode ) ;

      OSS_INLINE void _incNodeRef( LZW_CODE code ) ;

      OSS_INLINE void _initNodes(  _utilLZWNode *nodes, UINT32 nodeNum ) ;

      OSS_INLINE BOOLEAN _compByRefNum( const NODE_REF_ITEM &firstNode,
                                        const NODE_REF_ITEM &secondNode )
      {
         return firstNode.second < secondNode.second ;
      }

   private:
      _utilLZWDictHead _head ;   /* Header of the dictionary. */
      UINT32 _maxNodeNum ;       /* Maximum node number to fit the provided
                                  * dictionary size. */
      _utilLZWNode *_nodes ;     /* Nodes of the dictionary, which contain
                                  * strings in the dictionary. */
      std::vector<NODE_REF_ITEM> _vecNodeRefNum ; /* Reference number of each
                                                   * string. */
   } ;
   typedef _utilLZWDictionary utilLZWDictionary ;

   /*
    * For dictionary storage, both in memory and on disk. Dictionary header as
    * well as all the nodes will be stored together.
    */
   #define MIN_DICT_SIZE \
      ( sizeof( _utilLZWDictHead ) + sizeof( _utilLZWNode) * MIN_NODE_NUM )

   /*
    * Key structure for encoding and decoding, working together with utilLZW.
    */
   class _utilLZWContext : public SDBObject
   {
      friend class _utilLZWDictCreator ;
      friend class _utilLZW ;
   public:
      _utilLZWContext()
      {
         reset( FALSE ) ;
      }

      ~_utilLZWContext() {}

      OSS_INLINE BOOLEAN isReady() { return _ready ; }

      OSS_INLINE void setDictionary( _utilLZWDictionary *dict )
      {
         SDB_ASSERT( dict, "Dictionary is invalid" ) ;
         _dictionary = dict ;
         _ready = TRUE ;
      }

      _utilLZWDictionary* getDictionary() { return _dictionary ; }

      OSS_INLINE void reset( BOOLEAN keepDict )
      {
         if ( !keepDict )
         {
            _dictionary = NULL ;
            _ready = FALSE ;
         }

         _maxDictNodeNum = 0 ;
         _stream = NULL ;
         _streamLen = 0 ;
         _streamPos = 0 ;
         _bitBuf._buf = 0 ;
         _bitBuf._n = 0 ;
      }

   private:
      BOOLEAN _ready ;
      utilLZWDictionary *_dictionary ;   /* dictionary pointer */
      UINT32 _maxDictNodeNum ;
      UINT8 *_stream ;
      UINT32 _streamLen ;
      UINT32 _streamPos ;
      utilBitBuffer _bitBuf ;   /* bit buffer used during encoding and decoding */
   } ;
   typedef _utilLZWContext utilLZWContext ;

   class _utilLZWDictCreator : public SDBObject
   {
   public:
      _utilLZWDictCreator()
         : _maxDictSize( 0 ),
           _dictionary( NULL )
      {
         _ctx.reset( FALSE ) ;
      }

      ~_utilLZWDictCreator()
      {
         if ( _dictionary )
         {
            SDB_OSS_DEL _dictionary ;
         }
      }

      /* The following member functions are used to handle the dictionary. */
      /* Prepare for dictionary building, setting the maximum allowed size. */
      INT32 prepare( UINT32 maxSize ) ;

      void reset() ;

      /*
       * Use the source data to build the dictionary for LZW. It can be called
       * multiple times, and the dictionary will grow until reaching the maximum
       * size.
       * This function should be called after buildDictPrepare.
       */
      INT32 build( const CHAR *src, UINT32 srcLen, BOOLEAN &full ) ;

      /* Save the dictionary to the provided memory space. */
      INT32 save( CHAR *dictBuf, UINT32 &maxDictLen ) ;

      _utilLZWDictionary* getDictionary() { return _dictionary ; }

   private:
      UINT32 _maxDictSize ;
      _utilLZWDictionary *_dictionary ;
      _utilLZWContext _ctx ;
   } ;
   typedef _utilLZWDictCreator utilLZWDictCreator ;

   class _utilLZW : public SDBObject
   {
   public:
      OSS_INLINE INT32 encode( _utilLZWContext *ctx,
                               const CHAR *source, UINT32 sourceLen,
                               CHAR *destBuf, UINT32 &destLen ) ;

      OSS_INLINE INT32 decode( _utilLZWContext *ctx,
                               const CHAR *source, UINT32 sourceLen,
                               CHAR *destBuf, UINT32 &destLen );
   private:
      OSS_INLINE LZW_CODE _readCode( _utilLZWContext *ctx ) ;
      OSS_INLINE void _writeCode( _utilLZWContext *ctx, LZW_CODE code ) ;
      OSS_INLINE void _writeBits( _utilLZWContext *ctx, UINT32 bits,
                                  UINT32 codeSize ) ;
      OSS_INLINE UINT8 _readByte( _utilLZWContext *ctx ) ;
      OSS_INLINE void _writeByte( _utilLZWContext *ctx, UINT8 ch ) ;

      OSS_INLINE void _flushBits( _utilLZWContext *ctx ) ;
   } ;
   typedef _utilLZW utilLZW ;

   /* Add new string to the LZW dictionary. */
   OSS_INLINE LZW_CODE _utilLZWDictionary::addStr( LZW_CODE preCode, UINT8 ch )
   {
      LZW_CODE currCode ;
      LZW_CODE nextCode ;

      if ( _head._maxCode + 1 == _maxNodeNum )
      {
         /* Dictionary is full. */
         return DICT_INVALID_NODE ;
      }

      _head._maxCode++ ;
      /*
       * The current code size is not enough to represent the code, so increase
       * it by 1 bit.
       */
      if ( _head._maxCode == ( 1 << _head._codeSize ) )
      {
         _head._codeSize++ ;
      }

      _nodes[_head._maxCode]._prev = preCode ;
      _nodes[_head._maxCode]._first = DICT_INVALID_NODE ;

      if ( DICT_INVALID_NODE == _nodes[preCode]._first )
      {
         _nodes[preCode]._first = _head._maxCode ;
      }
      else
      {
         currCode = nextCode = _nodes[preCode]._first ;
         while ( ( DICT_INVALID_NODE != nextCode )
                 && ( ch < _nodes[nextCode]._ch ) )
         {
            currCode = nextCode ;
            nextCode = _nodes[currCode]._next ;
         }

         if ( currCode == _nodes[preCode]._first )
         {
            /* currCode not moved, then add the new node to the head. */
            _nodes[_head._maxCode]._next = _nodes[preCode]._first ;
            _nodes[preCode]._first = _head._maxCode;
         }
         else
         {
            /* Add the new node at the middle or end of the chain. */
            _nodes[_head._maxCode]._next = _nodes[currCode]._next ;
            _nodes[currCode]._next = _head._maxCode ;
         }
      }

      _nodes[_head._maxCode]._ch = ch ;
      _nodes[_head._maxCode]._len = _nodes[preCode]._len + 1 ;
      /*
       * Push the new node into the reference information vector, setting its
       * reference number to 1.
       */
      _vecNodeRefNum.push_back(std::make_pair(_head._maxCode, 1)) ;

      return _head._maxCode ;
   }

   OSS_INLINE LZW_CODE _utilLZWDictionary::findStr( LZW_CODE preCode, UINT8 ch )
   {
      LZW_CODE nextCode ;

      for ( nextCode = _nodes[preCode]._first; nextCode != DICT_INVALID_NODE;
            nextCode = _nodes[nextCode]._next )
      {
         if ( ch < _nodes[nextCode]._ch )
         {
            continue ;
         }

         if ( preCode == _nodes[nextCode]._prev && ch == _nodes[nextCode]._ch )
         {
            return nextCode ;
         }
         else
         {
            break ;
         }
      }

      return DICT_INVALID_NODE ;
   }

   OSS_INLINE LZW_CODE _utilLZWDictionary::findStrExt( LZW_CODE preCode,
                                                       UINT8 ch )
   {
      LZW_CODE code = findStr( preCode, ch ) ;
      if ( DICT_NODE_VALID( code ) )
      {
         /*
          * Once a code is found, increase its reference number. The first 256
          * nodes will be skipped.
          */
         _incNodeRef( code ) ;
      }

      return code ;
   }

   OSS_INLINE UINT32 _utilLZWDictionary::getStr( LZW_CODE code, UINT8 *buff,
                                                 UINT32 bufSize )
   {
      UINT32 strLen = _nodes[code]._len ;
      UINT32 i = strLen ;
      SDB_ASSERT( bufSize >= strLen, "Invalid argument, bufSize too small" ) ;

      while ( code != DICT_INVALID_NODE && i )
      {
         buff[--i] = _nodes[code]._ch ;
         code = _nodes[code]._prev ;
      }

      return strLen ;
   }

   OSS_INLINE void _utilLZWDictionary::_removeStr( LZW_CODE code )
   {
      LZW_CODE preCode = DICT_INVALID_NODE ;
      LZW_CODE currCode = DICT_INVALID_NODE ;

      SDB_ASSERT( code >= DICT_BASE_NODE_CODE,
                  "Invalid string code to remove" ) ;
      SDB_ASSERT( DICT_INVALID_NODE == _nodes[code]._first,
                  "The string to remove is prefix string of others" ) ;

      preCode = _nodes[code]._prev ;
      if ( DICT_INVALID_NODE != preCode )
      {
         if ( code == _nodes[preCode]._first )
         {
            _nodes[preCode]._first = _nodes[code]._next ;
         }
         else
         {
            currCode = _nodes[preCode]._first ;
            while ( _nodes[currCode]._next != code )
            {
               currCode = _nodes[currCode]._next ;
            }
            _nodes[currCode]._next = _nodes[code]._next ;
         }
      }

      _nodes[code].reset() ;
   }

   OSS_INLINE void _utilLZWDictionary::_moveStr( LZW_CODE code,
                                                 LZW_CODE newCode )
   {
      LZW_CODE preCode = DICT_INVALID_NODE ;
      LZW_CODE currCode = DICT_INVALID_NODE ;

      SDB_ASSERT( code >= DICT_BASE_NODE_CODE,
                  "Invalid string code to move" ) ;
      SDB_ASSERT( DICT_INVALID_NODE == _nodes[newCode]._prev
                  && DICT_INVALID_NODE == _nodes[newCode]._first
                  && DICT_INVALID_NODE == _nodes[newCode]._next,
                  "Target position is in use" ) ;

      ossMemcpy( &_nodes[newCode], &_nodes[code], sizeof( _utilLZWNode ) ) ;
      preCode = _nodes[code]._prev ;

      /* Change the nodes who 'point' to this node. */
      if ( DICT_INVALID_NODE != preCode )
      {
         if ( code == _nodes[preCode]._first )
         {
            _nodes[preCode]._first = newCode ;
         }
         else
         {
            currCode = _nodes[preCode]._first ;
            while ( _nodes[currCode]._next != code )
            {
               currCode = _nodes[currCode]._next ;
            }

            _nodes[currCode]._next = newCode ;
         }
      }

      /* Change the nodes which this node 'points to'. */
      currCode = _nodes[code]._first;
      if ( DICT_INVALID_NODE != currCode )
      {
         _nodes[currCode]._prev = newCode ;
         while ( DICT_INVALID_NODE != (currCode = _nodes[currCode]._next ) )
         {
            _nodes[currCode]._prev = newCode ;
         }
      }

      _nodes[code].reset() ;
   }

   OSS_INLINE void _utilLZWDictionary::_incNodeRef( LZW_CODE code )
   {
      SDB_ASSERT( code >= DICT_BASE_NODE_CODE && code <= _head._maxCode,
                  "Invalid code" ) ;

      /*
       * References of the first 256(0~255) items are not stored, as they are
       * always needed in the dictionary.
       */
      _vecNodeRefNum[code - DICT_BASE_NODE_CODE].second++ ;
   }

   OSS_INLINE void _utilLZWDictionary::_initNodes( _utilLZWNode *nodes,
                                                   UINT32 nodeNum )
   {
      for ( UINT32 i = 0; i < nodeNum; ++i )
      {
         nodes[i]._prev = DICT_INVALID_NODE ;
         nodes[i]._first = DICT_INVALID_NODE ;
         nodes[i]._next = DICT_INVALID_NODE ;
         /*
          * 1 byte(8 bits) can represent 256 characters. Every search will start
          * with them.
          */
         if ( i < 256 )
         {
            nodes[i]._ch = i ;
            nodes[i]._len = 1 ;
         }
         else
         {
            nodes[i]._ch = 0 ;
            nodes[i]._len = 0 ;
         }
      }
   }

   OSS_INLINE LZW_CODE _utilLZW::_readCode( _utilLZWContext *ctx )
   {
      UINT32 bits = 0 ;
      UINT32 codeSize = ctx->getDictionary()->getCodeSize() ;

      while ( ctx->_bitBuf._n < codeSize )
      {
         UINT8 ch = _readByte( ctx ) ;
         ctx->_bitBuf._buf = ( ctx->_bitBuf._buf << 8 ) | ch ;
         ctx->_bitBuf._n += 8 ;
      }

      ctx->_bitBuf._n -= codeSize ;
      bits = ( ctx->_bitBuf._buf >> ctx->_bitBuf._n )
             & (( 1 << codeSize ) - 1 ) ;

      return bits ;
   }

   OSS_INLINE void _utilLZW::_writeCode( _utilLZWContext *ctx, LZW_CODE code )
   {
      _writeBits( ctx, code, ctx->getDictionary()->getCodeSize() ) ;
   }

   OSS_INLINE void _utilLZW::_writeBits( _utilLZWContext *ctx, UINT32 bits,
                                         UINT32 bitNum)
   {
      ctx->_bitBuf._buf = ( ctx->_bitBuf._buf << bitNum )
                          | ( bits & (( 1 << bitNum ) - 1 ) ) ;

      bitNum += ctx->_bitBuf._n ;

      while ( bitNum >= 8 )
      {
         UINT8 ch ;
         bitNum -= 8 ;
         ch = ctx->_bitBuf._buf >> bitNum ;
         _writeByte( ctx, ch ) ;
      }

      ctx->_bitBuf._n = bitNum ;
   }


   OSS_INLINE UINT8 _utilLZW::_readByte( _utilLZWContext *ctx )
   {
      return ctx->_stream[ctx->_streamPos++] ;
   }

   OSS_INLINE void _utilLZW::_writeByte( _utilLZWContext *ctx, UINT8 ch )
   {
      ctx->_stream[ctx->_streamPos++] = ch ;
   }

   OSS_INLINE void _utilLZW::_flushBits( _utilLZWContext *ctx )
   {
      if ( ctx->_bitBuf._n )
      {
         _writeBits( ctx, 0, 8 - ctx->_bitBuf._n ) ;
      }
   }

   OSS_INLINE INT32 _utilLZW::encode( _utilLZWContext *ctx,
                                      const CHAR *source, UINT32 sourceLen,
                                      CHAR *destBuf, UINT32 &destLen )
   {
      INT32 rc = SDB_OK ;
      UINT8 ch = 0 ;
      UINT32 pos = 0 ;
      UINT32 strLen = 0 ;
      LZW_CODE code = DICT_INVALID_NODE ;
      LZW_CODE nextCode = DICT_INVALID_NODE ;
      utilLZWDictionary *dictionary = ctx->getDictionary() ;
      SDB_ASSERT( dictionary, "Compressor context is invalid" ) ;

      ctx->_stream = (UINT8* )destBuf ;
      ctx->_streamLen = destLen ;

      ch = source[0] ;
      code = ch ;
      pos++ ;
      strLen++ ;

      for ( ; pos < sourceLen; ++pos )
      {
         ch = source[pos] ;
         nextCode = dictionary->findStr( code, ch ) ;
         /*
          * If <code> + ch can not be found in the dictionary, write <code> to
          * the output stream. Otherwise make <code> = <code> + ch.
          */
         if ( DICT_INVALID_NODE == nextCode )
         {
            if ( code > dictionary->getMaxCode() )
            {
               SDB_ASSERT( 1 == 0, "invalid code" ) ;
            }
            _writeCode( ctx, code ) ;
            code = ch ;
            strLen = 1 ;
         }
         else
         {
            code = nextCode ;
            strLen++ ;
         }
      }

      /* Write the last code */
      if ( code > dictionary->getMaxCode() )
      {
         SDB_ASSERT( 1 == 0, "invalid code" ) ;
      }
      _writeCode( ctx, code ) ;
      _flushBits( ctx ) ;
      destLen = ctx->_streamPos;

      return rc ;
   }

   OSS_INLINE INT32 _utilLZW::decode( _utilLZWContext *ctx,
                                      const CHAR *source, UINT32 sourceLen,
                                      CHAR *destBuf, UINT32 &destLen )
   {
      INT32 rc = SDB_OK ;
      UINT32 strLen = 0 ;
      LZW_CODE code ;
      UINT32 totalOut = 0 ;
      utilLZWDictionary *dictionary = ctx->getDictionary() ;
      ctx->_stream = (UINT8 *)source ;
      ctx->_streamLen = sourceLen ;
      ctx->_streamPos = 0 ;

      for( ; ctx->_streamPos < ctx->_streamLen; )
      {
         code = _readCode( ctx ) ;
         SDB_ASSERT( code <= dictionary->getMaxCode(),
                     "Invalid code in data" ) ;
         strLen = dictionary->getStr( code, (UINT8*)(destBuf + totalOut),
                                      destLen - totalOut) ;
         totalOut += strLen ;
      }

      destLen = totalOut ;

      return rc ;
   }
}

#endif /* UTIL_LZW__ */

