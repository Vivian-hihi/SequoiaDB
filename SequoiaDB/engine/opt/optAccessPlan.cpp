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

   Source File Name = optAccessPlan.cpp

   Descriptive Name = Optimizer Access Plan

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains functions for optimizer
   access plan creation. It will calculate based on rules and try to estimate
   a lowest cost plan to access data.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "optAccessPlan.hpp"
#include "../bson/ordering.h"
#include "rtn.hpp"
#include "ixm.hpp"
#include "optAPM.hpp"
#include "optStatUnit.hpp"
#include "pdTrace.hpp"
#include "optTrace.hpp"
#include "pmd.hpp"

using namespace bson;
namespace engine
{

   /*
      _optAccessPlan implement
    */
   _optAccessPlan::_optAccessPlan ( const optAccessPlanKey &planKey,
                                    BOOLEAN needGetOwned )
   : _utilHashTableItem(),
     _key( planKey, needGetOwned ),
     _scanPath( &_planAllocator ),
     _activityID( OPT_INVALID_ACT_ID ),
     _refCount( 0 )
   {
      _isInitialized = FALSE ;
      _hintFailed = FALSE ;
      _predList = NULL ;
      _isAutoPlan = FALSE ;
      _autoHint = FALSE ;

      _useCount = 0 ;

      _cachedPlanMgr = NULL ;
   }

   INT32 _optAccessPlan::_checkOrderBy ()
   {
      INT32 rc = SDB_OK ;
      BSONObjIterator iter( _key._orderBy ) ;
      while ( iter.more() )
      {
         BSONElement ele = iter.next() ;
         INT32 value ;
         if ( ossStrcasecmp( ele.fieldName(), "" ) == 0 )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "orderBy's fieldName can't be empty:rc=%d", rc ) ;
            goto error ;
         }

         if ( !ele.isNumber() )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "orderBy's value must be numberic:rc=%d", rc ) ;
            goto error ;
         }

         value = ele.numberInt() ;
         if ( value != 1 && value != -1 )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "orderBy's value must be 1 or -1:rc=%d", rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN__ESTIXPLAN_NAME, "_optAccessPlan::_estimateIxScanPlan" )
   INT32 _optAccessPlan::_estimateIxScanPlan ( dmsStorageUnit *su,
                                               dmsMBContext *mbContext,
                                               optCollectionStat &collectionStat,
                                               const CHAR *pIndexName,
                                               OPT_PLAN_PATH_PRIORITY priority,
                                               UINT64 sortBufferSize,
                                               INT32 estCacheSize,
                                               optScanPath &ixScanPath )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__OPTACCPLAN__ESTIXPLAN_NAME ) ;

      dmsExtentID indexCBExtent = DMS_INVALID_EXTENT ;

      // Search by index name
      rc = su->index()->getIndexCBExtent( mbContext, pIndexName,
                                          indexCBExtent ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to get index extent ID from "
                   "collection [%s], index [%s], rc: %d", _key._fullName,
                   pIndexName, rc ) ;

      rc = _estimateIxScanPlan( su, collectionStat, indexCBExtent,
                                priority, sortBufferSize, estCacheSize,
                                ixScanPath ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to estimate ixscan plan for "
                   "collection [%s], index: [%s], rc: %d", _key._fullName,
                   pIndexName, rc ) ;

   done :
      PD_TRACE_EXITRC( SDB__OPTACCPLAN__ESTIXPLAN_NAME, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN__ESTIXPLAN_OID, "_optAccessPlan::_estimateIxScanPlan" )
   INT32 _optAccessPlan::_estimateIxScanPlan ( dmsStorageUnit *su,
                                               dmsMBContext *mbContext,
                                               optCollectionStat &collectionStat,
                                               const OID &indexOID,
                                               OPT_PLAN_PATH_PRIORITY priority,
                                               UINT64 sortBufferSize,
                                               INT32 estCacheSize,
                                               optScanPath &ixScanPath )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__OPTACCPLAN__ESTIXPLAN_OID ) ;

      dmsExtentID indexCBExtent = DMS_INVALID_EXTENT ;

      // Search by OID
      rc = su->index()->getIndexCBExtent( mbContext, indexOID, indexCBExtent ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to get index extent ID from "
                   "collection [%s], index [%s], rc: %d", _key._fullName,
                   indexOID.toString().c_str(), rc ) ;

      rc = _estimateIxScanPlan( su, collectionStat, indexCBExtent,
                                priority, sortBufferSize, estCacheSize,
                                ixScanPath ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to estimate ixscan plan for "
                   "collection [%s], index: [%s], rc: %d", _key._fullName,
                   indexOID.toString().c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB__OPTACCPLAN__ESTIXPLAN_OID, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN__ESTIXPLAN, "_optAccessPlan::_estimateIxScanPlan" )
   INT32 _optAccessPlan::_estimateIxScanPlan ( dmsStorageUnit *su,
                                               optCollectionStat &collectionStat,
                                               dmsExtentID indexCBExtent,
                                               OPT_PLAN_PATH_PRIORITY priority,
                                               UINT64 sortBufferSize,
                                               INT32 estCacheSize,
                                               optScanPath &ixScanPath )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__OPTACCPLAN__ESTIXPLAN ) ;

      ixmIndexCB indexCB ( indexCBExtent, su->index(), NULL ) ;

      PD_CHECK( indexCB.isInitialized(), SDB_DMS_INIT_INDEX, error, PDWARNING,
                "Index [%d] in collection [%s] is invalid", indexCBExtent,
                _key._fullName ) ;

      PD_CHECK( indexCB.getFlag() == IXM_INDEX_FLAG_NORMAL,
                SDB_IXM_UNEXPECTED_STATUS, error, PDDEBUG,
                "Index is not normal status, skip" ) ;

      try
      {
         optIndexStat indexStat( collectionStat, indexCB ) ;

         rc = ixScanPath.createIxScan( _key._fullName, indexCB, _key._selector,
                                       _matcher, _key._orderBy, priority,
                                       estCacheSize, collectionStat,
                                       indexStat ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to create index scan node, rc: %d", rc ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR,
                 "Failed to estimate index scan, received unexpected error:%s",
                 e.what() );
         rc = SDB_INVALIDARG;
         goto error;
      }

      if ( ixScanPath.isCandidate() )
      {
         ixScanPath.evaluate( _key._orderBy, _key._skip, _key._limit,
                              sortBufferSize ) ;
      }

   done :
       PD_TRACE_EXITRC( SDB__OPTACCPLAN__ESTIXPLAN, rc ) ;
      return rc ;
   error :
      ixScanPath.clearPath() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN__ESTTBPLAN, "_optAccessPlan::_estimateTbScanPlan" )
   INT32 _optAccessPlan::_estimateTbScanPlan ( optCollectionStat &collectionStat,
                                               UINT64 sortBufferSize,
                                               INT32 estCacheSize,
                                               optScanPath &tbScanPath )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__OPTACCPLAN__ESTTBPLAN ) ;

      rc = tbScanPath.createTbScan( _key._fullName, _key._selector, _matcher,
                                    estCacheSize, collectionStat ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to create index scan node, rc: %d", rc ) ;

      tbScanPath.evaluate( _key._orderBy, _key._skip, _key._limit,
                           sortBufferSize ) ;

   done :
       PD_TRACE_EXITRC( SDB__OPTACCPLAN__ESTTBPLAN, rc ) ;
      return rc ;
   error :
      tbScanPath.clearPath() ;
      goto done ;
   }


   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN_ESTHINTPLANS, "_optAccessPlan::_estimateHintPlans" )
   INT32 _optAccessPlan::_estimateHintPlans ( dmsStorageUnit *su,
                                              dmsMBContext *mbContext,
                                              dmsStatCache *statCache )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__OPTACCPLAN_ESTHINTPLANS ) ;

      UINT64 sortBufferSize = pmdGetOptionCB()->getSortBufSize() * 1024 * 1024 ;
      INT32 estCacheSize = pmdGetOptionCB()->getOptEstCacheSize() ;

      optCollectionStat collectionStat( su->getPageSize(), mbContext, statCache ) ;

      UINT64 bestEstimateCost = OSS_UINT64_MAX ;
      optScanPath bestPath( &_planAllocator ) ;

      BOOLEAN sortedIdxRequired =
            ( !_key._orderBy.isEmpty() ) &&
            ( OSS_BIT_TEST( _key._flag, FLG_QUERY_MODIFY ) ||
              OSS_BIT_TEST( _key._flag, FLG_QUERY_FORCE_IDX_BY_SORT ) ) ;

      UINT32 validHints = 0 ;
      BSONObjIterator iter( _key._hint ) ;

      PD_LOG ( PDDEBUG, "Hint is provided: %s", _key._hint.toString().c_str() ) ;

      rc = SDB_RTN_INVALID_HINT ;

      // user can define more than one index name/oid in hint, it will pickup
      // the first valid one
      while ( iter.more() )
      {
         BSONElement hint = iter.next() ;
         switch ( hint.type() )
         {
            case String :
            {
               const CHAR *pIndexName = hint.valuestr() ;
               if ( '\0' != *( pIndexName ) )
               {
                  optScanPath ixScanPath( &_planAllocator ) ;

                  OPT_PLAN_PATH_PRIORITY priority = sortedIdxRequired ?
                                                    OPT_PLAN_SORTED_IDX_REQUIRED :
                                                    OPT_PLAN_IDX_REQUIRED ;

                  PD_LOG ( PDDEBUG, "Try to use index: %s", pIndexName ) ;

                  rc = _estimateIxScanPlan( su, mbContext, collectionStat,
                                            pIndexName, priority, sortBufferSize,
                                            estCacheSize, ixScanPath ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDWARNING, "Failed to estimate index scan for "
                             "collection [%s], index [%s], rc: %d",
                             _key._fullName, pIndexName, rc ) ;
                     break ;
                  }

                  validHints ++ ;

                  if ( ixScanPath.isCandidate() &&
                       ixScanPath.getTotalCost() < bestEstimateCost )
                  {
                     bestEstimateCost = ixScanPath.getTotalCost() ;
                     bestPath.swap( ixScanPath ) ;
                  }
               }
               else if ( bestPath.isEmpty() )
               {
                  // First { "" : "" } goto auto hint
                  _autoHint = TRUE ;
                  rc = SDB_RTN_INVALID_HINT ;
                  goto error ;
               }
               break ;
            }
            case jstOID :
            {
               const OID &indexOID = hint.__oid() ;
               optScanPath ixScanPath( &_planAllocator ) ;

               OPT_PLAN_PATH_PRIORITY priority = sortedIdxRequired ?
                                                 OPT_PLAN_SORTED_IDX_REQUIRED :
                                                 OPT_PLAN_IDX_REQUIRED ;

               PD_LOG ( PDDEBUG, "Try to use index: %s",
                        indexOID.toString().c_str() ) ;

               rc = _estimateIxScanPlan( su, mbContext, collectionStat,
                                         indexOID, priority, sortBufferSize,
                                         estCacheSize, ixScanPath ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDWARNING, "Failed to estimate index scan for "
                          "collection [%s], index OID [%s], rc: %d",
                          _key._fullName, indexOID.toString().c_str(), rc ) ;
                  break ;
               }

               validHints ++ ;

               if ( ixScanPath.isCandidate() &&
                    ixScanPath.getTotalCost() < bestEstimateCost )
               {
                  bestEstimateCost = ixScanPath.getTotalCost() ;
                  bestPath.swap( ixScanPath ) ;
               }
               break ;
            }
            case jstNULL :
            {
               optScanPath tbScanPath( &_planAllocator ) ;

               PD_LOG ( PDDEBUG, "Use Collection Scan by Hint" ) ;

               validHints ++ ;

               // if we use null in the hint, we use tbscan
               rc = _estimateTbScanPlan( collectionStat, sortBufferSize,
                                         estCacheSize, tbScanPath ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDWARNING, "Failed to estimate table scan for "
                          "collection [%s], rc: %d", _key._fullName, rc ) ;
                  break ;
               }

               if ( tbScanPath.getTotalCost() < bestEstimateCost )
               {
                  bestEstimateCost = tbScanPath.getTotalCost() ;
                  bestPath.swap( tbScanPath ) ;
               }

               break ;
            }
            case Object :
            case Array :
            {
               break ;
            }
            default :
            {
               PD_LOG( PDWARNING, "Unknown hint type %d", hint.type() ) ;
               break ;
            }
         }
      }

      if ( sortedIdxRequired && validHints > 0 )
      {
         // Report the sort required earlier
         PD_CHECK ( DMS_INVALID_EXTENT != bestPath.getIndexExtID(),
                    SDB_RTN_QUERYMODIFY_SORT_NO_IDX, error, PDWARNING,
                    "when query and modify, sorting must use index" ) ;
      }

      if ( bestPath.isEmpty() )
      {
         PD_LOG( PDWARNING, "Failed to estimate hint plans" ) ;
         rc = SDB_RTN_INVALID_HINT ;
         goto error ;
      }

      rc = _usePath( su, bestPath ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to use hint path, rc: %d", rc ) ;

   done :
      PD_TRACE_EXITRC( SDB__OPTACCPLAN_ESTHINTPLANS, rc ) ;
      return rc ;
   error :
      _hintFailed = TRUE ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN__ESTPLANS, "_optAccessPlan::_estimatePlans" )
   INT32 _optAccessPlan::_estimatePlans ( dmsStorageUnit *su,
                                          dmsMBContext *mbContext,
                                          dmsStatCache *statCache )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__OPTACCPLAN__ESTPLANS ) ;

      const rtnPredicateSet &predicateSet = _matcher.getPredicateSet() ;
      UINT64 sortBufferSize = pmdGetOptionCB()->getSortBufSize() * 1024 * 1024 ;
      INT32 estCacheSize = pmdGetOptionCB()->getOptEstCacheSize() ;

      UINT64 bestEstimateCost = OSS_UINT64_MAX ;
      dmsExtentID bestIdxExtID = DMS_INVALID_EXTENT ;

      optScanPath tbScanPath( &_planAllocator ), bestPath( &_planAllocator ) ;

      optCollectionStat collectionStat( su->getPageSize(), mbContext,
                                        statCache ) ;
      UINT32 candidateCount = 0 ;

      optScanType scanType = UNKNOWNSCAN ;
      OPT_PLAN_PATH_PRIORITY priority = OPT_PLAN_DEFAULT_PRIORITY ;

      if ( !_key._orderBy.isEmpty() &&
           ( OSS_BIT_TEST( _key._flag, FLG_QUERY_MODIFY ) ||
             OSS_BIT_TEST( _key._flag, FLG_QUERY_FORCE_IDX_BY_SORT ) ) )
      {
         // Have order-by and with query flags to required sorted index
         priority = OPT_PLAN_SORTED_IDX_REQUIRED ;
      }
      else if ( _autoHint && _hintFailed &&
                ( predicateSet.getSize() > 0 || !_key._orderBy.isEmpty() ) )
      {
         // Have hints, predicates or order-by, which could prefer index-scan
         if ( OSS_BIT_TEST( _key._flag, FLG_QUERY_FORCE_HINT ) )
         {
            // Must use index
            if ( !_key._orderBy.isEmpty() )
            {
               priority = OPT_PLAN_SORTED_IDX_REQUIRED ;
            }
            else
            {
               priority = OPT_PLAN_IDX_REQUIRED ;
            }
         }
         else
         {
            // Index preferred
            priority = OPT_PLAN_IDX_PREFERRED ;
         }
      }

      if ( priority == OPT_PLAN_IDX_PREFERRED ||
           priority == OPT_PLAN_DEFAULT_PRIORITY )
      {
         // Estimate table scan
         rc = _estimateTbScanPlan( collectionStat, sortBufferSize,
                                   estCacheSize, tbScanPath ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to estimate table scan for "
                      "collection [%s], rc: %d", _key._fullName, rc ) ;
      }

      if ( predicateSet.getSize() > 0 ||
           !_key._orderBy.isEmpty() )
      {
         // We had found a best index earlier, check it first
         if ( collectionStat.getBestIndexName() )
         {
            const CHAR *pIndexName = collectionStat.getBestIndexName() ;
            optScanPath ixScanPath( &_planAllocator ) ;

            rc = _estimateIxScanPlan( su, mbContext, collectionStat, pIndexName,
                                      priority, sortBufferSize, estCacheSize,
                                      ixScanPath ) ;

            if ( SDB_OK != rc )
            {
               PD_LOG( PDWARNING, "Failed to estimate index scan for "
                       "collection [%s], index [%s], rc: %d", _key._fullName,
                       pIndexName, rc ) ;
            }
            else
            {
               bestIdxExtID = ixScanPath.getIndexExtID() ;
               if ( ixScanPath.isCandidate() &&
                    ixScanPath.getTotalCost() < bestEstimateCost )
               {
                  bestEstimateCost = ixScanPath.getTotalCost() ;
                  bestPath.swap( ixScanPath ) ;
                  candidateCount ++ ;
               }
            }
         }

         // Go through indexes to find candidate plans
         for ( INT32 idx = 0 ; idx < DMS_COLLECTION_MAX_INDEX ; idx ++ )
         {
            dmsExtentID indexCBExtent = DMS_INVALID_EXTENT ;
            optScanPath ixScanPath( &_planAllocator ) ;

            rc = su->index()->getIndexCBExtent( mbContext, idx,
                                                indexCBExtent ) ;
            if ( SDB_IXM_NOTEXIST == rc )
            {
               rc = SDB_OK ;
               break ;
            }
            if ( SDB_OK != rc )
            {
               // Continue to evaluate the rest of indexes
               PD_LOG( PDWARNING, "Failed to get index extent ID from "
                       "collection [%s], index [%d], rc: %d", _key._fullName,
                       idx, rc ) ;
               continue ;
            }
            if ( bestIdxExtID == indexCBExtent )
            {
               // Already evaluated
               continue ;
            }

            rc = _estimateIxScanPlan( su, collectionStat, indexCBExtent,
                                      priority, sortBufferSize, estCacheSize,
                                      ixScanPath ) ;
            if ( SDB_OK != rc )
            {
               // Continue to evaluate the rest of indexes
               PD_LOG( PDWARNING, "Failed to estimate index scan for "
                       "collection [%s], index [%d], rc: %d", _key._fullName,
                       idx, rc ) ;
               continue ;
            }
            if ( ixScanPath.isCandidate() &&
                 ixScanPath.getTotalCost() < bestEstimateCost )
            {
               bestEstimateCost = ixScanPath.getTotalCost() ;
               bestPath.swap( ixScanPath ) ;
               candidateCount ++ ;

               // Needn't to evaluate all indexes in below cases:
               // 1. got enough candidate plans
               if ( candidateCount >= OPT_MAX_CANDIDATE_COUNT )
               {
                  break ;
               }
            }
         }
      }

      // Check if sortedIdx is required
      if ( OPT_PLAN_SORTED_IDX_REQUIRED == priority )
      {
         PD_CHECK( DMS_INVALID_EXTENT != bestPath.getIndexExtID(),
                   SDB_RTN_QUERYMODIFY_SORT_NO_IDX, error, PDWARNING,
                   "Failed to estimate plans: when query and modify, sorting "
                    "must use index" ) ;
      }
      else if ( OPT_PLAN_IDX_REQUIRED == priority )
      {
         PD_CHECK( DMS_INVALID_EXTENT != bestPath.getIndexExtID(),
                   SDB_RTN_INVALID_HINT, error, PDWARNING,
                   "Failed to estimate plans: when hint is forced, must use "
                   "index" ) ;
      }

      // Check the cost of tbScanPath after ixScanPath, so the ixScanPath
      // have higher priority when they have the same costs
      if ( ( ( OPT_PLAN_IDX_PREFERRED == priority &&
               DMS_INVALID_EXTENT == bestPath.getIndexExtID() ) ||
             OPT_PLAN_DEFAULT_PRIORITY == priority ) &&
           tbScanPath.getTotalCost() < bestEstimateCost )
      {
         bestEstimateCost = tbScanPath.getTotalCost() ;
         bestPath.swap( tbScanPath ) ;
      }

      scanType = bestPath.getScanType() ;
      rc = _usePath( su, bestPath ) ;
      if ( SDB_OK != rc && IXSCAN == scanType )
      {
         PD_LOG( PDWARNING, "Failed to use index scan, rc: %d", rc ) ;
         if ( OPT_PLAN_SORTED_IDX_REQUIRED == priority )
         {
            rc = SDB_RTN_QUERYMODIFY_SORT_NO_IDX ;
         }
         else if ( OPT_PLAN_IDX_REQUIRED == priority )
         {
            rc = SDB_RTN_INVALID_HINT ;
         }
         else
         {
            if ( tbScanPath.isEmpty() )
            {
               PD_LOG( PDWARNING, "TblScan is not estimated" ) ;
            }
            rc = _usePath( su, tbScanPath ) ;
         }
      }
      PD_RC_CHECK( rc, PDWARNING, "Failed to use scan path, rc: %d", rc ) ;

      PD_LOG( PDDEBUG, "Optimizer: Use plan %s", _scanPath.toString().c_str() ) ;

   done :
      PD_TRACE_EXITRC( SDB__OPTACCPLAN__ESTPLANS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN__USEPATH, "_optAccessPlan::_usePath" )
   INT32 _optAccessPlan::_usePath ( dmsStorageUnit *su,
                                    optScanPath &path )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__OPTACCPLAN__USEPATH ) ;

      dmsExtentID idxExtID = path.getIndexExtID() ;
      optScanPath emptyPath( &_planAllocator ) ;

      // Clear earlier settings
      SAFE_OSS_DELETE( _predList ) ;
      _scanPath.swap( emptyPath ) ;

      PD_CHECK( !path.isEmpty(), SDB_SYS, error, PDWARNING,
                "Try to use unknown path" ) ;

      if ( DMS_INVALID_EXTENT != idxExtID )
      {
         // Check the index
         ixmIndexCB indexCB ( idxExtID, su->index(), NULL ) ;
         PD_CHECK( indexCB.isInitialized(), SDB_DMS_INIT_INDEX, error, PDWARNING,
                   "Failed to use index at extent %d", idxExtID ) ;

         // Create predicate list
         _predList = SDB_OSS_NEW rtnPredicateList ( _matcher.getPredicateSet(),
                                                    &indexCB,
                                                    path.getDirection() ) ;
         PD_CHECK( _predList, SDB_OOM, error, PDWARNING,
                   "Failed to allocate memory for rtnPredicateList" ) ;

         // Note: below processing should not be failed

         // Set if we need further match
         if ( !_matcher.isMatchesAll() && path.isMatchAll() )
         {
            _matcher.setMatchesAll( TRUE ) ;
         }
      }

      _scanPath.swap( path ) ;

   done :
      PD_TRACE_EXITRC( SDB__OPTACCPLAN__USEPATH, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN__PREPAREOPT, "_optAccessPlan::_prepareOptimize" )
   INT32 _optAccessPlan::_prepareOptimize ( dmsStorageUnit *su,
                                            dmsMBContext *mbContext )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__OPTACCPLAN__PREPAREOPT ) ;

      dmsStatCache *statCache = NULL ;

      BOOLEAN needCacheStat = FALSE, needCachedPlan = FALSE ;

      // Check if statistics need to be loaded
      statCache = su->getStatCache() ;
      if ( NULL != statCache &&
           UTIL_SU_CACHE_UNIT_STATUS_EMPTY == statCache->getStatus( _key._mbID ) )
      {
         needCacheStat = TRUE ;
      }

      // Check if cached plan status need to be added
      _cachedPlanMgr = su->getCachedPlanMgr() ;
      if ( NULL != _cachedPlanMgr &&
           UTIL_SU_CACHE_UNIT_STATUS_EMPTY ==
                 _cachedPlanMgr->getStatus( _key._mbID ) )
      {
         needCachedPlan = TRUE ;
      }

      if ( needCacheStat || needCachedPlan )
      {
         if ( SDB_OK == mbContext->mbLock( EXCLUSIVE ) )
         {
            // NOTE: should not goto error
            if ( needCacheStat )
            {
               // Reload statistics
               pmdEDUCB *cb = pmdGetThreadEDUCB() ;
               _SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
               rtnReloadCLStats ( su, mbContext, cb, dmsCB ) ;
            }

            if ( needCachedPlan )
            {
               // Create new cached plan status for this collection
               dmsCLCachedPlanUnit *pCachedPlanUnit =
                     SDB_OSS_NEW dmsCLCachedPlanUnit( _key._mbID, 0 ) ;
               if ( NULL != pCachedPlanUnit )
               {
                  _cachedPlanMgr->addCacheUnit( pCachedPlanUnit, TRUE, FALSE ) ;
               }
            }
         }

         rc = mbContext->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Lock dms mb context SHARED failed, "
                      "rc: %d", rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB__OPTACCPLAN__PREPAREOPT, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__OPTACCPLAN_OPT, "_optAccessPlan::optimize" )
   INT32 _optAccessPlan::optimize ( dmsStorageUnit *su,
                                    dmsMBContext *mbContext )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( SDB__OPTACCPLAN_OPT ) ;

      SDB_ASSERT( su, "su is invalid" ) ;
      SDB_ASSERT( mbContext, "mbContext is invalid" ) ;

      dmsStatCache *statCache = su->getStatCache() ;

      BOOLEAN mbLocked = FALSE ;

      rc = _checkOrderBy() ;
      PD_RC_CHECK( rc, PDERROR, "failed to check orderby", rc ) ;

      // First let's build matcher
      rc = _matcher.loadPattern ( _key._query ) ;
      PD_RC_CHECK ( rc, (SDB_RTN_INVALID_PREDICATES==rc) ? PDINFO : PDERROR,
                    "Failed to load query, rc = %d", rc ) ;

      // Lock the mbContext, then we could access the statistics informations
      if ( !mbContext->isMBLock() )
      {
         rc = mbContext->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to lock mbContext [%s], rc: %d",
                      _key._fullName, rc ) ;
         mbLocked = TRUE ;
      }

      rc = _prepareOptimize( su, mbContext ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to prepare for optimize, rc: %d", rc ) ;

      if ( _key._hint.isEmpty() )
      {
         rc = _estimatePlans( su, mbContext, statCache ) ;
      }
      else
      {
         // Evaluate hints first
         rc = _estimateHintPlans( su, mbContext, statCache ) ;
         if ( SDB_OK != rc &&
              SDB_RTN_QUERYMODIFY_SORT_NO_IDX != rc )
         {
            // Hint failed, could evaluate with all candidate plans again
            // Without sorted index should be reported
            rc = _estimatePlans( su, mbContext, statCache ) ;
         }
      }

      PD_RC_CHECK( rc, PDERROR, "Failed to create candidate plans, rc: %d", rc ) ;

      _isInitialized = TRUE ;
      _key.setValid( TRUE ) ;

      // Clear hint failed
      if ( _autoHint && _hintFailed && IXSCAN == getScanType() )
      {
         _hintFailed = FALSE ;
      }

      rc = SDB_OK ;

   done :
      if ( mbLocked )
      {
         mbContext->mbUnlock() ;
      }
      PD_TRACE_EXITRC ( SDB__OPTACCPLAN_OPT, rc );
      return rc ;

   error :
      goto done ;
   }

   void _optAccessPlan::release()
   {
      // If the plan is cached, decrease the reference count
      // If the plan is not cached, only delete the plan when it has only one
      // reference
      if ( isCached() )
      {
         decRefCount() ;
      }
      else if ( decRefCount() == 1 )
      {
         SDB_ASSERT( getRefCount() == 0, "Invalid ref count" ) ;
         SDB_OSS_DEL this ;
      }
   }

   std::string _optAccessPlan::toString() const
   {
      stringstream ss ;
      ss << "CollectionName:" << _key._fullName
         << ",IndexName:" << _scanPath.getIndexName()
         << ",OrderBy:" << _key._orderBy.toString().c_str()
         << ",Query:" << _key._query.toString().c_str()
         << ",Hint:" << _key._hint.toString().c_str()
         << ",HintFailed:" << _hintFailed
         << ",Direction:" << _scanPath.getDirection()
         << ",ScanType:" << ( TBSCAN == _scanPath.getScanType() ? "TBSCAN" : "IXSCAN" )
         << ",Valid:" << _key._isValid
         << ",AutoPlan:" << _isAutoPlan
         << ",HashValue:" << _key._keyCode
         << ",Count:" << getRefCount()
         << ",SortRequired:" << _scanPath.isSortRequired()
         << ",AutoHint:" << _autoHint ;
      return ss.str() ;
   }

}

