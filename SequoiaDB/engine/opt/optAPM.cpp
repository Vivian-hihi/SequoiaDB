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

   Source File Name = optAPM.cpp

   Descriptive Name = Optimizer Access Plan Manager

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains Optimizer Access Plan
   Manager, which is used to pool access plans that previously generated.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft
          01/07/2017  HGM Move from rtnAPM.cpp

   Last Changed =

*******************************************************************************/

#include "optAPM.hpp"
#include "rtn.hpp"
#include "dmsStorageUnit.hpp"
#include "pdTrace.hpp"
#include "optTrace.hpp"
#include "pmd.hpp"

namespace engine
{

   /*
      _optAccessPlanCache implement
    */
   _optAccessPlanCache::_optAccessPlanCache ()
   : _utilHashTable< optAccessPlanKey, optAccessPlan >()
   {
      _pMonitor = NULL ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_INIT, "_optAccessPlanCache::initialize" )
   BOOLEAN _optAccessPlanCache::initialize ( UINT16 bucketNum,
                                             optCachedPlanMonitor *pMonitor )
   {
      BOOLEAN result = FALSE ;

      PD_TRACE_ENTRY( SDB_OPTAPCACHES_INIT ) ;

      SDB_ASSERT( NULL != pMonitor, "pMonotir is invalid" ) ;

      if ( utilHashTable::initialize( bucketNum ) )
      {
         _pMonitor = pMonitor ;
         result = TRUE ;
      }

      PD_TRACE_EXIT( SDB_OPTAPCACHES_INIT ) ;

      return result ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_CLEAR, "_optAccessPlanCache::clear" )
   void _optAccessPlanCache::clear ()
   {
      PD_TRACE_ENTRY( SDB_OPTAPCACHES_CLEAR ) ;

      utilHashTable::clear() ;
      _pMonitor = NULL ;

      PD_TRACE_EXIT( SDB_OPTAPCACHES_CLEAR ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_ADDPLAN, "_optAccessPlanCache::addPlan" )
   BOOLEAN _optAccessPlanCache::addPlan ( optAccessPlan *pPlan )
   {
      BOOLEAN result = FALSE ;

      PD_TRACE_ENTRY( SDB_OPTAPCACHES_ADDPLAN ) ;

      if ( addItem( pPlan ) )
      {
         pPlan->incRefCount() ;
         result = TRUE ;
      }

      PD_TRACE_EXIT( SDB_OPTAPCACHES_ADDPLAN ) ;

      return result ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_RMPLAN, "_optAccessPlanCache::removeCachedPlan" )
   void _optAccessPlanCache::removeCachedPlan ( optAccessPlan *pPlan )
   {
      PD_TRACE_ENTRY( SDB_OPTAPCACHES_RMPLAN ) ;

      INT32 activityID = pPlan->resetActivityID() ;

      // We need to test the activity ID to check if
      // someone else is also deleting this plan
      if ( OPT_INVALID_ACT_ID != activityID )
      {
         _pMonitor->resetActivity( activityID ) ;

         // Increase the reference count before we delete it
         pPlan->incRefCount() ;
         if ( removeItem( pPlan ) )
         {
            _pMonitor->decCachedPlanCount( 1 ) ;
         }
         pPlan->release() ;
      }

      PD_TRACE_EXIT( SDB_OPTAPCACHES_RMPLAN ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_INVALIDSUPLANS, "_optAccessPlanCache::invalidateSUPlans" )
   void _optAccessPlanCache::invalidateSUPlans ( dmsCachedPlanMgr *pCachedPlanMgr,
                                                  UINT32 suLID )
   {
      PD_TRACE_ENTRY( SDB_OPTAPCACHES_INVALIDSUPLANS ) ;

      SDB_ASSERT( pCachedPlanMgr, "pCachedPlanMgr is invalid" ) ;

      UINT32 deleteCount = 0 ;

      for ( UINT32 bucketID = 0 ;
            bucketID < pCachedPlanMgr->getBucketNum() ;
            bucketID ++ )
      {
         if ( pCachedPlanMgr->testCacheBitMap( bucketID ) )
         {
            // Lock the clear lock shared, parallel removing for different
            // collections or collection spaces is allowed
            ossScopedRWLock scopedLock( _pMonitor->getClearLock(), SHARED ) ;
            utilHashTableBucket *pBucket = getBucket( bucketID, EXCLUSIVE ) ;

            if ( NULL != pBucket )
            {
               optAccessPlan *pPlan = pBucket->getHead() ;
               while ( NULL != pPlan )
               {
                  optAccessPlan *pNextPlan = (optAccessPlan *)pPlan->getNext() ;
                  if ( pPlan->getSULID() == suLID )
                  {
                     // Locked bucket already, safe to remove from bucket
                     // But we need to test the activity ID to check if
                     // someone else is also deleting this plan
                     INT32 activityID = pPlan->resetActivityID() ;
                     if ( OPT_INVALID_ACT_ID != activityID )
                     {
                        _pMonitor->resetActivity( activityID ) ;

                        // Increase the reference count before we delete it
                        pPlan->incRefCount() ;
                        pBucket->removeItem( pPlan ) ;
                        pPlan->release() ;

                        deleteCount ++ ;
                     }
                  }
                  pPlan = pNextPlan ;
               }
               releaseBucket( bucketID, EXCLUSIVE ) ;
            }
            else
            {
               SDB_ASSERT( pBucket, "pBucket is invalid" ) ;
            }
         }
      }

      _pMonitor->decCachedPlanCount( deleteCount ) ;

      PD_TRACE_EXIT( SDB_OPTAPCACHES_INVALIDSUPLANS ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_INVALIDCLPLANS, "_optAccessPlanCache::invalidateCLPlans" )
   void _optAccessPlanCache::invalidateCLPlans ( dmsCachedPlanMgr *pCachedPlanMgr,
                                                  UINT32 suLID, UINT32 clLID )
   {
      PD_TRACE_ENTRY( SDB_OPTAPCACHES_INVALIDCLPLANS ) ;

      SDB_ASSERT( pCachedPlanMgr, "pCachedPlanMgr is invalid" ) ;

      UINT32 deleteCount = 0 ;

      for ( UINT32 bucketID = 0 ;
            bucketID < pCachedPlanMgr->getBucketNum() ;
            bucketID ++ )
      {
         if ( pCachedPlanMgr->testCacheBitMap( bucketID ) )
         {
            // Lock the clear lock shared, parallel removing for different
            // collections or collection spaces is allowed
            ossScopedRWLock scopedLock( _pMonitor->getClearLock(), SHARED ) ;
            utilHashTableBucket *pBucket = getBucket( bucketID, EXCLUSIVE ) ;
            BOOLEAN clearBit = TRUE ;

            if ( NULL != pBucket )
            {
               optAccessPlan *pPlan = pBucket->getHead() ;
               while ( pPlan )
               {
                  optAccessPlan *pNextPlan = (optAccessPlan *)pPlan->getNext() ;
                  if ( pPlan->getSULID() == suLID && pPlan->getCLLID() == clLID )
                  {
                     // Locked bucket already, safe to remove from bucket
                     // But we need to test the activity ID to check if
                     // someone else is also deleting this plan
                     INT32 activityID = pPlan->resetActivityID() ;
                     if ( OPT_INVALID_ACT_ID != activityID )
                     {
                        _pMonitor->resetActivity( activityID ) ;

                        // Increase the reference count before we delete it
                        pPlan->incRefCount() ;
                        pBucket->removeItem( pPlan ) ;
                        pPlan->release() ;

                        deleteCount ++ ;
                     }
                     else if ( clearBit )
                     {
                        clearBit = FALSE ;
                     }
                  }
                  else if ( clearBit && pPlan->getSULID() == suLID )
                  {
                     clearBit = FALSE ;
                  }
                  pPlan = pNextPlan ;
               }

               if ( clearBit )
               {
                  // Bucket contains no plans of this SU any more
                  // clear the bit
                  pCachedPlanMgr->clearCacheBit( bucketID ) ;
               }

               releaseBucket( bucketID, EXCLUSIVE ) ;
            }
            else
            {
               SDB_ASSERT( pBucket, "pBucket is invalid" ) ;
            }
         }
      }

      _pMonitor->decCachedPlanCount( deleteCount ) ;

      PD_TRACE_EXIT( SDB_OPTAPCACHES_INVALIDCLPLANS ) ;
   }

   UINT32 _optAccessPlanCache::getCachedPlanCount () const
   {
      return _pMonitor->getCachedPlanCount() ;
   }

   void _optAccessPlanCache::afterGetItem ( UINT32 bucketID,
                                            optAccessPlan *pPlan )
   {
      SDB_ASSERT( pPlan, "pPlan is invalid" ) ;

      pPlan->incRefCount() ;
      _pMonitor->setCachedPlanActivity( pPlan ) ;
   }

   /*
      _optCachedPlanMonitor implement
    */
   _optCachedPlanMonitor::_optCachedPlanMonitor ()
   : _freeIndexBegin( 0 ),
     _freeIndexEnd( 0 ),
     _clearThread( 0 ),
     _allocateThread( 0 ),
     _cachedPlanCount( 0 ),
     _accessTimestamp( 0 ),
     _accessCount( 0 )
   {
      _pPlanCache = NULL ;
      _pFreeActivityIDs = NULL ;
      _clockIndex = 0 ;
      _pActivities = NULL ;
      _activityNum = 0 ;
      _highWaterMark = 0 ;
      _lowWaterMark = 0 ;
      _lastClearTimestamp = 0 ;
   }

   _optCachedPlanMonitor::~_optCachedPlanMonitor ()
   {
      clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON_INIT, "_optCachedPlanMonitor::initialize" )
   BOOLEAN _optCachedPlanMonitor::initialize ( optAccessPlanCache *pPlanCache )
   {
      BOOLEAN result = FALSE ;

      PD_TRACE_ENTRY( SDB_OPTCPMON_INIT ) ;

      UINT32 activityNum = 0 ;

      if ( isInitialized() )
      {
         result = TRUE ;
         goto done ;
      }

      if ( NULL != pPlanCache && !pPlanCache->isInitialized() )
      {
         goto error ;
      }

      activityNum = pPlanCache->getBucketNum() *
                    OPT_PLAN_CACHE_AVG_BUCKET_SIZE ;
      if ( activityNum == 0 )
      {
         goto error ;
      }

      // Allocate activity buffer
      _pFreeActivityIDs = new( std::nothrow ) UINT32[ activityNum ] ;
      if ( NULL == _pFreeActivityIDs )
      {
         goto error ;
      }

      _pActivities = new( std::nothrow ) optCachedPlanActivity[ activityNum ] ;
      if ( NULL == _pActivities )
      {
         goto error ;
      }

      _activityNum = activityNum ;
      _highWaterMark = activityNum * OPT_PLAN_CACHE_ACT_HIGH_PERC ;
      _lowWaterMark = activityNum * OPT_PLAN_CACHE_ACT_LOW_PERC ;
      _pPlanCache = pPlanCache ;

      for ( UINT32 i = 0 ; i < activityNum ; i++ )
      {
         _pFreeActivityIDs[ i ] = i ;
      }

      _freeIndexBegin.init( 0 ) ;
      _freeIndexEnd.init( _activityNum ) ;
      _clearThread.init( 0 ) ;
      _allocateThread.init( 0 ) ;
      _clockIndex = 0 ;

      _cachedPlanCount.init( 0 ) ;
      _accessTimestamp.init( 0 ) ;
      _accessCount.init( 0 ) ;

      _lastClearTimestamp = 0 ;

      result = TRUE ;

   done :
      PD_TRACE_EXIT( SDB_OPTCPMON_INIT ) ;
      return result ;

   error :
      clear() ;
      result = FALSE ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON_CLEAR, "_optCachedPlanMonitor::clear" )
   void _optCachedPlanMonitor::clear ()
   {
      PD_TRACE_ENTRY( SDB_OPTCPMON_CLEAR ) ;

      if ( NULL != _pFreeActivityIDs )
      {
         delete [] _pFreeActivityIDs ;
      }
      if ( NULL != _pActivities )
      {
         delete [] _pActivities ;
      }
      _pFreeActivityIDs = NULL ;
      _pActivities = NULL ;
      _activityNum = 0 ;
      _highWaterMark = 0 ;
      _lowWaterMark = 0 ;
      _freeIndexBegin.init( 0 ) ;
      _freeIndexEnd.init( 0 ) ;
      _clearThread.init( 0 ) ;
      _allocateThread.init( 0 ) ;
      _clockIndex = 0 ;
      _pPlanCache = NULL ;

      _cachedPlanCount.init( 0 ) ;
      _accessTimestamp.init( 0 ) ;
      _accessCount.init( 0 ) ;

      _lastClearTimestamp = 0 ;

      PD_TRACE_EXIT( SDB_OPTCPMON_CLEAR ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON_SETACT, "_optCachedPlanMonitor::setActivity" )
   BOOLEAN _optCachedPlanMonitor::setActivity ( optAccessPlan *pPlan )
   {
      BOOLEAN result = FALSE ;

      PD_TRACE_ENTRY( SDB_OPTCPMON_SETACT ) ;

      INT32 activityID = OPT_INVALID_ACT_ID ;
      UINT32 retryTimes = 0 ;

      while ( retryTimes < OPT_PLAN_CACHE_MAX_RETRY )
      {
         if ( _freeIndexEnd.peek() >= INT32_MAX )
         {
            _resetFreeIndexes() ;
            retryTimes ++ ;
         }
         else if ( _cachedPlanCount.peek() > _highWaterMark )
         {
            _clearCachedPlans() ;
            retryTimes ++ ;
         }
         else
         {
            activityID = _allocateActivity( pPlan ) ;
            break ;
         }
      }

      if ( OPT_INVALID_ACT_ID == activityID )
      {
         goto error ;
      }

      result = TRUE ;

   done :
      PD_TRACE_EXIT( SDB_OPTCPMON_SETACT ) ;
      return result ;

   error :
      result = FALSE ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON__RESETFREEIDX, "_optCachedPlanMonitor::_resetFreeIndexes" )
   void _optCachedPlanMonitor::_resetFreeIndexes ()
   {
      PD_TRACE_ENTRY( SDB_OPTCPMON__RESETFREEIDX ) ;

      if ( _clearThread.compareAndSwap( 0, 1 ) )
      {
         // Only one thread could enter this branch

         // Lock the removing lock exclusively, so other threads could not
         // remove plans during this procedure
         ossScopedRWLock scopedLock( &_clearLock, EXCLUSIVE ) ;

         UINT32 newFreeIndexBegin = 0, newFreeIndexEnd = 0 ;

         // Check again after lock
         if ( _freeIndexEnd.peek() < INT32_MAX )
         {
            _clearThread.init( 0 ) ;
            goto done ;
         }

         newFreeIndexBegin = _freeIndexBegin.peek() % _activityNum ;
         newFreeIndexEnd = _freeIndexEnd.peek() % _activityNum ;
         _freeIndexBegin.init( newFreeIndexBegin ) ;
         _freeIndexEnd.init( newFreeIndexEnd ) ;

         _clearThread.init( 0 ) ;
      }

   done :
      PD_TRACE_EXIT( SDB_OPTCPMON__RESETFREEIDX ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON__CLEARCP, "_optCachedPlanMonitor::_clearCachedPlans" )
   void _optCachedPlanMonitor::_clearCachedPlans ()
   {
      PD_TRACE_ENTRY( SDB_OPTCPMON__CLEARCP ) ;

      if ( _clearThread.compareAndSwap( 0, 1 ) )
      {
         // Only one thread could enter this branch

         // Lock the clear lock exclusively, so other threads could not
         // remove plans during this procedure
         ossScopedRWLock scopedLock( &_clearLock, EXCLUSIVE ) ;

         UINT32 needRemoveCount = 0 ;
         UINT64 timestampMinThreshold = 0 ;
         UINT64 timestampMaxThreshold = 0 ;
         UINT64 timestampFactor = 0 ;
         UINT64 currentTimestamp = 0 ;
         UINT32 accessCountThreshold = 0 ;
         UINT32 lastClockIndex = _clockIndex ;
         UINT32 loopCount = 0 ;

         // Check again after lock
         UINT32 cachedPlanCount = _cachedPlanCount.peek() ;
         if ( cachedPlanCount < _highWaterMark )
         {
            _clearThread.init( 0 ) ;
            goto done ;
         }

         // Clear one cached plan in below cases :
         // 1. last access timestamp < 1/3 * ( current timestamp -
         //    last clear timestamp ) + last clear timestamp
         // 2. access count < 1/3 * current average access counts
         // 3. TODO: by result scores
         currentTimestamp = _accessTimestamp.peek() ;
         timestampFactor = ( currentTimestamp - _lastClearTimestamp ) *
                           OPT_PLAN_CACHE_CLEAR_THRESHOLD ;
         timestampMinThreshold = _lastClearTimestamp + timestampFactor ;
         timestampMaxThreshold = currentTimestamp - timestampFactor ;
         accessCountThreshold = _accessCount.peek() *
                                OPT_PLAN_CACHE_CLEAR_THRESHOLD ;

         accessCountThreshold = accessCountThreshold / cachedPlanCount + 1 ;
         needRemoveCount = cachedPlanCount - _lowWaterMark ;

         // End searching conditions:
         // 1. removed enough plans
         // 2. loop after 4 times
         // Note, at most 3 loops wiil clear most plans
         while ( needRemoveCount > 0 && loopCount < 4 )
         {
            optCachedPlanActivity &activity = _pActivities[ _clockIndex ] ;
            UINT64 accessTime = activity.getAccessTime() ;
            UINT32 accessCount = activity.getAccessCount() ;

            if ( activity.isEmpty() )
            {
               _clockIndex = ( _clockIndex + 1 ) % _activityNum ;
               // Searched one loop
               if ( _clockIndex == lastClockIndex )
               {
                  loopCount ++ ;
               }
               continue ;
            }

            if ( 0 == accessCount ||
                 ( accessCount <= accessCountThreshold &&
                   accessTime <= timestampMaxThreshold ) ||
                 accessTime <= timestampMinThreshold )
            {
               _pPlanCache->removeCachedPlan( activity.getPlan() ) ;
               needRemoveCount -- ;
            }
            else
            {
               // Decrease the access count by 1/3, so the plan could be
               // cleared finally after at most 3 rounds
               activity.decAccessCount( accessCountThreshold ) ;
            }

            _clockIndex = ( _clockIndex + 1 ) % _activityNum ;
            // Searched one loop
            if ( _clockIndex == lastClockIndex )
            {
               loopCount ++ ;
            }
         }

         _accessCount.init( 0 ) ;
         _lastClearTimestamp = _accessTimestamp.inc() ;
         _clearThread.init( 0 ) ;
      }

   done :
      PD_TRACE_EXIT( SDB_OPTCPMON__CLEARCP ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON__ALLOCACT, "_optCachedPlanMonitor::_allocateActivity" )
   INT32 _optCachedPlanMonitor::_allocateActivity ( optAccessPlan *pPlan )
   {
      INT32 activityID = OPT_INVALID_ACT_ID ;

      PD_TRACE_ENTRY( SDB_OPTCPMON__ALLOCACT ) ;

      // During clearing processing, to be safe, only one thread could
      // allocate activity
      BOOLEAN singleThread = ( 1 == _clearThread.peek() ) ;

      if ( !singleThread ||
           _allocateThread.compareAndSwap( 0, 1 ) )
      {
         // During clearing and the activities are full, could not allocate
         // activity for the plan
         if ( singleThread && _freeIndexEnd.peek() == _freeIndexBegin.peek() )
         {
            _allocateThread.init( 0 ) ;
            goto done ;
         }

         // Get free activity from free index
         UINT32 freeActivityIndex = _freeIndexBegin.inc() % _activityNum ;
         activityID = _pFreeActivityIDs[ freeActivityIndex ] ;
         optCachedPlanActivity &activity = _pActivities[ activityID ] ;

         SDB_ASSERT( activity.isEmpty(), "Activity is not empty" ) ;

         pPlan->setActivityID( activityID ) ;
         activity.setPlan( pPlan, _accessTimestamp.inc() ) ;
         activity.incAccessCount() ;
         _accessCount.inc() ;

         if ( singleThread )
         {
            _allocateThread.init( 0 ) ;
         }
      }

   done :
      PD_TRACE_EXIT( SDB_OPTCPMON__ALLOCACT ) ;
      return activityID ;
   }

   /*
      _optAccessPlanManager implement
    */
   _optAccessPlanManager::_optAccessPlanManager ()
   : _planCache(),
     _monitor()
   {
      _dmsCB = NULL ;
   }

   _optAccessPlanManager::~_optAccessPlanManager ()
   {
      clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_INIT, "_optAccessPlanManager::init" )
   INT32 _optAccessPlanManager::init ( SDB_DMSCB *dmsCB, UINT32 bucketNum )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_INIT ) ;

      SDB_ASSERT( dmsCB, "dmsCB is invalid" ) ;

      if ( bucketNum > 0 )
      {
         _planCache.initialize( bucketNum, &_monitor ) ;
         PD_CHECK( _planCache.isInitialized(), SDB_OOM, error, PDERROR,
                   "Failed to initialize plan caches" ) ;

         _monitor.initialize( &_planCache ) ;
         PD_CHECK( _monitor.isInitialized(), SDB_OOM, error, PDERROR,
                   "Failed to initialize plan cache sweeper" ) ;
      }

      _dmsCB = dmsCB ;

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM_INIT, rc ) ;
      return rc ;

   error :
      clear() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_CLEAR, "_optAccessPlanManager::clear" )
   void _optAccessPlanManager::clear ()
   {
      PD_TRACE_ENTRY( SDB_OPTAPM_CLEAR ) ;

      _planCache.clear() ;
      _monitor.clear() ;

      PD_TRACE_EXIT( SDB_OPTAPM_CLEAR ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_GETAP, "_optAccessPlanManager::getAccessPlan" )
   INT32 _optAccessPlanManager::getAccessPlan ( dmsStorageUnit *su,
                                                dmsMBContext *mbContext,
                                                const CHAR *pCLFullName,
                                                const BSONObj &selector,
                                                const BSONObj &matcher,
                                                const BSONObj &orderBy,
                                                const BSONObj &hint,
                                                SINT32 flags,
                                                SINT64 numToSkip,
                                                SINT64 numToReturn,
                                                optAccessPlan **ppPlan )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_GETAP ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;
      SDB_ASSERT( ppPlan, "ppPlan is invalid" ) ;

      optAccessPlan *pPlan = NULL ;

      // Construct the plan key, but needn't to get owned at this stage
      optAccessPlanKey planKey( pCLFullName, selector, matcher, orderBy, hint,
                                flags, numToSkip, numToReturn, FALSE ) ;
      planKey.generateKeyCode( su, mbContext ) ;

      // The cache is not initialized, create the plan directly
      if ( !isInitialized() )
      {
         rc = _createAccessPlan( su, mbContext, planKey, &pPlan ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create access plan, rc: %d", rc ) ;
         goto done ;
      }

      // Try to get the plan from cache first
      rc = _getAccessPlan( planKey, &pPlan ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get access plan, rc: %d", rc ) ;

      if ( NULL == pPlan )
      {
         // Failed to get plan from cache, build it
         rc = _createAccessPlan( su, mbContext, planKey, &pPlan ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create access plan, rc: %d",
                      rc ) ;
      }

   done :
      (*ppPlan) = pPlan ;
      PD_TRACE_EXITRC( SDB_OPTAPM_GETAP, rc ) ;
      return rc ;

   error :
      pPlan = NULL ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONUNLOADCS, "_optAccessPlanManager::onUnloadCS" )
   INT32 _optAccessPlanManager::onUnloadCS ( IDmsEventHolder *pEventHolder,
                                             IDmsSUCacheHolder *pCacheHolder,
                                             pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONRENAMECL ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidSUPlans( pCacheHolder ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONRENAMECL, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONRENAMECS, "_optAccessPlanManager::onRenameCS" )
   INT32 _optAccessPlanManager::onRenameCS ( IDmsEventHolder *pEventHolder,
                                             IDmsSUCacheHolder *pCacheHolder,
                                             const CHAR *pOldCSName,
                                             const CHAR *pNewCSName,
                                             pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONRENAMECS ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidSUPlans( pCacheHolder ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONRENAMECS, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONDROPCS, "_optAccessPlanManager::onDropCS" )
   INT32 _optAccessPlanManager::onDropCS ( IDmsEventHolder *pEventHolder,
                                           IDmsSUCacheHolder *pCacheHolder,
                                           pmdEDUCB *cb,
                                           SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONDROPCS ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidSUPlans( pCacheHolder ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONDROPCS, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONRENAMECL, "_optAccessPlanManager::onRenameCL" )
   INT32 _optAccessPlanManager::onRenameCL ( IDmsEventHolder *pEventHolder,
                                             IDmsSUCacheHolder *pCacheHolder,
                                             const dmsEventCLItem &clItem,
                                             const CHAR *pNewCLName,
                                             pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONRENAMECL ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidCLPlans( pCacheHolder, clItem._mbID, clItem._clLID ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONRENAMECL, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONTRUNCATECL, "_optAccessPlanManager::onTruncateCL" )
   INT32 _optAccessPlanManager::onTruncateCL ( IDmsEventHolder *pEventHolder,
                                               IDmsSUCacheHolder *pCacheHolder,
                                               const dmsEventCLItem &clItem,
                                               UINT32 newCLLID,
                                               pmdEDUCB *cb,
                                               SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONTRUNCATECL ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidCLPlans( pCacheHolder, clItem._mbID, clItem._clLID ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONTRUNCATECL, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONDROPCL, "_optAccessPlanManager::onDropCL" )
   INT32 _optAccessPlanManager::onDropCL ( IDmsEventHolder *pEventHolder,
                                           IDmsSUCacheHolder *pCacheHolder,
                                           const dmsEventCLItem &clItem,
                                           pmdEDUCB *cb,
                                           SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONDROPCL ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidCLPlans( pCacheHolder, clItem._mbID, clItem._clLID ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONDROPCL, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONCRTIDX, "_optAccessPlanManager::onCreateIndex" )
   INT32 _optAccessPlanManager::onCreateIndex ( IDmsEventHolder *pEventHolder,
                                                IDmsSUCacheHolder *pCacheHolder,
                                                const dmsEventCLItem &clItem,
                                                const dmsEventIdxItem &idxItem,
                                                pmdEDUCB *cb,
                                                SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONCRTIDX ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidCLPlans( pCacheHolder, clItem._mbID, clItem._clLID ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONCRTIDX, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONDROPIDX, "_optAccessPlanManager::onDropIndex" )
   INT32 _optAccessPlanManager::onDropIndex ( IDmsEventHolder *pEventHolder,
                                              IDmsSUCacheHolder *pCacheHolder,
                                              const dmsEventCLItem &clItem,
                                              const dmsEventIdxItem &idxItem,
                                              pmdEDUCB *cb,
                                              SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONDROPIDX ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidCLPlans( pCacheHolder, clItem._mbID, clItem._clLID ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONDROPIDX, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONCLRSUCACHES, "_optAccessPlanManager::onClearSUCaches" )
   INT32 _optAccessPlanManager::onClearSUCaches ( IDmsEventHolder *pEventHolder,
                                                  IDmsSUCacheHolder *pCacheHolder )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONCLRSUCACHES ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidSUPlans( pCacheHolder ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONCLRSUCACHES, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_ONCLRCLCACHES, "_optAccessPlanManager::onClearCLCaches" )
   INT32 _optAccessPlanManager::onClearCLCaches ( IDmsEventHolder *pEventHolder,
                                                  IDmsSUCacheHolder *pCacheHolder,
                                                  const dmsEventCLItem &clItem )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_ONCLRCLCACHES ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      if ( pCacheHolder )
      {
         _invalidCLPlans( pCacheHolder, clItem._mbID, clItem._clLID ) ;
      }

      PD_TRACE_EXITRC( SDB_OPTAPM_ONCLRCLCACHES, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__CRTAP, "_optAccessPlanManager::_createAccessPlan" )
   INT32 _optAccessPlanManager::_createAccessPlan ( dmsStorageUnit *su,
                                                    dmsMBContext *mbContext,
                                                    const optAccessPlanKey &planKey,
                                                    optAccessPlan **ppPlan )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__CRTAP ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;
      SDB_ASSERT( ppPlan, "ppPlan is invalid" ) ;

      optAccessPlan *pPlan = NULL ;

      pPlan = SDB_OSS_NEW optAccessPlan ( planKey, TRUE ) ;
      PD_CHECK( NULL != pPlan, SDB_OOM, error, PDERROR,
                "Not able to allocate memory for new plan" ) ;

      rc = pPlan->optimize( su, mbContext ) ;
      PD_RC_CHECK( rc, ( SDB_RTN_INVALID_PREDICATES == rc ) ? PDINFO : PDERROR,
                   "Failed to optimize plan, query: %s\norder %s\nhint %s",
                   planKey._query.toString().c_str(),
                   planKey._orderBy.toString().c_str(),
                   planKey._hint.toString().c_str() ) ;

      // Set the outputs
      (*ppPlan) = pPlan ;

      // Cache the plan
      if ( isInitialized() )
      {
         _cacheAccessPlan( pPlan ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__CRTAP, rc ) ;
      return rc ;

   error :
      if ( NULL != pPlan )
      {
         SDB_OSS_DEL pPlan ;
         pPlan = NULL ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__GETAP, "_optAccessPlanManager::_getAccessPlan" )
   INT32 _optAccessPlanManager::_getAccessPlan ( const optAccessPlanKey &planKey,
                                                 optAccessPlan **ppPlan )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__GETAP ) ;

      SDB_ASSERT( ppPlan, "ppPlan is invalid" ) ;

      optAccessPlan *pPlan = NULL ;

      pPlan = (optAccessPlan *)_planCache.getItem( planKey ) ;

      (*ppPlan) = pPlan ;

      PD_TRACE_EXITRC( SDB_OPTAPM__GETAP, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__CACHEAP, "_optAccessPlanManager::_cacheAccessPlan" )
   BOOLEAN _optAccessPlanManager::_cacheAccessPlan ( optAccessPlan *pPlan )
   {
      BOOLEAN cached = FALSE ;

      PD_TRACE_ENTRY( SDB_OPTAPM__CACHEAP ) ;

      // To avoid dropCL during the processing, set the activity in advanced.
      // And pre-increase the cached plan count, so the monitor could test if
      // the number of cached plans will exceed the high water mark with this
      // new plan
      _monitor.incCachedPlanCount() ;
      if ( _monitor.setActivity( pPlan ) )
      {
         cached = _planCache.addPlan( pPlan ) ;
         if ( !cached )
         {
            INT32 activityID = pPlan->resetActivityID() ;
            if ( OPT_INVALID_ACT_ID != activityID )
            {
               _monitor.resetActivity( activityID ) ;
            }
         }
      }

      if ( !cached )
      {
         _monitor.decCachedPlanCount() ;
      }

      PD_TRACE_EXIT( SDB_OPTAPM__CACHEAP ) ;

      return cached ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__INVALIDPLANS, "_optAccessPlanManager::_invalidCachedPlans" )
   void _optAccessPlanManager::_invalidSUPlans ( IDmsSUCacheHolder *pCacheHolder )
   {
      PD_TRACE_ENTRY( SDB_OPTAPM__INVALIDCLPLANS ) ;

      SDB_ASSERT( pCacheHolder, "pCacheHolder is invalid" ) ;

      dmsCachedPlanMgr *pCachedPlanMgr =
            (dmsCachedPlanMgr *)pCacheHolder->getSUCache( DMS_CACHE_TYPE_PLAN ) ;
      if ( pCachedPlanMgr )
      {
         UINT32 suLID = pCacheHolder->getSULID() ;

         _planCache.invalidateSUPlans( pCachedPlanMgr, suLID ) ;

         // No plans belong to this SU, clear the bitmap and free the units
         pCachedPlanMgr->resetCacheBitmap() ;
         pCachedPlanMgr->clearCacheUnits() ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__INVALIDCLPLANS, "_optAccessPlanManager::_invalidCLPlans" )
   void _optAccessPlanManager::_invalidCLPlans ( IDmsSUCacheHolder *pCacheHolder,
                                                 UINT16 mbID, UINT32 clLID )
   {
      PD_TRACE_ENTRY( SDB_OPTAPM__INVALIDCLPLANS ) ;

      SDB_ASSERT( pCacheHolder, "pCacheHolder is invalid" ) ;

      dmsCachedPlanMgr *pCachedPlanMgr =
            (dmsCachedPlanMgr *)pCacheHolder->getSUCache( DMS_CACHE_TYPE_PLAN ) ;
      if ( pCachedPlanMgr )
      {
         UINT32 suLID = pCacheHolder->getSULID() ;

         _planCache.invalidateCLPlans( pCachedPlanMgr, suLID, clLID ) ;

         pCachedPlanMgr->removeCacheUnit( mbID, TRUE ) ;
      }

      PD_TRACE_EXIT( SDB_OPTAPM__INVALIDCLPLANS ) ;
   }

}

