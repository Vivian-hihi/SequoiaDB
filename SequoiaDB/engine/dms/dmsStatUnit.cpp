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

   Source File Name = dmsStatUnit.cpp

   Descriptive Name = DMS Statistics Units

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains code logic for
   statistics objects.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "dmsStatUnit.hpp"
#include "pdTrace.hpp"
#include "dmsTrace.hpp"
#include "dmsEventHandler.hpp"
#include "msgDef.hpp"

namespace engine
{

   #define DMS_STAT_VERSION                   "Version"
   #define DMS_STAT_SAMPLE_RECORDS            "SampleRecords"
   #define DMS_STAT_TOTAL_RECORDS             FIELD_NAME_TOTAL_RECORDS

   #define DMS_STAT_FIELD_FRAC_NAME           "Frac"

   #define DMS_STAT_CL_TOTAL_DATA_PAGES       FIELD_NAME_TOTAL_DATA_PAGES
   #define DMS_STAT_CL_TOTAL_DATA_SIZE        FIELD_NAME_TOTAL_DATA_SIZE
   #define DMS_STAT_CL_AVG_NUM_FIELDS         "AvgNumFields"

   #define DMS_STAT_IDX_KEY_PATTERN           "KeyPattern"
   #define DMS_STAT_IDX_INDEX_PAGES           "IndexPages"
   #define DMS_STAT_IDX_LEVELS                "IndexLevels"
   #define DMS_STAT_IDX_IS_UNIQUE             "IsUnique"
   #define DMS_STAT_IDX_DISTINCT_VALUES       "DistinctValues"
   #define DMS_STAT_IDX_NULL_FRAC             "NullFrac"
   #define DMS_STAT_IDX_UNDEF_FRAC            "UndefFrac"
   #define DMS_STAT_IDX_MCV                   "MCV"
   #define DMS_STAT_IDX_MCV_VALUES            "Values"
   #define DMS_STAT_IDX_MCV_FRAC              DMS_STAT_FIELD_FRAC_NAME
   #define DMS_STAT_IDX_HISTOGRAM             "Histogram"
   #define DMS_STAT_IDX_HISTOGRAM_FRAC        DMS_STAT_FIELD_FRAC_NAME
   #define DMS_STAT_IDX_HISTOGRAM_BOUNDS      "Bounds"
   #define DMS_STAT_IDX_TYPE_SET              "TypeSet"
   #define DMS_STAT_IDX_TYPE_SET_TYPES        "Types"
   #define DMS_STAT_IDX_TYPE_SET_FRAC         DMS_STAT_FIELD_FRAC_NAME

   #define DMS_STAT_CHECKHOLE_THRESHOLD       ( 10 )

   /*
      _dmsStatValues implement
    */
   _dmsStatValues::_dmsStatValues ()
   {
      _numKeys = 0 ;
      _size = 0 ;
      _pValues = NULL ;
   }

   _dmsStatValues::~_dmsStatValues ()
   {
      _clear() ;
   }

   INT32 _dmsStatValues::init ( UINT32 size )
   {
      INT32 rc = SDB_OK ;

      if ( size == 0 )
      {
         goto done ;
      }

      _pValues = new(std::nothrow) BSONObj[ size ] ;
      PD_CHECK( _pValues, SDB_OOM, error, PDWARNING,
                "Failed to allocate %u values", size ) ;

      _size = size ;

   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStatValues::binarySearch ( dmsStatKey &keyValue,
                                        INT32 keyIncFlag,
                                        BOOLEAN &isEqual ) const
   {
      isEqual = FALSE ;

      INT32 low = 0, high = _size - 1, mid ;
      INT32 index = -1 ;
      BSONObj boEmpty ;

      // The output idx means:
      // 1. idx is 0
      //    1. isEqual is true, value == _pValues[0]
      //    2. isEqual is false, value < _pValues[0]
      // 2. idx is 1 to _size - 1
      //    1. isEqual is true, value == _pValues[idx]
      //    2. isEqual is false, _pValues[idx-1]< value < _pValues[idx]
      // 3. idx is _size: _pValues[_size - 1] < value

      if ( 0 == _size )
      {
         return -1 ;
      }

      while ( low <= high )
      {
         mid = ( low + high ) / 2 ;

         INT32 res = keyValue.compareValue( keyIncFlag, _pValues[ mid ] ) ;

         if ( 0 == res )
         {
            index = mid ;
            isEqual = TRUE ;
            break ;
         }
         else if ( res > 0 )
         {
            index = mid + 1 ;
            low = mid + 1 ;
         }
         else
         {
            high = mid - 1 ;
            index = mid ;
         }
      }

      return index ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSIDXSTAT_CHKVALS, "_dmsStatValues::checkValues" )
   INT32 _dmsStatValues::checkValues ( UINT32 numKeys, const BSONObj &keyPattern )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__DMSIDXSTAT_CHKVALS ) ;

      for ( UINT32 idx = 0 ; idx < _size ; idx ++ )
      {
         PD_CHECK( numKeys >= (UINT32)_pValues[idx].nFields(),
                   SDB_INVALIDARG, error, PDWARNING, "Number of keys are not "
                   "matched, index: %u, expected: %u, actual: %d",
                   idx, numKeys, _pValues[idx].nFields() ) ;
         if ( numKeys == (UINT32)_pValues[idx].nFields() )
         {
            BSONObjIterator iterKey( keyPattern ) ;
            BSONObjIterator iterValue( _pValues[idx] ) ;
            while( iterKey.more() && iterValue.more() )
            {
               const BSONElement &beKey = iterKey.next() ;
               const BSONElement &beValue = iterValue.next() ;

               PD_CHECK( 0 == ossStrcmp( beKey.fieldName(), beValue.fieldName() ),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Key names are not matched, expected: %s, actual: %s",
                         beKey.fieldName(), beValue.fieldName() ) ;
            }
            PD_CHECK( !iterKey.more() && !iterValue.more(),
                      SDB_INVALIDARG, error, PDWARNING,
                      "Number of keys are not matched" ) ;
         }
         else
         {
            BSONObjIterator iterKey( keyPattern ) ;
            BSONObjIterator iterValue( _pValues[idx] ) ;
            BSONObjBuilder valueBuilder ;
            while( iterKey.more() )
            {
               const BSONElement &beKey = iterKey.next() ;
               if ( iterValue.more() )
               {
                  const BSONElement &beValue = iterValue.next() ;

                  PD_CHECK( 0 == ossStrcmp( beKey.fieldName(), beValue.fieldName() ),
                            SDB_INVALIDARG, error, PDWARNING,
                            "Key names are not matched, expected: %s, actual: %s",
                            beKey.fieldName(), beValue.fieldName() ) ;

                  valueBuilder.append( beValue ) ;
               }
               else
               {
                  valueBuilder.appendUndefined( beKey.fieldName() ) ;
               }
            }
            PD_CHECK( !iterValue.more() && !iterKey.more(),
                      SDB_INVALIDARG, error, PDWARNING,
                      "Number of keys are not matched" ) ;
            _pValues[idx] = valueBuilder.obj() ;
         }
      }

      _numKeys = numKeys ;

   done :
      PD_TRACE_EXIT( SDB__DMSIDXSTAT_CHKVALS ) ;
      return rc ;
   error :
      goto done ;
   }

   void _dmsStatValues::_clear ()
   {
      if ( _pValues )
      {
         delete [] _pValues ;
         _pValues = NULL ;
      }
      _size = 0 ;
   }

   BOOLEAN _dmsStatValues::_inRange ( UINT32 idx, dmsStatKey *pStartKey,
                                      dmsStatKey *pStopKey ) const
   {
      if ( idx > _size )
      {
         return FALSE ;
      }

      if ( idx == _size )
      {
         return pStopKey ? FALSE : TRUE ;
      }

      if ( pStartKey )
      {
         // Expect start key < value[idx]
         // The first element is compared in earlier range comparison
         if ( !pStartKey->compareAllValues( 1, -1, _pValues[idx] ) )
         {
            return FALSE ;
         }
      }
      if ( pStopKey )
      {
         // Expect stop key > value[idx]
         // The first element is compared in earlier range comparison
         if ( !pStopKey->compareAllValues( 1, 1, _pValues[idx] ) )
         {
            return FALSE ;
         }
      }

      return TRUE ;
   }

   /*
      _dmsStatMCVSet implement
    */
   _dmsStatMCVSet::_dmsStatMCVSet ()
   : _dmsStatValues ()
   {
      _pFractions = NULL ;
      _totalFrac = 0.0 ;
   }

   _dmsStatMCVSet::~_dmsStatMCVSet ()
   {
      if ( _pFractions )
      {
         delete [] _pFractions ;
         _pFractions = NULL ;
      }
   }

   INT32 _dmsStatMCVSet::init ( UINT32 size )
   {
      INT32 rc = SDB_OK ;

      if ( size == 0 )
      {
         goto done ;
      }

      rc = _dmsStatValues::init( size ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to init values, rc: %d", rc ) ;

      _pFractions = new(std::nothrow)double[ size ] ;
      PD_CHECK( _pFractions, SDB_OOM, error, PDWARNING,
                "Failed to allocate memory for fractions" ) ;

      _totalFrac = 0.0 ;

   done :
      return rc ;
   error :
      _clear() ;
      goto done ;
   }

   void _dmsStatMCVSet::clear ()
   {
      if ( _pFractions )
      {
         delete [] _pFractions ;
         _pFractions = NULL ;
      }
      _totalFrac = 0.0 ;
      _dmsStatValues::_clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATMCV_EVALOPTR, "_dmsStatMCVSet::evalOperator" )
   INT32 _dmsStatMCVSet::evalOperator ( dmsStatKey *pStartKey,
                                         dmsStatKey *pStopKey,
                                         BOOLEAN &hitMCV,
                                         double &predSelectivity,
                                         double &scanSelectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATMCV_EVALOPTR ) ;

      BOOLEAN startIncluded = FALSE, stopIncluded = FALSE ;
      BOOLEAN startEqual = FALSE, stopEqual = FALSE ;
      INT32 startFlag = 0, stopFlag = 0 ;
      INT32 startIdx = -1, stopIdx = -1 ;
      UINT32 rangeCount = 0 ;
      double tmpPredSel = 0.0, tmpScanSel = 0.0 ;

      BOOLEAN checkHoles = FALSE ;

      PD_CHECK( getSize() > 0, SDB_INVALIDARG, error, PDWARNING,
                "No MCV set is available" ) ;

      PD_CHECK( ( pStartKey && pStopKey && pStartKey->size() == pStopKey->size() ) ||
                  pStartKey || pStopKey, SDB_INVALIDARG, error, PDWARNING,
                  "Numbers of keys are not matched" ) ;

      // Only check holes when number of keys are larger than 1
      if ( _numKeys > 1 && ( ( pStartKey && pStartKey->size() > 1 ) ||
                             ( pStopKey && pStopKey->size() > 1 ) ) )
      {
         checkHoles = TRUE ;
      }

      // Get the index of start key
      if ( pStartKey )
      {
         startIncluded = pStartKey->isIncluded() ;
         startFlag = startIncluded ? -1 : ( _numKeys == pStartKey->size() ? 0 : 1 ) ;
         startIdx = binarySearch( *pStartKey, startFlag, startEqual ) ;
         PD_CHECK( startIdx >= 0, SDB_INVALIDARG, error, PDWARNING,
                   "Failed to locate start key %s in MCV set",
                   pStartKey->toString().c_str() ) ;
      }
      else
      {
         // $lt operator, include all smaller MCV items
         startIdx = 0 ;
         startIncluded = TRUE ;
         startEqual = FALSE ;
      }

      // Get the index of stop key
      if ( pStopKey )
      {
         stopIncluded = pStopKey->isIncluded() ;
         stopFlag = stopIncluded ? 1 : ( _numKeys == pStopKey->size() ? 0 : -1 ) ;
         stopIdx = binarySearch( *pStopKey, stopFlag, stopEqual ) ;
         PD_CHECK( stopIdx >= 0, SDB_INVALIDARG, error, PDWARNING,
                   "Failed to locate stop key %s in MCV set",
                   pStopKey->toString().c_str() ) ;
      }
      else
      {
         // $gt operator, include all greater MCV items
         stopIdx = getSize() ;
         stopIncluded = TRUE ;
         stopEqual = FALSE ;
      }

      // Don't need to check holes if the range is too large
      if ( checkHoles && ( stopIdx - startIdx > DMS_STAT_CHECKHOLE_THRESHOLD ) )
      {
         checkHoles = FALSE ;
      }

      if ( startIdx != stopIdx )
      {
         // Check startIdx
         if ( startIncluded || !startEqual )
         {
            // The start key <= value case, add the first selected MCV item
            tmpScanSel += getFrac( startIdx ) ;
            if ( checkHoles && ( ( startIncluded && startEqual ) ||
                                 _inRange( (UINT32)startIdx, pStartKey, pStopKey ) ) )
            {
               tmpPredSel += getFrac( startIdx ) ;
            }
            rangeCount ++ ;
         }

         // Check startIdx + 1 to stopIdx - 1
         for ( INT32 idx = startIdx + 1 ; idx < stopIdx ; idx ++ )
         {
            // Every ranges between selected MCV items are needed
            tmpScanSel += getFrac( idx ) ;
            if ( checkHoles && _inRange( idx, pStartKey, pStopKey ) )
            {
               tmpPredSel += getFrac( idx ) ;
            }
            rangeCount ++ ;
         }

         // Check stopIdx
         if ( stopIncluded && stopEqual && stopIdx < (INT32)getSize() )
         {
            // The stop key is equal to the last selected MCV item, add
            // the last selected MCV item
            tmpScanSel += getFrac( stopIdx ) ;
            if ( checkHoles )
            {
               tmpPredSel += getFrac( stopIdx ) ;
            }
            rangeCount ++ ;
         }
      }
      else
      {
         // start idx is equal to stop idx, which means the start key and stop
         // key are equal to the same MCV item
         if ( stopIncluded && startEqual && stopIncluded && stopEqual &&
              startIdx < (INT32)getSize() )
         {
            // The stop key is equal to the selected MCV item, which should
            // be included
            tmpScanSel = getFrac( startIdx ) ;
            if ( checkHoles )
            {
               tmpPredSel += getFrac( startIdx ) ;
            }
            rangeCount ++ ;
         }
      }

      scanSelectivity = DMS_STAT_ROUND_SELECTIVITY( tmpScanSel ) ;
      if ( checkHoles )
      {
         predSelectivity = DMS_STAT_ROUND_SELECTIVITY( tmpPredSel ) ;
      }
      else
      {
         predSelectivity = scanSelectivity ;
      }
      hitMCV = rangeCount > 0 ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATMCV_EVALOPTR, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATMCV_EVALETOPTR, "_dmsStatMCVSet::evalETOperator" )
   INT32 _dmsStatMCVSet::evalETOperator ( dmsStatKey &key,
                                           BOOLEAN &hitMCV,
                                           double &predSelectivity,
                                           double &scanSelectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATMCV_EVALETOPTR );

      BOOLEAN equal = FALSE ;
      INT32 idx = -1 ;
      double tmpPredSel = 0.0 ;

      PD_CHECK( getSize() > 0, SDB_INVALIDARG, error, PDWARNING,
                "No MCV set is available" ) ;

      idx = binarySearch( key, 0, equal ) ;
      PD_CHECK( idx >= 0, SDB_INVALIDARG, error, PDWARNING,
                "Failed to locate start key %s in MCV set",
                key.toString().c_str() ) ;

      if ( idx < (INT32)getSize() && equal )
      {
         tmpPredSel = getFrac( idx ) ;
         hitMCV = TRUE ;
      }
      else
      {
         hitMCV = FALSE ;
      }

      predSelectivity = DMS_STAT_ROUND_SELECTIVITY( tmpPredSel ) ;
      scanSelectivity = predSelectivity ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATMCV_EVALETOPTR, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
      _dmsStatUnit implement
    */
   _dmsStatUnit::_dmsStatUnit ()
   : _utilSUCacheUnit()
   {
      _initItems() ;
   }

   _dmsStatUnit::_dmsStatUnit ( UINT16 mbID, UINT64 version )
   : _utilSUCacheUnit( mbID, version )
   {
      _initItems() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATBASE_INIT, "_dmsStatUnit::init" )
   INT32 _dmsStatUnit::init ( const BSONObj &boStat )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATBASE_INIT ) ;

      try
      {
         BSONObjIterator iter( boStat ) ;

         while ( iter.more() )
         {
            BSONElement beItem = iter.next() ;

            if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_COLLECTION_SPACE ) )
            {
               const CHAR *pCSName = NULL ;

               PD_CHECK( String == beItem.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", DMS_STAT_COLLECTION_SPACE ) ;

               pCSName = beItem.valuestr() ;
               setCSName( pCSName ) ;
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_COLLECTION ) )
            {
               const CHAR *pCLName = NULL ;

               PD_CHECK( String == beItem.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", DMS_STAT_COLLECTION ) ;

               pCLName = beItem.valuestr() ;
               setCLName( pCLName ) ;
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_COLLECTION_MBID ) )
            {
               PD_CHECK( NumberInt == beItem.type() &&
                         (UINT32)beItem.Int() < DMS_MME_SLOTS,
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", DMS_STAT_COLLECTION_MBID ) ;
               // For statistics cache, mbID is ID of unit
               _setUnitID( (UINT16)beItem.Int() ) ;
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_VERSION ) )
            {
               PD_CHECK( beItem.isNumber(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", DMS_STAT_VERSION ) ;
               setVersion( (UINT64)beItem.numberLong() ) ;
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_SAMPLE_RECORDS ) )
            {
               PD_CHECK( beItem.isNumber(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", DMS_STAT_SAMPLE_RECORDS ) ;
               _sampleRecords = (UINT64)beItem.numberLong() ;
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_TOTAL_RECORDS ) )
            {
               PD_CHECK( beItem.isNumber(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", DMS_STAT_TOTAL_RECORDS ) ;
               _totalRecords = (UINT64)beItem.numberLong() ;
            }
            else
            {
               rc = _initItem( beItem ) ;
               if ( SDB_OK != rc )
               {
                  goto error ;
               }
            }
         }

         rc = _postInit() ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to process after initialization, rc: %d", rc ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Failed to initialize statistics object, received "
                 "unexpected error: %s", e.what() );
         rc = SDB_INVALIDARG;
         goto error;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATBASE_INIT, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   void _dmsStatUnit::_initItems ()
   {
      _sampleRecords = DMS_STAT_DEF_TOTAL_RECORDS ;
      _totalRecords = DMS_STAT_DEF_TOTAL_RECORDS ;
   }

   /*
      _dmsIndexStat implement
    */
   _dmsIndexStat::_dmsIndexStat ()
   : _dmsStatUnit ()
   {
      setCSName( NULL ) ;
      setCLName( NULL ) ;
      setIndexName( NULL ) ;
      _initItems() ;
   }

   _dmsIndexStat::_dmsIndexStat ( const CHAR *pCSName, const CHAR *pCLName,
                                  UINT16 mbID, UINT64 version,
                                  const CHAR *pIndexName )
   : _dmsStatUnit( mbID, version )
   {
      setCSName( pCSName ) ;
      setCLName( pCLName ) ;
      setIndexName( pIndexName ) ;
      _initItems() ;
   }

   _dmsIndexStat::~_dmsIndexStat ()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT_EVALRANGEOPTR, "_dmsIndexStat::evalRangeOperator" )
   INT32 _dmsIndexStat::evalRangeOperator ( dmsStatKey &startKey,
                                            dmsStatKey &stopKey,
                                            double &predSelectivity,
                                            double &scanSelectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT_EVALRANGEOPTR ) ;

      rc = _evalOperator( &startKey, &stopKey, predSelectivity, scanSelectivity ) ;

      PD_TRACE_EXITRC( SDB_DMSIDXSTAT_EVALRANGEOPTR, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT_EVALETOPTR, "_dmsIndexStat::evalETOperator" )
   INT32 _dmsIndexStat::evalETOperator ( dmsStatKey &key,
                                          double &predSelectivity,
                                          double &scanSelectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT_EVALETOPTR );

      BOOLEAN hitMCV = FALSE ;

      // Special case for unique index, could be one of the totalRecords
      if ( _isUnique && key.size() == _numKeys )
      {
         predSelectivity = 1.0 / (double)_totalRecords ;
         scanSelectivity = predSelectivity ;
         goto done ;
      }

      PD_CHECK( _mcvSet.getSize() > 0, SDB_INVALIDARG, error, PDWARNING,
                "No MCV set is available" ) ;

      rc = _mcvSet.evalETOperator( key, hitMCV, predSelectivity, scanSelectivity ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to evaluate from MCV set, rc: %d", rc ) ;

      if ( !hitMCV )
      {
         // The value is not in MCV set, evaluate in the rest of values
         if ( _distinctValues == _mcvSet.getSize() )
         {
            predSelectivity = ( 1.0 - _mcvSet.getTotalFrac() ) *
                              DMS_STAT_PRED_EQ_DEF_SELECTIVITY ;
         }
         else
         {
            predSelectivity = ( 1.0 - _mcvSet.getTotalFrac() ) /
                              (double) ( _distinctValues - _mcvSet.getSize() ) ;
         }
         scanSelectivity = predSelectivity ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSIDXSTAT_EVALETOPTR, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT_EVALGTOPTR, "_dmsIndexStat::evalGTOperator" )
   INT32 _dmsIndexStat::evalGTOperator ( dmsStatKey &startKey,
                                          double &predSelectivity,
                                          double &scanSelectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT_EVALGTOPTR );

      rc = _evalOperator( &startKey, NULL, predSelectivity, scanSelectivity ) ;

      PD_TRACE_EXITRC( SDB_DMSIDXSTAT_EVALGTOPTR, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT_EVALLTOPTR, "_dmsIndexStat::evalLTOperator" )
   INT32 _dmsIndexStat::evalLTOperator ( dmsStatKey &stopKey,
                                          double &predSelectivity,
                                          double &scanSelectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT_EVALLTOPTR );

      rc = _evalOperator( NULL, &stopKey, predSelectivity, scanSelectivity ) ;

      PD_TRACE_EXITRC( SDB_DMSIDXSTAT_EVALLTOPTR, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT__EVALOPTR, "_dmsIndexStat::_evalOperator" )
   INT32 _dmsIndexStat::_evalOperator ( dmsStatKey *pStartKey,
                                         dmsStatKey *pStopKey,
                                         double &predSelectivity,
                                         double &scanSelectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT__EVALOPTR ) ;

      BOOLEAN hitMCV = FALSE ;

      PD_CHECK( _mcvSet.getSize() > 0, SDB_INVALIDARG, error, PDWARNING,
                "No MCV set is available" ) ;

      rc = _mcvSet.evalOperator( pStartKey, pStopKey,
                                 hitMCV, predSelectivity, scanSelectivity ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to evaluate from MCV set, rc: %d", rc ) ;

      if ( !hitMCV )
      {
         // The values do not include any MCV items, try to evaluate from
         // the rest of values
         predSelectivity = ( 1.0 - _mcvSet.getTotalFrac() ) *
                           DMS_STAT_PRED_RANGE_DEF_SELECTIVITY ;
         scanSelectivity = predSelectivity ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSIDXSTAT__EVALOPTR, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   void _dmsIndexStat::_initItems ()
   {
      _pFirstField = NULL ;
      _numKeys = 0 ;

      _indexPages = DMS_STAT_DEF_TOTAL_PAGES ;
      _indexLevels = DMS_STAT_DEF_IDX_LEVELS ;

      _isUnique = FALSE ;
      _distinctValues = 0 ;

      _nullFrac = 0.0 ;
      _undefFrac = 0.0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT__INITITEM, "_dmsIndexStat::_initItem" )
   INT32 _dmsIndexStat::_initItem ( const BSONElement &beItem )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT__INITITEM ) ;

      if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_INDEX ) )
      {
         const CHAR *pIndexName = NULL ;

         PD_CHECK( String == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", DMS_STAT_IDX_INDEX ) ;

         pIndexName = beItem.valuestr() ;
         setIndexName( pIndexName ) ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_INDEX_PAGES ) )
      {
         PD_CHECK( beItem.isNumber(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", DMS_STAT_IDX_INDEX_PAGES ) ;
         _indexPages = (UINT32)beItem.numberInt() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_LEVELS ) )
      {
         PD_CHECK( beItem.isNumber(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", DMS_STAT_IDX_LEVELS ) ;
         _indexLevels = (UINT32)beItem.numberInt() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_KEY_PATTERN ) )
      {
         PD_CHECK( Object == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", DMS_STAT_IDX_KEY_PATTERN ) ;
         rc = _initKeyPattern ( beItem.embeddedObject() ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to init key pattern, rc: %d",
                      rc ) ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_IS_UNIQUE ) )
      {
         PD_CHECK( Bool == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", DMS_STAT_IDX_IS_UNIQUE ) ;
         _isUnique = beItem.booleanSafe() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_NULL_FRAC ) )
      {
         if ( beItem.isNumber() )
         {
            _nullFrac = beItem.numberDouble() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Type of field [%s] is not matched, "
                    "expected is %d, actually is %d",
                    DMS_STAT_IDX_NULL_FRAC, NumberDouble, beItem.type() ) ;
         }
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_UNDEF_FRAC ) )
      {
         if ( beItem.isNumber() )
         {
            _undefFrac = beItem.numberDouble() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Field [%s] is not matched", DMS_STAT_IDX_UNDEF_FRAC ) ;
         }
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_DISTINCT_VALUES ) )
      {
         if ( beItem.isNumber() )
         {
            _distinctValues = (UINT64)beItem.numberLong() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Field [%s] is not matched", DMS_STAT_IDX_DISTINCT_VALUES ) ;
         }
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_IDX_MCV ) )
      {
         if ( Object == beItem.type() )
         {
            rc = _initMCV( beItem.embeddedObject() ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDWARNING, "Failed to init mcv, rc: %d", rc ) ;
            }
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Field [%s] is not matched", DMS_STAT_IDX_MCV ) ;
         }
      }

      rc = SDB_OK ;

   done :
      PD_TRACE_EXITRC( SDB_DMSIDXSTAT__INITITEM, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT__POSTINIT, "_dmsIndexStat::_postInit" )
   INT32 _dmsIndexStat::_postInit ()
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT__POSTINIT ) ;

      // Initialize the distinct value number if it's not found
      if ( 0 == _distinctValues )
      {
         if ( _isUnique )
         {
            _distinctValues = _totalRecords ;
         }
         else
         {
            _distinctValues = _mcvSet.getSize() ;
         }
      }

      rc = _mcvSet.checkValues( _numKeys, _keyPattern ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to set numKeys of MCV set, rc: %d",
                   rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSIDXSTAT__POSTINIT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT__INITKEYPTN, "_dmsIndexStat::_initKeyPattern" )
   INT32 _dmsIndexStat::_initKeyPattern ( const BSONObj &boKeyPattern )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT__INITKEYPTN ) ;

      _keyPattern = boKeyPattern.copy() ;
      _numKeys = _keyPattern.nFields() ;

      PD_CHECK( _numKeys > 0, SDB_INVALIDARG, error, PDWARNING,
                "Empty key pattern" ) ;

      _pFirstField = _keyPattern.firstElementFieldName() ;

   done :
      PD_TRACE_EXITRC( SDB_DMSIDXSTAT__INITKEYPTN, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT__INITMCV, "_dmsIndexStat::_initMCV" )
   INT32 _dmsIndexStat::_initMCV ( const BSONObj &boMCV )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT__INITMCV ) ;

      double totalFrac = 0.0 ;

      BSONObjIterator iter( boMCV ) ;
      while ( iter.more() )
      {
         BSONElement eleTemp = iter.next() ;

         if ( 0 == ossStrcmp( eleTemp.fieldName(), DMS_STAT_IDX_MCV_VALUES ) )
         {
            PD_CHECK( Array == eleTemp.type(),
                      SDB_INVALIDARG, error, PDWARNING,
                      "Field [%s] 's type is not matched",
                      eleTemp.toString().c_str() ) ;
            BSONObj boValues = eleTemp.embeddedObject() ;
            if ( _mcvSet.getSize() == 0 && boValues.nFields() > 0 )
            {
               rc = _mcvSet.init( boValues.nFields() ) ;
               PD_RC_CHECK( rc, PDWARNING, "Failed to init mcv, rc: %d", rc ) ;
            }
            PD_CHECK( _mcvSet.getSize() == (UINT32)boValues.nFields(),
                      SDB_INVALIDARG, error, PDWARNING,
                      "Field [%s] 's type is not matched",
                      eleTemp.toString().c_str() ) ;
            BSONObjIterator iterValue( boValues ) ;
            UINT32 idx = 0 ;
            while ( iterValue.more() )
            {
               BSONElement tempVal = iterValue.next() ;
               PD_CHECK( Object == tempVal.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] 's type is not matched",
                         tempVal.toString().c_str() ) ;
               _mcvSet.setValue( idx, tempVal.embeddedObject() ) ;
               ++ idx ;
            }
         }
         else if ( 0 == ossStrcmp( eleTemp.fieldName(), DMS_STAT_IDX_MCV_FRAC ) )
         {
            PD_CHECK( Array == eleTemp.type(),
                      SDB_INVALIDARG, error, PDWARNING,
                      "Field [%s] 's type is not matched",
                      eleTemp.toString().c_str() ) ;
            BSONObj boFrac = eleTemp.embeddedObject() ;
            if ( _mcvSet.getSize() == 0 && boFrac.nFields() > 0 )
            {
               rc = _mcvSet.init( boFrac.nFields() ) ;
               PD_RC_CHECK( rc, PDWARNING, "Failed to init mcv, rc: %d", rc ) ;
            }
            PD_CHECK( _mcvSet.getSize() == (UINT32)boFrac.nFields(),
                      SDB_INVALIDARG, error, PDWARNING,
                      "Field [%s] 's type is not matched",
                      eleTemp.toString().c_str() ) ;
            BSONObjIterator iterFrac( boFrac ) ;
            UINT32 idx = 0 ;
            while ( iterFrac.more() )
            {
               double frac = 0.0 ;
               BSONElement tempFrac = iterFrac.next() ;
               PD_CHECK( tempFrac.isNumber(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] 's type is not matched",
                         tempFrac.toString().c_str() ) ;
               frac = tempFrac.number() ;
               _mcvSet.setFrac( idx, frac ) ;
               totalFrac += frac ;
               ++ idx ;
            }
         }
      }

      totalFrac = DMS_STAT_ROUND_SELECTIVITY( totalFrac ) ;
      _mcvSet.setTotalFrac( totalFrac ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSIDXSTAT__INITMCV, rc ) ;
      return rc ;
   error :
      _mcvSet.clear() ;
      goto done ;
   }

   /*
      _dmsCollectionStat implement
    */
   _dmsCollectionStat::_dmsCollectionStat ()
   : _dmsStatUnit()
   {
      setCSName( NULL ) ;
      setCLName( NULL ) ;
      _initItems() ;
   }

   _dmsCollectionStat::_dmsCollectionStat ( const CHAR *pCSName,
                                            const CHAR *pCLName,
                                            UINT16 mbID, UINT64 version )
   : _dmsStatUnit( mbID, version )
   {
      setCSName( pCSName ) ;
      setCLName( pCLName ) ;
      _initItems() ;
   }

   _dmsCollectionStat::~_dmsCollectionStat ()
   {
      clearSubUnits() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_ADDSUBUNIT, "_dmsCollectionStat::addSubUnit" )
   BOOLEAN _dmsCollectionStat::addSubUnit ( utilSUCacheUnit *pSubUnit,
                                            BOOLEAN ignoreVersion )
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_ADDSUBUNIT ) ;

      BOOLEAN added = FALSE ;

      if ( pSubUnit &&
           pSubUnit->getUnitType() == UTIL_SU_CACHE_UNIT_IDXSTAT &&
           pSubUnit->getUnitID() == getUnitID() )
      {
         dmsIndexStat *pIndexStat = (dmsIndexStat *)pSubUnit ;
         const CHAR *pIndexName = pIndexStat->getIndexName() ;
         INDEX_STAT_ITERATOR iter = _indexStats.find( pIndexName ) ;
         if ( iter != _indexStats.end() )
         {
            dmsIndexStat *pTempStat = iter->second ;

            if ( ignoreVersion ||
                 pTempStat->getVersion() < pIndexStat->getVersion() )
            {
               _indexStats.erase( iter ) ;
               SAFE_OSS_DELETE( pTempStat ) ;

               _indexStats[ pIndexName ] = pIndexStat ;
               added = TRUE ;
            }
         }
         else
         {
            _indexStats[ pIndexName ] = pIndexStat ;
            added = TRUE ;
         }

         if ( added )
         {
            // The name pointer should be fixed
            pIndexStat->setCSName( _pCSName ) ;
            pIndexStat->setCLName( _pCLName ) ;
            _addFieldStat( pIndexStat, ignoreVersion ) ;
         }
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_ADDSUBUNIT ) ;

      return added ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_CLEARSUBUNITS, "_dmsCollectionStat::clearSubUnits" )
   void _dmsCollectionStat::clearSubUnits ()
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_CLEARSUBUNITS ) ;

      INDEX_STAT_ITERATOR iter ;

      for ( iter = _indexStats.begin() ;
            iter != _indexStats.end() ;
            ++ iter )
      {
         SDB_OSS_DEL iter->second ;
      }
      _indexStats.clear() ;
      _fieldStats.clear() ;

      PD_TRACE_EXIT( SDB_DMSCLSTAT_CLEARSUBUNITS ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_RMIDXSTAT, "_dmsCollectionStat::removeIndexStat" )
   BOOLEAN _dmsCollectionStat::removeIndexStat ( const CHAR *pIndexName,
                                                 BOOLEAN findNewFieldStat )
   {
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB_DMSCLSTAT_RMIDXSTAT ) ;

      if ( pIndexName )
      {
         INDEX_STAT_ITERATOR iter = _indexStats.find( pIndexName ) ;
         if ( iter != _indexStats.end() )
         {
            dmsIndexStat *pDeletingStat = iter->second ;
            if ( pDeletingStat )
            {
               if ( findNewFieldStat )
               {
                  _findNewFieldStat( pDeletingStat ) ;
               }
               else
               {
                  _removeFieldStat( pDeletingStat ) ;
               }
            }
            _indexStats.erase( iter ) ;
            SAFE_OSS_DELETE( pDeletingStat ) ;
            deleted = TRUE ;
         }
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_RMIDXSTAT ) ;

      return deleted ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_RMFLDSTAT, "_dmsCollectionStat::removeFieldStat" )
   BOOLEAN _dmsCollectionStat::removeFieldStat ( const CHAR *pFieldName,
                                                 BOOLEAN findNewFieldStat )
   {
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB_DMSCLSTAT_RMFLDSTAT ) ;

      if ( pFieldName )
      {
         INDEX_STAT_ITERATOR iter = _fieldStats.find( pFieldName ) ;
         if ( iter != _fieldStats.end() )
         {
            dmsIndexStat *pDeletingStat = iter->second ;
            if ( pDeletingStat )
            {
               if ( findNewFieldStat )
               {
                  _findNewFieldStat( pDeletingStat ) ;
               }
               else
               {
                  _removeFieldStat( pDeletingStat ) ;
               }
            }
            // Erase item only, no need to delete statistics
            _fieldStats.erase( iter ) ;
            deleted = TRUE ;
         }
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_RMFLDSTAT ) ;

      return deleted ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_GETIDXSTAT, "_dmsCollectionStat::getIndexStat" )
   const dmsIndexStat * _dmsCollectionStat::getIndexStat ( const CHAR *pIndexName ) const
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_GETIDXSTAT ) ;

      const dmsIndexStat *pIndexStat = NULL ;
      INDEX_STAT_CONST_ITERATOR iter = _indexStats.find( pIndexName ) ;

      if ( iter != _indexStats.end() )
      {
         pIndexStat = iter->second ;
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_GETIDXSTAT ) ;

      return pIndexStat ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_GETFLDSTAT, "_dmsCollectionStat::getFieldStat" )
   const dmsIndexStat * _dmsCollectionStat::getFieldStat ( const CHAR *pFieldName ) const
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_GETFLDSTAT ) ;

      const dmsIndexStat *pFieldStat = NULL ;
      INDEX_STAT_CONST_ITERATOR iter = _fieldStats.find( pFieldName ) ;

      if ( iter != _fieldStats.end() )
      {
         pFieldStat = iter->second ;
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_GETFLDSTAT ) ;

      return pFieldStat ;
   }

   void _dmsCollectionStat::_initItems ()
   {
      _totalDataPages = DMS_STAT_DEF_TOTAL_PAGES ;
      _totalDataSize = DMS_STAT_DEF_DATA_SIZE * DMS_STAT_DEF_TOTAL_RECORDS ;
      _avgNumFields = DMS_STAT_DEF_AVG_NUM_FIELDS ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT__INITITEM, "_dmsCollectionStat::_initItem" )
   INT32 _dmsCollectionStat::_initItem ( const BSONElement &beItem )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSCLSTAT__INITITEM ) ;

      if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_CL_TOTAL_DATA_PAGES ) )
      {
         // Required
         PD_CHECK( beItem.isNumber(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", DMS_STAT_CL_TOTAL_DATA_SIZE ) ;
         _totalDataPages = (UINT32)beItem.numberInt() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_CL_TOTAL_DATA_SIZE ) )
      {
         // Required
         PD_CHECK( beItem.isNumber(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", DMS_STAT_CL_TOTAL_DATA_SIZE ) ;
         _totalDataSize = (UINT64)beItem.numberLong() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), DMS_STAT_CL_AVG_NUM_FIELDS ) )
      {
         // Optional
         if ( beItem.isNumber() )
         {
            _avgNumFields = (UINT32)beItem.numberInt() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Field [%s] is not matched", DMS_STAT_CL_AVG_NUM_FIELDS ) ;
         }
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSCLSTAT__INITITEM, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_ADDFLDSTAT, "_dmsCollectionStat::addFieldStat" )
   void _dmsCollectionStat::_addFieldStat ( dmsIndexStat *pIndexStat,
                                            BOOLEAN ignoreVersion )
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_ADDFLDSTAT ) ;

      if ( pIndexStat &&
           pIndexStat->isValid() &&
           pIndexStat->getNumKeys() > 0 )
      {
         const CHAR *pFirstField = pIndexStat->getFirstField() ;
         INDEX_STAT_ITERATOR iter = _fieldStats.find( pFirstField ) ;
         if ( iter != _fieldStats.end() )
         {
            dmsIndexStat *pTempStat = iter->second ;

            if ( pIndexStat->getNumKeys() < pTempStat->getNumKeys() )
            {
               _fieldStats[ pFirstField ] = pIndexStat ;
            }
            else if ( pTempStat->getNumKeys() == pIndexStat->getNumKeys() &&
                      ( ignoreVersion ||
                        pTempStat->getVersion() < pIndexStat->getVersion() ) )
            {
               _fieldStats[ pFirstField ] = pIndexStat ;
            }
         }
         else
         {
            _fieldStats[ pFirstField ] = pIndexStat ;
         }
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_ADDFLDSTAT ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_RMFLDSTAT, "_dmsCollectionStat::removeFieldStat" )
   void _dmsCollectionStat::_removeFieldStat ( dmsIndexStat *pDeletingStat )
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_RMFLDSTAT ) ;

      const CHAR *pFieldName = pDeletingStat->getFirstField() ;
      INDEX_STAT_ITERATOR iterField = _fieldStats.find( pFieldName ) ;

      if ( iterField->second == pDeletingStat )
      {
         _fieldStats.erase( iterField ) ;
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_RMFLDSTAT ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_FINDFLDSTAT, "_dmsCollectionStat::_findNewFieldStat" )
   void _dmsCollectionStat::_findNewFieldStat ( dmsIndexStat *pDeletingStat )
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_RMFLDSTAT ) ;

      dmsIndexStat *pNewFieldStat = NULL ;
      const CHAR *pFieldName = pDeletingStat->getFirstField() ;
      INDEX_STAT_ITERATOR iterField = _fieldStats.find( pFieldName ) ;

      if ( iterField != _fieldStats.end() && pDeletingStat == iterField->second )
      {
         for ( INDEX_STAT_ITERATOR iterIndex = _indexStats.begin() ;
               iterIndex != _indexStats.end() ;
               ++ iterIndex )
         {
            dmsIndexStat *pTempFieldStat = iterIndex->second ;
            if ( pTempFieldStat != pDeletingStat &&
                 0 == ossStrcmp( pFieldName, pTempFieldStat->getFirstField() ) )
            {
               if ( !pNewFieldStat )
               {
                  pNewFieldStat = pTempFieldStat ;
               }
               else if ( pTempFieldStat->getNumKeys() < pNewFieldStat->getNumKeys() )
               {
                  pNewFieldStat = pTempFieldStat ;
               }
               else if ( pTempFieldStat->getNumKeys() == pNewFieldStat->getNumKeys() &&
                         pTempFieldStat->getVersion() < pNewFieldStat->getVersion() )
               {
                  pNewFieldStat = pTempFieldStat ;
               }
            }
         }
         _fieldStats.erase( iterField ) ;
         if ( pNewFieldStat )
         {
            _fieldStats[ pNewFieldStat->getFirstField() ] = pNewFieldStat ;
         }
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_RMFLDSTAT ) ;
   }

   /*
      _dmsStatCache define
    */
   _dmsStatCache::_dmsStatCache ( _IUtilSUCacheHolder *pHolder )
   : utilSUCache( DMS_MME_SLOTS, DMS_CACHE_TYPE_STAT,
                  UTIL_SU_CACHE_UNIT_CLSTAT, pHolder )
   {
   }

}

