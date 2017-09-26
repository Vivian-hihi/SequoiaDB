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

   Source File Name = mthMatchRuntime.hpp

   Descriptive Name = Matcher Tree Runtime Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains runtime structure for
   matchers.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/17/2017  HGM  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MTHMATCHRUNTIME_HPP__
#define MTHMATCHRUNTIME_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "mthMatchTree.hpp"
#include "optCommon.hpp"

using namespace bson ;
using namespace std ;

namespace engine
{

   /*
      _mthMatchTreeHolder define
    */
   class _mthMatchTreeHolder
   {
      public :
         _mthMatchTreeHolder ()
         {
            _matchTree = NULL ;
         }

         virtual ~_mthMatchTreeHolder ()
         {
            _matchTree = NULL ;
         }

         OSS_INLINE mthMatchTree *getMatchTree ()
         {
            return _matchTree ;
         }

         OSS_INLINE const mthMatchTree *getMatchTree () const
         {
            return _matchTree ;
         }

         OSS_INLINE virtual void setMatchTree ( mthMatchTree *matchTree )
         {
            _matchTree = matchTree ;
         }

      protected :
         mthMatchTree * _matchTree ;
   } ;

   class _mthMatchTreeStackHolder : public _mthMatchTreeHolder
   {
      public :
         _mthMatchTreeStackHolder ()
         : _mthMatchTreeHolder()
         {
            _matchTree = (&_stackMatchTree ) ;
         }

         virtual ~_mthMatchTreeStackHolder () {}

         OSS_INLINE virtual void setMatchTree ( mthMatchTree *matchTree )
         {
            // Do nothing
         }

      protected :
         mthMatchTree _stackMatchTree ;
   } ;

   /*
      _mthParamPredListHolder define
    */
   class _mthParamPredListHolder
   {
      public :
         _mthParamPredListHolder ()
         {
            _paramPredList = NULL ;
         }

         virtual ~_mthParamPredListHolder ()
         {
            _paramPredList = NULL ;
         }

         OSS_INLINE RTN_PARAM_PREDICATE_LIST *getParamPredList ()
         {
            return _paramPredList ;
         }

         OSS_INLINE const RTN_PARAM_PREDICATE_LIST *getParamPredList () const
         {
            return _paramPredList ;
         }

         OSS_INLINE virtual void setParamPredList ( RTN_PARAM_PREDICATE_LIST *paramPredList )
         {
            _paramPredList = paramPredList ;
         }

      protected :
         RTN_PARAM_PREDICATE_LIST *_paramPredList ;
   } ;

   class _mthParamPredListStackHolder : public _mthParamPredListHolder
   {
      public :
         _mthParamPredListStackHolder ()
         : _mthParamPredListHolder()
         {
            _paramPredList = (&_stackParamPredList ) ;
         }

         virtual ~_mthParamPredListStackHolder () {}

         OSS_INLINE virtual void setParamPredList ( RTN_PARAM_PREDICATE_LIST *paramPredList )
         {
            // Do nothing
         }

      protected :
         // Predicate set generated from matcher tree
         RTN_PARAM_PREDICATE_LIST _stackParamPredList ;
   } ;

   typedef class _mthParamPredListHolder mthParamPredListHolder ;

   /*
      _mthMatchHelper define
    */
   class _mthMatchHelper : public SDBObject,
                           public _mthMatchTreeHolder,
                           public _mthMatchConfigHolder
   {
      public :
         _mthMatchHelper ( OPT_PLAN_CACHE_LEVEL cacheLevel,
                           const mthNodeConfig &config ) ;

         virtual ~_mthMatchHelper () ;

         void clear () ;

         OSS_INLINE BSONObj getQuery ()
         {
            return _query ;
         }

         void setMatchTree ( _mthMatchTree *matchTree ) ;

         void getEstimation ( optCollectionStat *pCollectionStat,
                              double &estSelectivity, UINT32 &estCPUCost ) ;

         OSS_INLINE void getPredSelectivity ( double &predSelectivity,
                                              double &scanSelectivity )
         {
            predSelectivity = _predSelectivity ;
            scanSelectivity = _scanSelectivity ;
         }

         OSS_INLINE BOOLEAN isEstimated ()
         {
            return _isEstimated ;
         }

         INT32 normalizeQuery ( const BSONObj &query,
                                BSONObjBuilder &normalBuilder,
                                rtnParamList &parameters,
                                BOOLEAN &invalidMatcher ) ;

         mthMatchNormalizer &getNormalizer ()
         {
            return _normalizer ;
         }

         OSS_INLINE const rtnPredicateSet &getPredicateSet() const
         {
            return _predicateSet ;
         }

         OSS_INLINE rtnPredicateSet &getPredicateSet ()
         {
            return _predicateSet ;
         }

         OSS_INLINE RTN_PREDICATE_MAP &getPredicates ()
         {
            return _predicateSet.predicates() ;
         }

         OSS_INLINE BOOLEAN isPredicateSetEmpty () const
         {
            return ( _predicateSet.getSize() == 0 ) ;
         }

      protected :
         void _evalEstimation ( optCollectionStat *pCollectionStat ) ;

      protected :
         BSONObj              _query ;
         OPT_PLAN_CACHE_LEVEL _cacheLevel ;
         mthMatchNormalizer   _normalizer ;
         rtnPredicateSet      _predicateSet ;

         /// Cost or selectivity estimations
         // Flag to indicate whether the cost and selectivity are estimated
         BOOLEAN           _isEstimated ;
         // Selectivity of the matcher
         double            _estSelectivity ;
         // The final selectivity of the predicates
         double            _predSelectivity ;
         // The scan selectivity of the predicates
         double            _scanSelectivity ;
         // The CPU cost of the matcher
         UINT32            _estCPUCost ;
   } ;

   typedef class _mthMatchHelper mthMatchHelper ;

   /*
      _mthMatchRuntime define
    */
   class _mthMatchRuntime : public SDBObject,
                            public _mthMatchTreeHolder,
                            public _mthParamPredListHolder
   {
      public :
         _mthMatchRuntime () ;

         virtual ~_mthMatchRuntime () ;

         OSS_INLINE BSONObj getQuery ()
         {
            return _query ;
         }

         void setQuery ( const BSONObj &query, BOOLEAN getOwned ) ;

         OSS_INLINE rtnParamList &getParameters ()
         {
            return _parameters ;
         }

         OSS_INLINE rtnParamList *getParametersPointer ()
         {
            return _parameters.isEmpty() ? NULL : &_parameters ;
         }

         OSS_INLINE rtnPredicateList *getPredList ()
         {
            return _predList.isInitialized() ? (&_predList) : NULL ;
         }

         OSS_INLINE BOOLEAN isFixedPredList () const
         {
            return _predList.isInitialized() &&
                   _predList.isFixedPredList() ;
         }

         INT32 generatePredList ( mthMatchHelper &matchHelper,
                                  const BSONObj &keyPattern,
                                  INT32 direction ) ;

         OSS_INLINE void clearPredList ()
         {
            _predList.clear() ;
         }

      protected :
         BSONObj              _query ;
         rtnParamList         _parameters ;
         rtnPredicateList     _predList ;
   } ;

   typedef class _mthMatchRuntime mthMatchRuntime ;

   /*
      _mthMatchRuntimeHolder define
    */
   class _mthMatchRuntimeHolder
   {
      public :
         _mthMatchRuntimeHolder () ;

         virtual ~_mthMatchRuntimeHolder () ;

         virtual void deleteMatchRuntime () ;

         virtual INT32 createMatchRuntime () ;

         void getMatchRuntimeOnwed ( _mthMatchRuntimeHolder &holder ) ;

         virtual const mthMatchRuntime *getMatchRuntime () const
         {
            return _matchRuntime ;
         }

         OSS_INLINE virtual mthMatchRuntime *getMatchRuntime ()
         {
            return _matchRuntime ;
         }

         OSS_INLINE virtual void setMatchRuntime ( mthMatchRuntime *matchRuntime )
         {
            deleteMatchRuntime() ;
            _matchRuntime = matchRuntime ;
         }

      protected :
         mthMatchRuntime *_matchRuntime ;
         BOOLEAN _ownedMatchRuntime ;
   } ;
}

#endif //MTHMATCHRUNTIME_HPP__

