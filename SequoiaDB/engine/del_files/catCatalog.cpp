#include "catCatalog.hpp"
#include "pmd.hpp"
#include "pmdEDU.hpp"
#include "msg.hpp"
#include "pmdShard.hpp"
namespace engine
{
   INT32 _catCatalog::onHeartBeat ( tcp::socket *sock )
   {
      INT32 rc = SDB_OK ;
      map<tcp::socket*,catDataNodeInfo*>::iterator it ;
      map<tcp::socket*,catCoordNodeInfo*>::iterator it1 ;
      catDataNodeInfo *node = NULL ;
      catCoordNodeInfo *node1 = NULL ;
      pdLog ( PDINFO, __FUNC__, __FILE__, __LINE__,
              "Heartbeat message is received" ) ;
      CATCATALOG_SLOCK
      if ( _dataNodesMap.end() != (it=_dataNodesMap.find ( sock ) ) )
      {
         node = (*it).second ;
         goto done ;
      }
      if ( _catalogNodesMap.end() != (it=_catalogNodesMap.find ( sock ) ) )
      {
         node = (*it).second ;
         goto done ;
      }
      if ( _authNodesMap.end() != (it=_authNodesMap.find ( sock ) ) )
      {
         node = (*it).second ;
         goto done ;
      }
      if ( _coordNodesMap.end() != (it1=_coordNodesMap.find ( sock ) ) )
      {
         node1 = (*it1).second ;
         goto done ;
      }
   done :
      if ( node )
      {
         node->_node._lastHeartBeatTick.sample () ;
      }
      else if ( node1 )
      {
         node1->_node._lastHeartBeatTick.sample () ;
      }
      else
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find the socket in catCatalog" ) ;
         rc = SDB_SYS ;
      }
      return rc ;
   }

   void _catCatalog::init ( catNetwork *network, catCollection *collection )
   {
      SDB_ASSERT ( network && collection,
                   "network and collection can't be NULL" )
      _network = network ;
      _collection = collection ;
   }

   INT32 _catCatalog::_handShakeCatalog ( CHAR *pHostName, CHAR *pServiceName,
                                          tcp::socket *sock )
   {
      INT32 rc = SDB_OK ;
      catDataNodeInfo *nodeInfo = NULL ;
      catNetworkCatalog *catalog = _network->getCatalog () ;
      rc = catalog->find ( pHostName, pServiceName, &nodeInfo ) ;
      if ( !rc )
      {
         SDB_ASSERT ( nodeInfo, "nodeInfo shoudln't be NULL here" )
         addCatalogNode ( nodeInfo, sock ) ;
      }
      return rc ;
   }

   INT32 _catCatalog::_handShakeData ( CHAR *pHostName, CHAR *pServiceName,
                                       tcp::socket *sock )
   {
      INT32 rc = SDB_OK ;
      catDataNodeInfo *nodeInfo = NULL ;
      catNetworkData *data = _network->getData () ;
      rc = data->find ( pHostName, pServiceName, &nodeInfo ) ;
      if ( !rc )
      {
         SDB_ASSERT ( nodeInfo, "nodeInfo shoudln't be NULL here" )
         addDataNode ( nodeInfo, sock ) ;
      }
      return rc ;
   }

   INT32 _catCatalog::_handShakeCoord ( CHAR *pHostName, CHAR *pServiceName,
                                        tcp::socket *sock )
   {
      INT32 rc = SDB_OK ;
      catCoordNodeInfo *nodeInfo = NULL ;
      catNetworkCoord *coord = _network->getCoord () ;
      rc = coord->find ( pHostName, pServiceName, &nodeInfo ) ;
      if ( !rc )
      {
         SDB_ASSERT ( nodeInfo, "nodeInfo shoudln't be NULL here" )
         addCoordNode ( nodeInfo, sock ) ;
      }
      return rc ;
   }

   INT32 _catCatalog::_handShakeAuth ( CHAR *pHostName, CHAR *pServiceName,
                                       tcp::socket *sock )
   {
      INT32 rc = SDB_OK ;
      catDataNodeInfo *nodeInfo = NULL ;
      catNetworkAuth *auth = _network->getAuth () ;
      rc = auth->find ( pHostName, pServiceName, &nodeInfo ) ;
      if ( !rc )
      {
         SDB_ASSERT ( nodeInfo, "nodeInfo shoudln't be NULL here" )
         addAuthNode ( nodeInfo, sock ) ;
      }
      return rc ;
   }

   INT32 _catCatalog::_handShakeNode ( CHAR *pHostName, CHAR *pServiceName,
                                       INT32 dbRole, tcp::socket *sock )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( pHostName && pServiceName && sock, "invalid parameters" )
      SDB_ASSERT ( _network, "_catCatalog must be init" )
      switch ( dbRole )
      {
      case SDB_ROLE_CATALOG :
         rc = _handShakeCatalog ( pHostName, pServiceName, sock ) ;
         break ;
      case SDB_ROLE_DATA :
         rc = _handShakeData ( pHostName, pServiceName, sock ) ;
         break ;
      case SDB_ROLE_COORD :
         rc = _handShakeCoord ( pHostName, pServiceName, sock ) ;
         break ;
      case SDB_ROLE_AUTH :
         rc = _handShakeAuth ( pHostName, pServiceName, sock ) ;
         break ;
      default :
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Invalid database role is received: %d", dbRole ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to handshake with incoming request, rc = %d", rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catCatalog::onHandShake ( CHAR *pHostName, CHAR *pServiceName,
                                    INT32 dbRole, tcp::socket *sock )
   {
      INT32 rc                             = SDB_OK ;
      INT32 sent                           = 0 ;
      const CHAR *pHandShakeReplyBuffer    = NULL ;
      INT32 handShakeReplyMsgSize          = 0 ;
      SDB_ASSERT ( sock, "socket can't be NULL" )
      pmdKRCB *krcb                        = pmdGetKRCB () ;
      SDB_REPLCB *replcb                   = krcb->getREPLCB () ;
      SDB_SHARDCB *shardcb                 = krcb->getSHARDCB () ;
      pHandShakeReplyBuffer                = shardcb->getHandShakeReplyBuffer();
      handShakeReplyMsgSize                = pHandShakeReplyBuffer?
                                             *(INT32*)pHandShakeReplyBuffer:0 ;
      pdLog ( PDDEBUG, __FUNC__, __FILE__, __LINE__,
              "Handshake packet is received from %s: %s",
              pHostName, pServiceName ) ;
      // when catalog server receive a handshake request, we should first check
      // whether we are the right one to handle the request
      // if i'm slave we should reject all shard handshakes
      if ( replcb->primaryIsMe() )
      {
         pdLog ( PDDEBUG, __FUNC__, __FILE__, __LINE__,
                 "Handshake is received by slave, we are going to terminate \
the connection" ) ;
         sock->close() ;
         goto done ;
      }
      else
      {
         // we should verify the hostname and service name with our catalog, if
         // it matches any, that means it's a valid coord/data/auth. Otherwise
         // we should reject the connection as well
         rc = _handShakeNode ( pHostName, pServiceName, dbRole, sock ) ;
         if ( rc )
         {
            // if we can't find the node
            pdLog ( PDDEBUG, __FUNC__, __FILE__, __LINE__,
                    "Not able to find the node in catalog, break connection" );
            sock->close () ;
            goto done ;
         }
         // finally, send ack back to sender
         try
         {
            while ( sent < handShakeReplyMsgSize )
            {
               sent += sock->send ( boost::asio::buffer (
                                    &pHandShakeReplyBuffer[sent],
                                    handShakeReplyMsgSize-sent ) ) ;
            }
         }
         catch( std::exception &e )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to reply handshake message: %s",
                    e.what() ) ;
            rc = SDB_NETWORK ;
            goto error ;
         }
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   void _catCatalog::addDataNode ( catDataNodeInfo *dataNode,
                                   tcp::socket *sock )
   {
      SDB_ASSERT ( dataNode, "dataNode can't be NULL" )
      CATCATALOG_XLOCK
      _dataNodesMap[sock] = dataNode ;
   }

   void _catCatalog::addCatalogNode ( catDataNodeInfo *dataNode,
                                      tcp::socket *sock )
   {
      SDB_ASSERT ( dataNode, "dataNode can't be NULL" )
      CATCATALOG_XLOCK
      _catalogNodesMap[sock] = dataNode ;
   }

   void _catCatalog::addAuthNode ( catDataNodeInfo *dataNode,
                                   tcp::socket *sock )
   {
      SDB_ASSERT ( dataNode, "dataNode can't be NULL" )
      CATCATALOG_XLOCK
      _authNodesMap[sock] = dataNode ;
   }

   void _catCatalog::addCoordNode ( catCoordNodeInfo *coordNode,
                                    tcp::socket *sock )
   {
      SDB_ASSERT ( coordNode, "coordNode can't be NULL" )
      CATCATALOG_XLOCK
      _coordNodesMap[sock] = coordNode ;
   }
   template<class T>
   void _catCatalog::_findTimeoutNodes ( vector<T> &dataNodes,
                                   map<tcp::socket*, T> &nodes )
   {
      INT32 count = 0 ;
      INT32 peekTimeRounds = 1000 ;
      ossTick currentTick ;
      ossTick nodeTick ;
      ossTickDelta deltaTick ;
      typename map<tcp::socket*, T>::iterator it ;
      currentTick.clear () ;
      UINT32 seconds;
      UINT32 microseconds ;
      ossTickConversionFactor factor ;
      CATCATALOG_SLOCK
      for ( it = nodes.begin() ; it != nodes.end() ; ++it, ++count )
      {
         // genereate current tick every 1000 rounds
         if ( count % peekTimeRounds == 0 )
            currentTick.sample() ;
         // get the heartbeat tick from node, since "=" operator is designed as
         // atomic operation, so this operation should be safe without explicit
         // locking the nodes
         nodeTick = (*it).second->_node._lastHeartBeatTick ;
         // compare using local memory
         if ( currentTick <= nodeTick )
            continue ;
         // otherwise we have to do delta and compare with our timeout
         deltaTick = currentTick - nodeTick ;
         deltaTick.convertToTime ( factor, seconds, microseconds ) ;
         if ( seconds > CAT_HEARTBEAT_TIMEOUT )
         {
            dataNodes.push_back ( (*it).second ) ;
         }
      }
   }

   void _catCatalog::findTimeoutNodes ( vector<catDataNodeInfo*> &dataNodes,
                                         vector<catDataNodeInfo*> &catalogNodes,
                                         vector<catDataNodeInfo*> &authNodes,
                                         vector<catCoordNodeInfo*> &coordNodes )
   {
      _findTimeoutNodes<catDataNodeInfo*> ( dataNodes, _dataNodesMap ) ;
      _findTimeoutNodes<catDataNodeInfo*> ( catalogNodes, _catalogNodesMap ) ;
      _findTimeoutNodes<catDataNodeInfo*> ( authNodes, _authNodesMap ) ;
      _findTimeoutNodes<catCoordNodeInfo*> ( coordNodes, _coordNodesMap ) ;
   }

   INT32 _catCatalog::findAllNetwork ( vector<BSONObj> &result )
   {
      INT32 rc = SDB_OK ;
      // use empty bson since we want to get everything
      BSONObj matcher ;
      rc = runCatalogSimpleQuery ( matcher,
                                   CAT_COLLECTION_CATALOG_NETWORK, result ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to run catalog query, rc = %d", rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 _catCatalog::findAllReplGroup ( vector<BSONObj> &result )
   {
      INT32 rc = SDB_OK ;
      BSONObj matcher ;
      // first create a query bson object
      try
      {
         matcher = BSON ( "Type"<<"S" ) ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Exception happened when creating matcher: %s",
                 e.what() ) ;
         goto error ;
      }
      rc = runCatalogSimpleQuery ( matcher,
                                   CAT_COLLECTION_CATALOG_NETWORK, result ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to run catalog query, rc = %d", rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   // query a given replication group from catalog server. This is called from
   // non-catalog server nodes
   INT32 _catCatalog::findReplGroup ( CAT_REPLGROUP_ID replGroupID,
                                      BSONObj &result )
   {
      INT32 rc = SDB_OK ;
      BSONObj matcher ;
      vector<BSONObj> resultList ;
      // first create a query bson object
      try
      {
         matcher = BSON ( "Type"<<"S"<<"ID"<<replGroupID ) ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Exception happened when creating matcher: %s",
                 e.what() ) ;
         goto error ;
      }
      rc = runCatalogSimpleQuery ( matcher,
                               CAT_COLLECTION_CATALOG_NETWORK, resultList ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to run catalog query, rc = %d", rc ) ;
         goto error ;
      }
      if ( resultList.size () > 1 )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Maximum one element is expected for replication group %d",
                 replGroupID ) ;
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Actually returned %d rows for replication group %d",
                 resultList.size(), replGroupID ) ;
         goto error ;
      }
      else if ( resultList.size() <= 0 )
      {
         rc = SDB_PMD_RG_NOT_EXIST ;
         goto error ;
      }
      result = resultList[0] ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catCatalog::runCatalogSimpleQuery ( BSONObj &matcher,
                                              const CHAR *pCollectionName,
                                              vector<BSONObj> result )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB () ;
      pmdEDUCB *cb = krcb->getEDUMgr()->getEDU() ;
      SDB_SHARDCB *shardcb = krcb->getSHARDCB () ;
      SDB_ASSERT ( SDB_ROLE_CATALOG != krcb->getDBRole(),
                   "catalog should not call findReplGroup" )
      pmdSessionID sessionID ;
      vector<BSONObj> resultList ;
      CHAR *pBuffer = NULL ;
      INT32 size ;
      // build query message, the pBuffer will be freed at end of the
      // function
      rc = msgBuildQueryMsg ( &pBuffer, &size, pCollectionName,
                              0, sessionID._requestID, 0, -1, &matcher ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed build query message, rc = %d", rc ) ;
         goto error ;
      }
      rc = _readCatalog ( (MsgHeader*)pBuffer, shardcb, &sessionID, cb,
                          resultList ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed query from catalog, rc = %d", rc ) ;
         goto error ;
      }
   done :
      if ( pBuffer )
         SDB_OSS_FREE ( pBuffer ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _catCatalog::_receiveReply ( CHAR *reply, // in
                                      vector<BSONObj> &resultList, //out
                                      INT64 &contextID ) // out
   {
      INT32 rc = SDB_OK ;
      INT32 replyFlag ;
      INT32 startFrom ;
      INT32 numReturned ;
      vector<BSONObj> localResultList ;
      rc = msgExtractReply ( reply, &replyFlag,
                             &contextID, &startFrom, &numReturned,
                             localResultList ) ;
      if ( replyFlag != SDB_OK )
      {
         if ( resultList.size() != 0 )
         {
            BSONObj obj = localResultList[0] ;
            rc = extractRC ( obj ) ;
         }
      }
      else
      {
         vector<BSONObj>::iterator it ;
         for ( it = localResultList.begin(); it != localResultList.end ();
               ++it )
         {
            BSONObj obj = (*it).copy() ;
            resultList.push_back ( obj ) ;
         }
         localResultList.clear() ;
      }

      return rc ;
   }
   // for catalog read activity, we first construct, we construct postMessage
   // and keep reading from it
   // This is a simplified version of full Send/Query/GetMore... because we know
   // we have to get all results, so we can put together everything in a single
   // function to simplify the process
   INT32 _catCatalog::_readCatalog ( _MsgHeader *pMessage,
                                     _SDB_SHARDCB *shardcb,
                                     _pmdSessionID *sessionID,
                                     _pmdEDUCB *cb,
                                     vector<BSONObj> &resultList )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( shardcb, "Shard control block can't be NULL" )
      SDB_ASSERT ( sessionID, "sessionID can't be NULL" )
      SDB_ASSERT ( cb, "EDUCB can't be NULL" )
      EDUID myEDUID = cb->getID () ;
      set<CAT_REPLGROUP_ID> replGroups ;
      replGroups.insert ( CAT_CATALOG_REPLGROUP_ID ) ;
      pmdPostMessage *postMessage = NULL ;
      BOOLEAN newRequestCreated = FALSE ;
      pmdEDUEvent event ;
      MsgOpGetMore getMore ;
      CHAR *pBuffer = (CHAR*)&getMore ;
      INT32 bufSize = sizeof(getMore) ;
      // first we initialize a get more message
      // note this GetMore message is using local stack memory
      // set context ID to 0
      rc = msgBuildGetMoreMsg ( &pBuffer, &bufSize, -1, 0,
                                pMessage->requestID ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to build get more message" ) ;
         goto error ;
      }
      // create a new request with postMessage to receive remote reply
      rc = shardcb->newRequest ( sessionID->_requestID, myEDUID, replGroups,
                                 &postMessage ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to create new request" ) ;
         goto error ;
      }
      newRequestCreated = TRUE ;
      // send to remote
      rc = shardcb->send ( CAT_CATALOG_REPLGROUP_ID, *sessionID, pMessage,
                           pmdDataAccessTypeWrite, FALSE, postMessage ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to send to catalog" ) ;
         goto error ;
      }
      // after send to catalog, let's stay in a loop and keep read
      while ( !rc )
      {
         if ( !cb->waitEvent ( event, 1000 ) )
         {
            if ( cb->isForced () )
            {
               pdLog ( PDEVENT, __FUNC__, __FILE__, __LINE__,
                       "EDU %d is forced", myEDUID ) ;
               rc = SDB_INTERRUPT ;
               goto error ;
            }
            continue ;
         }
         switch ( event._eventType )
         {
         case PMD_EDU_EVENT_SHARD_REPLY :
         {
            PMD_EVENT_MESSAGES *message = (PMD_EVENT_MESSAGES*)event._Data ;
            CHAR *reply = message->shardReply.reply ;
            SDB_ASSERT ( message, "reply can't be NULL here" )
            CAT_REPLGROUP_ID replGroupID = message->shardReply.replGroupID ;
            SDB_ASSERT ( CAT_CATALOG_REPLGROUP_ID == replGroupID,
                         "Invalid replgroup id is read, expect -1" )
            SDB_OSS_FREE ( message ) ;
            INT64 contextID ;
            rc = _receiveReply ( reply, resultList, contextID ) ;
            if ( !rc )
            {
               getMore.contextID = contextID ;
               rc = shardcb->send ( CAT_CATALOG_REPLGROUP_ID, *sessionID,
                                  (MsgHeader*)&getMore,
                                  pmdDataAccessTypeWrite, FALSE, postMessage ) ;
               if ( rc )
               {
                  pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                          "Failed to send, rc = %d", rc ) ;
                  goto error ;
               }
            }
            else if ( SDB_DMS_EOC != rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Error is received, rc = %d", rc ) ;
               goto error ;
            }
            // if rc == SDB_DMS_EOC, that means we hit end of collection so
            // let's exit
            break ;
         }
         case PMD_EDU_EVENT_TERM :
            rc = SDB_APP_FORCED ;
            break ;
         default :
            rc = SDB_SYS ;
            break ;
         }
      }
      if ( SDB_DMS_EOC == rc )
         rc = SDB_OK ;

   done :
      if ( newRequestCreated )
      {
         // send disconnect message
         shardcb->finish ( *sessionID, postMessage ) ;
         // get rid of postMessage memory
         shardcb->deleteRequest ( sessionID->_requestID ) ;
      }
      return rc ;
   error :
      goto done ;
   }
}
