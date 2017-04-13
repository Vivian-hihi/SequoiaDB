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

   Source File Name = coordUtil.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/13/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/


#include "coordUtil.hpp"
#include "msgDef.h"
#include "pmdEDU.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

using namespace bson ;
using namespace std ;

namespace engine
{

   void coordBuildFailedNodeReply( coordResource *pResource,
                                   ROUTE_RC_MAP &failedNodes,
                                   BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;

      if ( failedNodes.size() > 0 )
      {
         CoordGroupInfoPtr groupInfo ;
         string strHostName ;
         string strServiceName ;
         string strNodeName ;
         string strGroupName ;
         MsgRouteID routeID ;
         BSONObj errObj ;
         BSONArrayBuilder arrayBD( builder.subarrayStart(
                                   FIELD_NAME_ERROR_NODES ) ) ;
         ROUTE_RC_MAP::iterator iter = failedNodes.begin() ;
         while ( iter != failedNodes.end() )
         {
            strHostName.clear() ;
            strServiceName.clear() ;
            strNodeName.clear() ;
            strGroupName.clear() ;

            routeID.value = iter->first ;
            rc = pResource->getGroupInfo( routeID.columns.groupID, groupInfo ) ;
            if ( rc )
            {
               PD_LOG( PDWARNING, "Failed to get group[%d] info, rc: %d",
                       routeID.columns.groupID, rc ) ;
            }
            else
            {
               strGroupName = groupInfo->groupName() ;

               routeID.columns.serviceID = MSG_ROUTE_LOCAL_SERVICE ;
               rc = groupInfo->getNodeInfo( routeID, strHostName,
                                            strServiceName ) ;
               if ( rc )
               {
                  PD_LOG( PDWARNING, "Failed to get node[%d] info failed, "
                          "rc: %d", routeID.columns.nodeID, rc ) ;
               }
               else
               {
                  strNodeName = strHostName + ":" + strServiceName ;
               }
            }

            try
            {
               BSONObjBuilder objBD( arrayBD.subobjStart() ) ;
               objBD.append( FIELD_NAME_NODE_NAME, strNodeName ) ;
               //objBD.append( FIELD_NAME_HOST, strHostName ) ;
               //objBD.append( FIELD_NAME_SERVICE_NAME, strServiceName ) ;
               objBD.append( FIELD_NAME_GROUPNAME, strGroupName ) ;
               //objBD.append( FIELD_NAME_GROUPID, routeID.columns.groupID ) ;
               //objBD.append( FIELD_NAME_NODEID, (INT32)routeID.columns.nodeID ) ;
               objBD.append( FIELD_NAME_RCFLAG, iter->second._rc ) ;
               objBD.append( FIELD_NAME_ERROR_IINFO, iter->second._obj ) ;
               objBD.done() ;
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDWARNING, "Build error object occur exception: %s",
                       e.what() ) ;
               /// then ignored this record
            }
            ++iter ;
         }

         arrayBD.done() ;
      }
   }

   BSONObj coordBuildErrorObj( coordResource *pResource,
                               INT32 &flag,
                               pmdEDUCB *cb,
                               ROUTE_RC_MAP *pFailedNodes )
   {
      BSONObjBuilder builder ;
      const CHAR *pDetail = "" ;

      if ( SDB_OK == flag && pFailedNodes && pFailedNodes->size() > 0 )
      {
         flag = SDB_COORD_NOT_ALL_DONE ;
      }

      if ( cb && cb->getInfo( EDU_INFO_ERROR ) )
      {
         pDetail = cb->getInfo( EDU_INFO_ERROR ) ;
      }

      builder.append( OP_ERRNOFIELD, flag ) ;
      builder.append( OP_ERRDESP_FIELD, getErrDesp( flag ) ) ;
      builder.append( OP_ERR_DETAIL, pDetail ? pDetail : "" ) ;
      /// add ErrNodes
      if ( pFailedNodes && pFailedNodes->size() > 0 )
      {
         coordBuildFailedNodeReply( pResource, *pFailedNodes, builder ) ;
      }

      return builder.obj() ;
   }

}

