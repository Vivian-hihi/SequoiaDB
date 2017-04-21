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

   Source File Name = rtnStatObj.hpp

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
#ifndef RTNSTATOBJ_HPP__
#define RTNSTATOBJ_HPP__

#include "dmsStatObj.hpp"
#include "rtnStatMgr.hpp"
#include "rtnPredicate.hpp"
#include "../bson/bson.h"

using namespace std ;
using namespace bson ;

namespace engine
{

   class _dmsMBContext ;

   class _rtnCollectionStat ;
   typedef _rtnCollectionStat rtnCollectionStat ;

   typedef _utilList<rtnPredicate *> rtnStatPredList ;

   /*
      _rtnStatBase define
    */
   class _rtnStatBase : public SDBObject
   {
      public :
         _rtnStatBase ( const CHAR *pCollectionName, UINT64 totalRecords )
         {
            _pCollectionName = pCollectionName ;
            _totalRecords = totalRecords ;
         }

         virtual ~_rtnStatBase () {}

         OSS_INLINE const CHAR *getCollectionName () const
         {
            return _pCollectionName ;
         }

         OSS_INLINE UINT64 getTotalRecords () const
         {
            return _totalRecords ;
         }

         virtual double evalPredicate ( const CHAR *pFieldName,
                                        rtnPredicate &predicate,
                                        BOOLEAN &isAllRange ) const ;

         virtual double evalStartStopKeys ( const CHAR *pFieldName,
                                            dmsStatKey &startKey,
                                            BOOLEAN startIncluded,
                                            dmsStatKey &stopKey,
                                            BOOLEAN stopIncluded,
                                            BOOLEAN isEqual ) const = 0 ;

         virtual BOOLEAN isValid () const = 0 ;

      protected :
         INT32 _evalStartStopKeys ( const dmsIndexStat *pIndexStat,
                                    rtnStatPredList::iterator &predIter,
                                    dmsStatListKey &statKeys,
                                    BOOLEAN startIncluded,
                                    dmsStatListKey &stopKeys,
                                    BOOLEAN endIncluded,
                                    BOOLEAN isEqual, double &selectivity ) const ;

      protected :
         const CHAR *   _pCollectionName ;
         UINT64         _totalRecords ;
   } ;

   /*
      _rtnIndexStat define
    */
   class _rtnIndexStat : public _rtnStatBase
   {
      public :
         _rtnIndexStat ( const rtnCollectionStat &collectionStat,
                         const ixmIndexCB &indexCB ) ;

         virtual ~_rtnIndexStat () {}

         OSS_INLINE const CHAR *getIndexName () const
         {
            return _pIndexName ;
         }

         OSS_INLINE const BSONObj &getKeyPattern () const
         {
            return _keyPattern ;
         }

         OSS_INLINE UINT32 getIndexPages () const
         {
            return _indexPages ;
         }

         OSS_INLINE UINT32 getIndexLevels () const
         {
            return _pIndexStat ? _pIndexStat->getIndexLevels() :
                                 STAT_DEF_IDX_LEVELS ;
         }

         OSS_INLINE virtual BOOLEAN isValid () const
         {
            return ( _pIndexStat && _pIndexStat->isValid() ) ;
         }

         double evalPredicateList ( const CHAR *pFieldName,
                                    rtnStatPredList &predList ) const ;

         virtual double evalStartStopKeys ( const CHAR *pFieldName,
                                            dmsStatKey &startKey,
                                            BOOLEAN startIncluded,
                                            dmsStatKey &stopKey,
                                            BOOLEAN stopIncluded,
                                            BOOLEAN isEqual ) const ;

      protected :
         const rtnCollectionStat &_collectionStat ;
         const CHAR *    _pIndexName ;
         const dmsIndexStat *_pIndexStat ;

         BSONObj         _keyPattern ;
         UINT32          _indexPages ;
   } ;

   typedef _rtnIndexStat rtnIndexStat ;

   /*
      _rtnCollectionStat define
    */
   class _rtnCollectionStat : public _rtnStatBase
   {
      public :
         _rtnCollectionStat ( const CHAR *pCollectionName,
                              UINT32 pageSize,
                              _dmsMBContext *mbContext,
                              const rtnStatMgr *statMgr ) ;

         virtual ~_rtnCollectionStat () {}

         OSS_INLINE const dmsCollectionStat *getCollectionStat () const
         {
            return _pCollectionStat ;
         }

         OSS_INLINE const dmsIndexStat *getIndexStat ( const CHAR *pIndexName ) const
         {
            return ( _pCollectionStat && pIndexName ) ?
                        _pCollectionStat->getIndexStat( pIndexName ) : NULL ;
         }

         OSS_INLINE const dmsIndexStat *getFieldStat ( const CHAR *pFieldName ) const
         {
            return ( _pCollectionStat && pFieldName ) ?
                     _pCollectionStat->getFieldStat( pFieldName ) : NULL ;
         }

         OSS_INLINE UINT32 getTotalDataPages () const
         {
            return _totalDataPages ;
         }

         OSS_INLINE UINT64 getTotalDataLen () const
         {
            return _totalDataLen ;
         }

         OSS_INLINE UINT32 getPageSize () const
         {
            return _pageSize ;
         }

         OSS_INLINE UINT32 getNumIndexes () const
         {
            return _numIndexes ;
         }

         OSS_INLINE UINT32 getTotalIndexPages () const
         {
            return _totalIndexPages ;
         }

         OSS_INLINE UINT64 getTotalIndexSize () const
         {
            return _totalIndexSize ;
         }

         OSS_INLINE UINT32 getAvgIndexPages () const
         {
            return _avgIndexPages ;
         }

         OSS_INLINE UINT64 getAvgIndexSize () const
         {
            return _avgIndexSize ;
         }

         OSS_INLINE UINT32 getAvgNumFields () const
         {
            return _pCollectionStat ? _pCollectionStat->getAvgNumFields() :
                                      STAT_DEF_AVG_NUM_FIELDS ;
         }

         OSS_INLINE virtual BOOLEAN isValid () const
         {
            return ( _pCollectionStat != NULL ) ;
         }

         INT32 initCurStat ( _dmsMBContext *mbContext ) ;

         double evalPredicateSet ( rtnPredicateSet &predicateSet ) const ;

         virtual double evalStartStopKeys ( const CHAR *pFieldName,
                                            dmsStatKey &startKey,
                                            BOOLEAN startIncluded,
                                            dmsStatKey &stopKey,
                                            BOOLEAN stopIncluded,
                                            BOOLEAN isEqual ) const ;

         double evalETOpterator ( const CHAR *pFieldName,
                                  const BSONElement &beValue ) const ;

         double evalGTOpterator ( const CHAR *pFieldName,
                                  const BSONElement &beValue,
                                  BOOLEAN included ) const ;

         double evalLTOpterator ( const CHAR *pFieldName,
                                  const BSONElement &beValue,
                                  BOOLEAN included ) const ;

      protected :
         double _evalStartStopKeys ( const BSONElement &startKey,
                                     const BSONElement &stopKey ) const ;

         double _evalETOperator ( const BSONElement &beValue ) const ;

         double _evalRangeOperator ( const BSONElement &beStart,
                                     const BSONElement &beStop ) const ;

         double _evalGTOperator ( const BSONElement &beStart ) const ;

         double _evalLTOperator ( const BSONElement &beStop ) const ;

      protected :
         /* Init from dmsMBContext */
         UINT32            _totalDataPages ;
         UINT32            _pageSize ;
         UINT64            _totalDataLen ;

         UINT32            _numIndexes ;
         UINT32            _totalIndexPages ;
         UINT64            _totalIndexSize ;
         UINT32            _avgIndexPages ;
         UINT64            _avgIndexSize ;

         const dmsCollectionStat *_pCollectionStat ;
   } ;

}

#endif //RTNSTATOBJ_HPP__

