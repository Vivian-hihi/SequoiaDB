/*******************************************************************************

   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = rtnLobAccessManager.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/28/2017  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "rtnLobAccessManager.hpp"
#include "msgDef.h"
#include "pd.hpp"

namespace engine
{
   _rtnLobAccessInfo::_rtnLobAccessInfo( const bson::OID& oid, UINT32 mode, INT64 contextId )
      : _mode( mode ),
        _refCount( 0 )
   {
      _oid = oid ;
      if ( SDB_LOB_MODE_CREATEONLY == mode ||
           SDB_LOB_MODE_REMOVE == mode )
      {
         _contextId = contextId ;
      }
      else
      {
         _contextId = -1 ;
      }
   }

   _rtnLobAccessInfo::~_rtnLobAccessInfo()
   {
   }

   _rtnLobAccessManager::_rtnLobAccessManager()
   {
   }

   _rtnLobAccessManager::~_rtnLobAccessManager()
   {
      FOR_EACH_CMAP_BUCKET_X(RTN_LOB_MAP, _lobMap)
      {
         for ( RTN_LOB_MAP::map_const_iterator lobIter = bucket.begin() ;
               lobIter != bucket.end() ;
               lobIter++ )
         {
            _rtnLobAccessInfo* lobAccessInfo = lobIter->second ;
            SDB_OSS_DEL lobAccessInfo ;
         }

         bucket.clear() ;
      }
      FOR_EACH_CMAP_BUCKET_END
   }

   INT32 _rtnLobAccessManager::getAccessPrivilege( std::string clName, const bson::OID& oid, UINT32 mode, INT64 contextId )
   {
      INT32 rc = SDB_OK ;
      _rtnLobAccessKey key( clName, oid ) ;

      RTN_LOB_MAP::Bucket& bucket = _lobMap.getBucket( key ) ;
      BUCKET_XLOCK( bucket ) ;

      RTN_LOB_MAP::map_const_iterator lobIter = bucket.find( key ) ;
      if ( lobIter != bucket.end() )
      {
         _rtnLobAccessInfo* lobAccessInfo = lobIter->second ;
         switch ( lobAccessInfo->getMode() )
         {
         case SDB_LOB_MODE_CREATEONLY:
            // pass through
         case SDB_LOB_MODE_REMOVE:
            rc = SDB_LOB_IS_IN_USE ;
            goto error ;
         case SDB_LOB_MODE_READ:
            if ( SDB_LOB_MODE_READ != mode )
            {
               rc = SDB_LOB_IS_IN_USE ;
               goto error ;
            }
            else
            {
               lobAccessInfo->incRefCount() ;
               break ;
            }
         default:
            SDB_ASSERT( FALSE, "invalid mode" ) ;
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid LOB access mode: %u", lobAccessInfo->getMode() ) ;
            goto error ;
         }
      }
      else
      {
         _rtnLobAccessInfo* lobAccessInfo = SDB_OSS_NEW _rtnLobAccessInfo( oid, mode, contextId ) ;
         if ( NULL == lobAccessInfo )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Failed to alloc _rtnLobAccessInfo, rc=%d", rc ) ;
            goto error ;
         }

         try
         {
            bucket.insert( RTN_LOB_MAP::value_type( key, lobAccessInfo ) ) ;
         }
         catch ( std::exception& e )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Unexpected error happened: %s", e.what() ) ;
            goto error ;
         }

         if ( SDB_LOB_MODE_READ == mode )
         {
            lobAccessInfo->incRefCount() ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnLobAccessManager::releaseAccessPrivilege( std::string clName, const bson::OID& oid, UINT32 mode, INT64 contextId )
   {
      INT32 rc = SDB_OK ;
      _rtnLobAccessKey key( clName, oid ) ;

      RTN_LOB_MAP::Bucket& bucket = _lobMap.getBucket( key ) ;
      BUCKET_XLOCK( bucket ) ;

      RTN_LOB_MAP::map_const_iterator lobIter = bucket.find( key ) ;
      if ( lobIter != bucket.end() )
      {
         _rtnLobAccessInfo* lobAccessInfo = lobIter->second ;

         SDB_ASSERT( oid == lobAccessInfo->getOID(), "incorrect oid" ) ;

         if ( mode != lobAccessInfo->getMode() )
         {
            SDB_ASSERT( mode == lobAccessInfo->getMode(), "incorrect mode" ) ;
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid LOB access mode: %u, %u", mode, lobAccessInfo->getMode() ) ;
            goto error ;
         }

         if ( -1 != contextId &&
              -1 != lobAccessInfo->getContextId() &&
              contextId != lobAccessInfo->getContextId() )
         {
            SDB_ASSERT( contextId != lobAccessInfo->getContextId(), 
                        "incorrect contextId" ) ;
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid LOB access contextId: %lld, %lld",
                    contextId, lobAccessInfo->getContextId() ) ;
            goto error ;
         }

         switch ( lobAccessInfo->getMode() )
         {
         case SDB_LOB_MODE_CREATEONLY:
            // pass through
         case SDB_LOB_MODE_REMOVE:
            bucket.erase( key ) ;
            SAFE_OSS_DELETE( lobAccessInfo ) ;
            break ;
         case SDB_LOB_MODE_READ:
            lobAccessInfo->decRefCount() ;
            if ( 0 == lobAccessInfo->getRefCount() )
            {
               bucket.erase( key ) ;
               SAFE_OSS_DELETE( lobAccessInfo ) ;
            }
            break ;
         default:
            SDB_ASSERT( FALSE, "invalid mode" ) ;
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid LOB access mode: %u", lobAccessInfo->getMode() ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   _rtnLobAccessManager* sdbGetRTNLobAccessManager()
   {
      static _rtnLobAccessManager _lobAccessManager ;
      return &_lobAccessManager ;
   }
}

