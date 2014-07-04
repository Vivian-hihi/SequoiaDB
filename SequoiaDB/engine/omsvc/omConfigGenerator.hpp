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

   Source File Name = omConfigGenerator.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/03/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OM_CONFIG_GENERATOR_HPP__
#define OM_CONFIG_GENERATOR_HPP__

#include "core.hpp"
#include "oss.hpp"
#include <map>
#include "../bson/bson.h"

using namespace bson;

namespace engine
{
   INT32 generateSDBConfig( const BSONObj &bsonTemplate, 
                            const BSONObj &bsonConfigItem, 
                            const BSONObj &bsonHostInfo, BSONObj &bsonConfig ) ;
}

#endif  /*OM_CONFIG_GENERATOR_HPP__*/ 

