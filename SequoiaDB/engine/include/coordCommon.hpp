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

   Source File Name = coordCommon.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/05/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef COORD_COMMON_HPP__
#define COORD_COMMON_HPP__

#include "coordDef.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{

   INT32    coordInitCataPtrFromObj( const BSONObj &obj,
                                     CoordCataInfoPtr &cataPtr ) ;

   BOOLEAN  coordIsCataAddrSame( const CoordVecNodeInfo &left,
                                 const CoordVecNodeInfo &right ) ;

   BOOLEAN  coordCataCheckFlag( INT32 flag ) ;

}

#endif // COORD_COMMON_HPP__
