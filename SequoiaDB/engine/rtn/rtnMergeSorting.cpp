/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnMergeSorting.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains declare for runtime
   functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnMergeSorting.hpp"
#include "dmsRecord.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"
#include "pmdEDU.hpp"

namespace engine
{
   INT32 _rtnMergeBlock::next( _dmsTmpBlkUnit *unit,
                               _rtnSortTuple **tuple )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( _read <= _loadSize, "impossible" )

      do
      {
         if ( _loadSize - _read < sizeof( INT32 ) )
         {
            rc = _loadData( unit ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }
         else if ( _loadSize - _read <
                   *((INT32 *)( _buf + _read )))
         {
            rc = _loadData( unit ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }
         else
         {
            *tuple = ( _rtnSortTuple *)(_buf + _read) ;
            _read += (*tuple)->len() ;
            break ;
         }
      } while ( TRUE ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnMergeBlock::init( _dmsTmpBlk &blk, CHAR *begin, UINT64 size )
   {
      SDB_ASSERT( NULL != begin && DMS_RECORD_USER_MAX_SZ * 2 <= size,
                  "impossible" )
      _blk = blk ;
      _buf = begin ;
      _size = size ;
      _read = 0 ;
      _loadSize = 0 ;
      return  ;
   }

   INT32 _rtnMergeBlock::_loadData( _dmsTmpBlkUnit *unit )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( _read <= _loadSize, "impossible" )
      UINT64 lastLen = _loadSize - _read ;

      if ( 0 != lastLen )
      {
         /// keep the last obj in buf.
         ossMemcpy( _buf, _buf + _read, lastLen ) ;
      }

      rc = unit->read( _blk, _size - lastLen,
                       _buf + lastLen, _loadSize ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      _loadSize += lastLen ;
      _read = 0 ;
      SDB_ASSERT( _loadSize <= _size, "impossible" )
   done:
      return rc ;
   error:
      goto done ;
   }

///////////_rtnMergeSorting
   _rtnMergeSorting::_rtnMergeSorting( _dmsTmpBlkUnit *unit,
                                       const BSONObj &orderby )
   :_order( Ordering::make(orderby) ),
    _buf( NULL ),
    _size( 0 ),
    _mergeBufSize( 0 ),
    _mergePos( 0 ),
    _heap(_order),
    _mergeBlkSize( 0 ),
    _mergeMax(0),
    _src( NULL ),
    _mergeDone( FALSE),
    _unit( unit )
   {
      SDB_ASSERT( NULL != _unit, "impossible" )
      _unitHelper._originalSize = _unit->totalSize() ;
      _unitHelper._outputStart = _unitHelper._originalSize ;
   }

   _rtnMergeSorting::~_rtnMergeSorting()
   {
   }

   INT32 _rtnMergeSorting::init( CHAR *buf, UINT64 size,
                                 RTN_SORT_BLKS &src )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != buf && size != 0, "impossible" )

      UINT32 mergeSize = size / ( DMS_RECORD_USER_MAX_SZ * 2 );
      if ( mergeSize < RTN_SORT_MIN_MERGESIZE )
      {
         PD_LOG( PDERROR, "buf is too small to merge records." ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( RTN_SORT_MAX_MERGESIZE < mergeSize )
      {
         _mergeMax = RTN_SORT_MAX_MERGESIZE ;
      }
      else
      {
         _mergeMax = mergeSize ;
      }

      _src = &src ;
      _buf = buf ;
      _size = size ;

      PD_LOG( PDDEBUG, "number of blks[%d], size of buf[%lld]"
              " max merge size[%d]"
             , _src->size(), _size, _mergeMax ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnMergeSorting::fetch( BSONObj &next, _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != _buf && 0 < _size && NULL != _src,
                  "impossible" )

      _rtnMergeTuple node ;
      if ( !_mergeDone )
      {
         rc = _merge( cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to merge records:%d", rc ) ;
            goto error ;
         }

         rc = _makeHeap( *_src, cb ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         _mergeDone = TRUE ;
      }

      rc = _heap.pop( node ) ;
      if ( SDB_OK == rc )
      {
         SDB_ASSERT( NULL != node.tuple() && NULL != node.tuple()->obj(),
                     "can not be NULL" )
         BSONObj popped( node.obj() ) ;
         ossMemcpy( _buf, popped.objdata(), popped.objsize()) ;
         try
         {
             next = BSONObj( _buf ) ;
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         /// TODO: if there is only one blk left, we do not need to push record to
         /// heap.
         rc = _pushObjFromSink( node.getI() ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to push obj to heap:%d", rc ) ;
            goto error ;
         }
         else
         {
            /// do noting.
         }
      }
      else if ( SDB_CLS_EMPTY_HEAP != rc )
      {
         PD_LOG( PDERROR, "failed to fetch next from heap:%d", rc ) ;
         goto error ;
      }
      else
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnMergeSorting::_merge( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      RTN_SORT_BLKS *dst = &_merged ;
      RTN_SORT_BLKS *tmp = NULL ;
      while ( _mergeMax - 1 < _src->size() )
      {
         rc = _mergeBlks( *_src, *dst, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to merge blks:%d", rc ) ;
            goto error ;
         }

         SDB_ASSERT( _src->empty() && !dst->empty(), "impssible" )
         tmp = _src ;
         _src = dst ;
         dst = tmp ;

         SDB_ASSERT( 0 == _unit->totalSize() % 2, "impossible" )
         SDB_ASSERT( _unitHelper._outputStart == _unit->totalSize() ||
                     _unitHelper._outputStart == _unit->totalSize() / 2,
                     "impossible" )
         if ( _unitHelper._outputStart == _unit->totalSize() )
         {
            _unitHelper._outputStart = 0 ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnMergeSorting::_mergeBlks( RTN_SORT_BLKS &src,
                                       RTN_SORT_BLKS &dst,
                                       _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      _rtnMergeTuple node ;

      while ( !src.empty() )
      {
         if ( cb->isInterrupted() )
         {
            PD_LOG( PDERROR, "cb is interrupted" ) ;
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         rc = _makeHeap( src, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to make heap:%d", rc ) ;
            goto error ;
         }

         while ( TRUE )
         {
            if ( _heap.more() )
            {
               rc = _heap.pop( node ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "failed to pop from heap:%d", rc ) ;
                  goto error ;
               }

               SDB_ASSERT( 0 < node.getI() && NULL != node.tuple(), "impossible" )
               SDB_ASSERT( _mergePos <= _mergeBufSize, "impossible" )

               if ( node.tuple()->len() <=
                    _mergeBufSize - _mergePos )
               {
                  ossMemcpy( _buf + _mergePos,
                             node.tuple(),
                             node.tuple()->len());
                  _mergePos += node.tuple()->len() ;
               }
               /// if the buf is full. out put to unit.
               else
               {
                  rc = _unit->seek( _unitHelper._outputStart ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDERROR, "failed to seek file:%d", rc ) ;
                     goto error ;
                  }

                  rc = _unit->write( _buf, _mergePos,
                                     _unitHelper._outputStart ==
                                     _unit->totalSize() ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDERROR, "failed to write to unit:%d", rc ) ;
                     goto error ;
                  }

                  PD_LOG( PDDEBUG, "output start[%lld], [%lld] bytes "
                          "were mv to file at this time",
                          _unitHelper._outputStart, _mergePos ) ;


                  _unitHelper._outputStart += _mergePos ;
                  _unitHelper._newBlkSize += _mergePos ;

                  /// reset mem info.
                  ossMemcpy( _buf, node.tuple(), node.tuple()->len() ) ;
                  _mergePos = node.tuple()->len() ;
               }

               rc = _pushObjFromSink( node.getI() ) ;
               if ( SDB_DMS_EOC == rc )
               {
                  rc = SDB_OK ;
               }
               else if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "failed to push obj to heap:%d", rc ) ;
                  goto error ;
               }
               else
               {
                  /// do nothing.
               }
            }
            else
            {
               if ( 0 != _mergePos )
               {
                  rc = _unit->seek( _unitHelper._outputStart ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDERROR, "failed to seek file:%d", rc ) ;
                     goto error ;
                  }

                  rc = _unit->write( _buf, _mergePos,
                                     _unitHelper._outputStart ==
                                     _unit->totalSize() ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDERROR, "failed to write to unit:%d", rc ) ;
                     goto error ;
                  }

                  _unitHelper._outputStart += _mergePos ;
                  _unitHelper._newBlkSize += _mergePos ;

                  _mergePos = 0 ;
               }
               break ;
            }
         } // while (TRUE)

         /// one merge done.
         dmsTmpBlk blk ;
         rc = _unit->buildBlk( _unitHelper._outputStart - _unitHelper._newBlkSize,
                               _unitHelper._newBlkSize,
                               blk ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build block:%d", rc ) ;
            goto error ;
         }

         PD_LOG( PDDEBUG, "build new blk[%s]", blk.toString().c_str() ) ;

         dst.push_back( blk ) ;
         _unitHelper._newBlkSize = 0 ;

      }// whiel (!src.empty())
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnMergeSorting::_makeHeap( RTN_SORT_BLKS &src,
                                      _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      UINT32 blkSize = src.size() + 1 ;
      _mergeBlkSize = blkSize < _mergeMax ?
                      blkSize : _mergeMax ;
      _mergeBufSize = _size / _mergeBlkSize ;
      PD_LOG( PDDEBUG, "build [%d] blks to heap at tihs time.", _mergeBlkSize - 1 ) ;
      /// start from 1. 0 is out put buf.
      for ( UINT32 i = 1; i < _mergeBlkSize; i++ )
      {
         if ( cb->isInterrupted() )
         {
            PD_LOG( PDERROR, "cb is interrupted" ) ;
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         PD_LOG( PDDEBUG, "put the [%d] blk [%s] to the heap",
                 i, src.front().toString().c_str() ) ;

         /// init data sink
         _dataSink[i].init( src.front(),
                            _buf + i * _mergeBufSize,
                            _mergeBufSize );
         rc = _pushObjFromSink( i ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         src.pop_front() ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnMergeSorting::_pushObjFromSink( UINT32 i )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( 0 < i && i < _mergeBlkSize, "impossible" )
      _rtnSortTuple *tuple = NULL ;
      rc = _dataSink[i].next( _unit, &tuple ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      rc = _heap.push( _rtnMergeTuple( i, tuple)) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
   done:
      return  rc ;
   error:
      goto done ;
   }
}

