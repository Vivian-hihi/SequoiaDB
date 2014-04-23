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

   Source File Name = sptContainer.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_CONTAINER_HPP_
#define SPT_CONTAINER_HPP_

#include "core.hpp"
#include "oss.hpp"

namespace engine
{
   enum SPT_SCOPE_TYPE
   {
      SPT_SCOPE_TYPE_SP = 0,
      SPT_SCOPE_TYPE_V8 = 1,
   } ;

   class _sptScope ;

   class _sptContainer : public SDBObject
   {
   public:
      _sptContainer() ;
      virtual ~_sptContainer() ;

   public:
      _sptScope *newScope( SPT_SCOPE_TYPE type ) ;
   } ;

   typedef class _sptContainer sptContainer ;
}

#endif

