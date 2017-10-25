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

   Source File Name = optPlanPath.cpp

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
          03/14/2017  HGM Initial Draft

   Last Changed =

*******************************************************************************/

#include "optPlanPath.hpp"
#include "pdTrace.hpp"
#include "optTrace.hpp"
#include "msg.hpp"
#include <cmath>

using namespace bson;

namespace engine
{

   /*
      _optPlanNode implement
    */
   _optPlanNode::_optPlanNode ()
   {
      _pLNode = NULL ;
      _pRNode = NULL ;

      _startCost = 0 ;
      _runCost = 0 ;
      _totalCost = 0 ;

      _outputRecords = 0 ;
      _outputRecordSize = 0 ;
      _outputNumFields = 0 ;

      _sorted = FALSE ;
   }

   void _optPlanNode::toBSON ( BSONObjBuilder &builder ) const
   {
      builder.append( OPT_NODE_FIELD_NAME, getName() ) ;
      _toBSON ( builder ) ;

      if ( _pLNode )
      {
         BSONArrayBuilder childBuilder( builder.subarrayStart( OPT_NODE_FIELD_CHILDNODES ) ) ;

         if ( _pLNode )
         {
            BSONObjBuilder subBuilder( childBuilder.subobjStart( 0 ) ) ;
            _pLNode->toBSON( subBuilder ) ;
            subBuilder.done() ;
         }
         else
         {
            childBuilder.appendNull() ;
         }

         if ( _pRNode )
         {
            BSONObjBuilder subBuilder( childBuilder.subobjStart( 1 ) ) ;
            _pRNode->toBSON( subBuilder ) ;
            subBuilder.done() ;
         }

         childBuilder.done() ;
      }
   }

   void _optPlanNode::_toOutputBSON ( BSONObjBuilder &builder ) const
   {
      BSONObjBuilder subBuilder( builder.subobjStart( OPT_NODE_FIELD_OUTPUT ) ) ;

      subBuilder.append( OPT_NODE_FIELD_RECORDS, (INT64)_outputRecords ) ;
      subBuilder.append( OPT_NODE_FIELD_RECORD_SIZE, (INT32)_outputRecordSize ) ;
      subBuilder.append( OPT_NODE_FIELD_SORTED, _sorted ) ;

      subBuilder.done() ;
   }

   void _optPlanNode::_toEstimateBSON ( BSONObjBuilder &builder ) const
   {
      BSONObjBuilder subBuilder( builder.subobjStart( OPT_NODE_FIELD_ESTIMATE ) ) ;

      subBuilder.append( OPT_NODE_FIELD_START_COST,
                         (double)_startCost * OPT_COST_TO_MS ) ;
      subBuilder.append( OPT_NODE_FIELD_RUN_COST,
                         (double)_runCost * OPT_COST_TO_MS ) ;
      subBuilder.append( OPT_NODE_FIELD_TOTAL_COST,
                         (double)_totalCost * OPT_COST_TO_MS ) ;

      subBuilder.done() ;
   }

   /*
      _optScanNode implement
    */
   _optScanNode::_optScanNode ( const CHAR *pCollection,
                                INT32 estCacheSize )
   : _optPlanNode()
   {
      _pCollection = pCollection ;

      _inputRecords = 0 ;
      _inputPages = 0 ;
      _inputRecordSize = 0 ;
      _inputNumFields = 0 ;
      _pageSize = 0 ;

      _mthSelectivity = OPT_MTH_DEFAULT_SELECTIVITY ;
      _mthCPUCost = OPT_MTH_DEFAULT_CPU_COST ;

      _isCandidate = FALSE ;

      _estCacheSize = estCacheSize ;

      _clFromStat = FALSE ;
      _clStatTime = 0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTSCAN__PREEVAL, "_optScanNode::_preEvaluate" )
   void _optScanNode::_preEvaluate ( const BSONObj &selector,
                                     mthMatchHelper &matchHelper,
                                     optCollectionStat *collectionStat )
   {
      PD_TRACE_ENTRY( SDB_OPTSCAN__PREEVAL ) ;

      SDB_ASSERT( collectionStat, "collectionStat is invalid" ) ;

      _inputRecords = OPT_ROUND_NUM_DEF( collectionStat->getTotalRecords(),
                                         DMS_STAT_DEF_TOTAL_RECORDS ) ;
      _inputPages = OPT_ROUND_NUM( collectionStat->getTotalDataPages() ) ;
      _inputRecordSize = OPT_ROUND_NUM(
                  (UINT32)ceil( (double)collectionStat->getTotalDataSize() /
                                (double)_inputRecords ) ) ;

      _pageSize = collectionStat->getPageSize() ;
      _inputNumFields = collectionStat->getAvgNumFields() ;

      matchHelper.getEstimation( collectionStat, _mthSelectivity, _mthCPUCost ) ;

      _outputNumFields = selector.nFields() ;

      if ( collectionStat->isValid() )
      {
         _clFromStat = TRUE ;
         _clStatTime = collectionStat->getCreateTime() ;
      }

      PD_TRACE_EXIT( SDB_OPTSCAN__PREEVAL ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTSCAN_EVALOUTRECSIZE, "_optScanNode::_evalOutRecordSize" )
   void _optScanNode::_evalOutRecordSize ()
   {
      PD_TRACE_ENTRY( SDB_OPTSCAN_EVALOUTRECSIZE ) ;

      if ( _outputNumFields > 0 && _inputNumFields > 0 )
      {
         _outputRecordSize = _outputNumFields * _inputRecordSize / _inputNumFields ;
      }
      else
      {
         _outputNumFields = _inputNumFields ;
         _outputRecordSize = _inputRecordSize ;
      }

      PD_TRACE_EXIT( SDB_OPTSCAN_EVALOUTRECSIZE ) ;
   }

   void _optScanNode::_toInputBSON( BSONObjBuilder &builder ) const
   {
      BSONObjBuilder subBuilder( builder.subobjStart( OPT_NODE_FIELD_INPUT ) ) ;

      subBuilder.append( OPT_NODE_FIELD_PAGES, (INT64)_inputPages ) ;
      subBuilder.append( OPT_NODE_FIELD_RECORDS, (INT64)_inputRecords ) ;
      subBuilder.append( OPT_NODE_FIELD_RECORD_SIZE, (INT32)_inputRecordSize ) ;

      subBuilder.done() ;
   }

   /*
      _optTbScanNode implement
    */
   _optTbScanNode::_optTbScanNode ( const CHAR *pCollection,
                                    INT32 estCacheSize )
   : _optScanNode( pCollection, estCacheSize )
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTTBSCAN_PREEVAL, "_optTbScanNode::preEvaluate" )
   void _optTbScanNode::preEvaluate ( const BSONObj &selector,
                                      mthMatchHelper &matchHelper,
                                      optCollectionStat *collectionStat )
   {
      PD_TRACE_ENTRY( SDB_OPTTBSCAN_PREEVAL ) ;

      SDB_ASSERT( collectionStat, "collectionStat is invalid" ) ;

      _preEvaluate( selector, matchHelper, collectionStat ) ;
      _isCandidate = TRUE ;

      PD_TRACE_EXIT( SDB_OPTTBSCAN_PREEVAL ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTTBSCAN_EVAL, "_optTbScanNode::evaluate" )
   void _optTbScanNode::evaluate ()
   {
      PD_TRACE_ENTRY( SDB_OPTTBSCAN_EVAL ) ;

      UINT64 scanIOCost = 0, scanCPUCost = 0 ;

      if ( _estCacheSize >= 0 &&
           _inputPages > (UINT32)_estCacheSize )
      {
         scanIOCost = OPT_SEQ_SCAN_IO_COST *               // sequence scan cost
                      _inputPages *                        // page num
                      ( _pageSize / DMS_PAGE_SIZE_BASE ) ; // normalize to 4K
      }
      else
      {
         // Ignore IO cost
         scanIOCost = 0 ;
      }

      // Need to extract every records and evaluate matchers
      scanCPUCost = ( OPT_RECORD_CPU_COST + _mthCPUCost ) * _inputRecords ;

      _startCost = OPT_TBLSCAN_DEFAULT_START_COST ;
      _runCost = OPT_IO_CPU_RATE * scanIOCost + scanCPUCost ;
      _totalCost = _startCost + _runCost ;

      _outputRecords = OPT_ROUND_NUM(
                       (UINT64)ceil( (double)_inputRecords * _mthSelectivity ) ) ;

      _evalOutRecordSize() ;

      PD_TRACE_EXIT( SDB_OPTTBSCAN_EVAL ) ;
   }

   void _optTbScanNode::_toBSON ( BSONObjBuilder &builder ) const
   {
      builder.append( OPT_NODE_FIELD_COLLECTION, _pCollection ) ;

      _toInputBSON( builder ) ;
      _toOutputBSON( builder ) ;
      _toEstimateBSON( builder ) ;

      builder.appendBool( OPT_NODE_FIELD_CL_STAT_EST, _clFromStat ) ;
      builder.appendTimestamp( OPT_NODE_FIELD_CL_STAT_TIME, _clStatTime,
                               ( _clStatTime - ( _clStatTime / 1000 * 1000 ) ) * 1000 ) ;
   }

   /*
      _optIxScanNode implement
    */
   _optIxScanNode::_optIxScanNode ( const CHAR *pCollection,
                                    const ixmIndexCB &indexCB,
                                    INT32 estCacheSize )
   : _optScanNode ( pCollection, estCacheSize )
   {
      ossMemset ( _pIndexName, 0, sizeof( _pIndexName ) ) ;

      if ( indexCB.isInitialized() )
      {
         const CHAR *pIndexName = indexCB.getName() ;
         ossStrncpy( _pIndexName, pIndexName, sizeof( _pIndexName ) - 1 ) ;

         _indexExtID = indexCB.getExtentID() ;
         _indexLID = indexCB.getLogicalID() ;
         _keyPattern = indexCB.keyPattern().copy() ;
      }
      else
      {
         _indexExtID = DMS_INVALID_EXTENT ;
         _indexLID = DMS_INVALID_EXTENT ;
      }

      _scanSelectivity = OPT_PRED_DEFAULT_SELECTIVITY ;
      _predSelectivity = OPT_PRED_DEFAULT_SELECTIVITY ;
      _predCPUCost = OPT_PRED_DEFAULT_CPU_COST ;

      _direction = 1 ;
      _matchAll = FALSE ;
      _needMatch = TRUE ;
      _matchedFields = 0 ;
      _matchedOrders = 0 ;

      _indexPages = 0 ;
      _indexLevels = 0 ;

      _idxReadRecords = 0 ;
      _idxOutRecords = 0 ;

      _ixFromStat = FALSE ;
      _ixStatTime = 0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTIXSCAN_PREEVAL, "_optIxScanNode::preEvaluate" )
   void _optIxScanNode::preEvaluate ( const BSONObj &selector,
                                      mthMatchHelper &matchHelper,
                                      const BSONObj &boOrder,
                                      OPT_PLAN_PATH_PRIORITY priority,
                                      optCollectionStat *collectionStat,
                                      optIndexStat *indexStat )
   {
      PD_TRACE_ENTRY( SDB_OPTIXSCAN_PREEVAL ) ;

      SDB_ASSERT( collectionStat, "collectionStat is invalid" ) ;
      SDB_ASSERT( indexStat,"indexStat is invalid" ) ;

      _preEvaluate( selector, matchHelper, collectionStat ) ;
      _indexPages = indexStat->getIndexPages() ;
      _indexLevels = indexStat->getIndexLevels() ;

      BOOLEAN isBestIndex = collectionStat->isBestIndex( indexStat ) ;

      _evalPredEstimation( matchHelper, boOrder, isBestIndex, indexStat ) ;

      switch ( priority )
      {
         case OPT_PLAN_IDX_REQUIRED :
         {
            _isCandidate = TRUE ;
            break ;
         }
         case OPT_PLAN_SORTED_IDX_REQUIRED :
         {
            // Must be sorted index
            if ( _sorted )
            {
               _isCandidate = TRUE ;
            }
            break ;
         }
         case OPT_PLAN_IDX_PREFERRED :
         {
            // Either be sorted or matched predicates
            if ( _sorted || _matchedFields > 0 )
            {
               _isCandidate = TRUE ;
            }
            break ;
         }
         default :
         {
            // Either be sorted or scan selectivity smaller than threshold
            if ( _scanSelectivity <= OPT_PRED_THRESHOLD_SELECTIVITY ||
                 _sorted )
            {
               _isCandidate = TRUE ;
            }
            break;
         }
      }

      if ( indexStat->isValid() )
      {
         _ixFromStat = TRUE ;
         _ixStatTime = indexStat->getCreateTime() ;
      }

      PD_TRACE_EXIT( SDB_OPTIXSCAN_PREEVAL ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTIXSCAN_EVAL, "_optIxScanNode::evaluate" )
   void _optIxScanNode::evaluate ()
   {
      PD_TRACE_ENTRY( SDB_OPTIXSCAN_EVAL ) ;

      UINT64 scanIOCost = 0, scanCPUCost = 0 ;

      // Number of index pages and records will be read ( based on _scanSelectivity )
      UINT32 idxReadPages = OPT_ROUND_NUM(
                            (UINT32)ceil( (double)_indexPages * _scanSelectivity ) ) ;
      _idxReadRecords = OPT_ROUND_NUM(
                        (UINT64)ceil( (double)_inputRecords * _scanSelectivity ) ) ;

      // Number of data pages and records will be read ( based on _predSelectivity )
      // which is also the number of items output from index
      UINT32 dataReadPages = OPT_ROUND_NUM(
                             (UINT32)ceil( (double)_inputPages * _predSelectivity ) ) ;
      _idxOutRecords = OPT_ROUND_NUM(
                       (UINT64)ceil( (double)_inputRecords * _predSelectivity ) ) ;

      if ( _estCacheSize >= 0 && _inputPages > (UINT32)_estCacheSize )
      {
         scanIOCost = OPT_RANDOM_SCAN_IO_COST *             // random scan cost
                      ( idxReadPages + dataReadPages ) *    // number of pages read
                      ( _pageSize / DMS_PAGE_SIZE_BASE ) ;  // normalize to 4k-size
      }
      else
      {
         // Ignore IO cost
         scanIOCost = 0 ;
      }

      // For each index read records, need to be extracted from index page and
      // evaluated against predicates
      // For each index output records, need to be extracted from data page and
      // evaluated against matchers
      scanCPUCost = ( _idxReadRecords *
                     ( OPT_IDX_CPU_COST + _predCPUCost ) ) +
                    ( _idxOutRecords *
                     ( OPT_RECORD_CPU_COST + ( _needMatch ? _mthCPUCost : 0 ) ) ) ;

      _startCost = OPT_IDXSCAN_DEFAULT_START_COST + _predCPUCost * _indexLevels ;
      _runCost = OPT_IO_CPU_RATE * scanIOCost + scanCPUCost ;
      _totalCost = _startCost + _runCost ;

      if ( _predSelectivity < _mthSelectivity )
      {
         _outputRecords = OPT_ROUND_NUM(
                          (UINT64)ceil( (double)_inputRecords * _predSelectivity ) ) ;
      }
      else
      {
         _outputRecords = OPT_ROUND_NUM(
                          (UINT64)ceil( (double)_inputRecords * _mthSelectivity ) ) ;
      }

      _evalOutRecordSize() ;

      PD_TRACE_EXIT( SDB_OPTIXSCAN_EVAL ) ;
   }

   void _optIxScanNode::_toBSON ( BSONObjBuilder &builder ) const
   {
      builder.append( OPT_NODE_FIELD_COLLECTION, _pCollection ) ;
      builder.append( OPT_NODE_FIELD_INDEX, _pIndexName ) ;

      _toInputBSON( builder ) ;
      _toIndexBSON( builder ) ;
      _toOutputBSON( builder ) ;
      _toEstimateBSON( builder ) ;

      builder.appendBool( OPT_NODE_FIELD_CL_STAT_EST, _clFromStat ) ;
      builder.appendTimestamp( OPT_NODE_FIELD_CL_STAT_TIME, _clStatTime,
                               ( _clStatTime - ( _clStatTime / 1000 * 1000 ) ) * 1000 ) ;
      builder.appendBool( OPT_NODE_FIELD_IX_STAT_EST, _ixFromStat ) ;
      builder.appendTimestamp( OPT_NODE_FIELD_IX_STAT_TIME, _ixStatTime,
                               ( _ixStatTime - ( _ixStatTime / 1000 * 1000 ) ) * 1000 ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTIXSCAN_EVALPREDEST, "_optIxScanNode::_evalPredEstimation" )
   void _optIxScanNode::_evalPredEstimation ( mthMatchHelper &matchHelper,
                                              const BSONObj &boOrder,
                                              BOOLEAN isBestIndex,
                                              const optIndexStat *indexStat )
   {
      PD_TRACE_ENTRY( SDB_OPTIXSCAN_EVALPREDEST ) ;

      SDB_ASSERT( matchHelper.getMatchTree(), "matchTree is invalid" ) ;
      SDB_ASSERT( indexStat, "indexStat is invalid" ) ;

      UINT32 iterIdx = 0,
             matchedFields = 0,
             matchedOrders = 0 ;
      BOOLEAN startIncluded = TRUE, stopIncluded = TRUE ;
      const BSONObj &keyPattern = indexStat->getKeyPattern() ;
      UINT32 keyNum = (UINT32)keyPattern.nFields() ;

      rtnStatPredList predicateList ;
      BOOLEAN needMatchOrder = TRUE ;
      INT32 direction = 1 ;

      double predSelectivity = 1.0 ;
      double scanSelectivity = 1.0 ;

      BOOLEAN isEqual = TRUE ;
      const CHAR *pFirstField = NULL ;

      // The statistics of index are invalid, we need to evaluate each predicate
      // in the predicate set
      BOOLEAN fieldOnly = !indexStat->isValid() ;

      mthMatchTree *matcher = matchHelper.getMatchTree() ;
      RTN_PREDICATE_MAP &predicates = matchHelper.getPredicates() ;

      if ( !matchHelper.isEstimated() )
      {
         // The matcher has not been estimated, which could not be used
         // to evaluate predicates
         isBestIndex = FALSE ;
      }

      BSONObjIterator iterKey( keyPattern ) ;
      BSONObjIterator iterOrder( boOrder ) ;

      while ( iterKey.more() )
      {
         BSONElement beKey = iterKey.next() ;
         const CHAR *pFieldName = beKey.fieldName() ;

         // Set the first name
         if ( !pFirstField )
         {
            pFirstField = pFieldName ;
         }

         // Try to match order first
         if ( needMatchOrder && iterOrder.more() )
         {
            BSONElement beOrder = iterOrder.next() ;
            if ( 0 == ossStrcmp ( pFieldName, beOrder.fieldName() ) )
            {
               BOOLEAN orderMatched = ( ( ( beKey.number() * direction ) > 0 ) ==
                                          ( beOrder.number() > 0 ) ) ;
               if ( matchedOrders == 0 )
               {
                  direction = orderMatched ? 1 : -1 ;
                  matchedOrders ++ ;
               }
               else if ( orderMatched )
               {
                  matchedOrders ++ ;
               }
               else
               {
                  needMatchOrder = FALSE ;
               }
            }
            else
            {
               needMatchOrder = FALSE ;
            }
         }

         if ( predicates.empty() )
         {
            iterIdx ++ ;
            continue ;
         }

         RTN_PREDICATE_MAP::iterator iterPred = predicates.find( pFieldName ) ;

         if ( iterPred == predicates.end() ||
              iterPred->second.isEmpty() )
         {
            // The key is not included in the predicates
            // Cover all values in this key
            if ( !fieldOnly && !isBestIndex )
            {
               predicateList.push_back( NULL ) ;
               isEqual = FALSE ;
            }
         }
         else
         {
            rtnPredicate &curPredicate = iterPred->second ;

            if ( fieldOnly )
            {
               // Evaluate the predicate for this field only
               BOOLEAN curIsAllRange = FALSE ;
               double curSelectivity =  indexStat->evalPredicate(
                        pFieldName, curPredicate, matchHelper.mthEnabledMixCmp(),
                        curIsAllRange ) ;

               predSelectivity *= curSelectivity ;
               if ( iterIdx == 0 )
               {
                  scanSelectivity = curSelectivity ;
               }
            }
            else if ( !isBestIndex )
            {
               // Need to evaluate the whole predicate set together, add the
               // predicates into list, and evaluate them later
               predicateList.push_back( &curPredicate ) ;

               isEqual &= curPredicate.isEquality() ;
               startIncluded &= curPredicate.minInclusive() ;
               stopIncluded &= curPredicate.maxInclusive() ;
            }

            matchedFields ++ ;
         }

         iterIdx ++ ;
      }

      // Matched key in index
      if ( matchedFields > 0 )
      {
         if ( fieldOnly )
         {
            // Do nothing
            // scanSelectivity is estimated by the first field
         }
         else if ( isBestIndex )
         {
            // The best index has been evaluated and cached in matcher
            matchHelper.getPredSelectivity( predSelectivity, scanSelectivity ) ;
         }
         else
         {
            // The predicates contain multiple start stop key-pairs, evaluate
            // each of them
            predSelectivity = indexStat->evalPredicateList(
                  pFirstField, predicateList, matchHelper.mthEnabledMixCmp(),
                  scanSelectivity ) ;
         }
      }

      if ( !boOrder.isEmpty() )
      {
         // Set the scan direction
         _direction = direction ;
         if ( matchedOrders == (UINT32)boOrder.nFields() )
         {
            _sorted = TRUE ;
         }
      }

      // we try to set matchall only when all fields converted into predicates
      if ( matcher->totallyConverted() )
      {
         _matchAll = ( 0 == predicates.size() ) ||
                     ( ( 0 != matchedFields ) &&
                       ( matchedFields == predicates.size() ) &&
                         matchedFields <= keyNum ) ;
      }

      if ( matcher->isInitialized() && _matchAll &&
           !matcher->hasExpand() && !matcher->hasReturnMatch() )
      {
         _needMatch = FALSE ;
      }

      _matchedFields = matchedFields ;
      _matchedOrders = matchedOrders ;

      _predSelectivity = OPT_ROUND_SELECTIVITY( predSelectivity ) ;
      _scanSelectivity = OPT_ROUND_SELECTIVITY( scanSelectivity ) ;
      _predCPUCost = OPT_MTH_OPTR_BASE_CPU_COST * keyNum ;

      PD_TRACE_EXIT( SDB_OPTIXSCAN_EVALPREDEST ) ;
   }

   void _optIxScanNode::_toIndexBSON ( BSONObjBuilder &builder ) const
   {
      BSONObjBuilder subBuilder( builder.subobjStart( OPT_NODE_FIELD_INDEX_FILTER ) ) ;

      subBuilder.append( OPT_NODE_FIELD_READ_RECORDS, (INT64)_idxReadRecords ) ;
      subBuilder.append( OPT_NODE_FIELD_OUTPUT_RECORDS, (INT32)_idxOutRecords ) ;

      subBuilder.done() ;
   }

   /*
      _optSortNode implement
    */
   _optSortNode::_optSortNode ()
   : _optPlanNode()
   {
      _sortType = OPT_PLAN_IN_MEM_SORT ;
      _sortBufferSize = 0 ;
      _numOrders = 0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTSORT_EVAL, "_optSortNode::evaluate" )
   void _optSortNode::evaluate ( const BSONObj &boOrder, UINT64 sortBufferSize )
   {
      PD_TRACE_ENTRY( SDB_OPTSORT_EVAL ) ;

      SDB_ASSERT( _pLNode, "Child node should not be NULL" ) ;

      _numOrders = boOrder.nFields() ;
      _sortBufferSize = sortBufferSize ;

      if ( _pLNode )
      {
         UINT64 comparisionCPUCost = 2 * OPT_OPTR_BASE_CPU_COST * _numOrders ;
         UINT64 inputSize = _pLNode->getOutputRecords() * _pLNode->getOutputRecordSize() ;
         double inputRecords = (double)( OSS_MAX( 2, _pLNode->getOutputRecords() ) ) ;

         UINT64 sortCPUCost = (UINT64)( (double)comparisionCPUCost *
                              inputRecords * OPT_LOG2( inputRecords ) ) ;
         UINT64 sortIOCost = 0 ;

         // Check input size against sort buffer size
         if ( inputSize > _sortBufferSize )
         {
            // Use external merge-sort
            _sortType = OPT_PLAN_EXT_SORT ;
            UINT32 pages = OPT_ROUND_NUM( inputSize / DMS_PAGE_SIZE_BASE ) ;
            sortIOCost = (UINT64)pages * ( (double) OPT_SEQ_SCAN_IO_COST * 2.0 +
                                           (double) OPT_SEQ_SCAN_IO_COST * 0.75 +
                                           (double) OPT_RANDOM_SCAN_IO_COST * 0.25 ) ;
         }
         else
         {
            // Use in-memory sort
            _sortType = OPT_PLAN_IN_MEM_SORT ;
            sortIOCost = 0 ;
         }

         _startCost = _pLNode->getTotalCost() +
                      OPT_IO_CPU_RATE * sortIOCost +
                      sortCPUCost ;

         _runCost = OPT_OPTR_BASE_CPU_COST * _pLNode->getOutputRecords() ;
         _totalCost = _startCost + _runCost ;

         _outputRecordSize = _pLNode->getOutputRecordSize() ;
         _outputRecords = _pLNode->getOutputRecords() ;
         _outputNumFields = _pLNode->getOutputNumFields() ;
         _sorted = TRUE ;
      }

      PD_TRACE_EXIT( SDB_OPTSORT_EVAL ) ;
   }

   void _optSortNode::_toBSON ( BSONObjBuilder &builder ) const
   {
      builder.append( OPT_NODE_FIELD_SORT_TYPE,
                      ( _sortType == OPT_PLAN_IN_MEM_SORT ?
                                     OPT_NODE_FIELD_SORT_IN_MEM :
                                     OPT_NODE_FIELD_SORT_EXTERNAL ) ) ;

      _toOutputBSON( builder ) ;
      _toEstimateBSON( builder ) ;
   }

   /*
      _optReturnNode implement
    */
   _optReturnNode::_optReturnNode ()
   : _optPlanNode()
   {
      _numToSkip = 0 ;
      _numToReturn = -1 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTRETURN_EVAL, "_optReturnNode::evaluate" )
   void _optReturnNode::evaluate ( SINT64 numToSkip, SINT64 numToReturn )
   {
      PD_TRACE_ENTRY( SDB_OPTRETURN_EVAL ) ;

      SDB_ASSERT( _pLNode, "Child node should not be NULL" ) ;

      _numToSkip = OSS_MAX( numToSkip, 0 ) ;
      _numToReturn = numToReturn ;

      if ( _pLNode )
      {
         UINT64 avgRecordCost = 0 ;
         UINT64 skipCost = 0 ;

         if ( _pLNode->getOutputRecords() > 0 )
         {
            avgRecordCost = (UINT64)( (double)_pLNode->getRunCost() /
                                      (double)_pLNode->getOutputRecords() ) ;
         }

         if ( _numToReturn < 0 )
         {
            _outputRecords = _pLNode->getOutputRecords() - _numToSkip ;
         }
         else
         {
            _outputRecords = _numToReturn ;
         }

         skipCost = avgRecordCost * _numToSkip ;
         _startCost = _pLNode->getStartCost() + skipCost ;
         _runCost = _pLNode->getRunCost() - skipCost ;
         _totalCost = _startCost + _runCost ;

         _outputRecordSize = _pLNode->getOutputRecordSize() ;
         _outputNumFields = _pLNode->getOutputNumFields() ;
         _sorted = _pLNode->isSorted() ;
      }

      PD_TRACE_EXIT( SDB_OPTRETURN_EVAL ) ;
   }

   void _optReturnNode::_toBSON ( BSONObjBuilder &builder ) const
   {
      _toOutputBSON( builder ) ;
      _toEstimateBSON( builder ) ;
   }

   /*
      _optPlanPath implement
    */
   _optPlanPath::_optPlanPath ( optPlanAllocator *pAllocator )
   {
      _pRootNode = NULL ;
      _pAllocator = pAllocator ;
   }

   _optPlanPath::~_optPlanPath ()
   {
      _deleteNodes() ;
   }

   void _optPlanPath::_deleteNodes ()
   {
      if ( _pRootNode )
      {
         _deleteChildNodes( _pRootNode ) ;
         _pRootNode->release( _pAllocator ) ;
      }
      _pRootNode = NULL ;
   }

   void _optPlanPath::_deleteChildNodes ( optPlanNode *pNode )
   {
      if ( pNode )
      {
         optPlanNode *pLNode = pNode->getLNode() ;
         if ( pLNode )
         {
            _deleteChildNodes( pLNode ) ;
            pLNode->release( _pAllocator ) ;
            pNode->setLNode( NULL ) ;
         }
         optPlanNode *pRNode = pNode->getRNode() ;
         if ( pRNode )
         {
            _deleteChildNodes( pRNode ) ;
            pRNode->release( _pAllocator ) ;
            pNode->setRNode( NULL ) ;
         }
      }
   }

   void _optPlanPath::_setRootNode ( optPlanNode *pNode )
   {
      if ( pNode )
      {
         pNode->setLNode( _pRootNode ) ;
         _pRootNode = pNode ;
      }
   }

   void _optPlanPath::_joinPath ( optPlanNode *pJoinNode, optPlanPath &path )
   {
      if ( pJoinNode )
      {
         _setRootNode( pJoinNode ) ;
         pJoinNode->setRNode( path._pRootNode ) ;
         path._pRootNode = NULL ;
      }
   }

   void _optPlanPath::_swap( optPlanPath &path )
   {
      {
         optPlanNode *pTmpNode = _pRootNode ;
         _pRootNode = path._pRootNode ;
         path._pRootNode = pTmpNode ;
      }

      {
         optPlanAllocator *pTmpAllocator = _pAllocator ;
         _pAllocator = path._pAllocator ;
         path._pAllocator = pTmpAllocator ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTPATH_CRTSORT, "_optPlanPath::_createSortNode" )
   INT32 _optPlanPath::_createSortNode ( const BSONObj &boOrder,
                                         UINT64 sortBufferSize )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTPATH_CRTSORT ) ;

      optSortNode *pSort = NULL ;

      if ( boOrder.isEmpty() || !_pRootNode )
      {
         goto done ;
      }

      pSort = new ( _pAllocator, std::nothrow ) optSortNode() ;
      PD_CHECK( pSort, SDB_OOM, error, PDWARNING,
                "Failed to allocate optSortNode" ) ;

      _setRootNode( pSort ) ;

      pSort->evaluate( boOrder, sortBufferSize ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTPATH_CRTSORT, rc ) ;
      return rc ;
   error :
      SAFE_OSS_DELETE( pSort ) ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTPATH_CRTRETURN, "_optPlanPath::_createReturnNode" )
   INT32 _optPlanPath::_createReturnNode ( SINT64 numToSkip,
                                           SINT64 numToReturn )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTPATH_CRTRETURN ) ;

      optReturnNode *pReturn = NULL ;

      if ( ( 0 == numToSkip && -1 == numToReturn ) ||
           !_pRootNode )
      {
         goto done ;
      }

      pReturn = new ( _pAllocator, std::nothrow ) optReturnNode() ;
      PD_CHECK( pReturn, SDB_OOM, error, PDWARNING,
                "Failed to allocate optReturnNode" ) ;

      _setRootNode( pReturn ) ;

      pReturn->evaluate( numToSkip, numToReturn ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTPATH_CRTRETURN, rc ) ;
      return rc ;
   error :
      SAFE_OSS_DELETE( pReturn ) ;
      goto done ;
   }

   /*
      _optScanPath implement
    */
   _optScanPath::_optScanPath ( optPlanAllocator *pAllocator )
   : _optPlanPath( pAllocator )
   {
      _pScanNode = NULL ;
      _sortRequired = FALSE ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTSCANPATH_CRTTBSCAN, "_optPlanPath::createTbScan" )
   INT32 _optScanPath::createTbScan ( const CHAR *pCollection,
                                      const BSONObj &selector,
                                      mthMatchHelper &matchHelper,
                                      INT32 estCacheSize,
                                      optCollectionStat *collectionStat )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTSCANPATH_CRTTBSCAN ) ;

      SDB_ASSERT( pCollection, "pCollection is invalid" ) ;
      SDB_ASSERT( collectionStat, "collectionStat is invalid" ) ;

      optTbScanNode *pTbScan = NULL ;

      if ( _pScanNode )
      {
         _deleteNodes() ;
         _pScanNode = NULL ;
      }

      pTbScan = new ( _pAllocator, std::nothrow )
                      optTbScanNode( pCollection, estCacheSize ) ;
      PD_CHECK( pTbScan, SDB_OOM, error, PDWARNING,
                "Failed to allocate optTbScanNode" ) ;

      _setRootNode( pTbScan ) ;
      _pScanNode = pTbScan ;
      _sortRequired = FALSE ;

      pTbScan->preEvaluate( selector, matchHelper, collectionStat ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTSCANPATH_CRTTBSCAN, rc ) ;
      return rc ;
   error :
      SAFE_OSS_DELETE( pTbScan ) ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTSCANPATH_CRTIXSCAN, "_optPlanPath::createIxScan" )
   INT32 _optScanPath::createIxScan ( const CHAR *pCollection,
                                      const ixmIndexCB &indexCB,
                                      const BSONObj &selector,
                                      mthMatchHelper &matchHelper,
                                      const BSONObj &boOrder,
                                      OPT_PLAN_PATH_PRIORITY priority,
                                      INT32 estCacheSize,
                                      optCollectionStat *collectionStat,
                                      optIndexStat *indexStat )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTSCANPATH_CRTIXSCAN ) ;

      SDB_ASSERT( pCollection, "pCollection is invalid" ) ;
      SDB_ASSERT( collectionStat, "collectionStat is invalid" ) ;
      SDB_ASSERT( indexStat, "indexStat is invalid" ) ;

      optIxScanNode *pIdxScan = NULL ;

      if ( _pScanNode )
      {
         _deleteNodes() ;
         _pScanNode = NULL ;
      }

      pIdxScan = new ( _pAllocator, std::nothrow )
                       optIxScanNode( pCollection, indexCB, estCacheSize ) ;
      PD_CHECK( pIdxScan, SDB_OOM, error, PDWARNING,
                "Failed to allocate optIxScanNode" ) ;

      _setRootNode( pIdxScan ) ;
      _pScanNode = pIdxScan ;
      _sortRequired = FALSE ;

      pIdxScan->preEvaluate( selector, matchHelper, boOrder, priority,
                             collectionStat, indexStat ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTSCANPATH_CRTIXSCAN, rc ) ;
      return rc ;
   error :
      SAFE_OSS_DELETE( pIdxScan ) ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTSCANPATH_EVAL, "_optScanPath::evaluate" )
   INT32 _optScanPath::evaluate ( const BSONObj &boOrder,
                                  SINT64 numToSkip,
                                  SINT64 numToReturn,
                                  UINT64 sortBufferSize )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTSCANPATH_EVAL ) ;

      if ( !_pScanNode || !_pScanNode->isCandidate() )
      {
         goto done ;
      }

      _pScanNode->evaluate() ;
      _sortRequired = FALSE ;

      if ( !boOrder.isEmpty() && !_pScanNode->isSorted() )
      {
         rc = _createSortNode( boOrder, sortBufferSize ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to evaluate sort, rc: %d", rc ) ;
         _sortRequired = TRUE ;
      }

      if ( numToSkip > 0 || -1 != numToReturn )
      {
         rc = _createReturnNode( numToSkip, numToReturn ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to evaluate skip/limit, rc: %d", rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTSCANPATH_EVAL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   void _optScanPath::swap( optScanPath &path )
   {
      _optPlanPath::_swap( path ) ;

      {
         optScanNode *pTmpNode = _pScanNode ;
         _pScanNode = path._pScanNode ;
         path._pScanNode = pTmpNode ;
      }

      {
         BOOLEAN tmp = _sortRequired ;
         _sortRequired = path._sortRequired ;
         path._sortRequired = tmp ;
      }
   }

}
