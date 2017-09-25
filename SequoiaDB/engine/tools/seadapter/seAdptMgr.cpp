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
#define SEADPT_INIT_TEXT_INDEX_VERSION          -1
#define SEADPT_IDX_UPDATE_INTERVAL              ( 5 * OSS_ONE_SEC )
#define SEADPT_CAT_RETRY_MAX_TIMES              3

namespace engine
{
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

   void _seSvcSessionMgr::_onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                            const NET_HANDLE &handle,
                                            pmdAsyncSession *pSession )
   {
      // In case of error, let reply the error to the client.
      _reply( handle, rc, pReq ) ;
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
      _indexVersion = SEADPT_INIT_TEXT_INDEX_VERSION ;
      _indexSessionTimer = NET_INVALID_TIMER_ID ;
      _innerSessionID = 0 ;
      _updateStep = SEADPT_IDX_UPDATE_BEGIN ;
   }

   _seIndexSessionMgr::~_seIndexSessionMgr()
   {
   }

   UINT64 _seIndexSessionMgr::makeSessionID( const NET_HANDLE &handle,
                                             const MsgHeader *header )
   {
      return ossPack32To64( PMD_BASE_HANDLE_ID, header->TID ) ;
   }

   INT32 _seIndexSessionMgr::updateIndexInfo( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      BOOLEAN isPrimary = FALSE ;
      INT64 peerVersion = -1 ;

      try
      {
         ele = obj.getField( FIELD_NAME_IS_PRIMARY ) ;
         if ( EOO == ele.type() )
         {
            PD_LOG( PDERROR, "Get peer node role failed" ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         else
         {
            isPrimary = ele.Bool() ;
            if ( isPrimary )
            {
               if ( !_pAdptCB->isDataNodePrimary() )
               {
                  PD_LOG( PDEVENT, "Peer node is primary. Search engine adapter"
                          " switch from READONLY mode to READWRITE mode" ) ;
                  _pAdptCB->setDataNodePrimary( TRUE ) ;
               }
            }
            else
            {
               if ( _pAdptCB->isDataNodePrimary() )
               {
                  PD_LOG( PDEVENT, "Peer node is not primary. Search engine "
                          "adapter switch from READWRITE mode to READONLY "
                          "mode" ) ;
                  _pAdptCB->setDataNodePrimary( FALSE ) ;
                  stopAllIndexer() ;
                  // Reset the index version. If the mode change to full again,
                  // new indexing work should be started.
                  _indexVersion = SEADPT_INIT_TEXT_INDEX_VERSION ;
               }
               // If data node is slave, no indexing work should be started.
               // So just return.
               goto done ;
            }
         }

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
                     string( _pAdptCB->getDataNodeGrpName() ) ;

                     seIndexTask newTask( csName, clName, idxName, cappedCSName,
                                       cappedCLName, esIdxName,
                                       string( _pAdptCB->getDataNodeGrpName() ),
                                       key ) ;
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
                        PD_LOG( PDEVENT, "New index task added: %s",
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
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   void _seIndexSessionMgr::stopAllIndexer()
   {
      // Clean all remainning tasks first, to make it safe to release all task
      // below, because the parameters to start a session are maintained by
      // task list.
      _pAdptCB->cleanInnerSession( SEADPT_SESSION_INDEX ) ;

      // Clean all the remaining task, and release all indexing sessions.
      _taskList.clear() ;
      _newTaskList.clear() ;

      MAPSESSION_IT it = _mapSession.begin () ;
      while ( it != _mapSession.end() )
      {
         _releaseSession_i( it->second, TRUE, FALSE ) ;
         ++it ;
      }
      _mapSession.clear () ;
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
      // do nothing
   }

   pmdAsyncSession* _seIndexSessionMgr::_createSession( SDB_SESSION_TYPE sessionType,
                                                        INT32 startType,
                                                        UINT64 sessionID,
                                                        void *data )
   {
      pmdAsyncSession *pSession = NULL ;

      if ( !data )
      {
         PD_LOG( PDERROR, "Parameter data for session is NULL" ) ;
      }
      else if ( SDB_SESSION_SE_INDEX == sessionType )
      {
         seIndexTask *task = (seIndexTask *)data ;
         if ( task->valid() )
         {
            pSession = SDB_OSS_NEW seAdptIndexSession( sessionID, task ) ;
         }
      }
      else
      {
         PD_LOG( PDERROR, "Invalid session type[%d]", sessionType ) ;
      }

      return pSession ;
   }

   BEGIN_OBJ_MSG_MAP( _seAdptCB, _pmdObjBase )
      ON_MSG( MSG_AUTH_VERIFY_RES, _onRegisterRes )
      ON_MSG( MSG_CAT_QUERY_CATALOG_RSP, _onCatalogResMsg )
      ON_MSG( MSG_SEADPT_UPDATE_IDXINFO_RES, _onIdxUpdateRes )
      ON_MSG( MSG_COM_REMOTE_DISC, _onRemoteDisconnect )
   END_OBJ_MSG_MAP()

   _seAdptCB::_seAdptCB()
   : _indexMsgHandler( &_idxSessionMgr ),
     _svcMsgHandler( &_svcSessionMgr ),
     _indexTimerHandler( &_idxSessionMgr ),
     _svcTimerHandler( &_svcSessionMgr ),
     _indexNetRtAgent( &_indexMsgHandler ),
     _svcRtAgent( &_svcMsgHandler ),
     _idxSessionMgr( this )
   {
      ossMemset( _serviceName, 0, OSS_MAX_SERVICENAME + 1 ) ;
      _dataNodeID.value = MSG_INVALID_ROUTEID ;
      _cataNodeID.value = MSG_INVALID_ROUTEID ;
      _selfRouteID.value = MSG_INVALID_ROUTEID ;
      _peerPrimary = FALSE ;
      ossMemset( _peerGroupName, 0, OSS_MAX_GROUPNAME_SIZE + 1 ) ;
      _regTimerID = NET_INVALID_TIMER_ID ;
      _idxUpdateTimerID = NET_INVALID_TIMER_ID ;
      _oneSecTimerID = NET_INVALID_TIMER_ID ;
      _clVersion = -1 ;
   }

   _seAdptCB::~_seAdptCB()
   {
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
      svcRtID.columns.groupID = SDB_SEADPT_GRP_ID ;
      svcRtID.columns.nodeID = SDB_SEADPT_NODE_ID ;
      svcRtID.columns.serviceID = SDB_SEADPT_SVC_ID ;
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

      _attachEvent.reset() ;
      // 1. start se adapter manager edu.
      rc = pEDUMgr->startEDU( EDU_TYPE_SEADPTMGR, (_pmdObjBase*)this, &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start search engine adapter manager "
                   "edu, rc: %d", rc ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_SEADPTMGR, eduID ) ;
      rc = _attachEvent.wait( SEADPT_WAIT_CB_ATTACH_TIMEOUT ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait search engine adapter manager edu attach "
                   "failed, rc: %d", rc ) ;

      // 2. start network daemon for indexer reader.
      rc = pEDUMgr->startEDU( EDU_TYPE_SE_INDEXR, &_indexNetRtAgent, &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start search engine adapter net, "
                   "rc: %d", rc ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_SE_INDEXR, eduID ) ;
      rc = pEDUMgr->waitUntil( eduID, PMD_EDU_RUNNING ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait indexer reader network daemons active "
                   "failed[ %d ]", rc ) ;

      // 3. start se adapter service.
      rc = pEDUMgr->startEDU( EDU_TYPE_SE_SERVICE, (void *)&_svcRtAgent,
                              &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Start service listener failed[ %d ]", rc ) ;
      pEDUMgr->regSystemEDU( EDU_TYPE_SE_SERVICE, eduID ) ;
      rc = pEDUMgr->waitUntil( eduID, PMD_EDU_RUNNING ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait service listener active failed[ %d ]",
                   rc ) ;

      rc = _setTimers() ;
      PD_RC_CHECK( rc, PDERROR, "Set timers failed[ %d ]", rc ) ;

      // Register immediately.
      _sendRegisterMsg() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::deactive()
   {
      _svcRtAgent.closeListen() ;

      _indexNetRtAgent.stop() ;
      _svcRtAgent.stop() ;

      _idxSessionMgr.setForced() ;
      _svcSessionMgr.setForced() ;

      return SDB_OK ;
   }

   INT32 _seAdptCB::fini()
   {
      _idxSessionMgr.fini() ;
      _svcSessionMgr.fini() ;
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
   }

   void _seAdptCB::onTimer( UINT64 timerID, UINT32 interval )
   {
      if ( timerID == _regTimerID )
      {
         _sendRegisterMsg() ;
      }
      else if ( timerID == _oneSecTimerID )
      {
         _idxSessionMgr.onTimer( interval ) ;
         _svcSessionMgr.onTimer( interval ) ;
      }
      else if ( timerID == _idxUpdateTimerID )
      {
         // Only send the index update request when adapter is successfully
         // registered.
         if ( NET_INVALID_TIMER_ID == _regTimerID )
         {
            INT32 rc = _sendIdxUpdateReq() ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Send index update request failed[ %d ]", rc ) ;
            }
         }
      }

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

   INT32 _seAdptCB::startInnerSession( SEADPT_SESSION_TYPE type,
                                       INT32 innerTID, void *data )
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

   void _seAdptCB::cleanInnerSession( INT32 type )
   {
      ossScopedLock lock( &_seLatch, EXCLUSIVE ) ;

      VECINNERPARAM::iterator itr = _vecInnerSessionParam.begin() ;

      while ( itr != _vecInnerSessionParam.end() )
      {
         if ( type == itr->type )
         {
            _vecInnerSessionParam.erase( itr ) ;
         }
         else
         {
            ++itr ;
         }
      }
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

   INT32 _seAdptCB::syncUpdateCLVersion( const CHAR *collectionName,
                                         INT64 millsec, pmdEDUCB *cb,
                                         INT32 &version )
   {
      INT32 rc = SDB_OK ;
      BSONObj query ;
      BSONObj selector ;
      BOOLEAN needRetry = FALSE ;
      UINT32 retryTimes = 0 ;
      BOOLEAN hasSent = FALSE ;

      try
      {
         query = BSON( FIELD_NAME_NAME << collectionName ) ;
         selector = BSON( FIELD_NAME_VERSION << "" ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Exception occurred when creating query: %s",
                 e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   retry:
      _verUpdateLock.get() ;
      _clVersion = -1 ;
      if ( !hasSent )
      {
         _cataEvent.reset() ;
         rc = _sendCataQueryReq( query, selector, 0, cb ) ;
         if ( SDB_OK == rc )
         {
            hasSent = TRUE ;
         }
      }

      if ( SDB_OK == rc )
      {
         INT32 result = SDB_OK ;
         rc = _cataEvent.wait( millsec, &result ) ;
         if ( SDB_OK == rc )
         {
            // The collection dose not exist. So it has been dropped.
            // Let's quit.
            rc = result ;
            if ( SDB_DMS_EOC == rc || SDB_DMS_NOTEXIST == rc )
            {
               rc = SDB_DMS_NOTEXIST ;
               // Need to return. But DO NOT goto error, as the lock needs
               // to be released.
            }
            else if ( SDB_CLS_NOT_PRIMARY == rc )
            {
               needRetry = TRUE ;
               hasSent = FALSE ;
            }
         }

         if ( SDB_OK == rc )
         {
            version = _clVersion ;
         }
         else if ( SDB_NET_CANNOT_CONNECT == rc ||
                   SDB_NETWORK_CLOSE == rc ||
                   SDB_CLS_NOT_PRIMARY == rc )
         {
            needRetry = TRUE ;
         }
         else if ( rc && SDB_DMS_NOTEXIST != rc )

         {
            needRetry = FALSE ;
         }

      }

      _verUpdateLock.release() ;

      if ( needRetry && retryTimes < SEADPT_CAT_RETRY_MAX_TIMES )
      {
         goto retry ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_onCatalogResMsg( NET_HANDLE handle, MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      INT64 contextID = -1 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector<BSONObj> docObjs ;
      MsgOpReply *res = (MsgOpReply *)msg ;

      if ( SDB_OK != res->flags )
      {
         if ( SDB_DMS_EOC == res->flags || SDB_DMS_NOTEXIST == res->flags )
         {
            _cataEvent.signalAll( SDB_DMS_NOTEXIST ) ;
         }
         else
         {
            _cataEvent.signalAll( res->flags ) ;
         }
      }
      else
      {
         rc = msgExtractReply( (CHAR *)msg, &flag, &contextID, &startFrom,
                               &numReturned, docObjs ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to extract query result, rc: %d", rc ) ;
            goto error ;
         }

         SDB_ASSERT( 1 == numReturned && 1 == docObjs.size(),
                     "Respond size from catalog is wrong" ) ;
         _clVersion = docObjs[0].getIntField( FIELD_NAME_VERSION ) ;
         _cataEvent.signalAll( SDB_OK ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_sendCataQueryReq( const BSONObj &query,
                                       const BSONObj &selector,
                                       UINT64 requestID,
                                       pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 buffSize = 0 ;
      INT32 flag = FLG_QUERY_WITH_RETURNDATA ;

      rc = msgBuildQueryCatalogReqMsg( (CHAR **)&msg, &buffSize, flag, 0, 0,
                                       -1, 0, &query, &selector,
                                       NULL, NULL, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query catalog message failed[ %d ]",
                   rc ) ;

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
      if ( rc )
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
         PD_LOG( PDERROR, "Update route failed[ %d ]", rc ) ;
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

      try
      {
         // Should send the route information to data node. Then data node is
         // able to send request and reply. Currently we use a fixed node id
         // for the adapter.
         authObj = BSON( FIELD_NAME_HOST << pmdGetKRCB()->getHostName()
                         << FIELD_NAME_SERVICE_NAME << _options.getSvcName()
                         << FIELD_NAME_GROUPID << SDB_SEADPT_GRP_ID
                         << FIELD_NAME_NODEID << SDB_SEADPT_NODE_ID
                         << FIELD_NAME_SERVICE_TYPE << SDB_SEADPT_SVC_ID ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

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
      PD_RC_CHECK( rc, PDERROR, "Send register request to data node "
                   "failed[ %d ]", rc ) ;
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

   INT32 _seAdptCB::_resumeRegister()
   {
      INT32 rc = SDB_OK ;

      rc = _indexNetRtAgent.addTimer( OSS_ONE_SEC, &_indexTimerHandler,
                                      _regTimerID ) ;
      PD_RC_CHECK( rc, PDERROR, "Register timer failed[ %d ]", rc ) ;

      _sendRegisterMsg() ;

   done:
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
      const CHAR *peerGrpName = NULL ;
      MsgOpReply *res = (MsgOpReply *)msg ;

      // Adapter has registered successfully.
      if ( NET_INVALID_TIMER_ID == _regTimerID )
      {
         goto done ;
      }

      rc = res->flags ;
      PD_RC_CHECK( rc, PDERROR, "Adapter register on data node failed[ %d ]",
                   rc ) ;

      try
      {
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
         peerGrpName = objVec[0].getStringField( FIELD_NAME_GROUPNAME ) ;
         if ( !peerGrpName )
         {
            PD_LOG( PDERROR, "Find peer node group name failed" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         ossStrncpy( _peerGroupName, peerGrpName, OSS_MAX_GROUPNAME_SIZE ) ;
         _peerGroupName[ OSS_MAX_GROUPNAME_SIZE ] = '\0' ;

         cataInfoObj = objVec[0].getObjectField( FIELD_NAME_CATALOGINFO ) ;
         if ( cataInfoObj.isEmpty() )
         {
            PD_LOG( PDERROR, "Find catalog info in object failed" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         cataHost = cataInfoObj.getStringField( FIELD_NAME_HOST ) ;
         cataSvc = cataInfoObj.getStringField( FIELD_NAME_SERVICE_NAME ) ;
         _cataNodeID.columns.groupID =
            cataInfoObj.getIntField( FIELD_NAME_GROUPID ) ;
         _cataNodeID.columns.nodeID =
            cataInfoObj.getIntField( FIELD_NAME_NODEID ) ;
         _cataNodeID.columns.serviceID =
            cataInfoObj.getIntField( FIELD_NAME_SERVICE ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

      rc = _indexNetRtAgent.updateRoute( _cataNodeID, cataHost.c_str(),
                                         cataSvc.c_str() ) ;
      if ( rc && SDB_NET_UPDATE_EXISTING_NODE != rc )
      {
         PD_LOG( PDERROR, "Update route failed[ %d ]", rc ) ;
         goto error ;
      }

      _killTimer( _regTimerID ) ;
      _regTimerID = NET_INVALID_TIMER_ID ;

      pmdGetKRCB()->setBusinessOK( TRUE ) ;

      PD_LOG( PDEVENT, "Search engine adapter registered on SequoiaDB data "
              "node successfully" ) ;
      if ( TRUE == _peerPrimary )
      {
         PD_LOG( PDEVENT, "Peer node is primary. Search engine adapter is "
                 "running in READWRITE mode" ) ;
      }
      else
      {
         PD_LOG( PDEVENT, "Peer node is not primary. Search engine adapter is "
                 "running in READONLY mode" ) ;
      }

      // When connect to primary node, start the index update timer.
      rc = _indexNetRtAgent.addTimer( SEADPT_IDX_UPDATE_INTERVAL,
                                      &_indexTimerHandler,
                                      _idxUpdateTimerID ) ;
      PD_RC_CHECK( rc, PDERROR, "Register timer failed[ %d ]", rc ) ;

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

      try
      {
         msgBody =
            BSON( FIELD_NAME_VERSION << _idxSessionMgr.getIndexVersion() ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

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

      // Once finish updating the local text indices information, try to start
      // new pending sessions, if any.
      rc = _startInnerSession( SEADPT_SESSION_INDEX, &_idxSessionMgr ) ;
      PD_RC_CHECK( rc, PDERROR, "Start inner session failed[ %d ]", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_onRemoteDisconnect( NET_HANDLE handle, MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;

      // Kill the index update timer, and resume the register.
      if ( NET_INVALID_TIMER_ID != _idxUpdateTimerID )
      {
         _killTimer( _idxUpdateTimerID ) ;
         _idxUpdateTimerID = NET_INVALID_TIMER_ID ;
      }

      _idxSessionMgr.stopAllIndexer() ;

      rc = _resumeRegister() ;
      PD_RC_CHECK( rc, PDERROR, "Resume register failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptCB::_setTimers()
   {
      INT32 rc = SDB_OK ;

      // 1. Set the register timer. Before success, try to register every one
      //    second. Once success, the timer will be killed.
      rc = _indexNetRtAgent.addTimer( OSS_ONE_SEC, &_indexTimerHandler,
                                      _regTimerID ) ;
      PD_RC_CHECK( rc, PDERROR, "Register timer failed[ %d ]", rc ) ;

      // 2. Set the one second timer. This is for session managers to check
      //    deleting sessions.
      rc = _indexNetRtAgent.addTimer( OSS_ONE_SEC, &_indexTimerHandler,
                                      _oneSecTimerID ) ;
      PD_RC_CHECK( rc, PDERROR, "Register timer failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _seAdptCB::_killTimer( UINT32 timerID )
   {
      _indexNetRtAgent.removeTimer( timerID ) ;
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

