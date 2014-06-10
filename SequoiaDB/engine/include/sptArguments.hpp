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

   Source File Name = sptArguments.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_ARGUMENTS_HPP_
#define SPT_ARGUMENTS_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.hpp"

namespace engine
{
   class _sptArguments : public SDBObject
   {
   public:
      _sptArguments() {}
      virtual ~_sptArguments(){}

   public:
      /// start with zero.
      virtual INT32 getNative( UINT32 pos, void *value ) const = 0 ;
      virtual INT32 getString( UINT32 pos, std::string &value ) const = 0 ;
      virtual INT32 getBsonobj( UINT32 pos, bson::BSONObj &value ) const = 0 ;
      virtual UINT32 argc() const = 0 ;
   } ;
   typedef class _sptArguments sptArguments ;
}

#endif

