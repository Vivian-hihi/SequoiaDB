/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = seadptMgr.cpp

   Descriptive Name = Search engine adapter manager.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#include "pmd.hpp"
#include "sptCommon.hpp"
#include "seAdptMgr.hpp"
#include "seAdptAgentSession.hpp"
#include "seAdptIndexSession.hpp"
#include "seAdptDef.hpp"
#include "msgMessage.hpp"

#define DATA_NODE_GRP_ID                        10000
#define DATA_NODE_ID                            10000
#define SEADPT_NAME_CAPPED_COLLECTIONSPACE      "CappedCS"
#define SEADPT_NAME_CAPPED_COLLECTION           "CappedCL"
// Wait for at most 10 mins for register.
#define SDBADPT_REGIST_MAX_WAIT_TIME            600

namespace engine
{
   _seSvcSessionMgr::_seSvcSessionMgr( _seAdptCB *pAdptCB )
   {
   }

   INT32 _seSvcSessionMgr::handleSessionTimeout( UINT32 timerID, UINT32 interval )
   {
      return _pmdAsycSessionMgr::handleSessionTimeout( timerID, interval ) ;
   }

   UINT64 _seSvcSessionMgr::makeSessionID( const NET_HANDLE &handle,
                                        const MsgHeader *header )
   {
      return ossPack32To64( PMD_BASE_HANDLE_ID, header->TID ) ;
   }

   SDB_SESSION_TYPE _seSvcSessionMgr::_prepareCreate( UINT64 sessionID,
                                                      INT32 startType,
                                                      INT32 opCode )
   {
      return SDB_SESSION_SE_AGENT ;
   }

   pmdAsyncSession* _seSvcSessionMgr::_createSession( SDB_SESSION_TYPE sessionType,
                                                   INT32 startType,
                                                   UINT64 sessionID,
                                                   void *data )
   {
      pmdAsyncSession* session = NULL ;

      if ( SDB_SESSION_SE_AGENT == sessionType )
      {
         session = SDB_OSS_NEW seAdptAgentSession( sessionID ) ;
      }
      else
      {
         PD_LOG( PDERROR, "Invalid session type[%d]", sessionType ) ;
      }

      return session ;
   }

   _seIndexSessionMgr::_seIndexSessionMgr( _seAdptCB *pAdptCB )
   {
      _pAdptCB = pAdptCB ;
      _indexVersion = ossGetCurrentProcessID() ;
      _indexSessionTimer = NET_INVALID_TIMER_ID ;
      _innerSessionID = 0 ;
      _updateStep = SEADPT_IDX_UPDATE_BEGIN ;
   }

   _seIndexSessionMgr::~_seIndexSessionMgr()
   {

   }

   INT32 _seIndexSessionMgr::handleSessionTimeout( UINT32 timerID,
                                                   UINT32 interval )
   {
      INT32 rc = SDB_OK ;

      // Only when the data node is primary that we will start indexing jobs.
      if ( _indexSessionTimer == timerID && _pAdptCB->isDataNodePrimary() )
      {
         if ( SEADPT_IDX_UPDATE_BEGIN == _updateStep )
         {
            // Request text index information from SDB data node.
            rc = _pAdptCB->_sendIdxUpdateReq() ;
            PD_RC_CHECK( rc, PDERROR, "Update text index information failed[ %d ]",
                         rc ) ;
            _updateStep = SEADPT_IDX_UPDATE_QUERY ;
         }
         else if ( SEADPT_IDX_UPDATE_START_TASK == _updateStep )
         {
            _pAdptCB->_startInnerSession( SEADPT_SESSION_INDEX, this ) ;
            _updateStep = SEADPT_IDX_UPDATE_BEGIN ;
         }
      }

      // In the base class, the sessions which are waiting to quit will be
      // released.
      rc = _pmdAsycSessionMgr::handleSessionTimeout( timerID, interval ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seIndexSessionMgr::handleSessionClose( const NET_HANDLE handle )
   {
      return SDB_OK ;
   }

   UINT64 _seIndexSessionMgr::makeSessionID( const NET_HANDLE &handle,
                                             const MsgHeader *header )
   {
      return ossPack32To64( PMD_BASE_HANDLE_ID, header->TID ) ;
   }

   void _seIndexSessionMgr::onSessionDestoryed( pmdAsyncSession *pSession )
   {
      // Session is bind with index task. When the session exits, tell the
      // manager to remove the binding information.
   }

   BOOLEAN _seIndexSessionMgr::isIndexTimerStarted() const
   {
      return NET_INVALID_TIMER_ID == _indexSessionTimer ? FALSE : TRUE ;
   }

   void _seIndexSessionMgr::startIndexTimer( UINT32 interval )
   {
      if ( _pRTAgent && _pTimerHandle && !isIndexTimerStarted() )
      {
         _pRTAgent->addTimer( interval, _pTimerHandle, _indexSessionTimer ) ;
      }
   }

   void _seIndexSessionMgr::stopIndexTimer()
   {
      if ( _pRTAgent && _pTimerHandle && isIndexTimerStarted() )
      {
         _pRTAgent->removeTimer( _indexSessionTimer ) ;
         _indexSessionTimer = NET_INVALID_TIMER_ID ;
      }
   }

   INT32 _seIndexSessionMgr::updateIndexInfo( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      INT64 peerVersion = -1 ;

      ele = obj.getField( FIELD_NAME_VERSION ) ;
      if ( EOO == ele.type() )
      {
         PD_LOG( PDERROR, "Get peer text index version failed" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      else
      {
         peerVersion = ele.numberLong() ;
         if ( peerVersion != _indexVersion )
         {
            BSONElement idxEles = obj.getField( FIELD_NAME_INDEXES ) ;
            if ( EOO == idxEles.type() )
            {
               PD_LOG( PDERROR, "No index information found" ) ;
               rc = SDB_SYS ;
               goto error ;
            }

            {
               list<seIndexTask> newFullList ;
               vector<BSONElement> idxElements = idxEles.Array() ;

               _newTaskList.clear() ;

               for ( vector<BSONElement>::iterator itr = idxElements.begin();
                     itr != idxElements.end(); ++itr )
               {
                  BOOLEAN foundIdx = FALSE ;
                  const CHAR *csName =
                     itr->Obj().getStringField( FIELD_NAME_COLLECTIONSPACE ) ;
                  const CHAR *clName =
                     itr->Obj().getStringField( FIELD_NAME_COLLECTION ) ;
                  const CHAR *cappedCSName =
                     itr->Obj().getStringField( SEADPT_NAME_CAPPED_COLLECTIONSPACE ) ;
                  const CHAR *cappedCLName =
                     itr->Obj().getStringField( SEADPT_NAME_CAPPED_COLLECTION ) ;
                  BSONObj idxDef =
                     itr->Obj().getObjectField( FIELD_NAME_INDEX ) ;
                  BSONObj key = idxDef.getObjectField( IXM_FIELD_NAME_KEY ) ;
                  const CHAR *idxName =
                        idxDef.getStringField( IXM_FIELD_NAME_NAME ) ;
                  string esIdxName = string(csName) + string(clName) +
                                     string(idxName) ;
                  CHAR esTypeName[11] = {0} ;
                  ossItoa( _pAdptCB->getDataNodeGID(), esTypeName, 10 ) ;

                  seIndexTask newTask( csName, clName, idxName, cappedCSName,
                                       cappedCLName, esIdxName,
                                       esTypeName, key ) ;
                  newFullList.push_back( newTask ) ;

                  // Loop through the index information list, to check if this
                  // index is already there.
                  for ( list<seIndexTask>::iterator itr = _taskList.begin();
                        itr != _taskList.end(); ++itr )
                  {
                     // Check if it's in the current task list. If not, start
                     // a new task.
                     if ( csName == itr->_origCSName &&
                          clName == itr->_origCLName &&
                          idxName == itr->_origIdxName &&
                          key == itr->_indexDef )
                     {
                        // _newTaskList.push_back( newTask ) ;
                        foundIdx = TRUE ;
                        break ;
                     }
                  }

                  if ( !foundIdx )
                  {
                     _newTaskList.push_back( newTask ) ;
                     PD_LOG( PDEVENT, "Add new task into queue: %s",
                             newTask.toString().c_str() ) ;
                  }

                  // For new indices, start new indexing task, and then replace
                  // the whole old index information.
               }
               _taskList.clear() ;
               _taskList = newFullList ;
            }

            for ( list<seIndexTask>::iterator itr = _newTaskList.begin() ;
                  itr != _newTaskList.end(); ++itr )
            {
               _pAdptCB->startInnerSession( SEADPT_SESSION_INDEX,
                                            _getInnerSessionID(),
                                            (void *)(&*itr) ) ;
            }

            PD_LOG( PDDEBUG, "Change local version from [ %d ] to [ %d ]",
                    _indexVersion, peerVersion ) ;
            _indexVersion = peerVersion ;
         }
         else
         {
            PD_LOG( PDDEBUG, "Text index version are the same[%lld], no need "
                    "for updating", _indexVersion ) ;
         }
      }

      _updateStep = SEADPT_IDX_UPDATE_START_TASK ;
   done:
      return rc ;
   error:
      goto done ;
   }

   SDB_SESSION_TYPE _seIndexSessionMgr::_prepareCreate( UINT64 sessionID,
                                                        INT32 startType,
                                                        INT32 opCode )
   {
      return SDB_SESSION_SE_INDEX ;
   }

   BOOLEAN _seIndexSessionMgr::_canReuse( SDB_SESSION_TYPE sessionType )
   {
      return FALSE ;
   }

   UINT32 _seIndexSessionMgr::_maxCacheSize() const
   {
      return 0 ;
   }
   void _seIndexSessionMgr::_onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                              const NET_HANDLE &handle,
                                              pmdAsyncSession *pSession )
   {
      // TODO: change
      _reply( handle, rc, pReq ) ;
   }

   pmdAsyncSession* _seIndexSessionMgr::_createSession( SDB_SESSION_TYPE sessionType,
                                                        INT32 startType,
                                                        UINT64 sessionID,
                                                        void *data )
   {
      pmdAsyncSession *pSession = NULL ;

      if ( SDB_SESSION_SE_INDEX == sessionType )
      {
         seIndexTask *task = (seIndexTask *)data ;
         pSession =
            SDB_OSS_NEW seAdptIndexSession( sessionID, _pRTAgent, task ) ;
      }
      else
      {
         PD_LOG( PDERROR, "Invalid session type[%d]", sessionType ) ;
      }

      return pSession ;
   }

   BEGIN_OBJ_MSG_MAP( _seAdptCB, _pmdObjBase )
      ON_MSG( MSG_AUTH_VERIFY_RES, _onRegisterRes )
      ON_MSG( MSG_SEADPT_UPDATE_IDXINFO_RES, _onIdxUpdateRes )
   END_OBJ_MSG_MAP()

   _seAdptCB::_seAdptCB()
   : _indexMsgHandler( &_idxSessionMgr ),
     _svcMsgHandler( &_svcSessionMgr ),
     _indexTimerHandler( &_idxSessionMgr ),
     _svcTimerHandler( &_svcSessionMgr ),
     _indexNetRtAgent( &_indexMsgHandler ),
     _svcRtAgent( &_svcMsgHandler ),
     _idxSessionMgr( this ),
     _svcSessionMgr( this )
   {
      ossMemset( _serviceName, 0, OSS_MAX_SERVICENAME + 1 ) ;
      _oneSecTimer = NET_INVALID_TIMER_ID ;
      _dataNodeID.value = MSG_INVALID_ROUTEID ;
      _cataNodeID.value = MSG_INVALID_ROUTEID ;
      _selfRouteID.value = MSG_INVALID_ROUTEID ;
      _peerPrimary = FALSE ;
      _peerGroupID = 0 ;
   }

   _seAdptCB::~_seAdptCB()
   {
      // TODO
   }

   SDB_CB_TYPE _seAdptCB::cbType() const
   {
      return SDB_CB_SEADAPTER ;
   }

   const CHAR* _seAdptCB::cbName() const
   {
      return "SEADAPTER" ;
   }

   INT32 _seAdptCB::init()
   {
      INT32 rc = SDB_OK ;
      MsgRouteID svcRtID = _selfRouteID ;
      std::string seSvcPath ;
      const CHAR *hostName = pmdGetKRCB()->getHostName() ;

      // register config handler to se adapter options.
      _options.setConfigHandler( pmdGetKRCB() ) ;

      // Create listener socket. This is for searching and command processing.
      svcRtID.columns.serviceID = MSG_ROUTE_SE_SERVICE ;
      _svcRtAgent.updateRoute( svcRtID, hostName, _options.getSvcName() ) ;
      rc = _svcRtAgent.listen( svcRtID ) ;
      PD_RC_CHECK( rc, PDERROR, "Create listener for hostname[ %s ] and "
                   "service[ %s ] failed[ %d ]",
                   hostName, _options.getSvcName(), rc ) ;
      PD_LOG( PDEVENT, "Create search engine adapter listener[ServiceName: %s] "
              "successfully", _options.getSvcName() ) ;

      // Init sdb data node address.
      rc = _initSdbAddr() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init sdb data node address failed[ %d ]", rc ) ;
         goto error ;
      }

      rc = _initSearchEngineAddr() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init search engine address failed[ %d ]", rc ) ;
         goto error ;
      }

      rc = _idxSessionMgr.init( &_indexNetRtAgent, &_indexTimerHandler,
                                5 * OSS_ONE_SEC ) ;
      PD_RC_CHECK( rc, PDERROR, "Init index session manager failed[ %d ]",
                   rc ) ;

      rc = _svcSessionMgr.init( &_svcRtAgent, &_svcTimerHandler,
                                60 * OSS_ONE_SEC ) ;
      PD_RC_CHECK( rc, PDERROR, "Init service session manager failed[ %d ]",
                   rc ) ;

      // Initialize search engine client manager.
      seSvcPath = std::string( _options.getSeHost() ) + ":"
                  + std::string( _options.getSeService() ) ;
      rc = _seCltMgr.init( seSvcPath ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init search engine client manager failed[ %d ]",
                 rc ) ;
         goto error ;
      }
      PD_LOG( PDEVENT, "Search engine client manager init successfully" ) ;

      _textIdxVersion = ossGetCurrentProcessID() ;

      // Set the business status to not OK. Change to OK after successfully
      // registered on data node.
      pmdGetKRCB()->setBusinessOK( FALSE ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::active()
   {
      #define SEADPT_WAIT_CB_ATTACH_TIMEOUT             ( 300 * OSS_ONE_SEC )

      INT32 rc = SDB_OK ;
      EDUID eduID = PMD_INVALID_EDUID ;
      pmdEDUMgr *pEDUMgr = pmdGetKRCB()->getEDUMgr() ;
      INT32 registWaitTime = 0 ;

      // 1. start se adapter manager edu.
      rc = pEDUMgr->startEDU( EDU_TYPE_SEADPTMGR, (_pmdObjBase*)this, &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start search engine adapter manager "
                   "edu, rc: %d", rc ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_SEADPTMGR, eduID ) ;
      rc = _attachEvent.wait( SEADPT_WAIT_CB_ATTACH_TIMEOUT ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait search engine adapter manager edu attach "
                   "failed, rc: %d", rc ) ;

      // 2. start net edu between adapter and sdb node.
      rc = pEDUMgr->startEDU( EDU_TYPE_SE_INDEXR, &_indexNetRtAgent, &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start search engine adapter net, "
                   "rc: %d", rc ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_SE_INDEXR, eduID ) ;

      // 3. start se adapter service.
      rc = pEDUMgr->startEDU( EDU_TYPE_SE_SERVICE, (void *)&_svcRtAgent,
                              &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Start service listener failed[ %d ]", rc ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_SE_SERVICE, eduID ) ;
      rc = pEDUMgr->waitUntil( eduID, PMD_EDU_RUNNING ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait service listener active failed[ %d ]",
                   rc ) ;

      _idxSessionMgr.startIndexTimer( 5 * OSS_ONE_SEC ) ;

      // 4. Register on data node. Try at most 10 mins before exiting.
      while ( PMD_IS_DB_UP() && registWaitTime < SDBADPT_REGIST_MAX_WAIT_TIME )
      {
         rc = _sendRegisterMsg() ;
         registWaitTime++ ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Send register message to sdb data node "
                    "failed[ %d ]. Try times[ %d ]. Max try time[ %d ]",
                    rc, registWaitTime, SDBADPT_REGIST_MAX_WAIT_TIME ) ;
         }
         else
         {
            break ;
         }
         ossSleepsecs( 1 ) ;
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

   INT32 _seAdptCB::deactive()
   {
      return SDB_OK ;
   }

   INT32 _seAdptCB::fini()
   {
      return SDB_OK ;
   }

   void _seAdptCB::attachCB( _pmdEDUCB *cb )
   {
      if ( EDU_TYPE_SEADPTMGR == cb->getType() )
      {
         _svcMsgHandler.attach( cb ) ;
         _indexMsgHandler.attach( cb ) ;
         _svcTimerHandler.attach( cb ) ;
         _indexTimerHandler.attach( cb ) ;
      }
      else if ( EDU_TYPE_SE_SERVICE == cb->getType() )
      {
         _svcMsgHandler.attach( cb ) ;
      }
      else if ( EDU_TYPE_SE_INDEX == cb->getType() )
      {
         _indexMsgHandler.attach( cb ) ;
         _indexTimerHandler.attach( cb ) ;
      }
      _attachEvent.signalAll() ;
   }

   void _seAdptCB::detachCB( _pmdEDUCB *cb )
   {
      if ( EDU_TYPE_SEADPTMGR == cb->getType() )
      {
         _svcMsgHandler.detach() ;
         _indexMsgHandler.detach() ;
         _svcTimerHandler.detach() ;
         _indexTimerHandler.detach() ;
      }
      else if ( EDU_TYPE_SE_SERVICE == cb->getType() )
      {
         _svcMsgHandler.detach() ;
      }
      else if ( EDU_TYPE_SE_INDEX == cb->getType() )
      {
         _indexMsgHandler.detach() ;
         _indexTimerHandler.detach() ;
      }
   }

   void _seAdptCB::onTimer( UINT64 timerID, UINT32 interval )
   {
      return ;
   }

   seAdptOptionsMgr* _seAdptCB::getOptions()
   {
      return &_options ;
   }

   utilESCltMgr* _seAdptCB::getSeCltMgr()
   {
      return &_seCltMgr ;
   }

   seSvcSessionMgr* _seAdptCB::getSeAgentMgr()
   {
      return &_svcSessionMgr ;
   }

   seIndexSessionMgr* _seAdptCB::getIdxSessionMgr()
   {
      return &_idxSessionMgr ;
   }

   netRouteAgent* _seAdptCB::getIdxRouteAgent()
   {
      return &_indexNetRtAgent ;
   }

   INT32 _seAdptCB::startInnerSession( INT32 type, INT32 innerTID,
                                       void *data )
   {
      ossScopedLock lock ( &_seLatch, EXCLUSIVE ) ;

      seAdptSessionInfo info ;
      info.type = type ;
      info.startType = PMD_SESSION_ACTIVE ;
      info.innerTid = innerTID ;
      info.data = data ;

      // Rule should be the same with makeSessionID.
      info.sessionID = ossPack32To64 ( PMD_BASE_HANDLE_ID, innerTID ) ;

      _vecInnerSessionParam.push_back ( info ) ;

      return SDB_OK ;
   }

   INT32 _seAdptCB::sendToDataNode( MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;

      rc = _indexNetRtAgent.syncSend( _dataNodeID, (void *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send message to data node failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::sendToCataNode( MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;

      rc = _indexNetRtAgent.syncSend( _cataNodeID, (void *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send message to cata node failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_initSdbAddr()
   {
      // Add the route information of sdb node.
      INT32 rc = SDB_OK ;
      UINT16 port = 0 ;

      _dataNodeID.columns.groupID = DATA_NODE_GRP_ID ;
      _dataNodeID.columns.nodeID = DATA_NODE_ID ;

      // capped collection from shard flat.
      _dataNodeID.columns.serviceID = MSG_ROUTE_SHARD_SERVCIE ;
      ossSocket::getPort( _options.getDbService(), port ) ;
      port += MSG_ROUTE_SHARD_SERVCIE ;

      std::stringstream portStr ;
      portStr << port ;

      rc = _indexNetRtAgent.updateRoute( _dataNodeID, _options.getDbHost(),
                                         portStr.str().c_str() ) ;
      if ( rc && SDB_NET_UPDATE_EXISTING_NODE != rc )
      {
         PD_LOG( PDERROR, "Update route failed[ %d ]", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_initSearchEngineAddr()
   {
      INT32 rc = SDB_OK ;
      MsgRouteID nodeID ;

      nodeID.columns.groupID = INVALID_GROUPID ;
      nodeID.columns.nodeID = INVALID_NODEID ;
      nodeID.columns.serviceID = 0 ;

      rc = _indexNetRtAgent.updateRoute( nodeID, _options.getSeHost(),
                                         _options.getSeService() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to update route, rc: %d",
                 rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   // Register on the data node. Use the auth msg.
   INT32 _seAdptCB::_sendRegisterMsg()
   {
      INT32 rc = SDB_OK ;
      BSONObj authObj ;
      INT32 msgLen = 0 ;
      MsgAuthentication *authMsg = NULL ;

      // Should send the route information to data node. Then data node is able
      // to send request and reply.
      authObj = BSON( FIELD_NAME_HOST << pmdGetKRCB()->getHostName()
                      << FIELD_NAME_SERVICE_NAME << _options.getSvcName()
                      << FIELD_NAME_SERVICE_TYPE << MSG_ROUTE_SE_SERVICE  ) ;

      msgLen = sizeof( MsgAuthentication ) +
               ossRoundUpToMultipleX( authObj.objsize(), 4 ) ;

      authMsg = ( MsgAuthentication * )SDB_OSS_MALLOC( msgLen ) ;
      if ( !authMsg )
      {
         PD_LOG( PDERROR, "Allocate memory of size[ %d ] for message failed",
                 msgLen ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      authMsg->header.requestID = 0 ;
      authMsg->header.opCode = MSG_AUTH_VERIFY_REQ ;
      authMsg->header.messageLength = sizeof( MsgAuthentication ) +
                                      authObj.objsize() ;
      authMsg->header.routeID.value = 0 ;
      // Send to the main thread of shard.
      authMsg->header.TID = 0 ;
      ossMemcpy( (CHAR *)authMsg + sizeof( MsgAuthentication ),
                 authObj.objdata(), authObj.objsize() ) ;

      rc = sendToDataNode( (MsgHeader *)authMsg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send auth request to data node failed[ %d ]",
                   rc ) ;
      PD_LOG( PDDEBUG, "Send register message to data node successfully. "
              "Information: %s", authObj.toString().c_str() ) ;

   done:
      if ( authMsg )
      {
         SDB_OSS_FREE( authMsg ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_onRegisterRes( NET_HANDLE handle, MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> objVec ;
      BSONObj cataInfoObj ;
      string cataHost ;
      string cataSvc ;

      // Get the role, group id of the node.
      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, objVec ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract register reply failed[ %d ]", rc ) ;
      if ( 1 != objVec.size() )
      {
         PD_LOG( PDERROR, "Register reply is not as expected" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _peerPrimary = objVec[0].getBoolField( FIELD_NAME_IS_PRIMARY ) ;
      _peerGroupID = objVec[0].getIntField( FIELD_NAME_GROUPID ) ;

      cataInfoObj = objVec[0].getObjectField( FIELD_NAME_CATALOGINFO ) ;
      if ( cataInfoObj.isEmpty() )
      {
         PD_LOG( PDERROR, "Find catalog info in object failed" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      cataHost = cataInfoObj.getStringField( FIELD_NAME_HOST ) ;
      cataSvc = cataInfoObj.getStringField( FIELD_NAME_SERVICE_NAME ) ;
      _cataNodeID.columns.groupID = cataInfoObj.getIntField( FIELD_NAME_GROUPID ) ;
      _cataNodeID.columns.nodeID = cataInfoObj.getIntField( FIELD_NAME_NODEID ) ;
      _cataNodeID.columns.serviceID = cataInfoObj.getIntField( FIELD_NAME_SERVICE ) ;

      rc = _indexNetRtAgent.updateRoute( _cataNodeID, cataHost.c_str(),
                                         cataSvc.c_str() ) ;
      PD_RC_CHECK( rc, PDERROR, "Update route failed[ %d ]" ) ;

      pmdGetKRCB()->setBusinessOK( TRUE ) ;

      PD_LOG( PDEVENT, "Search engine adapter registered on SequoiaDB data "
              "node successfully" ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_startEDU( INT32 type, EDU_STATUS waitStatus,
                                  void *agrs, BOOLEAN regSys )
   {
      INT32 rc = SDB_OK ;
      EDUID eduID = PMD_INVALID_EDUID ;
      pmdKRCB *pKRCB = pmdGetKRCB () ;
      pmdEDUMgr *pEDUMgr = pKRCB->getEDUMgr () ;

      //Start EDU
      rc = pEDUMgr->startEDU( (EDU_TYPES)type, (void *)agrs, &eduID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to create EDU[type:%d(%s)], rc = %d",
                  type, getEDUName( (EDU_TYPES)type ), rc );
         goto error ;
      }

      //Resiter EDU Type
      if ( regSys )
      {
         pEDUMgr->regSystemEDU( (EDU_TYPES)type, eduID ) ;
      }

      //Wait edu running
      if ( PMD_EDU_UNKNOW != waitStatus )
      {
         rc = pEDUMgr->waitUntil( (EDU_TYPES)type, waitStatus ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to wait EDU[type:%d(%s)] to "
                    "status[%d(%s)], rc: %d", type,
                    getEDUName( (EDU_TYPES)type ), waitStatus,
                    getEDUStatusDesp( waitStatus ), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_startInnerSession( INT32 type,
                                        pmdAsycSessionMgr *pSessionMgr )
   {
      INT32 rc = SDB_OK ;
      _pmdAsyncSession *pSession = NULL ;
      ossScopedLock lock( &_seLatch, EXCLUSIVE ) ;

      VECINNERPARAM::iterator itr = _vecInnerSessionParam.begin() ;
      while ( itr != _vecInnerSessionParam.end() )
      {
         seAdptSessionInfo &info = *itr ;
         if ( info.type != type || pSessionMgr->getSession( info.sessionID,
                                                            info.startType,
                                                            NET_INVALID_HANDLE,
                                                            FALSE, 0, NULL ) )
         {
            ++itr ;
            continue ;
         }

         pSession = pSessionMgr->getSession( info.sessionID, info.startType,
                                             NET_INVALID_HANDLE, TRUE, 0,
                                             info.data ) ;
         if ( pSession )
         {
            PD_LOG( PDEVENT, "Create inner session[%s] succeed",
                    pSession->sessionName() ) ;
            itr = _vecInnerSessionParam.erase( itr ) ;
            continue ;
         }

         PD_LOG( PDERROR, "Create inner session[TID:%d] failed",
                 info.innerTid ) ;
         rc = SDB_SYS ;
         ++itr ;
      }

      return rc ;
   }

   // Communicate with SDB data node to ensure the local text index information
   // is fresh. If not, get it from SDB and refresh local copy.
   INT32 _seAdptCB::_sendIdxUpdateReq()
   {
      INT32 rc = SDB_OK ;

      // Send index query request to data node. The local index information
      // version number will be passed.
      BSONObj msgBody ;
      MsgOpMsg *msg = NULL ;
      INT32 msgLen = 0 ;

      msgBody =
         BSON( FIELD_NAME_VERSION << _idxSessionMgr.getIndexVersion() ) ;

      msgLen = sizeof( MsgHeader ) +
               ossRoundUpToMultipleX( msgBody.objsize(), 4 ) ;

      msg = ( MsgOpMsg * )SDB_OSS_MALLOC( msgLen ) ;
      if ( !msg )
      {
         PD_LOG( PDERROR, "Allocate memory of size[ %d ] for message failed",
                 msgLen ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      msg->header.opCode = MSG_SEADPT_UPDATE_IDXINFO_REQ ;
      msg->header.messageLength = sizeof( MsgHeader ) + msgBody.objsize() ;
      msg->header.routeID.value = 0 ;
      msg->header.TID = 0 ;
      msg->header.requestID = 0 ;
      ossMemcpy( (CHAR *)msg + sizeof( MsgHeader ),
                 msgBody.objdata(), msgBody.objsize() ) ;

      rc = sendToDataNode( (MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send message to data node failed[ %d ]", rc ) ;
      PD_LOG( PDDEBUG, "Send text index update request to data node. "
              "Message: %s", msgBody.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_onIdxUpdateRes( NET_HANDLE handle, MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> objVec ;
      BSONElement ele ;

      rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                            &numReturned, objVec ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract register reply failed[ %d ]", rc ) ;
      if ( 1 != objVec.size() )
      {
         PD_LOG( PDERROR, "Register reply is not as expected" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = _idxSessionMgr.updateIndexInfo( objVec[0] ) ;
      PD_RC_CHECK( rc, PDERROR, "Update text index information failed[ %d ]",
                   rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   seAdptCB* sdbGetSeAdapterCB()
   {
      static seAdptCB s_seAdptMgr ;
      return &s_seAdptMgr ;
   }

   seAdptOptionsMgr* sdbGetSeAdptOptions()
   {
      return sdbGetSeAdapterCB()->getOptions() ;
   }

   seSvcSessionMgr* sdbGetSeAgentCB()
   {
      return sdbGetSeAdapterCB()->getSeAgentMgr() ;
   }

   utilESCltMgr* sdbGetSeCltMgr()
   {
      return sdbGetSeAdapterCB()->getSeCltMgr() ;
   }
}

