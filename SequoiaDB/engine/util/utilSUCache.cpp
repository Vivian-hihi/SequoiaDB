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

   Source File Name = utilSUCache.cpp

   Descriptive Name = utility of storage unit cache management

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

#include "utilSUCache.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"

namespace engine
{

   /*
      _utilSUCacheMap implement
    */
   _utilSUCache::_utilSUCache ( UINT16 size, UINT32 type,
                                UTIL_SU_CACHE_UMIT_TYPE unitType,
                                _IUtilSUCacheHolder *pHolder )
   {
      _size = 0 ;
      _type = type ;
      _unitType = unitType ;
      _pHolder = pHolder ;

      // Do not allocate cache size with UTIL_SU_INVALID_UNITID which is
      // hold for invalid unit ID
      if ( size > 0 && size < UTIL_SU_INVALID_UNITID )
      {
         UINT32 memSize = sizeof( utilSUCacheUnit * ) * size ;
         _pUnits = (utilSUCacheUnit **)SDB_OSS_MALLOC( memSize ) ;
         if ( _pUnits )
         {
            ossMemset( _pUnits, 0, memSize ) ;
            _size = size ;
         }
      }
   }

   _utilSUCache::~_utilSUCache ()
   {
      if ( _pUnits )
      {
         clearCacheUnits () ;
         SDB_OSS_FREE( _pUnits ) ;
         _pUnits = NULL ;
         _size = 0 ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILSUCACHE_ADDUNIT, "_utilSUCache::addCacheUnit" )
   BOOLEAN _utilSUCache::addCacheUnit ( utilSUCacheUnit *pUnit,
                                        BOOLEAN ignoreVersion )
   {
      BOOLEAN added = FALSE ;

      PD_TRACE_ENTRY( SDB__UTILSUCACHE_ADDUNIT ) ;

      if ( _pUnits && pUnit && pUnit->getUnitType() == _unitType )
      {
         UINT16 unitID = pUnit->getUnitID() ;
         if ( unitID < _size )
         {
            utilSUCacheUnit *pTmpUnit = _pUnits[ unitID ] ;
            if ( pTmpUnit )
            {
               if ( ignoreVersion || pTmpUnit->getVersion() < pUnit->getVersion() )
               {
                  SDB_OSS_DEL pTmpUnit ;
                  _pUnits[ unitID ] = pUnit ;
                  added = TRUE ;
               }
            }
            else
            {
               if ( !_pHolder || _pHolder->checkCacheUnit( pUnit ) )
               {
                  _pUnits[ unitID ] = pUnit ;
                  added = TRUE ;
               }
            }
         }
      }

      PD_TRACE_EXIT( SDB__UTILSUCACHE_ADDUNIT ) ;

      return added ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILSUCACHE_ADDSUBUNIT, "_utilSUCache::addCacheSubUnit" )
   BOOLEAN _utilSUCache::addCacheSubUnit ( utilSUCacheUnit *pSubUnit,
                                           BOOLEAN ignoreVersion )
   {
      BOOLEAN added = FALSE ;

      PD_TRACE_ENTRY( SDB__UTILSUCACHE_ADDSUBUNIT ) ;

      if ( _pUnits && pSubUnit )
      {
         UINT16 unitID = pSubUnit->getUnitID() ;
         utilSUCacheUnit *pUnit = getCacheUnit( unitID ) ;
         if ( pUnit )
         {
            if ( !_pHolder || _pHolder->checkCacheUnit( pSubUnit ) )
            {
               added = pUnit->addSubUnit( pSubUnit, ignoreVersion ) ;
            }
         }
      }

      PD_TRACE_EXIT( SDB__UTILSUCACHE_ADDSUBUNIT ) ;

      return added ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILSUCACHE_RMCUNIT, "_utilSUCache::removeCacheUnit" )
   BOOLEAN _utilSUCache::removeCacheUnit ( UINT16 unitID, BOOLEAN needDelete )
   {
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB__UTILSUCACHE_RMCUNIT ) ;

      if ( _pUnits && unitID < _size )
      {
         utilSUCacheUnit *pTmpUnit = _pUnits[ unitID ] ;
         if ( pTmpUnit )
         {
            if ( needDelete )
            {
               SDB_OSS_DEL pTmpUnit ;
            }
            _pUnits[ unitID ] = NULL ;
            deleted = TRUE ;
         }
      }

      PD_TRACE_EXIT( SDB__UTILSUCACHE_RMCUNIT ) ;

      return deleted ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILSUCACHE_CLEARUNITS, "_utilSUCache::clearCacheUnits" )
   BOOLEAN _utilSUCache::clearCacheUnits ()
   {
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB__UTILSUCACHE_CLEARUNITS ) ;

      if ( _pUnits )
      {
         for ( UINT16 unitID = 0 ; unitID < _size ; unitID ++ )
         {
            utilSUCacheUnit *pTmpUnit = _pUnits[ unitID ] ;
            if ( pTmpUnit )
            {
               SDB_OSS_DEL pTmpUnit ;
               _pUnits[ unitID ] = NULL ;
               deleted = TRUE ;
            }
         }
      }

      PD_TRACE_EXIT( SDB__UTILSUCACHE_CLEARUNITS ) ;

      return deleted ;
   }
}

