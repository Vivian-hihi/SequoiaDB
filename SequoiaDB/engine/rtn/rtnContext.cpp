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

   Source File Name = rtnContext.cpp

   Descriptive Name = Runtime Context

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains Runtime Context helper
   functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "rtnContext.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "dmsStorageUnit.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "../bson/bsonobj.h"

using namespace bson ;

namespace engine
{

   /*
      Functions
   */
   const CHAR* getContextTypeDesp( RTN_CONTEXT_TYPE type )
   {
      const _rtnContextInfo* info = sdbGetRTNContextBuilder()->find( type ) ;
      if ( NULL != info )
      {
         SDB_ASSERT( type == info->type, "invalid context info" ) ;
         SDB_ASSERT( NULL != info->newFunc, "null pointer of newFunc" ) ;

         return info->name.c_str() ;
      }
      else
      {
         return "UNKNOW" ;
      }
   }

   /*
      _rtnContextBase implement
   */
   _rtnContextBase::_rtnContextBase( INT64 contextID, UINT64 eduID )
   :_waitPrefetchNum( 0 )
   {
      _contextID           = contextID ;
      _eduID               = eduID ;

      _pResultBuffer       = NULL ;
      _resultBufferSize    = 0 ;
      _bufferCurrentOffset = 0 ;
      _bufferEndOffset     = 0 ;
      _bufferNumRecords    = 0 ;
      _totalRecords        = 0 ;

      _hitEnd              = TRUE ;
      _isOpened            = FALSE ;

      _matcher             = NULL ;
      _ownedMatcher        = FALSE ;

      _prefetchID          = 0 ;
      _isInPrefetch        = FALSE ;
      _prefetchRet         = SDB_OK ;
      _pPrefWatcher        = NULL ;
      _pMonAppCB           = NULL ;

      _countOnly           = FALSE ;
      _pDpsCB              = NULL ;
      _w                   = 1 ;
   }

   _rtnContextBase::~_rtnContextBase()
   {
      _close() ;

      if ( _pResultBuffer )
      {
         *RTN_GET_CONTEXT_FLAG( _pResultBuffer ) = 0 ;

         if ( *RTN_GET_REFERENCE( _pResultBuffer ) == 0 )
         {
            SDB_OSS_FREE( RTN_BUFF_TO_REAL_PTR( _pResultBuffer ) ) ;
            _pResultBuffer = NULL ;
         }
         else
         {
            _dataLock.release_r() ;
         }
      }

      _prefetchLock.lock_w() ;
      _prefetchLock.release_w() ;

      if ( _matcher && _ownedMatcher )
      {
         SDB_OSS_DEL _matcher ;
         _matcher = NULL ;
         _ownedMatcher = FALSE ;
      }
      _pPrefWatcher = NULL ;

      SDB_ASSERT( 0 == _waitPrefetchNum.peek(), "Has wait prefetch jobs" ) ;
      SDB_ASSERT( FALSE == _isInPrefetch, "Has prefetch job run" ) ;
   }

   void _rtnContextBase::waitForPrefetch()
   {
      _close() ;

      if ( _canPrefetch() )
      {
         _dataLock.lock_r() ;
         _dataLock.release_r() ;
      }
   }

   void _rtnContextBase::setWriteInfo( SDB_DPSCB *dpsCB, INT16 w )
   {
      _pDpsCB  = dpsCB ;
      _w       = w ;
   }

   INT32 _rtnContextBase::getReference() const
   {
      if ( _pResultBuffer )
      {
         return *RTN_GET_REFERENCE( _pResultBuffer ) ;
      }
      return 0 ;
   }

   void _rtnContextBase::enablePrefetch( _pmdEDUCB * cb,
                                         rtnPrefWatcher *pWatcher )
   {
      _prefetchID = 1 ;
      _pPrefWatcher = pWatcher ;
      _pMonAppCB = cb->getMonAppCB() ;
   }

   string _rtnContextBase::toString()
   {
      stringstream ss ;

      ss << "IsOpened:" << ( _isOpened ? 1 : 0 )
         << ",HitEnd:" << ( _hitEnd ? 1 : 0 )
         << ",BufferSize:" << _resultBufferSize ;

      if ( _totalRecords > 0 )
      {
         ss << ",TotalRecordNum:" << _totalRecords ;
      }
      if ( _bufferNumRecords > 0 )
      {
         ss << ",BufferRecordNum:" << _bufferNumRecords ;
      }

      if ( isOpened() )
      {
         if ( _matcher && !_matcher->getMatchPattern().isEmpty() )
         {
            ss << ",Matcher:" << _matcher->getMatchPattern().toString() ;
         }

         _toString( ss ) ;
      }

      return ss.str() ;
   }

   INT32 _rtnContextBase::newMatcher ()
   {
      if ( _matcher && _ownedMatcher )
      {
         SDB_OSS_DEL _matcher ;
         _matcher = NULL ;
         _ownedMatcher = FALSE ;
      }

      _matcher = SDB_OSS_NEW _mthMatchTree() ;
      if ( !_matcher )
      {
         return SDB_OOM ;
      }

      _ownedMatcher = TRUE ;
      return SDB_OK ;
   }

   #define RTN_CONTEXT_MAX_BUFF_SIZE      ( 5 * RTN_RESULTBUFFER_SIZE_MAX )

   INT32 _rtnContextBase::_reallocBuffer( SINT32 requiredSize )
   {
      INT32 rc = SDB_OK ;
      CHAR *originalPointer = _pResultBuffer ;
      SINT32 originalSize   = _resultBufferSize ;

      if ( 0 == originalSize )
      {
         _resultBufferSize = RTN_DFT_BUFFERSIZE ;
      }

      // make sure we get enough memory in result buffer
      while ( requiredSize > _resultBufferSize )
      {
         // make sure we haven't hit max
         if ( _resultBufferSize >= RTN_CONTEXT_MAX_BUFF_SIZE )
         {
            PD_LOG ( PDERROR, "Result buffer is greater than %d bytes",
                     RTN_CONTEXT_MAX_BUFF_SIZE ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         // double buffer size until hitting RTN_CONTEXT_MAX_BUFF_SIZE
         _resultBufferSize = _resultBufferSize << 1 ;
         if (_resultBufferSize > RTN_CONTEXT_MAX_BUFF_SIZE )
         {
            _resultBufferSize = RTN_CONTEXT_MAX_BUFF_SIZE ;
         }
      }

      if ( NULL == _pResultBuffer )
      {
         _pResultBuffer = ( CHAR* )SDB_OSS_MALLOC(
                                 RTN_BUFF_TO_PTR_SIZE( _resultBufferSize ) ) ;
      }
      else
      {
         // reallocate memory
         _pResultBuffer = (CHAR*)SDB_OSS_REALLOC(
                                 RTN_BUFF_TO_REAL_PTR( _pResultBuffer ),
                                 RTN_BUFF_TO_PTR_SIZE( _resultBufferSize ) ) ;
      }
      // if reallocation fail, we exit
      if ( !_pResultBuffer )
      {
         PD_LOG ( PDERROR, "Unable to allocate buffer for %d bytes",
                  _resultBufferSize ) ;
         _pResultBuffer = originalPointer ;
         _resultBufferSize = originalSize ;
         rc = SDB_OOM ;
         goto error ;
      }
      else
      {
         _pResultBuffer = RTN_REAL_PTR_TO_BUFF( _pResultBuffer ) ;

         if ( !originalPointer )
         {
            *RTN_GET_REFERENCE( _pResultBuffer ) = 0 ;
            *RTN_GET_CONTEXT_FLAG( _pResultBuffer ) = 1 ;
         }
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _rtnContextBase::append( const BSONObj &result )
   {
      INT32 rc = SDB_OK ;

      if ( !_isOpened )
      {
         _totalRecords = 0 ;
         _isOpened = TRUE ;
      }

      if ( !_countOnly )
      {
         _bufferEndOffset = ossAlign4( (UINT32)_bufferEndOffset ) ;
         if ( _bufferEndOffset + result.objsize () > _resultBufferSize )
         {
            rc = _reallocBuffer ( _bufferEndOffset + result.objsize() ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to reallocate buffer for context, rc: "
                        "%d", rc ) ;
               goto error ;
            }
         }
         ossMemcpy ( &(_pResultBuffer[_bufferEndOffset]), result.objdata(),
                     result.objsize() ) ;

         ++_totalRecords ; // total num
         ++_bufferNumRecords ; // cur buff num
         _bufferEndOffset += result.objsize() ;
      }
      else
      {
         ++_totalRecords ; // total num
         ++_bufferNumRecords ; // cur buff num
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextBase::appendObjs( const CHAR * pObjBuff, INT32 len,
                                      INT32 num, BOOLEAN needAliened )
   {
      INT32 rc = SDB_OK ;

      if ( !_isOpened )
      {
         _isOpened = TRUE ;
      }

      if ( !_countOnly )
      {
         if ( 0 < len )
         {
            if ( needAliened )
            {
               _bufferEndOffset = ossAlign4( (UINT32)_bufferEndOffset ) ;
            }

            if ( _bufferEndOffset + len > _resultBufferSize )
            {
               rc = _reallocBuffer ( _bufferEndOffset + len ) ;
               if ( rc )
               {
                  PD_LOG ( PDERROR, "Failed to reallocate buffer for context, "
                           "rc: %d", rc ) ;
                  goto error ;
               }
            }

            ossMemcpy ( &(_pResultBuffer[_bufferEndOffset]), pObjBuff, len ) ;
         }

         _totalRecords += num ; // total num
         _bufferNumRecords += num ; // cur buff num
         _bufferEndOffset += len ;
      }
      else
      {
         _totalRecords += num ; // total num
         _bufferNumRecords += num ; // cur buff num
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextBase::_onDataEmpty ()
   {
      if ( _canPrefetch() && 0 != _prefetchID )
      {
         SDB_BPSCB *bpsCB = pmdGetKRCB()->getBPSCB() ;
         if ( bpsCB->isPrefetchEnabled() )
         {
            _prefetchLock.lock_r() ;
            if ( SDB_OK == bpsCB->sendPrefechReq( bpsDataPref( _prefetchID,
                                                               this ) ) )
            {
               _waitPrefetchNum.inc() ;
            }
            else
            {
               _prefetchLock.release_r() ;
            }
         }
      }
   }

   INT32 _rtnContextBase::prefetch( pmdEDUCB * cb, UINT32 prefetchID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN locked = FALSE ;
      BOOLEAN againTry = FALSE ;
      UINT32 timeout = 0 ;

      while ( timeout < OSS_ONE_SEC )
      {
         if ( prefetchID != _prefetchID )
         {
            goto done ;
         }
         rc = _dataLock.lock_w( 100 ) ;
         if ( SDB_OK == rc )
         {
            locked = TRUE ;
            _isInPrefetch = TRUE ;
            if ( _pPrefWatcher )
            {
               _pPrefWatcher->ntyBegin() ;
            }
            break ;
         }
         else if ( rc && SDB_TIMEOUT != rc )
         {
            goto error ;
         }
         timeout += 100 ;
      }

      if ( FALSE == locked )
      {
         goto error ;
      }

      if ( prefetchID != _prefetchID )
      {
         goto done ;
      }

      if ( !isOpened() || eof() || !isEmpty() )
      {
         goto done ;
      }

      if ( _pMonAppCB && cb->getID() != eduID() )
      {
         cb->getMonAppCB()->reset() ;
      }
      rc = _prepareData( cb ) ;
      _prefetchRet = rc ;
      if ( rc && SDB_DMS_EOC != rc )
      {
         PD_LOG( PDWARNING, "Prepare data failed, rc: %d", rc ) ;
      }

      if ( _pMonAppCB && cb->getID() != eduID() )
      {
         *_pMonAppCB += *cb->getMonAppCB() ;
         _monCtxCB.dataRead += _pMonAppCB->totalDataRead ;
         cb->getMonAppCB()->reset() ;
      }

      if ( SDB_OK == rc && isEmpty() && isOpened() && !eof() &&
           SDB_OK == pmdGetKRCB()->getBPSCB()->sendPrefechReq(
                     bpsDataPref( ++_prefetchID, this ), TRUE ) )
      {
         _waitPrefetchNum.inc() ;
         againTry = TRUE ;
      }

   done:
      // inc idle
      pmdGetKRCB()->getBPSCB()->_idlePrefAgentNum.inc() ;
      if ( locked )
      {
         _isInPrefetch = FALSE ;
         if ( _pPrefWatcher )
         {
            _pPrefWatcher->ntyEnd() ;
         }
         _dataLock.release_w() ;
      }
      _waitPrefetchNum.dec() ;
      if ( FALSE == againTry )
      {
         _prefetchLock.release_r() ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextBase::_prepareMoreData( _pmdEDUCB *cb )
   {
      const INT32 PREPARE_DATA_SIZE_LIMIT = 512 * 1024 ; // 512KB
      const UINT32 PREPARE_TIMEOUT = 2000 ; // 2ms

      INT32 rc = SDB_OK ;
      UINT64 beginTime ;

      SDB_ASSERT( isEmpty(), "buf is not empty" ) ;

      beginTime = ossGetCurrentMicroseconds() ;

      while ( !eof() )
      {
         INT32 startOffset = _bufferEndOffset ;
         INT32 currentPreparedSize ;
         UINT64 currentTime ;

         rc = _prepareData( cb ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         currentPreparedSize = _bufferEndOffset - startOffset ;

         // assume next prepared size equals current,
         // so break if data size exceeds limit when prepare once more time
         if ( _bufferEndOffset + currentPreparedSize >= PREPARE_DATA_SIZE_LIMIT )
         {
            break ;
         }

         // prepare timeout
         currentTime = ossGetCurrentMicroseconds() ;
         if ( currentTime - beginTime >= PREPARE_TIMEOUT )
         {
            break ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextBase::getMore( INT32 maxNumToReturn,
                                   rtnContextBuf &buffObj,
                                   pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN locked = FALSE ;

      // release buff obj
      buffObj.release() ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE ;
         goto error ;
      }
      else if ( eof() && isEmpty() )
      {
         rc = SDB_DMS_EOC ;
         _isOpened = FALSE ;
         goto error ;
      }

      // need to get data lock
      while ( TRUE )
      {
         rc = _dataLock.lock_r( OSS_ONE_SEC ) ;
         if ( SDB_OK == rc )
         {
            locked = TRUE ;
            break ;
         }
         else if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }
      }

      if ( 0 != _prefetchID )
      {
         ++_prefetchID ;
      }
      // check prefetch has error
      if ( _prefetchRet && SDB_DMS_EOC != _prefetchRet )
      {
         rc = _prefetchRet ;
         PD_LOG( PDWARNING, "Occur error in prefetch, rc: %d", rc ) ;
         goto error ;
      }

      // need to get more datas
      if ( isEmpty() && !eof() )
      {
         UINT64 tmpTotalRead = cb->getMonAppCB()->totalDataRead ;

         if ( _canPrepareMoreData() )
         {
            rc = _prepareMoreData( cb ) ;
         }
         else
         {
            rc = _prepareData( cb ) ;
         }

         if ( rc && SDB_DMS_EOC != rc )
         {
            PD_LOG( PDERROR, "Prepare data failed, rc: %d", rc ) ;
            goto error ;
         }

         _monCtxCB.dataRead += ( cb->getMonAppCB()->totalDataRead -
                                 tmpTotalRead ) ;
      }

      // if not empty, get current data
      if ( !isEmpty() )
      {
         if ( !_countOnly )
         {
            _bufferCurrentOffset = ossAlign4( (UINT32)_bufferCurrentOffset ) ;
            buffObj._pOrgBuff = _pResultBuffer ;
            buffObj._pBuff = &_pResultBuffer[ _bufferCurrentOffset ] ;
            buffObj._startFrom = _totalRecords - _bufferNumRecords ;

            // return cur all
            if ( maxNumToReturn < 0 )
            {
               buffObj._buffSize = _bufferEndOffset - _bufferCurrentOffset ;
               buffObj._recordNum = _bufferNumRecords ;
               // clean info
               _bufferCurrentOffset = _bufferEndOffset ;
               _bufferNumRecords = 0 ;
            }
            else
            {
               INT32 prevCurOffset = _bufferCurrentOffset ;
               while ( _bufferCurrentOffset < _bufferEndOffset &&
                       maxNumToReturn > 0 )
               {
                  try
                  {
                     BSONObj obj( &_pResultBuffer[_bufferCurrentOffset] ) ;
                     _bufferCurrentOffset += ossAlign4( (UINT32)obj.objsize() ) ;
                  }
                  catch ( std::exception &e )
                  {
                     PD_LOG( PDERROR, "Can't convert into BSON object: %s",
                             e.what() ) ;
                     rc = SDB_SYS ;
                     goto error ;
                  }

                  ++buffObj._recordNum ;
                  --_bufferNumRecords ;
                  --maxNumToReturn ;
               } // end while

               if ( _bufferCurrentOffset > _bufferEndOffset )
               {
                  _bufferCurrentOffset = _bufferEndOffset ;
                  SDB_ASSERT( 0 == _bufferNumRecords, "buffer num records must "
                              " be zero" ) ;
               }
               buffObj._buffSize = _bufferCurrentOffset - prevCurOffset ;
            }

            buffObj._reference( RTN_GET_REFERENCE( _pResultBuffer ), &_dataLock ) ;
            locked = FALSE ;
            rc = SDB_OK ;

            // if get all data
            if ( isEmpty() && !eof() )
            {
               _bufferCurrentOffset = 0 ;
               _bufferEndOffset     = 0 ;

               _onDataEmpty() ;
            }
         }
         else
         {
            if ( maxNumToReturn < 0 || maxNumToReturn >= _bufferNumRecords )
            {
               buffObj._recordNum = _bufferNumRecords ;
               _bufferNumRecords = 0 ;
            }
            else
            {
               buffObj._recordNum = maxNumToReturn ;
               _bufferNumRecords -= maxNumToReturn ;
            }
         }
      }
      else
      {
         rc = SDB_DMS_EOC ;
         _isOpened = FALSE ;
      }

   done:
      if ( locked )
      {
         _dataLock.release_r() ;
      }
      return rc ;
   error:
      goto done ;
   }

   UINT32 _rtnContextBase::getCachedRecordNum()
   {
      return _bufferNumRecords ;
   }

   _rtnContextAssit::_rtnContextAssit( RTN_CONTEXT_TYPE type,
                                            std::string name,
                                            RTN_CTX_NEW_FUNC func )
   {
      SDB_ASSERT( NULL != func, "func is null" ) ;
      SDB_ASSERT( !name.empty(), "name is empty" ) ;

      sdbGetRTNContextBuilder()->_register( type, name, func ) ;
   }

   _rtnContextAssit::~_rtnContextAssit()
   {
   }

   _rtnContextBuilder::_rtnContextBuilder()
   {
   }

   _rtnContextBuilder::~_rtnContextBuilder()
   {
      _releaseContextInfos() ;
   }

   _rtnContextBase* _rtnContextBuilder::create ( RTN_CONTEXT_TYPE type,
                                                   INT64 contextId,
                                                   EDUID eduId )
   {
      const _rtnContextInfo* info = find( type ) ;
      if ( NULL != info )
      {
         SDB_ASSERT( type == info->type, "invalid context info" ) ;
         SDB_ASSERT( NULL != info->newFunc, "null pointer of newFunc" ) ;

         _rtnContextBase* ctx = (*(info->newFunc))( contextId, eduId ) ;
         SDB_ASSERT( ctx->name() == info->name, "name is wrong" ) ;
         SDB_ASSERT( ctx->getType() == info->type, "type is wrong" ) ;
         return ctx ;
      }
      else
      {
         SDB_ASSERT( FALSE, "unknown RTN_CONTEXT_TYPE" ) ;
         return NULL ;
      }
   }

   void _rtnContextBuilder::release ( _rtnContextBase* context )
   {
      if ( NULL != context)
      {
         SDB_OSS_DEL context ;
      }
   }

   const _rtnContextInfo* _rtnContextBuilder::find( RTN_CONTEXT_TYPE type ) const
   {
      ctx_info_iterator it = _contextInfoMap.find( type ) ;
      if ( it != _contextInfoMap.end() )
      {
         return (*it).second ;
      }
      else
      {
         return NULL ;
      }
   }

   INT32 _rtnContextBuilder::_register( RTN_CONTEXT_TYPE type,
                                       std::string name,
                                       RTN_CTX_NEW_FUNC func )
   {
      INT32 rc = SDB_OK ;
      _rtnContextInfo* newInfo = NULL ;

      const _rtnContextInfo* info = find( type ) ;
      if ( NULL != info )
      {
         PD_LOG( PDERROR, "RTN context info is registered: type=%d, name=%s",
                 info->type, info->name.c_str() ) ;
         SDB_ASSERT( FALSE, "RTN context info is registered" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      newInfo = SDB_OSS_NEW _rtnContextInfo() ;
      if ( NULL == newInfo )
      {
         rc = SDB_OOM ;
         goto error ;
      }

      newInfo->type = type ;
      newInfo->name = name ;
      newInfo->newFunc = func ;

      rc = _insert( newInfo ) ;
      if ( SDB_OK != rc )
      {
         SDB_OSS_DEL newInfo ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextBuilder::_insert( _rtnContextInfo* contextInfo )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( NULL != contextInfo, "contextInfo is null") ;

      try
      {
         _contextInfoMap.insert( pair_type( contextInfo->type, contextInfo ) ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG(PDERROR, "unexpected error happened: %s", e.what());
         rc = SDB_SYS ;
      }

      return rc ;
   }

   void _rtnContextBuilder::_releaseContextInfos()
   {
      for ( ctx_info_iterator it = _contextInfoMap.begin() ;
            it != _contextInfoMap.end() ;
            it++ )
      {
         _rtnContextInfo* info = (*it).second ;
         SDB_ASSERT( NULL != info, "info is null" ) ;
         SDB_OSS_DEL info ;
      }

      _contextInfoMap.clear() ;
   }

   _rtnContextBuilder* sdbGetRTNContextBuilder()
   {
      static _rtnContextBuilder ctxBuilder ;
      return &ctxBuilder ;
   }
}

