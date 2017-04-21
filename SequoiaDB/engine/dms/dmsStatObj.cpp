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

   Source File Name = dmsStatObj.cpp

   Descriptive Name = Data Management Service Statistics Objects

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

#include "dmsStatObj.hpp"
#include "../bson/bson.h"
#include "rtn.hpp"
#include "pdTrace.hpp"
#include "dmsTrace.hpp"
#include "optCommon.hpp"

using namespace bson ;

namespace engine
{

   /*
      _dmsStatListKey implement
    */
   _dmsStatListKey::_dmsStatListKey ()
   : _utilList< BSONElement > ()
   {
   }

   _dmsStatListKey::_dmsStatListKey ( const BSONElement &beValue )
   : _utilList< BSONElement > ()
   {
      push_back( beValue ) ;
   }

   INT32 _dmsStatListKey::compareValue ( INT32 incFlag,
                                         const BSONObj &rValue )
   {
      INT32 res = 0 ;
      _dmsStatListKey::iterator iterLeft = begin() ;
      BSONObjIterator iterRight( rValue ) ;

      while ( iterLeft != end() &&
              iterRight.more() )
      {
         BSONElement beLeft = *iterLeft ;
         iterLeft ++ ;

         BSONElement beRight = iterRight.next() ;

         res = beLeft.woCompare( beRight, FALSE ) ;

         if ( 0 != res )
         {
            break ;
         }
      }

      if ( 0 == res )
      {
         if ( iterRight.more() )
         {
            res = _compareRightMore( incFlag ) ;
         }
         else if ( iterLeft != end() )
         {
            res = _compareLeftMore( incFlag ) ;
         }
         else
         {
            res = _compareDefault( incFlag ) ;
         }
      }

      return res ;
   }

   string _dmsStatListKey::toString ()
   {
      if ( empty() )
      {
         return "{}" ;
      }

      BOOLEAN first = TRUE ;
      StringBuilder s ;
      s << "{ " ;

      iterator iter = begin() ;
      while ( iter != end() )
      {
         if ( first )
         {
            first = FALSE ;
         }
         else
         {
            s << ", " ;
         }

         (*iter).toString( s, TRUE, TRUE ) ;

         iter ++ ;
      }

      s << " }" ;

      return s.str() ;
   }

   /*
      _dmsStatElementKey implement
    */
   _dmsStatElementKey::_dmsStatElementKey ( const BSONElement &element )
   : BSONElement( element )
   {
   }

   INT32 _dmsStatElementKey::compareValue ( INT32 incFlag,
                                            const BSONObj &rValue )
   {
      INT32 res = 0 ;
      BSONObjIterator iterRight( rValue ) ;

      if ( iterRight.more() )
      {
         BSONElement beRight = iterRight.next() ;
         res = woCompare( beRight, FALSE ) ;
      }

      if ( 0 == res )
      {
         if ( iterRight.more() )
         {
            res = _compareRightMore( incFlag ) ;
         }
         else if ( rValue.nFields() == 0 )
         {
            res = _compareLeftMore( incFlag ) ;
         }
         else
         {
            res = _compareDefault( incFlag ) ;
         }
      }

      return res ;
   }

   /*
      _dmsStatValues implement
    */
   _dmsStatValues::_dmsStatValues ()
   {
      _size = 0 ;
      _pValues = NULL ;
   }

   _dmsStatValues::~_dmsStatValues ()
   {
      _clear() ;
   }

   INT32 _dmsStatValues::binarySearch ( dmsStatKey &keyValue,
                                        INT32 keyIncFlag,
                                        BOOLEAN &isEqual ) const
   {
      isEqual = FALSE ;

      INT32 low = 0, high = _size - 1, mid ;
      INT32 index = -1 ;
      BSONObj boEmpty ;

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

   void _dmsStatValues::_clear ()
   {
      if ( _pValues )
      {
         delete [] _pValues ;
         _pValues = NULL ;
      }
      _size = 0 ;
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
   INT32 _dmsStatMCVSet::evalOperator ( dmsStatKey *pStartKey, BOOLEAN startIncluded,
                                        dmsStatKey *pStopKey, BOOLEAN stopIncluded,
                                        UINT32 numKeys, BOOLEAN &hitMCV,
                                        double &selectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATMCV_EVALOPTR ) ;

      BOOLEAN startEqual = FALSE, stopEqual = FALSE ;
      INT32 startIdx = -1, stopIdx = -1 ;
      UINT32 rangeCount = 0 ;
      double tempSelectivity = 0.0 ;

      PD_CHECK( getSize() > 0, SDB_INVALIDARG, error, PDWARNING,
                "No MCV set is available" ) ;

      // Get the index of start key
      if ( pStartKey )
      {
         INT32 startFlag = startIncluded ? -1 :
                           ( numKeys == pStartKey->size() ? 0 : 1 ) ;
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
         INT32 stopFlag = stopIncluded ? 1 :
                          ( numKeys == pStopKey->size() ? 0 : -1 ) ;
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

      if ( startIdx != stopIdx )
      {
         // Check startIdx
         if ( startIncluded || !startEqual )
         {
            // The start key <= value case, add the first selected MCV item
            tempSelectivity += getFrac( startIdx ) ;
            rangeCount ++ ;
         }

         // Check startIdx + 1 to stopIdx - 1
         for ( INT32 idx = startIdx + 1 ; idx < stopIdx ; idx ++ )
         {
            // Every ranges between selected MCV items are needed
            tempSelectivity += getFrac( idx ) ;
            rangeCount ++ ;
         }

         // Check stopIdx
         if ( stopIncluded && stopEqual && stopIdx < (INT32)getSize() )
         {
            // The stop key is equal to the last selected MCV item, add
            // the last selected MCV item
            tempSelectivity += getFrac( stopIdx ) ;
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
            tempSelectivity = getFrac( startIdx ) ;
            rangeCount ++ ;
         }
      }

      selectivity = OPT_ROUND_SELECTIVITY( tempSelectivity ) ;
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
                                          double &selectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATMCV_EVALETOPTR );

      BOOLEAN equal = FALSE ;
      INT32 idx = -1 ;

      PD_CHECK( getSize() > 0, SDB_INVALIDARG, error, PDWARNING,
                "No MCV set is available" ) ;

      idx = binarySearch( key, 0, equal ) ;
      PD_CHECK( idx >= 0, SDB_INVALIDARG, error, PDWARNING,
                "Failed to locate start key %s in MCV set",
                key.toString().c_str() ) ;

      if ( idx < (INT32)getSize() && equal )
      {
         selectivity = getFrac( idx ) ;
         hitMCV = TRUE ;
      }
      else
      {
         hitMCV = FALSE ;
      }

      selectivity = OPT_ROUND_SELECTIVITY( selectivity ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATMCV_EVALETOPTR, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
      _dmsStatBase implement
    */
   _dmsStatBase::_dmsStatBase ( const CHAR *pCSName,
                                const CHAR *pCLName )
   {
      setCSName( pCSName ) ;
      setCLName( pCLName ) ;

      _mbID = DMS_INVALID_MBID ;
      _version = 0 ;
      _sampleRecords = STAT_DEF_TOTAL_RECORDS ;
      _totalRecords = STAT_DEF_TOTAL_RECORDS ;
   }

   void _dmsStatBase::setCLName ( const CHAR *pCLName )
   {
      ossMemset ( _pCLName, 0, sizeof( _pCLName ) ) ;
      if ( pCLName )
      {
         ossStrncpy ( _pCLName, pCLName, sizeof( _pCLName ) ) ;
      }
   }

   void _dmsStatBase::setCSName ( const CHAR *pCSName )
   {
      ossMemset ( _pCSName, 0, sizeof( _pCSName ) ) ;
      if ( pCSName )
      {
         ossStrncpy ( _pCSName, pCSName, sizeof( _pCSName ) ) ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATBASE_INIT, "_dmsStatBase::init" )
   INT32 _dmsStatBase::init ( const BSONObj &boStat )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATBASE_INIT ) ;

      try
      {
         BSONObjIterator iter( boStat ) ;

         while ( iter.more() )
         {
            BSONElement beItem = iter.next() ;

            if ( 0 == ossStrcmp( beItem.fieldName(), STAT_COLLECTION_SPACE ) )
            {
               const CHAR *pCSName = NULL ;

               PD_CHECK( String == beItem.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", STAT_COLLECTION_SPACE ) ;

               pCSName = beItem.valuestr() ;

               if ( '\0' != _pCSName[0] )
               {
                  PD_CHECK( 0 == ossStrncmp( pCSName, _pCSName, sizeof( _pCSName ) ),
                            SDB_INVALIDARG, error, PDWARNING,
                            "Field [%s] is not matched, expected is %s, "
                            "given is %s", STAT_COLLECTION_SPACE, _pCSName,
                            pCSName ) ;
               }
               else
               {
                  setCSName( pCSName ) ;
               }
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_COLLECTION ) )
            {
               const CHAR *pCLName = NULL ;

               PD_CHECK( String == beItem.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", STAT_COLLECTION ) ;

               pCLName = beItem.valuestr() ;

               if ( '\0' != _pCLName[0] )
               {
                  PD_CHECK( 0 == ossStrncmp( pCLName, _pCLName, sizeof( _pCLName ) ),
                            SDB_INVALIDARG, error, PDWARNING,
                            "Field [%s] is not matched, expected is %s, "
                            "given is %s", STAT_COLLECTION, _pCLName,
                            pCLName ) ;
               }
               else
               {
                  setCLName( pCLName ) ;
               }
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_COLLECTION_MBID ) )
            {
               PD_CHECK( NumberInt == beItem.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", STAT_COLLECTION_MBID ) ;
               _mbID = (UINT32)beItem.Int() ;
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_VERSION ) )
            {
               PD_CHECK( NumberLong == beItem.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", STAT_VERSION ) ;
               _version = (UINT64)beItem.Long() ;
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_SAMPLE_RECORDS ) )
            {
               PD_CHECK( NumberLong == beItem.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", STAT_SAMPLE_RECORDS ) ;
               _sampleRecords = (UINT64)beItem.Long() ;
            }
            else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_TOTAL_RECORDS ) )
            {
               PD_CHECK( NumberLong == beItem.type(),
                         SDB_INVALIDARG, error, PDWARNING,
                         "Field [%s] is not matched", STAT_TOTAL_RECORDS ) ;
               _totalRecords = (UINT64)beItem.Long() ;
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

   /*
      _dmsIndexStat implement
    */
   _dmsIndexStat::_dmsIndexStat ( const CHAR *pCSName, const CHAR *pCLName,
                                  const CHAR *pIndexName )
   : _dmsStatBase( pCSName, pCLName )
   {
      setIndexName( pIndexName ) ;

      _pFirstField = NULL ;
      _numKeys = 0 ;

      _indexPages = STAT_DEF_TOTAL_PAGES ;
      _indexLevels = STAT_DEF_IDX_LEVELS ;

      _isUnique = FALSE ;
      _distinctValues = 0 ;

      _avgFieldSize = 0 ;

      _nullFrac = 0.0 ;
      _undefFrac = 0.0 ;
   }

   _dmsIndexStat::~_dmsIndexStat ()
   {
   }

   void _dmsIndexStat::setIndexName ( const CHAR *pIndexName )
   {
      ossMemset( _pIndexName, 0, sizeof( _pIndexName ) ) ;
      if ( pIndexName )
      {
         ossMemcpy( _pIndexName, pIndexName, ossStrlen( pIndexName ) ) ;
      }
   }



   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT_EVALRANGEOPTR, "_dmsIndexStat::evalRangeOperator" )
   INT32 _dmsIndexStat::evalRangeOperator ( dmsStatKey &startKey,
                                            BOOLEAN startIncluded,
                                            dmsStatKey &stopKey,
                                            BOOLEAN stopIncluded,
                                            double &selectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT_EVALRANGEOPTR ) ;

      rc = _evalOperator( &startKey, startIncluded, &stopKey, stopIncluded,
                          selectivity ) ;

      PD_TRACE_EXITRC( SDB_DMSIDXSTAT_EVALRANGEOPTR, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT_EVALETOPTR, "_dmsIndexStat::evalETOperator" )
   INT32 _dmsIndexStat::evalETOperator ( dmsStatKey &key,
                                         double &selectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT_EVALETOPTR );

      BOOLEAN hitMCV = FALSE ;

      // Special case for unique index, could be one of the totalRecords
      if ( _isUnique && key.size() == _numKeys )
      {
         selectivity = 1.0 / (double)_totalRecords ;
         goto done ;
      }

      PD_CHECK( _mcvSet.getSize() > 0, SDB_INVALIDARG, error, PDWARNING,
                "No MCV set is available" ) ;

      rc = _mcvSet.evalETOperator( key, hitMCV, selectivity ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to evaluate from MCV set, rc: %d", rc ) ;

      if ( !hitMCV )
      {
         // The value is not in MCV set, evaluate in the rest of values
         if ( _distinctValues == _mcvSet.getSize() )
         {
            selectivity = ( 1.0 - _mcvSet.getTotalFrac() ) *
                          OPT_PRED_EQ_DEF_SELECTIVITY ;
         }
         else
         {
            selectivity = ( 1.0 - _mcvSet.getTotalFrac() ) /
                          (double) ( _distinctValues - _mcvSet.getSize() ) ;
         }
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSIDXSTAT_EVALETOPTR, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT_EVALGTOPTR, "_dmsIndexStat::evalGTOperator" )
   INT32 _dmsIndexStat::evalGTOperator ( dmsStatKey &startKey,
                                         BOOLEAN startIncluded,
                                         double &selectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT_EVALGTOPTR );

      rc = _evalOperator( &startKey, startIncluded, NULL, TRUE,
                          selectivity ) ;

      PD_TRACE_EXITRC( SDB_DMSIDXSTAT_EVALGTOPTR, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT_EVALLTOPTR, "_dmsIndexStat::evalLTOperator" )
   INT32 _dmsIndexStat::evalLTOperator ( dmsStatKey &stopKey,
                                         BOOLEAN stopIncluded,
                                         double &selectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT_EVALLTOPTR );

      rc = _evalOperator( NULL, TRUE, &stopKey, stopIncluded,
                          selectivity ) ;

      PD_TRACE_EXITRC( SDB_DMSIDXSTAT_EVALLTOPTR, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSIDXSTAT__EVALOPTR, "_dmsIndexStat::_evalOperator" )
   INT32 _dmsIndexStat::_evalOperator ( dmsStatKey *pStartKey,
                                        BOOLEAN startIncluded,
                                        dmsStatKey *pStopKey,
                                        BOOLEAN stopIncluded,
                                        double &selectivity ) const
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSIDXSTAT__EVALOPTR ) ;

      BOOLEAN hitMCV = FALSE ;

      PD_CHECK( _mcvSet.getSize() > 0, SDB_INVALIDARG, error, PDWARNING,
                "No MCV set is available" ) ;

      rc = _mcvSet.evalOperator( pStartKey, startIncluded,
                                 pStopKey, stopIncluded,
                                 _numKeys, hitMCV, selectivity ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to evaluate from MCV set, rc: %d", rc ) ;

      if ( !hitMCV )
      {
         // The values do not include any MCV items, try to evaluate from
         // the rest of values
         selectivity = ( 1.0 - _mcvSet.getTotalFrac() ) *
                       OPT_PRED_RANGE_DEF_SELECTIVITY ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSIDXSTAT__EVALOPTR, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsIndexStat::_initItem ( const BSONElement &beItem )
   {
      INT32 rc = SDB_OK ;

      if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_INDEX ) )
      {
         const CHAR *pIndexName = NULL ;

         PD_CHECK( String == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", STAT_IDX_INDEX ) ;

         pIndexName = beItem.valuestr() ;

         if ( '\0' != _pIndexName[0] )
         {
            PD_CHECK( 0 == ossStrncmp( pIndexName, _pIndexName, sizeof( _pIndexName ) ),
                      SDB_INVALIDARG, error, PDWARNING,
                      "Field [%s] is not matched, expected is %s, "
                      "given is %s", STAT_IDX_INDEX, _pIndexName,
                      pIndexName ) ;
         }
         else
         {
            setIndexName( pIndexName ) ;
         }
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_INDEX_PAGES ) )
      {
         PD_CHECK( NumberInt == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", STAT_IDX_INDEX_PAGES ) ;
         _indexPages = (UINT32)beItem.Int() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_LEVELS ) )
      {
         PD_CHECK( NumberInt == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", STAT_IDX_LEVELS ) ;
         _indexLevels = (UINT32)beItem.Int() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_KEY_PATTERN ) )
      {
         PD_CHECK( Object == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", STAT_IDX_KEY_PATTERN ) ;
         rc = _initKeyPattern ( beItem.embeddedObject() ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to init key pattern, rc: %d",
                      rc ) ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_IS_UNIQUE ) )
      {
         PD_CHECK( Bool == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", STAT_IDX_IS_UNIQUE ) ;
         _isUnique = beItem.Bool() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_NULL_FRAC ) )
      {
         if ( beItem.isNumber() )
         {
            _nullFrac = beItem.Number() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Type of field [%s] is not matched, "
                    "expected is %d, actually is %d",
                    STAT_IDX_NULL_FRAC, NumberDouble, beItem.type() ) ;
         }
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_UNDEF_FRAC ) )
      {
         if ( beItem.isNumber() )
         {
            _undefFrac = beItem.Number() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Field [%s] is not matched", STAT_IDX_UNDEF_FRAC ) ;
         }
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_DISTINCT_VALUES ) )
      {
         if ( NumberLong == beItem.type() )
         {
            _distinctValues = (UINT64)beItem.Long() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Field [%s] is not matched", STAT_IDX_DISTINCT_VALUES ) ;
         }
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_IDX_MCV ) )
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
                    "Field [%s] is not matched", STAT_IDX_MCV ) ;
         }
      }

      rc = SDB_OK ;

   done :
      return rc ;

   error :
      goto done ;
   }

   INT32 _dmsIndexStat::_initKeyPattern ( const BSONObj &boKeyPattern )
   {
      INT32 rc = SDB_OK ;

      _keyPattern = boKeyPattern.copy() ;
      _numKeys = _keyPattern.nFields() ;

      PD_CHECK( _numKeys > 0, SDB_INVALIDARG, error, PDWARNING,
                "Empty key pattern" ) ;

      _pFirstField = _keyPattern.firstElementFieldName() ;

   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsIndexStat::_initMCV ( const BSONObj &boMCV )
   {
      INT32 rc = SDB_OK ;

      double totalFrac = 0.0 ;

      BSONObjIterator iter( boMCV ) ;
      while ( iter.more() )
      {
         BSONElement eleTemp = iter.next() ;

         if ( 0 == ossStrcmp( eleTemp.fieldName(), STAT_IDX_MCV_VALUES ) )
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
         else if ( 0 == ossStrcmp( eleTemp.fieldName(), STAT_IDX_MCV_FRAC ) )
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

      totalFrac = OPT_ROUND_SELECTIVITY( totalFrac ) ;
      _mcvSet.setTotalFrac( totalFrac ) ;

   done :
      return rc ;
   error :
      _mcvSet.clear() ;
      goto done ;
   }

   INT32 _dmsIndexStat::_postInit ()
   {
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

      return SDB_OK ;
   }

   /*
      _dmsCollectionStat implement
    */
   _dmsCollectionStat::_dmsCollectionStat ( const CHAR *pCSName,
                                            const CHAR *pCLName )
   : _dmsStatBase( pCSName, pCLName )
   {
      _totalDataLen = STAT_DEF_DATA_LEN * STAT_DEF_TOTAL_RECORDS ;
      _avgNumFields = STAT_DEF_AVG_NUM_FIELDS ;
   }

   _dmsCollectionStat::~_dmsCollectionStat ()
   {
      clearStats() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_ADDIDXSTAT, "_dmsCollectionStat::addIndexStat" )
   BOOLEAN _dmsCollectionStat::addIndexStat ( dmsIndexStat *pIndexStat,
                                              BOOLEAN ignoreVersion )
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_ADDIDXSTAT ) ;

      BOOLEAN added = FALSE ;

      if ( pIndexStat )
      {
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
      }

      if ( added )
      {
         addFieldStat( pIndexStat, ignoreVersion ) ;
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_ADDIDXSTAT ) ;

      return added ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_ADDFLDSTAT, "_dmsCollectionStat::addFieldStat" )
   BOOLEAN _dmsCollectionStat::addFieldStat ( dmsIndexStat *pIndexStat,
                                              BOOLEAN ignoreVersion )
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_ADDFLDSTAT ) ;

      BOOLEAN added = FALSE ;

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
               added = TRUE ;
            }
            else if ( pTempStat->getNumKeys() == pIndexStat->getNumKeys() &&
                      ( ignoreVersion ||
                        pTempStat->getVersion() < pIndexStat->getVersion() ) )
            {
               _fieldStats[ pFirstField ] = pIndexStat ;
               added = TRUE ;
            }
         }
         else
         {
            _fieldStats[ pFirstField ] = pIndexStat ;
            added = TRUE ;
         }
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_ADDFLDSTAT ) ;

      return added ;
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
            if ( pDeletingStat && findNewFieldStat )
            {
               _findNewFieldStat( pDeletingStat->getFirstField(),
                                  pDeletingStat ) ;
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
   void _dmsCollectionStat::removeFieldStat ( const CHAR *pFieldName )
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_RMFLDSTAT ) ;

      if ( pFieldName )
      {
         _fieldStats.erase( pFieldName ) ;
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_RMFLDSTAT ) ;
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

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSCLSTAT_GETMATCHEDIDX, "_dmsCollectionStat::getMatchedIndex" )
   const dmsIndexStat * _dmsCollectionStat::getMatchedIndex ( rtnPredicateSet &predicateSet ) const
   {
      PD_TRACE_ENTRY( SDB_DMSCLSTAT_GETMATCHEDIDX ) ;

      RTN_PREDICATE_MAP &predicates = predicateSet.predicates() ;

      if ( predicates.size() == 0 )
      {
         return NULL ;
      }
      else if ( predicates.size() == 1 )
      {
         RTN_PREDICATE_MAP::const_iterator iterPred = predicates.begin() ;
         return getFieldStat( iterPred->first.c_str() ) ;
      }

      const dmsIndexStat *pBestIndexStat = NULL ;

      for ( INDEX_STAT_CONST_ITERATOR iter = _indexStats.begin() ;
            iter != _indexStats.end() ;
            ++ iter )
      {
         const dmsIndexStat *pIndexStat = iter->second ;

         if ( pIndexStat->getNumKeys() < predicates.size() )
         {
            continue ;
         }

         BSONObjIterator iterKey( pIndexStat->getKeyPattern() ) ;
         UINT32 matchedCount = 0 ;
         while ( iterKey.more() )
         {
            BSONElement beKey = iterKey.next() ;
            RTN_PREDICATE_MAP::iterator iterPred = predicates.find( beKey.fieldName() ) ;
            if ( iterPred == predicates.end() ||
                 iterPred->second.isEmpty() )
            {
               break ;
            }
            rtnPredicate &predicate = iterPred->second ;
            if ( predicate._startStopKeys.size() == 1 &&
                 predicate.isEquality() )
            {
               // Equal operator
               matchedCount ++ ;
            }
            else if ( matchedCount == predicates.size() - 1 )
            {
               // Last one, on matters
               matchedCount ++ ;
            }
            else if ( predicate.isAllEqual() )
            {
               matchedCount ++ ;
            }
         }

         if ( matchedCount == predicates.size() )
         {
            if ( matchedCount == pIndexStat->getNumKeys() )
            {
               return pIndexStat ;
            }

            if ( pBestIndexStat )
            {
               if ( pIndexStat->getNumKeys() < pBestIndexStat->getNumKeys() )
               {
                  pBestIndexStat = pIndexStat ;
               }
            }
            else
            {
               pBestIndexStat = pIndexStat ;
            }
         }
      }

      PD_TRACE_EXIT( SDB_DMSCLSTAT_GETMATCHEDIDX ) ;

      return pBestIndexStat ;
   }

   void _dmsCollectionStat::clearStats ()
   {
      INDEX_STAT_ITERATOR iter ;

      for ( iter = _indexStats.begin() ;
            iter != _indexStats.end() ;
            ++ iter )
      {
         SDB_OSS_DEL iter->second ;
      }
      _indexStats.clear() ;
      _fieldStats.clear() ;
   }

   void _dmsCollectionStat::setCLName ( const CHAR *pCLName )
   {
      _dmsStatBase::setCLName( pCLName ) ;

      for ( INDEX_STAT_ITERATOR iter = _indexStats.begin() ;
            iter != _indexStats.end() ;
            ++ iter )
      {
         dmsIndexStat *pIndexStat = iter->second ;
         if ( pIndexStat )
         {
            pIndexStat->setCLName( pCLName ) ;
         }
      }
   }

   void _dmsCollectionStat::setCSName ( const CHAR *pCSName )
   {
      _dmsStatBase::setCSName( pCSName ) ;

      for ( INDEX_STAT_ITERATOR iter = _indexStats.begin() ;
            iter != _indexStats.end() ;
            ++ iter )
      {
         dmsIndexStat *pIndexStat = iter->second ;
         if ( pIndexStat )
         {
            pIndexStat->setCSName( pCSName ) ;
         }
      }
   }

   INT32 _dmsCollectionStat::_initItem ( const BSONElement &beItem )
   {
      INT32 rc = SDB_OK ;

      if ( 0 == ossStrcmp( beItem.fieldName(), STAT_CL_TOTAL_DATA_LEN ) )
      {
         PD_CHECK( NumberLong == beItem.type(),
                   SDB_INVALIDARG, error, PDWARNING,
                   "Field [%s] is not matched", STAT_CL_TOTAL_DATA_LEN ) ;
         _totalDataLen = (UINT64)beItem.Long() ;
      }
      else if ( 0 == ossStrcmp( beItem.fieldName(), STAT_CL_AVG_NUM_FIELDS ) )
      {
         if ( NumberInt == beItem.type() )
         {
            _avgNumFields = (UINT64)beItem.Int() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Field [%s] is not matched", STAT_CL_AVG_NUM_FIELDS ) ;
         }
      }

   done :
      return rc ;

   error :
      goto done ;
   }

   void _dmsCollectionStat::_findNewFieldStat ( const CHAR *pFieldName,
                                                dmsIndexStat *pDeletingStat )
   {
      dmsIndexStat *pNewFieldStat = NULL ;
      INDEX_STAT_ITERATOR iterField = _fieldStats.find( pFieldName ) ;

      if ( iterField != _fieldStats.end() &&
           pDeletingStat == iterField->second )
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
   }

}

