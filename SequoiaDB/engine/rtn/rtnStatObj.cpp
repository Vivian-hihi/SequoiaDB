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

   Source File Name = rtnStatObj.cpp

   Descriptive Name = Runtime Statistics Object Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Statistics
   Objects.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "rtnStatObj.hpp"
#include "dmsStorageUnit.hpp"
#include "catCommon.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "pmd.hpp"
#include "ixm.hpp"
#include "ixmExtent.hpp"
#include "optCommon.hpp"

namespace engine
{

   /*
      _rtnStatBase implement
    */
   double _rtnStatBase::evalPredicate ( const CHAR *pFieldName,
                                        rtnPredicate &predicate,
                                        BOOLEAN &isAllRange ) const
   {
      double selectivity = 1.0 ;

      if ( predicate.isEvaluated() )
      {
         isAllRange = predicate.isAllRange() ;
         selectivity = predicate.getSelectivity() ;
      }
      else
      {
         selectivity = 0.0 ;

         for ( vector<rtnStartStopKey>::iterator iterSSKey = predicate._startStopKeys.begin();
               iterSSKey != predicate._startStopKeys.end() ;
               iterSSKey ++ )
         {
            const rtnKeyBoundary &startKey = iterSSKey->_startKey ;
            const rtnKeyBoundary &stopKey = iterSSKey->_stopKey ;

            dmsStatElementKey beStart( startKey._bound ) ;
            dmsStatElementKey beStop( stopKey._bound ) ;

            if ( beStart.type() == MinKey && beStop.type() == MaxKey )
            {
               isAllRange = TRUE ;
               break ;
            }

            BOOLEAN subIsEqual = iterSSKey->isEquality() ;

            double subSelectivity = 1.0 ;

            subSelectivity = evalStartStopKeys( pFieldName,
                                                beStart, startKey._inclusive,
                                                beStop, stopKey._inclusive,
                                                subIsEqual ) ;
            selectivity += subSelectivity ;
         }

         if ( isAllRange )
         {
            selectivity = 1.0 ;
         }
         else
         {
            selectivity = OPT_ROUND_SELECTIVITY( selectivity ) ;
         }

         // Cache the selectivity to avoid duplicated evaluations
         predicate.setSelectivity( selectivity, isAllRange ) ;
      }

      return selectivity ;
   }

   /*
      _rtnIndexStat implement
    */
   _rtnIndexStat::_rtnIndexStat ( const rtnCollectionStat &collectionStat,
                                  const ixmIndexCB &indexCB )
   : _rtnStatBase( collectionStat.getCollectionName(),
                   collectionStat.getTotalRecords() ),
     _collectionStat( collectionStat )
   {
      _pIndexName = indexCB.getName() ;
      _pIndexStat = collectionStat.getIndexStat( _pIndexName ) ;
      _keyPattern = indexCB.keyPattern().copy() ;

      if ( _pIndexStat && _pIndexStat->getTotalRecords() > 0 )
      {
         double rate = (double)_totalRecords / (double)_pIndexStat->getTotalRecords() ;
         _indexPages = (UINT32)( (double)_pIndexStat->getIndexPages() * rate ) ;
      }
      else
      {
         _indexPages = collectionStat.getAvgIndexPages() ;
      }
   }

   void _rtnIndexStat::evalPredicateList ( const CHAR *pFirstFieldName,
                                           rtnStatPredList &predList,
                                           double &selectivity ) const
   {
      INT32 rc = SDB_INVALIDARG ;

      if ( predList.size() == 0 )
      {
         selectivity = 1.0 ;
      }
      else if ( isValid() )
      {
         rtnStatPredList::iterator predIter = predList.begin() ;
         dmsStatListKey startKeys, stopKeys ;
         rc = _evalStartStopKeys( predIter,
                                  startKeys, TRUE, stopKeys, TRUE,
                                  TRUE, selectivity ) ;
      }

      if ( SDB_OK != rc )
      {
         rtnPredicate *pPredicate = predList.front() ;

         if ( pPredicate )
         {
            BOOLEAN isAllRange = FALSE ;
            selectivity = evalPredicate( pFirstFieldName, *pPredicate, isAllRange ) ;
         }
         else
         {
            selectivity = 1.0 ;
         }
      }
   }

   double _rtnIndexStat::evalStartStopKeys ( const CHAR *pFieldName,
                                             dmsStatKey &startKey,
                                             BOOLEAN startIncluded,
                                             dmsStatKey &stopKey,
                                             BOOLEAN stopIncluded,
                                             BOOLEAN isEqual ) const
   {
      INT32 rc = SDB_INVALIDARG ;
      double selectivity = 1.0 ;

      if ( isValid() )
      {
         if ( isEqual )
         {
            rc = _pIndexStat->evalETOperator( startKey, selectivity ) ;
         }
         else
         {
            rc = _pIndexStat->evalRangeOperator( startKey, startIncluded,
                                                 stopKey, stopIncluded,
                                                 selectivity ) ;
         }
      }

      if ( SDB_OK != rc )
      {
         // Simply evaluate one
         selectivity = _collectionStat.evalStartStopKeys( pFieldName,
                                                          startKey, startIncluded,
                                                          stopKey, stopIncluded,
                                                          isEqual ) ;
      }

      return selectivity ;
   }

   INT32 _rtnIndexStat::_evalStartStopKeys ( rtnStatPredList::iterator &predIter,
                                             dmsStatListKey &startKeys, BOOLEAN startIncluded,
                                             dmsStatListKey &stopKeys, BOOLEAN stopIncluded,
                                             BOOLEAN isEqual,
                                             double &selectivity ) const
   {
      SDB_ASSERT( isValid(), "Should not be invalid" ) ;

      INT32 rc = SDB_OK ;
      rtnStatPredList::iterator curPred = predIter ;
      rtnStatPredList::iterator nextPred = ++ predIter ;
      if ( curPred == nextPred )
      {
         if ( isEqual )
         {
            rc = _pIndexStat->evalETOperator( startKeys, selectivity ) ;
         }
         else
         {
            rc = _pIndexStat->evalRangeOperator( startKeys, startIncluded,
                                                 stopKeys, stopIncluded,
                                                 selectivity ) ;
         }
      }
      else
      {
         rtnPredicate *pPredicate = *curPred ;

         if ( pPredicate )
         {
            double tempSelectivity = 0.0 ;
            for ( vector<rtnStartStopKey>::const_iterator iterSSKey = pPredicate->_startStopKeys.begin();
                  iterSSKey != pPredicate->_startStopKeys.end() ;
                  iterSSKey ++ )
            {
               const rtnKeyBoundary &startKey = iterSSKey->_startKey ;
               const rtnKeyBoundary &stopKey = iterSSKey->_stopKey ;

               const BSONElement &beStart = startKey._bound ;
               const BSONElement &beStop = stopKey._bound ;

               BOOLEAN subStartIncluded = startIncluded && startKey._inclusive ;
               BOOLEAN subStopIncluded = stopIncluded && stopKey._inclusive ;
               BOOLEAN subIsEqual = isEqual && iterSSKey->isEquality() ;

               double subSelectivity = 1.0 ;

               startKeys.push_back( beStart ) ;
               stopKeys.push_back( beStop ) ;

               rc = _evalStartStopKeys( nextPred,
                                        startKeys, subStartIncluded,
                                        stopKeys, subStopIncluded,
                                        subIsEqual, subSelectivity ) ;
               if ( SDB_OK != rc )
               {
                  goto error ;
               }

               tempSelectivity += subSelectivity ;

               startKeys.pop_back() ;
               stopKeys.pop_back() ;
            }
            selectivity = OPT_ROUND_SELECTIVITY( tempSelectivity ) ;
         }
         else
         {
            startKeys.push_back( minKey.firstElement() ) ;
            stopKeys.push_back( maxKey.firstElement() ) ;

            rc = _evalStartStopKeys( nextPred,
                                     startKeys, startIncluded,
                                     stopKeys, stopIncluded,
                                     FALSE, selectivity ) ;

            startKeys.pop_back() ;
            stopKeys.pop_back() ;
         }
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   /*
      _rtnCollectionStat implement
    */
   _rtnCollectionStat::_rtnCollectionStat ( const CHAR *pCollectionName,
                                            UINT32 pageSize,
                                            _dmsMBContext *mbContext,
                                            const rtnStatMgr *statMgr )
   : _rtnStatBase( pCollectionName, 0 )
   {
      _pageSize = pageSize ;
      _pCollectionStat = statMgr ?
            statMgr->getCollectionStat( mbContext->mbID() ) : NULL ;

      _totalDataPages = 0 ;
      _totalDataLen = 0 ;

      _numIndexes = 0 ;
      _totalIndexPages = 0 ;
      _totalIndexSize = 0 ;
      _avgIndexPages = 0 ;
      _avgIndexSize = 0 ;

      initCurStat( mbContext ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCLSTAT_INITCURSTAT, "_rtnCollectionStat::initCurStat" )
   INT32 _rtnCollectionStat::initCurStat( _dmsMBContext *mbContext )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_RTNCLSTAT_INITCURSTAT ) ;

      _totalRecords = mbContext->mbStat()->_totalRecords ;
      _totalDataPages = mbContext->mbStat()->_totalDataPages ;
      _totalDataLen = mbContext->mbStat()->_totalOrgDataLen ;

      _numIndexes = mbContext->mb()->_numIndexes ;
      if ( _numIndexes > 0 )
      {
         _totalIndexPages = mbContext->mbStat()->_totalIndexPages ;
         _totalIndexSize = mbContext->mbStat()->_totalIndexPages * _pageSize -
                         mbContext->mbStat()->_totalIndexFreeSpace ;
         _avgIndexPages = (UINT32)( (double)_totalIndexPages / (double)_numIndexes ) ;
         _avgIndexSize = (UINT64)( (double)_totalIndexSize / (double)_numIndexes ) ;
      }

      PD_TRACE_EXITRC( SDB_RTNCLSTAT_INITCURSTAT, rc ) ;
      return rc ;
   }

   double _rtnCollectionStat::evalStartStopKeys ( const CHAR *pFieldName,
                                                  dmsStatKey &startKey,
                                                  BOOLEAN startIncluded,
                                                  dmsStatKey &stopKey,
                                                  BOOLEAN stopIncluded,
                                                  BOOLEAN isEqual ) const
   {
      INT32 rc = SDB_INVALIDARG ;

      double selectivity = 1.0 ;

      // Use the first field only
      const BSONElement &beStart = startKey.firstElement() ;
      const BSONElement &beStop = stopKey.firstElement() ;

      // Try to use the field statistics first
      const dmsIndexStat *pIndexStat = getFieldStat( pFieldName ) ;
      if ( pIndexStat )
      {
         if ( isEqual && pIndexStat->getNumKeys() == 1 )
         {
            dmsStatElementKey eleKey( beStart ) ;
            rc = pIndexStat->evalETOperator( eleKey, selectivity ) ;
         }
         else
         {
            dmsStatElementKey startEleKey( beStart ) ;
            dmsStatElementKey stopEleKey( beStop ) ;
            rc = pIndexStat->evalRangeOperator( startEleKey, startIncluded,
                                                stopEleKey, stopIncluded,
                                                selectivity ) ;
         }
      }

      // Failed to use field statistics, evaluate by default
      if ( SDB_OK != rc )
      {
         if ( isEqual )
         {
            selectivity = _evalETOperator( beStart ) ;
         }
         else
         {
            selectivity = _evalStartStopKeys( beStart, beStop ) ;
         }
      }

      return selectivity ;
   }

   double _rtnCollectionStat::evalETOpterator ( const CHAR *pFieldName,
                                                const BSONElement &beValue ) const
   {
      INT32 rc = SDB_INVALIDARG ;
      double selectivity = 1.0 ;

      // Try to use the field statistics first
      const dmsIndexStat *pIndexStat = getFieldStat( pFieldName ) ;
      if ( pIndexStat && pIndexStat->isValid() )
      {
         dmsStatElementKey statKey( beValue ) ;
         if ( pIndexStat->getNumKeys() == 1 )
         {
            rc = pIndexStat->evalETOperator( statKey, selectivity ) ;
         }
         else
         {
            rc = pIndexStat->evalRangeOperator( statKey, TRUE,
                                                statKey, TRUE,
                                                selectivity ) ;
         }
      }

      if ( SDB_OK != rc )
      {
         // Failed to use field statistics, evaluate by default
         selectivity = _evalETOperator( beValue ) ;
      }

      return selectivity ;
   }

   double _rtnCollectionStat::evalGTOpterator ( const CHAR *pFieldName,
                                                const BSONElement &beValue,
                                                BOOLEAN included ) const
   {
      INT32 rc = SDB_INVALIDARG ;
      double selectivity = 1.0 ;

      const dmsIndexStat *pIndexStat = getFieldStat( pFieldName ) ;
      if ( pIndexStat && pIndexStat->isValid() )
      {
         dmsStatElementKey statKey( beValue ) ;
         rc = pIndexStat->evalGTOperator( statKey, included, selectivity ) ;
      }

      if ( SDB_OK != rc )
      {
         // Failed to use field statistics, evaluate by default
         selectivity = _evalGTOperator( beValue ) ;
      }

      return selectivity ;
   }

   double _rtnCollectionStat::evalLTOpterator ( const CHAR *pFieldName,
                                                const BSONElement &beValue,
                                                BOOLEAN included ) const
   {
      INT32 rc = SDB_INVALIDARG ;
      double selectivity = 1.0 ;

      const dmsIndexStat *pIndexStat = getFieldStat( pFieldName ) ;
      if ( pIndexStat && pIndexStat->isValid() )
      {
         dmsStatElementKey statKey( beValue ) ;
         rc = pIndexStat->evalLTOperator( statKey, included, selectivity ) ;
      }

      if ( SDB_OK != rc )
      {
         // Failed to use field statistics, evaluate by default
         selectivity = _evalLTOperator( beValue ) ;
      }

      return selectivity ;
   }

   double _rtnCollectionStat::_evalStartStopKeys ( const BSONElement &startKey,
                                                   const BSONElement &stopKey ) const
   {
      double selectivity = OPT_PRED_DEF_SELECTIVITY ;

      if ( startKey.type() == stopKey.type() )
      {
         selectivity = _evalRangeOperator( startKey, stopKey ) ;
      }
      else if ( startKey.type() == MinKey &&
                stopKey.type() == MaxKey )
      {
         // Cover all values
         selectivity = 1.0 ;
      }
      else if ( stopKey.type() == MaxKey )
      {
         selectivity = _evalGTOperator( startKey ) ;
      }
      else if ( startKey.type() == MinKey )
      {
         selectivity = _evalLTOperator( stopKey ) ;
      }

      return selectivity ;
   }

   double _rtnCollectionStat::_evalETOperator ( const BSONElement &beValue ) const
   {
      if ( beValue.type() == Bool )
      {
         return 0.5 ;
      }
      else if ( _totalRecords > STAT_DEF_TOTAL_RECORDS )
      {
         return OSS_MIN( OPT_PRED_EQ_DEF_SELECTIVITY, 1.0 / (double)_totalRecords ) ;
      }

      return OPT_PRED_EQ_DEF_SELECTIVITY ;
   }

   double _rtnCollectionStat::_evalRangeOperator ( const BSONElement &beStart,
                                                   const BSONElement &beStop ) const
   {
      double selectivity = OPT_PRED_RANGE_DEF_SELECTIVITY ;

      if ( beStart.type() == beStop.type() )
      {
         switch ( beStart.type() )
         {
            case Bool :
            {
               // [ true, false ] is all range
               selectivity = 1.0 ;
               break ;
            }
            case NumberDouble :
            case NumberInt :
            case NumberLong :
            case NumberDecimal :
            {
               double start = OPT_ROUND_BSON_NUM( beStart.number() ) ;
               double stop = OPT_ROUND_BSON_NUM( beStop.number() ) ;
               selectivity = fabs( stop - start ) /
                             ( OPT_BSON_NUM_MAX - OPT_BSON_NUM_MIN ) ;
               break ;
            }
            case Timestamp :
            case Date :
            {
               double start = OPT_ROUND_BSON_NUM( beStart.number() ) ;
               double stop = OPT_ROUND_BSON_NUM( beStop.number() ) ;
               selectivity = fabs( stop - start ) /
                             ( OPT_BSON_NUM_MAX - OPT_BSON_NUM_MIN ) ;
               break ;
            }
            case String :
            {
               UINT32 startSize = beStart.valuestrsize() ;
               const CHAR *pStartStr = beStart.valuestr() ;
               startSize = OSS_MIN( startSize, OPT_BSON_STR_MIN_LEN ) ;

               UINT32 stopSize = beStop.valuestrsize() ;
               const CHAR *pStopStr = beStop.valuestr() ;
               stopSize = OSS_MIN( stopSize, OPT_BSON_STR_MIN_LEN ) ;

               selectivity = fabs( optConvertStrToScalar( pStopStr, stopSize,
                                                          OPT_BSON_STR_MIN,
                                                          OPT_BSON_STR_MAX ) -
                                   optConvertStrToScalar( pStartStr, startSize,
                                                          OPT_BSON_STR_MIN,
                                                          OPT_BSON_STR_MAX ) ) ;
               break ;
            }
            default :
            {
               selectivity = OPT_PRED_RANGE_DEF_SELECTIVITY ;
               break ;
            }
         }
      }
      else
      {
         // Start key and stop key are different types
         selectivity = OPT_PRED_DEF_SELECTIVITY ;
      }

      return OPT_ROUND_SELECTIVITY( selectivity ) ;
   }

   double _rtnCollectionStat::_evalGTOperator ( const BSONElement &beStart ) const
   {
      double selectivity = OPT_PRED_DEF_SELECTIVITY ;

      switch ( beStart.type() )
      {
         case Bool :
         {
            // Either true or false
            selectivity = 0.5 ;
            break ;
         }
         case NumberDouble :
         case NumberInt :
         case NumberLong :
         case NumberDecimal :
         {
            double start = OPT_ROUND_BSON_NUM( beStart.number() ) ;
            selectivity = ( OPT_BSON_NUM_MAX - start ) /
                          ( OPT_BSON_NUM_MAX - OPT_BSON_NUM_MIN ) ;
            break ;
         }
         case Timestamp :
         case Date :
         {
            double start = OPT_ROUND_BSON_NUM( beStart.number() ) ;
            selectivity = ( OPT_BSON_NUM_MAX - start ) /
                          ( OPT_BSON_NUM_MAX - OPT_BSON_NUM_MIN ) ;
            break ;
         }
         case String :
         {
            UINT32 strSize = beStart.valuestrsize() ;
            const CHAR *pStr = beStart.valuestr() ;
            strSize = OSS_MIN( strSize, OPT_BSON_STR_MIN_LEN ) ;
            selectivity = 1.0 - optConvertStrToScalar( pStr, strSize,
                                                       OPT_BSON_STR_MIN,
                                                       OPT_BSON_STR_MAX ) ;
            break ;
         }
         default :
         {
            selectivity = OPT_PRED_DEF_SELECTIVITY ;
            break ;
         }
      }

      return OPT_ROUND_SELECTIVITY( selectivity ) ;
   }

   double _rtnCollectionStat::_evalLTOperator ( const BSONElement &beStop ) const
   {
      double selectivity = OPT_PRED_DEF_SELECTIVITY ;

      switch ( beStop.type() )
      {
         case Bool :
         {
            // Either true or false
            selectivity = 0.5 ;
            break ;
         }
         case NumberDouble :
         case NumberInt :
         case NumberLong :
         case NumberDecimal :
         {
            double stop = OPT_ROUND_BSON_NUM( beStop.number() ) ;
            selectivity = ( stop - OPT_BSON_NUM_MIN ) /
                          ( OPT_BSON_NUM_MAX - OPT_BSON_NUM_MIN ) ;
            break ;
         }
         case Timestamp :
         case Date :
         {
            double stop = OPT_ROUND_BSON_NUM( beStop.number() ) ;
            selectivity = ( stop - OPT_BSON_NUM_MIN ) /
                          ( OPT_BSON_NUM_MAX - OPT_BSON_NUM_MIN );
            break ;
         }
         case String :
         {
            UINT32 strSize = beStop.valuestrsize() ;
            const CHAR *pStr = beStop.valuestr() ;
            strSize = OSS_MIN( strSize, OPT_BSON_STR_MIN_LEN ) ;
            selectivity = optConvertStrToScalar( pStr, strSize,
                                                 OPT_BSON_STR_MIN,
                                                 OPT_BSON_STR_MAX ) ;
            break ;
         }
         default :
         {
            selectivity = OPT_PRED_DEF_SELECTIVITY ;
            break ;
         }
      }

      return OPT_ROUND_SELECTIVITY( selectivity ) ;
   }

}

