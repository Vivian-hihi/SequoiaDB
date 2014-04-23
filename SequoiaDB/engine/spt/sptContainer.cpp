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

   Source File Name = sptContainer.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptContainer.hpp"
#include "sptSPScope.hpp"
#include "pd.hpp"

namespace engine
{
   _sptContainer::_sptContainer()
   {
   }

   _sptContainer::~_sptContainer()
   {
   }

   _sptScope *_sptContainer::newScope( SPT_SCOPE_TYPE type )
   {
      _sptScope *scope = NULL ;
      INT32 rc = SDB_OK ;
      if ( SPT_SCOPE_TYPE_SP == type )
      {
         scope = SDB_OSS_NEW _sptSPScope() ;
      }

      if ( NULL == scope )
      {
         PD_LOG( PDERROR, "it is a unknown type of scope:%d", type ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = scope->start() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init scope:%d", rc ) ;
         goto error ;
      }
   done:
      return scope ;
   error:
      SAFE_OSS_DELETE( scope ) ;
      scope = NULL ;
      goto done ;
   }
}

