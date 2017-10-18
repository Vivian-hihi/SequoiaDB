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

   Source File Name = optAccessPlanRuntime.hpp

   Descriptive Name = Optimizer Access Plan Runtime Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains runtime structure for
   access plan.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/17/2017  HGM  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OPTACCESSPLANRUNTIME_HPP__
#define OPTACCESSPLANRUNTIME_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "mthMatchRuntime.hpp"
#include "optAccessPlan.hpp"

using namespace bson ;

namespace engine
{
   class _optAccessPlanManager ;
   typedef class _optAccessPlanManager optAccessPlanManager ;

   /*
      _optQueryActivity define
    */
   class _optQueryActivity : public SDBObject
   {
      public :
         _optQueryActivity () ;

         _optQueryActivity ( INT64 contextID,
                             MON_OPERATION_TYPES optrType,
                             ossTick startTSTick,
                             ossTickDelta queryTimeTick ) ;

         virtual ~_optQueryActivity () ;

         void clear () ;

         _optQueryActivity & operator = ( const _optQueryActivity & activity ) ;

         void toBSON ( BSONObjBuilder &builder ) const ;

         OSS_INLINE INT64 getContextID () const
         {
            return _contextID ;
         }

         OSS_INLINE MON_OPERATION_TYPES getOptrType () const
         {
            return _optrType ;
         }

         OSS_INLINE const ossTick & getStartTSTick () const
         {
            return _startTSTick ;
         }

         OSS_INLINE const ossTickDelta & getQueryTimeTick () const
         {
            return _queryTimeTick ;
         }

         OSS_INLINE BOOLEAN isValid () const
         {
            return ( -1 != _contextID ||
                     MON_COUNTER_OPERATION_NONE != _optrType ) ;
         }

      protected :
         INT64                _contextID ;
         MON_OPERATION_TYPES  _optrType ;
         ossTick              _startTSTick ;
         ossTickDelta         _queryTimeTick ;
   } ;

   typedef class _optQueryActivity optQueryActivity ;

   /*
      _optAccessPlanInfo define
    */
   class _optAccessPlanInfo : public SDBObject,
                              public _optAccessPlanInfoBase
   {
      public :
         _optAccessPlanInfo () ;

         _optAccessPlanInfo ( const _optAccessPlanInfo &info ) ;

         virtual ~_optAccessPlanInfo () {}

         OSS_INLINE virtual void setIndexExtID ( dmsExtentID indexExtID )
         {
            _indexExtID = indexExtID ;
         }

         OSS_INLINE virtual void setIndexLID ( dmsExtentID indexLID )
         {
            _indexLID = indexLID ;
         }

         OSS_INLINE virtual dmsExtentID getIndexExtID () const
         {
            return _indexExtID ;
         }

         OSS_INLINE virtual dmsExtentID getIndexLID () const
         {
            return _indexLID ;
         }

         OSS_INLINE const CHAR *getCLFullName () const
         {
            return _clFullName ;
         }

         OSS_INLINE void setCLFullName ( const CHAR *pCLFullName )
         {
            if ( NULL != pCLFullName )
            {
               ossStrncpy( _clFullName, pCLFullName,
                           DMS_COLLECTION_FULL_NAME_SZ ) ;
            }
            else
            {
               _clFullName[0] = '\0' ;
            }
         }

      public :
         dmsExtentID       _indexExtID ;
         dmsExtentID       _indexLID ;
         CHAR              _clFullName[ DMS_COLLECTION_FULL_NAME_SZ + 1 ] ;
   } ;

   /*
      _optAccessPlanRuntime define
    */
   class _optAccessPlanRuntime : public SDBObject,
                                 public _mthMatchRuntimeHolder
   {
      public :
         _optAccessPlanRuntime () ;

         virtual ~_optAccessPlanRuntime () ;

         void clear () ;

         OSS_INLINE void inheritRuntime ( _optAccessPlanRuntime *planRuntime )
         {
            // The plan is reused, increase the reference count
            planRuntime->_plan->incRefCount() ;
            setPlan( planRuntime->_plan, planRuntime->_apm,
                     planRuntime->_isNewPlan ) ;
            setMatchRuntime( planRuntime->getMatchRuntime() ) ;
         }

         OSS_INLINE virtual const mthMatchRuntime *getMatchRuntime () const
         {
            return _matchRuntime ? _matchRuntime :
                                  ( _plan ? _plan->getMatchRuntime() : NULL ) ;
         }

         OSS_INLINE virtual mthMatchRuntime *getMatchRuntime ()
         {
            return _matchRuntime ? _matchRuntime :
                                  ( _plan ? _plan->getMatchRuntime() : NULL ) ;
         }

         virtual mthMatchRuntime *getMatchRuntime ( BOOLEAN checkValid ) ;

         INT32 createPlanInfo () ;

         void deletePlanInfo () ;

         INT32 bindPlanInfo ( const CHAR *pCLFullName,
                              dmsStorageUnit *su,
                              dmsMBContext *mbContext,
                              dmsExtentID indexExtID,
                              dmsExtentID indexLID ) ;

         OSS_INLINE _mthMatchTree *getMatchTree ()
         {
            return getMatchRuntime()->getMatchTree() ;
         }

         OSS_INLINE rtnPredicateList *getPredList ()
         {
            SDB_ASSERT ( _plan && _plan->isInitialized(),
                         "optAccessPlan must be optimized before start using" ) ;
            return ( _plan->getMatchRuntime()->isFixedPredList() ?
                     _plan->getMatchRuntime()->getPredList() :
                     getMatchRuntime()->getPredList() ) ;
         }

         OSS_INLINE rtnParamList &getParameters ()
         {
            return getMatchRuntime()->getParameters() ;
         }

         OSS_INLINE void setPlan ( optAccessPlan *plan,
                                   optAccessPlanManager *apm,
                                   BOOLEAN isNewPlan )
         {
            _plan = plan ;
            _apm = apm ;
            _isNewPlan = isNewPlan ;
         }

         INT32 bindParamPlan ( mthMatchHelper &matchHelper,
                               optAccessPlan *plan ) ;

         OSS_INLINE optAccessPlan *getPlan ()
         {
            return _plan ;
         }

         void releasePlan () ;

         OSS_INLINE BOOLEAN isNewPlan () const
         {
            return _isNewPlan ;
         }

         OSS_INLINE optScanType getScanType () const
         {
            return _plan ? _plan->getScanType() : UNKNOWNSCAN ;
         }

         OSS_INLINE BOOLEAN sortRequired () const
         {
            return _plan ? _plan->sortRequired() : FALSE ;
         }

         OSS_INLINE BOOLEAN isHintFailed () const
         {
            return _plan ? _plan->isHintFailed() : TRUE ;
         }

         OSS_INLINE BOOLEAN isAutoGen () const
         {
            return _plan ? _plan->isAutoGen() : FALSE ;
         }

         OSS_INLINE INT32 getDirection () const
         {
            SDB_ASSERT( _plan, "_plan is invalid" ) ;
            return _plan ? _plan->getDirection() : 1 ;
         }

         OSS_INLINE const CHAR *getIndexName () const
         {
            SDB_ASSERT( _plan, "_plan is invalid" ) ;
            return _plan ? _plan->getIndexName() : NULL ;
         }

         OSS_INLINE dmsExtentID getIndexCBExtent () const
         {
            SDB_ASSERT( _plan, "_plan is invalid" ) ;
            return _planInfo ? _planInfo->getIndexExtID() :
                               _plan->getIndexCBExtent() ;
         }

         OSS_INLINE dmsExtentID getIndexLID () const
         {
            SDB_ASSERT( _plan, "_plan is invalid" ) ;
            return _planInfo ? _planInfo->getIndexLID() :
                               _plan->getIndexLID() ;
         }

         OSS_INLINE const CHAR *getCLFullName () const
         {
            SDB_ASSERT( _plan, "_plan is invalid" ) ;
            return _planInfo ? _planInfo->getCLFullName() :
                               _plan->getCLFullName() ;
         }

         OSS_INLINE UINT16 getCLMBID () const
         {
            SDB_ASSERT( _plan, "_plan is invalid" ) ;
            return _planInfo ? _planInfo->getCLMBID() :
                               _plan->getCLMBID() ;
         }

         OSS_INLINE UINT32 getCLLID () const
         {
            SDB_ASSERT( _plan, "_plan is invalid" ) ;
            return _planInfo ? _planInfo->getCLLID() :
                               _plan->getCLLID() ;
         }

         void setQueryActivity ( INT64 contextID,
                                 MON_OPERATION_TYPES optrType,
                                 ossTick startTSTick,
                                 ossTickDelta queryTimeTick ) ;

      protected :
         // Pointer to access plan
         optAccessPlan *         _plan ;

         // Pointer to access plan manager
         optAccessPlanManager *  _apm ;

         // Whether query activity is set
         BOOLEAN                 _hasQueryActivity ;

         // Mark the plan is new created or got from cache
         BOOLEAN                 _isNewPlan ;

         // Used for main CL plan, bind sub-collection and index
         BOOLEAN                 _ownedPlanInfo ;
         _optAccessPlanInfo *    _planInfo ;
   } ;

   typedef class _optAccessPlanRuntime optAccessPlanRuntime ;

}

#endif //OPTACCESSPLANRUNTIME_HPP__

