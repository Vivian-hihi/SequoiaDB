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

   Source File Name = optAccessPlanKey.hpp

   Descriptive Name = Optimizer Access Plan Key Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains structure for key of
   access plan.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/17/2017  HGM  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OPTACCESSPLANKEY_HPP__
#define OPTACCESSPLANKEY_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "../bson/oid.h"
#include "../bson/bson.h"
#include "ossUtil.hpp"
#include "utilHashTable.hpp"
#include "utilList.hpp"
#include "rtnQueryOptions.hpp"
#include "dms.hpp"
#include "dmsStorageUnit.hpp"
#include "mthMatchRuntime.hpp"

using namespace bson ;

namespace engine
{

   /*
      _optAccessPlanInfoBase define
    */
   class _optAccessPlanInfoBase
   {
      public :
         _optAccessPlanInfoBase ()
         {
            _suID = DMS_INVALID_SUID ;
            _suLID = DMS_INVALID_LOGICCSID ;
            _clLID = DMS_INVALID_CLID ;
            _mbID = DMS_INVALID_MBID ;
         }

         _optAccessPlanInfoBase ( const _optAccessPlanInfoBase &info )
         {
            _suID = info._suID ;
            _suLID = info._suLID ;
            _clLID = info._clLID ;
            _mbID = info._mbID ;
         }

         virtual ~_optAccessPlanInfoBase ()
         {
         }

         OSS_INLINE virtual dmsStorageUnitID getSUID () const
         {
            return _suID ;
         }

         OSS_INLINE virtual UINT32 getSULID () const
         {
            return _suLID ;
         }

         OSS_INLINE virtual UINT16 getCLMBID () const
         {
            return _mbID ;
         }

         OSS_INLINE virtual UINT32 getCLLID () const
         {
            return _clLID ;
         }

         OSS_INLINE virtual void setCSInfo ( dmsStorageUnit *su )
         {
            if ( NULL != su )
            {
               _suID = su->CSID() ;
               _suLID = su->LogicalCSID() ;
            }
         }

         OSS_INLINE virtual void setCLInfo ( dmsMBContext *mbContext )
         {
            if ( NULL != mbContext )
            {
               _mbID = mbContext->mbID() ;
               _clLID = mbContext->clLID() ;
            }
         }

      protected :
         dmsStorageUnitID        _suID ;
         UINT32                  _suLID ;
         UINT32                  _clLID ;
         UINT16                  _mbID ;
   } ;

   /*
      _optAccessPlanKey define
    */
   class _optAccessPlanKey : public _rtnQueryOptions,
                             public _utilHashTableKey,
                             public _optAccessPlanInfoBase
   {
      public :
         _optAccessPlanKey ( const rtnQueryOptions &options,
                             OPT_PLAN_CACHE_LEVEL cacheLevel ) ;

         _optAccessPlanKey ( _optAccessPlanKey &planKey ) ;

         virtual ~_optAccessPlanKey () ;

         OSS_INLINE virtual INT32 getOwned ()
         {
            _normalizedQuery = _normalizedQuery.getOwned() ;
            return _rtnQueryOptions::getOwned() ;
         }

         OSS_INLINE const CHAR *getCLFullName () const
         {
            return _fullName ;
         }

         OSS_INLINE virtual UINT32 getKeyCode () const
         {
            return _keyCode ;
         }

         OSS_INLINE void setValid ( BOOLEAN valid )
         {
            _isValid = valid ;
         }

         OSS_INLINE BOOLEAN getValid () const
         {
            return _isValid ;
         }

         virtual BOOLEAN isEqual ( const _optAccessPlanKey &key ) const ;

         OSS_INLINE const BSONObj &getQuery () const
         {
            return _query ;
         }

         OSS_INLINE const BSONObj &getNormalizedQuery () const
         {
            return _normalizedQuery ;
         }

         OSS_INLINE const BSONObj &getOrderBy () const
         {
            return _orderBy ;
         }

         OSS_INLINE const BSONObj &getHint () const
         {
            return _hint ;
         }

         OSS_INLINE const BSONObj &getSelector () const
         {
            return _selector ;
         }

         OSS_INLINE INT32 getFlag () const
         {
            return _flag ;
         }

         OSS_INLINE SINT64 getSkip () const
         {
            return _skip ;
         }

         OSS_INLINE SINT64 getLimit () const
         {
            return _limit ;
         }

         OSS_INLINE OPT_PLAN_CACHE_LEVEL getCacheLevel () const
         {
            return _cacheLevel ;
         }

         OSS_INLINE BOOLEAN isValid () const
         {
            return _isValid ;
         }

         OSS_INLINE void generateKeyCode ( dmsStorageUnit *su, dmsMBContext *mbContext )
         {
            setCSInfo( su ) ;
            setCLInfo( mbContext ) ;

            if ( _cacheLevel > OPT_PLAN_NOCACHE )
            {
               // Key code is not needed for no-cache mode
               _generateKeyCodeInternal() ;
            }

            _isValid = TRUE ;
         }

         INT32 normalize ( mthMatchHelper &matchHelper,
                           mthMatchRuntime *matchRuntime ) ;

         OSS_INLINE const CHAR *getCacheLevelName () const
         {
            switch ( _cacheLevel )
            {
               case OPT_PLAN_ORIGINAL :
                  return OPT_CACHE_ORIGINAL_NAME ;
               case OPT_PLAN_NORMALIZED :
                  return OPT_CACHE_NORMALIZED_NAME ;
               case OPT_PLAN_PARAMETERIZED :
                  return OPT_CACHE_PARAMETERIZED_NAME ;
               case OPT_PLAN_FUZZYOPTR :
                  return OPT_CACHE_FUZZYOPTR_NAME ;
               default :
                  break ;
            }
            return OPT_CACHE_NOCACHE_NAME ;
         }

      protected :

         void _generateKeyCodeInternal () ;

         UINT32 _generateKeyCodeHash () ;

      protected :
         BOOLEAN                 _isValid ;
         OPT_PLAN_CACHE_LEVEL    _cacheLevel ;
         BSONObj                 _normalizedQuery ;
   } ;

   typedef class _optAccessPlanKey optAccessPlanKey ;

}

#endif //OPTACCESSPLANKEY_HPP__

