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

   Source File Name = rtnCoordDCCommands.cpp

   Descriptive Name = Runtime Coord Common

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          02/11/15    XJH Init
   Last Changed =

*******************************************************************************/

#include "rtnCoordDCCommands.hpp"
#include "pmdCB.hpp"

using namespace bson ;

namespace engine
{

   /*
      rtnCoordImageBase implement
   */
   INT32 rtnCoordImageBase::execute( CHAR *pReceiveBuffer, SINT32 packSize,
                                       CHAR **ppResultBuffer, pmdEDUCB *cb,
                                       MsgOpReply &replyHeader,
                                       BSONObj **ppErrorObj )
   {
      INT32 rc = SDB_OK ;
      CoordCB *pCoord = pmdGetKRCB()->getCoordCB() ;
      netMultiRouteAgent *pRouteAgent = pCoord->getRouteAgent() ;
      CoordGroupList datagroups ;
      CoordGroupList sendgroups ;
      CoordGroupList allgroups ;

      // fill default-reply
      MsgHeader *pHeader               = (MsgHeader *)pReceiveBuffer;
      replyHeader.header.messageLength = sizeof( MsgOpReply );
      replyHeader.header.opCode        = MSG_BS_QUERY_RES;
      replyHeader.header.requestID     = pHeader->requestID;
      replyHeader.header.routeID.value = 0;
      replyHeader.header.TID           = pHeader->TID;
      replyHeader.contextID            = -1;
      replyHeader.flags                = SDB_OK;
      replyHeader.numReturned          = 0;
      replyHeader.startFrom            = 0;

      MsgOpQuery *pAttachMsg           = (MsgOpQuery *)pReceiveBuffer ;
      pAttachMsg->header.routeID.value = 0 ;
      pAttachMsg->header.TID           = cb->getTID() ;
      pAttachMsg->header.opCode        = _getInnerOpCode() ;

      // 1. execute on catalog
      rc = executeOnCataGroup( pReceiveBuffer, pRouteAgent, cb,
                               NULL, &datagroups ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to execute %s on catalog node, rc: %d",
                 _getName(), rc ) ;
         goto error ;
      }

      // update all groups
      rc = rtnCoordGetAllGroupList( cb, allgroups, NULL, FALSE, TRUE ) ;
      if ( rc )
      {
         PD_LOG( PDWARNING, "Failed to update all group list, rc: %d", rc ) ;
         rc = SDB_OK ;
      }

      // 2. execute on the special groups, ignore error
      pAttachMsg->header.opCode        = MSG_BS_QUERY_REQ ;
      rc = executeOnDataGroup( &pAttachMsg->header, datagroups, sendgroups,
                               pRouteAgent, cb, TRUE ) ;
      if ( rc )
      {
         PD_LOG( PDWARNING, "Failed to execute %s on data nodes, "
                 "rc: %d", _getName(), rc ) ;
         rc = SDB_OK ;
      }

   done:
      replyHeader.flags = rc ;
      return rc ;
   error:
      goto done ;
   }

   /*
      rtnCoordAttachImage implement
   */
   INT32 rtnCoordAttachImage::_getInnerOpCode() const
   {
      return MSG_CAT_ATTACH_IMAGE_REQ ;
   }

   const CHAR* rtnCoordAttachImage::_getName() const
   {
      return COORD_CMD_ATTACH_IMAGE ;
   }

   /*
      rtnCoordEnableImage implement
   */
   INT32 rtnCoordEnableImage::_getInnerOpCode() const
   {
      return MSG_CAT_ENABLE_IMAGE_REQ ;
   }

   const CHAR* rtnCoordEnableImage::_getName() const
   {
      return COORD_CMD_ENABLE_IMAGE ;
   }

}

