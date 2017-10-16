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
#include "optPlanClearJob.hpp"

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

      // Increase reference count before we cache the plan
      pPlan->incRefCount() ;
      result = addItem( pPlan ) ;

      PD_TRACE_EXIT( SDB_OPTAPCACHES_ADDPLAN ) ;

      return result ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_RMPLAN, "_optAccessPlanCache::removeCachedPlan" )
   void _optAccessPlanCache::removeCachedPlan ( optAccessPlan *pPlan )
   {
      PD_TRACE_ENTRY( SDB_OPTAPCACHES_RMPLAN ) ;

      // Increase the reference count before we delete it
      pPlan->incRefCount() ;
      if ( removeItem( pPlan ) )
      {
         // We need to test the activity ID to check if
         // someone else is also deleting this plan
         INT32 activityID = pPlan->resetActivityID() ;
         if ( OPT_INVALID_ACT_ID != activityID )
         {
            _pMonitor->resetActivity( activityID ) ;
         }
         _pMonitor->decCachedPlanCount( 1 ) ;
      }
      pPlan->release() ;

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
         if ( pCachedPlanMgr->testCacheBitmap( bucketID ) )
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
                     // Increase the reference count before we delete it
                     pPlan->incRefCount() ;

                     // Locked bucket already, safe to remove from bucket
                     if ( pBucket->removeItem( pPlan ) )
                     {
                        // We need to test the activity ID to check if someone
                        // else is also deleting this plan
                        INT32 activityID = pPlan->resetActivityID() ;
                        if ( OPT_INVALID_ACT_ID != activityID )
                        {
                           _pMonitor->resetActivity( activityID ) ;
                        }
                        deleteCount ++ ;
                     }

                     pPlan->release() ;
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

      if ( deleteCount > 0 )
      {
         _pMonitor->decCachedPlanCount( deleteCount ) ;
      }

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
         if ( pCachedPlanMgr->testCacheBitmap( bucketID ) )
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
                     // Increase the reference count before we delete it
                     pPlan->incRefCount() ;

                     // Locked bucket already, safe to remove from bucket
                     if ( pBucket->removeItem( pPlan ) )
                     {
                        // We need to test the activity ID to check if someone
                        // else is also deleting this plan
                        INT32 activityID = pPlan->resetActivityID() ;
                        if ( OPT_INVALID_ACT_ID != activityID )
                        {
                           _pMonitor->resetActivity( activityID ) ;
                        }
                        deleteCount ++ ;
                     }
                     else if ( clearBit )
                     {
                        // The plan is not removed, so to be safe,
                        // could not clear the bit
                        clearBit = FALSE ;
                     }

                     pPlan->release() ;
                  }
                  else if ( clearBit && pPlan->getSULID() == suLID )
                  {
                     // Still contains plans from the same collection space
                     // could not clear the bit
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

      if ( deleteCount > 0 )
      {
         _pMonitor->decCachedPlanCount( deleteCount ) ;
      }

      PD_TRACE_EXIT( SDB_OPTAPCACHES_INVALIDCLPLANS ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_INVALIDALLPLANS, "_optAccessPlanCache::invalidateAllPlans" )
   void _optAccessPlanCache::invalidateAllPlans ()
   {
      PD_TRACE_ENTRY( SDB_OPTAPCACHES_INVALIDALLPLANS ) ;


      UINT32 deleteCount = 0 ;

      for ( UINT32 bucketID = 0 ; bucketID < _bucketNum ; bucketID ++ )
      {
         ossScopedRWLock scopedLock( _pMonitor->getClearLock(), EXCLUSIVE ) ;
         utilHashTableBucket *pBucket = getBucket( bucketID, EXCLUSIVE ) ;

         if ( NULL != pBucket )
         {
            optAccessPlan *pPlan = pBucket->getHead() ;
            while ( pPlan )
            {
               optAccessPlan *pNextPlan = (optAccessPlan *)pPlan->getNext() ;
               // Increase the reference count before we delete it
               pPlan->incRefCount() ;

               // Locked bucket already, safe to remove from bucket
               if ( pBucket->removeItem( pPlan ) )
               {
                  // We need to test the activity ID to check if someone
                  // else is also deleting this plan
                  INT32 activityID = pPlan->resetActivityID() ;
                  if ( OPT_INVALID_ACT_ID != activityID )
                  {
                     _pMonitor->resetActivity( activityID ) ;
                  }
                  deleteCount ++ ;
               }
               pPlan->release() ;
               pPlan = pNextPlan ;
            }

            releaseBucket( bucketID, EXCLUSIVE ) ;
         }
         else
         {
            SDB_ASSERT( pBucket, "pBucket is invalid" ) ;
         }
      }

      if ( deleteCount > 0 )
      {
         _pMonitor->decCachedPlanCount( deleteCount ) ;
      }

      PD_TRACE_EXIT( SDB_OPTAPCACHES_INVALIDALLPLANS ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPCACHES_INVALIDCLPLANS_NAME, "_optAccessPlanCache::invalidateCLPlans" )
   void _optAccessPlanCache::invalidateCLPlans ( const CHAR *pCLFullName )
   {
      PD_TRACE_ENTRY( SDB_OPTAPCACHES_INVALIDCLPLANS_NAME ) ;

      UINT32 deleteCount = 0 ;

      for ( UINT32 bucketID = 0 ; bucketID < _bucketNum ; bucketID ++ )
      {
         // Lock the clear lock shared, parallel removing for different
         ossScopedRWLock scopedLock( _pMonitor->getClearLock(), SHARED ) ;
         utilHashTableBucket *pBucket = getBucket( bucketID, EXCLUSIVE ) ;

         if ( NULL != pBucket )
         {
            optAccessPlan *pPlan = pBucket->getHead() ;
            while ( pPlan )
            {
               optAccessPlan *pNextPlan = (optAccessPlan *)pPlan->getNext() ;
               if ( 0 == ossStrncmp( pCLFullName, pPlan->getCLFullName(),
                                     DMS_COLLECTION_FULL_NAME_SZ ) )
               {
                  // Increase the reference count before we delete it
                  pPlan->incRefCount() ;

                  // Locked bucket already, safe to remove from bucket
                  if ( pBucket->removeItem( pPlan ) )
                  {
                     // We need to test the activity ID to check if someone
                     // else is also deleting this plan
                     INT32 activityID = pPlan->resetActivityID() ;
                     if ( OPT_INVALID_ACT_ID != activityID )
                     {
                        _pMonitor->resetActivity( activityID ) ;
                     }
                     deleteCount ++ ;
                  }
                  pPlan->release() ;
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

      if ( deleteCount > 0 )
      {
         _pMonitor->decCachedPlanCount( deleteCount ) ;
      }

      PD_TRACE_EXIT( SDB_OPTAPCACHES_INVALIDCLPLANS_NAME ) ;
   }

   UINT32 _optAccessPlanCache::getCachedPlanCount () const
   {
      return _pMonitor->getCachedPlanCount() ;
   }

   void _optAccessPlanCache::afterAddItem ( UINT32 bucketID,
                                            optAccessPlan *pPlan )
   {
      SDB_ASSERT( pPlan, "pPlan is invalid" ) ;
      pPlan->setCachedBitmap() ;
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
     _accessTimestamp( 0 )
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

      _lastClearTimestamp = 0 ;

      PD_TRACE_EXIT( SDB_OPTCPMON_CLEAR ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON_SETACT, "_optCachedPlanMonitor::setActivity" )
   BOOLEAN _optCachedPlanMonitor::setActivity ( optAccessPlan *pPlan )
   {
      BOOLEAN result = FALSE ;

      PD_TRACE_ENTRY( SDB_OPTCPMON_SETACT ) ;

      INT32 activityID = OPT_INVALID_ACT_ID ;
      BOOLEAN criticalMode = FALSE ;

      if ( _freeIndexEnd.peek() >= OPT_PLAN_CACHE_UINT64_LIMIT )
      {
         criticalMode = TRUE ;
      }
      else if ( _cachedPlanCount.peek() > _highWaterMark )
      {
         signalPlanClearJob() ;
         criticalMode = TRUE ;
      }

      activityID = _allocateActivity( pPlan, criticalMode ) ;

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

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON_SIGNALJOB, "_optCachedPlanMonitor::signalPlanClearJob" )
   void _optCachedPlanMonitor::signalPlanClearJob ()
   {
      PD_TRACE_ENTRY( SDB_OPTCPMON_SIGNALJOB ) ;
      _clearEvent.signal() ;
      PD_TRACE_EXIT( SDB_OPTCPMON_SIGNALJOB ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON_CHKFREEIDX, "_optCachedPlanMonitor::checkFreeIndexes" )
   void _optCachedPlanMonitor::checkFreeIndexes ()
   {
      PD_TRACE_ENTRY( SDB_OPTCPMON_CHKFREEIDX ) ;

      if ( _freeIndexEnd.peek() >= OPT_PLAN_CACHE_UINT64_LIMIT &&
           _clearThread.compareAndSwap( 0, 1 ) )
      {
         // Only one thread could enter this branch

         // Lock the clear lock exclusively, so other threads could not
         // remove plans during this procedure
         ossScopedRWLock scopedLock( &_clearLock, EXCLUSIVE ) ;

         // Must disable the allocation
         while ( !_allocateThread.compareAndSwap( 0, 1 ) )
         {
            ossSleep( 100 ) ;
         }

         PD_LOG( PDDEBUG, "Cached Plan Monitor: free index is too large "
                 "[ %llu - %llu ], need reset", _freeIndexBegin.peek(),
                 _freeIndexEnd.peek() ) ;

         // The clear lock is exclusive, safe to update the end index
         UINT64 newFreeIndexEnd = _freeIndexEnd.peek() % _activityNum ;
         _freeIndexEnd.init( newFreeIndexEnd ) ;

         // After last step, the end index is smaller than the begin index.
         // In that case, although the thread to cache plan passed the
         // allocating test, it is not able to allocate activity any more

         // Must loop until the begin index is reset
         UINT64 oldFreeIndexBegin = _freeIndexBegin.peek() ;
         UINT64 tmpFreeIndexBegin = oldFreeIndexBegin ;
         UINT64 newFreeIndexBegin = 0 ;
         while ( TRUE )
         {
            newFreeIndexBegin = oldFreeIndexBegin % _activityNum ;
            tmpFreeIndexBegin = _freeIndexBegin.compareAndSwapWithReturn(
                                    oldFreeIndexBegin, newFreeIndexBegin ) ;
            if ( tmpFreeIndexBegin == oldFreeIndexBegin )
            {
               break ;
            }
            oldFreeIndexBegin = tmpFreeIndexBegin ;
         }

         if ( newFreeIndexBegin > newFreeIndexEnd )
         {
            _freeIndexEnd.init( newFreeIndexEnd + _activityNum ) ;
         }

         PD_LOG( PDDEBUG, "Cached Plan Monitor: free index reseted "
                 "[ %llu - %llu ]", _freeIndexBegin.peek(),
                 _freeIndexEnd.peek() ) ;

         // Allow to allocate now
         _allocateThread.init( 0 ) ;
         _clearThread.init( 0 ) ;
      }

      PD_TRACE_EXIT( SDB_OPTCPMON_CHKFREEIDX ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON_CHKACTIME, "_optCachedPlanMonitor::checkAccessTimestamp" )
   void _optCachedPlanMonitor::checkAccessTimestamp ()
   {
      PD_TRACE_ENTRY( SDB_OPTCPMON_CHKACTIME ) ;

      // Safe to reset access time if it is too large
      // NOTE: plans with large access timestamp without reset would not pass
      // the -EPSILON clear score test in clearing process
      if ( _accessTimestamp.peek() > OPT_PLAN_CACHE_UINT64_LIMIT )
      {
         PD_LOG( PDDEBUG, "Cached Plan Monitor: access timestamp reseted" ) ;
         _accessTimestamp.init( 0 ) ;
         _lastClearTimestamp = 0 ;
      }

      PD_TRACE_EXIT( SDB_OPTCPMON_CHKACTIME ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON_CLEARCP, "_optCachedPlanMonitor::clearCachedPlans" )
   void _optCachedPlanMonitor::clearCachedPlans ()
   {
      PD_TRACE_ENTRY( SDB_OPTCPMON_CLEARCP ) ;

      if ( _clearThread.compareAndSwap( 0, 1 ) )
      {
         // Only one thread could enter this branch

         // Lock the clear lock exclusively, so other threads could not
         // remove plans during this procedure
         ossScopedRWLock scopedLock( &_clearLock, EXCLUSIVE ) ;

         UINT32 needRemoveCount = 0 ;
         double avgClearScore = 0.0 ;
         UINT64 currentTimestamp = 0 ;
         UINT64 totalAccessCount = 0 ;
         UINT64 avgAccessCount = 0 ;
         UINT32 lastClockIndex = _clockIndex ;

         // Check again after lock
         UINT32 cachedPlanCount = _cachedPlanCount.peek() ;
         if ( cachedPlanCount < _highWaterMark )
         {
            _clearThread.init( 0 ) ;
            goto done ;
         }

         needRemoveCount = cachedPlanCount - _lowWaterMark ;

         PD_LOG( PDDEBUG, "Cached Plan Monitor: %u plans are cached, "
                 "%u need to be removed", cachedPlanCount, needRemoveCount ) ;

         // Calculate the average clear score of all cached plans
         currentTimestamp = _accessTimestamp.inc() ;

         // Total access count is difference between current timestamp and last
         // clear timestamp since the logical timestamp is used
         totalAccessCount = currentTimestamp - _lastClearTimestamp ;
         totalAccessCount = OSS_MAX( 1, totalAccessCount ) ;

         avgAccessCount = totalAccessCount / cachedPlanCount ;
         avgAccessCount = OSS_MAX( 1, avgAccessCount ) ;

         avgClearScore = 1.0 / (double)cachedPlanCount ;

         // End searching conditions:
         // 1. removed enough plans
         // 2. searched one loop
         while ( needRemoveCount > 0 )
         {
            optCachedPlanActivity &activity = _pActivities[ _clockIndex ] ;
            UINT64 accessTime = 0 ;
            UINT32 accessCount = 0 ;
            double curClearScore = 0.0 ;

            if ( activity.isEmpty() )
            {
               _clockIndex = ( _clockIndex + 1 ) % _activityNum ;
               // Searched one loop
               if ( _clockIndex == lastClockIndex )
               {
                  break ;
               }
               continue ;
            }

            // Calculate the clear score of current plan
            accessTime = activity.getAccessTime() ;
            accessCount = activity.getAccessCount() ;
            curClearScore = (double)accessCount /
                            (double)( currentTimestamp - accessTime ) ;

            if ( curClearScore > -OSS_EPSILON &&
                 curClearScore < avgClearScore )
            {
               // The score is smaller than average score, clear the plan
               // NOTE: the score < 0.0, means access time is larger than
               // current clear time
               _pPlanCache->removeCachedPlan( activity.getPlan() ) ;
               needRemoveCount -- ;
            }
            else
            {
               // Decrease the access count by average access count
               activity.decAccessCount( avgAccessCount ) ;
            }

            _clockIndex = ( _clockIndex + 1 ) % _activityNum ;
            if ( _clockIndex == lastClockIndex )
            {
               // Searched one loop and could be stopped
               break ;
            }
         }

         PD_LOG( PDDEBUG, "Cached Plan Monitor: cleared %u cached plans, "
                 "%u left", cachedPlanCount - _lowWaterMark - needRemoveCount,
                 _cachedPlanCount.peek() ) ;

         _lastClearTimestamp = _accessTimestamp.inc() ;
         _clearThread.init( 0 ) ;
      }

   done :
      PD_TRACE_EXIT( SDB_OPTCPMON_CLEARCP ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTCPMON__ALLOCACT, "_optCachedPlanMonitor::_allocateActivity" )
   INT32 _optCachedPlanMonitor::_allocateActivity ( optAccessPlan *pPlan,
                                                    BOOLEAN criticalMode )
   {
      INT32 activityID = OPT_INVALID_ACT_ID ;

      PD_TRACE_ENTRY( SDB_OPTCPMON__ALLOCACT ) ;

      // During clearing processing, to be safe, only one thread could
      // allocate activity
      criticalMode |= _clearThread.compare( 1 ) ;

      if ( !criticalMode ||
           _allocateThread.compareAndSwap( 0, 1 ) )
      {
         // During clearing and the activities are full, could not allocate
         // activity for the plan
         if ( criticalMode &&
              _freeIndexEnd.compare( _freeIndexBegin.peek() ) )
         {
            _allocateThread.init( 0 ) ;
            goto done ;
         }

         // Get free activity from free index
         UINT64 freeActivityIndex = _freeIndexBegin.inc() ;

         if ( criticalMode )
         {
            _allocateThread.init( 0 ) ;
         }

         // In critical mode, safe to allocate without checking the end index
         // But in non-critical mode, we need to check the end index
         if ( criticalMode ||
              freeActivityIndex < _freeIndexEnd.peek() )
         {
            activityID = _pFreeActivityIDs[ freeActivityIndex % _activityNum ] ;
            optCachedPlanActivity &activity = _pActivities[ activityID ] ;

            SDB_ASSERT( activity.isEmpty(), "Activity is not empty" ) ;

            pPlan->setActivityID( activityID ) ;
            activity.setPlan( pPlan, _accessTimestamp.inc() ) ;
            activity.incAccessCount() ;
         }
         else
         {
            // The begin index caught with the end index, which means:
            // 1. the indexes are reseting for too large index values
            // 2. the number of cached plans reached the maximum limitation

            // The clear job is handling the first case, and the second case
            // will be handled by the next query by notifying the clear job

            // The allocated index will be reused in the next round after
            // the end index catching up with the begin index by clearing

            // Do nothing here
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
   : _mthMatchConfigHolder(),
     _planCache(),
     _monitor()
   {
      _clearJobEduID = PMD_INVALID_EDUID ;
      _cacheBucketNum = 0 ;
      _cacheLevel = OPT_PLAN_NOCACHE ;
   }

   _optAccessPlanManager::~_optAccessPlanManager ()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_INIT, "_optAccessPlanManager::init" )
   INT32 _optAccessPlanManager::init ( UINT32 bucketNum,
                                       OPT_PLAN_CACHE_LEVEL cacheLevel,
                                       BOOLEAN enableMixCmp )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_INIT ) ;

      SDB_ASSERT( !_planCache.isInitialized(),
                  "cache should not be initialized" ) ;

      _cacheBucketNum = bucketNum ;
      _cacheLevel = OPT_PLAN_NOCACHE ;

      // Always update mix-compare mode
      setMthEnableMixCmp( enableMixCmp ) ;

      if ( bucketNum > 0 && cacheLevel > OPT_PLAN_NOCACHE )
      {
         _planCache.initialize( bucketNum, &_monitor ) ;
         PD_CHECK( _planCache.isInitialized(), SDB_OOM, error, PDERROR,
                   "Failed to initialize plan caches" ) ;

         _monitor.initialize( &_planCache ) ;
         PD_CHECK( _monitor.isInitialized(), SDB_OOM, error, PDERROR,
                   "Failed to initialize plan cache sweeper" ) ;

         rc = startPlanClearJob( &_clearJobEduID ) ;
         PD_RC_CHECK( rc, PDERROR, "Start cached plan clearing job thread "
                      "failed, rc: %d", rc ) ;

         _cacheLevel = cacheLevel ;
      }

      // Update parameterize and fuzzy-operator by cache level
      setMthEnableParameterized( _cacheLevel >= OPT_PLAN_PARAMETERIZED ) ;
      setMthEnableFuzzyOptr( _cacheLevel >= OPT_PLAN_FUZZYOPTR ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM_INIT, rc ) ;
      return rc ;

   error :
      clear() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_REINIT, "_optAccessPlanManager::reinit" )
   INT32 _optAccessPlanManager::reinit ( OPT_PLAN_CACHE_LEVEL cacheLevel,
                                         BOOLEAN enableMixCmp )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_REINIT ) ;

      ossScopedLock scopedLock( &_reinitLatch ) ;

      if ( _planCache.isInitialized() )
      {
         // For change to OPT_PLAN_NOCACHE, we only clear the cache, but we
         // don't stop the clearing job
         if ( cacheLevel != _cacheLevel ||
              enableMixCmp != mthEnabledMixCmp() )
         {
            sdbGetDMSCB()->clearSUCaches( DMS_EVENT_MASK_PLAN ) ;
         }

         _cacheLevel = cacheLevel ;
         setMthEnableMixCmp( enableMixCmp ) ;
         setMthEnableParameterized( _cacheLevel >= OPT_PLAN_PARAMETERIZED ) ;
         setMthEnableFuzzyOptr( _cacheLevel >= OPT_PLAN_FUZZYOPTR ) ;
      }
      else
      {
         rc = init( _cacheBucketNum, cacheLevel, enableMixCmp ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to initialize access plan manager, "
                      "rc: %d", rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM_REINIT, rc ) ;
      return rc ;

   error :
      clear() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_CLEAR, "_optAccessPlanManager::clear" )
   void _optAccessPlanManager::clear ()
   {
      PD_TRACE_ENTRY( SDB_OPTAPM_CLEAR ) ;

      if ( _planCache.isInitialized() )
      {
         _planCache.invalidateAllPlans() ;
      }
      _planCache.clear() ;
      _monitor.clear() ;

      PD_TRACE_EXIT( SDB_OPTAPM_CLEAR ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_GETAP, "_optAccessPlanManager::getAccessPlan" )
   INT32 _optAccessPlanManager::getAccessPlan ( const rtnQueryOptions &options,
                                                dmsStorageUnit *su,
                                                dmsMBContext *mbContext,
                                                optAccessPlanRuntime &planRuntime )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_GETAP ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;

      BOOLEAN gotMainCLPlan = FALSE ;

      if ( isInitialized() &&
           _cacheLevel >= OPT_PLAN_PARAMETERIZED &&
           NULL != options._mainCLName )
      {
         dmsCachedPlanMgr *pCachedPlanMgr = su->getCachedPlanMgr() ;
         if ( NULL == pCachedPlanMgr ||
              pCachedPlanMgr->testMainCLInvalidBitmap( mbContext->mbID() ) )
         {
            // The sub-collection is not validated to use main-collection plans,
            // generate a general plan for it
            gotMainCLPlan = FALSE ;
         }
         else
         {
            // If it is from main-collection, try to get or create main-collection
            // plan
            // Note: sub-collection name is considered as one of parameters
            rc = _getMainCLAccessPlan( options, su, mbContext, planRuntime ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to get main-collection access plan "
                         "for query [ %s ], rc: %d", options.toString().c_str(),
                         rc ) ;
            gotMainCLPlan = TRUE ;
         }
      }

      if ( !gotMainCLPlan )
      {
         // If cache is not initialized, or it not from main-collection, or the
         // cache level is too low, get or create normal plan
         rc = _getCLAccessPlan( options, su, mbContext, planRuntime ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get collection access plan for "
                      "query [ %s ], rc: %d", options.toString().c_str(), rc ) ;
      }

#ifdef _DEBUG
      if ( OPT_PLAN_TYPE_MAINCL == planRuntime.getPlan()->getPlanType() )
      {
         PD_LOG( PDDEBUG, "Got main-collection plan [%s]",
                 planRuntime.getPlan()->toString().c_str() ) ;
      }
#endif

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM_GETAP, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_GETTEMPAP, "_optAccessPlanManager::getTempAccessPlan" )
   INT32 _optAccessPlanManager::getTempAccessPlan ( const rtnQueryOptions &options,
                                                    dmsStorageUnit *su,
                                                    dmsMBContext *mbContext,
                                                    optAccessPlanRuntime &planRuntime )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM_GETTEMPAP ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;

      rc = _getCLAccessPlan( options, OPT_PLAN_NOCACHE, su, mbContext,
                             planRuntime ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get collection access plan for "
                   "query [ %s ], rc: %d", options.toString().c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM_GETTEMPAP, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_INVALIDCLPLANS, "_optAccessPlanManager::invalidateCLPlans" )
   void _optAccessPlanManager::invalidateCLPlans ( const CHAR *pCLFullName )
   {
      PD_TRACE_ENTRY( SDB_OPTAPM_INVALIDCLPLANS ) ;

      if ( isInitialized() )
      {
         _planCache.invalidateCLPlans( pCLFullName ) ;
      }

      PD_TRACE_EXIT( SDB_OPTAPM_INVALIDCLPLANS ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM_INVALIDALLPLANS, "_optAccessPlanManager::invalidateAllPlans" )
   void _optAccessPlanManager::invalidateAllPlans ()
   {
      PD_TRACE_ENTRY( SDB_OPTAPM_INVALIDALLPLANS ) ;

      if ( isInitialized() )
      {
         _planCache.invalidateAllPlans() ;
      }

      PD_TRACE_EXIT( SDB_OPTAPM_INVALIDALLPLANS ) ;
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


   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__GETCLAP, "_optAccessPlanManager::_getCLAccessPlan" )
   INT32 _optAccessPlanManager::_getCLAccessPlan ( const rtnQueryOptions &options,
                                                   dmsStorageUnit *su,
                                                   dmsMBContext *mbContext,
                                                   optAccessPlanRuntime &planRuntime )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__GETCLAP ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;

      OPT_PLAN_CACHE_LEVEL cacheLevel = _cacheLevel ;

      // If the collection have been marked parameterized invalid,
      // lower the cache level to normalized
      if ( cacheLevel >= OPT_PLAN_PARAMETERIZED )
      {
         dmsCachedPlanMgr *pCachedPlanMgr = su->getCachedPlanMgr() ;
         if ( pCachedPlanMgr == NULL ||
              pCachedPlanMgr->testParamInvalidBitmap( mbContext->mbID() ) )
         {
            PD_LOG( PDDEBUG, "Collection [%s] is invalid for parameterized "
                    "plans", options._fullName ) ;
            cacheLevel = OPT_PLAN_NORMALIZED ;
         }
      }

      rc = _getCLAccessPlan( options, cacheLevel, su, mbContext, planRuntime ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get collection access plan for "
                   "query [ %s ], rc: %d", options.toString().c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__GETCLAP, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__GETCLAP_LEVEL, "_optAccessPlanManager::_getCLAccessPlan" )
   INT32 _optAccessPlanManager::_getCLAccessPlan ( const rtnQueryOptions &options,
                                                   OPT_PLAN_CACHE_LEVEL cacheLevel,
                                                   dmsStorageUnit *su,
                                                   dmsMBContext *mbContext,
                                                   optAccessPlanRuntime &planRuntime )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__GETCLAP_LEVEL ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;

      optGeneralAccessPlan *pPlan = NULL ;
      optAccessPlan *pTmpPlan = NULL ;

      // Construct the plan key, but needn't to get owned at this stage
      optAccessPlanKey planKey( options, cacheLevel ) ;

      mthMatchHelper matchHelper( cacheLevel, getMatchConfig() ) ;
      BOOLEAN needCache = ( isInitialized() &&
                            cacheLevel > OPT_PLAN_NOCACHE ) ;

      planRuntime.clear() ;

      rc = _prepareAccessPlanKey( su, mbContext, planKey, matchHelper,
                                  planRuntime ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to prepare key of access plan, rc: %d",
                   rc ) ;

      // If cache is initialized, try to get plan from cache first
      if ( needCache )
      {
         rc = _getCachedAccessPlan( planKey, &pTmpPlan ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get access plan, rc: %d", rc ) ;
         if ( NULL != pTmpPlan )
         {
            pPlan = dynamic_cast<_optGeneralAccessPlan *>( pTmpPlan ) ;
            if ( NULL == pPlan )
            {
               // Cast failed, release the temp plan
               pTmpPlan->release() ;
            }
         }
      }

      if ( NULL == pPlan )
      {
         // Failed to get plan from cache, create it
         rc = _createAccessPlan( su, mbContext, planKey, planRuntime,
                                 matchHelper, &pPlan, needCache ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create access plan, rc: %d",
                      rc ) ;

         planRuntime.setPlan( pPlan, TRUE ) ;
      }
      else
      {
         if ( pPlan->getCacheLevel() >= OPT_PLAN_PARAMETERIZED )
         {
            optParamAccessPlan *paramPlan = dynamic_cast<optParamAccessPlan *>( pPlan ) ;
            SDB_ASSERT( paramPlan, "paramPlan is invalid" ) ;

            // Plan is parameterized, bind the parameters
            rc = _validateParamPlan( su, mbContext, planKey, planRuntime,
                                     matchHelper, paramPlan ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to validate parameterized plan, "
                         "rc: %d", rc ) ;
         }
         else
         {
            // We don't need the match runtime any more
            // Use the one in the plan
            planRuntime.deleteMatchRuntime() ;
         }

         if ( planRuntime.getPlan() != NULL )
         {
            pPlan->release() ;
         }
         else
         {
            planRuntime.setPlan( pPlan, FALSE ) ;
         }
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__GETCLAP_LEVEL, rc ) ;
      return rc ;

   error :
      if ( NULL != pPlan )
      {
         pPlan->release() ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__GETMAINAP, "_optAccessPlanManager::_getMainCLAccessPlan" )
   INT32 _optAccessPlanManager::_getMainCLAccessPlan ( const rtnQueryOptions &options,
                                                       dmsStorageUnit *su,
                                                       dmsMBContext *mbContext,
                                                       optAccessPlanRuntime &planRuntime )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__GETMAINAP ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;
      SDB_ASSERT( options._mainCLName, "mainCLName is invalid" ) ;

      optAccessPlan *pPlan = NULL ;
      OPT_PLAN_CACHE_LEVEL cacheLevel = _cacheLevel ;
      UINT16 subCLMBID = mbContext->mbID() ;

      dmsCachedPlanMgr *pCachedPlanMgr = su->getCachedPlanMgr() ;

      if ( NULL == pCachedPlanMgr ||
           pCachedPlanMgr->testMainCLInvalidBitmap( subCLMBID ) )
      {
         // The sub-collection is not validated to use main-collection plans,
         // generate a general plan for it
         rc = _getCLAccessPlan( options, su, mbContext, planRuntime ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get collection access plan for "
                      "query [ %s ], rc: %d", options.toString().c_str(), rc ) ;
      }
      else
      {
         optMainCLAccessPlan *mainPlan = NULL ;

         // Construct the plan key for main-collection
         optAccessPlanKey planKey( options, cacheLevel ) ;
         planKey.setCLFullName( options._mainCLName ) ;
         planKey.setMainCLName( NULL ) ;

         mthMatchHelper matchHelper( cacheLevel, getMatchConfig() ) ;

         rc = _prepareAccessPlanKey( NULL, NULL, planKey, matchHelper,
                                     planRuntime ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to prepare key of access plan, "
                      "rc: %d", rc ) ;

         // Try to get the main-collection plan from cache first
         rc = _getCachedAccessPlan( planKey, &pPlan ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get access plan, rc: %d", rc ) ;

         if ( NULL == pPlan )
         {
            // Could not find the main-collection plan from cache, so create
            // the plan for sub-collection and bind it to the main-collection
            // plan
            rc = _createMainCLPlan( planKey, options, su, mbContext,
                                    planRuntime, matchHelper, &mainPlan ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to create main-collection "
                         "query, rc: %d", rc ) ;

            _cacheAccessPlan( mainPlan ) ;

            // Use the sub-collection plan for the this time
            mainPlan->release() ;
            pPlan = NULL ;
         }
         else
         {
            mainPlan = dynamic_cast<optMainCLAccessPlan *>( pPlan ) ;
            SDB_ASSERT( mainPlan, "mainPlan is invalid " ) ;

            if ( pCachedPlanMgr->testParamInvalidBitmap( subCLMBID ) )
            {
               // The sub-collection is not parameterized validated, we need
               // to verify if it is validate to main-collection plan
               rc = _validateMainCLPlan( mainPlan, options, su, mbContext,
                                         planRuntime ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to validate main-collection "
                            "plan, rc: %d", rc ) ;
            }
            else
            {
               rc = _bindMainCLPlan( mainPlan, options, su, mbContext,
                                     planRuntime, matchHelper ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to bind main-collection "
                            "plan, rc: %d", rc ) ;
            }

            if ( planRuntime.getPlan() != NULL )
            {
               // Already got one plan, release this one
               pPlan->release() ;
            }
            else
            {
               // Set the plan
               planRuntime.setPlan( pPlan, FALSE ) ;
            }
         }
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__GETMAINAP, rc ) ;
      return rc ;

   error :
      if ( NULL != pPlan )
      {
         pPlan->release() ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__PREPAREAPKEY, "_optAccessPlanManager::_prepareAccessPlanKey" )
   INT32 _optAccessPlanManager::_prepareAccessPlanKey ( dmsStorageUnit *su,
                                                        dmsMBContext *mbContext,
                                                        optAccessPlanKey &planKey,
                                                        mthMatchHelper &matchHelper,
                                                        optAccessPlanRuntime &planRuntime )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__PREPAREAPKEY ) ;

#ifdef _DEBUG
      PD_LOG( PDDEBUG, "Original query: [%s] %s", planKey.getCLFullName(),
              planKey._query.toString( FALSE, TRUE ).c_str() ) ;
#endif

      if ( planKey.getCacheLevel() >= OPT_PLAN_NORMALIZED )
      {
         // Normalize the query
         rc = planRuntime.createMatchRuntime() ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create match runtime, rc: %d",
                      rc ) ;

         rc = planKey.normalize( matchHelper, planRuntime.getMatchRuntime() ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to normalize plan key, rc: %d", rc ) ;

         if ( planKey.getCacheLevel() >= OPT_PLAN_NORMALIZED )
         {
#ifdef _DEBUG
            PD_LOG( PDDEBUG, "Normalized query: [%s] %s Params: %s",
                    planKey.getCLFullName(),
                    planKey.getNormalizedQuery().toString( FALSE, TRUE ).c_str(),
                    planRuntime.getParameters().toString().c_str() ) ;
#endif
         }
         else
         {
            // The match runtime is not needed
            planRuntime.deleteMatchRuntime() ;
         }
      }

      planKey.generateKeyCode( su, mbContext ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__PREPAREAPKEY, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__CRTAP, "_optAccessPlanManager::_createAccessPlan" )
   INT32 _optAccessPlanManager::_createAccessPlan ( dmsStorageUnit *su,
                                                    dmsMBContext *mbContext,
                                                    optAccessPlanKey &planKey,
                                                    optAccessPlanRuntime &planRuntime,
                                                    mthMatchHelper &matchHelper,
                                                    optGeneralAccessPlan **ppPlan,
                                                    BOOLEAN needCache )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__CRTAP ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;
      SDB_ASSERT( ppPlan, "ppPlan is invalid" ) ;

      optGeneralAccessPlan *pPlan = NULL ;
      BOOLEAN isParameterized = needCache &&
                                planKey.getCacheLevel() >= OPT_PLAN_PARAMETERIZED ;

      if ( isParameterized )
      {
         pPlan = SDB_OSS_NEW optParamAccessPlan( planKey,
                                                 matchHelper.getMatchConfig() ) ;
      }
      else
      {
         pPlan = SDB_OSS_NEW optGeneralAccessPlan( planKey,
                                                   matchHelper.getMatchConfig() ) ;
      }
      PD_CHECK( NULL != pPlan, SDB_OOM, error, PDERROR,
                "Not able to allocate memory for new plan" ) ;

      rc = pPlan->getKeyOwned() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get key of access plan owned, "
                   "rc: %d", rc ) ;

      if ( NULL == planRuntime.getMatchRuntime() )
      {
         rc = pPlan->createMatchRuntime() ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create match runtime, rc: %d",
                      rc ) ;
         planRuntime.setMatchRuntime( pPlan->getMatchRuntime() ) ;
      }
      else
      {
         pPlan->getMatchRuntimeOnwed( planRuntime ) ;
      }

      rc = pPlan->optimize( su, mbContext, matchHelper ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to optimize plan, query: %s\norder %s\nhint %s",
                   planKey.getQuery().toString().c_str(),
                   planKey.getOrderBy().toString().c_str(),
                   planKey.getHint().toString().c_str() ) ;

      if ( isParameterized )
      {
         // Validate self
         pPlan->validateParameterized( *pPlan ) ;
      }

      // Set the outputs
      (*ppPlan) = pPlan ;

      // Cache the plan
      if ( needCache && isInitialized() )
      {
         _cacheAccessPlan( pPlan ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__CRTAP, rc ) ;
      return rc ;

   error :
      if ( NULL != pPlan )
      {
         pPlan->release() ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__GETCACHEDAP, "_optAccessPlanManager::_getCachedAccessPlan" )
   INT32 _optAccessPlanManager::_getCachedAccessPlan ( const optAccessPlanKey &planKey,
                                                       optAccessPlan **ppPlan )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__GETCACHEDAP ) ;

      SDB_ASSERT( ppPlan, "ppPlan is invalid" ) ;

      optAccessPlan *pPlan = NULL ;
      pPlan = (optAccessPlan *)_planCache.getItem( planKey ) ;
      (*ppPlan) = pPlan ;

      PD_TRACE_EXITRC( SDB_OPTAPM__GETCACHEDAP, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__CACHEAP, "_optAccessPlanManager::_cacheAccessPlan" )
   BOOLEAN _optAccessPlanManager::_cacheAccessPlan ( optAccessPlan *pPlan )
   {
      BOOLEAN cached = FALSE ;

      PD_TRACE_ENTRY( SDB_OPTAPM__CACHEAP ) ;

      // Pre-increase the cached plan count, so the monitor could test if
      // the number of cached plans will exceed the high water mark with this
      // new plan
      _monitor.incCachedPlanCount() ;

      cached = _planCache.addPlan( pPlan ) ;
      if ( cached )
      {
         if ( !_monitor.setActivity( pPlan ) )
         {
            // Could not allocate activity for the plan
            // remove it from cache
            if ( _planCache.removeItem( pPlan ) )
            {
               cached = FALSE ;
               _monitor.decCachedPlanCount( 1 ) ;
            }
         }
         else
         {
            // Re-check if the plan is still cached.
            // If it is not cached after setting the activity, it might be
            // removed by dropCL, so we need to reset the activity if the
            // dropCL didn't
            if ( !pPlan->isCached() )
            {
               INT32 activityID = pPlan->resetActivityID() ;
               if ( OPT_INVALID_ACT_ID != activityID )
               {
                  _monitor.resetActivity( activityID ) ;
               }
               // NOTE: no need to dec cached plan count, The dropCL will do it
               cached = FALSE ;
            }
         }
      }
      else
      {
         _monitor.decCachedPlanCount( 1 ) ;
      }

      PD_TRACE_EXIT( SDB_OPTAPM__CACHEAP ) ;

      return cached ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__VALIDPARAMPLAN, "_optAccessPlanManager::_validateParamPlan" )
   INT32 _optAccessPlanManager::_validateParamPlan ( dmsStorageUnit *su,
                                                     dmsMBContext *mbContext,
                                                     optAccessPlanKey &planKey,
                                                     optAccessPlanRuntime &planRuntime,
                                                     mthMatchHelper &matchHelper,
                                                     optParamAccessPlan *plan )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__VALIDPARAMPLAN ) ;

      SDB_ASSERT ( NULL != plan &&
                   plan->getCacheLevel() >= OPT_PLAN_PARAMETERIZED,
                   "plan is invalid" ) ;

      optGeneralAccessPlan *tempPlan = NULL ;

      // access plan is parameterized validated or has the same original query
      if ( plan->isParamValid() ||
           plan->checkSavedParam( planKey ) )
      {
         rc = planRuntime.bindParamPlan( matchHelper, plan ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to bind parameterized plan, rc: %d",
                      rc ) ;
         goto done ;
      }

      rc = _getCLAccessPlan( planKey, OPT_PLAN_NOCACHE, su, mbContext,
                             planRuntime ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get access plan for with "
                   "query [ %s ], rc: %d", planKey.toString().c_str(), rc ) ;

      tempPlan = dynamic_cast<optGeneralAccessPlan *>( planRuntime.getPlan() ) ;
      SDB_ASSERT( tempPlan, "subPlan is invalid " ) ;

      if ( plan->validateParameterized( *tempPlan ) )
      {
         // Do nothing
      }
      else
      {
         PD_LOG( PDDEBUG, "Invalid parameterized plan" ) ;
         plan->markParamInvalid( mbContext ) ;
         _planCache.removeCachedPlan( plan ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__VALIDPARAMPLAN, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__CRTMAINCLPLAN, "_optAccessPlanManager::_createMainCLPlan" )
   INT32 _optAccessPlanManager::_createMainCLPlan ( optAccessPlanKey &planKey,
                                                    const rtnQueryOptions &subOptions,
                                                    dmsStorageUnit *su,
                                                    dmsMBContext *mbContext,
                                                    optAccessPlanRuntime &planRuntime,
                                                    mthMatchHelper &matchHelper,
                                                    optMainCLAccessPlan **ppPlan )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__CRTMAINCLPLAN ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;
      SDB_ASSERT( ppPlan, "ppPlan is invalid" ) ;

      optMainCLAccessPlan *mainPlan = NULL ;
      optGeneralAccessPlan *subPlan = NULL ;

      mainPlan = SDB_OSS_NEW optMainCLAccessPlan( planKey,
                                                  matchHelper.getMatchConfig() ) ;
      PD_CHECK( mainPlan, SDB_OOM, error, PDERROR,
                "Failed to allocate main-collection access plan" ) ;

      // Get the key owned
      rc = mainPlan->getKeyOwned() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get key of access plan owned, "
                   "rc: %d", rc ) ;

      if ( NULL == planRuntime.getMatchRuntime() )
      {
         rc = mainPlan->createMatchRuntime() ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create match runtime, "
                      "rc: %d", rc ) ;
         planRuntime.setMatchRuntime( mainPlan->getMatchRuntime() ) ;
      }
      else
      {
         mainPlan->getMatchRuntimeOnwed( planRuntime ) ;
      }

      // Prepare to bind the sub-collection plan
      rc = mainPlan->prepareBindSubCL( matchHelper ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to prepare main-collection "
                   "plan, rc: %d", rc ) ;

      // Generate the sub-collection plan
      // Specify the cache level, APM is not allowed to adjust it
      rc = _getCLAccessPlan( subOptions, OPT_PLAN_NOCACHE, su, mbContext,
                             planRuntime ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get access plan for "
                   "sub-collection with query [ %s ], rc: %d",
                   subOptions.toString().c_str(), rc ) ;

      // Bind the sub-collection plan
      subPlan = dynamic_cast<optGeneralAccessPlan *>( planRuntime.getPlan() ) ;
      SDB_ASSERT( subPlan, "subPlan is invalid " ) ;

      rc = mainPlan->bindSubCLAccessPlan( matchHelper, subPlan ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to bind main-collection access "
                   "plan, rc: %d" ) ;

      (*ppPlan) = mainPlan ;

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__CRTMAINCLPLAN, rc ) ;
      return rc ;

   error :
      if ( NULL != mainPlan )
      {
         mainPlan->release() ;
      }
      if ( NULL != subPlan )
      {
         subPlan->release() ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__VALIDMAINCLPLAN, "_optAccessPlanManager::_validateMainCLPlan" )
   INT32 _optAccessPlanManager::_validateMainCLPlan ( optMainCLAccessPlan *mainPlan,
                                                      const rtnQueryOptions &subOptions,
                                                      dmsStorageUnit *su,
                                                      dmsMBContext *mbContext,
                                                      optAccessPlanRuntime &planRuntime )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__VALIDMAINCLPLAN ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;

      // The sub-collection is not parameterized validated, we need
      // to verify if it is validate to main-collection plan
      optGeneralAccessPlan *subPlan = NULL ;
      dmsCachedPlanMgr *pCachedPlanMgr = su->getCachedPlanMgr() ;

      // Specify the cache level, APM is not allowed to adjust it
      rc = _getCLAccessPlan( subOptions, OPT_PLAN_NOCACHE, su, mbContext,
                             planRuntime ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get access plan for "
                   "sub-collection with query [ %s ], rc: %d",
                   subOptions.toString().c_str(), rc ) ;

      subPlan = dynamic_cast<optGeneralAccessPlan *>( planRuntime.getPlan() ) ;
      SDB_ASSERT( subPlan, "subPlan is invalid " ) ;

      if ( !mainPlan->validateSubCL( subPlan ) )
      {
         // The sub-collection is not validate for the main-collection
         // plan, mark it invalidate for main-collection plans
         rc = mainPlan->markMainCLInvalid( pCachedPlanMgr,
                                           mbContext,
                                           FALSE ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to mark sub-collection "
                      "invalidated to reuse main-collection plan, "
                      "rc: %d", rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__VALIDMAINCLPLAN, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__BINDMAINCLPLAN, "_optAccessPlanManager::_bindMainCLPlan" )
   INT32 _optAccessPlanManager::_bindMainCLPlan ( optMainCLAccessPlan *mainPlan,
                                                  const rtnQueryOptions &subOptions,
                                                  dmsStorageUnit *su,
                                                  dmsMBContext *mbContext,
                                                  optAccessPlanRuntime &planRuntime,
                                                  mthMatchHelper &matchHelper )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPM__BINDMAINCLPLAN ) ;

      SDB_ASSERT( mainPlan, "mainPlan is invalid" ) ;
      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;

      dmsCachedPlanMgr *pCachedPlanMgr = su->getCachedPlanMgr() ;
      dmsExtentID indexExtID = DMS_INVALID_EXTENT ;
      dmsExtentID indexLID = DMS_INVALID_EXTENT ;

      // The sub-collection is parameterized validated, we need
      // to verify if it has the index specified by main-colleciton
      // plan, etc
      rc = mainPlan->validateSubCL( su, mbContext, indexExtID, indexLID ) ;
      if ( SDB_OK != rc )
      {
         // Failed to validate sub-collection, generate a general plan
         // for sub-collection ( e.g. missing index )
         rc = mainPlan->markMainCLInvalid( pCachedPlanMgr,
                                           mbContext,
                                           TRUE ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to mark sub-collection "
                      "invalidated to reuse main-collection plan, "
                      "rc: %d", rc ) ;

         // Create a general plan for sub-collection
         rc = _getCLAccessPlan( subOptions, su, mbContext, planRuntime ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get collection access plan for "
                      "query [ %s ], rc: %d", subOptions.toString().c_str(),
                      rc ) ;

         goto done ;
      }

      // Bind plan info ( suID, mbID, etc )
      rc = planRuntime.bindPlanInfo( subOptions._fullName, su, mbContext,
                                     indexExtID, indexLID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to bind plan info, rc: %d",
                   rc ) ;

      // Bind parameters
      if ( mainPlan->getCacheLevel() >= OPT_PLAN_PARAMETERIZED )
      {
         rc = planRuntime.bindParamPlan( matchHelper, mainPlan ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to bind parameterized "
                      "plan, rc: %d", rc ) ;
      }
      else
      {
         // We don't need the match runtime any more
         // Use the one in the plan
         planRuntime.deleteMatchRuntime() ;
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPM__BINDMAINCLPLAN, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPM__INVALIDPLANS, "_optAccessPlanManager::_invalidCachedPlans" )
   void _optAccessPlanManager::_invalidSUPlans ( IDmsSUCacheHolder *pCacheHolder )
   {
      PD_TRACE_ENTRY( SDB_OPTAPM__INVALIDPLANS ) ;

      SDB_ASSERT( pCacheHolder, "pCacheHolder is invalid" ) ;

      dmsCachedPlanMgr *pCachedPlanMgr =
            (dmsCachedPlanMgr *)pCacheHolder->getSUCache( DMS_CACHE_TYPE_PLAN ) ;
      if ( pCachedPlanMgr )
      {
         UINT32 suLID = pCacheHolder->getSULID() ;

         _planCache.invalidateSUPlans( pCachedPlanMgr, suLID ) ;

         // No plans belong to this SU, clear the bitmap and free the units
         pCachedPlanMgr->resetCacheBitmap() ;
         pCachedPlanMgr->resetParamInvalidBitmap() ;
         pCachedPlanMgr->resetMainCLInvalidBitmap() ;
         pCachedPlanMgr->clearCacheUnits() ;
      }
      PD_TRACE_EXIT( SDB_OPTAPM__INVALIDPLANS ) ;
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

         pCachedPlanMgr->clearParamInvalidBit( mbID ) ;
         pCachedPlanMgr->clearMainCLInvalidBit( mbID ) ;
         pCachedPlanMgr->removeCacheUnit( mbID, TRUE ) ;
      }

      PD_TRACE_EXIT( SDB_OPTAPM__INVALIDCLPLANS ) ;
   }

}

