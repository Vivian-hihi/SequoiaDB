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

   Source File Name = optAPM.hpp

   Descriptive Name = Optimizer Access Plan Manager Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Access
   Plan Manager, which is pooling access plans that has been used.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft
          01/07/2017  HGM Move from rtnAPM.hpp

   Last Changed =

*******************************************************************************/
#ifndef OPTAPM_HPP__
#define OPTAPM_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "optAccessPlan.hpp"
#include "optAccessPlanRuntime.hpp"
#include "utilHashTable.hpp"
#include "dmsCachedPlanUnit.hpp"
#include "dmsEventHandler.hpp"

using namespace std ;

namespace engine
{

   #define OPT_PLAN_CACHE_DFT_LOCK_NUM    ( 64 )
   #define OPT_PLAN_CACHE_ACT_HIGH_PERC   ( 0.80 )
   #define OPT_PLAN_CACHE_ACT_LOW_PERC    ( 0.50 )
   #define OPT_PLAN_CACHE_AVG_BUCKET_SIZE ( 3 )
   #define OPT_PLAN_CACHE_UINT64_LIMIT    ( 0xFFFFFFFF00000000uLL )

   class _optCachedPlanMonitor ;
   typedef class _optCachedPlanMonitor optCachedPlanMonitor ;

   /*
      _optAccessPlanCache define
    */
   class _optAccessPlanCache : public _utilHashTable< optAccessPlanKey,
                                                      optAccessPlan,
                                                      OPT_PLAN_CACHE_DFT_LOCK_NUM >
   {
      public :
         _optAccessPlanCache () ;

         virtual ~_optAccessPlanCache () {}

         BOOLEAN initialize ( UINT16 bucketNum,
                              optCachedPlanMonitor *pMonitor ) ;

         void clear () ;

         BOOLEAN addPlan ( optAccessPlan *pPlan ) ;

         void removeCachedPlan ( optAccessPlan *pPlan ) ;

         void invalidateSUPlans ( dmsCachedPlanMgr *pCachedPlanMgr,
                                  UINT32 suLID ) ;

         void invalidateCLPlans ( dmsCachedPlanMgr *pCachedPlanMgr,
                                  UINT32 suLID, UINT32 clLID ) ;

         void invalidateAllPlans () ;

         void invalidateCLPlans ( const CHAR *pCLFullName ) ;

         UINT32 getCachedPlanCount () const ;

      protected :
         virtual void afterAddItem ( UINT32 bucketID, optAccessPlan *pPlan ) ;

         virtual void afterGetItem ( UINT32 bucketID, optAccessPlan *pPlan ) ;

      protected :
         optCachedPlanMonitor *_pMonitor ;
   } ;

   typedef class _optAccessPlanCache optAccessPlanCache ;

   /*
      _optCachedPlanActivity define
    */
   class _optCachedPlanActivity : public SDBObject
   {
      public :
         _optCachedPlanActivity ()
         {
            _pPlan = NULL ;
            _accessTime = 0 ;
            _accessCount = 0 ;
         }

         virtual ~_optCachedPlanActivity () {} ;

         OSS_INLINE void setPlan ( optAccessPlan *pPlan, UINT64 timestamp )
         {
            _pPlan = pPlan ;
            _accessTime = timestamp ;
            _accessCount = 0 ;
         }

         OSS_INLINE optAccessPlan *getPlan ()
         {
            return _pPlan ;
         }

         OSS_INLINE void setAccessTime ( UINT64 accessTime )
         {
            _accessTime = accessTime ;
         }

         OSS_INLINE void incAccessCount ()
         {
            _accessCount ++ ;
         }

         OSS_INLINE void decAccessCount ( UINT32 count )
         {
            if ( _accessCount > count )
            {
               _accessCount -= count ;
            }
            else
            {
               _accessCount = 0 ;
            }
         }

         OSS_INLINE void clear ()
         {
            _accessTime = 0 ;
            _accessCount = 0 ;
            _pPlan = NULL ;
         }

         OSS_INLINE BOOLEAN isEmpty () const
         {
            return ( NULL == _pPlan ) ;
         }

         OSS_INLINE UINT64 getAccessTime () const
         {
            return _accessTime ;
         }

         OSS_INLINE UINT32 getAccessCount () const
         {
            return _accessCount ;
         }

      protected :
         optAccessPlan *   _pPlan ;
         UINT64            _accessTime ;
         UINT32            _accessCount ;
   } ;

   typedef class _optCachedPlanActivity optCachedPlanActivity ;

   /*
      _optCachedPlanMonitor define
    */
   class _optCachedPlanMonitor : public SDBObject
   {
      public :
         _optCachedPlanMonitor () ;

         virtual ~_optCachedPlanMonitor () ;

         BOOLEAN initialize ( optAccessPlanCache *pPlanCache ) ;

         void clear () ;

         OSS_INLINE BOOLEAN isInitialized () const
         {
            return ( NULL != _pActivities ) ;
         }

         BOOLEAN setActivity ( optAccessPlan *pPlan ) ;

         OSS_INLINE void setCachedPlanActivity ( optAccessPlan *pPlan )
         {
            INT32 activityID = pPlan->getActivityID() ;
            if ( OPT_INVALID_ACT_ID != activityID )
            {
               optCachedPlanActivity &activity = _pActivities[ activityID ] ;
               activity.setAccessTime( _accessTimestamp.inc() ) ;
               activity.incAccessCount() ;
            }
         }

         OSS_INLINE void resetActivity ( UINT32 activityID )
         {
            _pActivities[ activityID ].setPlan( NULL, 0 ) ;

            UINT64 freeActivityIndex = _freeIndexEnd.inc() % _activityNum ;
            _pFreeActivityIDs[ freeActivityIndex ] = activityID ;
         }

         OSS_INLINE UINT32 getCachedPlanCount () const
         {
            return _cachedPlanCount.peek() ;
         }

         OSS_INLINE void incCachedPlanCount ()
         {
            _cachedPlanCount.inc() ;
         }

         OSS_INLINE void decCachedPlanCount ( UINT32 count = 1 )
         {
            _cachedPlanCount.sub( count ) ;
         }

         OSS_INLINE ossRWMutex *getClearLock ()
         {
            return &_clearLock ;
         }

         OSS_INLINE ossEvent *getClearEvent ()
         {
            return &_clearEvent ;
         }

         void signalPlanClearJob () ;

         void clearCachedPlans () ;

         void checkFreeIndexes () ;

         void checkAccessTimestamp () ;

      protected :
         INT32 _allocateActivity ( optAccessPlan *pPlan,
                                   BOOLEAN criticalMode ) ;

      protected :
         // Begin to the free index, where to get free activities
         ossAtomic64 _freeIndexBegin ;

         // End to the free index, where to return free activities
         ossAtomic64 _freeIndexEnd ;

         // Free index
         UINT32 *_pFreeActivityIDs ;

         // Thread flag to clearing procedure
         ossAtomic32 _clearThread ;

         // Thread flag to allocate activity during clearing procedure
         ossAtomic32 _allocateThread ;

         // Mutex to protect clearing procedure
         ossRWMutex _clearLock ;

         // Clear event to signal clear job
         ossEvent _clearEvent ;

         // Number of activities ( The capacity of plan cache )
         UINT32 _activityNum ;

         // High water mark to clear cached plans
         UINT32 _highWaterMark ;

         // Low water mark to stop clear cached plans
         UINT32 _lowWaterMark ;

         // Clock index to scan the activity table
         UINT32 _clockIndex ;

         // Activity table
         optCachedPlanActivity *_pActivities ;

         // Total number of cached plans
         ossAtomic32 _cachedPlanCount ;

         // Access timestamp for cached plans
         ossAtomic64 _accessTimestamp ;

         // Last timestamp to finished clearing procedure
         UINT64 _lastClearTimestamp ;

         // Pointer to plan cache
         optAccessPlanCache *_pPlanCache ;
   } ;

   /*
      _optAccessPlanManager define
    */
   class _optAccessPlanManager : public SDBObject,
                                 public _IDmsEventHandler,
                                 public _mthMatchConfigHolder
   {
      public :
         _optAccessPlanManager () ;

         virtual ~_optAccessPlanManager () ;

         INT32 init ( UINT32 bucketNum,
                      OPT_PLAN_CACHE_LEVEL cacheLevel,
                      BOOLEAN enableMixCmp ) ;

         INT32 reinit ( OPT_PLAN_CACHE_LEVEL cacheLevel,
                        BOOLEAN enableMixCmp ) ;

         void clear () ;

         OSS_INLINE OPT_PLAN_CACHE_LEVEL getCacheLevel () const
         {
            return _cacheLevel ;
         }

         OSS_INLINE BOOLEAN isInitialized () const
         {
            return _planCache.isInitialized() ;
         }

         OSS_INLINE optAccessPlanCache *getPlanCache ()
         {
            return &_planCache ;
         }

         OSS_INLINE optCachedPlanMonitor *getPlanMonitor ()
         {
            return &_monitor ;
         }

         // Try to get access plan from cache, if could not get access plan
         // from cache, create one
         INT32 getAccessPlan ( const rtnQueryOptions &options,
                               dmsStorageUnit *su,
                               dmsMBContext *mbContext,
                               optAccessPlanRuntime &planRuntime ) ;

         // Create access plan directly without caching
         INT32 getTempAccessPlan ( const rtnQueryOptions &options,
                                   dmsStorageUnit *su,
                                   dmsMBContext *mbContext,
                                   optAccessPlanRuntime &planRuntime ) ;

         void invalidateCLPlans ( const CHAR *pCLFullName ) ;

         void invalidateAllPlans () ;

      public :
         // For _IDmsEventHandler
         virtual INT32 onUnloadCS ( IDmsEventHolder *pEventHolder,
                                    IDmsSUCacheHolder *pCacheHolder,
                                    pmdEDUCB *cb,
                                    SDB_DPSCB *dpsCB ) ;

         virtual INT32 onRenameCS ( IDmsEventHolder *pEventHolder,
                                    IDmsSUCacheHolder *pCacheHolder,
                                    const CHAR *pOldCSName,
                                    const CHAR *pNewCSName,
                                    pmdEDUCB *cb,
                                    SDB_DPSCB *dpsCB ) ;

         virtual INT32 onDropCS ( IDmsEventHolder *pEventHolder,
                                  IDmsSUCacheHolder *pCacheHolder,
                                  pmdEDUCB *cb,
                                  SDB_DPSCB *dpsCB ) ;

         virtual INT32 onRenameCL ( IDmsEventHolder *pEventHolder,
                                    IDmsSUCacheHolder *pCacheHolder,
                                    const dmsEventCLItem &clItem,
                                    const CHAR *pNewCLName,
                                    pmdEDUCB *cb,
                                    SDB_DPSCB *dpsCB ) ;

         virtual INT32 onTruncateCL ( IDmsEventHolder *pEventHolder,
                                      IDmsSUCacheHolder *pCacheHolder,
                                      const dmsEventCLItem &clItem,
                                      UINT32 newCLLID,
                                      pmdEDUCB *cb,
                                      SDB_DPSCB *dpsCB ) ;

         virtual INT32 onDropCL ( IDmsEventHolder *pEventHolder,
                                  IDmsSUCacheHolder *pCacheHolder,
                                  const dmsEventCLItem &clItem,
                                  pmdEDUCB *cb,
                                  SDB_DPSCB *dpsCB ) ;

         virtual INT32 onCreateIndex ( IDmsEventHolder *pEventHolder,
                                       IDmsSUCacheHolder *pCacheHolder,
                                       const dmsEventCLItem &clItem,
                                       const dmsEventIdxItem &idxItem,
                                       pmdEDUCB *cb,
                                       SDB_DPSCB *dpsCB ) ;

         virtual INT32 onDropIndex ( IDmsEventHolder *pEventHolder,
                                     IDmsSUCacheHolder *pCacheHolder,
                                     const dmsEventCLItem &clItem,
                                     const dmsEventIdxItem &idxItem,
                                     pmdEDUCB *cb,
                                     SDB_DPSCB *dpsCB ) ;

         virtual INT32 onClearSUCaches ( IDmsEventHolder *pEventHolder,
                                         IDmsSUCacheHolder *pCacheHolder ) ;

         virtual INT32 onClearCLCaches ( IDmsEventHolder *pEventHolder,
                                         IDmsSUCacheHolder *pCacheHolder,
                                         const dmsEventCLItem &clItem ) ;

         OSS_INLINE virtual UINT32 getMask ()
         {
            return DMS_EVENT_MASK_PLAN ;
         }

      protected :
         INT32 _getCLAccessPlan ( const rtnQueryOptions &options,
                                  dmsStorageUnit *su,
                                  dmsMBContext *mbContext,
                                  optAccessPlanRuntime &planRuntime ) ;

         INT32 _getCLAccessPlan ( const rtnQueryOptions &options,
                                  OPT_PLAN_CACHE_LEVEL cacheLevel,
                                  dmsStorageUnit *su,
                                  dmsMBContext *mbContext,
                                  optAccessPlanRuntime &planRuntime ) ;

         INT32 _getMainCLAccessPlan ( const rtnQueryOptions &options,
                                      dmsStorageUnit *su,
                                      dmsMBContext *mbContext,
                                      optAccessPlanRuntime &planRuntime ) ;

         INT32 _prepareAccessPlanKey ( dmsStorageUnit *su,
                                       dmsMBContext *mbContext,
                                       optAccessPlanKey &planKey,
                                       mthMatchHelper &matchHelper,
                                       optAccessPlanRuntime &planRuntime ) ;

         INT32 _createAccessPlan ( dmsStorageUnit *su,
                                   dmsMBContext *mbContext,
                                   optAccessPlanKey &planKey,
                                   optAccessPlanRuntime &planRuntime,
                                   mthMatchHelper &matchHelper,
                                   optGeneralAccessPlan **ppPlan,
                                   BOOLEAN needCache ) ;

         INT32 _getCachedAccessPlan ( const optAccessPlanKey &planKey,
                                      optAccessPlan **ppPlan ) ;

         BOOLEAN _cacheAccessPlan ( optAccessPlan *pPlan ) ;

         // Helpers for parameterized plans
         INT32 _validateParamPlan ( dmsStorageUnit *su,
                                    dmsMBContext *mbContext,
                                    optAccessPlanKey &planKey,
                                    optAccessPlanRuntime &planRuntime,
                                    mthMatchHelper &matchHelper,
                                    optParamAccessPlan *plan ) ;

         // Helpers for main-collection plans
         INT32 _createMainCLPlan ( optAccessPlanKey &planKey,
                                   const rtnQueryOptions &subOptions,
                                   dmsStorageUnit *su,
                                   dmsMBContext *mbContext,
                                   optAccessPlanRuntime &planRuntime,
                                   mthMatchHelper &matchHelper,
                                   optMainCLAccessPlan **ppPlan ) ;

         INT32 _validateMainCLPlan ( optMainCLAccessPlan *mainPlan,
                                     const rtnQueryOptions &subOptions,
                                     dmsStorageUnit *su,
                                     dmsMBContext *mbContext,
                                     optAccessPlanRuntime &planRuntime ) ;

         INT32 _bindMainCLPlan ( optMainCLAccessPlan *mainPlan,
                                 const rtnQueryOptions &subOptions,
                                 dmsStorageUnit *su,
                                 dmsMBContext *mbContext,
                                 optAccessPlanRuntime &planRuntime,
                                 mthMatchHelper &matchHelper ) ;

         // Helpers for _IDmsEventHandler
         void _invalidSUPlans ( IDmsSUCacheHolder *pCacheHolder ) ;

         void _invalidCLPlans ( IDmsSUCacheHolder *pCacheHolder,
                                UINT16 mbID, UINT32 clLID ) ;

      protected :
         ossSpinXLatch           _reinitLatch ;
         optAccessPlanCache      _planCache ;
         optCachedPlanMonitor    _monitor ;
         EDUID                   _clearJobEduID ;

         // Configured options
         UINT32                  _cacheBucketNum ;
         OPT_PLAN_CACHE_LEVEL    _cacheLevel ;
   } ;

   typedef _optAccessPlanManager optAccessPlanManager ;
}

#endif //OPTAPM_HPP__

