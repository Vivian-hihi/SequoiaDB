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

   Source File Name = sptParamContainer.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_PARAMCONTAINER_HPP_
#define SPT_PARAMCONTAINER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.hpp"

namespace engine
{
   class _sptParamContainer : public SDBObject
   {
   public:
      _sptParamContainer(){}
      virtual ~_sptParamContainer(){}

   public:
      virtual INT32 getINT32( UINT32 skip, BOOLEAN optional, INT32 &value ) = 0 ;
      virtual INT32 getFLOAT64( UINT32 skip, BOOLEAN optional, FLOAT64 &value ) = 0 ;
      virtual INT32 getString( UINT32 skip, BOOLEAN optional, std::string &value ) = 0 ;
      virtual INT32 getBSONObj( UINT32 skip, BOOLEAN optional, bson::BSONObj &value ) = 0 ;
      virtual INT32 getBOOLEAN( UINT32 skip, BOOLEAN optional, BOOLEAN &value ) = 0 ;
   } ;
   typedef class _sptParamContainer sptParamContainer ;
}

#endif

