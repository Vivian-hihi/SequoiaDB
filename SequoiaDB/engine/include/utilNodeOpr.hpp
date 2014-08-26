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

   Source File Name = utilNodeOpr.hpp

   Descriptive Name =

   When/how to use: node operation

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/08/2014  XJH Initial Draft

   Last Changed =

******************************************************************************/

#ifndef UTIL_NODE_OPR_HPP__
#define UTIL_NODE_OPR_HPP__

#include "core.hpp"
#include <string>
#include <vector>

using namespace std ;

namespace engine
{

   /*
      _utilNodeInfo define
   */
   struct _utilNodeInfo
   {
      string   _orgname ;
      string   _svcname ;
      INT32    _type ;
      OSSPID   _pid ;

      _utilNodeInfo()
      {
         _type = -1 ;
         _pid  = OSS_INVALID_PID ;
      }
   } ;
   typedef _utilNodeInfo utilNodeInfo ;

   typedef vector< utilNodeInfo >   UTIL_VEC_NODES ;

   /*
      list nodes
   */
   INT32    utilListNodes( UTIL_VEC_NODES &nodes,
                           INT32 typeFilter = -1,
                           const CHAR *svcnameFilter = NULL,
                           OSSPID pidFilter = OSS_INVALID_PID ) ;

}

#endif // UTIL_NODE_OPR_HPP__

