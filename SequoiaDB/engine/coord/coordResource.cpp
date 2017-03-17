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

   Source File Name = coordResource.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/14/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "coordResource.hpp"
#include "pmdEDU.hpp"
#include "msgCatalog.hpp"
#include "msgMessageFormat.hpp"
#include "msgMessage.hpp"
#include "coordRemoteHandle.hpp"
#include "coordRemoteSession.hpp"
#include "../bson/bson.h"
#include "utilArray.hpp"

using namespace bson ;

namespace engine
{

   #define COORD_SOCKET_OPR_DFT_TIME         ( 5000 )
   #define COORD_SOCKET_FORCE_TIMEOUT        ( 600000 )

   typedef _utilArray< UINT64, CLS_REPLSET_MAX_NODE_SIZE >     NODE_ARRAY ;

   /*
      _coordResource implement
   */
   _coordResource::_coordResource()
   {
      _upGrpIndentify = 1 ;
      _pAgent = NULL ;
   }

   _coordResource::~_coordResource()
   {
   }

   INT32 _coordResource::init( _netRouteAgent *pAgent )
   {
      INT32 rc = SDB_OK ;
      /// TODO:XUJIANHUI
   done:
      return rc ;
   error:
      goto done ;
   }

   void _coordResource::setCataGroupInfo( CoordGroupInfoPtr &groupPtr )
   {
      /// TODO:XUJIANHUI
   }

   INT32 _coordResource::getGroupInfo( UINT32 groupID,
                                       CoordGroupInfoPtr &groupPtr )
   {
      INT32 rc = SDB_OK ;
      MAP_GROUP_INFO_IT it  ;
      ossScopedLock lock( &_nodeMutex, SHARED ) ;

      it = _mapGroupInfo.find( groupID ) ;
      if ( it != _mapGroupInfo.end() )
      {
         groupPtr = it->second ;
      }
      else
      {
         rc = SDB_COOR_NO_NODEGROUP_INFO ;
      }

      return rc ;
   }

   INT32 _coordResource::getGroupInfo( const CHAR *groupName,
                                       CoordGroupInfoPtr &groupPtr )
   {
      INT32 rc = SDB_COOR_NO_NODEGROUP_INFO ;
      MAP_GROUP_INFO_IT it  ;
      MAP_GROUP_NAME_IT itName ;

      ossScopedLock lock( &_nodeMutex, SHARED ) ;

      itName = _mapGroupName.find( groupName ) ;
      if ( itName != _mapGroupName.end() )
      {
         it = _mapGroupInfo.find( itName->second ) ;
         if ( it != _mapGroupInfo.end() )
         {
            groupPtr = it->second ;
            rc = SDB_OK ;
         }
      }

      return rc ;
   }

   INT32 _coordResource::updateGroupInfo( UINT32 groupID,
                                          CoordGroupInfoPtr &groupPtr,
                                          _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      if ( CATALOG_GROUPID == rc )
      {
         rc = updateCataGroupInfo( groupPtr, cb ) ;
      }
      else
      {
         MsgCatGroupReq msgGroupReq ;
         /// init message
         msgGroupReq.id.columns.groupID = groupID ;
         msgGroupReq.id.columns.nodeID = 0 ;
         msgGroupReq.id.columns.serviceID = 0;
         msgGroupReq.header.messageLength = sizeof( MsgCatGroupReq ) ;
         msgGroupReq.header.opCode = MSG_CAT_GRP_REQ ;
         msgGroupReq.header.routeID.value = 0 ;

         rc = _updateGroupInfo( ( MsgHeader* )&msgGroupReq, cb,
                                MSG_ROUTE_CAT_SERVICE, groupPtr ) ;
         if ( SDB_CLS_GRP_NOT_EXIST == rc )
         {
            removeGroupInfo( groupID ) ;
         }
      }
      if ( rc )
      {
         PD_LOG( PDERROR, "Update group[%u] info failed, rc: %d",
                 groupID, rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordResource::updateGroupInfo( const CHAR *groupName,
                                          CoordGroupInfoPtr &groupPtr,
                                          _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      CHAR *pBuf = NULL ;

      if ( 0 == ossStrcmp( groupName, CATALOG_GROUPNAME ) )
      {
         rc = updateCataGroupInfo( groupPtr, cb ) ;
      }
      else
      {
         UINT32 nameLen = ossStrlen( groupName ) + 1 ;
         UINT32 msgLen = nameLen +  sizeof(MsgCatGroupReq) ;
         MsgCatGroupReq *msg = NULL ;

         rc = cb->allocBuff( msgLen, &pBuf ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Alloc memory[Size:%u] failed, rc: %d",
                    msgLen, rc ) ;
            goto error ;
         }
         msg = ( MsgCatGroupReq * )pBuf ;
         msg->id.value = 0 ;
         msg->header.messageLength = msgLen ;
         msg->header.opCode = MSG_CAT_GRP_REQ ;
         msg->header.routeID.value = 0 ;
         ossMemcpy( pBuf + sizeof(MsgCatGroupReq),
                    groupName, nameLen ) ;

         rc = _updateGroupInfo( ( MsgHeader*)msg, cb,
                                MSG_ROUTE_CAT_SERVICE, groupPtr ) ;
         if ( SDB_CLS_GRP_NOT_EXIST == rc )
         {
            removeGroupInfo( groupName ) ;
         }
      }
      if ( rc )
      {
         PD_LOG( PDERROR, "Update group[%s] info failed, rc: %d",
                 groupName, rc ) ;
         goto error ;
      }

   done:
      if ( pBuf )
      {
         cb->releaseBuff( pBuf ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   void _coordResource::removeGroupInfo( UINT32 groupID )
   {
      ossScopedLock lock( &_nodeMutex, EXCLUSIVE ) ;

      _mapGroupInfo.erase( groupID ) ;
      _clearGroupName( groupID ) ;
   }

   void _coordResource::removeGroupInfo( const CHAR *groupName )
   {
      MAP_GROUP_NAME_IT itName ;
      ossScopedLock lock( &_nodeMutex, EXCLUSIVE ) ;

      itName = _mapGroupName.find( groupName ) ;
      if ( itName != _mapGroupName.end() )
      {
         _mapGroupInfo.erase( itName->second ) ;
         _mapGroupName.erase( itName ) ;
      }
   }

   void _coordResource::addGroupInfo( CoordGroupInfoPtr &groupPtr )
   {
      ossScopedLock _lock( &_nodeMutex, EXCLUSIVE ) ;

      groupPtr->setIdentify( ++_upGrpIndentify ) ;
      _mapGroupInfo[groupPtr->groupID()] = groupPtr ;

      // clear group name map
      _clearGroupName( groupPtr->groupID() ) ;

      // add to group name map
      _addGroupName( groupPtr->groupName(), groupPtr->groupID() ) ;
   }

   CoordGroupInfoPtr _coordResource::getCataGroupInfo()
   {
      CoordGroupInfoPtr tmpPtr ;

      ossScopedLock lock( &_nodeMutex, SHARED ) ;
      tmpPtr = _cataGroupInfo ;

      return tmpPtr ;
   }

   INT32 _coordResource::groupID2Name( UINT32 id, std::string &name )
   {
      INT32 rc = SDB_OK ;
      MAP_GROUP_INFO_IT it ;

      ossScopedLock _lock( &_nodeMutex, SHARED ) ;

      it = _mapGroupInfo.find( id ) ;
      if ( it == _mapGroupInfo.end() )
      {
         rc = SDB_COOR_NO_NODEGROUP_INFO ;
      }
      else
      {
         name = it->second->groupName() ;
      }

      return rc ;
   }

   INT32 _coordResource::groupName2ID( const CHAR *name, UINT32 &id )
   {
      INT32 rc = SDB_OK ;
      MAP_GROUP_NAME_IT itName ;

      ossScopedLock _lock( &_nodeMutex, SHARED ) ;

      itName = _mapGroupName.find( name ) ;
      if ( itName == _mapGroupName.end() )
      {
         rc = SDB_COOR_NO_NODEGROUP_INFO ;
      }
      else
      {
         id = itName->second ;
      }

      return rc ;
   }

   void _coordResource::_clearGroupName( UINT32 groupID )
   {
      MAP_GROUP_NAME_IT itName = _mapGroupName.begin() ;
      while( itName != _mapGroupName.end() )
      {
         if ( itName->second == groupID )
         {
            _mapGroupName.erase( itName ) ;
            break ;
         }
         ++itName ;
      }
   }

   void _coordResource::_addGroupName( const std::string &name, UINT32 id )
   {
      _mapGroupName[name] = id ;
   }

   void _coordResource::getCataNodeAddrList( CoordVecNodeInfo &vecCata )
   {
      ossScopedLock lock( &_nodeMutex, SHARED ) ;
      vecCata = _cataNodeAddrList ;
   }

   INT32 _coordResource::_updateCataGroupInfoByAddr( _pmdEDUCB *cb,
                                                     CoordGroupInfoPtr &groupPtr )
   {
      INT32 rc = SDB_OK ;
      MsgCatCatGroupReq msgGroupReq ;
      CoordVecNodeInfo cataNodeAddrList ;
      UINT32 sendPos = 0 ;
      UINT16 port = 0 ;
      MsgHeader *pReply = NULL ;

      getCataNodeAddrList( cataNodeAddrList ) ;
      /// init message
      msgGroupReq.id.columns.groupID = CAT_CATALOG_GROUPID ;
      msgGroupReq.id.columns.nodeID = 0 ;
      msgGroupReq.id.columns.serviceID = 0;
      msgGroupReq.header.messageLength = sizeof( MsgCatGroupReq );
      msgGroupReq.header.opCode = MSG_CAT_GRP_REQ ;
      msgGroupReq.header.routeID.value = 0 ;
      msgGroupReq.header.TID = cb->getTID() ;
      msgGroupReq.header.requestID = cb->incCurRequestID() ;

      if ( cataNodeAddrList.empty() )
      {
         rc = SDB_CAT_NO_ADDR_LIST ;
         goto error ;
      }

      while( sendPos < cataNodeAddrList.size() )
      {
         clsNodeItem &item = cataNodeAddrList[ sendPos++ ] ;
         rc = ossGetPort( item._service[ MSG_ROUTE_CAT_SERVICE ].c_str(),
                          port ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Convert service[%s:%s] to port failed, rc: %d",
                    item._host, item._service[ MSG_ROUTE_CAT_SERVICE ].c_str(),
                    rc ) ;
            goto error ;
         }
         else
         {
            ossSocket sock( item._host, port ) ;
            rc = sock.initSocket() ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Init socket[%s:%u] failed, rc: %d",
                       item._host, port, rc ) ;
               continue ;
            }
            rc = sock.connect( COORD_SOCKET_OPR_DFT_TIME ) ;
            if ( rc )
            {
               PD_LOG( PDWARNING, "Connect to %s:%u failed, rc: %d",
                       item._host, port, rc ) ;
               continue ;
            }

            /// send and recv message
            rc = pmdSyncSendMsg( ( const MsgHeader* )&msgGroupReq, &pReply,
                                 &sock, cb, TRUE, OSS_SOCKET_DFT_TIMEOUT,
                                 COORD_SOCKET_FORCE_TIMEOUT ) ;
            if ( rc )
            {
               if ( SDB_APP_FORCED == rc )
               {
                  goto error ;
               }
               PD_LOG( PDWARNING, "Sync send message to %s:%u failed, rc: %d",
                       item._host, port, rc ) ;
               continue ;
            }

            /// process the result
            if ( pReply->opCode != MSG_CAT_GRP_RES )
            {
               PD_LOG( PDERROR, "Recieve unexpect response[%s]",
                       msg2String( pReply ).c_str() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            rc = _processGroupReply( pReply, groupPtr ) ;
            /// release reply message
            cb->releaseBuff( ( CHAR* )pReply ) ;
            pReply = NULL ;
            if ( rc )
            {
               PD_LOG( PDWARNING, "Failed to process catalog group reply, "
                       "rc: %d", rc ) ;
               continue ;
            }
            else
            {
               break ;
            }
         }
      }

   done:
      if ( pReply )
      {
         cb->releaseBuff( ( CHAR* )pReply ) ;
         pReply = NULL ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordResource::_processGroupReply( MsgHeader *pMsg,
                                             CoordGroupInfoPtr &groupPtr )
   {
      INT32 rc = SDB_OK ;
      CoordGroupInfo *pGroupInfo = NULL ;
      UINT32 headerLen = MSG_GET_INNER_REPLY_HEADER_LEN( pMsg ) ;

      if ( SDB_OK == MSG_GET_INNER_REPLY_RC( pMsg ) &&
           (UINT32)pMsg->messageLength >= headerLen + 5 )
      {
         try
         {
            BSONObj boGroupInfo( MSG_GET_INNER_REPLY_DATA( pMsg ) ) ;
            BSONElement beGroupID = boGroupInfo.getField( CAT_GROUPID_NAME ) ;
            if ( beGroupID.eoo() || !beGroupID.isNumber() )
            {
               rc = SDB_SYS ;
               PD_LOG ( PDERROR, "Reply object[%s] is invalid",
                        boGroupInfo.toString().c_str() ) ;
               goto error ;
            }
            pGroupInfo = SDB_OSS_NEW CoordGroupInfo( beGroupID.number() ) ;
            if ( NULL == pGroupInfo )
            {
               rc = SDB_OOM ;
               PD_LOG ( PDERROR, "Alloc group info failed" ) ;
               goto error ;
            }

            rc = pGroupInfo->updateGroupItem( boGroupInfo ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Update group info from bson[%s] failed, "
                       "rc: %d", boGroupInfo.toString().c_str(), rc ) ;
               goto error ;
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDERROR, "Occur exception: %s", e.what() ) ;
            goto error ;
         }

         groupPtr = CoordGroupInfoPtr( pGroupInfo ) ;
         pGroupInfo = NULL ;
      }
      else
      {
         rc = MSG_GET_INNER_REPLY_RC( pMsg ) ;
      }

   done:
      if ( pGroupInfo )
      {
         SDB_OSS_DEL pGroupInfo ;
         pGroupInfo = NULL ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordResource::_updateCataGroupInfo( _pmdEDUCB *cb,
                                               const CoordGroupInfoPtr &cataGroupPtr,
                                               CoordGroupInfoPtr &groupPtr )
   {
      INT32 rc = SDB_OK ;
      MsgCatCatGroupReq msgGroupReq ;
      pmdRemoteSessionSite *pSite = NULL ;
      pmdRemoteSession *pRSession = NULL ;
      pmdSubSession *pSubSession = NULL ;
      coordRemoteHandleBase baseHandle ;

      clsGroupItem *groupItem = NULL ;
      NODE_ARRAY nodes ;
      NODE_ARRAY faultNodes ;
      UINT32 normalNodeCount = 0 ;
      UINT32 beginPos = 0 ;
      UINT32 sendTimes = 0 ;
      INT32  status = 0 ;
      MsgRouteID nodeID ;

      /// init message
      msgGroupReq.id.columns.groupID = CAT_CATALOG_GROUPID ;
      msgGroupReq.id.columns.nodeID = 0 ;
      msgGroupReq.id.columns.serviceID = 0;
      msgGroupReq.header.messageLength = sizeof( MsgCatGroupReq ) ;
      msgGroupReq.header.opCode = MSG_CAT_GRP_REQ ;
      msgGroupReq.header.routeID.value = 0 ;

      groupItem = groupPtr.get() ;
      SDB_ASSERT( groupItem && groupItem->nodeCount() > 0,
                  "Group item's node count must grater than zero" ) ;

      /// prepare nodes
      beginPos = ossRand() % groupItem->nodeCount() ;
      while( sendTimes < groupItem->nodeCount() )
      {
         rc = groupItem->getNodeID( beginPos, nodeID, MSG_ROUTE_CAT_SERVICE ) ;
         if ( rc )
         {
            goto error ;
         }
         rc = groupItem->getNodeInfo( beginPos, status ) ;
         if ( rc )
         {
            goto error ;
         }
         if ( NET_NODE_STAT_NORMAL == status )
         {
            nodes.append( nodeID.value ) ;
         }
         else
         {
            faultNodes.append( nodeID.value ) ;
         }
         beginPos = ( beginPos + 1 ) % groupItem->nodeCount() ;
         ++sendTimes ;
      }
      normalNodeCount = nodes.size() ;

      /// push fault nodes to nodes
      if ( !faultNodes.empty() )
      {
         NODE_ARRAY::iterator it( faultNodes ) ;
         UINT64 tmpID = 0 ;
         while( it.more() )
         {
            it.next( tmpID ) ;
            nodes.append( tmpID ) ;
         }
      }

      pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;
      if ( !pSite )
      {
         PD_LOG( PDERROR, "Session[%s] is not registered for remote session",
                 cb->toString().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      pRSession = pSite->addSession( -1, &baseHandle ) ;

      /// send message to node
      for ( UINT32 i = 0 ; i < nodes.size() ; ++i )
      {
         nodeID.value = nodes[ i ] ;
         pRSession->clearSubSession() ;
         /// send message
         pSubSession = pRSession->addSubSession( nodeID.value ) ;
         pSubSession->setReqMsg( ( MsgHeader* )&msgGroupReq,
                                 PMD_EDU_MEM_NONE ) ;
         rc = pRSession->sendMsg( pSubSession ) ;
         if ( rc )
         {
            PD_LOG( PDWARNING, "Send message to node[%s] failed, rc: %d",
                    routeID2String( nodeID ).c_str(), rc ) ;
            /// when the node is normal, but send message failed
            if ( i < normalNodeCount )
            {
               groupItem->updateNodeStat( nodeID.columns.nodeID,
                                          netResult2Status( rc ) ) ;
            }
            continue ;
         }
         /// when the node is fault, but send message succeed
         if ( i >= normalNodeCount )
         {
            groupItem->updateNodeStat( nodeID.columns.nodeID,
                                       NET_NODE_STAT_NORMAL ) ;
         }

         /// recv reply
         rc = pRSession->waitReply( TRUE ) ;
         if ( rc )
         {
            if ( SDB_APP_INTERRUPT != rc )
            {
               PD_LOG( PDERROR, "Wait reply from node[%s] failed, rc: %d",
                       routeID2String( nodeID ).c_str(), rc ) ;
            }
            goto error ;
         }

         /// process reply
         rc = _processGroupReply( pSubSession->getRspMsg(), groupPtr ) ;
         if ( rc )
         {
            PD_LOG( PDWARNING, "Failed to process catalog group reply, "
                    "rc: %d", rc ) ;
            if ( SDB_CLS_FULL_SYNC == rc || SDB_RTN_IN_REBUILD == rc )
            {
               groupItem->updateNodeStat( nodeID.columns.nodeID,
                                          netResult2Status( rc ) ) ;
            }
            continue ;
         }
         else
         {
            break ;
         }
      }

   done:
      if ( pRSession )
      {
         pSite->removeSession( pRSession ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordResource::updateCataGroupInfo( CoordGroupInfoPtr &groupPtr,
                                              _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      CoordGroupInfoPtr cataGroupInfo ;

      cataGroupInfo = getCataGroupInfo() ;

      if ( 0 == cataGroupInfo->nodeCount() )
      {
         rc = _updateCataGroupInfoByAddr( cb, groupPtr ) ;
      }
      else
      {
         rc = _updateCataGroupInfo( cb, cataGroupInfo, groupPtr ) ;
      }
      if ( rc )
      {
         goto error ;
      }

      /// update route info
      rc = _updateRouteInfo( groupPtr, MSG_ROUTE_CAT_SERVICE ) ;
      PD_RC_CHECK( rc, PDERROR, "Update catalog group's cata serivce route "
                   "info failed, rc: %d", rc ) ;
      rc = _updateRouteInfo( groupPtr, MSG_ROUTE_SHARD_SERVCIE ) ;
      PD_RC_CHECK( rc, PDERROR, "Update catalog group's shard serivce route "
                   "info failed, rc: %d", rc ) ;

      /// add to group info
      addGroupInfo( groupPtr ) ;
      setCataGroupInfo( groupPtr ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordResource::_updateRouteInfo( const CoordGroupInfoPtr &groupPtr,
                                           MSG_ROUTE_SERVICE_TYPE type )
   {
      INT32 rc = SDB_OK ;

      string host ;
      string service ;
      MsgRouteID routeID ;
      routeID.value = MSG_INVALID_ROUTEID ;

      UINT32 index = 0 ;
      clsGroupItem *groupItem = groupPtr.get() ;
      while ( SDB_OK == groupItem->getNodeInfo( index++, routeID, host,
                                                service, type ) )
      {
         rc = _pAgent->updateRoute( routeID, host.c_str(),
                                    service.c_str() ) ;
         if ( rc != SDB_OK )
         {
            if ( SDB_NET_UPDATE_EXISTING_NODE == rc )
            {
               rc = SDB_OK ;
            }
            else
            {
               PD_LOG ( PDERROR, "Update route[%s] failed, rc: %d",
                        routeID2String( routeID ).c_str(), rc ) ;
               break ;
            }
         }
      }

      return rc ;
   }

   INT32 _coordResource::_updateGroupInfo( MsgHeader *pMsg,
                                           _pmdEDUCB *cb,
                                           MSG_ROUTE_SERVICE_TYPE type,
                                           CoordGroupInfoPtr &groupPtr )
   {
      INT32 rc = SDB_OK ;
      coordGroupSession session ;
      pmdSubSession *pSub = NULL ;
      MsgHeader *pReply = NULL ;

      rc = session.init( this, cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init coord remote session failed, rc: %d", rc ) ;
         goto error ;
      }
      session.getGroupSel()->setPrimary( TRUE ) ;
      session.getGroupSel()->setServiceType( type ) ;

   retry:
      session.getSession()->clearSubSession() ;
      rc = session.sendMsg( pMsg, CATALOG_GROUPID, NULL, &pSub ) ;
      if ( rc )
      {
         goto error ;
      }

      /// recv reply
      rc = session.getSession()->waitReply1( TRUE ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Wait reply from catalog group failed, rc: %d",
                 rc ) ;
         goto error ;
      }

      /// process reply
      pReply = pSub->getRspMsg() ;
      rc = _processGroupReply( pReply, groupPtr ) ;
      if ( rc )
      {
         coordGroupSessionCtrl *pGroupCtrl = session.getGroupCtrl() ;
         UINT32 primaryID = MSG_GET_INNER_REPLY_STARTFROM( pReply ) ;

         if ( pGroupCtrl->canRetry( rc, pReply->routeID,
                                    primaryID, TRUE, TRUE ) )
         {
            pGroupCtrl->incRetry() ;
            goto retry ;
         }
      }
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to process group reply, rc: %d", rc ) ;
         goto error ;
      }

      /// update route info
      rc = _updateRouteInfo( groupPtr, MSG_ROUTE_SHARD_SERVCIE ) ;
      PD_RC_CHECK( rc, PDERROR, "Update group[%u]'s shard serivce route "
                   "info failed, rc: %d", groupPtr->groupID(), rc ) ;

      /// add to group info
      addGroupInfo( groupPtr ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

}

