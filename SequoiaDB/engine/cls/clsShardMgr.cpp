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

   Source File Name = clsShardMgr.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          22/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "clsShardMgr.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "rtn.hpp"
#include "msgMessage.hpp"
#include "msgCatalog.hpp"
#include "../bson/bson.h"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

using namespace bson ;

namespace engine 
{

   struct _hostAndPort
   {
      std::string _host ;
      std::string _svc ;

      _hostAndPort( std::string host, std::string svc )
      {
         _host = host ;
         _svc = svc ;
      }
      _hostAndPort() {}
   } ;

   BEGIN_OBJ_MSG_MAP(_clsShardMgr, _pmdObjBase)
      ON_MSG ( MSG_CAT_CATGRP_RES, _onCatCatGroupRes )
      ON_MSG ( MSG_CAT_NODEGRP_RES, _onCatGroupRes )
      ON_MSG ( MSG_CAT_QUERY_CATALOG_RSP, _onCatalogReqMsg )
      ON_MSG ( MSG_CAT_QUERY_SPACEINFO_RSP, _onQueryCSInfoRsp )
   END_OBJ_MSG_MAP()

   _clsShardMgr::_clsShardMgr ( _netRouteAgent *rtAgent )
   {
      _pNetRtAgent = rtAgent ;
      _requestID = 0 ;
      _pCatAgent = NULL ;
      _pNodeMgrAgent = NULL ;

      _primary = -1 ;
      _catVerion = 0 ;
      _nodeID.value = 0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_DECONSTRUCTOR, "_clsShardMgr::~_clsShardMgr" )
   _clsShardMgr::~_clsShardMgr()
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_DECONSTRUCTOR );
      _pNetRtAgent = NULL ;

      SAFE_DELETE ( _pCatAgent ) ;
      SAFE_DELETE ( _pNodeMgrAgent ) ;

      //release event
      MAP_CAT_EVENT_IT it = _mapSyncCatEvent.begin () ;
      while ( it != _mapSyncCatEvent.end() )
      {
         SDB_OSS_DEL it->second ;
         ++it ;
      }
      _mapSyncCatEvent.clear () ;

      MAP_NM_EVENT_IT itNM = _mapSyncNMEvent.begin () ;
      while ( itNM != _mapSyncNMEvent.end() )
      {
         SDB_OSS_DEL itNM->second ;
         ++itNM ;
      }
      _mapSyncNMEvent.clear () ;
      PD_TRACE_EXIT ( SDB__CLSSHDMGR_DECONSTRUCTOR );
   }

   void _clsShardMgr::setNodeID ( const MsgRouteID & nodeID )
   {
      _nodeID = nodeID ;
      _nodeID.columns.serviceID = 0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_INIT, "_clsShardMgr::initialize" )
   INT32 _clsShardMgr::initialize()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_INIT );
      UINT32 index = 0 ;
      UINT32 catGID = CATALOG_GROUPID ;
      UINT16 catNID = SYS_NODE_ID_BEGIN + CLS_REPLSET_MAX_NODE_SIZE ;
      MsgRouteID id ;
      pmdOptionsCB *optCB = pmdGetOptionCB() ;
      // catAddrs is pointing to option control block
      vector< _pmdAddrPair > catAddrs = optCB->catAddrs() ;

      if ( !_pNetRtAgent )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "network runtime agent can't be NULL, rc = %d",
                  rc ) ;
         goto error ;
      }

      // init param, get configured catalog address
      for ( UINT32 i = 0 ; i < catAddrs.size() ; ++i )
      {
         if ( 0 == catAddrs[i]._host[ 0 ] )
         {
            break ;
         }
         id.columns.groupID = catGID ;
         id.columns.nodeID = catNID++ ;
         id.columns.serviceID = MSG_ROUTE_CAT_SERVICE ;
         setCatlogInfo( id, catAddrs[i]._host, catAddrs[i]._service ) ;
      }

      if ( _vecCatlog.size() == 0 )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Catalog information was not properly configured, "
                  "rc = %d", rc ) ;
         goto error ;
      }

      // update catalog to agent
      for ( index = 0 ; index < _vecCatlog.size() ; ++index )
      {
         _pNetRtAgent->updateRoute ( _vecCatlog[index].nodeID,
                                     _vecCatlog[index].host.c_str(),
                                     _vecCatlog[index].service.c_str() ) ;
      }

      SAFE_NEW_GOTO_ERROR  ( _pCatAgent, _clsCatalogAgent ) ;
      SAFE_NEW_GOTO_ERROR  ( _pNodeMgrAgent, _clsNodeMgrAgent ) ;

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_INIT, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsShardMgr::active ()
   {
      return SDB_OK ;
   }

   INT32 _clsShardMgr::deactive ()
   {
      return SDB_OK ;
   }

   INT32 _clsShardMgr::final ()
   {
      return SDB_OK ;
   }

   void _clsShardMgr::onConfigChange ()
   {
   }

   void _clsShardMgr::ntyPrimaryChange( BOOLEAN primary,
                                        SDB_EVENT_OCCUR_TYPE type )
   {
      if ( primary && SDB_EVT_OCCUR_BEFORE == type )
      {
           // clear catalog info
         _pCatAgent->lock_w() ;
         _pCatAgent->clearAll() ;
         _pCatAgent->release_w() ;
      }
   }

   void _clsShardMgr::attachCB( _pmdEDUCB * cb )
   {
      sdbGetClsCB()->attachCB( cb ) ;
   }

   void _clsShardMgr::detachCB( _pmdEDUCB * cb )
   {
      sdbGetClsCB()->detachCB( cb ) ;
   }

   void _clsShardMgr::onTimer ( UINT32 timerID, UINT32 interval )
   {
   }

   NodeID _clsShardMgr::nodeID () const
   {
      return _nodeID ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SETCATINFO, "_clsShardMgr::setCatlogInfo" )
   void _clsShardMgr::setCatlogInfo ( const NodeID & id,
                                      const std::string& host,
                                      const std::string& service )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_SETCATINFO );
      _catlogServerInfo info ;
      info.nodeID.value = id.value ;
      info.nodeID.columns.serviceID = MSG_ROUTE_CAT_SERVICE ;
      info.host = host ;
      info.service = service ;

      _vecCatlog.push_back ( info ) ;
      PD_TRACE_EXIT ( SDB__CLSSHDMGR_SETCATINFO );
   }

   catAgent* _clsShardMgr::getCataAgent ()
   {
      return _pCatAgent ;
   }

   nodeMgrAgent* _clsShardMgr::getNodeMgrAgent ()
   {
      return _pNodeMgrAgent ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SYNCSND, "_clsShardMgr::syncSend" )
   INT32 _clsShardMgr::syncSend( MsgHeader * msg, UINT32 groupID,
                                 BOOLEAN primary, MsgHeader **ppRecvMsg,
                                 INT64 millisec )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_SYNCSND ) ;
      INT32 rc = SDB_OK ;
      std::vector< _hostAndPort > hosts ;
      BOOLEAN hasUpdateGroup = FALSE ;

   retry:
      // if we are sending to catalog group
      if ( CATALOG_GROUPID == groupID )
      {
         ossScopedLock lock ( &_shardLatch, SHARED ) ;
         // detect if we want to send to catalog primary
         if ( primary && _primary >= 0 && _primary < (INT32)_vecCatlog.size() )
         {
            hosts.push_back( _hostAndPort( _vecCatlog[_primary].host,
                                           _vecCatlog[_primary].service ) ) ;
         }
         // if we want to broadcast, we push everything in catalog list
         else if ( !primary )
         {
            for ( UINT32 i = 0 ; i < _vecCatlog.size() ; ++i )
            {
               hosts.push_back( _hostAndPort( _vecCatlog[i].host,
                                              _vecCatlog[i].service ) ) ;
            }
         }
      }
      // if we are sending to user group
      else
      {
         clsGroupItem *item = NULL ;
         rc = getAndLockGroupItem( groupID, &item, TRUE, CLS_SHARD_TIMEOUT,
                                   &hasUpdateGroup ) ;
         if ( SDB_OK == rc )
         {
            std::string host ;
            std::string svc ;

            // sending to primary only
            if ( primary )
            {
               rc = item->getNodeInfo( item->primary(MSG_ROUTE_SHARD_SERVCIE),
                                       host, svc ) ;
               if ( rc )
               {
                  goto error ;
               }
               hosts.push_back( _hostAndPort( host, svc ) ) ;
            }
            // broadcast to all nodes in a given group
            else
            {
               const VEC_NODE_INFO *pNodes = item->getNodes() ;
               for ( UINT32 i = 0 ; i < pNodes->size() ; ++i )
               {
                  hosts.push_back( _hostAndPort( (*pNodes)[i]._host,
                     (*pNodes)[i]._service[MSG_ROUTE_SHARD_SERVCIE] ) ) ;
               }
            }

            unlockGroupItem( item ) ;
         }
      }

      // make sure we get some nodes at least
      // if we can't find anything in the cache, let's try
      // to update using up-to-date catalog information
      if ( 0 == hosts.size() )
      {
         rc = SDB_CLS_NODE_NOT_EXIST ;
      }

      // if we didn't find anything and we have not tried to refresh catalog
      if ( SDB_CLS_NODE_NOT_EXIST == rc && !hasUpdateGroup )
      {
         hasUpdateGroup = TRUE ;
         // need to update
         if ( CATALOG_GROUPID == groupID )
         {
            rc = updateCatGroup( FALSE, CLS_SHARD_TIMEOUT ) ;
         }
         else
         {
            rc = syncUpdateGroupInfo( groupID ) ;
         }

         if ( SDB_OK == rc )
         {
            goto retry ;
         }
      }

      PD_RC_CHECK( rc, PDERROR, "Failed to find nodes for sync send, "
                   "group id = %d, rc = %d", groupID, rc ) ;
      // now let's send the information
      {
         UINT32 msgLength = 0 ;
         INT32 receivedLen = 0 ;
         INT32 sentLen = 0 ;
         CHAR* buff = NULL ;
         UINT16 port = 0 ;
         // randomly pickup a starting position
         UINT32 pos = ossRand() % hosts.size() ;

         for ( UINT32 i = 0 ; i < hosts.size() ; ++i )
         {
            // convert from service port to real port
            _hostAndPort &tmpInfo = hosts[pos] ;
            pos = ( pos + 1 ) % hosts.size() ;
            ossGetPort( tmpInfo._svc.c_str(), port ) ;

            // use millisecond
            // establish a socket connection, will be closed by end of
            // the scope
            ossSocket tmpSocket ( tmpInfo._host.c_str(), port, millisec ) ;
            rc = tmpSocket.initSocket() ;
            PD_RC_CHECK( rc, PDERROR, "Init socket %s:%d failed, rc:%d",
                         tmpInfo._host.c_str(), port, rc ) ;

            rc = tmpSocket.connect() ;
            // if we are not able to connect to a node, let's skip and retry
            // next one
            if ( rc )
            {
               PD_LOG( PDWARNING, "Connect to %s:%d failed, rc:%d",
                       tmpInfo._host.c_str(), port, rc ) ;
               continue ;
            }

            // send msg, if we can connect to the node but failed to send
            // we do not retry, simply indicate something goes wrong
            rc = tmpSocket.send( (const CHAR *)msg, msg->messageLength,
                                 sentLen, millisec ) ;
            PD_RC_CHECK( rc, PDERROR, "Send messge to %s:%d failed, rc:%d",
                         tmpInfo._host.c_str(), port, rc ) ;

            // recieve msg, do not loop and retry
            rc = tmpSocket.recv( (CHAR*)&msgLength, sizeof(INT32), receivedLen,
                                 millisec ) ;
            PD_RC_CHECK( rc, PDERROR, "Recieve msg length failed, rc: %d",
                         rc ) ;

            if ( msgLength < sizeof(INT32) || msgLength > SDB_MAX_MSG_LENGTH )
            {
               PD_LOG ( PDERROR, "Recieve msg length[%d] error", msgLength ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            // buff is freed outside the function
            buff = (CHAR*)SDB_OSS_MALLOC( msgLength + 1 ) ;
            if ( !buff )
            {
               rc = SDB_OOM ;
               PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
                        msgLength + 1 ) ;
               goto error ;
            }
            *(INT32*)buff = msgLength ;
            // do not loop and retry, simply return error message when we failed to
            // recv, including timeout, because this is internal communication and
            // we should control the timeout value
            rc = tmpSocket.recv( &buff[sizeof(INT32)], msgLength-sizeof(INT32),
                                 receivedLen,
                                 millisec ) ;
            if ( rc )
            {
               SDB_OSS_FREE( buff ) ;
               PD_LOG ( PDERROR, "Recieve response message failed, rc: %d", rc ) ;
               goto error ;
            }
            // Once we received something, we just break out the loop
            // so no memory leak here
            *ppRecvMsg = (MsgHeader*)buff ;
            break ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_SYNCSND, rc );
      return rc ;
   error:
      goto done ;
   }

   // send message specifically to catalog
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SND2CAT, "_clsShardMgr::sendToCatlog" )
   INT32 _clsShardMgr::sendToCatlog ( MsgHeader * msg, INT32 *pSendNum )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_SND2CAT );

      ossScopedLock lock ( &_shardLatch, SHARED ) ;
      INT32 tmpPrimary = _primary ;

      // set returned sent num to 0
      if ( pSendNum )
      {
         *pSendNum = 0 ;
      }

      // sanity check
      if ( !_pNetRtAgent || _vecCatlog.size() == 0 )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Either network runtime agent does not exist, "
                  "or catalog list is empty, rc = %d", rc ) ;
         goto error ;
      }

      // if we know the catalog primary node, let's try to send
      if ( tmpPrimary >= 0 && tmpPrimary < (INT32)_vecCatlog.size () )
      {
         // if we are not able to get successful send, let's try to
         // broadcast to all catalog node
         rc = _pNetRtAgent->syncSend ( _vecCatlog[tmpPrimary].nodeID,
                                       (void*)msg ) ;
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDWARNING,
                     "Send message to primary catlog[%s:%s] failed[rc:%d]",
                     _vecCatlog[tmpPrimary].host.c_str(),
                     _vecCatlog[tmpPrimary].service.c_str(),
                     rc ) ;
            _primary = -1 ;
            //will send to all catalog node
         }
         else
         {
            // if send success, let's set sent num to 1
            if ( pSendNum )
            {
               *pSendNum = 1 ;
            }
            goto done ;
         }
      }

      // either we want to broadcast or failed to send to primary,
      // let's send to all catlog node
      {
         UINT32 index = 0 ;
         INT32 rc1 = SDB_OK ;
         rc = SDB_NET_SEND_ERR ;

         while ( index < _vecCatlog.size () )
         {
            // we sent to each node in catalog list
            rc1 = _pNetRtAgent->syncSend ( _vecCatlog[index].nodeID,
                                           (void*)msg ) ;
            // for any success send, let's set pSendNum+1, and set rc to SDB_OK
            if ( rc1 == SDB_OK )
            {
               rc = rc1 ;
               if ( pSendNum )
               {
                  ++(*pSendNum) ;
               }
            }
            else
            {
               PD_LOG ( PDWARNING,
                        "Send message to catlog[%s:%s] failed[rc:%d]. "
                        "It is possible because the remote service was not "
                        "started yet",
                        _vecCatlog[index].host.c_str(),
                        _vecCatlog[index].service.c_str() , rc1 ) ;
            }

            index++ ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_SND2CAT, rc );
      return rc ;
   error:
      goto done ;
   }

   // update cached catalog group information
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_UPDCATGRP, "_clsShardMgr::updateCatGroup" )
   INT32 _clsShardMgr::updateCatGroup ( BOOLEAN unsetPrimary, INT64 millsec )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_UPDCATGRP );
      SDB_ASSERT ( _vecCatlog.size() > 0,
                   "there's at least 1 catalog exist" ) ;
      if ( unsetPrimary )
      {
         _primary = -1 ;
      }

      // build catalog group request
      MsgCatCatGroupReq req ;
      req.header.opCode = MSG_CAT_CATGRP_REQ ;
      req.id.value = 0 ;
      req.id.columns.groupID = CATALOG_GROUPID ;

      //send to a catalog
      UINT32 index = 0 ;
      INT32 rc = SDB_OK ;

      if ( millsec > 0 )
      {
         // use request id to store tid
         req.header.requestID = (UINT64)ossGetCurrentThreadID() ;
         _upCatEvent.reset() ;
      }

      // send message
      {
         ossScopedLock lock ( &_shardLatch, SHARED ) ;

         while ( index < _vecCatlog.size () )
         {
            // send message to each catalog node
            rc = _pNetRtAgent->syncSend ( _vecCatlog[index].nodeID,
               (void*)&req ) ;
            // we consider success for any SDB_OK return
            if ( SDB_OK == rc )
            {
               break ;
            }

            index++ ;
         }
      }

      // we can guarantee _vecCatlog is not empty here, so if we didn't
      // send anything success, we'll get error here for sure
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to sync send to catalog, rc = %d", rc ) ;
         goto error ;
      }

      if ( millsec > 0 )
      {
         INT32 result = 0 ;
         rc = _upCatEvent.wait( millsec, &result ) ;
         if ( SDB_OK == rc )
         {
            rc = result ;
         }
      }
   done :
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_UPDCATGRP, rc );
      return rc ;
   error :
      goto done ;
   }

   // drop all data cs
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_CLRALLDATA, "_clsShardMgr::clearAllData" )
   INT32 _clsShardMgr::clearAllData ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_CLRALLDATA );
      _SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      std::set<_monCollectionSpace> csList ;

      PD_LOG ( PDEVENT, "Clear all dms data" ) ;

      //dump all collectionspace
      dmsCB->dumpInfo( csList, TRUE ) ;
      std::set<_monCollectionSpace>::const_iterator it = csList.begin() ;
      // for now we don't need to latch dms, since it's only used by
      // full sync, and in that case there's no application able to modify
      // collection space information
      while ( it != csList.end() )
      {
         // drop all collection spaces
         const _monCollectionSpace &cs = *it ;
         rc = rtnDropCollectionSpaceCommand ( cs._name, NULL, dmsCB, NULL,
                                              TRUE ) ;
         if ( SDB_OK != rc && SDB_DMS_CS_NOTEXIST != rc )
         {
            PD_LOG ( PDERROR, "Clear collectionspace[%s] failed[rc:%d]",
               cs._name, rc ) ;
            break ;
         }
         PD_LOG ( PDDEBUG, "Clear collectionspace[%s] succeed", cs._name ) ;
         ++it ;
      }

      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_CLRALLDATA, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SYNCUPDCAT, "_clsShardMgr::syncUpdateCatalog" )
   INT32 _clsShardMgr::syncUpdateCatalog ( const CHAR *pCollectionName,
                                           INT64 millsec )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_SYNCUPDCAT );
      BOOLEAN send = FALSE ;
      clsEventItem *pEventInfo = NULL ;

      if ( !pCollectionName )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "collection name can't be NULL, rc = %d", rc ) ;
         goto error ;
      }

      _catLatch.get() ;
      // look for sync event from cache
      // memory will be released in this function, when there's no thread
      // wait for the event
      pEventInfo = _findCatSyncEvent( pCollectionName, TRUE ) ;
      if ( !pEventInfo )
      {
         _catLatch.release () ;
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "Failed to allocate memory for event info, "
                  "rc = %d", rc ) ;
         goto error ;
      }

      //First judge the request is send or not
      if ( FALSE == pEventInfo->send )
      {
         // if the event has not been sent, let's send to catalog
         rc = _sendCatalogReq ( pCollectionName, 0, &(pEventInfo->sendNums) ) ;
         if ( SDB_OK == rc )
         {
            pEventInfo->send = TRUE ;
            send = TRUE ;
            pEventInfo->waitNum++ ;
            pEventInfo->requestID = _requestID ;
            pEventInfo->event.reset () ;
         }
      }
      else
      {
         // if the event is already sent, let's increase the wait counter
         // note this counter must be protected within catLatch
         pEventInfo->waitNum++ ;
      }

      _catLatch.release () ;

      // we wait for the event if we didn't send anything, or the sent
      // complete successfully
      if ( SDB_OK == rc )
      {
         // wait until event is acknowledged
         INT32 result = SDB_OK ;
         rc = pEventInfo->event.wait ( millsec, &result ) ;

         if ( SDB_OK == rc )
         {
            rc = result ;
         }

         // if send=TRUE, must reset send flag
         _catLatch.get () ;
         // decrease the wait number, this must be protected within catLatch
         pEventInfo->waitNum-- ;

         if ( send )
         {
            pEventInfo->send = FALSE ;
         }

         // clear event info if there's no thread wait for it
         if ( 0 == pEventInfo->waitNum )
         {
            pEventInfo->event.reset () ;

            //release the event info
            SDB_OSS_DEL pEventInfo ;
            pEventInfo = NULL ;
            _mapSyncCatEvent.erase ( pCollectionName ) ;
         }

         _catLatch.release () ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_SYNCUPDCAT, rc );
      return rc ;
   error:
      goto done ;
   }

   // Update information for any group
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SYNCUPDGPINFO, "_clsShardMgr::syncUpdateGroupInfo" )
   INT32 _clsShardMgr::syncUpdateGroupInfo ( UINT32 groupID, INT64 millsec )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_SYNCUPDGPINFO );
      BOOLEAN send = FALSE ;
      clsEventItem *pEventInfo = NULL ;

      _catLatch.get() ;

      // let's try to create or find existing sync event for a given group
      // memory will be released in this function if there's no other threads
      // wait for the event
      pEventInfo = _findNMSyncEvent( groupID, TRUE ) ;
      if ( !pEventInfo )
      {
         _catLatch.release () ;
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "Failed to allocate event info for group %d, "
                  "rc = %d", groupID, rc ) ;
         goto error ;
      }

      //First judge the request is send or not
      if ( FALSE == pEventInfo->send )
      {
         // send group request to catalog ONLY if it's not been sent yet
         rc = _sendGroupReq( groupID, 0, &(pEventInfo->sendNums) ) ;
         // if it's successfully sent, let's mark and wait
         if ( SDB_OK == rc )
         {
            pEventInfo->send = TRUE ;
            send = TRUE ;
            pEventInfo->waitNum++ ;
            pEventInfo->requestID = _requestID ;
            pEventInfo->event.reset () ;
         }
      }
      else
      {
         pEventInfo->waitNum++ ;
      }

      _catLatch.release () ;

      // wait only when someone else is already sent the request
      // or the sent from current thread success
      if ( SDB_OK == rc )
      {
         INT32 result = SDB_OK ;
         rc = pEventInfo->event.wait ( millsec, &result ) ;

         if ( SDB_OK == rc )
         {
            rc = result ;
         }

         // if send=TRUE, must reset send flag
         _catLatch.get () ;
         pEventInfo->waitNum-- ;

         if ( send )
         {
            pEventInfo->send = FALSE ;
         }

         // clear memory resource when there's no other threads
         // wait for the event
         if ( 0 == pEventInfo->waitNum )
         {
            pEventInfo->event.reset () ;

            //release the event info
            SDB_OSS_DEL pEventInfo ;
            pEventInfo = NULL ;
            _mapSyncNMEvent.erase ( groupID ) ;
         }

         _catLatch.release () ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_SYNCUPDGPINFO,  rc );
      return rc ;
   error:
      goto done ;
   }

   // set or unset a given node id as catalog primary
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_UPDPRM, "_clsShardMgr::updatePrimary" )
   INT32 _clsShardMgr::updatePrimary ( const NodeID & id, BOOLEAN primary )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_UPDPRM );
      UINT32 index = 0 ;

      ossScopedLock lock ( &_shardLatch, SHARED ) ;

      // we don't do anything if we want to unset primary but we don't
      // actually have any primary in cache
      if ( _primary == -1 && FALSE == primary )
      {
         goto done ;
      }

      // loop through all catalog node and compare the id
      // if we find the primary let's set _primary to such node
      while ( index < _vecCatlog.size() )
      {
         if ( id.value == _vecCatlog[index].nodeID.value )
         {
            if ( primary )
            {
               _primary = index ;
            }
            else
            {
               _primary = -1 ;
            }
            goto done ;
         }
         index++ ;
      }
      // if we get here, that means we cannot find the id in _vecCatlog
      rc = SDB_SYS ;
      PD_LOG ( PDERROR, "Catalog primary node to [%s] id error[%u:%u:%u]",
               primary ? "primary" : "slave",
               id.columns.groupID,
               id.columns.nodeID,
               id.columns.serviceID ) ;

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_UPDPRM, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__SNDGPREQ, "_clsShardMgr::_sendGroupReq" )
   INT32 _clsShardMgr::_sendGroupReq ( UINT32 groupID, UINT64 requestID,
                                       INT32 *pSendNum )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__SNDGPREQ );
      _MsgCatGroupReq msg ;
      msg.header.opCode = MSG_CAT_NODEGRP_REQ ;

      // set to default request id if we didn't specify any
      if ( 0 == requestID )
      {
         requestID = ++_requestID ;
      }
      msg.header.requestID = requestID ;
      msg.id.columns.groupID = groupID ;

      INT32 rc = sendToCatlog( (MsgHeader *)&msg, pSendNum ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to send to catalog, rc = %d", rc ) ;
         goto error ;
      }

      PD_LOG ( PDDEBUG, "send group req[id: %d, requestID: %lld, rc: %d]",
               groupID, _requestID, rc ) ;
   done :
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__SNDGPREQ, rc );
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__SENDCATAQUERYREQ, "_clsShardMgr::_sendCataQueryReq" )
   INT32 _clsShardMgr::_sendCataQueryReq( INT32 queryType,
                                          const BSONObj & query,
                                          UINT64 requestID,
                                          INT32 *pSendNum )
   {
      INT32 rc        = SDB_OK ;
      CHAR *pBuffer   = NULL ;
      INT32 buffSize  = 0 ;
      MsgHeader * msg = NULL ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__SENDCATAQUERYREQ ) ;

      if ( 0 == requestID )
      {
         requestID = ++_requestID ;
      }

      // pBuffer is freed by end of the function
      rc = msgBuildQueryMsg ( &pBuffer, &buffSize, "CAT", 0, requestID, 0,
                              -1, &query, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to build query msg, rc = %d", rc ) ;
         goto error ;
      }

      msg = (MsgHeader *) pBuffer ;
      msg->opCode = queryType ;
      msg->TID = 0 ;
      msg->routeID.value = 0 ;
      //send message
      rc = sendToCatlog ( msg, pSendNum ) ;
      if ( rc )
      {
         // we don't want the error flush diaglog when catalog is offline
         PD_LOG ( PDDEBUG, "Failed to send message to catalog, rc = %d", rc ) ;
         goto error ;
      }
   done:
      if ( pBuffer )
      {
         SDB_OSS_FREE ( pBuffer ) ;
         pBuffer = NULL ;
      }
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__SENDCATAQUERYREQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__SNDCATREQ, "_clsShardMgr::_sendCatalogReq" )
   INT32 _clsShardMgr::_sendCatalogReq ( const CHAR *pCollectionName,
                                         UINT64 requestID,
                                         INT32 *pSendNum )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__SNDCATREQ );
      BSONObj query ;
      // sanity check
      if ( !pCollectionName )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "collection name can't be NULL, rc = %d", rc ) ;
         goto error ;
      }

      // build BSON object
      try
      {
         query = BSON ( CAT_COLLECTION_NAME << pCollectionName ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception when creating query: %s",
                  e.what () ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // attempt to send to catalog
      rc = _sendCataQueryReq( MSG_CAT_QUERY_CATALOG_REQ, query, requestID,
                              pSendNum ) ;
      if ( SDB_OK != rc )
      {
         // use debug level since we don't want this message flush
         // diaglog when catalog is offline
         PD_LOG ( PDDEBUG, "send catelog req[name: %s, requestID: %lld, "
                  "rc: %d]", pCollectionName, requestID, rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__SNDCATREQ, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__SENDCSINFOREQ, "_clsShardMgr::_sendCSInfoReq" )
   INT32 _clsShardMgr::_sendCSInfoReq( const CHAR * pCSName, UINT64 requestID,
                                       INT32 *pSendNum )
   {
      INT32 rc = SDB_OK ;
      BSONObj query ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__SENDCSINFOREQ ) ;
      // sanity check
      if ( !pCSName )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "cs name can't be NULL, rc = %d", rc ) ;
         goto error ;
      }

      // build query
      try
      {
         query = BSON ( CAT_COLLECTION_SPACE_NAME << pCSName ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception when creating query: %s",
                  e.what () ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // send catalog query request to catalog
      rc = _sendCataQueryReq( MSG_CAT_QUERY_SPACEINFO_REQ, query, requestID,
                              pSendNum ) ;
      if ( SDB_OK != rc )
      {
         // we use debug level here since we don't want this message
         // flush diaglog if catalog is offline
         PD_LOG ( PDDEBUG, "send collection space req[name: %s, requestID: "
                  "%lld, rc: %d]", pCSName, requestID, rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__SENDCSINFOREQ, rc );
      return rc ;
   error:
      goto done ;
   }

   // get catalog group response information
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__ONCATGPRES, "_clsShardMgr::_onCatCatGroupRes" )
   INT32 _clsShardMgr::_onCatCatGroupRes ( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__ONCATGPRES );
      UINT32 version = 0 ;
      MsgRouteID primaryNode ;
      UINT32 primary = 0 ;
      std::string groupName ;
      map<UINT64, _netRouteNode> mapNodes ;
      MsgCatCatGroupRes *res = (MsgCatCatGroupRes*)msg ;

      // sanity check, make sure the response is OKAY
      if ( SDB_OK != MSG_GET_INNER_REPLY_RC(msg) )
      {
         PD_LOG ( PDERROR, "Update catalog group info failed[rc: %d]",
                  MSG_GET_INNER_REPLY_RC(msg) ) ;
         rc = MSG_GET_INNER_REPLY_RC(msg) ;
         goto error ;
      }

      // parse the returned result
      rc = msgParseCatGroupRes ( res, version, groupName, mapNodes, &primary ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Parse MsgCatCatGroupRes failed[rc: %d]", rc ) ;
         goto error ;
      }
      primaryNode.columns.groupID = CATALOG_GROUPID ;

      //update to shard net agent
      if ( 0 == version || version != _catVerion )
      {
         _shardLatch.get () ;

         pmdOptionsCB *optCB = pmdGetOptionCB() ;
         string oldCfg, newCfg ;
         // remember the old info
         optCB->toString( oldCfg ) ;
         VECCATLOG oldCatNodes = _vecCatlog ;
         NodeID oldID ;

         _catVerion = version ;
         _vecCatlog.clear() ;
         optCB->clearCatAddr() ;
         map<UINT64, _netRouteNode>::iterator it = mapNodes.begin() ;
         // iterate for each nodes in catalog list
         while ( it != mapNodes.end() )
         {
            setCatlogInfo ( it->second._id, it->second._host,
                            it->second._service[MSG_ROUTE_CAT_SERVICE] ) ;
            optCB->setCatAddr( it->second._host,
                               it->second._service[
                               MSG_ROUTE_CAT_SERVICE].c_str() ) ;
            ++it ;
         }

         UINT32 index = 0 ;
         // loop through each catalog information
         while ( index < _vecCatlog.size() )
         {
            // try to find old catalog information by id
            if ( SDB_OK == _findCatNodeID ( oldCatNodes, _vecCatlog[index].host,
                                            _vecCatlog[index].service, oldID ) )
            {
               // if any field changed
               if ( oldID.value != _vecCatlog[index].nodeID.value )
               {
                  // update network runtime component
                  _pNetRtAgent->updateRoute ( oldID,
                                              _vecCatlog[index].nodeID ) ;
                  PD_LOG ( PDDEBUG, "Update catalog node[%u:%u] to [%u:%u]",
                           oldID.columns.groupID, oldID.columns.nodeID,
                           _vecCatlog[index].nodeID.columns.groupID,
                           _vecCatlog[index].nodeID.columns.nodeID ) ;
               }
            }
            else
            {
               // if another to find by old id, let's simply update using the
               // new info
               _pNetRtAgent->updateRoute ( _vecCatlog[index].nodeID,
                                           _vecCatlog[index].host.c_str(), 
                                           _vecCatlog[index].service.c_str() ) ;
               PD_LOG ( PDDEBUG, "Update catalog node[%u:%u] to %s:%s",
                        _vecCatlog[index].nodeID.columns.groupID,
                        _vecCatlog[index].nodeID.columns.nodeID,
                        _vecCatlog[index].host.c_str(),
                        _vecCatlog[index].service.c_str() ) ;
            }
            // note we do not remove oldID that no longer appears in new config
            // total refresh must be done by restarting database
            index++ ;
         }
         // convert new optcb to string
         optCB->toString( newCfg ) ;
         // if old and new are different, let's flush
         if ( oldCfg != newCfg )
         {
            // refresh to config file
            optCB->reflush2File() ;
         }

         _shardLatch.release () ;
      }

      // if catalog group, get the primary from repl
      if ( CATALOG_GROUPID == nodeID().columns.groupID )
      {
         replCB *pRepl = sdbGetReplCB() ;
         primary = pRepl->getPrimary().columns.nodeID ;
      }

      //update primary
      if ( primary != 0 )
      {
         primaryNode.columns.serviceID = MSG_ROUTE_CAT_SERVICE ;
         primaryNode.columns.nodeID = primary ;
         rc = updatePrimary ( primaryNode, TRUE ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to update primary, rc = %d", rc ) ;
            goto error ;
         }
      }

   done:
      // signal all threads that's wait for catalog update
      _upCatEvent.signalAll( rc ) ;
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__ONCATGPRES, rc );
      return rc ;
   error:
      goto done ;
   }

   // event of data group update
   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__ONCATGRPRES, "_clsShardMgr::_onCatGroupRes" )
   INT32 _clsShardMgr::_onCatGroupRes ( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__ONCATGRPRES );
      clsEventItem *pEventInfo = NULL ;

      PD_LOG ( PDDEBUG, "Recieve catalog group res[requestID:%lld,flag:%d]",
               msg->requestID, MSG_GET_INNER_REPLY_RC(msg) ) ;

      ossScopedLock lock ( &_catLatch ) ;

      // find a request, we don't create new memory if such event doesn't exist
      // for async call we use 0 as requestID, so we may not get this event
      // in this case if we get success result we have to get the event based on
      // group id
      pEventInfo = _findNMSyncEvent( msg->requestID ) ;

      if ( SDB_OK != MSG_GET_INNER_REPLY_RC(msg) )
      {
         // for error handling
         // 1) group not exist -> we have to clear local cache
         // 2) not primary -> ignore or resend request
         // 3) others report error
         rc = MSG_GET_INNER_REPLY_RC(msg) ;
         // if we are able to find the event
         if ( pEventInfo )
         {
            // let's check if response shows unable to find group
            if ( SDB_CLS_GRP_NOT_EXIST == rc ||
                 SDB_DMS_EOC == rc )
            {
               // in that case, let's clear local group cache information
               _pNodeMgrAgent->lock_w() ;
               _pNodeMgrAgent->clearGroup( pEventInfo->groupID ) ;
               _pNodeMgrAgent->release_w() ;
               pEventInfo->event.signalAll( SDB_CLS_GRP_NOT_EXIST ) ;
            }
            else if ( SDB_CLS_NOT_PRIMARY == rc )
            {
               // if response shows it's not primary, we keep decrease sendnum
               // once it hits 0 we have to resend the request
               --(pEventInfo->sendNums) ;
               if ( pEventInfo->sendNums > 0 )
               {
                  // ignore if we still have other nodes wait for reply
                  // we don't need to do anything here
                  // since there's no other logic behind, we don't do
                  // goto done here
                  rc = SDB_OK ;
               }
               else
               {
                  // if we can't find any primary, we have to send everything
                  // to catalog again
                  updateCatGroup( TRUE ) ;
                  rc = _sendGroupReq( pEventInfo->groupID,
                                      msg->requestID,
                                      &(pEventInfo->sendNums) ) ;
                  // if we failed to resend, we have to notify rc to all other
                  // waited threads
                  if ( rc )
                  {
                     PD_LOG( PDERROR, "Resend group req to catalog failed, "
                             "rc: %d", rc ) ;
                     pEventInfo->event.signalAll( rc ) ;
                  }
               }
            }
            else
            {
               // error handling
               PD_LOG ( PDERROR, "Update group[%d] failed[rc:%d]",
                        pEventInfo->groupID, rc ) ;
               pEventInfo->event.signalAll( rc ) ;
            }
         }
      }
      else
      {
         // if successful returned
         _pNodeMgrAgent->lock_w() ;
         const CHAR* objdata = MSG_GET_INNER_REPLY_DATA(msg) ;
         UINT32 length = msg->messageLength -
                         MSG_GET_INNER_REPLY_HEADER_LEN(msg) ;
         UINT32 groupID = 0 ;

         rc = _pNodeMgrAgent->updateGroupInfo( objdata, length, &groupID ) ;
         PD_LOG ( ( SDB_OK == rc ? PDEVENT : PDERROR ),
                  "Update group[groupID:%u, rc: %d]", groupID, rc ) ;

         //udpate node info to netAgent
         clsGroupItem* groupItem = NULL ;
         if ( SDB_OK == rc )
         {
            groupItem = _pNodeMgrAgent->groupItem( groupID ) ;
            // if the event does not exist, we look for the event based
            // on group id
            // this may happen in async request where requestID = 0
            // in this case the previous call will not be able to get anything
            // so we have to get the wait event here
            if ( !pEventInfo )
            {
               pEventInfo = _findNMSyncEvent( groupID, FALSE ) ;
            }
         }
         // if we do have group information in cache, let's update them
         if ( groupItem )
         {
            UINT32 indexPos = 0 ;
            MsgRouteID nodeID ;
            std::string hostName ;
            std::string service ;

            while ( SDB_OK == groupItem->getNodeInfo( indexPos, nodeID,
                                                      hostName, service,
                                                    MSG_ROUTE_SHARD_SERVCIE ) )
            {
               _pNetRtAgent->updateRoute( nodeID, hostName.c_str(),
                                          service.c_str() ) ;
               ++indexPos ;
            }
         }
         _pNodeMgrAgent->release_w() ;

         // if we have event registered, let's signal to all waiters
         if ( pEventInfo )
         {
            pEventInfo->event.signalAll( rc ) ;
         }
      }

      PD_TRACE_EXITRC (SDB__CLSSHDMGR__ONCATGRPRES, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__ONCATREQMSG, "_clsShardMgr::_onCatalogReqMsg" )
   INT32 _clsShardMgr::_onCatalogReqMsg ( NET_HANDLE handle, MsgHeader* msg )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__ONCATREQMSG );
      MsgCatQueryCatRsp *res   = ( MsgCatQueryCatRsp*)msg ;
      PD_LOG ( PDDEBUG, "Recieve catalog response[requestID: %lld, flag: %d]",
               msg->requestID, res->flags ) ;

      INT32 flag               = 0 ;
      INT64 contextID          = -1 ;
      INT32 startFrom          = 0 ;
      INT32 numReturned        = 0 ;
      vector < BSONObj > objList ;
      UINT32 groupID           = nodeID().columns.groupID ;
      INT32 rc                 = SDB_OK ;
      clsEventItem *pEventInfo = NULL ;

      ossScopedLock lock ( &_catLatch ) ;

      if ( SDB_OK != res->flags )
      {
         rc = SDB_CLS_UPDATE_CAT_FAILED ;

         //need to found event info by request id
         // note this request id could be 0 if it's async call
         // in that case we'll get pEventInfo=NULL
         // and that is absolutely expected
         pEventInfo = _findCatSyncEvent ( msg->requestID ) ;
         if ( pEventInfo )
         {
            //the catalog info is delete, so will delete the local item
            if ( SDB_DMS_EOC == res->flags ||
                 SDB_DMS_NOTEXIST == res->flags )
            {
               _pCatAgent->lock_w () ;
               rc = _pCatAgent->clear ( pEventInfo->name.c_str() ) ;
               _pCatAgent->release_w () ;
               pEventInfo->event.signalAll ( SDB_DMS_NOTEXIST ) ;
            }
            //not primary node, should update catalog group info, and send again
            else if ( SDB_CLS_NOT_PRIMARY == res->flags )
            {
               --(pEventInfo->sendNums) ;
               if ( pEventInfo->sendNums > 0 )
               {
                  // if there's still other pending send event
                  // we can safely ignore the NOTPRIMARY error
                  rc = SDB_OK ;
                  goto done ;
               }

               updateCatGroup ( TRUE ) ;

               rc = _sendCatalogReq ( pEventInfo->name.c_str(), 
                                      msg->requestID,
                                      &(pEventInfo->sendNums) ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Resend catalog req to catalog failed, "
                          "rc: %d", rc ) ;
                  pEventInfo->event.signalAll ( rc ) ;
                  goto error ;
               }
            }
            //update catalog failed
            else
            {
               PD_LOG ( PDERROR, "Update catalog[%s] failed[response: %d]",
                  pEventInfo->name.c_str(), res->flags ) ;
               pEventInfo->event.signalAll ( res->flags ) ;
            }
         }
      }
      else
      {
         _clsCatalogSet *catSet = NULL ;
         INT32 version = 0 ;
         UINT32 groupCount = 0 ;
         const CHAR *pCLType = "normal" ;
         string collectionName ;
         rc = msgExtractReply ( (CHAR *)msg, &flag, &contextID, &startFrom,
                                &numReturned, objList ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG ( PDERROR, "Failed to extract reply msg, rc = %d", rc ) ;
            goto error ;
         }

         SDB_ASSERT ( numReturned == 1 && objList.size() == 1,
                      "Collection catalog item num must be 1" ) ;

         _pCatAgent->lock_w () ;
         rc = _pCatAgent->updateCatalog ( 0, groupID, objList[0].objdata(),
                                          objList[0].objsize(), &catSet ) ;
         if ( catSet )
         {
            version = catSet->getVersion() ;
            groupCount = catSet->groupCount() ;
            collectionName = catSet->name() ;
            if ( catSet->isMainCL() )
            {
               pCLType = "main" ;
            }
            else if ( !catSet->getMainCLName().empty() )
            {
               pCLType = "sub" ;
            }
         }
         _pCatAgent->release_w () ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to update catalog:%d", rc ) ;
            goto error ;
         }

         PD_LOG ( PDEVENT, "Update catalog[name: %s, version:%u, type: %s, "
                  "group count: %d, rc: %d]", collectionName.c_str(),
                  version, pCLType, groupCount, rc ) ;

         //signal collection info event, since the previous pEVentInfo
         //could be NULL ( if it's async call, requestID is 0 ), we'll have
         //to check for pEventInfo here again
         BSONElement ele = objList[0].getField ( CAT_COLLECTION_NAME ) ;
         clsEventItem *pEventInfo = _findCatSyncEvent( ele.str().c_str(),
                                                       FALSE ) ;
         if ( pEventInfo )
         {
            pEventInfo->event.signalAll ( rc ) ;
         }
      }
   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__ONCATREQMSG, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDCATSYNCEV, "_clsShardMgr::_findCatSyncEvent" )
   clsEventItem *_clsShardMgr::_findCatSyncEvent ( const CHAR * pCollectionName,
                                                   BOOLEAN bCreate )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__FNDCATSYNCEV );
      SDB_ASSERT ( pCollectionName , "Collection name can't be NULL" ) ;

      clsEventItem *pEventInfo = NULL ;
      MAP_CAT_EVENT_IT it = _mapSyncCatEvent.find ( pCollectionName ) ;
      if ( it != _mapSyncCatEvent.end() )
      {
         pEventInfo = it->second ;
         goto done ;
      }

      if ( !bCreate )
      {
         goto done ;
      }

      //create new event info
      pEventInfo = SDB_OSS_NEW _clsEventItem ;
      pEventInfo->name = pCollectionName ;
      //add to map
      _mapSyncCatEvent[pCollectionName] = pEventInfo ;

   done:
      PD_TRACE_EXIT ( SDB__CLSSHDMGR__FNDCATSYNCEV );
      return pEventInfo ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDCATSYNCEVN, "_clsShardMgr::_findCatSyncEvent" )
   clsEventItem *_clsShardMgr::_findCatSyncEvent ( UINT64 requestID )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__FNDCATSYNCEVN );
      clsEventItem *pEventInfo = NULL ;
      MAP_CAT_EVENT_IT it = _mapSyncCatEvent.begin() ;
      while ( it != _mapSyncCatEvent.end() )
      {
         pEventInfo = it->second ;
         if ( pEventInfo->requestID == requestID )
         {
            goto done ;
         }
         ++it ;
      }
      pEventInfo = NULL ;
   done :
      PD_TRACE_EXIT ( SDB__CLSSHDMGR__FNDCATSYNCEVN );
      return pEventInfo ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDNMSYNCEV, "_clsShardMgr::_findNMSyncEvent" )
   clsEventItem* _clsShardMgr::_findNMSyncEvent( UINT32 groupID,
                                                 BOOLEAN bCreate )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__FNDNMSYNCEV );
      clsEventItem *pEventInfo = NULL ;
      MAP_NM_EVENT_IT it = _mapSyncNMEvent.find ( groupID ) ;
      if ( it != _mapSyncNMEvent.end() )
      {
         pEventInfo = it->second ;
         goto done ;
      }

      if ( !bCreate )
      {
         goto done ;
      }

      //create new event info
      pEventInfo = SDB_OSS_NEW _clsEventItem ;
      pEventInfo->groupID = groupID ;
      //add to map
      _mapSyncNMEvent[groupID] = pEventInfo ;

   done:
      PD_TRACE_EXIT ( SDB__CLSSHDMGR__FNDNMSYNCEV );
      return pEventInfo ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDNMSYNCEVN, "_clsShardMgr::_findNMSyncEvent" )
   clsEventItem* _clsShardMgr::_findNMSyncEvent ( UINT64 requestID )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__FNDNMSYNCEVN );
      clsEventItem *pEventInfo = NULL ;
      MAP_NM_EVENT_IT it = _mapSyncNMEvent.begin() ;
      while ( it != _mapSyncNMEvent.end() )
      {
         pEventInfo = it->second ;
         if ( pEventInfo->requestID == requestID )
         {
            goto done ;
         }
         ++it ;
      }
      pEventInfo = NULL ;
   done :
      PD_TRACE_EXIT ( SDB__CLSSHDMGR__FNDNMSYNCEVN );
      return pEventInfo ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDCATNODEID, "_clsShardMgr::_findCatNodeID" )
   INT32 _clsShardMgr::_findCatNodeID ( const VECCATLOG & catNodes,
                                        const std::string & host,
                                        const std::string & service,
                                        NodeID & id )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__FNDCATNODEID );
      UINT32 index = 0 ;
      while ( index < catNodes.size() )
      {
         const _catlogServerInfo& node = catNodes[index] ;
         if ( node.host == host && node.service == service )
         {
            id = node.nodeID ;
            goto done ;
         }
         ++index ;
      }
      rc = SDB_CLS_NODE_NOT_EXIST ;
   done :
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__FNDCATNODEID, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_GETANDLOCKCATSET, "_clsShardMgr::getAndLockCataSet" )
   INT32 _clsShardMgr::getAndLockCataSet( const CHAR * name,
                                          clsCatalogSet **ppSet,
                                          BOOLEAN noWithUpdate,
                                          INT64 waitMillSec,
                                          BOOLEAN * pUpdated )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_GETANDLOCKCATSET );
      // sanity check
      SDB_ASSERT ( ppSet && name,
                   "ppSet and name can't be NULL" ) ;

      while ( SDB_OK == rc )
      {
         _pCatAgent->lock_r() ;
         *ppSet = _pCatAgent->collectionSet( name ) ;
         // if we can't find the name and request to update catalog
         // we'll call syncUpdateCatalog and refind again
         if ( !(*ppSet) && noWithUpdate )
         {
            _pCatAgent->release_r() ;
            // request to update catalog
            rc = syncUpdateCatalog( name, waitMillSec ) ;
            if ( rc )
            {
               // if we can't find the collection and not able to update
               // catalog, we'll return the error of synUpdateCatalog
               // call
               PD_LOG ( PDERROR, "Failed to sync update catalog, rc = %d",
                        rc ) ;
               goto error ;
            }
            if ( pUpdated )
            {
               *pUpdated = TRUE ;
            }
            // we don't want to update again
            noWithUpdate = FALSE ;
            continue ;
         }
         // if still not able to find it
         if ( !(*ppSet) )
         {
            _pCatAgent->release_r() ;
            rc = SDB_CLS_NO_CATALOG_INFO ;
         }
         break ;
      }
   done :
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_GETANDLOCKCATSET, rc );
      return rc ;
   error :
      goto done ;
   }

   INT32 _clsShardMgr::unlockCataSet( clsCatalogSet * catSet )
   {
      if ( catSet )
      {
         _pCatAgent->release_r() ;
      }
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_GETNLCKGPITEM, "_clsShardMgr::getAndLockGroupItem" )
   INT32 _clsShardMgr::getAndLockGroupItem( UINT32 id, clsGroupItem **ppItem,
                                            BOOLEAN noWithUpdate,
                                            INT64 waitMillSec,
                                            BOOLEAN * pUpdated )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_GETNLCKGPITEM );
      SDB_ASSERT ( ppItem, "ppItem can't be NULL" ) ;

      while ( SDB_OK == rc )
      {
         _pNodeMgrAgent->lock_r() ;
         *ppItem = _pNodeMgrAgent->groupItem( id ) ;
         // can we find the group from local cache?
         if ( !(*ppItem) && noWithUpdate )
         {
            _pNodeMgrAgent->release_r() ;
            // if we can't find such group and is okay to update cache
            // we'll update cache from catalog
            rc = syncUpdateGroupInfo( id, waitMillSec ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to sync update group info, rc = %d",
                        rc ) ;
               goto error ;
            }
            if ( pUpdated )
            {
               *pUpdated = TRUE ;
            }
            // only update it once
            noWithUpdate = FALSE ;
            continue ;
         }
         // if we are not able to find one, let's return group not found
         if ( !(*ppItem) )
         {
            _pNodeMgrAgent->release_r() ;
            rc = SDB_CLS_NO_GROUP_INFO ;
         }
         break ;
      }
   done :
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_GETNLCKGPITEM, rc );
      return rc ;
   error :
      goto done ;
   }

   INT32 _clsShardMgr::unlockGroupItem( clsGroupItem * item )
   {
      if ( item )
      {
         _pNodeMgrAgent->release_r() ;
      }
      return SDB_OK ;
   }

   INT32 _clsShardMgr::rGetCSPageSize( const CHAR * csName,
                                       UINT32 &pageSize,
                                       UINT32 &lobPageSize,
                                       INT64 waitMillSec )
   {
      INT32 rc = SDB_OK ;
      clsCSEventItem *item = NULL ;
      UINT64 requestID = 0 ;
      INT32 result = 0 ;
      SDB_ASSERT ( csName, "collection space name can't be NULL" ) ;

      // memory will be freed by end of the function
      item = SDB_OSS_NEW clsCSEventItem() ;
      if ( NULL == item )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Alloc memory failed" ) ;
         goto error ;
      }
      item->csName = csName ;

      _catLatch.get() ;
      requestID = ++_requestID ;
      _mapSyncCSEvent[ requestID ] = item ;
      _catLatch.release() ;

      // send request
      rc = _sendCSInfoReq( csName, requestID, &(item->sendNums) ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to send cs info request, rc = %d", rc ) ;
         goto error ;
      }
      // wait for response
      rc = item->event.wait( waitMillSec, &result ) ;
      if ( SDB_OK == rc )
      {
         rc = result ;
      }

      // sanity chekc for result
      PD_RC_CHECK( rc, PDWARNING, "Get collection space pagesize failed, "
                   "rc: %d", rc ) ;

      pageSize = item->pageSize ;
      lobPageSize = item->lobPageSize ;

   done:
      _catLatch.get() ;
      _mapSyncCSEvent.erase( requestID ) ;
      _catLatch.release() ;

      if ( item )
      {
         SDB_OSS_DEL item ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsShardMgr::_onQueryCSInfoRsp( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      clsCSEventItem *csItem = NULL ;
      MAP_CS_EVENT_IT it ;
      BSONElement ele ;

      MsgOpReply *res = ( MsgOpReply* )msg ;

      PD_LOG ( PDDEBUG, "Recieve collecton space query response[requestID: "
               "%lld, flag: %d]", msg->requestID, res->flags ) ;

      INT32 flag = 0 ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector < BSONObj > objList ;

      ossScopedLock lock ( &_catLatch ) ;

      // based on the requestid, try to find the event
      // there's no async operation for querycs, so we must have request id here
      it = _mapSyncCSEvent.find( msg->requestID ) ;
      if ( it == _mapSyncCSEvent.end() )
      {
         // not found, timeout
         goto done ;
      }

      csItem = it->second ;

      if ( SDB_OK != res->flags )
      {
         rc = res->flags ;

         //not primary node, should send again
         if ( SDB_CLS_NOT_PRIMARY == res->flags )
         {
            --(csItem->sendNums) ;
            // if there's still pending nodes not received response
            // let's simply ignore not-primary error
            if ( csItem->sendNums > 0 )
            {
               // ignore
               rc = SDB_OK ;
               goto done ;
            }
            updateCatGroup ( TRUE ) ;
            // if we don't have any pending nodes, let's try it again
            // with sync request
            rc = _sendCSInfoReq( csItem->csName.c_str(),
                                 it->first,
                                 &(csItem->sendNums) ) ;
            if ( SDB_OK == rc )
            {
               goto done ;
            }
            PD_LOG( PDERROR, "Resend csinfo req to catalog failed, rc: %d",
                    rc ) ;
            goto error ;
         }
         else
         {
            PD_LOG ( PDERROR, "Query collection space[%s] info failed, rc: %d",
                     csItem->csName.c_str(), rc ) ;
            goto error ;
         }
      }
      else
      {
         // if the response is okay, let's extract reply
         rc = msgExtractReply ( (CHAR *)msg, &flag, &contextID, &startFrom,
                                &numReturned, objList ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         SDB_ASSERT ( numReturned == 1 && objList.size() == 1,
                      "Collection space item num must be 1" ) ;

         //signal collection info event
         ele = objList[0].getField ( CAT_PAGE_SIZE_NAME ) ;
         if ( ele.isNumber() )
         {
            csItem->pageSize = (UINT32)ele.numberInt() ;
         }
         else
         {
            csItem->pageSize = DMS_PAGE_SIZE_DFT ;
         }

         ele = objList[0].getField( CAT_LOB_PAGE_SZ_NAME ) ;
         if ( ele.isNumber() )
         {
            csItem->lobPageSize = (UINT32)ele.numberInt() ;
         }
         else
         {
            csItem->lobPageSize = DMS_DEFAULT_LOB_PAGE_SZ ;
         }
      }

      csItem->event.signalAll( rc ) ;

   done:
      return rc ;
   error:
      if ( csItem )
      {
         csItem->event.signalAll( rc ) ;
      }
      goto done ;
   }

   INT64 _clsShardMgr::netIn()
   {
      return _pNetRtAgent->netIn() ;
   }

   INT64 _clsShardMgr::netOut()
   {
      return _pNetRtAgent->netOut() ;
   }

   void _clsShardMgr::resetMon()
   {
      return _pNetRtAgent->resetMon() ;
   }

}

