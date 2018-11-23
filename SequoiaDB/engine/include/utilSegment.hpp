/******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = utilSegment.hpp

   Descriptive Name = Segment manager Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/10/2018  JT  Initial Draft

   Last Changed =

******************************************************************************/
#ifndef UTIL_SEGMENT_HPP__
#define UTIL_SEGMENT_HPP__

#include <vector>
#include "ossLatch.hpp"  // ossSpinXLatch
#include "ossMem.hpp"
#include "ossUtil.hpp"

typedef UINT32 UTIL_OBJIDX ;

// using namespace std ;
#define UTIL_INVALID_OBJ_INDEX   (( UTIL_OBJIDX )( -1 ))

namespace engine
{

//
//
// segment 0: a segment is an array of object ( T )
//   +-------------+-------------+   +-------------+
//   | obj 0       | obj 1       |...| obj n - 1   |
//   +-------------+-------------+   +-------------+
//
// segment 1:
//   +-------------+-------------+   +-------------+
//   | obj n+0     | obj n+1     |...| obj n+n-1   |
//   +-------------+-------------+   +-------------+
//
//   ...
//
// segment m:
//   +-------------+-------------+   +-------------+
//   | obj m*n + 0 | obj m*n + 1 |...| obj m*n+n-1 |
//   +-------------+-------------+   +-------------+
//
// _list: an array of index ( object index in segments )
//
//    0     1    2    3          m*n + n-1
//   +----+----+----+----+      +----+
//   | -1 | -1 |  2 | 73 | ...  | -1 |
//   +----+----+----+----+      +----+
//               ^                ^
//               |                |
//              _begin            _end
//
// _begin: the position in _list where to acquire a free object 
// _end  : the position in _list where to release/return an object

   #define _SEGMENT_OBJ_EYE_CATCHER    ( ( UINT32 ) 0xBEEFCAFE )
   #define _SEGMENT_OBJ_FLAG_ACQUIRED  ( ( UINT32 ) 0x0F0F     )
   #define _SEGMENT_OBJ_FLAG_RELEASED  ( ( UINT32 ) 0xF0F0     )

   #define IS_VALID_SEG_OBJ_INDEX( _objectIndex_ ) \
      ( UTIL_INVALID_OBJ_INDEX != _objectIndex_ )

   template < class T >
   class _utilSegmentManager : public SDBObject
   {
   private :
      struct _objX : public SDBObject
      {
         UINT32 _eyeCatcher ;
         UINT32 _flag ;
         T      _obj ;

         _objX()
         {
            _eyeCatcher = _SEGMENT_OBJ_EYE_CATCHER ;
            _flag       = _SEGMENT_OBJ_FLAG_RELEASED ;
         }
      } ;

      UTIL_OBJIDX *  _list  ;         // a list of obj indices to a segment
      UTIL_OBJIDX    _begin ;         // the position in list where acquire from
      UTIL_OBJIDX    _end ;           // the position in list where release to
      UTIL_OBJIDX    _numOfObjs ;     // total number of objects
      UTIL_OBJIDX    _delta ;         // number of objects in a segment
      UTIL_OBJIDX    _maxNumOfObjs ;  // max number of objects
      UTIL_OBJIDX    _acquiredCounter;// acquired objects counter
      UTIL_OBJIDX    _exponent ;      // exponent when round up to power of 2
      ossSpinXLatch  _latch ;
      std::vector< _objX * > _segList;// a list of segments, each segment is
                                      // an array of object(T)
      BOOLEAN        _isInitialized ;

   public :
      _utilSegmentManager() : _numOfObjs(0),
                              _maxNumOfObjs(0),
                              _acquiredCounter(0),
                              _delta(0),
                              _begin(0),
                              _end(0),
                              _isInitialized( FALSE ),
                              _exponent(0),
                              _list(NULL) { }

      ~_utilSegmentManager() ;

   private :
      // add a new segment and expand the _list when no free objects. 
      INT32 _expandList( const UTIL_OBJIDX delta ) ;

      BOOLEAN _isFull() const
      {
         return _numOfObjs - 1 <= _acquiredCounter ? TRUE : FALSE ;
      }

      BOOLEAN _isUpToLimit() const
      {
         return _numOfObjs >= _maxNumOfObjs ? TRUE : FALSE ;
      }

      // get an objX address by its index
      _objX * _getObjXByIndex( const UTIL_OBJIDX idx )
      {
         _objX * pObj     = NULL ;
         _objX * pSegList = NULL ;
         UTIL_OBJIDX i    = 0 ;
         UTIL_OBJIDX j    = 0 ;

         SDB_ASSERT( ( idx < _numOfObjs ) && IS_VALID_SEG_OBJ_INDEX( idx ),
                     "Invalid object index." ) ;
         if ( idx < _numOfObjs )
         {
            // i = idx / _delta ;
            // j = idx % _delta ;
            // the _delta is round up to nearest power of 2,
            // so divide and modulo can be optimized
            i = idx >> _exponent ;
            j = idx & ( _delta - 1 ) ;
            pSegList = ( _objX * )( _segList[i] ) ;
            if ( pSegList )
            {
               pObj  = ( _objX * )&( pSegList[j] ) ;
            }
         }
         return pObj ;
      }


      // round up to nearest power of 2
      // input :
      //     n  -- number to calculate
      // output :
      //     exponent -- exponent when round n up to the nearest power of 2
      // return:
      //   round up to nearest power of 2
      UTIL_OBJIDX _nextPowerOf2( UTIL_OBJIDX n, UTIL_OBJIDX & exponent )
      {
         UTIL_OBJIDX result = 0 ;
         BOOLEAN isPowerOf2 = FALSE ;
         exponent = 0 ;
         if ( ( n > 0 ) && ( ! ( n & ( n - 1 ) ) ) )
         {
            result = n ;
            isPowerOf2 = TRUE ;
         }
         while ( n != 0 )
         {
            n >>= 1 ;
            exponent ++ ;
         }
         if ( ! isPowerOf2 )
         {
            result = ( 1 << exponent ) ;
         }
         else
         {
            exponent -- ;
         }
         return result ;
      }


   public :
      // initialization
      //   numberOfObjs    : number of objects to allocate in a segemnt.
      //   maxNumberOfObjs : the max number of objects.
      INT32 init( const UTIL_OBJIDX numberOfObjs,
                  const UTIL_OBJIDX maxNumberOfObjs ) ;

      // free all object allocated in segments
      void fini() ;

      // get total number of objects without latch
      UTIL_OBJIDX getNumOfObjectsNoSync() ;

      // get total number of objects with latch
      UTIL_OBJIDX getNumOfObjects() ;

      // get an object address by its index
      T * getObjPtrByIndex( const UTIL_OBJIDX idx ) ;

      // get an object index from its address
      UTIL_OBJIDX getIndexByAddr ( const T * pT ) ;

      // acquire/apply a free object from the segments
      //   idx : the object index ( to the segment, the object arrary )
      //   pT  : the pointer of the object
      // when _acquiredCounter is equal to '_numOfObjs - 1', means
      // lack of free objects and a new segment ( array of object T ) will
      // be added and the _list will be expanded.
      INT32 acquire( UTIL_OBJIDX & idx,  T * &pT ) ;

      // acquire/apply a free object from the segments
      // pT : the pointer of the object
      INT32 acquire( T * &pT ) ;

      // relase/return an object to the segments
      //   idx : object index
      INT32 release( const UTIL_OBJIDX idx ) ;

      // release/return an object to the segments by its address
      //   pT : address / pointer of the object
      INT32 release( const T * pT ) ;

   #ifdef _DEBUG
      UTIL_OBJIDX * getListAddr() { return _list ; } 
   #endif
   } ;

   template < class T >
   UTIL_OBJIDX _utilSegmentManager< T >::getIndexByAddr( const T * pT )
   {
      UTIL_OBJIDX idx  = UTIL_INVALID_OBJ_INDEX ;

      if ( NULL != pT )
      {
         UTIL_OBJIDX segs = _segList.size() ;
         _objX  * pSegList = NULL ;
         for ( UTIL_OBJIDX i = 0; i < segs ; i++ )
         {
            pSegList = _segList[ i ] ;
            if (    pSegList
                 && ( pT >= &( pSegList[ 0 ]._obj ) )
                 && ( pT <= &( pSegList[ _delta - 1 ]._obj ) ) )
            {
               idx = i * _delta + 
                     ((CHAR*)pT - (CHAR*)&(pSegList[0]._obj)) / sizeof( _objX );
               break ;
            }
         }
      }
      return idx ; 
   }

   template < class T >
   void _utilSegmentManager< T >::fini()
   {
      if ( _isInitialized )
      {
         UTIL_OBJIDX len = _segList.size() ;
         for ( UTIL_OBJIDX i = 0; i < len ; i++ )
         {
            if ( _segList[i] )
            {
               SDB_OSS_DEL [] ( _segList[i] ) ;
            }
            _segList[i] = NULL ;
         }
         _segList.clear() ;
         SAFE_OSS_FREE( _list ) ;
         _list = NULL ;
         _isInitialized = FALSE ;
      }
   }

   template < class T >
   _utilSegmentManager< T >::~_utilSegmentManager() 
   {
      if ( _isInitialized )
      {
         fini();
      }
   }

   template < class T >
   INT32 _utilSegmentManager< T >::_expandList( const UTIL_OBJIDX delta )
   {
      INT32         rc       = SDB_OK ;
      UTIL_OBJIDX * pListTmp = NULL ;
      _objX       * pSegTmp  = NULL ;
      UTIL_OBJIDX   newSize  = _numOfObjs + delta ;

      if (    _isInitialized
           && ( UTIL_INVALID_OBJ_INDEX != newSize )
           && ( newSize <= _maxNumOfObjs )
           && ( 0 != delta ) )
      {
         pSegTmp  = SDB_OSS_NEW _objX[ delta ] ;
         pListTmp = ( UTIL_OBJIDX * )SDB_OSS_MALLOC( sizeof( UTIL_OBJIDX ) *
                                                     newSize ) ;
         if ( ( NULL != pListTmp ) && ( NULL != pSegTmp ) )
         {
            // copy old _list content
            ossMemcpy( pListTmp, _list, sizeof( UTIL_OBJIDX ) * _numOfObjs ) ;

            // initiate the newly allocated portion
            for ( UTIL_OBJIDX i = 0 ; i < delta ; i++ )
            {
               // initialize the list
               pListTmp[ _numOfObjs + i ] = _numOfObjs + i - 1 ;
               // initialize the _objX
               pSegTmp[i]._eyeCatcher     = _SEGMENT_OBJ_EYE_CATCHER ;
               pSegTmp[i]._flag           = _SEGMENT_OBJ_FLAG_RELEASED ;
            }

            // move _begin and _end to new position
            _begin = _numOfObjs ;
            _end   = 0 ;

            // set _numOfObjs to new size
            _numOfObjs = newSize ;

            // discard old _list
            SAFE_OSS_FREE( _list ) ;

            // assign _list with new allocation, pListTmp
            _list = pListTmp ;

            // add new segment to segment list
            _segList.push_back( pSegTmp );
         }
         else
         {
            SAFE_OSS_FREE( pListTmp ) ;
            if ( NULL != pSegTmp )
            {
               SDB_OSS_DEL [] pSegTmp ;  
            }
            rc = SDB_OOM ;
         }
      }
      else
      {
         // exceed the lock resorce limitation
         rc = SDB_OSS_UP_TO_LIMIT ;
      }
      return rc ;
   }

   template < class T >
   INT32 _utilSegmentManager< T >::init
   (
      const UTIL_OBJIDX numberOfObjs,
      const UTIL_OBJIDX maxNumberOfObjs
   )
   {
      INT32   rc      = SDB_OK ;
      _objX * pSegTmp = NULL ;

      if (    ( UTIL_INVALID_OBJ_INDEX != numberOfObjs )
           && ( UTIL_INVALID_OBJ_INDEX != maxNumberOfObjs )
           && ( 0 != numberOfObjs )
           && ( 0 != maxNumberOfObjs )
           && ( numberOfObjs <= maxNumberOfObjs ) )
      {
         // round up numberOfObjs to the nearest power of 2
         _delta        = _nextPowerOf2( numberOfObjs, _exponent ) ;
         _numOfObjs    = _delta ;
         _maxNumOfObjs = maxNumberOfObjs ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // allcoate memory
      pSegTmp = SDB_OSS_NEW _objX[ _numOfObjs ] ;
      _list = (UTIL_OBJIDX *)SDB_OSS_MALLOC( sizeof(UTIL_OBJIDX) * _numOfObjs );
      if ( ( NULL != _list ) && ( NULL != pSegTmp ) )
      {
         _begin = 0 ;
         for ( UTIL_OBJIDX i = 0 ; i < _numOfObjs ; i++ )
         {
            // initialize the list
            _list[ i ] = i ;
            // initialize the _objX
            pSegTmp[i]._eyeCatcher = _SEGMENT_OBJ_EYE_CATCHER ;
            pSegTmp[i]._flag       = _SEGMENT_OBJ_FLAG_RELEASED ;
         }
         _list[ _numOfObjs - 1 ] = UTIL_INVALID_OBJ_INDEX ;
         _end = _numOfObjs - 1 ;
         _segList.push_back( pSegTmp ) ;
         _isInitialized = TRUE ;
      }
      else
      {
         if ( NULL != pSegTmp )
         {
            SDB_OSS_DEL [] pSegTmp ;
            pSegTmp = NULL ;
         }
         SAFE_OSS_FREE( _list  ) ;
         _list = NULL ;
         rc = SDB_OOM ;
      }
   done:
      return rc ;
   error:
      goto done;
   }

   template < class T >
   UTIL_OBJIDX _utilSegmentManager< T >::getNumOfObjectsNoSync()
   {  
      return _numOfObjs ;
   }

   template < class T >
   UTIL_OBJIDX _utilSegmentManager< T >::getNumOfObjects()
   {  
      UTIL_OBJIDX numOfObjs = 0 ;

      _latch.get() ;
      numOfObjs = _numOfObjs ;
      _latch.release() ;

      return numOfObjs ;
   }

   template < class T >
   T * _utilSegmentManager< T >::getObjPtrByIndex( const UTIL_OBJIDX idx )
   {
      T * pObj          = NULL ;
      _objX *pObjX      = _getObjXByIndex( idx ) ;
      if ( pObjX )
      {
         pObj = ( T * )&( pObjX->_obj ) ;
      }
      return pObj ;
   }

   template < class T >
   INT32 _utilSegmentManager< T >::acquire( UTIL_OBJIDX & idx,  T * &pT )
   {
      INT32   rc        = SDB_OK;
      BOOLEAN bLatched  = FALSE ;
      _objX * pObjX     = NULL ;

      //         *       .
      // -       -       -       -      -
      //         ^               ^
      //         |  ==>          |
      //         begin   next    end
      // when _acquiredCounter is equal to '_numOfObjs - 1' ( i.e,
      // '_begin + 1' == '_end' ), means lack of free objects and we will
      // add a new segment and expand the _list
      if ( _isInitialized && ( NULL != _list ) )
      {
         _latch.get() ;
         bLatched = TRUE ;

         if ( _isFull() ) // need to expand
         {
            if ( _isUpToLimit() )
            {
               // exceed limitation
               rc = SDB_OSS_UP_TO_LIMIT ;
               goto error ;
            }

            rc = _expandList( _delta ) ;
            if ( rc )
            {
               goto error ;
            }
         }

         if ( _begin == _end )
         {
            rc = SDB_SYS ;
            goto error ;
         }

         // sanity check
         pObjX = _getObjXByIndex( _list[ _begin ] ) ;
         if (    ( NULL != pObjX )
              && ( _SEGMENT_OBJ_EYE_CATCHER   == pObjX->_eyeCatcher )
              && ( _SEGMENT_OBJ_FLAG_RELEASED == pObjX->_flag ) )
         {
            // mark the _objX flag
            pObjX->_flag = _SEGMENT_OBJ_FLAG_ACQUIRED ;
         }
         else
         {
            idx = UTIL_INVALID_OBJ_INDEX ;
            pT  = NULL ;
            rc  = SDB_SYS ;
            goto error ;
         }

         // get the obj index( to the segment, the object array )
         idx = _list[ _begin ] ;

         // mark this slot is empty with invalid index number, -1
         _list[ _begin ] = UTIL_INVALID_OBJ_INDEX ;

         // advance to next position
         _begin = ( _begin + 1 ) % _numOfObjs ;

         // increase the acquired counter
         _acquiredCounter ++ ;

         // return the object address
         pT = ( T * )&( pObjX->_obj ) ;
      }
      else
      {
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      if ( bLatched )
      {
         _latch.release() ;
      }
      return rc ;
   error:
      goto done ;
   }

   template < class T >
   INT32 _utilSegmentManager< T >::acquire( T * &pT )
   {
      UTIL_OBJIDX idx = UTIL_INVALID_OBJ_INDEX ;
      INT32  rc = acquire( idx, pT ) ;
      return rc ;
   }

   template < class T >
   INT32 _utilSegmentManager< T >::release( const T * pT )
   {
      INT32 rc   = SDB_OK ;
      UTIL_OBJIDX idx = getIndexByAddr( pT ) ;

      if ( UTIL_INVALID_OBJ_INDEX != idx )
      {
         rc = release( idx ) ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
      }
      return rc ;
   }

   template < class T >
   INT32 _utilSegmentManager< T >::release( const UTIL_OBJIDX idx )
   {
      INT32   rc           = SDB_OK ;
      BOOLEAN bLatched     = FALSE ;
      UTIL_OBJIDX  next    = UTIL_INVALID_OBJ_INDEX ;
      _objX * pObjX        = NULL ;

      //                 *       .
      // -       -       -       -      -
      //         ^       ^
      //         |       |   ==>
      //         begin   end     next
      //
      // when 'end' + 1 is equal to 'begin' means all objects are returned
      if ( _isInitialized && ( NULL != _list ) )
      {
         _latch.get() ;
         bLatched = TRUE ;

         if ( _acquiredCounter > 0 )
         {
            next = ( _end + 1 ) % _numOfObjs ;
            if ( next != _begin ) // _list is not full
            {
               // get the _objX address by the index
               pObjX = _getObjXByIndex( idx ) ;

               // sanity check
               if (    ( NULL != pObjX )
                    && ( _SEGMENT_OBJ_EYE_CATCHER   == pObjX->_eyeCatcher )
                    && ( _SEGMENT_OBJ_FLAG_ACQUIRED == pObjX->_flag ) )
               {
                  // mark the _objX flag
                  pObjX->_flag = _SEGMENT_OBJ_FLAG_RELEASED ;
               }
               else
               {
                  rc = SDB_SYS ;
                  goto error ;
               }

               // fill in current slot with the obj index( to the segment)
               _list[ _end ] = idx ;

               // advance to next position
               _end = next ;

               // decrease the acquired counter
               _acquiredCounter -- ;
            }
            else
            {
               // error, probably the caller or someone else,
               // returned/released a non-acquired object
               rc = SDB_SYS ;
               goto error ;
            }
         }
         else
         {
            rc = SDB_SYS ;
            goto error ;
         }
      }
      else
      {
         rc = SDB_SYS ;
         goto error ;
      }

    done:
       if ( bLatched )
       {
          _latch.release() ;
          bLatched = FALSE ;
       }
       return rc ;
    error:
      goto done ;
   }

} // namespace engine

#endif // UTIL_SEGMENT_HPP__

