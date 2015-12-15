#include "pd.hpp"
#include "utilCompressor.hpp"
#include "utilLZW.hpp"

#define CALCULATE_NODE_NUM( maxSize ) \
   ( ( (maxSize) - sizeof( _utilLZWDictHead ) ) / sizeof( _utilLZWNode ) )

namespace engine
{
   INT32 _utilLZWDictionary::init_old( UINT32 maxNodeNum )
   {
      INT32 rc = SDB_OK ;

      _maxNodeNum = maxNodeNum <= DICT_MAX_NODE_CODE ?
                    maxNodeNum : DICT_MAX_NODE_CODE;
      _nodes = ( _utilLZWNode * )SDB_OSS_MALLOC(
                                       sizeof( _utilLZWNode ) * maxNodeNum ) ;
      PD_CHECK( _nodes, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for dictionary items, "
                "requested size: %d", sizeof( _utilLZWNode ) * maxNodeNum );

      for ( UINT32 i = 0; i < maxNodeNum; ++i )
      {
         _nodes[i]._prev = DICT_INVALID_NODE ;
         _nodes[i]._first = DICT_INVALID_NODE ;
         _nodes[i]._next = DICT_INVALID_NODE ;
         /*
          * 1 byte(8 bits) can represent 256 characters. Every search will start
          * with them.
          */
         if ( i < 256 )
         {
            _nodes[i]._ch = i ;
            _nodes[i]._len = 1 ;
         }
         else
         {
            _nodes[i]._len = 0 ;
         }
      }

      _head._maxCode = 255 ;
      _head._codeSize = 8 ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilLZWDictionary::init( UINT32 maxSize )
   {
      INT32 rc = SDB_OK ;
      UINT32 maxNodeNum = CALCULATE_NODE_NUM( maxSize ) ;

      maxNodeNum = maxNodeNum <= DICT_MAX_NODE_CODE ?
                    maxNodeNum : DICT_MAX_NODE_CODE;
      /* Increase the node number 3 times, for further shrink. */
      maxNodeNum = maxNodeNum * 3 ;
      _nodes = ( _utilLZWNode * )SDB_OSS_MALLOC(
                                       sizeof( _utilLZWNode ) * maxNodeNum ) ;
      PD_CHECK( _nodes, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for dictionary items, "
                "requested size: %d", sizeof( _utilLZWNode ) * maxNodeNum );

      _initNodes( _nodes, maxNodeNum ) ;

      _maxNodeNum = maxNodeNum ;
      _head._maxCode = 255 ;
      _head._codeSize = 8 ;

   done:
      return rc ;
   error:
      _maxNodeNum = 0 ;
      goto done ;
   }

   void _utilLZWDictionary::reset()
   {
      _head._codeSize = 0 ;
      _head._maxCode = 0 ;
      if ( _nodes )
      {
         SDB_OSS_FREE( _nodes ) ;
         _nodes = NULL ;
      }
   }

   INT32 _utilLZWDictionary::shrink( UINT32 maxSize )
   {
      INT32 rc = SDB_OK ;
      UINT32 rmNodeNum = 0 ;
      UINT32 holeNum = 0 ;
      UINT32 sequare = 0 ;
      LZW_CODE rmNodeCode = DICT_INVALID_NODE ;
      std::vector<UINT32> vecRmNodes ;
      NODE_REF_NUM_VEC_ITR endPos ;
      utilLZWNode *newNodes = NULL ;
      LZW_CODE origCode = DICT_INVALID_NODE ;
      LZW_CODE newCode = DICT_INVALID_NODE ;
      LZW_CODE traceCode = DICT_INVALID_NODE ;
      UINT32 refNum = 0 ;
      UINT32 square = 0 ;
      UINT32 maxNodeNum = CALCULATE_NODE_NUM( maxSize ) ;

      /*
       * Shrink the dictionary to contain nodeNum nodes. It will pick up the
       * 'best' nodes in the dictionary, those who have greater reference
       * values. That would results in better compression ratio.
       */

      if ( maxNodeNum > _head._maxCode + 1 )
      {
         goto done ;
      }

      /*
       * Create another dictionary with maxNodeNum, format all the content of
       * the dictionary into it, and release the old one.
       */

      // sort by reference number increase
      std::stable_sort( _vecNodeRefNum.begin(), _vecNodeRefNum.end(),
                        _cmpByRefNum ) ;

      newNodes = ( utilLZWNode *)
                  SDB_OSS_MALLOC( sizeof(utilLZWNode) * maxNodeNum ) ;
      PD_CHECK( newNodes, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for dictionary, requested size: %d",
                sizeof(utilLZWNode) * maxNodeNum ) ;

      _initNodes( newNodes, maxNodeNum ) ;

      newCode = DICT_BASE_NODE_CODE - 1 ;

      //for ( origCode = _vecNodeRefNum.pop_back().first;


      while ( _vecNodeRefNum.size() > 0 )
      {
         origCode = _vecNodeRefNum.back().first ;
         refNum = _vecNodeRefNum.back().second ;

         /* If reference number is only 1, throw it away. */
         if ( refNum < 2 )
         {
            break ;
         }

         _vecNodeRefNum.pop_back() ;

         if ( newCode + _nodes[origCode]._len > maxNodeNum )
         {
            break ;
         }

         newCode++ ;

         /* Only the longest strings will be stored, not their sub-strings. */
         if ( DICT_INVALID_NODE != _nodes[origCode]._first )
         {
            continue ;
         }

         newNodes[newCode]._ch = _nodes[origCode]._ch ;
         traceCode = _nodes[origCode]._prev ;
         if ( traceCode < DICT_BASE_NODE_CODE )
         {
            if ( DICT_INVALID_NODE == newNodes[traceCode]._first )
            {
               newNodes[traceCode]._first = newCode ;
            }
            else
            {
               traceCode = newNodes[newNodes[traceCode]._first]._next ;
               while ( DICT_INVALID_NODE != newNodes[traceCode]._next )
                  ;
               newNodes[traceCode]._next = newCode ;
            }
         }
      }

      _head._maxCode = newCode ;
      if ( 0 == newCode % 2 )
      {
         ossIsPowerOf2( newCode, &square ) ;
      }
      else
      {
         ossIsPowerOf2( newCode + 1, &square ) ;
      }

      _head._codeSize = square + 1 ;
      SDB_OSS_FREE( _nodes ) ;
      _nodes = newNodes ;


#if 0


      rmNodeNum = ( _head._maxCode + 1 ) - maxNodeNum ;

      endPos =  _vecNodeRefNum.begin() + rmNodeNum ;
      for ( NODE_REF_NUM_VEC_ITR itr = _vecNodeRefNum.begin();
            itr < endPos; ++itr )
      {
         vecRmNodes.push_back( itr->first ) ;
      }

      /*
       * Remove the nodes from the reference information vector, and sort it
       * in code increasing order.
       */
      _vecNodeRefNum = NODE_REF_NUM_VEC( _vecNodeRefNum.begin() + rmNodeNum,
                                         _vecNodeRefNum.end() ) ;

      /* Recover the node reference number vector, in code increasing order. */
      std::sort( _vecNodeRefNum.begin(), _vecNodeRefNum.end(),
                 _cmpByNodeNum ) ;

      /*
       * Now remove the nodes from the dictionary.
       */
      std::sort( vecRmNodes.begin(), vecRmNodes.end() ) ;
      for ( std::vector<UINT32>::iterator itr = vecRmNodes.begin();
            itr != vecRmNodes.end(); ++itr )
      {
         rmNodeCode = *itr ;
         _removeStr( rmNodeCode ) ;
         holeNum++ ;
         while ( ++rmNodeCode < *( itr + 1 ) )
         {
            _moveStr( rmNodeCode, rmNodeCode - holeNum ) ;
         }
      }

      _head._maxCode = maxNodeNum - 1 ;
      ossIsPowerOf2( _head._maxCode, &sequare ) ;
      _head._codeSize = sequare + 1 ;
#endif

   done:
      return rc ;
   error:
      if ( newNodes )
      {
         SDB_OSS_FREE( newNodes ) ;
      }
      goto done ;
   }

   UINT32 _utilLZWDictionary::getDictSize()
   {
      return sizeof( _utilLZWDictHead ) +
             sizeof( _utilLZWNode ) * ( _head._maxCode + 1 ) ;
   }

   INT32 _utilLZWDictionary::dumpToStream( CHAR *stream, UINT32 &length )
   {
      INT32 rc = SDB_OK ;
      UINT32 pos = 0 ;
      UINT32 nodeTotalSize = sizeof( _utilLZWNode ) * ( _head._maxCode + 1 ) ;
      UINT32 totalSize = sizeof( _utilLZWDictHead ) + nodeTotalSize ;

      SDB_ASSERT( stream, "Buffer dor dictionary is invalid" ) ;
      PD_CHECK( length >= totalSize, SDB_INVALIDARG, error, PDERROR,
                "Length of dictionary buffer is invalid, requested size: %d, "
                "actual size: %d", totalSize, length ) ;

      *(UINT32 *)stream = _head._codeSize ;
      pos += sizeof( UINT32 ) ;
      *(UINT32 *)(stream + pos) = _head._maxCode ;
      pos += sizeof( UINT32 ) ;
      ossMemcpy( stream + pos, (CHAR *)_nodes, nodeTotalSize ) ;
      length = totalSize ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilLZWDictionary::loadFromStream( const CHAR *stream, UINT32 len )
   {
      INT32 rc = SDB_OK ;
      UINT32 readPos = 0 ;
      UINT32 totalNodeSize = 0 ;

      _head._codeSize = *( UINT32 * )stream ;
      readPos += sizeof( UINT32 ) ;
      _head._maxCode = *( UINT32 * )( stream + readPos ) ;
      readPos += sizeof( UINT32 ) ;

      totalNodeSize = sizeof( utilLZWNode ) * ( _head._maxCode + 1 ) ;

      SDB_ASSERT( len == ( sizeof(UINT32) * 2 + totalNodeSize ),
                  "Dictionary data is invalid" ) ;

      _nodes = ( _utilLZWNode * )SDB_OSS_MALLOC( totalNodeSize ) ;
      PD_CHECK( _nodes, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for compressor dictionary, "
                "requested size: %d", totalNodeSize ) ;
      ossMemcpy( _nodes, stream + sizeof( UINT32 ) * 2, totalNodeSize ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilLZWDictCreator::prepare( UINT32 maxSize )
   {
      INT32 rc = SDB_OK ;

      PD_CHECK( maxSize >= MIN_DICT_SIZE, SDB_INVALIDARG, error, PDERROR,
                "Dictionary size provided is too small: %d. The mininum requred"
                " size is: %d", maxSize, MIN_DICT_SIZE ) ;

      _dictionary = SDB_OSS_NEW utilLZWDictionary ;
      PD_CHECK( _dictionary, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for creating dictionry, "
                "requested size: %d", maxSize ) ;

      _dictionary->init( maxSize ) ;
      _ctx.setDictionary( _dictionary ) ;
      _maxDictSize = maxSize ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _utilLZWDictCreator::reset()
   {
      _maxDictSize = 0 ;
      if ( _dictionary )
      {
         SDB_OSS_DEL _dictionary ;
         _dictionary = NULL ;
      }

      _ctx.reset( FALSE ) ;
   }

   INT32 _utilLZWDictCreator::build( const CHAR *source, UINT32 sourceLen,
                                     BOOLEAN &full )
   {
      INT32 rc = SDB_OK ;

      UINT8 ch = 0 ;
      UINT32 pos = 0 ;
      UINT32 startPos = 0 ;
      UINT32 strLen = 0 ;
      LZW_CODE code = DICT_INVALID_NODE ;
      LZW_CODE nextCode = DICT_INVALID_NODE ;
      LZW_CODE maxCode = _ctx.getDictionary()->getMaxNodeNum() - 1 ;

      (void)full ;
      ch = source[0] ;
      code = ch ;
      pos++ ;
      startPos = 0 ;
      strLen++ ;
      full = FALSE ;

      for ( ; pos < sourceLen; ++pos )
      {
         ch = source[pos] ;
         //nextCode = _dictionary->findStr( code, ch ) ;
         nextCode = _dictionary->findStrExt( code, ch ) ;
         if ( DICT_INVALID_NODE == nextCode )
         {
            nextCode = _dictionary->addStr( code, ch ) ;
            if ( nextCode == maxCode )
            {
               /* Dictionary is full */

               // shrink the dictionary to the target size
               _dictionary->shrink( _maxDictSize ) ;
               full = TRUE ;
               goto done ;
               /* After shrink, restart the search of the current string. */
               //code = source[startPos] ;
               //pos = startPos ;
               //continue ;
               //nextCode = _dictionary->addStr( code, ch ) ;
               /*
                * After shrink, insertting of string into dictionary should
                * always succeed.
                */
                /*
               SDB_ASSERT( DICT_INVALID_NODE != nextCode,
                           "Adding string to dictionary failed" ) ;
                */
               //full = TRUE ;
               //goto done ;
            }

            code = ch ;
            strLen = 1 ;
            startPos = pos ;
         }
         else
         {
            code = nextCode ;
            strLen++ ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilLZWDictCreator::save( CHAR *dictBuf, UINT32 &maxDictLen )
   {
      INT32 rc = SDB_OK ;

      // shrink the dictionary to target size
      //_dictionary->shrink( _maxDictSize ) ;
      rc = _dictionary->dumpToStream( dictBuf, maxDictLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get LZW dictionary, rc: %d", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}

