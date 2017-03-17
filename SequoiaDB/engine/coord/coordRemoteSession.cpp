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

   Source File Name = coordRemoteSession.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/03/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "coordRemoteSession.hpp"
#include "msgMessageFormat.hpp"
#include "pmdEDU.hpp"

namespace engine
{

   /*
      _coordSessionPropSite implement
   */
   _coordSessionPropSite::_coordSessionPropSite()
   {
      _pEDUCB           = NULL ;
      _preferInsType    = PREFER_REPL_ANYONE ;
      _oprTimeout       = -1 ;
   }

   _coordSessionPropSite::~_coordSessionPropSite()
   {
      SDB_ASSERT( NULL == _pEDUCB, "EDU must be NULL" ) ;
   }

   void _coordSessionPropSite::clear()
   {
      _mapLastNodes.clear() ;
   }

   void _coordSessionPropSite::addLastNode( UINT32 groupID, UINT64 nodeID )
   {
      _mapLastNodes[ groupID ] = nodeID ;
   }

   UINT64 _coordSessionPropSite::getLastNode( UINT32 groupID ) const
   {
      MAP_GROUP_2_NODE::const_iterator cit = _mapLastNodes.find( groupID ) ;
      if ( cit != _mapLastNodes.end() )
      {
         return cit->second ;
      }
      return MSG_INVALID_ROUTEID ;
   }

   BOOLEAN _coordSessionPropSite::existNode( UINT32 groupID ) const
   {
      MAP_GROUP_2_NODE::const_iterator cit = _mapLastNodes.find( groupID ) ;
      if ( cit != _mapLastNodes.end() )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   void _coordSessionPropSite::delLastNode( UINT32 groupID )
   {
      _mapLastNodes.erase( groupID ) ;
   }

   void _coordSessionPropSite::delLastNode( UINT32 groupID, UINT64 nodeID )
   {
      MAP_GROUP_2_NODE_IT it = _mapLastNodes.find( groupID ) ;
      if ( it != _mapLastNodes.end() && it->second == nodeID )
      {
         _mapLastNodes.erase( it ) ;
      }
   }

   void _coordSessionPropSite::setPreferInsType( INT32 preferType )
   {
      _preferInsType = preferType ;
   }

   INT32 _coordSessionPropSite::getPreferInstype() const
   {
      return _preferInsType ;
   }

   void _coordSessionPropSite::setOprTimeout( INT64 oprTimeout )
   {
      _oprTimeout = oprTimeout ;
   }

   INT64 _coordSessionPropSite::getOprTimeout() const
   {
      return _oprTimeout ;
   }

   void _coordSessionPropSite::setEduCB( _pmdEDUCB *cb )
   {
      _pEDUCB = cb ;
   }

   /*
      _coordSessionPropMgr implement
   */
   _coordSessionPropMgr::_coordSessionPropMgr()
   {
      _preferInsType = PREFER_REPL_ANYONE ;
      _oprTimeout    = -1 ;
   }

   _coordSessionPropMgr::~_coordSessionPropMgr()
   {
      SDB_ASSERT( 0 == _mapProps.size(), "Props must be empty" ) ;
   }

   void _coordSessionPropMgr::setPreferInsType( INT32 preferInsType )
   {
      _preferInsType = preferInsType ;
   }

   void _coordSessionPropMgr::setOprTimeout( INT64 oprTimeout )
   {
      _oprTimeout = oprTimeout ;
   }

   void _coordSessionPropMgr::onRegister( _pmdRemoteSessionSite *pSite,
                                          _pmdEDUCB *cb )
   {
      coordSessionPropSite &propSite = _mapProps[ cb->getTID() ] ;
      propSite.setPreferInsType( _preferInsType ) ;
      propSite.setOprTimeout( _oprTimeout ) ;
      propSite.setEduCB( cb ) ;
      pSite->setUserData( (UINT64)&propSite ) ;
   }

   void _coordSessionPropMgr::onUnreg( _pmdRemoteSessionSite *pSite,
                                       _pmdEDUCB *cb )
   {
      coordSessionPropSite *pPropSite = NULL ;
      pPropSite = ( coordSessionPropSite* )pSite->getUserData() ;
      if ( pPropSite )
      {
         pPropSite->setEduCB( NULL ) ;
         pSite->setUserData( 0 ) ;
      }
      _mapProps.erase( cb->getTID() ) ;
   }

   coordSessionPropSite* _coordSessionPropMgr::getSite( _pmdEDUCB *cb )
   {
      _pmdRemoteSessionSite *pSite = NULL ;
      coordSessionPropSite *propSite = NULL ;

      pSite = ( _pmdRemoteSessionSite* )cb->getRemoteSite() ;
      if ( pSite )
      {
         propSite = ( coordSessionPropSite* )pSite->getUserData() ;
      }

      return propSite ;
   }

   /*
      _coordGroupSel implement
   */
   _coordGroupSel::_coordGroupSel()
   {
      _pResource     = NULL ;
      _pPropSite     = NULL ;
      _primary       = FALSE ;
      _svcType       = MSG_ROUTE_SHARD_SERVCIE ;
      _hasUpdate     = FALSE ;
      _pos           = -1 ;
      _selTimes      = 0 ;
      _ignoredNum    = 0 ;
      _lastNodeID.value = MSG_INVALID_ROUTEID ;
   }

   _coordGroupSel::~_coordGroupSel()
   {
   }

   void _coordGroupSel::init( coordResource *pResource,
                              coordSessionPropSite *pPropSite,
                              BOOLEAN primary,
                              MSG_ROUTE_SERVICE_TYPE svcType )
   {
      _pResource        = pResource ;
      _pPropSite        = pPropSite ;
      _primary          = primary ;
      _svcType          = svcType ;
   }

   void _coordGroupSel::setPrimary( BOOLEAN primary )
   {
      _primary          = primary ;
   }

   void _coordGroupSel::setServiceType( MSG_ROUTE_SERVICE_TYPE svcType )
   {
      _svcType          = svcType ;
   }

   void _coordGroupSel::_resetStatus()
   {
      _pos = -1 ;
      _selTimes = 0 ;
      _ignoredNum = 0 ;
      _hasUpdate = FALSE ;
      _lastNodeID.value = MSG_INVALID_ROUTEID ;
   }

   INT32 _coordGroupSel::selBegin( UINT32 groupID, MsgRouteID &nodeID )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( -1 == _pos, "Last sel doesn't call selDone" ) ;

      _resetStatus() ;

      rc = _pResource->getGroupInfo( groupID, _groupPtr ) ;
      if ( rc && !_hasUpdate )
      {
         _hasUpdate = TRUE ;
         rc = _pResource->updateGroupInfo( groupID, _groupPtr,
                                           _pPropSite->getEDUCB() ) ;
      }
      if ( rc )
      {
         PD_LOG( PDERROR, "Get or update group[%u] info failed, rc: %d",
                 groupID, rc ) ;
         goto error ;
      }

      _pPropSite->getEDUCB()->getTransNodeRouteID( groupID, nodeID ) ;
      /// In transaction, need to use the trans node
      if ( MSG_INVALID_ROUTEID != nodeID.value &&
           _svcType == nodeID.columns.serviceID )
      {
         goto done ;
      }
      else if ( _primary )
      {
         rc = _selPrimaryBegin( nodeID ) ;
      }
      else
      {
         rc = _selOtherBegin( nodeID ) ;
      }
      if ( rc )
      {
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordGroupSel::_selPrimaryBegin( MsgRouteID &nodeID )
   {
      INT32 rc = SDB_OK ;

      nodeID = _groupPtr->primary( _svcType ) ;
      if ( MSG_INVALID_ROUTEID == nodeID.value && !_hasUpdate )
      {
         _hasUpdate = TRUE ;
         UINT32 groupID = _groupPtr->groupID() ;
         rc = _pResource->updateGroupInfo( groupID, _groupPtr,
                                           _pPropSite->getEDUCB() ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Update group[%u] info failed, rc: %d",
                    groupID, rc ) ;
            goto error ;
         }
         nodeID = _groupPtr->primary( _svcType ) ;
      }

      if ( MSG_INVALID_ROUTEID == nodeID.value )
      {
         /// when no primary node, select any one
         rc = _selOtherBegin( nodeID ) ;
         if ( rc )
         {
            goto error ;
         }
      }
      else
      {
         _lastNodeID.value = nodeID.value ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordGroupSel::_selOtherBegin( MsgRouteID &nodeID )
   {
      INT32 rc = SDB_OK ;
      UINT32 groupID = _groupPtr->groupID() ;
      clsGroupItem *groupItem = NULL ;
      UINT32 nodeNum = 0 ;

      nodeID.value = _pPropSite->getLastNode( groupID ) ;
      // last node is valid and in group info
      if ( MSG_INVALID_ROUTEID != nodeID.value )
      {
         if ( _groupPtr->nodePos( nodeID.columns.nodeID ) >= 0 )
         {
            nodeID.columns.serviceID = _svcType ;
            goto done ;
         }
         _pPropSite->delLastNode( groupID ) ;
      }

      groupItem = _groupPtr.get() ;
      nodeNum = groupItem->nodeCount() ;
      if ( nodeNum <= 0 && !_hasUpdate )
      {
         _hasUpdate = TRUE ;
         rc = _pResource->updateGroupInfo( groupID, _groupPtr,
                                           _pPropSite->getEDUCB() ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Update group[%u] info failed, rc: %d",
                    groupID, rc ) ;
            goto error ;
         }
         groupItem = _groupPtr.get() ;
         nodeNum = groupItem->nodeCount() ;
      }
      if ( nodeNum <= 0 )
      {
         rc = SDB_CLS_EMPTY_GROUP ;
         PD_LOG( PDERROR, "Group[%u] is empty", groupID ) ;
         goto error ;
      }

      _pos = _calcBeginPos( groupItem, _pPropSite->getPreferInstype(),
                            ossRand() ) ;
      rc = _nextPos( _groupPtr, _pPropSite->getPreferInstype(),
                     _selTimes, _pos, nodeID ) ;
      if ( rc )
      {
         goto error ;
      }
      else
      {
         _lastNodeID.value = nodeID.value ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordGroupSel::selNext( MsgRouteID &nodeID )
   {
      INT32 rc = SDB_OK ;

      if ( -1 == _pos )
      {
         rc = SDB_CLS_NODE_BSFAULT ;
         goto error ;
      }

      rc = _nextPos( _groupPtr, _pPropSite->getPreferInstype(),
                     _selTimes, _pos, nodeID ) ;
      if ( rc )
      {
         goto error ;
      }
      else
      {
         _lastNodeID.value = nodeID.value ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordGroupSel::_calcBeginPos( clsGroupItem *pGroupItem,
                                        INT32 preferInsType,
                                        UINT32 random )
   {
      UINT32 posTmp = 0 ;

      switch( preferInsType )
      {
         case PREFER_REPL_NODE_1:
         case PREFER_REPL_NODE_2:
         case PREFER_REPL_NODE_3:
         case PREFER_REPL_NODE_4:
         case PREFER_REPL_NODE_5:
         case PREFER_REPL_NODE_6:
         case PREFER_REPL_NODE_7:
         {
            posTmp = preferInsType - 1 ;
            break;
         }
         case PREFER_REPL_MASTER:
         {
            posTmp = pGroupItem->getPrimaryPos() ;
            // if there is no primary,
            // then do not break and go on to
            // get random node
            if ( CLS_RG_NODE_POS_INVALID != posTmp )
            {
               break ;
            }
         }
         case PREFER_REPL_ANYONE:
         case PREFER_REPL_SLAVE:
         default:
         {
            posTmp = random ;
            break;
         }
      }

      if( pGroupItem->nodeCount() > 0 )
      {
         posTmp = posTmp % pGroupItem->nodeCount() ;
      }

      return (INT32)posTmp ;
   }

   INT32 _coordGroupSel::_nextPos( CoordGroupInfoPtr &groupPtr,
                                   INT32 preferInsType,
                                   UINT32 &selTimes,
                                   INT32 &pos,
                                   MsgRouteID &nodeID )
   {
      INT32 rc = SDB_OK ;
      INT32 status = NET_NODE_STAT_UNKNOWN ;
      UINT32 tmpPos = 0 ;
      BOOLEAN foundNode = FALSE ;
      clsGroupItem *pGroupItem = NULL ;
      UINT32 groupID = groupPtr->groupID() ;

      pGroupItem = groupPtr.get() ;
      while( selTimes < pGroupItem->nodeCount() )
      {
         tmpPos = pos ;
         if ( selTimes > 0 )
         {
            tmpPos = ( pos + 1 ) % pGroupItem->nodeCount() ;
         }

         if ( PREFER_REPL_SLAVE == preferInsType )
         {
            UINT32 pimaryPos = pGroupItem->getPrimaryPos() ;

            if ( CLS_RG_NODE_POS_INVALID != pimaryPos &&
                 selTimes + 1 == pGroupItem->nodeCount() )
            {
               tmpPos = pimaryPos ;
            }
            else if ( tmpPos == pimaryPos )
            {
               tmpPos = ( tmpPos + 1 ) % pGroupItem->nodeCount() ;
            }
         }

         pos = tmpPos ;
         ++selTimes ;

         rc = pGroupItem->getNodeID( pos, nodeID, _svcType ) ;
         if ( rc )
         {
            goto error ;
         }
         rc = pGroupItem->getNodeInfo( pos, status ) ;
         if ( rc )
         {
            goto error ;
         }
         if ( NET_NODE_STAT_NORMAL == status )
         {
            foundNode = TRUE ;
            break ;
         }
         ++_ignoredNum ;
      }

      if ( !foundNode )
      {
         rc = SDB_CLS_NODE_BSFAULT ;

         if ( !_hasUpdate )
         {
            _hasUpdate = TRUE ;
            rc = _pResource->updateGroupInfo( groupID, groupPtr,
                                              _pPropSite->getEDUCB() ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Update group[%u] info failed, rc: %d",
                       groupID, rc ) ;
            }
         }

         if ( SDB_OK == rc )
         {
            _resetStatus() ;
            rc = _selOtherBegin( nodeID ) ;
         }
         if ( rc )
         {
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _coordGroupSel::selDone()
   {
      if ( MSG_INVALID_ROUTEID != _lastNodeID.value )
      {
         _pPropSite->addLastNode( _lastNodeID.columns.groupID,
                                  _lastNodeID.value ) ;
      }
      _resetStatus() ;
   }

   void _coordGroupSel::updateStat( const MsgRouteID &nodeID, INT32 rc )
   {
      if ( MSG_INVALID_ROUTEID != nodeID.value )
      {
         if ( SDB_OK != rc && _pPropSite )
         {
            _pPropSite->delLastNode( nodeID.columns.groupID,
                                     nodeID.value ) ;
         }

         if( _groupPtr.get() )
         {
            _groupPtr->updateNodeStat( nodeID.columns.nodeID,
                                       netResult2Status( rc ) ) ;
         }
      }
   }

   /*
      _coordGroupSessionCtrl implement
   */
   _coordGroupSessionCtrl::_coordGroupSessionCtrl()
   {
      _retryTime = 0 ;
   }

   _coordGroupSessionCtrl::~_coordGroupSessionCtrl()
   {
   }

   void _coordGroupSessionCtrl::incRetry()
   {
      ++_retryTime ;
   }

   BOOLEAN _coordGroupSessionCtrl::canRetry( INT32 flag,
                                             const MsgRouteID &nodeID,
                                             UINT32 newPrimaryID,
                                             BOOLEAN isReadCmd,
                                             BOOLEAN canUpdate )
   {
      /// TODO:XUJIANHUI
      return FALSE ;
   }

   /*
      _coordGroupSession implement
   */
   _coordGroupSession::_coordGroupSession()
   {
      _pSite      = NULL ;
      _pPropSite  = NULL ;
      _pSession   = NULL ;
   }

   _coordGroupSession::~_coordGroupSession()
   {
      if ( _pSession )
      {
         _pSite->removeSession( _pSession ) ;
      }
      _pSite      = NULL ;
      _pPropSite  = NULL ;
      _pSession   = NULL ;
   }

   INT32 _coordGroupSession::init( coordResource *pResource,
                                   _pmdEDUCB *cb,
                                   INT64 timeout,
                                   IRemoteSessionHandler *pHandle )
   {
      INT32 rc = SDB_OK ;

      if ( !pResource || !cb )
      {
         SDB_ASSERT( FALSE, "Invalid arguments" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( _pSession )
      {
         SDB_ASSERT( FALSE, "Already init" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;
      if ( !_pSite )
      {
         PD_LOG( PDERROR, "Session[%s] is not registered",
                 cb->toString().c_str() ) ;
         rc = SDB_SYS ;
         SDB_ASSERT( FALSE, "Session is not registered" ) ;
         goto error ;
      }
      _pPropSite = ( coordSessionPropSite* )_pSite->getUserData() ;
      if ( !_pPropSite )
      {
         rc = SDB_SYS ;
         SDB_ASSERT( FALSE, "Prop site can't be NULL" ) ;
         goto error ;
      }
      _groupSel.init( pResource, _pPropSite ) ;

      if ( 0 == timeout )
      {
         timeout = _pPropSite->getOprTimeout() ;
      }
      if ( !pHandle )
      {
         pHandle = &_baseHandle ;
      }
      _pSession = _pSite->addSession( timeout, pHandle ) ;
      if ( !_pSession )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      _pSession->setUserData( (UINT64)this ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _coordGroupSession::clear()
   {
      if ( _pSession )
      {
         _pSession->clearSubSession() ;
      }
   }

   pmdRemoteSession* _coordGroupSession::getSession()
   {
      return _pSession ;
   }

   coordGroupSel* _coordGroupSession::getGroupSel()
   {
      return &_groupSel ;
   }

   coordGroupSessionCtrl* _coordGroupSession::getGroupCtrl()
   {
      return &_groupCtrl ;
   }

   coordRemoteHandleBase* _coordGroupSession::getBaseHandle()
   {
      return &_baseHandle ;
   }

   coordSessionPropSite* _coordGroupSession::getPropSite()
   {
      return _pPropSite ;
   }

   INT32 _coordGroupSession::sendMsg( MsgHeader *pSrcMsg,
                                      UINT32 groupID,
                                      const netIOVec *pIov,
                                      pmdSubSession **ppSub )
   {
      return _sendMsg( pSrcMsg, groupID, pIov, ppSub ) ;
   }

   INT32 _coordGroupSession::sendMsg( MsgHeader *pSrcMsg,
                                      CoordGroupList &grpLst,
                                      const netIOVec *pIov )
   {
      INT32 rc = SDB_OK ;
      CoordGroupList::iterator it ;

      it = grpLst.begin() ;
      while( it != grpLst.end() )
      {
         rc = _sendMsg( pSrcMsg, it->first, pIov ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Send msg[%s] to group[%u] failed, rc:%d",
                    msg2String( pSrcMsg ).c_str(), it->first, rc ) ;
            goto error ;
         }
         ++it ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordGroupSession::sendMsg( MsgHeader *pSrcMsg,
                                      CoordGroupList &grpLst,
                                      const GROUP_2_IOVEC &iov )
   {
      INT32 rc = SDB_OK ;
      CoordGroupList::iterator it ;
      GROUP_2_IOVEC::const_iterator itIO ;
      const netIOVec *pCommonIO = NULL ;
      const netIOVec *pIOVec = NULL ;

      // find common iovec
      itIO = iov.find( 0 ) ; // group id is 0 for common iovec
      if ( iov.end() != itIO )
      {
         pCommonIO = &( itIO->second ) ;
      }

      it = grpLst.begin() ;
      while( it != grpLst.end() )
      {
         itIO = iov.find( it->first ) ;
         if ( iov.end() != itIO )
         {
            pIOVec = &( itIO->second ) ;
         }
         else if ( pCommonIO )
         {
            pIOVec = pCommonIO ;
         }
         else
         {
            PD_LOG( PDERROR, "Can't find the group[%u]'s iovec datas",
                    it->first ) ;
            rc = SDB_SYS ;
            SDB_ASSERT( FALSE, "Group iovec is null" ) ;
            goto error ;
         }

         rc = _sendMsg( pSrcMsg, it->first, pIOVec ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Send msg[%s] to group[%u] failed, rc:%d",
                    msg2String( pSrcMsg ).c_str(), it->first, rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordGroupSession::_sendMsg( MsgHeader *pSrcMsg,
                                       UINT32 groupID,
                                       const netIOVec *pIov,
                                       pmdSubSession **ppSub )
   {
      INT32 rc = SDB_OK ;
      pmdSubSession *pSub = NULL ;
      MsgRouteID nodeID ;

      rc = _groupSel.selBegin( groupID, nodeID ) ;
      if ( rc )
      {
         goto error ;
      }

      while( TRUE )
      {
         pSub = _pSession->addSubSession( nodeID.value ) ;
         if ( !pSub )
         {
            PD_LOG( PDERROR, "Add sub[%s] to session[%llu] failed",
                    routeID2String( nodeID ).c_str(),
                    _pSession->sessionID() ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         pSub->setReqMsg( pSrcMsg, PMD_EDU_MEM_NONE ) ;
         if ( pIov )
         {
            pSub->addIODatas( *pIov ) ;
         }

         rc = _pSession->sendMsg( pSub ) ;
         if ( SDB_OK == rc )
         {
            if ( ppSub )
            {
               *ppSub = pSub ;
            }
            _groupSel.selDone() ;
            break ;
         }
         /// remove the sub node
         _pSession->delSubSession( nodeID.value ) ;
         /// update node stat
         _groupSel.updateStat( nodeID, rc ) ;
         /// get next node
         if ( SDB_OK != _groupSel.selNext( nodeID ) )
         {
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

}

