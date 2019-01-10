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
          01/08/2019  CW  Optimize memory access

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
#pragma pack(4)
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
//               ^
//               |
//              _begin
//
// _begin: the position in _list where to acquire a free object 
// _begin-1 : the position in _list where to release/return an object

#ifdef _DEBUG
   #define _SEGMENT_OBJ_EYE_CATCHER    ( ( UINT32 ) 0xBEEFCAFE )
   #define _SEGMENT_OBJ_FLAG_ACQUIRED  ( ( UINT32 ) 0x0F0F     )
   #define _SEGMENT_OBJ_FLAG_RELEASED  ( ( UINT32 ) 0xF0F0     )
#endif

   #define IS_VALID_SEG_OBJ_INDEX( _objectIndex_ ) \
      ( UTIL_INVALID_OBJ_INDEX != _objectIndex_ )

   template < class T >
   class _utilSegmentManager : public SDBObject
   {
   private :
      struct _objX : public SDBObject
      {
#ifdef _DEBUG
         UINT32 _eyeCatcher ;
         UINT32 _flag ;

         _objX()
         {
            _eyeCatcher = _SEGMENT_OBJ_EYE_CATCHER ;
            _flag       = _SEGMENT_OBJ_FLAG_RELEASED ;
         }
#endif
         T      _obj ;
      } ;

      UTIL_OBJIDX *  _list  ;         // a list of obj indices to a segment
      UTIL_OBJIDX    _begin ;         // the position in list where acquire from
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
      _utilSegmentManager() : _list(NULL),
                              _begin(0),
                              _numOfObjs(0),
                              _delta(0),
                              _maxNumOfObjs(0),
                              _acquiredCounter(0),
                              _exponent(0),
                              _isInitialized( FALSE )
                              { }

      ~_utilSegmentManager() ;

   private :
      //
      // Description: add a new segment and expand the _list
      //              when no free objects
      // Input      : none
      // Return     : SDB_OK               -- normal
      //              SDB_OSS_UP_TO_LIMIT  -- exceed resource max threshold
      //              SDB_OOM              -- out of memory
      // Dependency : this function is called by acquire() only; and the caller
      //              shall make sure the operation is protected by latch
      INT32 _expandList() ;

      //
      // Description: check if need to expand before acquire an free object
      // Input      : none
      // Return     : TRUE   --  yes, need to expand
      //              FALSE  --  no, acquire operation can proceed
      // Dependency : this function is called by acquire() only; the caller
      //              shall acquire the latch first.
      //
      OSS_INLINE BOOLEAN _isFull() const
      {
         return _numOfObjs - 1 <= _acquiredCounter ? TRUE : FALSE ;
      }

      //
      // Description: check if current _numOfObjs is beyond the max threshold
      //              _maxNumOfObjs
      // Input      : none
      // Return     : TRUE
      //              FALSE
      // Dependency : this function is called by acquire() only, which shall
      //              acquire the latch first. 
      //
      OSS_INLINE BOOLEAN _isUpToLimit() const
      {
         return _numOfObjs >= _maxNumOfObjs ? TRUE : FALSE ;
      }

      //
      // Description: get the address of internal object _objX by its index
      // Input      : 
      //              idx     -- object index
      // Output     : 
      //              _objX * -- the address of the object
      //               
      // Dependency : this function is internal/private helper function
      //              called by getObjPtrByIndex(), acquire(), release() 
      //              
      OSS_INLINE _objX * _getObjXByIndex( const UTIL_OBJIDX idx )
      {
         _objX * pObj     = NULL ;
         _objX * pSegList = NULL ;
         UTIL_OBJIDX i    = 0 ;
         UTIL_OBJIDX j    = 0 ;
#ifdef _DEBUG
         SDB_ASSERT( ( idx < _numOfObjs ) && IS_VALID_SEG_OBJ_INDEX( idx ),
                     "Invalid object index." ) ;
#endif
         if ( IS_VALID_SEG_OBJ_INDEX( idx ) )
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
      //
      // Description: initialization
      //               . allocate the first array of objects, segment,
      //                 and save the address of this segment in _segList
      //               . fill the object index array, _list,
      //                 with index of the newly allocated object 
      // Input      :
      //              numberOfObjs    -- number of objects in a segment
      //              maxNumberOfObjs -- max number of objects, the max thresold
      //
      // Return     : SDB_OK          -- initialized successfully
      //              SDB_INVALIDARG  -- invalid arguments
      //              SDB_OOM         -- out of memory
      // Dependency : this function shall be called one time only, and before
      //              any other function of this class.
      //
      INT32 init( const UTIL_OBJIDX numberOfObjs,
                  const UTIL_OBJIDX maxNumberOfObjs ) ;

      //
      // Description: Free all objects allocated in segments and the object
      //              index array, _list
      // Return     : none
      // Dependency : this function shall be called just before destory,
      //              the caller shall guarantee no other threads are accessing
      //              the objects. 
      //
      void fini() ;

      //
      // Description: peek total number of objects without latch
      // Return     : total number of objects in segments
      //
      UTIL_OBJIDX getNumOfObjectsNoSync() ;

      //
      // Description: get total number of objects in segments
      // Return     : total number of objects in segments
      //
      UTIL_OBJIDX getNumOfObjects() ;

      //
      // Description: get an object's address by its index
      // Input      :
      //              idx -- the object index
      // Return     : address of the object ( specified by the index )
      // Dependency : this function shall be called after the class is
      //              initialized. It expects an correct index, i.e.,
      //              idx < _numOfObjs
      OSS_INLINE T * getObjPtrByIndex( const UTIL_OBJIDX idx ) ;

      //
      // Description: get an object's index by its address
      // Return     : the object's index 
      // Input      :
      //              idx -- the object index
      // Return     : index of the object ( specified by the address )
      // Dependency : this function shall be called after the class is
      //              initialized. It may return UTIL_INVALID_OBJ_INDEX,
      //              if invalid address is passed in.
      //
      UTIL_OBJIDX getIndexByAddr ( const T * pT ) ;

      //
      // Description: acquire/apply a free object from the segments
      // Input      : none
      // Output     : 
      //              idx -- the object index, each object has an unique index,
      //                     the index number is continuous, starting from 0
      //              pT  -- the pointer/address of the object
      // Return     : SDB_OK, 
      //              SDB_OSS_UP_TO_LIMIT,
      //              SDB_SYS,
      //              error rc returned from _expandList()
      // Dependency : this function shall be called after the class is
      //              initialized.
      //              when _acquiredCounter is equal to '_numOfObjs - 1',
      //              means lack of free objects and a new segment ( array of
      //              object T ) will be added and the _list will be expanded.
      //              acquire() protects all underneath operations by latch.
      //
      INT32 acquire( UTIL_OBJIDX & idx,  T * &pT ) ;

      //
      // Description: acquire/apply a free object from the segments
      // Input      : none
      // Output     :
      //              pT  -- the pointer/address of the object
      // Return     : SDB_OK, 
      //              SDB_OSS_UP_TO_LIMIT,
      //              SDB_SYS,
      //              error rc returned from _expandList()
      // Dependency : this function is thin wrapper of above acquire()
      //
      INT32 acquire( T * &pT ) ;

      //
      // Description: release/return an object to the segments by its index
      // Input      : idx -- the object index
      // Output     : none
      // Return     : SDB_OK  
      //              SDB_SYS -- when error occurs
      // Dependency : this function shall be called after the class is
      //              initialized.
      //              release() protects all underneath operations by latch.
      //
      INT32 release( const UTIL_OBJIDX idx ) ;

      //
      // Description: release/return an object to the segments by its address
      // Input      : pT -- address/pointer of the object
      // Output     : none
      // Return     : SDB_OK
      //              SDB_SYS        -- when error occurs
      //              SDB_INVALIDARG -- pT is invalid address
      // Dependency : this function is thin wrapper function of above release()
      //              this function will be slower than above release() as an
      //              extra operation, getIndexByAddr(), is performed ( convert
      //              address to index )
      //
      INT32 release( const T * pT ) ;

   #ifdef _DEBUG
      UTIL_OBJIDX * getListAddr() { return _list ; } 
   #endif
   } ;

   // get an object's index by its address
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

   // Free all objects allocated in segments and the object index array, _list
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

   // allocate a new segment of objects and expand the _list
   template < class T >
   INT32 _utilSegmentManager< T >::_expandList()
   {
      INT32         rc       = SDB_OK ;
      UTIL_OBJIDX * pListTmp = NULL ;
      _objX       * pSegTmp  = NULL ;
      UTIL_OBJIDX   newSize  = _numOfObjs + _delta ;

#ifdef _DEBUG
      SDB_ASSERT( _isInitialized,
                  "Expand can only be done when segment is initialized" ) ;
      SDB_ASSERT( _isFull(),
                  "Expand can only be done if all current objs are acquired" ) ;
      SDB_ASSERT( ( newSize <= _maxNumOfObjs ),
                  "Expand failed due to exceed object maximum threshold."  ) ;
#endif

      if (    ( UTIL_INVALID_OBJ_INDEX != newSize )
           && ( newSize <= _maxNumOfObjs )
           && ( 0 != _delta ) )
      {
         // allocate a new segment of object
         pSegTmp  = SDB_OSS_NEW _objX[ _delta ] ;
         // expand _list with new size
         pListTmp = ( UTIL_OBJIDX * )SDB_OSS_MALLOC( sizeof( UTIL_OBJIDX ) *
                                                     newSize ) ;
         if ( ( NULL != pListTmp ) && ( NULL != pSegTmp ) )
         {
            // copy old _list content
            ossMemcpy( pListTmp, _list, sizeof( UTIL_OBJIDX ) * _numOfObjs ) ;

            // initiate the newly allocated portion
            for ( UTIL_OBJIDX i = 0 ; i < _delta ; i++ )
            {
               // initialize the list
               pListTmp[ _numOfObjs + i ] = _numOfObjs + i;
            }

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

#ifdef _DEBUG
            SDB_ASSERT( ( SDB_OK == rc ), "Failed to expand, out of memory" ) ;
#endif
         }
      }
      else
      {
         // exceed the lock resorce limitation
         rc = SDB_OSS_UP_TO_LIMIT ;
      }
      
      return rc ;
   }

   // Initialization
   //   numberOfObjs     -- number of objects in a segment
   //   maxNumberOfObjs  -- max number of objects 
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

         SDB_ASSERT( ( _maxNumOfObjs >= _delta ),
                     "Invalid arguments, maxNumberOfObjs value is too small." );

         if ( _maxNumOfObjs < _delta )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      else
      {
         rc = SDB_INVALIDARG ;
         SDB_ASSERT( ( SDB_OK == rc ), "Invalid arguments" ) ;
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
         }
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
         SDB_ASSERT( ( SDB_OK == rc ), "Failed to initialize, out of memory" );
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   // peek _numOfObjs without latch
   template < class T >
   UTIL_OBJIDX _utilSegmentManager< T >::getNumOfObjectsNoSync()
   {  
      return _numOfObjs ;
   }

   // get _numOfObjs
   template < class T >
   UTIL_OBJIDX _utilSegmentManager< T >::getNumOfObjects()
   {  
      UTIL_OBJIDX numOfObjs = 0 ;

      _latch.get() ;
      numOfObjs = _numOfObjs ;
      _latch.release() ;

      return numOfObjs ;
   }

   // get an object's address by its index 
   template < class T >
   OSS_INLINE T * _utilSegmentManager< T >::getObjPtrByIndex
   (
      const UTIL_OBJIDX idx
   )
   {
      T * pObj     = NULL ;
      _objX *pObjX = _getObjXByIndex( idx ) ;
      if ( pObjX )
      {
         pObj = ( T * )&( pObjX->_obj ) ;
      }
      return pObj ;
   }

   // acquire a free object, upon successfully return,
   //   idx -- the index of the object
   //   pT  -- the address of the object
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
      // when _acquiredCounter is equal to '_numOfObjs - 1',
      // means lack of free objects and we will
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
            
            rc = _expandList() ;
            if ( rc )
            {
               goto error ;
            }
         }

         pObjX = _getObjXByIndex( _list[ _begin ] ) ;

#ifdef _DEBUG
         // sanity check
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
#endif
         // get the obj index( to the segment, the object array )
         idx = _list[ _begin ] ;

         // mark this slot is empty with invalid index number, -1
         _list[ _begin ] = UTIL_INVALID_OBJ_INDEX ;

         // advance to next position
         _begin ++; 

         // increase the acquired counter
         _acquiredCounter ++ ;

         // return the object address
         pT = ( T * )&( pObjX->_obj ) ;
      }
      else
      {
#ifdef _DEBUG
         SDB_ASSERT( ( _isInitialized && _list ),
                     "_utilSegmentManager has to be initialized." ) ;
#endif
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      if ( bLatched )
      {
         _latch.release() ;
      }
#ifdef _DEBUG
      SDB_ASSERT( ( SDB_OK == rc ), "Acquire failed" ) ;
#endif
      return rc ;
   error:
      goto done ;
   }

   // acquire a free object, upon successfully return,
   //   pT  -- the address of the object
   template < class T >
   INT32 _utilSegmentManager< T >::acquire( T * &pT )
   {
      UTIL_OBJIDX idx = UTIL_INVALID_OBJ_INDEX ;
      INT32  rc = acquire( idx, pT ) ;
      return rc ;
   }

   // release/return an object to the segments by its address
   //   pT -- address of the object
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
#ifdef _DEBUG
         SDB_ASSERT( ( SDB_OK == rc ), "Invalid object address" ) ;
#endif
      }
      return rc ;
   }

   // release/return an object to the segments by its index
   //   idx -- the object index
   template < class T >
   INT32 _utilSegmentManager< T >::release( const UTIL_OBJIDX idx )
   {
      INT32   rc           = SDB_OK ;
      BOOLEAN bLatched     = FALSE ;
      _objX * pObjX        = NULL ;

      if ( _isInitialized && ( NULL != _list ) )
      {
         _latch.get() ;
         bLatched = TRUE ;

         if ( _acquiredCounter > 0 )
         {
            if ( _numOfObjs != _begin ) // _list is not full
            {
               // get the _objX address by the index
               pObjX = _getObjXByIndex( idx ) ;

#ifdef _DEBUG
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

               SDB_ASSERT (_list[_begin - 1] == UTIL_INVALID_OBJ_INDEX,
                           "Corruption detected in list");
#endif

               // fill in current slot with the obj index( to the segment)
               _list[ --_begin ] = idx ;

               // decrease the acquired counter
               _acquiredCounter -- ;
            }
            else
            {
               // error, probably the caller or someone else,
               // returned/released a non-acquired object
               rc = SDB_SYS ;
#ifdef _DEBUG
               SDB_ASSERT( ( SDB_OK == rc ),
                           "Can't release object more than acquired" ) ;
#endif
               goto error ;
            }
         }
         else
         {
            rc = SDB_SYS ;
#ifdef _DEBUG
            SDB_ASSERT( ( SDB_OK == rc ),
                        "Can't release an object without acquiring" ) ;
#endif
            goto error ;
         }
      }
      else
      {
#ifdef _DEBUG
         SDB_ASSERT( ( _isInitialized && _list ),
                     "_utilSegmentManager has to be initialized." ) ;
#endif
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      if ( bLatched )
      {
         _latch.release() ;
         bLatched = FALSE ;
      }
#ifdef _DEBUG
      SDB_ASSERT( ( SDB_OK == rc ), "Release failed" ) ;
#endif
      return rc ;
   error:
      goto done ;
   }
#pragma pack()
} // namespace engine

#endif // UTIL_SEGMENT_HPP__

