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

   Source File Name = optAccessPlanRuntime.cpp

   Descriptive Name = Optimizer Access Plan Runtime

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains functions for runtime of
   access plan.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/17/2017  HGM Initial Draft

   Last Changed =

*******************************************************************************/

#include "optAccessPlanRuntime.hpp"
#include "pdTrace.hpp"
#include "optTrace.hpp"
#include "pmd.hpp"
#include "optAPM.hpp"
#include "rtnCB.hpp"

using namespace bson ;

namespace engine
{

   /*
      _optQueryActivity implement
    */
   _optQueryActivity::_optQueryActivity ()
   {
      clear() ;
   }

   _optQueryActivity::_optQueryActivity ( INT64 contextID,
                                          MON_OPERATION_TYPES optrType,
                                          ossTick startTSTick,
                                          ossTickDelta queryTimeTick )
   {
      _contextID = contextID ;
      _optrType = optrType ;
      _startTSTick = startTSTick ;
      _queryTimeTick = queryTimeTick ;
   }

   _optQueryActivity::~_optQueryActivity ()
   {
      clear() ;
   }

   void _optQueryActivity::clear ()
   {
      _contextID = -1 ;
      _optrType = MON_COUNTER_OPERATION_NONE ;
      _startTSTick.clear() ;
      _queryTimeTick.clear() ;
   }

   _optQueryActivity & _optQueryActivity::operator = (
                                       const _optQueryActivity & activity )
   {
      _contextID = activity._contextID ;
      _optrType = activity._optrType ;
      _startTSTick = activity._startTSTick ;
      _queryTimeTick = activity._queryTimeTick ;
      return (*this) ;
   }

   void _optQueryActivity::toBSON ( BSONObjBuilder &builder ) const
   {
      ossTickConversionFactor factor ;
      UINT32 seconds = 0, microseconds = 0 ;
      double queryTime = 0.0 ;
      CHAR timestampStr[ OSS_TIMESTAMP_STRING_LEN + 1 ] = { 0 } ;
      ossTimestamp startTime ;

      _queryTimeTick.convertToTime( factor, seconds, microseconds ) ;
      queryTime = (double)( seconds ) +
                  (double)( microseconds ) / (double)( OSS_ONE_MILLION ) ;

      _startTSTick.convertToTimestamp( startTime ) ;
      ossTimestampToString( startTime, timestampStr ) ;

      builder.append( OPT_FIELD_CONTEXT_ID, _contextID ) ;
      switch ( _optrType )
      {
         case MON_UPDATE :
            builder.append( OPT_FIELD_QUERY_TYPE, OPT_QUERY_TYPE_UPDATE ) ;
            break ;
         case MON_DELETE :
            builder.append( OPT_FIELD_QUERY_TYPE, OPT_QUERY_TYPE_DELETE ) ;
            break ;
         default :
            builder.append( OPT_FIELD_QUERY_TYPE, OPT_QUERY_TYPE_SELECT ) ;
            break ;
      }

      builder.append( OPT_QUERY_TIME_SPENT, queryTime ) ;
      builder.append( OPT_QUERY_START_TIME, timestampStr ) ;
   }

   /*
      _optAccessPlanInfo implement
    */
   _optAccessPlanInfo::_optAccessPlanInfo ()
   : _optAccessPlanInfoBase()
   {
      _indexExtID = DMS_INVALID_EXTENT ;
      _indexLID = DMS_INVALID_EXTENT ;
      setCLFullName( NULL ) ;
   }

   _optAccessPlanInfo::_optAccessPlanInfo ( const _optAccessPlanInfo &info )
   : _optAccessPlanInfoBase( info )
   {
      _indexExtID = info._indexExtID ;
      _indexLID = info._indexLID ;
      setCLFullName( info._clFullName ) ;
   }


   /*
      _optAccessPlanRuntime implement
    */
   _optAccessPlanRuntime::_optAccessPlanRuntime ()
   : _mthMatchRuntimeHolder()
   {
      _plan = NULL ;
      _apm = NULL ;
      _isNewPlan = FALSE ;
      _hasQueryActivity = FALSE ;
      _ownedPlanInfo = FALSE ;
      _planInfo = NULL ;
   }

   _optAccessPlanRuntime::~_optAccessPlanRuntime ()
   {
      clear() ;
   }

   void _optAccessPlanRuntime::clear ()
   {
      deleteMatchRuntime() ;
      deletePlanInfo() ;
      _isNewPlan = FALSE ;
      _hasQueryActivity = FALSE ;
      _apm = NULL ;
      releasePlan() ;
   }

   mthMatchRuntime *_optAccessPlanRuntime::getMatchRuntime ( BOOLEAN checkValid )
   {
      mthMatchRuntime *matchRuntime = getMatchRuntime() ;
      if ( checkValid && matchRuntime )
      {
         if ( !_plan->getMatchTree()->isInitialized() ||
              _plan->getMatchTree()->isMatchesAll() )
         {
            matchRuntime = NULL ;
         }
      }
      return matchRuntime ;
   }

   INT32 _optAccessPlanRuntime::createPlanInfo ()
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( NULL == _planInfo, "_planInfo should be NULL" ) ;

      _planInfo = SDB_OSS_NEW _optAccessPlanInfo() ;
      PD_CHECK( _planInfo, SDB_OOM, error, PDERROR,
                "Failed to allocate plan info" ) ;
      _ownedPlanInfo = TRUE ;

      done :
         return rc ;
      error :
         goto done ;
   }

   void _optAccessPlanRuntime::deletePlanInfo ()
   {
      if ( _ownedPlanInfo && NULL != _planInfo )
      {
         SDB_OSS_DEL _planInfo ;
      }
      _planInfo = NULL ;
      _ownedPlanInfo = FALSE ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPRTM_BINDPLANINFO, "_optAccessPlanRuntime::bindPlanInfo" )
   INT32 _optAccessPlanRuntime::bindPlanInfo ( const CHAR *pCLFullName,
                                               dmsStorageUnit *su,
                                               dmsMBContext *mbContext,
                                               dmsExtentID indexExtID,
                                               dmsExtentID indexLID )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPRTM_BINDPLANINFO ) ;

      if ( NULL == _planInfo )
      {
         rc = createPlanInfo() ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create plan info, rc: %d", rc ) ;
      }

      _planInfo->setCLFullName( pCLFullName ) ;
      _planInfo->setCSInfo( su ) ;
      _planInfo->setCLInfo( mbContext ) ;
      _planInfo->setIndexExtID( indexExtID ) ;
      _planInfo->setIndexLID( indexLID ) ;

   done :
      PD_TRACE_EXITRC( SDB_OPTAPRTM_BINDPLANINFO, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_OPTAPRTM_BINDPARAMPLAN, "_optAccessPlanRuntime::bindParamPlan" )
   INT32 _optAccessPlanRuntime::bindParamPlan ( mthMatchHelper &matchHelper,
                                                optAccessPlan *plan )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_OPTAPRTM_BINDPARAMPLAN ) ;

      SDB_ASSERT ( NULL != plan &&
                   plan->getCacheLevel() >= OPT_PLAN_PARAMETERIZED,
                   "plan is invalid" ) ;

      mthMatchRuntime *matchRuntime = _matchRuntime ;
      if ( NULL == matchRuntime )
      {
         matchRuntime = plan->getMatchRuntime() ;
      }
      SDB_ASSERT( matchRuntime, "matchRuntime is invalid" ) ;

      rc = plan->bindMatchRuntime( matchRuntime ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to bind match runtime, rc: %d", rc ) ;

      // Only need to bind predicates for
      // 1. index-scan plan
      // 2. no predicates in current match runtime
      // 3. the plan's predicates are not fixed
      if ( IXSCAN == plan->getScanType() &&
           NULL == matchRuntime->getPredList() &&
           !plan->getMatchRuntime()->isFixedPredList() )
      {
         rc = matchRuntime->generatePredList( matchHelper,
                                              plan->getKeyPattern(),
                                              plan->getDirection() ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to generate predicate list, rc: %d",
                      rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_OPTAPRTM_BINDPARAMPLAN, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   void _optAccessPlanRuntime::releasePlan ()
   {
      if ( NULL != _plan )
      {
         _plan->release() ;
         _plan = NULL ;
      }
   }

   void _optAccessPlanRuntime::setQueryActivity ( INT64 contextID,
                                                  MON_OPERATION_TYPES optrType,
                                                  ossTick startTSTick,
                                                  ossTickDelta queryTimeTick )
   {
      if ( NULL != _plan && NULL != _apm && !_hasQueryActivity )
      {
         optQueryActivity queryActivity( contextID, optrType, startTSTick,
                                         queryTimeTick ) ;
         _apm->setQueryActivity( _plan->getActivityID(), queryActivity ) ;
         _hasQueryActivity = TRUE ;
      }
   }

}

