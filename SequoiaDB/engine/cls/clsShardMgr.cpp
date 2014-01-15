/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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

   BEGIN_OBJ_MSG_MAP(_clsShardMgr, _clsObjBase)
      ON_MSG ( MSG_CAT_CATGRP_RES, _onCatCatGroupRes )
      ON_MSG ( MSG_CAT_NODEGRP_RES, _onCatGroupRes )
      ON_MSG ( MSG_CAT_QUERY_CATALOG_RSP, _onCatalogReqMsg )
      ON_MSG ( MSG_CAT_QUERY_SPACEINFO_RSP, _onQueryCSInfoRsp )
   END_OBJ_MSG_MAP()

   _clsShardMgr::_clsShardMgr (  _netRouteAgent *rtAgent )
   {
      _pNetRtAgent = rtAgent ;
      _requestID = 0 ;
      _pCatAgent = NULL ;
      _pNodeMgrAgent = NULL ;

      _primary = -1 ;
      _catVerion = 0 ;
      _nodeID.value = 0 ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_DECONSTRUCTOR, "_clsShardMgr::~_clsShardMgr" )
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_INIT, "_clsShardMgr::initialize" )
   INT32 _clsShardMgr::initialize( )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_INIT );
      UINT32 index = 0 ;

      if ( !_pNetRtAgent )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( _vecCatlog.size() == 0 )
      {
         PD_LOG ( PDERROR, "Not config catalog info" ) ;
         rc = SDB_INVALIDARG ;
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

   INT32 _clsShardMgr::final ()
   {
      return SDB_OK ;
   }

   void _clsShardMgr::onTimer ( UINT32 timerID, UINT32 interval )
   {
   }

   NodeID _clsShardMgr::nodeID () const
   {
      return _nodeID ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SETCATINFO, "_clsShardMgr::setCatlogInfo" )
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SYNCSND, "_clsShardMgr::syncSend" )
   INT32 _clsShardMgr::syncSend( MsgHeader * msg, UINT32 groupID,
                                 BOOLEAN primary, MsgHeader **ppRecvMsg,
                                 INT64 millisec )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_SYNCSND );
      std::string hostName ;
      INT32 indexID = -1 ;
      UINT16 port = 0 ;
      INT32 rc = SDB_OK ;

      if ( CATALOG_GROUPID == groupID )
      {
         ossScopedLock lock ( &_shardLatch, SHARED ) ;
         indexID = _primary ;

         if ( !primary )
         {
            indexID = ossRand() % _vecCatlog.size() ;
         }

         if ( indexID < 0 || indexID >= (INT32)_vecCatlog.size() )
         {
            rc = SDB_CLS_NODE_NOT_EXIST ;
         }
         else
         {
            hostName = _vecCatlog[indexID].host ;
            rc = ossSocket::getPort( _vecCatlog[indexID].service.c_str(),
                                     port ) ;
         }
      }
      else
      {
         clsGroupItem *item = NULL ;
         rc = getAndLockGroupItem( groupID, &item ) ;
         if ( SDB_OK == rc )
         {
            if ( primary )
            {
               indexID = item->nodePos(
                  item->primary( MSG_ROUTE_SHARD_SERVCIE ).columns.nodeID ) ;
            }
            else
            {
               indexID = ossRand() % item->nodeCount() ;
            }

            if ( indexID < 0 )
            {
               rc = SDB_CLS_NODE_NOT_EXIST ;
            }
            else
            {
               MsgRouteID nodeID ;
               std::string serviceName ;
               rc = item->getNodeInfo( indexID, nodeID, hostName,
                                       serviceName, MSG_ROUTE_SHARD_SERVCIE ) ;
               if ( SDB_OK == rc )
               {
                  rc = ossSocket::getPort( serviceName.c_str(), port ) ;
               }
            }

            unlockGroupItem( item ) ;
         }
      }

      if ( SDB_CLS_NODE_NOT_EXIST == rc )
      {
         // need to update
         if ( CATALOG_GROUPID == groupID )
         {
            updateCatGroup( FALSE ) ;
         }
         else
         {
            syncUpdateGroupInfo( groupID ) ;
         }
      }

      PD_RC_CHECK( rc, PDERROR, "Find node failed, rc:%d", rc ) ;

      {
         UINT32 msgLength = 0 ;
         INT32 receivedLen = 0 ;
         INT32 sentLen = 0 ;
         CHAR* buff = NULL ;
         // use millisecond
         ossSocket tmpSocket ( hostName.c_str(), port, millisec ) ;
         rc = tmpSocket.initSocket() ;
         PD_RC_CHECK( rc, PDERROR, "Init socket %s:%d failed, rc:%d",
                      hostName.c_str(), port, rc ) ;

         rc = tmpSocket.connect() ;
         PD_RC_CHECK( rc, PDERROR, "Connect to %s:%d failed, rc:%d",
                      hostName.c_str(), port, rc ) ;

         // send msg
         rc = tmpSocket.send( (const CHAR *)msg, msg->messageLength,
                              sentLen,
                              millisec ) ;
         PD_RC_CHECK( rc, PDERROR, "Send messge to %s:%d failed, rc:%d",
                      hostName.c_str(), port, rc ) ;

         // recieve msg, do not loop and retry
         rc = tmpSocket.recv( (CHAR*)&msgLength, sizeof(INT32), receivedLen,
                              millisec ) ;
         PD_RC_CHECK( rc, PDERROR, "Recieve msg length failed, rc: %d", rc ) ;

         if ( msgLength < sizeof(INT32) )
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

         *ppRecvMsg = (MsgHeader*)buff ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_SYNCSND, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SND2CAT, "_clsShardMgr::sendToCatlog" )
   INT32 _clsShardMgr::sendToCatlog ( MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_SND2CAT );

      ossScopedLock lock ( &_shardLatch, SHARED ) ;
      INT32 tmpPrimary = _primary ;

      if ( !_pNetRtAgent || _vecCatlog.size() == 0 )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      if ( tmpPrimary >= 0 && tmpPrimary < (INT32)_vecCatlog.size () )
      {
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
            goto done ;
         }
      }

      //send to all catlog node
      {
         UINT32 index = 0 ;
         INT32 rc1 = SDB_OK ;
         rc = SDB_NET_SEND_ERR ;

         while ( index < _vecCatlog.size () )
         {
            rc1 = _pNetRtAgent->syncSend ( _vecCatlog[index].nodeID,
               (void*)msg ) ;
            if ( rc1 == SDB_OK )
            {
               rc = rc1 ;
            }
            else
            {
               PD_LOG ( PDWARNING, "Send message to catlog[%s:%s] failed[rc:%d]. "
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_UPDCATGRP, "_clsShardMgr::updateCatGroup" )
   INT32 _clsShardMgr::updateCatGroup ( BOOLEAN unsetPrimary )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_UPDCATGRP );
      if ( unsetPrimary )
      {
         _primary = -1 ;
      }

      MsgCatCatGroupReq req ;
      req.header.opCode = MSG_CAT_CATGRP_REQ ;
      req.id.value = 0 ;
      req.id.columns.groupID = CATALOG_GROUPID ;

      //send to a catalog
      UINT32 index = 0 ;
      INT32 rc = SDB_OK ;

      ossScopedLock lock ( &_shardLatch, SHARED ) ;

      while ( index < _vecCatlog.size () )
      {
         rc = _pNetRtAgent->syncSend ( _vecCatlog[index].nodeID,
            (void*)&req ) ;
         if ( SDB_OK == rc )
         {
            break ;
         }

         index++ ;
      }

      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_UPDCATGRP, rc );
      return rc ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_CLRALLDATA, "_clsShardMgr::clearAllData" )
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
      while ( it != csList.end() )
      {
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SYNCUPDCAT, "_clsShardMgr::syncUpdateCatalog" )
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
         goto error ;
      }

      _catLatch.get() ;

      pEventInfo = _findCatSyncEvent( pCollectionName, TRUE ) ;
      if ( !pEventInfo )
      {
         _catLatch.release () ;
         rc = SDB_OOM ;
         goto error ;
      }

      //First judge the request is send or not
      if ( FALSE == pEventInfo->send )
      {
         rc = _sendCatalogReq ( pCollectionName ) ;
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

         if ( 0 == pEventInfo->waitNum )
         {
            pEventInfo->reSend = FALSE ;
            pEventInfo->event.reset () ;

            //release the event info
            SDB_OSS_DEL pEventInfo ;
            pEventInfo = NULL ;
            _mapSyncCatEvent.erase ( pCollectionName ) ;
         }

         _catLatch.release () ;
      }

      if ( SDB_OK == rc )
      {
         _clsCatalogSet *pClSet = NULL;
         std::set< std::string > subCLLst;
         _pCatAgent->lock_r();
         pClSet = _pCatAgent->collectionSet( pCollectionName );
         if ( NULL == pClSet )
         {
            _pCatAgent->release_r();
            rc = SDB_CLS_NO_CATALOG_INFO;
            goto error;
         }
         pClSet->getSubCLList( subCLLst );
         _pCatAgent->release_r();
         {
         // update all sub-collection catalog info               
         std::set< std::string >::iterator iterLst = subCLLst.begin();
         while( SDB_OK == rc && iterLst != subCLLst.end() )
         {
            rc = syncUpdateCatalog( iterLst->c_str(), millsec );
            ++iterLst;
         }
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_SYNCUPDCAT, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_SYNCUPDGPINFO, "_clsShardMgr::syncUpdateGroupInfo" )
   INT32 _clsShardMgr::syncUpdateGroupInfo ( UINT32 groupID, INT64 millsec )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_SYNCUPDGPINFO );
      BOOLEAN send = FALSE ;
      clsEventItem *pEventInfo = NULL ;

      _catLatch.get() ;

      pEventInfo = _findNMSyncEvent( groupID, TRUE ) ;
      if ( !pEventInfo )
      {
         _catLatch.release () ;
         rc = SDB_OOM ;
         goto error ;
      }

      //First judge the request is send or not
      if ( FALSE == pEventInfo->send )
      {
         rc = _sendGroupReq( groupID ) ;
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

         if ( 0 == pEventInfo->waitNum )
         {
            pEventInfo->reSend = FALSE ;
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_UPDPRM, "_clsShardMgr::updatePrimary" )
   INT32 _clsShardMgr::updatePrimary ( const NodeID & id, BOOLEAN primary )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_UPDPRM );
      UINT32 index = 0 ;

      ossScopedLock lock ( &_shardLatch, SHARED ) ;

      if ( _primary == -1 && FALSE == primary )
      {
         goto done ;
      }

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
      PD_LOG ( PDINFO, "Catlog primary node to [%s] id error[%u:%u:%u]",
               primary ? "primary" : "slave",
               id.columns.groupID,
               id.columns.nodeID,
               id.columns.serviceID ) ;

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR_UPDPRM, rc );
      return rc ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__SNDGPREQ, "_clsShardMgr::_sendGroupReq" )
   INT32 _clsShardMgr::_sendGroupReq ( UINT32 groupID, UINT64 requestID )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__SNDGPREQ );
      _MsgCatGroupReq msg ;
      msg.header.opCode = MSG_CAT_NODEGRP_REQ ;

      if ( 0 == requestID )
      {
         requestID = ++_requestID ;
      }
      msg.header.requestID = requestID ;
      msg.id.columns.groupID = groupID ;

      INT32 rc = sendToCatlog( (MsgHeader *)&msg ) ;

      PD_LOG ( PDDEBUG, "send group req[id: %d, requestID: %lld, rc: %d]",
               groupID, _requestID, rc ) ;
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__SNDGPREQ, rc );
      return rc ;      
   }

   INT32 _clsShardMgr::_sendCataQueryReq( INT32 queryType,
                                          const BSONObj & query,
                                          UINT64 requestID )
   {
      INT32 rc = SDB_OK ;
      CHAR *pBuffer = NULL ;
      INT32 buffSize = 0 ;
      MsgHeader * msg = NULL ;

      if ( 0 == requestID )
      {
         requestID = ++_requestID ;
      }

      rc = msgBuildQueryMsg ( &pBuffer, &buffSize, "CAT", 0, requestID, 0,
                              -1, &query, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      msg = (MsgHeader *) pBuffer ;
      msg->opCode = queryType ;
      msg->TID = 0 ;
      msg->routeID.value = 0 ;
      //send message
      rc = sendToCatlog ( msg ) ;

   done:
      if ( pBuffer )
      {
         SDB_OSS_FREE ( pBuffer ) ;
         pBuffer = NULL ;
      }
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__SNDCATREQ, "_clsShardMgr::_sendCatalogReq" )
   INT32 _clsShardMgr::_sendCatalogReq ( const CHAR *pCollectionName,
                                         UINT64 requestID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__SNDCATREQ );
      BSONObj query ;
      if ( !pCollectionName )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

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

      rc = _sendCataQueryReq( MSG_CAT_QUERY_CATALOG_REQ, query, requestID ) ;
      if ( SDB_OK != rc )
      {
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

   INT32 _clsShardMgr::_sendCSInfoReq(const CHAR * pCSName, UINT64 requestID )
   {
      INT32 rc = SDB_OK ;
      BSONObj query ;
      if ( !pCSName )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

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

      rc = _sendCataQueryReq( MSG_CAT_QUERY_SPACEINFO_REQ, query, requestID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDDEBUG, "send collection space req[name: %s, requestID: "
                  "%lld, rc: %d]", pCSName, requestID, rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__SNDCATREQ, rc );
      return rc ;
   error:
      goto done ;
   }

   //message fuctions
   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__ONCATGPRES, "_clsShardMgr::_onCatCatGroupRes" )
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

      if ( SDB_OK != res->header.res )
      {
         PD_LOG ( PDERROR, "Update catalog group info failed[rc: %d]",
                  res->header.res ) ;
         rc = res->header.res ;
         goto error ;
      }

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

         VECCATLOG oldCatNodes = _vecCatlog ;
         NodeID oldID ;

         _catVerion = version ;
         _vecCatlog.clear() ;
         map<UINT64, _netRouteNode>::iterator it = mapNodes.begin() ;
         while ( it != mapNodes.end() )
         {
            setCatlogInfo ( it->second._id, it->second._host,
                            it->second._service[MSG_ROUTE_CAT_SERVICE] ) ;
            ++it ;
         }

         UINT32 index = 0 ;
         while ( index < _vecCatlog.size() )
         {
            if ( SDB_OK == _findCatNodeID ( oldCatNodes, _vecCatlog[index].host,
                                            _vecCatlog[index].service, oldID ) )
            {
               if ( oldID.value != _vecCatlog[index].nodeID.value )
               {
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
               _pNetRtAgent->updateRoute ( _vecCatlog[index].nodeID,
                                           _vecCatlog[index].host.c_str(), 
                                           _vecCatlog[index].service.c_str() ) ;
               PD_LOG ( PDDEBUG, "Update catalog node[%u:%u] to %s:%s",
                        _vecCatlog[index].nodeID.columns.groupID,
                        _vecCatlog[index].nodeID.columns.nodeID,
                        _vecCatlog[index].host.c_str(),
                        _vecCatlog[index].service.c_str() ) ;
            }
            index++ ;
         }

         _shardLatch.release () ;
      }

      // if catalog group, get the primary from repl
      if ( CATALOG_GROUPID == nodeID().columns.groupID )
      {
         replCB *pRepl = pmdGetKRCB()->getReplCB() ;
         primary = pRepl->getPrimary().columns.nodeID ;
      }

      //update primary
      if ( primary != 0 )
      {
         primaryNode.columns.serviceID = MSG_ROUTE_CAT_SERVICE ;
         primaryNode.columns.nodeID = primary ;
         rc = updatePrimary ( primaryNode, TRUE ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSSHDMGR__ONCATGPRES, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__ONCATGRPRES, "_clsShardMgr::_onCatGroupRes" )
   INT32 _clsShardMgr::_onCatGroupRes ( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__ONCATGRPRES );
      MsgCatGroupRes *res = ( MsgCatGroupRes* )msg ;
      clsEventItem *pEventInfo = NULL ;

      PD_LOG ( PDDEBUG, "Recieve catalog group res[requestID:%lld,flag:%d]",
               msg->requestID, res->header.res ) ;

      ossScopedLock lock ( &_catLatch ) ;

      pEventInfo = _findNMSyncEvent( msg->requestID ) ;

      if ( SDB_OK != res->header.res )
      {
         rc = res->header.res ;
         if ( pEventInfo )
         {
            if ( SDB_CLS_GRP_NOT_EXIST == res->header.res ||
                 SDB_DMS_EOC == res->header.res )
            {
               _pNodeMgrAgent->lock_w() ;
               _pNodeMgrAgent->clearGroup( pEventInfo->groupID ) ;
               _pNodeMgrAgent->release_w() ;
               pEventInfo->event.signalAll( SDB_CLS_GRP_NOT_EXIST ) ;
            }
            else if ( SDB_CLS_NOT_PRIMARY == res->header.res )
            {
               updateCatGroup( TRUE ) ;
               if ( TRUE == pEventInfo->reSend )
               {
                  ossSleep ( 200 ) ;
               }
               if ( SDB_OK == _sendGroupReq( pEventInfo->groupID,
                                             msg->requestID ) )
               {
                  pEventInfo->reSend = TRUE ;
               }
            }
            else
            {
               PD_LOG ( PDERROR, "Update group[%d] failed[rc:%d]",
                        pEventInfo->groupID, res->header.res ) ;
               pEventInfo->event.signalAll( res->header.res ) ;
            }
         }
      }
      else
      {
         _pNodeMgrAgent->lock_w() ;
         const CHAR* objdata = (const CHAR*)res + sizeof( MsgCatGroupRes ) ;
         UINT32 length = msg->messageLength - sizeof( MsgCatGroupRes ) ;
         UINT32 groupID = 0 ;

         rc = _pNodeMgrAgent->updateGroupInfo( objdata, length, &groupID ) ;

         PD_LOG ( PDEVENT, "Update group[groupID:%u, rc: %d]", groupID, rc ) ;

         //udpate node info to netAgent
         clsGroupItem* groupItem = NULL ;
         if ( SDB_OK == rc )
         {
            groupItem = _pNodeMgrAgent->groupItem( groupID ) ;

            if ( !pEventInfo )
            {
               pEventInfo = _findNMSyncEvent( groupID, FALSE ) ;
            }
         }
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

         //find the event
         if ( pEventInfo )
         {
            pEventInfo->event.signalAll( rc ) ;
         }
      }

      PD_TRACE_EXITRC (SDB__CLSSHDMGR__ONCATGRPRES, rc );
      return rc ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__ONCATREQMSG, "_clsShardMgr::_onCatalogReqMsg" )
   INT32 _clsShardMgr::_onCatalogReqMsg ( NET_HANDLE handle, MsgHeader* msg )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__ONCATREQMSG );
      MsgCatQueryCatRsp *res = ( MsgCatQueryCatRsp*)msg ;

      PD_LOG ( PDDEBUG, "Recieve catalog response[requestID: %lld, flag: %d]",
               msg->requestID, res->flags ) ;

      INT32 flag = 0 ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector < BSONObj > objList ;
      UINT32 groupID = nodeID().columns.groupID ;
      INT32 rc = SDB_OK ;
      clsEventItem *pEventInfo = NULL ;

      ossScopedLock lock ( &_catLatch ) ;

      if ( SDB_OK != res->flags )
      {
         rc = SDB_CLS_UPDATE_CAT_FAILED ;

         //need to found event info by request id
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
               updateCatGroup ( TRUE ) ;
               if ( TRUE == pEventInfo->reSend )
               {
                  ossSleep ( 200 ) ;
               }
               if ( SDB_OK == _sendCatalogReq ( pEventInfo->name.c_str(), 
                                                msg->requestID ) )
               {
                  pEventInfo->reSend = TRUE ;
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
         rc = msgExtractReply ( (CHAR *)msg, &flag, &contextID, &startFrom,
                                &numReturned, objList ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         SDB_ASSERT ( numReturned == 1 && objList.size() == 1,
                      "Collection catalog item num must be 1" )

         _pCatAgent->lock_w () ;
         rc = _pCatAgent->updateCatalog ( 0, groupID, objList[0].objdata(),
                                          objList[0].objsize() ) ;
         _pCatAgent->release_w () ;

         PD_LOG ( PDEVENT, "Update catalog [version:%u, rc: %d]", 0, rc ) ;

         //signal collection info event
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDCATSYNCEV, "_clsShardMgr::_findCatSyncEvent" )
   clsEventItem *_clsShardMgr::_findCatSyncEvent ( const CHAR * pCollectionName,
                                                   BOOLEAN bCreate )
   {
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR__FNDCATSYNCEV );
      SDB_ASSERT ( pCollectionName , "Collection name can't be NULL" )

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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDCATSYNCEVN, "_clsShardMgr::_findCatSyncEvent" )
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDNMSYNCEV, "_clsShardMgr::_findNMSyncEvent" )
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDNMSYNCEVN, "_clsShardMgr::_findNMSyncEvent" )
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR__FNDCATNODEID, "_clsShardMgr::_findCatNodeID" )
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

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_GETANDLOCKCATSET, "_clsShardMgr::getAndLockCataSet" )
   INT32 _clsShardMgr::getAndLockCataSet( const CHAR * name,
                                          clsCatalogSet **ppSet,
                                          BOOLEAN noWithUpdate,
                                          INT64 waitMillSec,
                                          BOOLEAN * pUpdated )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_GETANDLOCKCATSET );
      if ( !ppSet || !name )
      {
         rc = SDB_INVALIDARG ;
         goto done ;
      }

      while ( SDB_OK == rc )
      {
         _pCatAgent->lock_r() ;
         *ppSet = _pCatAgent->collectionSet( name ) ;
         if ( !(*ppSet) && noWithUpdate )
         {
            _pCatAgent->release_r() ;
            rc = syncUpdateCatalog( name, waitMillSec ) ;
            if ( pUpdated )
            {
               *pUpdated = TRUE ;
            }
            noWithUpdate = FALSE ;
            continue ;
         }

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
   }

   INT32 _clsShardMgr::unlockCataSet( clsCatalogSet * catSet )
   {
      if ( catSet )
      {
         _pCatAgent->release_r() ;
      }
      return SDB_OK ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSHDMGR_GETNLCKGPITEM, "_clsShardMgr::getAndLockGroupItem" )
   INT32 _clsShardMgr::getAndLockGroupItem( UINT32 id, clsGroupItem **ppItem,
                                            BOOLEAN noWithUpdate,
                                            INT64 waitMillSec,
                                            BOOLEAN * pUpdated )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSHDMGR_GETNLCKGPITEM );
      if ( !ppItem )
      {
         rc = SDB_INVALIDARG ;
         goto done ;
      }

      while ( SDB_OK == rc )
      {
         _pNodeMgrAgent->lock_r() ;
         *ppItem = _pNodeMgrAgent->groupItem( id ) ;
         if ( !(*ppItem) && noWithUpdate )
         {
            _pNodeMgrAgent->release_r() ;
            rc = syncUpdateGroupInfo( id, waitMillSec ) ;
            if ( pUpdated )
            {
               *pUpdated = TRUE ;
            }
            noWithUpdate = FALSE ;
            continue ;
         }

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
                                       INT64 waitMillSec )
   {
      INT32 rc = SDB_OK ;
      clsCSEventItem *item = NULL ;
      UINT64 requestID = 0 ;
      INT32 result = 0 ;

      if ( NULL == csName )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

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
      rc = _sendCSInfoReq( csName, requestID ) ;
      if ( rc )
      {
         goto error ;
      }

      rc = item->event.wait( waitMillSec, &result ) ;
      if ( SDB_OK == rc )
      {
         rc = result ;
      }

      PD_RC_CHECK( rc, PDWARNING, "Get collection space pagesize failed, "
                   "rc: %d", rc ) ;

      pageSize = item->pageSize ;

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
            updateCatGroup ( TRUE ) ;
            ossSleep ( 200 ) ;
            if ( 0 == csItem->pageSize &&
                 SDB_OK == _sendCSInfoReq( csItem->csName.c_str(),
                                           it->first ) )
            {
               rc = SDB_OK ;
               goto done ;
            }
         }

         PD_LOG ( PDERROR, "Query collection space[%s] info failed, rc: %d",
                  csItem->csName.c_str(), rc ) ;
         goto error ;
      }
      else
      {
         rc = msgExtractReply ( (CHAR *)msg, &flag, &contextID, &startFrom,
                                &numReturned, objList ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         SDB_ASSERT ( numReturned == 1 && objList.size() == 1,
                      "Collection space item num must be 1" )

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

}

