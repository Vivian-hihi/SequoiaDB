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

   Source File Name = mthMatchRuntime.cpp

   Descriptive Name = Match Tree Runtime

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains functions for runtime of
   matchers.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/17/2017  HGM Initial Draft

   Last Changed =

*******************************************************************************/

#include "mthMatchRuntime.hpp"
#include "mthMatchNormalizer.hpp"
#include "pdTrace.hpp"
#include "mthTrace.hpp"
#include "pmd.hpp"
#include "rtnCB.hpp"

using namespace bson ;

namespace engine
{

   /*
      _mthMatchHelper implement
    */
   _mthMatchHelper::_mthMatchHelper ( OPT_PLAN_CACHE_LEVEL cacheLevel,
                                      const mthNodeConfig &config )
   : _mthMatchTreeHolder(),
     _mthMatchConfigHolder( config ),
     _normalizer( getMatchConfigPtr() )
   {
      _cacheLevel = cacheLevel ;

      // Adjust with cache level
      setMthEnableParameterized( config._enableParameterized &&
                                 ( cacheLevel >= OPT_PLAN_PARAMETERIZED ) ) ;
      setMthEnableFuzzyOptr( config._enableFuzzyOptr &&
                             ( cacheLevel >= OPT_PLAN_FUZZYOPTR ) ) ;

      _isEstimated      = FALSE ;
      _estSelectivity   = OPT_MTH_DEFAULT_SELECTIVITY ;
      _predSelectivity  = OPT_MTH_DEFAULT_SELECTIVITY ;
      _scanSelectivity  = OPT_MTH_DEFAULT_SELECTIVITY ;
      _estCPUCost       = OPT_MTH_OPTR_DEFAULT_SELECTIVITY ;

   }

   _mthMatchHelper::~_mthMatchHelper ()
   {
      clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHHELP_CLEAR, "_mthMatchHelper::clear" )
   void _mthMatchHelper::clear ()
   {
      _mthMatchTreeHolder::setMatchTree( NULL ) ;
      _predicateSet.clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHHELP_SETMTH, "_mthMatchHelper::setMatcher" )
   void _mthMatchHelper::setMatchTree ( _mthMatchTree *matchTree )
   {
      PD_TRACE_ENTRY( SDB__MTHHELP_SETMTH ) ;

      _mthMatchTreeHolder::setMatchTree( matchTree ) ;
      _predicateSet.clear() ;

      _isEstimated         = FALSE ;
      _estSelectivity      = OPT_MTH_DEFAULT_SELECTIVITY ;
      _predSelectivity     = OPT_MTH_DEFAULT_SELECTIVITY ;
      _scanSelectivity     = OPT_MTH_DEFAULT_SELECTIVITY ;
      _estCPUCost          = OPT_MTH_OPTR_DEFAULT_SELECTIVITY ;

      PD_TRACE_EXIT( SDB__MTHHELP_SETMTH ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHHELP_GETEST, "_mthMatchHelper::getEstimation" )
   void _mthMatchHelper::getEstimation ( optCollectionStat *pCollectionStat,
                                         double &estSelectivity,
                                         UINT32 &estCPUCost )
   {
      PD_TRACE_ENTRY( SDB__MTHHELP_GETEST ) ;

      if ( !_isEstimated )
      {
         _evalEstimation( pCollectionStat ) ;
      }
      estSelectivity = _estSelectivity ;
      estCPUCost = _estCPUCost ;

      PD_TRACE_EXIT( SDB__MTHHELP_GETEST ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHHELP__EVALEST, "_mthMatchHelper::_evalEstimation" )
   void _mthMatchHelper::_evalEstimation ( optCollectionStat *pCollectionStat )
   {
      PD_TRACE_ENTRY( SDB__MTHHELP__EVALEST ) ;

      double predSelectivity = OPT_MTH_DEFAULT_SELECTIVITY ;
      double scanSelectivity = OPT_MTH_DEFAULT_SELECTIVITY ;
      double tmpSelectivity = OPT_MTH_DEFAULT_SELECTIVITY ;
      UINT32 tmpCPUCost = OPT_MTH_DEFAULT_CPU_COST ;

      if ( _matchTree != NULL )
      {
         if ( pCollectionStat )
         {
            predSelectivity = pCollectionStat->evalPredicateSet(
                  _predicateSet, mthEnabledMixCmp(), scanSelectivity ) ;
         }
         _matchTree->evalEstimation( pCollectionStat, tmpSelectivity,
                                     tmpCPUCost ) ;
         tmpSelectivity *= predSelectivity ;
      }

      _estSelectivity = OPT_ROUND_SELECTIVITY( tmpSelectivity ) ;
      _predSelectivity = OPT_ROUND_SELECTIVITY( predSelectivity ) ;
      _scanSelectivity = OPT_ROUND_SELECTIVITY( scanSelectivity ) ;
      _estCPUCost = tmpCPUCost ;
      _isEstimated = TRUE ;

      PD_TRACE_EXIT( SDB__MTHHELP__EVALEST ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHHELP_GENSIMMTH, "_mthMatchHelper::generateSimpleMatcher" )
   INT32 _mthMatchHelper::normalizeQuery ( const BSONObj &query,
                                           BSONObjBuilder &normalBuilder,
                                           rtnParamList &parameters,
                                           BOOLEAN &invalidMatcher )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__MTHHELP_GENSIMMTH ) ;

      // No need to be copied
      _query = query ;

      // Normalize the query with simple parser
      rc = _normalizer.normalize( query, normalBuilder, parameters ) ;
      invalidMatcher = _normalizer.isInvalidMatcher() ;
      PD_RC_CHECK( rc, invalidMatcher ? PDERROR : PDDEBUG,
                   "Failed to normalize query [%s] with normalizer, rc: %d",
                   query.toString( FALSE, TRUE ).c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB__MTHHELP_GENSIMMTH, rc ) ;
      return rc ;

   error :
      _normalizer.clear() ;
      parameters.clearParams() ;
      goto done ;
   }

   /*
      _mthMatchRuntime implement
    */
   _mthMatchRuntime::_mthMatchRuntime ()
   : _mthMatchTreeHolder(),
     _mthParamPredListHolder()
   {
   }

   _mthMatchRuntime::~_mthMatchRuntime ()
   {
      _parameters.clearParams() ;
      clearPredList() ;
   }

   void _mthMatchRuntime::setQuery ( const BSONObj &query, BOOLEAN getOwned )
   {
      if ( getOwned )
      {
         _query = query.copy() ;
      }
      else
      {
         _query = query ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHRTM_GENPREDLST, "_mthMatchRuntime::generatePredList" )
   INT32 _mthMatchRuntime::generatePredList ( mthMatchHelper &matchHelper,
                                              const BSONObj &keyPattern,
                                              INT32 direction )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__MTHRTM_GENPREDLST ) ;

      clearPredList() ;

      if ( NULL != _matchTree )
      {
         rtnPredicateSet &predicateSet = matchHelper.getPredicateSet() ;
         RTN_PARAM_PREDICATE_LIST *paramPredList = getParamPredList() ;

         // Create predicate list
         if ( _parameters.isEmpty() || NULL == paramPredList )
         {
            UINT32 addedLevel = 0 ;
            // Not parameterized, initialize with predicate set
            rc = _predList.initialize( predicateSet, keyPattern, direction,
                                       addedLevel ) ;
            matchHelper.getNormalizer().setDoneByPred( keyPattern,
                                                       addedLevel ) ;
         }
         else if ( paramPredList->empty() )
         {
            // parameterized predicate list is empty, initialize predicate list
            // with predicate set, and generate parameterized predicate list
            // for the next query
            rc = _predList.initialize( predicateSet, keyPattern, direction,
                                       _parameters, (*paramPredList) ) ;
         }
         else
         {
            // parameterized predicate list is initialized, initialize
            // predicate list with parameterized predicate list
            rc = _predList.initialize( (*paramPredList), keyPattern, direction,
                                       _parameters ) ;
         }
         PD_RC_CHECK( rc, PDWARNING, "Failed to generate predicate list, "
                      "rc: %d", rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB__MTHRTM_GENPREDLST, rc ) ;
      return rc ;

   error :
      clearPredList() ;
      goto done ;
   }

   /*
      _mthMatchRuntimeHolder implement
    */
   _mthMatchRuntimeHolder::_mthMatchRuntimeHolder ()
   {
      _matchRuntime = NULL ;
      _ownedMatchRuntime = FALSE ;
   }

   _mthMatchRuntimeHolder::~_mthMatchRuntimeHolder ()
   {
      deleteMatchRuntime() ;
   }

   void _mthMatchRuntimeHolder::deleteMatchRuntime ()
   {
      if ( _ownedMatchRuntime && NULL != _matchRuntime )
      {
         SDB_OSS_DEL _matchRuntime ;
      }
      _matchRuntime = NULL ;
      _ownedMatchRuntime = FALSE ;
   }

   INT32 _mthMatchRuntimeHolder::createMatchRuntime ()
   {
      INT32 rc = SDB_OK ;

      deleteMatchRuntime() ;

      _matchRuntime = SDB_OSS_NEW mthMatchRuntime () ;
      PD_CHECK( _matchRuntime, SDB_OOM, error, PDERROR,
                "Failed to allocate match runtime" ) ;
      _ownedMatchRuntime = TRUE ;

   done :
      return rc ;
   error :
      goto done ;
   }

   void _mthMatchRuntimeHolder::getMatchRuntimeOnwed ( _mthMatchRuntimeHolder &holder )
   {
      deleteMatchRuntime() ;
      if ( holder._ownedMatchRuntime && NULL != holder._matchRuntime )
      {
         _matchRuntime = holder._matchRuntime ;
         _ownedMatchRuntime = TRUE ;
         holder._ownedMatchRuntime = FALSE ;
      }
   }

}

