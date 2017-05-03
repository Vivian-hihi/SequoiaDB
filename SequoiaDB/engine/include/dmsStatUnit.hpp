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

   Source File Name = dmsStatsUnit.hpp

   Descriptive Name = DMS Statistics Units Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   statistics objects.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#ifndef DMSSTATUNIT_HPP__
#define DMSSTATUNIT_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "dms.hpp"
#include "ixm.hpp"
#include "utilMap.hpp"
#include "utilList.hpp"
#include "utilSUCache.hpp"
#include "../bson/bson.h"

using namespace std ;
using namespace bson ;

namespace engine
{
   // Fields to create indexes of SYSSTAT collections
   #define DMS_STAT_COLLECTION_SPACE           "CollectionSpace"
   #define DMS_STAT_COLLECTION                 "Collection"
   #define DMS_STAT_COLLECTION_MBID            "CollectionMBID"

   #define DMS_STAT_IDX_INDEX                  "Index"

   #define DMS_STAT_DEF_VERSION                ( 0 )
   #define DMS_STAT_DEF_AVG_NUM_FIELDS         ( 10 )
   #define DMS_STAT_DEF_TOTAL_PAGES            ( 1 )
   #define DMS_STAT_DEF_IDX_LEVELS             ( 1 )
   #define DMS_STAT_DEF_TOTAL_RECORDS          ( 10 )
   #define DMS_STAT_DEF_DATA_SIZE              ( 400 )

   // Default selectivity of a range predicate
   #define DMS_STAT_PRED_RANGE_DEF_SELECTIVITY ( 0.05 )

   // Default selectivity of a $et predicate
   #define DMS_STAT_PRED_EQ_DEF_SELECTIVITY    ( 0.005 )

   #define DMS_STAT_ROUND( x, min, max )       ( OSS_MIN( OSS_MAX( ( x ), ( min ) ), ( max ) ) )
   #define DMS_STAT_ROUND_SELECTIVITY( x )     DMS_STAT_ROUND( ( x ), ( 0.0 ), ( 1.0 ) )

   /*
      _dmsStatKey define
    */
   class _dmsStatKey
   {
      public :
         _dmsStatKey ( BOOLEAN included = TRUE ) { _included = included ; }

         virtual ~_dmsStatKey () {}

         virtual INT32 compareValue ( INT32 incFlag,
                                      const BSONObj &rValue ) = 0 ;

         virtual BOOLEAN compareAllValues ( UINT32 startIdx, INT32 expRes,
                                            const BSONObj &rValue ) = 0 ;

         virtual string toString () = 0 ;

         virtual UINT32 size () = 0 ;

         virtual const BSONElement &firstElement () = 0 ;

         OSS_INLINE BOOLEAN isIncluded () const
         {
            return _included ;
         }

         OSS_INLINE void setIncluded ( BOOLEAN included )
         {
            _included = included ;
         }

      protected :
         OSS_INLINE INT32 _equalButLeftMore ( INT32 incFlag )
         {
            // The compared elements are equal, but left has more elements,
            // normally it is left > right
            // But if incFlag is -1 which means a virtual $minKey is appended
            // left, so right > left
            return incFlag < 0 ? -1 : 1 ;
         }

         OSS_INLINE INT32 _equalButRightMore ( INT32 incFlag )
         {
            // The compared elements are equal, but right has more elements,
            // normally it is left < right
            // But if incFlag is 1 which means a virtual $maxKey is appended to
            // left, so left > right
            return incFlag > 0 ? 1 : -1 ;
         }

         OSS_INLINE INT32 _equalDefault ( INT32 incFlag )
         {
            // The compared elements are equal
            // If $maxKey is appended to left, left > right
            // If $minKey is appended to left, left < right
            return incFlag > 0 ? 1 : ( incFlag < 0 ? -1 : 0 ) ;
         }

      protected :
         BOOLEAN _included ;
   } ;

   typedef class _dmsStatKey dmsStatKey ;

   /*
      _dmsStatSubUnitKey define
    */
   typedef struct _dmsStatSubUnitKey dmsStatSubUnitKey ;

   struct _dmsStatSubUnitKey
   {
      OSS_INLINE _dmsStatSubUnitKey ( const CHAR * pName = NULL )
      {
         _pName = pName ;
      }

      const CHAR * _pName ;

      OSS_INLINE bool operator== ( const dmsStatSubUnitKey &objName ) const
      {
         return ossStrcmp( _pName, objName._pName ) == 0 ;
      }

      OSS_INLINE bool operator< ( const dmsStatSubUnitKey &objName ) const
      {
         return ossStrcmp( _pName, objName._pName ) < 0 ;
      }
   } ;

   /*
      _dmsStatValues define
    */
   class _dmsStatValues : public SDBObject
   {
      public :
         _dmsStatValues () ;

         virtual ~_dmsStatValues () ;

         INT32 init ( UINT32 size ) ;

         INT32 binarySearch ( dmsStatKey &keyValue, INT32 keyIncFlag,
                              BOOLEAN &isEqual ) const ;

         OSS_INLINE UINT32 getSize () const
         {
            return _size ;
         }

         OSS_INLINE void setValue ( UINT32 idx, const BSONObj &boValue )
         {
            if ( idx < _size )
            {
               _pValues[ idx ] = boValue.copy() ;
            }
         }

         OSS_INLINE const BSONObj &getValue ( UINT32 idx ) const
         {
            SDB_ASSERT( idx < _size, "Wrong index" ) ;
            return _pValues[ idx ] ;
         }

         INT32 checkValues ( UINT32 numKeys, const BSONObj &keyPattern ) ;

      protected :

         void _clear () ;

         BOOLEAN _inRange ( UINT32 idx, dmsStatKey *pStartKey,
                            dmsStatKey *pStopKey ) const ;

      protected :
         UINT32            _numKeys ;
         UINT32            _size ;
         BSONObj *         _pValues ;
   } ;

   /*
      _dmsStatMCVSet define
    */
   class _dmsStatMCVSet : public _dmsStatValues
   {
      public :
         _dmsStatMCVSet () ;

         virtual ~_dmsStatMCVSet () ;

         INT32 init ( UINT32 size ) ;

         OSS_INLINE void setFrac ( UINT32 idx, double fraction )
         {
            if ( idx < _size )
            {
               _pFractions[ idx ] = fraction ;
            }
         }

         OSS_INLINE double getFrac ( UINT32 idx ) const
         {
            if ( idx < _size )
            {
               return _pFractions[ idx ] ;
            }
            return 0.0 ;
         }

         OSS_INLINE void setTotalFrac ( double totalFrac )
         {
            _totalFrac = totalFrac ;
         }

         OSS_INLINE double getTotalFrac () const
         {
            return _totalFrac ;
         }

         void clear () ;

         INT32 evalOperator ( dmsStatKey *pStartKey, dmsStatKey *pStopKey,
                              BOOLEAN &hitMCV,
                              double &predSelectivity,
                              double &scanSelectivity ) const ;

         INT32 evalETOperator ( dmsStatKey &key, BOOLEAN &hitMCV,
                                double &predSelectivity,
                                double &scanSelectivity ) const ;

      protected :
         double *          _pFractions ;
         double            _totalFrac ;
   } ;

   typedef class _dmsStatMCVSet dmsStatMCVSet ;

   /*
      _dmsStatUnit define
    */
   class _dmsStatUnit : public _utilSUCacheUnit
   {
      public :
         _dmsStatUnit () ;

         // For statistics cache, mbID is ID of unit
         _dmsStatUnit ( UINT16 mbID, UINT64 version ) ;

         virtual ~_dmsStatUnit () {}

         OSS_INLINE UINT16 getMBID () const
         {
            // For statistics cache, mbID is ID of unit
            return getUnitID() ;
         }

         OSS_INLINE UINT64 getSampleNum () const
         {
            return _sampleRecords ;
         }

         OSS_INLINE UINT64 getTotalRecords () const
         {
            return _totalRecords ;
         }

         INT32 init ( const BSONObj &boStat ) ;

      protected :
         void _initItems () ;

         virtual INT32 _initItem ( const BSONElement &beItem ) = 0 ;

         virtual INT32 _postInit () = 0 ;

      protected :
         // Number of records in the sample
         UINT64   _sampleRecords ;

         // Number of records in the collection when collecting this statistics
         UINT64   _totalRecords ;
   } ;

   /*
      _dmsIndexStat define
    */
   class _dmsIndexStat : public _dmsStatUnit
   {
      public :
         _dmsIndexStat () ;

         _dmsIndexStat ( const CHAR *pCSName, const CHAR *pCLName, UINT16 mbID,
                         UINT64 version, const CHAR *pIndexName ) ;

         virtual ~_dmsIndexStat () ;

         OSS_INLINE virtual const CHAR *getCSName () const
         {
            return _pCSName ;
         }

         OSS_INLINE virtual void setCSName ( const CHAR *pCSName )
         {
            _pCSName = pCSName ;
         }

         OSS_INLINE virtual const CHAR *getCLName () const
         {
            return _pCLName ;
         }

         OSS_INLINE virtual void setCLName ( const CHAR *pCLName )
         {
            _pCLName = pCLName ;
         }

         OSS_INLINE virtual UTIL_SU_CACHE_UMIT_TYPE getUnitType () const
         {
            return UTIL_SU_CACHE_UNIT_IDXSTAT ;
         }

         OSS_INLINE virtual BOOLEAN addSubUnit ( utilSUCacheUnit *pSubUnit,
                                                 BOOLEAN ignoreVersion )
         {
            return FALSE ;
         }

         OSS_INLINE virtual void clearSubUnits () {}

         OSS_INLINE const CHAR *getIndexName () const
         {
            return _pIndexName ;
         }

         OSS_INLINE void setIndexName ( const CHAR *pIndexName )
         {
            ossMemset( _pIndexName, 0, sizeof( _pIndexName ) ) ;
            if ( pIndexName )
            {
               ossMemcpy( _pIndexName, pIndexName, ossStrlen( pIndexName ) ) ;
            }
         }

         OSS_INLINE const BSONObj &getKeyPattern () const
         {
            return _keyPattern ;
         }

         OSS_INLINE const CHAR *getFirstField () const
         {
            return _pFirstField ;
         }

         OSS_INLINE UINT32 getNumKeys () const
         {
            return _numKeys ;
         }

         OSS_INLINE UINT32 getIndexPages () const
         {
            return _indexPages ;
         }

         OSS_INLINE UINT32 getIndexLevels () const
         {
            return _indexLevels ;
         }

         OSS_INLINE BOOLEAN isUnique () const
         {
            return _isUnique ;
         }

         OSS_INLINE UINT64 getDistinctValues () const
         {
            return _distinctValues ;
         }

         OSS_INLINE double getNullFrac () const
         {
            return _nullFrac ;
         }

         OSS_INLINE double getUndefFrac () const
         {
            return _undefFrac ;
         }

         OSS_INLINE const dmsStatMCVSet &getMCVSet () const
         {
            return _mcvSet ;
         }

         INT32 evalRangeOperator ( dmsStatKey &startKey,
                                   dmsStatKey &stopKey,
                                   double &predSelectivity,
                                   double &scanSelectivity ) const ;

         INT32 evalETOperator ( dmsStatKey &key,
                                double &predSelectivity,
                                double &scanSelectivity ) const ;

         INT32 evalGTOperator ( dmsStatKey &startKey,
                                double &predSelectivity,
                                double &scanSelectivity ) const ;

         INT32 evalLTOperator ( dmsStatKey &stopKey,
                                double &predSelectivity,
                                double &scanSelectivity ) const ;

         OSS_INLINE BOOLEAN isValid () const
         {
            return _mcvSet.getSize() > 0 ;
         }

      protected :
         void _initItems () ;

         virtual INT32 _initItem ( const BSONElement &beItem ) ;
         virtual INT32 _postInit () ;

         INT32 _initKeyPattern ( const BSONObj &boKeyPattern ) ;
         INT32 _initMCV ( const BSONObj &boMCV ) ;

         INT32 _evalOperator ( dmsStatKey *pStartKey, dmsStatKey *pStopKey,
                               double &predSelectivity, double &scanSelectivity ) const ;

      protected :
         const CHAR *      _pCSName ;
         const CHAR *      _pCLName ;

         CHAR              _pIndexName [ IXM_INDEX_NAME_SIZE + 1 ] ;

         // Definition of the index
         BSONObj           _keyPattern ;

         // First field in the index
         const CHAR *      _pFirstField ;

         // Number of keys in the index
         UINT32            _numKeys ;

         // Number of index pages
         UINT32            _indexPages ;

         // Number of index levels
         UINT32            _indexLevels ;

         // Is a unique index
         BOOLEAN           _isUnique ;

         // Number of distinct values in the index
         UINT64            _distinctValues ;

         double            _nullFrac ;
         double            _undefFrac ;

         dmsStatMCVSet     _mcvSet ;
   } ;

   typedef _dmsIndexStat dmsIndexStat ;

   typedef _utilMap< dmsStatSubUnitKey, dmsIndexStat * > INDEX_STAT_MAP ;

   typedef INDEX_STAT_MAP::iterator INDEX_STAT_ITERATOR ;
   typedef INDEX_STAT_MAP::const_iterator INDEX_STAT_CONST_ITERATOR ;

   /*
      _dmsCollectionStat define
    */
   class _dmsCollectionStat : public _dmsStatUnit
   {
      public :
         _dmsCollectionStat () ;

         _dmsCollectionStat ( const CHAR *pCSName, const CHAR *pCLName,
                              UINT16 mbID, UINT64 version ) ;

         virtual ~_dmsCollectionStat () ;

         OSS_INLINE virtual const CHAR *getCSName () const
         {
            return _pCSName ;
         }

         OSS_INLINE virtual void setCSName ( const CHAR *pCSName )
         {
            ossMemset ( _pCSName, 0, sizeof( _pCSName ) ) ;
            if ( pCSName )
            {
               ossStrncpy ( _pCSName, pCSName, sizeof( _pCSName ) ) ;
            }
         }

         OSS_INLINE virtual const CHAR *getCLName () const
         {
            return _pCLName ;
         }

         OSS_INLINE virtual void setCLName ( const CHAR *pCLName )
         {
            ossMemset ( _pCLName, 0, sizeof( _pCLName ) ) ;
            if ( pCLName )
            {
               ossStrncpy ( _pCLName, pCLName, sizeof( _pCLName ) ) ;
            }
         }

         OSS_INLINE virtual UTIL_SU_CACHE_UMIT_TYPE getUnitType () const
         {
            return UTIL_SU_CACHE_UNIT_CLSTAT ;
         }

         OSS_INLINE UINT32 getTotalDataPages () const
         {
            return _totalDataPages ;
         }

         OSS_INLINE UINT64 getTotalDataSize () const
         {
            return _totalDataSize ;
         }

         OSS_INLINE UINT32 getAvgNumFields () const
         {
            return _avgNumFields ;
         }

         OSS_INLINE const INDEX_STAT_MAP & getIndexStats () const
         {
            return _indexStats ;
         }

         OSS_INLINE INDEX_STAT_MAP & getIndexStats ()
         {
            return _indexStats ;
         }

         virtual BOOLEAN addSubUnit ( utilSUCacheUnit *pSubUnit,
                                      BOOLEAN ignoreVersion ) ;

         virtual void clearSubUnits () ;

         const dmsIndexStat *getIndexStat ( const CHAR *pIndexName ) const ;

         const dmsIndexStat *getFieldStat ( const CHAR *pFieldName ) const ;

         OSS_INLINE BOOLEAN addIndexStat ( dmsIndexStat *pIndexStat,
                                           BOOLEAN ignoreVersion )
         {
            return addSubUnit( pIndexStat, ignoreVersion ) ;
         }

         BOOLEAN removeIndexStat ( const CHAR *pIndexName,
                                   BOOLEAN findNewFieldStat ) ;

         BOOLEAN removeFieldStat ( const CHAR *pFieldName,
                                   BOOLEAN findNewFieldStat ) ;

         OSS_INLINE void renameCS ( const CHAR *pCSName )
         {
            setCSName( pCSName ) ;
         }

         OSS_INLINE void renameCL ( const CHAR *pCLName )
         {
            setCLName( pCLName ) ;
         }

      protected :
         void _initItems () ;

         virtual INT32 _initItem ( const BSONElement &beItem ) ;
         virtual INT32 _postInit () { return SDB_OK ; }

         void _addFieldStat ( dmsIndexStat *pIndexStat, BOOLEAN ignoreVersion ) ;

         void _removeFieldStat ( dmsIndexStat *pDeletingStat ) ;

         void _findNewFieldStat ( dmsIndexStat *pDeletingStat ) ;

      protected :
         // Name of collection space
         CHAR     _pCSName [ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] ;

         // Name of collection ( short name )
         CHAR     _pCLName [ DMS_COLLECTION_NAME_SZ + 1 ] ;

         UINT32            _totalDataPages ;
         UINT64            _totalDataSize ;
         UINT32            _avgNumFields ;

         INDEX_STAT_MAP    _indexStats ;
         INDEX_STAT_MAP    _fieldStats ;
   } ;

   typedef _dmsCollectionStat dmsCollectionStat ;

   /*
      _dmsStatCache define
    */
   class _dmsStatCache : public utilSUCache
   {
      public :
         _dmsStatCache( _IUtilSUCacheHolder *pHolder = NULL ) ;

         virtual ~_dmsStatCache () {}
   } ;

   typedef class _dmsStatCache dmsStatCache ;
}

#endif //DMSSTATUNIT_HPP__

