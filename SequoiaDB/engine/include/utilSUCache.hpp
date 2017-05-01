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

   Source File Name = utilSUCache.hpp

   Descriptive Name = utility of storage unit cache management header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for SU cache
   management ( including plan cache, statistics cache, etc. )

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/
#ifndef UTILSUCACHE_HPP__
#define UTILSUCACHE_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"

namespace engine
{

   #define UTIL_SU_INVALID_UNITID ( 65535 )

   class _IUtilSUCacheHolder ;

   enum UTIL_SU_CACHE_UMIT_TYPE
   {
      UTIL_SU_CACHE_UNIT_CLSTAT,
      UTIL_SU_CACHE_UNIT_IDXSTAT
   } ;

   class _utilSUCacheUnit ;
   typedef class _utilSUCacheUnit utilSUCacheUnit ;

   /*
      _utilSUCacheUnit define
    */
   class _utilSUCacheUnit : public SDBObject
   {
      public :
         _utilSUCacheUnit ()
         {
            _unitID = UTIL_SU_INVALID_UNITID ;
            _version = 0 ;
         }

         _utilSUCacheUnit ( UINT16 unitID, UINT64 version )
         {
            _unitID = unitID ;
            _version = version ;
         }

         virtual ~_utilSUCacheUnit () {}

         OSS_INLINE UINT16 getUnitID () const
         {
            return _unitID ;
         }

         OSS_INLINE UINT64 getVersion () const
         {
            return _version ;
         }

         OSS_INLINE void setVersion ( UINT64 version )
         {
            _version = version ;
         }

         virtual const CHAR *getCSName () const = 0 ;

         virtual void setCSName ( const CHAR *pCSName ) = 0 ;

         virtual const CHAR *getCLName () const = 0 ;

         virtual void setCLName ( const CHAR *pCLName ) = 0 ;

         virtual UTIL_SU_CACHE_UMIT_TYPE getUnitType () const = 0 ;

         virtual BOOLEAN addSubUnit ( utilSUCacheUnit *pSubUnit,
                                      BOOLEAN ignoreVersion ) = 0 ;

         virtual void clearSubUnits () = 0 ;

      protected :
         OSS_INLINE void _setUnitID ( UINT16 unitID )
         {
            _unitID = unitID ;
         }

      protected :
         // Unit ID to index in cache
         UINT16   _unitID ;

         // Version (timestamp) of the cache unit
         UINT64   _version ;
   } ;

   /*
      _utilSUCache define
    */
   class _utilSUCache : public SDBObject
   {
      public :
         _utilSUCache ( UINT16 size, UINT32 type,
                        UTIL_SU_CACHE_UMIT_TYPE uintType,
                        _IUtilSUCacheHolder *pHolder = NULL ) ;

         virtual ~_utilSUCache () ;

         OSS_INLINE BOOLEAN isValid () const
         {
            return NULL != _pUnits ;
         }

         OSS_INLINE UINT32 getType () const
         {
            return _type ;
         }

         OSS_INLINE const utilSUCacheUnit *getCacheUnit ( UINT16 unitID ) const
         {
            if ( _pUnits && unitID < _size )
            {
               return _pUnits[ unitID ] ;
            }
            return NULL ;
         }

         OSS_INLINE utilSUCacheUnit *getCacheUnit ( UINT16 unitID )
         {
            if ( _pUnits && unitID < _size )
            {
               return _pUnits[ unitID ] ;
            }
            return NULL ;
         }

         OSS_INLINE UINT32 getSize () const
         {
            return _size ;
         }

         BOOLEAN addCacheUnit ( utilSUCacheUnit *pUnit, BOOLEAN ignoreVersion ) ;

         BOOLEAN addCacheSubUnit ( utilSUCacheUnit *pSubUnit,
                                   BOOLEAN ignoreVersion ) ;

         BOOLEAN removeCacheUnit ( UINT16 unitID, BOOLEAN needDelete ) ;

         BOOLEAN clearCacheUnits () ;

      protected :
         UINT16                  _size ;
         UINT32                  _type ;
         UTIL_SU_CACHE_UMIT_TYPE _unitType ;
         _IUtilSUCacheHolder *   _pHolder ;
         utilSUCacheUnit **      _pUnits ;
   } ;

   typedef class _utilSUCache utilSUCache ;

   /*
      _IUtilSUCacheHolder define
    */
   class _IUtilSUCacheHolder
   {
      public :
         _IUtilSUCacheHolder () {}

         virtual ~_IUtilSUCacheHolder () {}

         virtual const CHAR *getCSName () const = 0 ;

         virtual BOOLEAN isSysSU () const = 0 ;

         virtual BOOLEAN checkCacheUnit ( utilSUCacheUnit *pCacheUnit ) = 0 ;

         virtual BOOLEAN createSUCache ( UINT32 type ) = 0 ;

         virtual BOOLEAN deleteSUCache ( UINT32 type ) = 0 ;

         virtual void deleteAllSUCaches () = 0 ;

         virtual utilSUCache *getSUCache ( UINT32 type ) = 0 ;
   } ;

}

#endif //UTILSUCACHE_HPP__

