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

   Source File Name = rtnLobAccessManager.hpp

   Descriptive Name = LOB access manager

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/28/2017  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTN_LOB_ACCESS_MANAGER_HPP_
#define RTN_LOB_ACCESS_MANAGER_HPP_

#include "oss.hpp"
#include "ossUtil.hpp"
#include "utilConcurrentMap.hpp"
#include "../bson/bson.hpp"
#include <string>

namespace engine
{
   class _rtnLobAccessInfo: public SDBObject
   {
   public:
      _rtnLobAccessInfo( const bson::OID& oid, UINT32 mode, INT64 contextId = -1 ) ;
      ~_rtnLobAccessInfo() ;

   private:
      // disallow copy and assign
      _rtnLobAccessInfo( const _rtnLobAccessInfo& ) ;
      void operator=( const _rtnLobAccessInfo& ) ;

   public:
      OSS_INLINE const bson::OID&   getOID() const { return _oid ; }
      OSS_INLINE UINT32             getMode() const { return _mode ; }
      OSS_INLINE INT32              getRefCount() const { return _refCount ; }
      OSS_INLINE INT64              getContextId() const { return _contextId ; }

      OSS_INLINE void               incRefCount() { _refCount++ ; }
      OSS_INLINE void               decRefCount() { _refCount-- ; }

   private:
      bson::OID   _oid ;
      UINT32      _mode ;
      INT32       _refCount ;
      INT64       _contextId ;
   } ;
   typedef _rtnLobAccessInfo rtnLobAccessInfo ;

   struct _rtnLobAccessKey: public SDBObject
   {
      std::string clName ;
      bson::OID   oid ;

      _rtnLobAccessKey( const std::string& lobCLName, const bson::OID& lobOid )
      {
         clName = lobCLName ;
         oid = lobOid ;
      }

      bool operator<( const _rtnLobAccessKey& other ) const
      {
         INT32 result = clName.compare( other.clName ) ;
         if ( result < 0 )
         {
            return true ;
         }
         else if ( result > 0 )
         {
            return false ;
         }
         else // ( 0 == result )
         {
            return oid < other.oid ;
         }
      }
   } ;

   class _rtnLobAccessKeyHasher
   {
   public:
      _rtnLobAccessKeyHasher() {}
      ~_rtnLobAccessKeyHasher() {}

      UINT32 operator() ( const _rtnLobAccessKey& key ) const
      {
         UINT32 nameHash = ossHash( key.clName.c_str(), (UINT32)key.clName.size() ) ;
         UINT32 oidHash = ossHash( (const CHAR*)&(key.oid), (UINT32)sizeof( bson::OID ) ) ;
         return ( nameHash << 5 ) + oidHash ;
      }
   } ;

   typedef utilConcurrentMap<_rtnLobAccessKey, _rtnLobAccessInfo*, UTIL_CONCURRENT_MAP_DEFAULT_BUCKET_NUM, _rtnLobAccessKeyHasher> RTN_LOB_MAP ;

   class _rtnLobAccessManager: public SDBObject
   {
   public:
      _rtnLobAccessManager() ;
      ~_rtnLobAccessManager() ;

   public:
      INT32 getAccessPrivilege( std::string clName, const bson::OID& oid, UINT32 mode, INT64 contextId = -1 ) ;
      INT32 releaseAccessPrivilege( std::string clName, const bson::OID& oid, UINT32 mode, INT64 contextId = -1 ) ;

   private:
      RTN_LOB_MAP _lobMap ;
   } ;
   typedef _rtnLobAccessManager rtnLobAccessManager ;

   _rtnLobAccessManager* sdbGetRTNLobAccessManager() ;
}

#endif /* RTN_LOB_ACCESS_MANAGER_HPP_ */

