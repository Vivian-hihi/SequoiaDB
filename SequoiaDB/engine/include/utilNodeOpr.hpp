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
      INT32    _role ;
      OSSPID   _pid ;

      _utilNodeInfo()
      {
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
                           OSSPID pidFilter = OSS_INVALID_PID,
                           INT32 roleFilter = -1 ) ;

   #define UTIL_WAIT_NODE_TIMEOUT         ( 15 * 60 ) // second

   /*
      enum nodes
   */
   INT32    utilEnumNodes( const string &localPath,
                           UTIL_VEC_NODES &nodes,
                           INT32 typeFilter = -1,
                           const CHAR *svcnameFilter = NULL,
                           INT32 roleFilter = -1 ) ;

   /*
      wait node bussiness ok
   */
   INT32    utilWaitNodeOK( utilNodeInfo &node,
                            const CHAR *svcname,
                            OSSPID pid = OSS_INVALID_PID,
                            INT32 typeFilter = -1,
                            INT32 timeout = UTIL_WAIT_NODE_TIMEOUT ) ;

   #define UTIL_STOP_NODE_TIMEOUT         ( 5 * 60 ) // second

   /*
      stop node
   */
   INT32    utilStopNode ( utilNodeInfo &node,
                           INT32 timeout = UTIL_STOP_NODE_TIMEOUT ) ;

   struct _utilNodeVerInfo
   {
      INT32    _version ;
      INT32    _subVersion ;
      INT32    _release ;
      string   _buildStr ;
   } ;
   typedef _utilNodeVerInfo utilNodeVerInfo ;

   /*
      get node version info
   */
   INT32    utilGetNodeVerInfo( const CHAR* pCommand,
                                utilNodeVerInfo &verInfo ) ;

}

#endif // UTIL_NODE_OPR_HPP__

