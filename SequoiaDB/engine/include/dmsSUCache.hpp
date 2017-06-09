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

   Source File Name = dmsSUCache.hpp

   Descriptive Name = Data Management Service SU Cache Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS event handler.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/
#ifndef DMS_SUCACHE_HPP_
#define DMS_SUCACHE_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "utilSUCache.hpp"

namespace engine
{

   #define DMS_CACHE_TYPE_STAT ( 0 )
   #define DMS_CACHE_TYPE_NUM  ( 1 )

   typedef class _utilSUCache<UTIL_SU_CACHE_DFT_SIZE> dmsSUCache ;
   typedef class _IUtilSUCacheHolder<DMS_MME_SLOTS> IDmsSUCacheHolder ;

   /*
      _dmsStatCache define
    */
   class _dmsStatCache : public dmsSUCache
   {
      public :
         _dmsStatCache( IDmsSUCacheHolder *pHolder = NULL )
         : dmsSUCache( DMS_CACHE_TYPE_STAT, UTIL_SU_CACHE_UNIT_CLSTAT, pHolder )
         {
         }

         virtual ~_dmsStatCache () {}
   } ;

   typedef class _dmsStatCache dmsStatCache ;

}

#endif //DMS_SUCACHE_HPP_

