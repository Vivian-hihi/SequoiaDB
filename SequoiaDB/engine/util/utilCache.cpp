/*******************************************************************************

   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = utilCache.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/04/2016  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "utilCache.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"

namespace engine
{

   #define UTIL_MAX_EXCEED_SLOT_SIZE            ( 3 )

   /*
      _utilCachePage implement
   */
   _utilCachePage::_utilCachePage()
   {
      _start = 0 ;
      _length = 0 ;
      _lastTime = 0 ;
      _lastWriteTime = 0 ;
      _readTimes = 0 ;
      _writeTimes = 0 ;
      _status = 0 ;
      _beginLSN = ~0 ;
      _endLSN = ~0 ;
      _lsnNum = 0 ;
   }

   _utilCachePage::_utilCachePage( const _utilCachePage& right )
   {
      _start = right._start ;
      _length = right._length ;
      _lastTime = right._lastTime ;
      _lastWriteTime = right._lastWriteTime ;
      _readTimes = right._readTimes ;
      _writeTimes = right._writeTimes ;
      _status = right._status ;
      _beginLSN = right._beginLSN ;
      _endLSN = right._endLSN ;
      _lsnNum = right._lsnNum ;

      _first.size = 0 ;
      _first._pBuff = NULL ;

      UINT32 pos = right.beginBlock() ;
      CHAR *pPage = NULL ;
      UINT32 pageSize = 0 ;

      while( NULL != ( pPage = right.nextBlock( pageSize, pos ) ) )
      {
         addPage( pPage, pageSize ) ;
      }
   }

   _utilCachePage::~_utilCachePage()
   {
      clear() ;
   }

   void _utilCachePage::clearDataInfo()
   {
      _start = 0 ;
      _length = 0 ;

      _lastTime = 0 ;
      _lastWriteTime = 0 ;
      _readTimes = 0 ;
      _writeTimes = 0 ;

      clearLSNInfo() ;
   }

   void _utilCachePage::clearLSNInfo()
   {
      _beginLSN = ~0 ;
      _endLSN = ~0 ;
      _lsnNum = 0 ;
   }

   BOOLEAN _utilCachePage::isDataEmpty() const
   {
      if ( _start == _length )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   void _utilCachePage::clear()
   {
      _next.clear() ;
      _start = 0 ;
      _length = 0 ;
      _first._size = 0 ;
      _first._pBuff = NULL ;
      _lastTime = 0 ;
      _lastWriteTime = 0 ;
      _readTimes = 0 ;
      _writeTimes = 0 ;
      _status = 0 ;
      _beginLSN = ~0 ;
      _endLSN = ~0 ;
      _lsnNum = 0 ;
   }

   UINT32 _utilCachePage::size() const
   {
      UINT32 totalSize = 0 ;
      UINT32 blockSize = 0 ;
      UINT32 pos = beginBlock() ;
      while( NULL != nextBlock( blockSize, pos ) )
      {
         totalSize += blockSize ;
      }
      return totalSize ;
   }

   _utilCachePage& _utilCachePage::operator= ( const _utilCachePage& rhs )
   {
      clear() ;

      _start = rhs._start ;
      _length = rhs._length ;
      _lastTime = rhs._lastTime ;
      _lastWriteTime = rhs._lastWriteTime ;
      _readTimes = rhs._readTimes ;
      _writeTimes = rhs._writeTimes ;
      _status = rhs._status ;
      _beginLSN = rhs._beginLSN ;
      _endLSN = rhs._endLSN ;
      _lsnNum = rhs._lsnNum ;

      UINT32 pos = rhs.beginBlock() ;
      CHAR *pPage = NULL ;
      UINT32 pageSize = 0 ;

      while( NULL != ( pPage = rhs.nextBlock( pageSize, pos ) ) )
      {
         addPage( pPage, pageSize ) ;
      }
      return *this ;
   }

   UINT32 _utilCachePage::blockNum() const
   {
      if ( _first.empty() )
      {
         return 0 ;
      }
      return 1 + _next.size() ;
   }

   INT32 _utilCachePage::write( const CHAR *pBuf, UINT32 offset, UINT32 len )
   {
      INT32 rc = SDB_OK ;
      ossTimestamp t ;
      UINT32 pos = 0 ;
      CHAR *pPage = NULL ;
      UINT32 blockSize = 0 ;
      UINT32 lastOffset = offset ;
      UINT32 lastLen = len ;
      UINT32 onceWrite = 0 ;

      if ( size() < offset + len )
      {
         /// no space for write data
         rc = SDB_NOSPC ;
         goto error ;
      }

      pos = beginBlock() ;
      while ( lastLen > 0 && NULL != ( pPage = nextBlock( blockSize, pos ) ) )
      {
         if ( blockSize <= lastOffset )
         {
            lastOffset -= blockSize ;
            continue ;
         }
         else if ( lastOffset > 0 )
         {
            pPage += lastOffset ;
            blockSize -= lastOffset ;
            lastOffset = 0 ;
         }
         onceWrite = lastLen < blockSize ? lastLen : blockSize ;
         ossMemcpy( pPage, pBuf, onceWrite ) ;
         lastLen -= onceWrite ;
      }

      SDB_ASSERT( lastLen == 0, "Last len must be 0" ) ;

      /// update meta
      ++_writeTimes ;
      ossGetCurrentTime( t ) ;
      _lastTime = t.time * 1000 + t.microtm / 1000 ;
      _lastWriteTime = _lastTime ;

      if ( 0 == _start || offset < _start )
      {
         _start = offset ;
      }
      if ( offset + len > _length )
      {
         _length = offset + len ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   UINT32 _utilCachePage::read( CHAR *pBuf, UINT32 offset, UINT32 len )
   {
      ossTimestamp t ;
      UINT32 hasRead = 0 ;
      CHAR *pPage = NULL ;
      UINT32 blockSize = 0 ;
      UINT32 onceRead = 0 ;
      UINT32 lastOffset = offset ;
      UINT32 lastRead = len ;
      UINT32 pos = 0 ;

      pos = beginBlock() ;
      while ( lastRead > 0 && NULL != ( pPage = nextBlock( blockSize, pos ) ) )
      {
         if ( blockSize <= lastOffset )
         {
            lastOffset -= blockSize ;
            continue ;
         }
         else if ( lastOffset > 0 )
         {
            pPage += lastOffset ;
            blockSize -= lastOffset ;
            lastOffset = 0 ;
         }
         onceRead = lastRead < blockSize ? lastRead : blockSize ;
         ossMemcpy( pBuf + hasRead, pPage, onceRead ) ;
         lastRead -= onceRead ;
         hasRead += onceRead ;
      }

      /// update meta
      ++_readTimes ;
      ossGetCurrentTime( t ) ;
      _lastTime = t.time * 1000 + t.microtm / 1000 ;

      return hasRead ;
   }

   INT32 _utilCachePage::copy( const _utilCachePage &right )
   {
      INT32 rc = SDB_OK ;
      CHAR *pData = NULL ;
      UINT32 blockSz = 0 ;
      UINT32 offset = 0 ;

      UINT32 rightPos = right.beginBlock() ;
      while( NULL != ( pData = right.nextBlock( blockSz, rightPos ) ) )
      {
         rc = write( pData, offset, blockSz ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Copy data failed, rc: %d", rc ) ;
            goto error ;
         }
         offset += blockSz ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCachePage::addPage( CHAR *pPage, UINT32 size )
   {
      INT32 rc = SDB_OK ;

      if ( !pPage || 0 == size )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( _first.empty() )
      {
         _first._pBuff = pPage ;
         _first._size = size ;
      }
      else
      {
         _next.push_back( utilCacheBlock( pPage, size ) ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   UINT32 _utilCachePage::beginBlock() const
   {
      return 0 ;
   }

   CHAR* _utilCachePage::nextBlock( UINT32 &size, UINT32 &pos ) const
   {
      UINT32 tmpPos = pos ;
      ++pos ;

      if ( 0 == tmpPos )
      {
         if ( _first.empty() )
         {
            return NULL ;
         }
         size = _first._size ;
         return _first._pBuff ;
      }
      else if ( _next.size() >= tmpPos )
      {
         const utilCacheBlock& item = _next[ tmpPos - 1 ] ;
         size = item._size ;
         return item._pBuff ;
      }

      return NULL ;
   }

   void _utilCachePage::addLSN( UINT64 lsn )
   {
      if ( ~0 == _beginLSN )
      {
         _beginLSN = lsn ;
         _endLSN = lsn ;
      }
      else
      {
         _endLSN = lsn ;
      }
      ++_lsnNum ;
   }

   /*
      _utilCacheMgr implement
   */
   _utilCacheMgr::_utilCacheMgr()
   :_freeSize( 0 ), _totalSize( 0 )
   {
      _beginPageSizeSqrt = 0 ;
      _maxCacheSize = 0 ;
   }

   _utilCacheMgr::_utilCacheMgr()
   {
      fini() ;
   }

   INT32 _utilCacheMgr::init( UINT64 cacheSize )
   {
      INT32 rc = SDB_OK ;
      blkLatch* pLatch = NULL ;
      _maxCacheSize = cacheSize * 1024 * 1024 ;

      if ( !ossIsPowerOf2( UTIL_PAGE_SLOT_BEGIN_SIZE, &_beginPageSizeSqrt ) )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      for ( UINT32 i = 0 ; i < UTIL_PAGE_SLOT_SIZE ; ++i )
      {
         pLatch = SDB_OSS_NEW blkLatch() ;
         if ( pLatch )
         {
            _latch.push_back( pLatch ) ;
         }
         else
         {
            PD_LOG( PDERROR, "Failed to alloc bucket latch" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _utilCacheMgr::fini()
   {
      SDB_ASSERT( _totalSize.peek() == _freeSize.peek(),
                  "Total size must be equal with the free size" ) ;

      UINT32 size = 0 ;
      for ( UINT32 i = 0 ; i < UTIL_PAGE_SLOT_SIZE ; ++i )
      {
         size = _slot[ i ].size() ;
         for ( UINT32 j = 0 ; j < size ; ++j )
         {
            SDB_OSS_FREE( _slot[ i ][ j ] ) ;
         }
         _slot[ i ].clear() ;
      }

      for ( UINT32 i = 0 ; i < _latch.size() ; ++i )
      {
         SDB_OSS_DEL _latch[ i ] ;
      }
      _latch.clear() ;
   }

   void _utilCacheMgr::resetEvent()
   {
      _releaseEvent.reset() ;
   }

   INT32 _utilCacheMgr::waitEvent( INT64 millisec )
   {
      return _releaseEvent.wait( millisec ) ;
   }

   INT32 _utilCacheMgr::alloc( UINT32 size, utilCachePage &item )
   {
      INT32 rc = SDB_OK ;
      UINT32 beginSlot = 0 ;
      UINT32 pageSize = 0 ;
      UINT32 extendNum = 0 ;
      UINT32 lastSize = 0 ;
      UINT32 exceedSlot = 0 ;
      blkLatch *pLatch = NULL ;

      if ( item.size() >= size )
      {
         return rc ;
      }
      lastSize = size - item.size() ;

   retry:
      beginSlot = _size2Slot( lastSize ) ;
      exceedSlot = 0 ;

      /// first to alloc a whole page
      while( beginSlot < UTIL_PAGE_SLOT_SIZE &&
             exceedSlot < UTIL_MAX_EXCEED_SLOT_SIZE )
      {
         pLatch = _latch[ beginSlot ] ;
         pLatch->get() ;
         vector< CHAR* > &slotItem = _slot[ beginSlot ] ;
         if ( !slotItem.empty() )
         {
            pageSize = _slot2PageSize( beginSlot ) ;
            item.addPage( slotItem.back(), pageSize ) ;
            slotItem.pop_back() ;
            _freeSize.sub( pageSize ) ;
            pLatch->release() ;
            goto done ;
         }
         pLatch->release() ;
         ++beginSlot ;
         ++exceedSlot ;
      }

      /// then not alloc, but freeSize is more than lastSize, merge the pages
      if ( beginSlot >= UTIL_PAGE_SLOT_SIZE )
      {
         beginSlot = UTIL_PAGE_SLOT_SIZE - 1 ;
      }
      while ( beginSlot > 0 && _freeSize.peek() >= lastSize )
      {
         --beginSlot ;
         pageSize = _slot2PageSize( beginSlot ) ;
         pLatch = _latch[ beginSlot ] ;
         pLatch->get() ;
         vector< CHAR* > &slotItem = _slot[ beginSlot ] ;
         while( slotItem.empty() )
         {
            item.addPage( slotItem.back(), pageSize ) ;
            slotItem.pop_back() ;
            _freeSize.sub( pageSize ) ;

            if ( lastSize > pageSize )
            {
               lastSize -= pageSize ;
            }
            else
            {
               pLatch->release() ;
               goto done ;
            }
         }
         pLatch->release() ;
      }

      /// no space for the lastSize, alloc and retry
      if ( _size2Slot( lastSize ) >= UTIL_PAGE_SLOT_SIZE )
      {
         pageSize = _slot2PageSize( UTIL_PAGE_SLOT_SIZE - 1 ) ;
         extendNum = ( lastSize + pageSize - 1 ) / pageSize ;
      }
      else
      {
         pageSize = _size2PageSize( lastSize ) ;
         extendNum = 1 ;
      }
      rc = _allocMem( pageSize, extendNum ) ;
      if ( SDB_OK == rc )
      {
         goto retry ;
      }
      else
      {
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCacheMgr::allocWholePage( UINT32 size, utilCachePage &item,
                                        BOOLEAN keepData )
   {
      INT32 rc = SDB_OK ;
      UINT32 beginSlot = 0 ;
      UINT32 pageSize = 0 ;
      UINT32 exceedSlot = 0 ;
      utilCachePage tmpPage ;

      if ( item.size() >= size )
      {
         return rc ;
      }

   retry:
      beginSlot = _size2Slot( size ) ;
      exceedSlot = 0 ;

      /// first to alloc a whole page
      while( beginSlot < UTIL_PAGE_SLOT_SIZE &&
             exceedSlot < UTIL_MAX_EXCEED_SLOT_SIZE )
      {
         _latch[ beginSlot ]->get() ;
         vector< CHAR* > &slotItem = _slot[ beginSlot ] ;
         if ( !slotItem.empty() )
         {
            pageSize = _slot2PageSize( beginSlot ) ;
            tmpPage.addPage( slotItem.back(), pageSize ) ;
            slotItem.pop_back() ;
            _freeSize.sub( pageSize ) ;
            _latch[ beginSlot ]->release() ;
            goto done ;
         }
         _latch[ beginSlot ]->release() ;
         ++beginSlot ;
         ++exceedSlot ;
      }

      /// no space for the size, alloc and retry
      rc = _allocMem( size ) ;
      if ( SDB_OK == rc )
      {
         goto retry ;
      }
      else
      {
         goto error ;
      }

      /// copy data from source page
      if ( keepData && !item.isEmpty() )
      {
         rc = tmpPage.copy( item ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Copy data from source page failed, rc: %d",
                    rc ) ;
            goto error ;
         }
      }
      /// release the source page
      release( item ) ;
      item = tmpPage ;

   done:
      return rc ;
   error:
      release( tmpPage ) ;
      goto done ;
   }

   void _utilCacheMgr::release( utilCachePage &item )
   {
      UINT32 pos = 0 ;
      UINT32 slot = 0 ;
      CHAR *pPage = NULL ;
      UINT32 pageSize = 0 ;

      SDB_ASSERT( !item.isDirty(), "Page can't be dirty" ) ;
      SDB_ASSERT( !item.isLocked(), "Page can't be locked" ) ;

      pos = item.beginBlock() ;

      while( NULL != ( pPage = item.nextBlock( pageSize, pos ) ) )
      {
         slot = _size2Slot( pageSize ) ;
         _latch[ slot ]->get() ;
         _slot[ slot ].push_back( pPage ) ;
         _latch[ slot ]->release() ;
         _freeSize.add( pageSize ) ;
      }
      _releaseEvent.signalAll() ;

      item.clear() ;
   }

   INT32 _utilCacheMgr::alloc( UINT32 size, utilCachePage &item,
                               BOOLEAN wholePage, BOOLEAN keepData )
   {
      if ( !wholePage )
      {
         return alloc( size, item ) ;
      }
      return allocWholePage( size, item, keepData ) ;
   }

   INT32 _utilCacheMgr::_allocMem( UINT32 size, UINT32 pageNum )
   {
      INT32 rc = SDB_OK ;
      UINT32 count = 0 ;
      CHAR *pPage = NULL ;
      UINT32 pageSize = 0 ;
      UINT32 slot = 0 ;

      if ( 0 == size )
      {
         goto done ;
      }

      pageSize = _size2PageSize( size ) ;
      slot = _size2Slot( size ) ;

      if ( slot >= UTIL_PAGE_SLOT_SIZE )
      {
         PD_LOG( PDERROR, "Size[%u] is more than the max page size[2^%u]",
                 size, _beginPageSizeSqrt + UTIL_PAGE_SLOT_SIZE - 1 ) ;
         SDB_ASSERT( slot < UTIL_PAGE_SLOT_SIZE, "Size is invalid" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      while( count < pageNum )
      {
         if ( _totalSize.peek() + pageSize > _maxCacheSize )
         {
            /// up to the limit
            rc = SDB_OSS_UP_TO_LIMIT ;
            goto error ;
         }

         pPage = (CHAR*)SDB_OSS_MALLOC( pageSize ) ;
         if ( !pPage )
         {
            rc = SDB_OOM ;
            goto error ;
         }

         _latch[ slot ]->get() ;
         /// push to vector
         _slot[ slot ].push_back( pPage ) ;
         _latch[ slot ]->release() ;
         /// update the meta
         _totalSize.add( pageSize ) ;
         _freeSize.add( pageSize ) ;
         ++count ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _utilCacheBucket implement
   */
   _utilCacheBucket::_utilCacheBucket( UINT32 blkID )
   {
      _blkID = blkID ;
      _dirtyPages = 0 ;
   }

   _utilCacheBucket::~_utilCacheBucket()
   {
      SDB_ASSERT( _pages.size() == 0, "Pages must be released" ) ;
   }

   utilCachePage* _utilCacheBucket::getPage( INT32 pageID )
   {
      MAP_BLK_PAGE::iterator it = _pages.find( pageID ) ;
      if ( it != _pages.end() )
      {
         return &(it->second) ;
      }
      return NULL ;
   }

   utilCachePage* _utilCacheBucket::addPage( INT32 pageID,
                                             const utilCachePage &page )
   {
      SDB_ASSERT( !page.isInvalid(), "Page cant' be invalid" ) ;
      utilCachePage& tmpPage = _pages[ pageID ] ;
      tmpPage = page ;
      return &tmpPage ;
   }

   utilCachePage* _utilCacheBucket::delPage( INT32 pageID )
   {
      utilCachePage* pPage = NULL ;
      MAP_BLK_PAGE::iterator it = _pages.find( pageID ) ;
      if ( it != _pages.end() )
      {
         pPage = &(it->second) ;
         SDB_ASSERT( !pPage->isDirty(), "Page can't be dirty" ) ;
         _pages.erase( it ) ;
      }
      return pPage ;
   }

   INT32 _utilCacheBucket::lock( OSS_LATCH_MODE mode, INT32 millisec )
   {
      INT32 rc = SDB_OK ;

      if ( EXCLUSIVE == mode )
      {
         rc = _rwMutex.lock_w( millisec ) ;
      }
      else if ( SHARED == mode )
      {
         rc = _rwMutex.lock_r( millisec ) ;
      }
      else
      {
         goto done ;
      }

   done:
      return rc ;
   }

   void _utilCacheBucket::unlock( OSS_LATCH_MODE mode )
   {
      if ( SHARED == mode )
      {
         _rwMutex.release_r() ;
      }
      else if ( EXCLUSIVE == mode )
      {
         _rwMutex.release_w() ;
      }
   }

   /*
      _utilCacheContext implement
   */
   _utilCacheContext::_utilCacheContext()
   {
      _pData = NULL ;
      _offset = 0 ;
      _len = 0 ;
      _pageID = 0 ;
      _makeDirty = FALSE ;
      _pPage = NULL ;
      _pBucket = NULL ;
      _pUnit = NULL ;
      _mode = -1 ;
      _isWrite = FALSE ;
      _size = 0 ;
      _writeBack = FALSE ;
      _usePage = FALSE ;
   }

   _utilCacheContext::~_utilCacheContext()
   {
      release() ;
   }

   BOOLEAN _utilCacheContext::isValid() const
   {
      return ( _pBucket && _pUnit && _pageID > 0 ) ? TRUE : FALSE ;
   }

   BOOLEAN _utilCacheContext::isPageValid() const
   {
      return ( _pPage && isValid() ) ? TRUE : FALSE ;
   }

   BOOLEAN _utilCacheContext::isLocked() const
   {
      return ( EXCLUSIVE == _mode || SHARED == _mode ) ? TRUE : FALSE ;
   }

   BOOLEAN _utilCacheContext::isLockRead() const
   {
      return SHARED == _mode ? TRUE : FALSE ;
   }

   BOOLEAN _utilCacheContext::isLockWrite() const
   {
      return EXCLUSIVE == _mode ? TRUE : FALSE ;
   }

   void _utilCacheContext::unLock()
   {
      if ( _pBucket && isLocked() )
      {
         _pBucket->unlock( (OSS_LATCH_MODE)_mode ) ;
      }
      _mode = -1 ;
   }

   BOOLEAN _utilCacheContext::isDone() const
   {
      return _pData ? FALSE : TRUE ;
   }

   void _utilCacheContext::discardPage()
   {
      if ( isPageValid() && _pPage->isDirty() )
      {
         _pPage->clearDirty() ;
         _pPage->clearDataInfo() ;
      }
   }

   BOOLEAN _utilCacheContext::isInCache( UINT32 offset, UINT32 len ) const
   {
      if ( _pPage && offset >= _pPage->start() &&
           offset + len <= _pPage->length() )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   INT32 _utilCacheContext::write( const CHAR *pData,
                                   UINT32 offset,
                                   UINT32 len,
                                   IExecutor *cb )
   {
      INT32 rc = SDB_OK ;

      if ( !isValid() )
      {
         SDB_ASSERT( FALSE, "Context is invalid" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      else if ( offset + len > _size )
      {
         SDB_ASSERT( FALSE, "offset + len must <= _size" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !isDone() )
      {
         SDB_ASSERT( FALSE, "Must done before next write" ) ;
         submit( cb ) ;
      }

      if ( isPageValid() )
      {
         /// the page
         /// -----------|<start>-------|<end>-----------------
         ///  |--A--|-C-|              |---D--|--B--|
         /// when write in A, need to load the data C
         /// when write in B, need to load the data D
         if ( !_pPage->isDataEmpty() &&
              ( offset + len < _pPage->start() ||
                _pPage->length() < offset ) )
         {
            if ( !_pPage->isDirty() )
            {
               _pPage->clearDataInfo() ;
            }
            else
            {
               rc = _loadPage( offset, len, cb ) ;
               if( rc )
               {
                  PD_LOG( PDERROR, "Load page[ID:%d,Off:%u,Len:%u] failed, "
                          "rc: %d", _pageID, offset, len, rc ) ;
                  goto error ;
               }
            }
         }

         _pData = (CHAR*)pData ;
         _offset = offset ;
         _len = len ;
         _isWrite = TRUE ;
         _makeDirty = TRUE ;
         _usePage = TRUE ;
         _writeBack = FALSE ;
      }
      else
      {
         _pData = (CHAR*)pData ;
         _offset = offset ;
         _len = len ;
         _isWrite = TRUE ;
         _makeDirty = TRUE ;
         _usePage = FALSE ;
         _writeBack = FALSE ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCacheContext::read( CHAR *pBuff,
                                  UINT32 offset,
                                  UINT32 len,
                                  IExecutor *cb )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN readPage = FALSE ;

      if ( !isValid() )
      {
         SDB_ASSERT( FALSE, "Context is invalid" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      else if ( offset + len > _size )
      {
         SDB_ASSERT( FALSE, "offset + len must <= _size" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !isDone() )
      {
         SDB_ASSERT( FALSE, "Must done before next read" ) ;
         submit( cb ) ;
      }

      if ( isPageValid() )
      {
         /// the page
         /// -----------|<start>-------|<end>-----------------
         ///  |--A----------|     |------B--|
         /// when read in A, need to load the data
         /// when read in B, need to load the data
         if ( offset >= _pPage->start() &&
              offset + len <= _pPage->length() )
         {
            readPage = TRUE ;
         }
         else if ( offset + len <= _pPage->start() ||
                   _pPage->length() <= offset )
         {
            /// read from file
            readPage = FALSE ;
         }
         else if ( _pPage->isDirty() )
         {
            /// load the data
            if ( offset < _pPage->start() )
            {
               rc = _loadPage( offset, _pPage->start() - offset, cb ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Load page[ID:%d,Off:%u,Len:%u] data "
                          "failed, rc: %d", _pageID, offset,
                          _pPage->start() - offset, rc ) ;
                  goto error ;
               }
            }
            if ( offset + len > _pPage->length() )
            {
               rc = _loadPage( _pPage->length(),
                               offset + len - _pPage->length(), cb ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Load page[ID:%d,Off:%u,Len:%u] data "
                          "failed, rc: %d", _pageID, _pPage->length(),
                          offset + len - _pPage->length(), rc ) ;
                  goto error ;
               }
            }
            readPage = TRUE ;
         }
      }

      if ( readPage )
      {
         _pData = (CHAR*)pBuff ;
         _offset = offset ;
         _len = len ;
         _isWrite = FALSE ;
         _makeDirty = FALSE ;
         _usePage = TRUE ;
         _writeBack = FALSE ;
      }
      else
      {
         _pData = (CHAR*)pBuff ;
         _offset = offset ;
         _len = len ;
         _isWrite = FALSE ;
         _makeDirty = FALSE ;
         _usePage = FALSE ;
         _writeBack = FALSE ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCacheContext::readAndCache( CHAR *pBuff,
                                          UINT32 offset,
                                          UINT32 len,
                                          IExecutor *cb )
   {
      INT32 rc = SDB_OK ;

      rc = read( pBuff, offset, len, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      _writeBack = TRUE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   UINT32 _utilCacheContext::submit( IExecutor *cb )
   {
      UINT32 len = 0 ;

      if ( _pData )
      {
         INT32 rc = SDB_OK ;

         if ( _isWrite )
         {
            if ( _usePage )
            {
               /// write to page
               rc = _pPage->write( _pData, _offset, _len ) ;
               if ( _makeDirty && !_pPage->isDirty() )
               {
                  _pPage->makeDirty() ;
                  _pUnit->incDirtyPages( _pBucket ) ;
               }
               if( cb && _makeDirty )
               {
                  _pPage->addLSN( cb->getEndLSN() ) ;
               }
            }
            else
            {
               /// write to file
               utilCachFileBase* pFile = _pUnit->getCacheFile() ;
               rc = pFile->write( _pageID, _pData, _len, _offset, cb ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Write page[ID:%d,Off:%u,Len:%u] to "
                          "file[%s] failed, rc: %d", _pageID, _offset, _len,
                          pFile->getFileName(), rc ) ;
               }
            }
         }
         else
         {
            if ( _usePage )
            {
               /// read from page
               len = _pPage->read( _pData, _offset, _len ) ;
            }
            else
            {
               /// read from file
               utilCachFileBase* pFile = _pUnit->getCacheFile() ;
               rc = pFile->read( _pageID, _pData, _len, _offset, len, cb ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Read page[ID:%d,Off:%u,Len:%u] from "
                          "file[%s] failed, rc: %d", _pageID, _offset,
                          _len, pFile->getFileName(), rc ) ;
               }
               /// write to cache page
               if ( _writeBack && _pPage )
               {
                  _pPage->write( _pData, _offset, len ) ;
               }
            }
         }

         if ( rc )
         {
            ossPanic() ;
         }

         /// clear data info
         _pData = NULL ;
         _len = 0 ;
         _offset = 0 ;
         _isWrite = FALSE ;
         _makeDirty = FALSE ;
         _writeBack = FALSE ;
      }

      return len ;
   }

   void _utilCacheContext::rollback()
   {
      if ( _pData )
      {
         _pData = NULL ;
         _offset = 0 ;
         _len = 0 ;
         _isWrite = FALSE ;
         _makeDirty = FALSE ;
         _writeBack = FALSE ;
      }
   }

   void _utilCacheContext::release()
   {
      rollback() ;
      unLock() ;
      _pPage = NULL ;
      _pBucket = NULL ;
      _pUnit = NULL ;
   }

   INT32 _utilCacheContext::_loadPage( UINT32 offset,
                                       UINT32 len,
                                       IExecutor *cb )
   {
      INT32 rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _utilCacheUnit implement
   */
   _utilCacheUnit::_utilCacheUnit()
   :_dirtySize( 0 ), _totalPage( 0 )
   {
      _pMgr = NULL ;
      _pCacheFile = NULL ;
      _bucketSize = 0 ;
      _pageSize = 0 ;
      _closed = FALSE ;
      _lastRecycle = 0 ;
      _wholePage = FALSE ;
      _pageTimeout = 0 ;
   }

   _utilCacheUnit::~_utilCacheUnit()
   {
      fini() ;
   }

   INT32 _utilCacheUnit::init ( utilCacheMgr *pMgr,
                                utilCachFileBase *pFile,
                                UINT32 pageSize,
                                UINT32 bucketSize,
                                BOOLEAN wholePage,
                                UINT32 pageTimeout )
   {
      INT32 rc = SDB_OK ;
      utilCacheBucket* pBucket = NULL ;

      if ( !pMgr || !pFile || 0 == pageSize || 0 == bucketSize )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      _pMgr = pMgr ;
      _pCacheFile = pFile ;
      _pageSize = pageSize ;
      _wholePage = wholePage ;
      _closed = FALSE ;
      _pageTimeout = pageTimeout ;

      for ( UINT32 i = 0 ; i < bucketSize ; ++i )
      {
         pBucket = SDB_OSS_NEW utilCacheBucket( i ) ;
         if ( !pBucket )
         {
            PD_LOG( PDERROR, "Alloc bucket[%u] failed", i ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         _vecBucket.push_back( pBucket ) ;
         ++_bucketSize ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _utilCacheUnit::fini()
   {
      utilCacheBucket* pBucket = NULL ;
      utilCacheBucket::MAP_BLK_PAGE* pPages = NULL ;

      _closed = TRUE ;
      /// wait all dirty page flushed to file
      while( dirtyPages() > 0 )
      {
         syncPages() ;
      }

      for ( UINT32 i = 0 ; i < _bucketSize ; ++i )
      {
         pBucket = _vecBucket[ i ] ;

         pPages = pBucket->getPages() ;

         utilCacheBucket::MAP_BLK_PAGE::iterator it = pPages->begin() ;
         while ( it != pPages->end() )
         {
            _pMgr->release( it->second ) ;
            ++it ;
         }
         pPages->clear() ;

         SDB_OSS_DEL pBucket ;
      }
      _vecBucket.clear() ;
      _bucketSize = 0 ;
   }

   UINT32 _utilCacheUnit::calcBucketID( INT32 pageID ) const
   {
      if ( 0 == _bucketSize )
      {
         return 0 ;
      }
      return (UINT32)pageID % _bucketSize ;
   }

   UINT64 _utilCacheUnit::totalPages()
   {
      return _totalPage.peek() ;
   }

   UINT64 _utilCacheUnit::dirtyPages()
   {
      return _dirtySize.peek() ;
   }

   void _utilCacheUnit::decDirtyPages( utilCacheBucket *pBucket )
   {
      _dirtySize.dec() ;
      pBucket->decDirty() ;
   }

   void _utilCacheUnit::incDirtyPages( utilCacheBucket *pBucket )
   {
      _dirtySize.inc() ;
      pBucket->incDirty() ;
   }

   utilCachePage* _utilCacheUnit::getAndLock( INT32 pageID, UINT32 size,
                                              utilCacheBucket **ppBucket,
                                              OSS_LATCH_MODE mode,
                                              BOOLEAN alloc,
                                              IExecutor *cb )
   {
      utilCachePage* pPage = NULL ;
      UINT32 bucketID = calcBucketID( pageID ) ;
      utilCacheBucket* pBucket = NULL ;
      BOOLEAN locked = FALSE ;

      if ( _pMgr->maxCacheSize() < size || _closed )
      {
         goto done ;
      }
      else if ( pageID < 0 )
      {
         PD_LOG( PDERROR, "Page id[%d] is invalid", pageID ) ;
         SDB_ASSERT( pageID >= 0, "Page must >= 0" ) ;
         goto done ;
      }

      pBucket = _vecBucket[ bucketID ] ;
      pBucket->lock( mode ) ;
      locked = TRUE ;

      if ( ppBucket )
      {
         *ppBucket = pBucket ;
      }

      pPage = pBucket->getPage( pageID ) ;
      if ( !pPage && alloc )
      {
         utilCachePage tmpPage ;
         INT32 rc = _pMgr->alloc( size, tmpPage, _wholePage ) ;
         if ( SDB_OK == rc )
         {
            /// add to bucket
            pPage = pBucket->addPage( pageID, tmpPage ) ;
            _totalPage.inc() ;
         }
         else
         {
            if ( SDB_OSS_UP_TO_LIMIT != rc )
            {
               PD_LOG( PDERROR, "Alloc page[%u] failed, rc: %d",
                       size, rc ) ;
            }
            goto done ;
         }
      }
      else if ( pPage && pPage->size() < size )
      {
         BOOLEAN lockPage = FALSE ;
         INT32 rc = _pMgr->alloc( size, *pPage, _wholePage,
                                  pPage->isInvalid() ? FALSE : TRUE ) ;
         if ( rc )
         {
            if( !pPage->isLocked() )
            {
               pPage->lock() ;
               lockPage = TRUE ;
            }
            pBucket->unlock( mode ) ;
            locked = FALSE ;

            /// recycle and try again
            recyclePages( 0, size ) ;

            pBucket->lock( mode ) ;
            locked = TRUE ;

            if( lockPage )
            {
               pPage->unlock() ;
            }

            rc = _pMgr->alloc( size, *pPage, _wholePage,
                               pPage->isInvalid() ? FALSE : TRUE ) ;
            if ( rc )
            {
               if ( SDB_OSS_UP_TO_LIMIT != rc )
               {
                  PD_LOG( PDERROR, "Alloc page[%u] failed, rc: %d",
                          size, rc ) ;
               }
               /// if the page is dirty, need to sync first
               if ( pPage->isDirty() )
               {
                  rc = _syncPage( pBucket, pPage, pageID, cb ) ;
                  if ( rc )
                  {
                     PD_LOG( PDERROR, "Sync page[%d] failed, rc: %d",
                             pageID, rc ) ;
                     ossPanic() ;
                  }
               }
               pPage->clearDataInfo() ;
               pPage = NULL ;
               goto done ;
            }
         }
      }

   done:
      if ( !pPage && pBucket && locked )
      {
         pBucket->unlock( mode ) ;
      }
      return pPage ;
   }

   utilCacheBucket* _utilCacheUnit::getBucket( UINT32 index )
   {
      if ( index >= _bucketSize )
      {
         return NULL ;
      }
      return _vecBucket[ index ] ;
   }

   INT32 _utilCacheUnit::write( INT32 pageID, const CHAR *pData,
                                UINT32 offset, UINT32 len,
                                IExecutor *cb,
                                utilCacheContext &context )
   {
      INT32 rc = SDB_OK ;
      utilCachePage* pPage = NULL ;
      utilCacheBucket* pBucket = NULL ;

      pPage = getAndLock( pageID, offset + len, &pBucket,
                          EXCLUSIVE, TRUE, cb ) ;
      if ( pPage )
      {
         /// the page
         /// -----------|<start>-------|<end>-----------------
         ///  |--A--|-C-|              |---D--|--B--|
         /// when write in A, need to load the data C
         /// when write in B, need to load the data D
         if ( !pPage->isDataEmpty() &&
              ( offset + len < pPage->start() ||
                pPage->length() < offset ) )
         {
            if ( !pPage->isDirty() )
            {
               pPage->clearDataInfo() ;
            }
            else
            {
               rc = _loadPage( pBucket, pPage, pageID, offset, len, cb ) ;
               if( rc )
               {
                  pBucket->unlock( EXCLUSIVE ) ;
                  PD_LOG( PDERROR, "Load page[ID:%d,Off:%u,Len:%u] failed, "
                          "rc: %d", pageID, offset, len, rc ) ;
                  goto error ;
               }
            }
         }

         /// create context
         context._pPage = pPage ;
         context._isWrite = TRUE ;
         context._len = len ;
         context._makeDirty = TRUE ;
         context._mode = EXCLUSIVE ;
         context._offset = offset ;
         context._pageID = pageID ;
         context._pData = ( CHAR* )pData ;
         context._pUnit = this ;
      }
      else
      {
         /// write to file directly
         rc = _pCacheFile->write( pageID, pData, len, offset, cb ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Write page[ID:%d,Off:%u,Len:%u] to "
                    "file[%s] failed, rc: %d", pageID, offset, len,
                    _pCacheFile->getFileName(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilCacheUnit::read( INT32 pageID, CHAR *pBuff,
                               UINT32 offset, UINT32 len,
                               IExecutor *cb,
                               UINT32 &readLen,
                               utilCacheContext &context )
   {
      INT32 rc = SDB_OK ;
      utilCachePage* pPage = NULL ;
      utilCacheBucket* pBucket = NULL ;

      pPage = getAndLock( pageID, offset + len, &pBucket,
                          SHARED, FALSE, cb ) ;
      if ( pPage )
      {
         /// the page
         /// -----------|<start>-------|<end>-----------------
         ///  |--A----------|     |------B--|
         /// when read in A, need to load the data
         /// when read in B, need to load the data
         if ( pPage->isDataEmpty() ||
              offset + len <= pPage->start() ||
              pPage->length() <= offset )
         {
            pBucket->unlock( SHARED ) ;
            /// read from file
            pPage = NULL ;
         }
         else if ( offset >= pPage->start() &&
                   offset + len <= pPage->length() )
         {
            /// in the data window, do nothing
         }
         else
         {
            if ( !pPage->isDirty() )
            {
               pBucket->unlock( SHARED ) ;
               pPage = NULL ;
            }
            else
            {
               /// load the data
               if ( offset < pPage->start() )
               {
                  rc = _loadPage( pBucket, pPage, pageID, offset,
                                  pPage->start() - offset, cb ) ;
                  if ( rc )
                  {
                     PD_LOG( PDERROR, "Load page[ID:%d,Off:%u,Len:%u] data "
                             "failed, rc: %d", pageID, offset,
                             pPage->start() - offset, rc ) ;
                     pBucket->unlock( SHARED ) ;
                     goto error ;
                  }
               }
               if ( offset + len > pPage->length() )
               {
                  rc = _loadPage( pBucket, pPage, pageID, pPage->length(),
                                  offset + len - pPage->length(), cb ) ;
                  if ( rc )
                  {
                     PD_LOG( PDERROR, "Load page[ID:%d,Off:%u,Len:%u] data "
                             "failed, rc: %d", pageID, pPage->length(),
                             offset + len - pPage->length(), rc ) ;
                     pBucket->unlock( SHARED ) ;
                     goto error ;
                  }
               }
            }
         }
      }

      if ( pPage )
      {
         /// create context
         context._pPage = pPage ;
         context._isWrite = FALSE ;
         context._len = len ;
         context._makeDirty = FALSE ;
         context._mode = SHARED ;
         context._offset = offset ;
         context._pageID = pageID ;
         context._pData = pBuff ;
         context._pUnit = this ;
      }
      else
      {
         rc = _pCacheFile->read( pageID, pBuff, len, offset, readLen, cb ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Read page[ID:%d,Off:%u,Len:%u] from file[%s] "
                    "failed, rc: %d", pageID, offset, len,
                    _pCacheFile->getFileName(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   UINT32 _utilCacheUnit::recyclePages( UINT32 timeout )
   {
      UINT32 count = 0 ;
      dmsCacheBucket* pBucket = NULL ;
      dmsCacheBucket::MAP_BLK_PAGE* pPages = NULL ;

      UINT64 curTime = time( NULL ) ;
      if ( _lastRecycle == curTime )
      {
         goto done ;
      }
      _lastRecycle = curTime ;

      for ( UINT32 i = 0 ; i < _bucketSize ; ++i )
      {
         pBucket = _vecBucket[ i ] ;
         pBucket->lock( EXCLUSIVE ) ;
         pPages = pBucket->getPages() ;
         dmsCacheBucket::MAP_BLK_PAGE::iterator it = pPages->begin() ;
         while ( it != pPages->end() )
         {
            dmsCachePage& page = it->second ;
            if ( page.isDirty() || time( NULL ) - page.lastTime() <= timeout )
            {
               ++it ;
               break ;
            }
            else
            {
               page.invalidate() ;
               _pMgr->release( page ) ;
               it = pPages->erase( it ) ;
               _totalPage.dec() ;
               ++count ;
            }
         }
         pBucket->unlock( EXCLUSIVE ) ;
      }

   done:
      return count ;
   }

}

