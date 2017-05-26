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
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "rtnContext.hpp"
#include "dmsStorageUnit.hpp"
#include "msgMessage.hpp"
#include "spdSession.hpp"
#include "rtn.hpp"
#include "dpsOp2Record.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

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
      const INT32 PREPARE_DATA_SIZE_LIMIT = 1024 * 1024 ; // 1MB
      const UINT32 PREPARE_TIMEOUT = 10000 ; // 10ms

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

   /*
      _rtnContextQGM implement
   */

   RTN_CTX_AUTO_REGISTER(_rtnContextQGM, RTN_CONTEXT_QGM, "QGM")

   _rtnContextQGM::_rtnContextQGM( INT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _accPlan    = NULL ;
   }

   _rtnContextQGM::~_rtnContextQGM ()
   {
      if ( NULL != _accPlan )
      {
         SAFE_OSS_DELETE( _accPlan ) ;
         _accPlan = NULL ;
      }
   }

   std::string _rtnContextQGM::name() const
   {
      return "QGM" ;
   }

   RTN_CONTEXT_TYPE _rtnContextQGM::getType () const
   {
      return RTN_CONTEXT_QGM ;
   }

   INT32 _rtnContextQGM::open( qgmPlanContainer *accPlan )
   {
      INT32 rc = SDB_OK ;

      if ( _isOpened )
      {
         rc = SDB_DMS_CONTEXT_IS_OPEN ;
         goto error ;
      }
      if ( NULL == accPlan )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      _accPlan = accPlan ;
      _isOpened = TRUE ;
      _hitEnd = FALSE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextQGM::_prepareData( pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      INT32 index = 0 ;
      monAppCB *pMonAppCB = cb ? cb->getMonAppCB() : NULL ;

      for ( ; index < RTN_CONTEXT_GETNUM_ONCE ; ++index )
      {
         try
         {
            rc = _accPlan->fetch( obj ) ;
         }
         catch( std::exception &e )
         {
            PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         if ( SDB_DMS_EOC == rc )
         {
            _hitEnd = TRUE ;
            break ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Qgm fetch failed, rc: %d", rc ) ;
            goto error ;
         }

         rc = append( obj ) ;
         PD_RC_CHECK( rc, PDERROR, "Append obj[%s] failed, rc: %d",
                      obj.toString().c_str(), rc ) ;

         // increase counter
         DMS_MON_OP_COUNT_INC( pMonAppCB, MON_SELECT, 1 ) ;

         // make sure we still have room to read another
         // record_max_sz (i.e. 16MB). if we have less than 16MB
         // to 256MB, we can't safely assume the next record we
         // read will not overflow the buffer, so let's just break
         // before reading the next record
         if ( buffEndOffset() + DMS_RECORD_MAX_SZ > RTN_RESULTBUFFER_SIZE_MAX )
         {
            // let's break if there's no room for another max record
            break ;
         }
      }

      if ( !isEmpty() )
      {
         rc = SDB_OK ;
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

   /*
      _rtnContextSP define
   */

   RTN_CTX_AUTO_REGISTER(_rtnContextSP, RTN_CONTEXT_SP, "SP")

   _rtnContextSP::_rtnContextSP( INT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID ),
    _sp(NULL)
   {

   }

   _rtnContextSP::~_rtnContextSP()
   {
      SAFE_OSS_DELETE( _sp ) ;
   }

   std::string _rtnContextSP::name() const
   {
      return "SP" ;
   }

   RTN_CONTEXT_TYPE _rtnContextSP::getType() const
   {
      return RTN_CONTEXT_SP ;
   }

   INT32 _rtnContextSP::open( _spdSession *sp )
   {
      INT32 rc = SDB_OK ;
      if ( _isOpened )
      {
         rc = SDB_DMS_CONTEXT_IS_OPEN ;
         goto error ;
      }
      if ( NULL == sp )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      _sp = sp ;
      _isOpened = TRUE ;
      _hitEnd = FALSE ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32  _rtnContextSP::_prepareData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      monAppCB *pMonAppCB = cb ? cb->getMonAppCB() : NULL ;

      for ( INT32 i = 0; i < RTN_CONTEXT_GETNUM_ONCE; i++ )
      {
         rc = _sp->next( obj ) ;
         if ( SDB_DMS_EOC == rc )
         {
            _hitEnd = TRUE ;
            break ;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to fetch spdSession:%d", rc ) ;
            goto error ;
         }
         else
         {
            rc = append( obj ) ;
            PD_RC_CHECK( rc, PDERROR, "Append obj[%s] failed, rc: %d",
                      obj.toString().c_str(), rc ) ;
         }

         DMS_MON_OP_COUNT_INC( pMonAppCB, MON_SELECT, 1 ) ;

         if ( buffEndOffset() + DMS_RECORD_MAX_SZ > RTN_RESULTBUFFER_SIZE_MAX )
         {
            break ;
         }
      }

      if ( !isEmpty() )
      {
         rc = SDB_OK ;
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

   /*
      _coordOrderKey implement
   */
   _coordOrderKey::_coordOrderKey ( const _coordOrderKey &orderKey )
   {
      _orderBy = orderKey._orderBy ;
      _hash = orderKey._hash ;
      _keyObj = orderKey._keyObj ;
      _arrEle = orderKey._arrEle ;
   }

   _coordOrderKey::_coordOrderKey ()
   {
      _hash.hash = 0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORDERKEY_OPELT, "coordOrderKey::operator<" )
   BOOLEAN _coordOrderKey::operator<( const _coordOrderKey &rhs ) const
   {
      PD_TRACE_ENTRY ( SDB_COORDERKEY_OPELT ) ;
      BOOLEAN result = FALSE ;
      INT32 rsCmp = _keyObj.woCompare( rhs._keyObj, _orderBy, FALSE ) ;
      if ( rsCmp < 0
         || ( 0 == rsCmp && _hash.hash < rhs._hash.hash ))
      {
         result = TRUE ;
      }
      PD_TRACE1 ( SDB_COORDERKEY_OPELT, PD_PACK_INT(result) );
      PD_TRACE_EXIT ( SDB_COORDERKEY_OPELT ) ;
      return result;
   }

   void _coordOrderKey::clear()
   {
      _arrEle = BSONElement() ;
      _hash.columns.hash1 = 0 ;
      _hash.columns.hash2 = 0 ;
   }

   void _coordOrderKey::setOrderBy( const BSONObj &orderBy )
   {
      _orderBy = orderBy;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORDERKEY_GENKEY, "coordOrderKey::generateKey" )
   INT32 _coordOrderKey::generateKey( const BSONObj &record,
                                    _ixmIndexKeyGen *keyGen )
   {
      INT32 rc = SDB_OK;
      SDB_ASSERT( keyGen != NULL, "keyGen can't be null!" ) ;
      PD_TRACE_ENTRY ( SDB_COORDERKEY_GENKEY ) ;
      clear();
      BSONObjSet keySet( _orderBy ) ;

      rc = keyGen->getKeys( record, keySet, &_arrEle ) ;
      PD_RC_CHECK( rc, PDERROR,
                  "failed to generate order-key(rc=%d)",
                  rc ) ;
      SDB_ASSERT( !keySet.empty(), "empty key-set!" ) ;
      _keyObj = *(keySet.begin()) ;
      if ( _arrEle.eoo() )
      {
         _hash.hash = 0 ;
      }
      else
      {
         ixmMakeHashValue( _arrEle, _hash ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_COORDERKEY_GENKEY, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   _rtnSubCLBuf::_rtnSubCLBuf()
   {
      _isOrderKeyChange = TRUE;
      _remainNum = 0;
   }

   _rtnSubCLBuf::_rtnSubCLBuf( BSONObj &orderBy,
                               _ixmIndexKeyGen *keyGen )
   {
      _orderKey.setOrderBy( orderBy );
      _isOrderKeyChange = TRUE;
      _remainNum = 0;
      _keyGen = keyGen;
   }

   _rtnSubCLBuf::~_rtnSubCLBuf()
   {
      _keyGen = NULL;
   }

   const CHAR* _rtnSubCLBuf::front()
   {
      return _buffer.front();
   }

   INT32 _rtnSubCLBuf::pop()
   {
      INT32 rc = SDB_OK;
      BSONObj obj;
      _isOrderKeyChange = TRUE;
      _remainNum--;
      rc = _buffer.nextObj( obj );
      if ( _remainNum <= 0 )
      {
         rtnContextBuf emptyBuf;
         _buffer = emptyBuf;
      }
      return rc;
   }

   INT32 _rtnSubCLBuf::popN( SINT32 num )
   {
      INT32 rc = SDB_OK;
      _isOrderKeyChange = TRUE;
      if ( num >= recordNum() )
      {
         rc = popAll();
         goto done;
      }
      while ( num > 0 )
      {
         rc = pop();
         if ( rc )
         {
            goto error;
         }
         --num;
      }
   done:
      return rc;
   error:
      goto done;
   }
   INT32 _rtnSubCLBuf::popAll()
   {
      _isOrderKeyChange = TRUE;
      _remainNum = 0;
      rtnContextBuf emptyBuf;
      _buffer = emptyBuf;
      return SDB_OK;
   }

   INT32 _rtnSubCLBuf::recordNum()
   {
      return _remainNum;
   }

   INT32 _rtnSubCLBuf::getOrderKey( coordOrderKey &orderKey )
   {
      INT32 rc = SDB_OK;
      if ( _isOrderKeyChange )
      {
         if ( recordNum() <= 0 )
         {
            _orderKey.clear();
         }
         else
         {
            try
            {
               BSONObj boRecord( front() );
               rc = _orderKey.generateKey( boRecord, _keyGen );
               PD_RC_CHECK( rc, PDERROR, "Failed to get order-key(rc=%d)",
                            rc );
            }
            catch ( std::exception &e )
            {
               PD_RC_CHECK( SDB_INVALIDARG, PDERROR,
                            "Occur unexpected error:%s", e.what() ) ;
            }
         }
      }
      orderKey = _orderKey;
      _isOrderKeyChange = FALSE;
   done:
      return rc;
   error:
      goto done;
   }

   rtnContextBuf _rtnSubCLBuf::buffer()
   {
      return _buffer;
   }

   void _rtnSubCLBuf::setBuffer( rtnContextBuf &buffer )
   {
      _buffer = buffer;
      _isOrderKeyChange = TRUE;
      _remainNum = buffer.recordNum();
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextMainCL, RTN_CONTEXT_MAINCL, "MAINCL")

   _rtnContextMainCL::_rtnContextMainCL( INT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID ),
    _includeShardingOrder( FALSE ),
    _keyGen( NULL ),
    _numToReturn( -1 ),
    _numToSkip( 0 )
   {

   }
   _rtnContextMainCL::~_rtnContextMainCL()
   {
      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();
      pmdEDUCB *cb = pKrcb->getEDUMgr()->getEDUByID( eduID() );
      SubCLBufList::iterator iterLst = _subCLBufList.begin();
      while( iterLst != _subCLBufList.end() )
      {
         pRtncb->contextDelete( iterLst->first, cb );
         ++iterLst;
      }
      _subCLBufList.clear();
      SAFE_OSS_DELETE( _keyGen );
   }

   std::string _rtnContextMainCL::name() const
   {
      return "MAINCL" ;
   }

   RTN_CONTEXT_TYPE _rtnContextMainCL::getType () const
   {
      return RTN_CONTEXT_MAINCL;
   }

   INT32 _rtnContextMainCL::open( const _rtnQueryOptions &options,
                                  const std::vector<string> &subs,
                                  BOOLEAN shardSort,
                                  _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      rc = _initArgs( options, subs, shardSort ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init args:%d", rc ) ;
         goto error ;
      }

      rc = _initCLBuf( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init cl buf:%d", rc ) ;
         goto error ;
      }

      _isOpened = TRUE ;
      _hitEnd = ( 0 == _options._limit ) ||
                ( _subCLBufList.empty() ) ;
   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextMainCL::open( const bson::BSONObj & orderBy,
                                  INT64 numToReturn,
                                  INT64 numToSkip )
   {
      INT32 rc = SDB_OK;
      _options._orderBy = orderBy.getOwned();
      _numToSkip = numToSkip ;
      _numToReturn = numToReturn ;
      _keyGen = SDB_OSS_NEW _ixmIndexKeyGen( _options._orderBy ) ;
      PD_CHECK( _keyGen != NULL, SDB_OOM, error, PDERROR,
                "malloc failed!" ) ;

      _isOpened = TRUE;
      _hitEnd = 0 == numToReturn ;
   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextMainCL::_initCLBuf( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      INT32 loop = 0 ;
      SINT64 contextID = -1 ;

      loop = ( !(_options._orderBy.isEmpty()) &&
               !_includeShardingOrder ) ?
             _subs.size() : 5 ;


      while ( 0 < loop-- )
      {
         contextID = -1 ;
         rc = _getNextContext( cb, contextID ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get next context:%d", rc ) ;
            goto error ;
         }

         if ( -1 == contextID )
         {
            break ;
         }
         else
         {
            rc = addSubContext( contextID ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to add context to main context:%d", rc ) ;
               goto error ;
            }
         }
      }
   done:
      return rc ;
   error:
      if ( -1 != contextID )
      {
         sdbGetRTNCB()->contextDelete( contextID, cb ) ;
      }
      goto done ;
   }

   INT32 _rtnContextMainCL::_getNextContext( _pmdEDUCB *cb,
                                             SINT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      SINT64 context = -1 ;
      _SDB_RTNCB *rtnCB = sdbGetRTNCB() ;
      rtnContextBase *contextObj = NULL ;
      if ( !_subs.empty() )
      {
         const string &clName = *( _subs.begin() ) ;
         rc = rtnQuery( clName.c_str(),
                        _options._selector,
                        _options._query,
                        _options._orderBy,
                        _options._hint,
                        _options._flag,
                        cb, _options._skip,
                        _options._limit,
                        sdbGetDMSCB(), rtnCB,
                        context,
                        &contextObj ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to query on cl:%s, rc:%d",
                    clName.c_str(), rc ) ;
            goto error ;
         }

         if ( NULL != contextObj && contextObj->isWrite() )
         {
            contextObj->setWriteInfo( this->getDPSCB(),
                                      this->getW() ) ;
         }

         _subs.pop_front() ;
         /// do not use clName again.
      }
   done:
      contextID = context ;
      return rc ;
   error:
      if ( -1 != context )
      {
         rtnCB->contextDelete( context, cb ) ;
         context = -1 ;
      }
      goto done ;
   }

   INT32 _rtnContextMainCL::_initArgs( const _rtnQueryOptions &options,
                                       const std::vector<string> &subs,
                                       BOOLEAN shardSort )
   {
      INT32 rc = SDB_OK ;
      _options = options ;
      rc = _options.getOwned() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get owned:%d", rc ) ;
         goto error ;
      }

      _numToReturn = _options._limit ;
      _numToSkip = _options._skip ;
      _includeShardingOrder = shardSort ;

      /// _options._skip will be used in sub query.
      _options._skip = 0 ;
      _keyGen = SDB_OSS_NEW _ixmIndexKeyGen( _options._orderBy ) ;
      PD_CHECK( _keyGen != NULL, SDB_OOM, error, PDERROR,
                "malloc failed!" ) ;

      if ( subs.size() <= 1 )
      {
         _includeShardingOrder = FALSE ;
         _options._skip = _numToSkip ;
         _numToSkip = 0 ;
      }
      else
      {
         if ( 0 < _numToSkip && 0 < _numToReturn )
         {
            _options._limit = _numToSkip + _numToReturn ;
         }
      }

      for ( std::vector<string>::const_iterator itr = subs.begin() ;
            itr != subs.end() ;
            ++itr )
      {
         _subs.push_back( *itr ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextMainCL::addSubContext( SINT64 contextID )
   {
      rtnSubCLBuf emptyCTXBuf( _options._orderBy, _keyGen ) ;
      _subCLBufList[ contextID ] = emptyCTXBuf;
      return SDB_OK;
   }

   BOOLEAN _rtnContextMainCL::requireOrder () const
   {
      return 1 < _subCLBufList.size() && !(_options._orderBy.isEmpty() ) ;
   }

   INT32 _rtnContextMainCL::getMore( INT32 maxNumToReturn,
                                     rtnContextBuf &buffObj,
                                     _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      BOOLEAN hasData = FALSE;
      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();

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

      // OrderBy: get data one by one and caused copy
      if ( !isEmpty() || ( requireOrder() && !_includeShardingOrder ) )
      {
         rc = this->_rtnContextBase::getMore( maxNumToReturn, buffObj,
                                              cb );
         goto done;
      }

      // buffer is empty and not need order,
      // directly get data from sub-context
      while( !hasData )
      {
         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         // skip the records
         while ( _numToSkip > 0 )
         {
            SubCLBufList::iterator iterSubCTXSkip = _subCLBufList.begin();
            if ( _subCLBufList.end() == iterSubCTXSkip ||
                 iterSubCTXSkip->second.recordNum() <= 0 )
            {
               break;
            }
            if ( _numToSkip >= iterSubCTXSkip->second.recordNum() )
            {
               _numToSkip -= iterSubCTXSkip->second.recordNum();
               iterSubCTXSkip->second.popAll();
            }
            else
            {
               iterSubCTXSkip->second.popN( _numToSkip );
               _numToSkip = 0;

               // popN() only changed the offset of rtnContextBuf, so it's
               // need to jump over '_numToSkip' records
               rtnContextBuf buf = iterSubCTXSkip->second.buffer() ;
               rc = appendObjs( buf.front(),
                                buf.size() - buf.offset(),
                                iterSubCTXSkip->second.recordNum() ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to append objs, rc: %d", rc ) ;

               /// clear data in buff
               iterSubCTXSkip->second.popAll();

               rc = this->_rtnContextBase::getMore( maxNumToReturn,
                                                    buffObj, cb );
               goto done ;
            }
         }

         SINT64 curContext = -1 ;
         SubCLBufList::iterator iterSubCTX = _subCLBufList.begin();
         if ( _subCLBufList.end() == iterSubCTX )
         {
            _hitEnd = TRUE ;
            _isOpened = FALSE ;
            rc = SDB_DMS_EOC;
            goto error ;
         }

         curContext = iterSubCTX->first ;
         if ( iterSubCTX->second.recordNum() <= 0 )
         {
            rc = _prepareSubCTXData( curContext, cb, maxNumToReturn ) ;
            if ( rc != SDB_OK )
            {
               pRtncb->contextDelete( curContext, cb );
               _subCLBufList.erase( curContext );
               if ( SDB_DMS_EOC != rc )
               {
                  goto error;
               }
            }
            continue ;
         }
         buffObj = iterSubCTX->second.buffer() ;
         iterSubCTX->second.popAll() ;
         hasData = TRUE ;

         if ( _numToReturn > 0 )
         {
            if ( buffObj.recordNum() > _numToReturn )
            {
               buffObj.truncate( _numToReturn ) ;
            }
            _numToReturn -= buffObj.recordNum() ;
         }
      }

      if ( 0 == _numToReturn )
      {
         _hitEnd = TRUE ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextMainCL::_prepareSubCTXData( SINT64 contextID,
                                                _pmdEDUCB * cb,
                                                INT32 maxNumToReturn )
   {
      INT32 rc = SDB_OK;
      _SDB_RTNCB *pRtnCB = pmdGetKRCB()->getRTNCB();
      rtnContext *pContext = NULL;
      rtnContextBuf contextBuf;
      SubCLBufList::iterator iterSubCTX = _subCLBufList.find( contextID ) ;
      if ( _subCLBufList.end() == iterSubCTX )
      {
         PD_LOG( PDERROR, "can not find context[%lld] in local context buf",
                 contextID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( iterSubCTX->second.recordNum() > 0 )
      {
         goto done;
      }

      pContext = pRtnCB->contextFind( contextID );
      PD_CHECK( pContext, SDB_RTN_CONTEXT_NOTEXIST, error, PDERROR,
                "Context %lld does not exist", iterSubCTX->first );
      rc = pContext->getMore( maxNumToReturn,
                              contextBuf,
                              cb );
      if ( SDB_OK == rc )
      {
         iterSubCTX->second.setBuffer( contextBuf ) ;
      }
      else if ( SDB_DMS_EOC == rc )
      {
         INT32 rcTmp = SDB_OK ;
         SINT64 context = -1 ;
         rcTmp = _getNextContext( cb, context ) ;
         if ( SDB_OK != rcTmp )
         {
            PD_LOG( PDERROR, "failed to get next context:%d", rcTmp ) ;
            rc = rcTmp ;
            goto error ;
         }
         else if ( -1 != context )
         {
            rcTmp = addSubContext( context ) ;
            if ( SDB_OK != rcTmp )
            {
               PD_LOG( PDERROR, "failed to add context:%d", rc ) ;
               rc = rcTmp ;
               sdbGetRTNCB()->contextDelete( context, cb ) ;
               goto error ;
            }
         }
         else
         {
            SDB_ASSERT( _subs.empty(), "must be empty" ) ;
            /// do nothing.
         }
      }
      else
      {
         PD_LOG( PDERROR, "getmore failed(rc=%d)", rc );
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextMainCL::_prepareData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      SDB_ASSERT( requireOrder(), "here should be order!" );
      SDB_ASSERT( _subs.empty(), "should be empty" ) ;
      rc = _prepareDataByOrder( cb );
      return rc;
   }

   void _rtnContextMainCL::_toString( stringstream &ss )
   {
      if ( !_options._orderBy.isEmpty() )
      {
         ss << ",Orderby:" << _options._orderBy.toString().c_str()
            << ",IsShardingOrder:" << _includeShardingOrder ;
      }
      if ( _numToReturn > 0 )
      {
         ss << ",NumToReturn:" << _numToReturn ;
      }
      if ( _numToSkip > 0 )
      {
         ss << ",NumToSkip:" << _numToSkip ;
      }
   }

   INT32 _rtnContextMainCL::_prepareDataByOrder( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();

      if ( !_subs.empty() )
      {
         SDB_ASSERT( FALSE, "shoud be empty" ) ;
         PD_LOG( PDERROR, "subs shoud be empty" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      while ( 0 != _numToReturn )
      {
         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         SubCLBufList::iterator iterSubCTXFirst = _subCLBufList.begin();
         if ( _subCLBufList.end() == iterSubCTXFirst )
         {
            _hitEnd = TRUE ;
            if ( isEmpty() )
            {
               rc = SDB_DMS_EOC;
            }
            break;
         }
         if ( iterSubCTXFirst->second.recordNum() <= 0 )
         {
            if ( !isEmpty() )
            {
               goto done;
            }
            /// _subs shoud be empty in this function.
            /// so no new element will be put into _subCLBufList.
            rc = _prepareSubCTXData( iterSubCTXFirst->first, cb, -1 );
            if ( rc )
            {
               pRtncb->contextDelete( iterSubCTXFirst->first, cb );
               _subCLBufList.erase( iterSubCTXFirst );
               if ( rc != SDB_DMS_EOC )
               {
                  goto error;
               }
               continue;
            }
         }
         SubCLBufList::iterator iterSubCTXCur = iterSubCTXFirst;
         ++iterSubCTXCur;
         coordOrderKey firstOrderKey;
         coordOrderKey curOrderKey;
         rc = iterSubCTXFirst->second.getOrderKey( firstOrderKey );
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to generate order-key(rc=%d)", rc );
         while( iterSubCTXCur != _subCLBufList.end() )
         {
            if ( iterSubCTXCur->second.recordNum() <= 0 )
            {
               if ( !isEmpty() )
               {
                  goto done;
               }
               rc = _prepareSubCTXData( iterSubCTXCur->first, cb, -1 );
               if ( rc )
               {
                  pRtncb->contextDelete( iterSubCTXCur->first, cb );
                  iterSubCTXCur = _subCLBufList.erase( iterSubCTXCur );
                  if ( rc != SDB_DMS_EOC )
                  {
                     goto error;
                  }
                  continue;
               }
            }
            rc = iterSubCTXCur->second.getOrderKey( curOrderKey );
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to generate order-key(rc=%d)", rc );
            if ( curOrderKey < firstOrderKey )
            {
               iterSubCTXFirst = iterSubCTXCur;
               firstOrderKey = curOrderKey;
            }
            ++iterSubCTXCur;
         }

         if ( _numToSkip <= 0 )
         {
            try
            {
               BSONObj obj( iterSubCTXFirst->second.front() );
               rc = append( obj ) ;
               PD_RC_CHECK( rc, PDERROR,
                            "Failed to append data(rc=%d)", rc );
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "occur unexpected error:%s", e.what() );
               goto error;
            }

            if ( _numToReturn > 0 )
            {
               --_numToReturn ;
            }
         }
         else
         {
            --_numToSkip ;
         }
         rc = iterSubCTXFirst->second.pop();
         if ( rc )
         {
            goto error;
         }
      }

      if ( 0 == _numToReturn )
      {
         _hitEnd = TRUE ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextQgmSort, RTN_CONTEXT_QGMSORT, "QGMSORT")

   _rtnContextQgmSort::_rtnContextQgmSort( INT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID ),
    _qp(NULL)
   {

   }

   _rtnContextQgmSort::~_rtnContextQgmSort()
   {
     /// qgmPlan should be released by plan tree.
      _qp = NULL ;
   }

   std::string _rtnContextQgmSort::name() const
   {
      return "QGMSORT" ;
   }

   RTN_CONTEXT_TYPE _rtnContextQgmSort::getType () const
   {
      return RTN_CONTEXT_QGMSORT ;
   }

   INT32 _rtnContextQgmSort::open( _qgmPlan *qp )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != qp, "impossible" ) ;
      if ( _isOpened )
      {
         rc = SDB_DMS_CONTEXT_IS_OPEN ;
         goto error ;
      }

      _qp = qp ;
      _isOpened = TRUE ;
      _hitEnd = FALSE ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextQgmSort::_prepareData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != _qp, "impossible" ) ;
      qgmFetchOut next ;
      INT32 index = 0 ;
      monAppCB *pMonAppCB = cb ? cb->getMonAppCB() : NULL ;
      for ( ; index < RTN_CONTEXT_GETNUM_ONCE ; ++index )
      {
         try
         {
            rc = _qp->fetchNext( next ) ;
         }
         catch( std::exception &e )
         {
            PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         if ( SDB_DMS_EOC == rc )
         {
            _hitEnd = TRUE ;
            break ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Qgm fetch failed, rc: %d", rc ) ;
            goto error ;
         }

         rc = append( next.obj ) ;
         PD_RC_CHECK( rc, PDERROR, "Append obj[%s] failed, rc: %d",
                      next.obj.toString().c_str(), rc ) ;
         DMS_MON_OP_COUNT_INC( pMonAppCB, MON_SELECT, 1 ) ;
         if ( buffEndOffset() + DMS_RECORD_MAX_SZ > RTN_RESULTBUFFER_SIZE_MAX )
         {
            break ;
         }
      }

      if ( !isEmpty() )
      {
         rc = SDB_OK ;
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

   RTN_CTX_AUTO_REGISTER(_rtnContextDelCS, RTN_CONTEXT_DELCS, "DELCS")

   _rtnContextDelCS::_rtnContextDelCS( SINT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _status = DELCSPHASE_0;
      _pDmsCB = pmdGetKRCB()->getDMSCB() ;
      _pDpsCB = pmdGetKRCB()->getDPSCB() ;
      _pCatAgent = pmdGetKRCB()->getClsCB ()->getCatAgent () ;
      _pTransCB = pmdGetKRCB()->getTransCB();
      _gotDmsCBWrite = FALSE;
      _gotLogSize = 0;
      _logicCSID = DMS_INVALID_LOGICCSID;
      ossMemset( _name, 0, DMS_COLLECTION_SPACE_NAME_SZ + 1 );
   }

   _rtnContextDelCS::~_rtnContextDelCS()
   {
      pmdEDUMgr *eduMgr = pmdGetKRCB()->getEDUMgr() ;
      pmdEDUCB *cb = eduMgr->getEDUByID( eduID() ) ;
      if ( DELCSPHASE_1 == _status )
      {
         INT32 rcTmp = SDB_OK;
         rcTmp = rtnDropCollectionSpaceP1Cancel( _name, cb, _pDmsCB,
                                                 getDPSCB() );
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "failed to cancel drop cs(name:%s, rc=%d)",
                    _name, rcTmp );
         }
         _status = DELCSPHASE_0;
      }
      _clean( cb );
   }

   std::string _rtnContextDelCS::name() const
   {
      return "DELCS" ;
   }

   RTN_CONTEXT_TYPE _rtnContextDelCS::getType () const
   {
      return RTN_CONTEXT_DELCS;
   }

   INT32 _rtnContextDelCS::open( const CHAR *pCollectionName,
                                 _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      dpsMergeInfo info ;
      dpsLogRecord &record = info.getMergeBlock().record();

      SDB_ASSERT( pCollectionName, "pCollectionName can't be null!" );
      PD_CHECK( pCollectionName, SDB_INVALIDARG, error, PDERROR,
                "pCollectionName is null!" );
      rc = dmsCheckCSName( pCollectionName );
      PD_RC_CHECK( rc, PDERROR, "Invalid cs name(name:%s)",
                   pCollectionName );

      ossStrncpy( _name, pCollectionName, DMS_COLLECTION_SPACE_NAME_SZ ) ;

      /// test collection space exist
      rc = rtnTestCollectionSpaceCommand( pCollectionName, _pDmsCB ) ;
      if ( SDB_DMS_CS_NOTEXIST == rc )
      {
         /// ignore collection space not exist
         PD_LOG( PDINFO, "Ignored error[%d] when drop collection space[%s]",
                 rc, pCollectionName ) ;
         rc = SDB_OK ;
         _isOpened = TRUE ;
         goto done ;
      }

      if ( NULL != getDPSCB() )
      {
         // reserved log-size
         UINT32 logRecSize = 0;
         rc = dpsCSDel2Record( pCollectionName, record ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to build record:%d",rc ) ;

         rc = getDPSCB()->checkSyncControl( record.alignedLen(), cb ) ;
         PD_RC_CHECK( rc, PDERROR, "Check sync control failed, rc: %d", rc ) ;

         logRecSize = record.alignedLen() ;
         rc = _pTransCB->reservedLogSpace( logRecSize, cb );
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to reserved log space(length=%u)",
                      logRecSize );
         _gotLogSize = logRecSize ;
      }

      rc = _pDmsCB->writable ( cb ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "dms is not writable, rc = %d", rc ) ;
      _gotDmsCBWrite = TRUE;

      rc = _tryLock( pCollectionName, cb );
      PD_RC_CHECK( rc, PDERROR, "Failed to lock, rc: %d", rc ) ;

      rc = rtnDropCollectionSpaceP1( _name, cb, _pDmsCB, getDPSCB() );
      PD_RC_CHECK( rc, PDERROR, "Failed to drop cs in phase1, rc: %d", rc );
      _status = DELCSPHASE_1 ;
      _isOpened = TRUE ;

   done:
      return rc;
   error:
      _clean( cb );
      goto done;
   }

   void _rtnContextDelCS::_toString( stringstream &ss )
   {
      ss << ",Name:" << _name
         << ",GotLogSize:" << _gotLogSize
         << ",GotDMSWrite:" << _gotDmsCBWrite
         << ",LogicalID:" << _logicCSID
         << ",Step:" << _status ;
   }

   INT32 _rtnContextDelCS::getMore( INT32 maxNumToReturn,
                                    rtnContextBuf &buffObj,
                                    _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      clsCB *pClsCB = sdbGetClsCB() ;
      shardCB *pShdMgr = pClsCB->getShardCB() ;
      clsTaskMgr *pTaskMgr = pmdGetKRCB()->getClsCB()->getTaskMgr() ;
      vector< string > subCLs ;
      vector< string >::iterator it ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE;
         goto error ;
      }

      _pCatAgent->lock_w() ;
      _pCatAgent->clearBySpaceName( _name, &subCLs ) ;
      _pCatAgent->release_w() ;

      it = subCLs.begin() ;
      while( it != subCLs.end() )
      {
         if ( SDB_OK != pShdMgr->syncUpdateCatalog( (*it).c_str() ) )
         {
            _pCatAgent->lock_w() ;
            _pCatAgent->clear( (*it).c_str() ) ;
            _pCatAgent->release_w() ;
         }
         pClsCB->invalidateCata( (*it).c_str() ) ;
         ++it ;
      }
      pClsCB->invalidateCata( _name ) ;

      /// already drop phrase1
      if ( DELCSPHASE_1 == _status )
      {
         rc = rtnDropCollectionSpaceP2( _name, cb, _pDmsCB, getDPSCB() ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to drop cs in phase2(%d)", rc ) ;
         _status = DELCSPHASE_0 ;
         _clean( cb ) ;
      }

      /// close context
      _isOpened = FALSE ;
      rc = SDB_DMS_EOC ;

      /// wait all collection space's task finished
      cb->writingDB( FALSE ) ;
      while( pTaskMgr->taskCountByCS( _name ) > 0 )
      {
         pTaskMgr->waitTaskEvent() ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextDelCS::_tryLock( const CHAR *pCollectionName,
                                     _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      if ( getDPSCB() )
      {
         dmsStorageUnitID suID = DMS_INVALID_CS;
         UINT32 logicCSID = DMS_INVALID_LOGICCSID;
         dmsStorageUnit *su = NULL;
         _releaseLock( cb );
         UINT32 length = ossStrlen ( pCollectionName );
         PD_CHECK( (length > 0 && length <= DMS_SU_NAME_SZ), SDB_INVALIDARG,
                   error, PDERROR, "Invalid length of collectionspace name:%s",
                   pCollectionName );

         rc = _pDmsCB->nameToSUAndLock( pCollectionName, suID, &su );
         PD_RC_CHECK(rc, PDERROR, "lock collection space(%s) failed(rc=%d)",
                     pCollectionName, rc );
         logicCSID = su->LogicalCSID();
         _pDmsCB->suUnlock ( suID ) ;
         rc = _pTransCB->transLockTryX( cb, logicCSID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Get transaction-lock of CS(%s) failed(rc=%d)",
                      pCollectionName, rc ) ;
         _logicCSID = logicCSID ;
      }
   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextDelCS::_releaseLock( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      if ( cb && getDPSCB() && ( _logicCSID != DMS_INVALID_LOGICCSID ) )
      {
         _pTransCB->transLockRelease( cb, _logicCSID );
         _logicCSID = DMS_INVALID_LOGICCSID;
      }
      return rc;
   }

   void _rtnContextDelCS::_clean( _pmdEDUCB *cb )
   {
      INT32 rcTmp = SDB_OK;
      rcTmp = _releaseLock( cb );
      if ( rcTmp )
      {
         PD_LOG( PDERROR, "releas lock failed, rc: %d", rcTmp );
      }
      if ( _gotDmsCBWrite )
      {
         _pDmsCB->writeDown ( cb ) ;
         _gotDmsCBWrite = FALSE;
      }
      if ( _gotLogSize > 0 )
      {
         _pTransCB->releaseLogSpace( _gotLogSize, cb );
         _gotLogSize = 0;
      }
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextDelCL, RTN_CONTEXT_DELCL, "DELCL")

   _rtnContextDelCL::_rtnContextDelCL( SINT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _pDmsCB        = pmdGetKRCB()->getDMSCB() ;
      _pDpsCB        = pmdGetKRCB()->getDPSCB() ;
      _pCatAgent     = pmdGetKRCB()->getClsCB()->getCatAgent () ;
      _pTransCB      = pmdGetKRCB()->getTransCB();
      _gotDmsCBWrite = FALSE ;
      _hasLock       = FALSE ;
      _hasDropped    = FALSE ;
      _mbContext     = NULL ;
      _su            = NULL ;
      _clShortName   = NULL ;
      ossMemset( _collectionName, 0, sizeof( _collectionName ) ) ;
   }

   _rtnContextDelCL::~_rtnContextDelCL()
   {
      pmdEDUMgr *eduMgr    = pmdGetKRCB()->getEDUMgr() ;
      pmdEDUCB *cb         = eduMgr->getEDUByID( eduID() ) ;
      _clean( cb ) ;
   }

   INT32 _rtnContextDelCL::_tryLock( const CHAR *pCollectionName,
                                     _pmdEDUCB *cb )
   {
      INT32 rc                = SDB_OK ;
      dmsStorageUnitID suID   = DMS_INVALID_CS ;

      ossStrncpy( _collectionName, pCollectionName,
                  DMS_COLLECTION_FULL_NAME_SZ ) ;

      rc = rtnResolveCollectionNameAndLock ( _collectionName, _pDmsCB,
                                             &_su, &_clShortName,
                                             suID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to resolve collection name"
                   "(collection:%s, rc: %d)", _collectionName, rc ) ;

      // lock collection
      if ( getDPSCB() && _pTransCB->isTransOn() )
      {
         rc = _su->data()->getMBContext( &_mbContext, _clShortName,
                                         EXCLUSIVE ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pCollectionName, rc ) ;

         rc = _pTransCB->transLockTryX( cb, _su->LogicalCSID(),
                                        _mbContext->mbID() ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Get transaction-lock of collection(%s) failed(rc=%d)",
                      pCollectionName, rc ) ;
         _hasLock = TRUE ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextDelCL::_releaseLock( _pmdEDUCB *cb )
   {
      if ( cb && _hasLock )
      {
         _pTransCB->transLockRelease( cb, _su->LogicalCSID(),
                                      _mbContext->mbID() ) ;
         _hasLock = FALSE ;
      }
      return SDB_OK ;
   }

   std::string _rtnContextDelCL::name() const
   {
      return "DELCL" ;
   }

   RTN_CONTEXT_TYPE _rtnContextDelCL::getType () const
   {
      return RTN_CONTEXT_DELCL ;
   }

   INT32 _rtnContextDelCL::open( const CHAR *pCollectionName,
                                 _pmdEDUCB *cb, INT16 w )
   {
      INT32 rc = SDB_OK ;

      /// set w info
      _w = w ;

      SDB_ASSERT( pCollectionName, "pCollectionName can't be null!" );
      PD_CHECK( pCollectionName, SDB_INVALIDARG, error, PDERROR,
               "pCollectionName is null!" );
      rc = dmsCheckFullCLName( pCollectionName );
      PD_RC_CHECK( rc, PDERROR, "Invalid collection name(name:%s)",
                   pCollectionName ) ;

      rc = _pDmsCB->writable ( cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Database is not writable, rc = %d", rc ) ;
      _gotDmsCBWrite = TRUE ;

      rc = _tryLock( pCollectionName, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to lock(rc=%d)", rc ) ;
      _isOpened = TRUE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextDelCL::getMore( INT32 maxNumToReturn,
                                    rtnContextBuf &buffObj,
                                    _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      clsTaskMgr *pTaskMgr = pmdGetKRCB()->getClsCB()->getTaskMgr() ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE;
         goto error ;
      }
      _pCatAgent->lock_w () ;
      _pCatAgent->clear ( _collectionName ) ;
      _pCatAgent->release_w () ;
      pmdGetKRCB()->getClsCB()->invalidateCata( _collectionName ) ;

      // drop collection
      rc = _su->data()->dropCollection ( _clShortName, cb, getDPSCB(),
                                         TRUE, _mbContext ) ;
      if ( rc )
      {
         // Ignore SDB_DMS_NOTEXIST, which means the CL mignt be deleted already
         if ( SDB_DMS_NOTEXIST == rc )
         {
            PD_LOG ( PDWARNING, "Collection %s doesn't exist, ignored in drop "
                     "collection, rc: %d", _collectionName, rc ) ;
            rc = SDB_OK ;
         }
         else
         {
            PD_LOG ( PDERROR, "Failed to drop collection %s, rc: %d",
                     _collectionName, rc ) ;
            goto error ;
         }
      }

      _su->getAPM()->invalidatePlans ( _clShortName ) ;
      _hasDropped = TRUE ;

      _clean( cb ) ;
      _isOpened = FALSE ;
      rc = SDB_DMS_EOC ;

      /// wait all collection's task finished
      cb->writingDB( FALSE ) ;

      {
         UINT32 waitCnt = 0 ;
         while( pTaskMgr->taskCountByCL( _collectionName ) > 0 )
         {
            pTaskMgr->waitTaskEvent() ;
            waitCnt ++ ;

            // Log the task list after waiting over 10 minutes
            if ( waitCnt > 600 )
            {
               PD_LOG( PDDEBUG, "DropCL [%s] is waiting for split tasks:\n%s",
                       _collectionName,
                       pTaskMgr->dumpTasks( CLS_TASK_UNKNOW ).c_str() ) ;
               waitCnt = 0 ;
            }

         }
      }

   done:
      return rc;
   error:
      goto done;
   }

   void _rtnContextDelCL::_toString( stringstream &ss )
   {
      ss << ",Name:" << _collectionName
         << ",GotDMSWrite:" << _gotDmsCBWrite
         << ",HasLock:" << _hasLock
         << ",HasDropped:" << _hasDropped ;
   }

   void _rtnContextDelCL::_clean( _pmdEDUCB *cb )
   {
      INT32 rcTmp = SDB_OK;
      rcTmp = _releaseLock( cb ) ;
      if ( rcTmp )
      {
         PD_LOG( PDERROR, "release lock failed, rc: %d", rcTmp ) ;
      }
      if ( _su && _mbContext )
      {
         _su->data()->releaseMBContext( _mbContext ) ;
      }
      // unlock su
      if ( _pDmsCB && _su )
      {
         string csname = _su->CSName() ;
         _pDmsCB->suUnlock ( _su->CSID() ) ;
         _su = NULL ;

         if ( _hasDropped )
         {
            // ignore errors
            _pDmsCB->dropEmptyCollectionSpace( csname.c_str(),
                                               cb, getDPSCB() ) ;
         }
      }
      if ( _gotDmsCBWrite )
      {
         _pDmsCB->writeDown( cb ) ;
         _gotDmsCBWrite = FALSE ;
      }
      _isOpened = FALSE ;
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextDelMainCL, RTN_CONTEXT_DELMAINCL, "DELMAINCL")

   _rtnContextDelMainCL::_rtnContextDelMainCL( SINT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _pCatAgent     = pmdGetKRCB()->getClsCB()->getCatAgent() ;
      _pRtncb        = pmdGetKRCB()->getRTNCB();
      _version       = -1 ;
      ossMemset( _name, 0, DMS_COLLECTION_FULL_NAME_SZ + 1 );
   }

   _rtnContextDelMainCL::~_rtnContextDelMainCL()
   {
      pmdEDUMgr *eduMgr = pmdGetKRCB()->getEDUMgr() ;
      pmdEDUCB *cb = eduMgr->getEDUByID( eduID() ) ;
      _clean( cb );
   }

   void _rtnContextDelMainCL::_clean( _pmdEDUCB *cb )
   {
      SUBCL_CONTEXT_LIST::iterator iter = _subContextList.begin();
      while( iter != _subContextList.end() )
      {
         if ( iter->second != -1 )
         {
            _pRtncb->contextDelete( iter->second, cb );
         }
         iter = _subContextList.erase( iter );
      }
   }

   std::string _rtnContextDelMainCL::name() const
   {
      return "DELMAINCL" ;
   }

   RTN_CONTEXT_TYPE _rtnContextDelMainCL::getType () const
   {
      return RTN_CONTEXT_DELMAINCL;
   }

   INT32 _rtnContextDelMainCL::open( const CHAR *pCollectionName,
                                     vector< string > &subCLList,
                                     INT32 version,
                                     _pmdEDUCB *cb,
                                     INT16 w )
   {
      INT32 rc = SDB_OK ;
      vector< string >::iterator iter ;
      rtnContextDelCL *delContext   = NULL ;
      SINT64 contextID              = -1 ;

      _version                      = version ;

      SDB_ASSERT( pCollectionName, "pCollectionName can't be null!" ) ;
      PD_CHECK( pCollectionName, SDB_INVALIDARG, error, PDERROR,
                "pCollectionName is null!" ) ;

      rc = dmsCheckFullCLName( pCollectionName ) ;
      PD_RC_CHECK( rc, PDERROR, "Invalid collection name[%s])",
                   pCollectionName ) ;

      /// open sub collection context
      iter = subCLList.begin() ;
      while( iter != subCLList.end() )
      {
         rc = _pRtncb->contextNew( RTN_CONTEXT_DELCL,
                                   (rtnContext **)&delContext,
                                   contextID, cb ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create sub-context of sub-"
                      "collection[%s] in drop collection[%s], rc: %d",
                      (*iter).c_str(), pCollectionName, rc ) ;

         rc = delContext->open( (*iter).c_str(), cb, w ) ;
         if ( rc != SDB_OK )
         {
            _pRtncb->contextDelete( contextID, cb ) ;
            if ( SDB_DMS_NOTEXIST == rc )
            {
               ++iter;
               continue;
            }
            PD_LOG( PDERROR, "Failed to open sub-context of sub-"
                    "collection[%s] in drop collection[%s], rc: %d",
                    (*iter).c_str(), pCollectionName, rc ) ;
            goto error;
         }
         _subContextList[ *iter ] = contextID ;
         ++iter ;
      }

      ossStrcpy( _name, pCollectionName ) ;
      _isOpened = TRUE;
   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextDelMainCL::getMore( INT32 maxNumToReturn,
                                        rtnContextBuf &buffObj,
                                        _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      INT32 curVer = -1 ;
      _clsCatalogSet *pCataSet = NULL ;
      SUBCL_CONTEXT_LIST::iterator iterCtx ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE;
         goto error ;
      }

      /// get last catalog info
      _pCatAgent->lock_r() ;
      pCataSet = _pCatAgent->collectionSet( _name ) ;
      if ( pCataSet )
      {
         curVer = pCataSet->getVersion() ;
      }
      _pCatAgent->release_r() ;

      if ( -1 != curVer && curVer != _version )
      {
         rc = SDB_CLS_COORD_NODE_CAT_VER_OLD ;
         goto error ;
      }

      /// drop sub collections
      iterCtx = _subContextList.begin() ;
      while( iterCtx != _subContextList.end() )
      {
         rtnContextBuf buffObj;
         rc = rtnGetMore( iterCtx->second, -1, buffObj, cb, _pRtncb ) ;
         PD_CHECK( SDB_DMS_EOC == rc || SDB_DMS_NOTEXIST == rc,
                   rc, error, PDERROR,
                   "Failed to del sub-collection, rc: %d",
                   rc ) ;
         rc = SDB_OK ;
         iterCtx = _subContextList.erase( iterCtx ) ;
      }

      /// clear main collection's catalog info
      _pCatAgent->lock_w () ;
      _pCatAgent->clear ( _name ) ;
      _pCatAgent->release_w () ;
      pmdGetKRCB()->getClsCB()->invalidateCata( _name ) ;
      _isOpened = FALSE ;
      rc = SDB_DMS_EOC ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextDelMainCL::_toString( stringstream &ss )
   {
      ss << ",Name:" << _name
         << ",Version:" << _version ;
   }
}

