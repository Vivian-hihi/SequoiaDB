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

   Source File Name = ixmGeoDef.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Index Manager component. This file contains functions for index
   key generator, which is used to create key pairs from data record and index
   definition.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef IXMGEODEF_HPP_
#define IXMGEODEF_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   #define IXM_GEO_MIN_X -180
   #define IXM_GEO_MIN_Y -180
   #define IXM_GEO_MAX_X 180
   #define IXM_GEO_MAX_Y 180

   #define IXM_GEO_DEFAULT_CUT 32

   class _ixm2dTuple : public SDBObject
   {
   public :
      FLOAT64 x ;
      FLOAT64 y ;
      _ixm2dTuple():x(0),y(0){}
      _ixm2dTuple( const FLOAT64 &a, const FLOAT64 &b ):
      x(a), y(b){}
   } ;
   typedef class _ixm2dTuple geoTuple ;

}

#endif

