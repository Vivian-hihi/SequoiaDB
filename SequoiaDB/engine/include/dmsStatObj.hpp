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

   Source File Name = dmsStatsObj.hpp

   Descriptive Name = Data Management Service Statistics Object Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS Statistics Objects.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#ifndef DMSSTATOBJ_HPP__
#define DMSSTATOBJ_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ixm.hpp"

#include "utilMap.hpp"
#include "utilList.hpp"

using namespace std ;

namespace engine
{

#define STAT_COLLECTION_SPACE          "CollectionSpace"
#define STAT_COLLECTION                "Collection"
#define STAT_COLLECTION_MBID           "CollectionMBID"
#define STAT_VERSION                   "Version"
#define STAT_SAMPLE_RECORDS            "SampleRecords"
#define STAT_TOTAL_RECORDS             FIELD_NAME_TOTAL_RECORDS

#define STAT_CL_TOTAL_DATA_LEN         FIELD_NAME_TOTAL_DATA_SIZE
#define STAT_CL_AVG_NUM_FIELDS         "AvgNumFields"

#define STAT_IDX_INDEX                 "Index"
#define STAT_IDX_KEY_PATTERN           "KeyPattern"
#define STAT_IDX_INDEX_PAGES           "IndexPages"
#define STAT_IDX_LEVELS                "IndexLevels"
#define STAT_IDX_IS_UNIQUE             "IsUnique"
#define STAT_IDX_DISTINCT_VALUES       "DistinctValues"
#define STAT_IDX_NULL_FRAC             "NullFrac"
#define STAT_IDX_UNDEF_FRAC            "UndefFrac"
#define STAT_IDX_MCV                   "MCV"
#define STAT_IDX_MCV_VALUES            "Values"
#define STAT_IDX_MCV_FRAC              "Frac"
#define STAT_IDX_HISTOGRAM             "Histogram"
#define STAT_IDX_HISTOGRAM_FRAC        "Frac"
#define STAT_IDX_HISTOGRAM_BOUNDS      "Bounds"
#define STAT_IDX_TYPE_SET              "TypeSet"
#define STAT_IDX_TYPE_SET_TYPES        "Types"
#define STAT_IDX_TYPE_SET_FRAC         "Frac"

#define STAT_DEF_VERSION               ( 0 )
#define STAT_DEF_AVG_NUM_FIELDS        ( 10 )
#define STAT_DEF_TOTAL_PAGES           ( 1 )
#define STAT_DEF_IDX_LEVELS            ( 1 )
#define STAT_DEF_TOTAL_RECORDS         ( 10 )
#define STAT_DEF_DATA_LEN              ( 400 )

   /*
      _dmsStatKey define
    */
   class _dmsStatKey
   {
      public :
         _dmsStatKey () {}

         virtual ~_dmsStatKey () {}

         virtual INT32 compareValue ( INT32 incFlag,
                                      const BSONObj &rValue ) = 0 ;
         virtual string toString () = 0 ;

         virtual UINT32 size () = 0 ;

         virtual BSONElement &firstElement () = 0 ;

      protected :
         OSS_INLINE INT32 _compareLeftMore ( INT32 incFlag )
         {
            // left has more elements, normally left > right
            // We need to include all right, so right > left
            return incFlag < 0 ? -1 : 1 ;
         }

         INT32 _compareRightMore ( INT32 incFlag )
         {
            // right has more elements, normally left < right
            // We need to include all right, so left > right
            return incFlag > 0 ? 1 : -1 ;
         }

         INT32 _compareDefault ( INT32 incFlag )
         {
            return incFlag > 0 ? 1 : ( incFlag < 0 ? -1 : 0 ) ;
         }
   } ;

   typedef class _dmsStatKey dmsStatKey ;

   /*
      _dmsStatListKey define
    */
   class _dmsStatListKey : public _utilList<BSONElement>,
                           public _dmsStatKey
   {
      public :
         explicit _dmsStatListKey () ;

         explicit _dmsStatListKey ( const BSONElement &beValue ) ;

         virtual ~_dmsStatListKey () {}

         virtual INT32 compareValue( INT32 incFlag,
                                     const BSONObj &rValue ) ;

         virtual string toString () ;

         OSS_INLINE virtual UINT32 size ()
         {
            return _utilList<BSONElement>::size() ;
         }

         OSS_INLINE virtual BSONElement &firstElement ()
         {
            return front() ;
         }
   } ;

   typedef class _dmsStatListKey dmsStatListKey ;

   /*
      _dmsStatElementKey define
    */
   class _dmsStatElementKey : public BSONElement,
                              public _dmsStatKey
   {
      public :
         explicit _dmsStatElementKey ( const BSONElement &element ) ;

         virtual ~_dmsStatElementKey () {}

         virtual INT32 compareValue ( INT32 incFlag, const BSONObj &rValue ) ;

         OSS_INLINE virtual string toString ()
         {
            return BSONElement::toString( TRUE, TRUE ) ;
         }

         OSS_INLINE virtual UINT32 size ()
         {
            return 1 ;
         }

         OSS_INLINE virtual BSONElement &firstElement ()
         {
            return (BSONElement &)(*this) ;
         }
   } ;

   typedef class _dmsStatElementKey dmsStatElementKey ;


   /*
      _dmsStatObjName define
    */
   typedef struct _dmsStatObjName dmsStatObjName ;

   struct _dmsStatObjName
   {
      OSS_INLINE _dmsStatObjName ( const CHAR * pName = NULL )
      {
         _pName = pName ;
      }

      const CHAR * _pName ;

      OSS_INLINE bool operator== ( const dmsStatObjName &objName ) const
      {
         return ossStrcmp( _pName, objName._pName ) == 0 ;
      }

      OSS_INLINE bool operator< ( const dmsStatObjName &objName ) const
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

      protected :

         void _clear () ;

      protected :
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

         INT32 evalOperator ( dmsStatKey *pStartKey, BOOLEAN startIncluded,
                              dmsStatKey *pStopKey, BOOLEAN stopIncluded,
                              UINT32 numKeys, BOOLEAN &hitMCV, double &selectivity ) const ;

         INT32 evalETOperator ( dmsStatKey &key, BOOLEAN &hitMCV,
                                double &selectivity ) const ;

      protected :
         double *          _pFractions ;
         double            _totalFrac ;
   } ;

   typedef class _dmsStatMCVSet dmsStatMCVSet ;

   /*
      _dmsStatBase define
    */
   class _dmsStatBase : public SDBObject
   {
      public :
         _dmsStatBase ( const CHAR *pCSName,
                        const CHAR *pCLName ) ;

         virtual ~_dmsStatBase () {}

         OSS_INLINE const CHAR *getCSName () const
         {
            return _pCSName ;
         }

         OSS_INLINE const CHAR *getCLName () const
         {
            return _pCLName ;
         }

         OSS_INLINE UINT16 getMBID () const
         {
            return _mbID ;
         }

         OSS_INLINE UINT64 getVersion () const
         {
            return _version ;
         }

         OSS_INLINE UINT64 getSampleNum () const
         {
            return _sampleRecords ;
         }

         OSS_INLINE UINT64 getTotalRecords () const
         {
            return _totalRecords ;
         }

         virtual void setCSName ( const CHAR *pCSName ) ;
         virtual void setCLName ( const CHAR *pCLName ) ;

         INT32 init ( const BSONObj &boStat ) ;

      protected :
         virtual INT32 _initItem ( const BSONElement &beItem ) = 0 ;

         virtual INT32 _postInit () = 0 ;

      protected :
         // Name of collection space
         CHAR     _pCSName [ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] ;

         // Name of collection ( short name )
         CHAR     _pCLName [ DMS_COLLECTION_NAME_SZ + 1 ] ;

         // Block ID of the collection
         UINT16   _mbID ;

         // Version of the statistics record
         // timestamp when collecting this statistics
         UINT64   _version ;

         // Number of records in the sample
         UINT64   _sampleRecords ;

         // Number of records in the collection when collecting this statistics
         UINT64   _totalRecords ;
   } ;

   /*
      _dmsIndexStat define
    */
   class _dmsIndexStat : public _dmsStatBase
   {
      public :
         _dmsIndexStat ( const CHAR *pCSName, const CHAR *pCLName,
                         const CHAR *pIndexName ) ;

         virtual ~_dmsIndexStat () ;

         OSS_INLINE const CHAR *getIndexName () const
         {
            return _pIndexName ;
         }

         void setIndexName ( const CHAR *pIndexName ) ;

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

         OSS_INLINE UINT32 getAvgFieldSize () const
         {
            return _avgFieldSize ;
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

         INT32 evalRangeOperator ( dmsStatKey &startKey, BOOLEAN startIncluded,
                                   dmsStatKey &stopKey, BOOLEAN stopIncluded,
                                   double &selectivity ) const ;

         INT32 evalETOperator ( dmsStatKey &key, double &selectivity ) const ;

         INT32 evalGTOperator ( dmsStatKey &startKey,
                                BOOLEAN startIncluded,
                                double &selectivity ) const ;

         INT32 evalLTOperator ( dmsStatKey &stopKey,
                                BOOLEAN stopIncluded,
                                double &selectivity ) const ;

         OSS_INLINE BOOLEAN isValid () const
         {
            return _mcvSet.getSize() > 0 ;
         }

      protected :
         virtual INT32 _initItem ( const BSONElement &beItem ) ;

         INT32 _initKeyPattern ( const BSONObj &boKeyPattern ) ;
         INT32 _initMCV ( const BSONObj &boMCV ) ;

         virtual INT32 _postInit () ;

         INT32 _evalOperator ( dmsStatKey *pStartKey, BOOLEAN startIncluded,
                               dmsStatKey *pStopKey, BOOLEAN stopIncluded,
                               double &selectivity ) const ;

      protected :
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

         //
         UINT32            _avgFieldSize ;

         double            _nullFrac ;
         double            _undefFrac ;

         dmsStatMCVSet     _mcvSet ;
   } ;

   typedef _dmsIndexStat dmsIndexStat ;

   /*
      _dmsCollectionStat define
    */
   class _dmsCollectionStat : public _dmsStatBase
   {
      public :
         _dmsCollectionStat ( const CHAR *pCSName,
                              const CHAR *pCLName ) ;

         virtual ~_dmsCollectionStat () ;

         OSS_INLINE UINT64 getTotalDataLen () const
         {
            return _totalDataLen ;
         }

         OSS_INLINE UINT32 getAvgNumFields () const
         {
            return _avgNumFields ;
         }

         const dmsIndexStat *getIndexStat ( const CHAR *pIndexName ) const ;

         const dmsIndexStat *getFieldStat ( const CHAR *pFieldName ) const ;

         BOOLEAN addIndexStat ( dmsIndexStat *pIndexStat,
                                BOOLEAN ignoreVersion ) ;

         BOOLEAN addFieldStat ( dmsIndexStat *pIndexStat,
                                BOOLEAN ignoreVersion ) ;

         BOOLEAN removeIndexStat ( const CHAR *pIndexName,
                                   BOOLEAN findNewFieldStat ) ;

         void removeFieldStat ( const CHAR *pFieldName ) ;

         void clearStats () ;

         virtual void setCSName ( const CHAR *pCSName ) ;
         virtual void setCLName ( const CHAR *pCLName ) ;

      protected :
         virtual INT32 _initItem ( const BSONElement &beItem ) ;

         virtual INT32 _postInit () { return SDB_OK ; }

         void _findNewFieldStat ( const CHAR *pFieldName,
                                  dmsIndexStat *pDeletingStat ) ;

      protected :
         UINT64            _totalDataLen ;
         UINT32            _avgNumFields ;

         typedef _utilMap< dmsStatObjName,
                           dmsIndexStat * >::iterator INDEX_STAT_ITERATOR ;
         typedef _utilMap< dmsStatObjName,
                           dmsIndexStat * >::const_iterator INDEX_STAT_CONST_ITERATOR ;

         _utilMap< dmsStatObjName, dmsIndexStat * > _indexStats ;
         _utilMap< dmsStatObjName, dmsIndexStat * > _fieldStats ;
   } ;

   typedef _dmsCollectionStat dmsCollectionStat ;


}

#endif //DMSSTATCB_HPP__

