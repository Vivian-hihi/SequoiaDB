#include "catCB.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "pmdShard.hpp"
#include "rtn.hpp"
namespace engine
{
   INT32 _catNodeInfo::set ( INT32 id, BSONElement &be )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      BSONElement e ;
      CATNODEINFO_XLOCK
      ossMemset ( _hostName, 0, sizeof(_hostName ) ) ;
      _nodeID = id ;
      if ( be.type() != Object )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "node info must be object" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // get the object
      obj = be.embeddedObject() ;
      // get the Host field
      e = obj.getField ( CAT_HOST_FIELDNAME ) ;
      if ( e.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field", CAT_HOST_FIELDNAME ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( e.type() != String )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "%s field must be String type", CAT_HOST_FIELDNAME ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      ossStrncpy ( _hostName, e.valuestr(), OSS_MAX_HOSTNAME ) ;
      // get the Service field
      e = obj.getField ( CAT_SERVICE_FIELDNAME ) ;
      if ( e.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field", CAT_SERVICE_FIELDNAME ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( e.type() == NumberInt )
      {
         ossSnprintf ( _service, sizeof(_service), "%d", e.numberInt() ) ;
      }
      else if ( e.type() == String )
      {
         ossStrncpy ( _service, e.valuestr(), sizeof(_service) ) ;
      }
      else
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "%s field must be Integer type", CAT_SERVICE_FIELDNAME ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // get the status field
      _status = CAT_STATUS_OFFLINE ;
      e = obj.getField ( CAT_STATUS_FIELDNAME ) ;
      if ( !e.eoo() )
      {
         if ( e.type() == NumberInt && e.numberInt() == CAT_STATUS_PEER )
         {
            INT32 s = e.numberInt() ;
            if ( CAT_STATUS_PEER == s || CAT_STATUS_RC == s )
               _status = s ;
         }
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   // dump info for single node
   // include nodeID, hostName and service
   INT32 _catNodeInfo::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNODEINFO_SLOCK
      try
      {
         BSONObjBuilder ob ;
         ob.append ( "NodeID", _nodeID ) ;
         ob.append ( "HostName", _hostName ) ;
         ob.append ( "Service", _service ) ;
         switch ( _status )
         {
         case CAT_STATUS_PEER :
            ob.append ( "Status", "PEER" ) ;
            break ;
         case CAT_STATUS_OFFLINE :
            ob.append ( "Status", "OFFLINE" ) ;
            break ;
         case CAT_STATUS_RC :
            ob.append ( "Status", "REMOTE CATCHUP" ) ;
            break ;
         default :
            ob.append ( "Status", "Unknown" ) ;
            break ;
         }
         obj = ob.obj() ;
      }
      catch( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for node info: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catDataNodeInfo::set ( INT32 id, BSONElement &be,
                                 CAT_DATA_ROLE &role )
   {
      INT32 rc = SDB_OK ;
      BSONElement e ;
      CATDATANODEINFO_XLOCK
      rc = _node.set ( id, be ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to add node, rc=%d, element=%s",
                 rc, be.toString().c_str() ) ;
         goto error ;
      }
      // here be must be Object
      _role = CAT_DATA_SECONDARY ;
      e = be.embeddedObject().getField ( CAT_ROLE_FIELDNAME ) ;
      if ( !e.eoo() )
      {
         if ( e.type() == NumberInt && e.numberInt() == CAT_ROLE_PRIMARY )
         {
            _role = CAT_DATA_PRIMARY ;
         }
      }
      role = _role ;
   done :
      return rc ;
   error :
      goto done ;
   }

   // dump info for data node, include it's role ( primary or secondary ) and
   // node info
   INT32 _catDataNodeInfo::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATDATANODEINFO_SLOCK
      try
      {
         BSONObjBuilder ob ;
         BSONObj objNode ;
         ob.append ( "Role", CAT_DATA_PRIMARY == _role?"Primary":"Secondary" );
         rc = _node.dumpInfo ( objNode ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to dump info for data node" ) ;
            goto error ;
         }
         ob.append ( "Node", objNode ) ;
         obj = ob.obj () ;
      }
      catch( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for data node: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   BOOLEAN _catDataNodeInfo::match ( CHAR *pHostName, CHAR *pServiceName )
   {
      CATDATANODEINFO_SLOCK
      return ( 0 == ossStrncmp ( pHostName,
               _node._hostName, OSS_MAX_HOSTNAME ) &&
               0 == ossStrncmp ( pServiceName,
               _node._service, OSS_MAX_SERVICENAME ) ) ;
   }

   INT32 _catCoordNodeInfo::set ( INT32 id, BSONElement &e )
   {
      CATCOORDNODEINFO_XLOCK
      return _node.set( id, e ) ;
   }

   // dump info for coord node, it's a wrapper of node info
   INT32 _catCoordNodeInfo::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATCOORDNODEINFO_SLOCK
      try
      {
         BSONObjBuilder ob ;
         BSONObj objNode ;
         rc = _node.dumpInfo ( objNode ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to dump info for coord node" ) ;
            goto error ;
         }
         ob.append ( "Node", objNode ) ;
         obj = ob.obj () ;
      }
      catch( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for coord node: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   BOOLEAN _catCoordNodeInfo::match ( CHAR *pHostName, CHAR *pServiceName )
   {
      CATCOORDNODEINFO_SLOCK
      return ( 0 == ossStrncmp ( pHostName,
               _node._hostName, OSS_MAX_HOSTNAME ) &&
               0 == ossStrncmp ( pServiceName,
               _node._service, OSS_MAX_SERVICENAME ) ) ;
   }

   // update is difficult, because in catCatalog we have a socket map mapping
   // each socket into catDataNodInfo ( so when we get a message from a socket,
   // we know which host it's coming from ). This requires us to maintain all
   // existing catDataNodeInfo if the updated replGroup still have them, or we
   // need to insert new ones into replGroup and delete/close the ones that no
   // longer in the updated replGroup.
   INT32 _catReplGroup::update ( INT32 id, BSONElement &be )
   {
      INT32 rc = SDB_OK ;
      CAT_DATA_ROLE role ;
      UINT32 count = 0 ;
      map<CAT_NODE_ID, catDataNodeInfo*>::iterator it ;
      SDB_ASSERT ( id == _replGroupID, "Unexpected replication group id" )
      if ( be.type() != Array )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "%s field must be Array", CAT_NODES_FIELDNAME ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      {
         // first we store a list of pointers pointing to each old nodes
         vector<catDataNodeInfo *> vec ;
         // and we should also use another vector to store new setup
         vector<catDataNodeInfo *> vec1 ;
         // and then we compare each element with the new BSONElement
         BSONObjIterator i ( be.embeddedObject() ) ;
         while ( i.moreWithEOO() )
         {
            // get each element
            BSONElement e = i.next() ;
            // if we hit the last element in the new input, let's break
            if ( e.eoo() )
               break ;
            // create a temp object, this obj is deleted if we have duplicate,
            // otherwise will be add into nodesMap
            catDataNodeInfo *dataNodeInfo = new catDataNodeInfo( this ) ;
            if ( ! dataNodeInfo )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to allocate memory for dataNodeInfo" ) ;
               rc = SDB_OOM ;
               goto error ;
            }
            // initialize data node
            rc = dataNodeInfo->set ( count, e, role ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed add data node info" ) ;
               goto error ;
            }
            vec1.push_back ( dataNodeInfo ) ;
            ++count ;
         } // while ( i.moreWithEOO() )
         // after we parsed all elements in the input, let's clear nodesMap
         // note we already saved pointer for each catDataNodInfo in vec, so
         // this clear will not cause memory leak

         // from here, make sure we don't have any logic to exit without fully
         // recover _nodesMap
         _mutex.get() ;
         for ( it = _nodesMap.begin(); it != _nodesMap.end(); ++it )
         {
            vec.push_back ( (*it).second ) ;
         }
         _nodesMap.clear() ;

         // this loop is used to map each new element with existing ones, and
         // compare to see which ones remains and which ones are new/deleted
         for ( count = 0 ; count<vec1.size(); ++count )
         {
            catDataNodeInfo *dataNodeInfo = vec1[count] ;
            vector<catDataNodeInfo *>::iterator it1 ;
            // loop through all dataNodes that we haven't compare
            for ( it1 = vec.begin(); it1 != vec.end(); ++it1 )
            {
               // get the pointer for each uncompared nodes
               catDataNodeInfo *p = (*it1) ;
               // if we have a match
               if ( p->_node == dataNodeInfo->_node )
               {
                  p->_mutex.get() ;
                  // if we have a match, let's see if the ID remains
                  p->_node._nodeID = count ;
                  _nodesMap [ count ] = p ;
                  // and check role
                  p->_role = role ;
                  p->_mutex.release() ;
                  // set new primary, we don't have to break connections from
                  // old primary because when primary is demote to secondary it
                  // should disconnect all connections from data node side
                  if ( CAT_DATA_PRIMARY == role )
                  {
                     _currentMasterID = count ;
                  }
                  // remove the node from vector
                  vec.erase ( it1 ) ;
                  // delete the temp node
                  delete dataNodeInfo ;
                  dataNodeInfo = NULL ;
                  break ;
               }
            } // for ( INT32 c = 0; c < vec.size(); ++c )
            // after comparing all elements, if we still have dataNodeInfo
            // exist, that means it's a new one, let's add into map
            if ( dataNodeInfo )
            {
               // if we don't have a match, that means we have a new element
               _nodesMap [ count ] = dataNodeInfo ;
            }
         } // for ( count = 0 ; count<vec1.size(); ++count )
         // if there's still any leftover elements in vec, that means we have
         // those elements deleted in catalog. However there may still opened
         // connections so let's put those dataNode into replGroup's
         // _removedNodes list
         for ( UINT32 c = 0; c < vec.size(); ++c )
         {
            _removedNodes.push_back ( vec[c] ) ;
         }
         _mutex.release () ;
         vec.clear() ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catReplGroup::set ( INT32 id, BSONElement &be )
   {
      INT32 rc = SDB_OK ;
      CAT_DATA_ROLE role ;
      INT32 count = 0 ;
      SDB_ASSERT ( _nodesMap.size() == 0,
                   "we shouldn't set an existing replGroup, please use update" )
      // reinitialize
      map<CAT_NODE_ID, catDataNodeInfo*>::iterator it ;
      CATREPLGROUP_XLOCK
      // make sure element is arra
      if ( be.type() != Array )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "%s field must be Array", CAT_NODES_FIELDNAME ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      _replGroupID = id ;
      _currentMasterID = CAT_REPLGROUP_MASTER_NONE ;
      {
         // for each element in the array
         BSONObjIterator i ( be.embeddedObject() ) ;
         while ( i.moreWithEOO() )
         {
            BSONElement e = i.next() ;
            if ( e.eoo() )
               break ;
            // memory is free in destructor
            catDataNodeInfo *dataNodeInfo = new catDataNodeInfo ( this ) ;
            if ( !dataNodeInfo )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to allocate memory for dataNodeInfo" ) ;
               rc = SDB_OOM ;
               goto error ;
            }
            // parse data node info
            rc = dataNodeInfo->set ( count, e, role ) ;
            if ( rc )
            {
               delete dataNodeInfo ;
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed add data node info" ) ;
               goto error ;
            }

            // if it's primary
            if ( CAT_DATA_PRIMARY == role )
            {
               if ( CAT_REPLGROUP_MASTER_NONE != _currentMasterID )
               {
                  delete dataNodeInfo ;
                  pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                          "Only one primray can be exist in \
replication group" ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               _currentMasterID = count ;
            }

            // add into map
            _nodesMap [ count ] = dataNodeInfo ;
            ++count ;
         }
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   // dump info for replication group, it's a list of multiple data nodes
   INT32 _catReplGroup::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATREPLGROUP_SLOCK
      try
      {
         BSONObjBuilder ob ;
         BSONArrayBuilder ba ;
         map<CAT_NODE_ID, catDataNodeInfo*>::const_iterator it ;
         ob.append ( "ReplGroupID", _replGroupID ) ;
         ob.append ( "Primary", _currentMasterID ) ;
         for ( it = _nodesMap.begin(); it != _nodesMap.end(); ++it )
         {
            BSONObj objNode ;
            rc = (*it).second->dumpInfo ( objNode ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to dump info for replGroup" ) ;
               goto error ;
            }
            ba.append ( objNode ) ;
         }
         ob.append ( "Nodes", ba.arr() ) ;
         obj = ob.obj() ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for data node: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   void _catReplGroup::demotePrimary ( CAT_NODE_ID &oldPrimaryNodeID )
   {
      map<CAT_NODE_ID, catDataNodeInfo*>::iterator it ;
      INT32 count = 0 ;
      oldPrimaryNodeID = -1 ;
      CATREPLGROUP_XLOCK
      for ( it = _nodesMap.begin();
            it != _nodesMap.end() ;
            ++it )
      {
         catDataNodeInfo *dataInfo = (*it).second ;
         if ( CAT_DATA_PRIMARY == dataInfo->_role )
         {
            oldPrimaryNodeID = count ;
            dataInfo->_role = CAT_DATA_SECONDARY ;
            _currentMasterID = CAT_REPLGROUP_MASTER_NONE ;
            return ;
         }
         ++count ;
      }
   }

   void _catReplGroup::promotePrimary ( CAT_NODE_ID primaryID,
                                        CAT_NODE_ID &oldPrimaryNodeID )
   {
      map<CAT_NODE_ID, catDataNodeInfo*>::iterator it ;
      INT32 count = 0 ;
      oldPrimaryNodeID = -1 ;
      SDB_ASSERT ( primaryID >= 0, "primary id must be greater than 0" )
      CATREPLGROUP_XLOCK
      primaryID = primaryID % _nodesMap.size() ;
      for ( it = _nodesMap.begin();
            it != _nodesMap.end() ;
            ++it )
      {
         catDataNodeInfo *dataInfo = (*it).second ;
         if ( CAT_DATA_PRIMARY == dataInfo->_role )
         {
            oldPrimaryNodeID = count ;
            dataInfo->_role = CAT_DATA_SECONDARY ;
         }
         if ( count == primaryID )
         {
            dataInfo->_role = CAT_DATA_PRIMARY ;
         }
         ++count ;
      }
      _currentMasterID = primaryID ;
   }

   INT32 _catReplGroup::find ( CHAR *pHostName, CHAR *pServiceName,
                               catDataNodeInfo **nodeInfo )
   {
      INT32 rc = SDB_CAT_NODE_NOT_FOUND ;
      SDB_ASSERT ( pHostName && pServiceName && nodeInfo,
                   "Input can't be NULL" )
      map<CAT_NODE_ID, catDataNodeInfo *>::iterator it ;
      CATREPLGROUP_SLOCK
      for ( it = _nodesMap.begin(); it != _nodesMap.end(); ++it )
      {
         catDataNodeInfo *n = (*it).second ;
         if ( n->match ( pHostName, pServiceName ) )
         {
            *nodeInfo = n ;
            rc = SDB_OK ;
            goto done ;
         }
      }
   done :
      return rc ;
   }

   INT32 _catNetworkData::getAllReplGroups ( set<CAT_REPLGROUP_ID>
                                             &replGroups )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKDATA_SLOCK
      map<CAT_REPLGROUP_ID, catReplGroup*>::const_iterator it ;
      for ( it = _replGroupMap.begin(); it != _replGroupMap.end(); ++it )
      {
         replGroups.insert ( (*it).first ) ;
      }
      return rc ;
   }
   INT32 _catNetworkData::find ( CAT_REPLGROUP_ID id, pmdReplGroup *group )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( group, "group can't be NULL" )
      map<CAT_REPLGROUP_ID, catReplGroup*>::const_iterator it ;
      map<CAT_NODE_ID, catDataNodeInfo*>::const_iterator it1 ;
      catReplGroup *replGroup ;
      CATNETWORKDATA_SLOCK
      if ( _replGroupMap.end() == (it = _replGroupMap.find ( id )) )
      {
         if ( SDB_ROLE_CATALOG != pmdGetKRCB()->getDBRole () )
         {
            BSONObj result ;
            // we need to talk with remote catalog server to fetch the info
            //rc = catCatalog::findReplGroup ( id, result ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to find replication group %d", id ) ;
               goto error ;
            }

         }
         // otherwise we should have everything in memory in catalog node, so if
         // we hit this condition, let's just report can't find
         rc = SDB_PMD_RG_NOT_EXIST ;
         goto error ;
      }
      replGroup = (*it).second ;
      for ( it1 = replGroup->_nodesMap.begin();
            it1 != replGroup->_nodesMap.end() ;
            ++it1 )
      {
         catDataNodeInfo *dataInfo = (*it1).second ;
         // free in pmdReplGroup destructor
         pmdConnectPair *pair = new pmdConnectPair (
               dataInfo->_node._nodeID,
               dataInfo->_node._hostName,
               dataInfo->_node._service ) ;
         if ( !pair )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Not able to allocate memory for pair" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         if ( CAT_DATA_PRIMARY == dataInfo->_role )
         {
            group->_primary = pair ;
         }
         else
         {
            group->_secondary.push_back ( pair ) ;
         }
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkData::find ( CHAR *pHostName, CHAR *pServiceName,
                                 catDataNodeInfo **nodeInfo )
   {
      INT32 rc = SDB_CAT_NODE_NOT_FOUND ;
      SDB_ASSERT ( pHostName && pServiceName && nodeInfo,
                   "Input can't be NULL" )
      map<CAT_REPLGROUP_ID, catReplGroup*>::const_iterator it ;
      CATNETWORKDATA_SLOCK
      // should we use map instead of loop? Or it doesn't make much difference
      // because we won't have millions of replGroups in the system?
      for ( it = _replGroupMap.begin(); it != _replGroupMap.end(); ++it )
      {
         rc = (*it).second->find ( pHostName, pServiceName, nodeInfo ) ;
         if ( !rc )
         {
            // we found one
            rc = SDB_OK ;
            goto done ;
         }
      }
      // if we can't find anything after loop through all, let's see if we are
      // coord or others
      if ( SDB_ROLE_CATALOG != pmdGetKRCB()->getDBRole () )
      {
         // fetch from remote

      }
   done :
      return rc ;
   }
   INT32 _catNetworkData::update ( INT32 id, BSONObj &obj )
   {
      map<CAT_REPLGROUP_ID, catReplGroup*>::iterator it ;
      CATNETWORKDATA_XLOCK
      if ( _replGroupMap.end() == (it=_replGroupMap.find(id)) )
      {
         return _add ( id, obj ) ;
      }
      return _update ( id, obj, (*it).second ) ;
   }
   INT32 _catNetworkData::_update ( INT32 id, BSONObj &obj,
                                    catReplGroup* replGroup )
   {
      INT32 rc = SDB_OK ;
      BSONElement be ;
      be = obj.getField ( CAT_NODES_FIELDNAME ) ;
      if ( be.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field from %s", CAT_NODES_FIELDNAME,
                 obj.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = replGroup->update ( id, be ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to add replication group: %s",
                 be.toString().c_str() ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 _catNetworkData::add ( INT32 id, BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKDATA_XLOCK
      // make sure same id doesn't exist
      if ( _replGroupMap.end() != _replGroupMap.find(id) )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Duplicate replication group id %d: %s",
                 id, obj.toString().c_str() ) ;
         rc = SDB_PMD_RG_EXIST ;
         goto error ;
      }
      rc = _add ( id, obj ) ;
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 _catNetworkData::_add ( INT32 id, BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      BSONElement be ;
      catReplGroup *replGroup = NULL ;
      be = obj.getField ( CAT_NODES_FIELDNAME ) ;
      if ( be.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field from %s", CAT_NODES_FIELDNAME,
                 obj.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // free in destructor
      replGroup = new catReplGroup() ;
      if ( !replGroup )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to allocate memory for replGroup" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = replGroup->set ( id, be ) ;
      if ( rc )
      {
         delete replGroup ;
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to add replication group: %s",
                 be.toString().c_str() ) ;
         goto error ;
      }
      _replGroupMap [ id ] = replGroup ;
   done :
      return rc ;
   error :
      goto done ;
   }

   // data network topology includes one or more replication groups
   INT32 _catNetworkData::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKDATA_SLOCK
      try
      {
         BSONObjBuilder ob ;
         BSONArrayBuilder ba ;
         map<CAT_REPLGROUP_ID, catReplGroup*>::const_iterator it ;
         for ( it = _replGroupMap.begin(); it != _replGroupMap.end(); ++it )
         {
            BSONObj objRG ;
            rc = (*it).second->dumpInfo ( objRG ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to dump info for data network topology" ) ;
               goto error ;
            }
            ba.append ( objRG ) ;
         }
         ob.append ( "ReplicationGroup", ba.arr() ) ;
         obj = ob.obj() ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for data network topology: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCoord::add ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      INT32 count = 0 ;
      BSONElement be = obj.getField ( CAT_NODES_FIELDNAME ) ;
      if ( be.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field from %s", CAT_NODES_FIELDNAME,
                 obj.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // make sure the element is array
      if ( be.type() != Array )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "%s field must be Array", CAT_NODES_FIELDNAME ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      {
         CATNETWORKCOORD_XLOCK
         // for each element in the array
         BSONObjIterator i ( be.embeddedObject() ) ;
         while ( i.moreWithEOO() )
         {
            BSONElement e = i.next() ;
            if ( e.eoo() )
               break ;
            // memory is deleted in destructor
            catCoordNodeInfo *coordNodeInfo = new catCoordNodeInfo() ;
            if ( !coordNodeInfo )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to allocate memory for coordNodeInfo" ) ;
               rc = SDB_OOM ;
               goto error ;
            }
            // parse the catalog node element
            rc = coordNodeInfo->set ( count, e ) ;
            if ( rc )
            {
               delete coordNodeInfo ;
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed add coord node info" ) ;
               goto error ;
            }
            // add into _coordMap
            _coordMap[count] = coordNodeInfo ;
            ++count ;
         }
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCoord::update ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      UINT32 count = 0 ;
      map<CAT_NODE_ID, catCoordNodeInfo*>::iterator it ;
      BSONElement be = obj.getField ( CAT_NODES_FIELDNAME ) ;
      if ( be.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field from %s", CAT_NODES_FIELDNAME,
                 obj.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // make sure the element is array
      if ( be.type() != Array )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "%s field must be Array", CAT_NODES_FIELDNAME ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      {
         // first store a list of pointers pointing to each old nodes
         vector<catCoordNodeInfo *> vec ;
         // then we should also use another vector to store new setup
         vector<catCoordNodeInfo *> vec1 ;
         // and then we compare each element with the new BSONElement
         BSONObjIterator i ( be.embeddedObject() ) ;
         while ( i.moreWithEOO() )
         {
            // get each element
            BSONElement e = i.next () ;
            if ( e.eoo() )
               break ;
            // create a temp object, this obj is deleted if we have duplicates,
            // otherwise will be added into nodesMap
            catCoordNodeInfo *coordNodeInfo = new catCoordNodeInfo () ;
            if ( ! coordNodeInfo )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to allocate memory for coordNodeInfo" ) ;
               rc = SDB_OOM ;
               goto error ;
            }
            // initialize coord node
            rc = coordNodeInfo->set ( count, e ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to set coord node info" ) ;
               goto error ;
            }
            vec1.push_back ( coordNodeInfo ) ;
            ++count ;
         } // while ( i.moreWithEOO() )
         // after we parse all elements in the input, let's clear coordMap, note
         // that we already saved pointer for each catCoordNodeInfo in vec, so
         // this clear will not cause memory leak
         // from here, make sure we don't have any logic to exit without fully
         // recover _nodeMap
         _mutex.get() ;
         for ( it = _coordMap.begin(); it != _coordMap.end(); ++it )
         {
            vec.push_back ( (*it).second ) ;
         }
         _coordMap.clear() ;

         // this loop is used to map each new element with existing ones, and
         // compare to see which ones remains and which ones are new/deleted
         for ( count = 0 ; count<vec1.size() ; ++ count )
         {
            catCoordNodeInfo *coordNodeInfo = vec1[count] ;
            vector<catCoordNodeInfo *>::iterator it1 ;
            // loop through all coordNodes that we haven't compare
            for ( it1 = vec.begin(); it1 != vec.end(); ++it1 )
            {
               // get the pointer for each uncompared nodes
               catCoordNodeInfo *p = (*it1) ;
               // if we have a match
               if ( p->_node == coordNodeInfo->_node )
               {
                  p->_mutex.get() ;
                  // if we have a match, let's see if the ID remains
                  p->_node._nodeID = count ;
                  _coordMap [ count ] = p ;
                  // remove the node from vector
                  vec.erase ( it1 ) ;
                  // delete temp object
                  delete coordNodeInfo ;
                  coordNodeInfo = NULL ;
                  break ;
               }
            } // for ( it1 = vec.begin(); it1 != vec.end(); ++it1 )
            // after comparing all elements, if we still have coordNodeInfo
            // exist, that means it's a new one, let's add into map
            if ( coordNodeInfo )
            {
               // if we don't have a match, that means we have a new element
               _coordMap [ count ] = coordNodeInfo ;
            }
         } // for ( count = 0 ; count<vec1.size() ; ++ count )
         // if there's still any leftover elements in vec, that means we have
         // those elements deleted in catalog, However there may still opened
         // connections so let's put those dataNode into _removedNodes list
         for ( UINT32 c = 0 ; c < vec.size(); ++c )
         {
            _removedNodes.push_back ( vec[c] ) ;
         }
         _mutex.release () ;
         vec.clear() ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCoord::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKCOORD_SLOCK
      try
      {
         BSONObjBuilder ob ;
         BSONArrayBuilder ba ;
         map<CAT_NODE_ID, catCoordNodeInfo*>::const_iterator it ;
         for ( it = _coordMap.begin(); it != _coordMap.end(); ++it )
         {
            BSONObj objCoord ;
            rc = (*it).second->dumpInfo ( objCoord ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to dump info for coord network topology" ) ;
               goto error ;
            }
            ba.append ( objCoord ) ;
         }
         ob.append ( "Coordinators", ba.arr() ) ;
         obj = ob.obj() ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for coord network topology: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCoord::find ( CHAR *pHostName, CHAR *pServiceName,
                                  catCoordNodeInfo **nodeInfo )
   {
      INT32 rc = SDB_CAT_NODE_NOT_FOUND ;
      SDB_ASSERT ( pHostName && pServiceName && nodeInfo,
                   "Input can't be NULL" )
      map<CAT_NODE_ID, catCoordNodeInfo *>::iterator it ;
      CATNETWORKCOORD_SLOCK
      for ( it = _coordMap.begin(); it != _coordMap.end(); ++it )
      {
         catCoordNodeInfo *n = (*it).second ;
         if ( n->match ( pHostName, pServiceName ) )
         {
            *nodeInfo = n ;
            rc = SDB_OK ;
            goto done ;
         }
      }
      // if we get here, that means we can't find the node
      // if we are not catalog, let's fetch from remote catalog
      if ( SDB_ROLE_CATALOG != pmdGetKRCB()->getDBRole () )
      {
                
      }
   done :
      return rc ;
   }

   void _catNetworkCatalog::demotePrimary ( CAT_NODE_ID &oldPrimaryNodeID )
   {
      _catalog.demotePrimary ( oldPrimaryNodeID ) ;
   }

   void _catNetworkCatalog::promotePrimary ( CAT_NODE_ID primaryID,
                                             CAT_NODE_ID &oldPrimaryNodeID )
   {
      _catalog.promotePrimary ( primaryID, oldPrimaryNodeID ) ;
   }
   CAT_NODE_ID _catNetworkCatalog::getNextAttemptNode ()
   {
      CATNETWORKCATALOG_XLOCK ;
      if ( 0 != _catalog._nodesMap.size() )
         _previousAttemptNode = ++_previousAttemptNode % _catalog._nodesMap.size();
      else
         _previousAttemptNode = 0 ;
      return _previousAttemptNode ;
   }

   INT32 _catNetworkCatalog::get ( pmdReplGroup *group )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( group, "group can't be NULL" )
      map<CAT_NODE_ID, catDataNodeInfo*>::iterator it ;
      CATNETWORKCATALOG_SLOCK
      group->clear () ;
      for ( it = _catalog._nodesMap.begin();
            it != _catalog._nodesMap.end() ;
            ++it )
      {
         catDataNodeInfo *dataInfo = (*it).second ;
         // free in group destructor
         pmdConnectPair *pair = new pmdConnectPair (
               dataInfo->_node._nodeID,
               dataInfo->_node._hostName,
               dataInfo->_node._service ) ;
         if ( !pair )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Not able to allocate memory for pair" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         if ( CAT_DATA_PRIMARY == dataInfo->_role )
         {
            group->_primary = pair ;
         }
         else
         {
            group->_secondary.push_back ( pair ) ;
         }
      }
   done :
      return rc ;
   error :
      goto done ;
   }


   INT32 _catNetworkCatalog::add ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKCATALOG_SLOCK
      BSONElement be = obj.getField ( CAT_NODES_FIELDNAME ) ;
      if ( be.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field from %s", CAT_NODES_FIELDNAME,
                 obj.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      rc = _catalog.set ( 0, be ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to add catalog into catalog, rc = %d",
                 rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCatalog::update ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKCATALOG_SLOCK
      BSONElement be = obj.getField ( CAT_NODES_FIELDNAME ) ;
      if ( be.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field from %s", CAT_NODES_FIELDNAME,
                 obj.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      rc = _catalog.update ( 0, be ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to update catalog into catalog, rc = %d",
                 rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCatalog::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKCATALOG_SLOCK
      try
      {
         BSONObjBuilder ob ;
         BSONObj objCatalog ;
         rc = _catalog.dumpInfo ( objCatalog ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to dump info for catalog network topology" ) ;
            goto error ;
         }
         ob.append ( "Catalog", objCatalog ) ;
         obj = ob.obj() ;
      }
      catch( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for catalog network topology: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCatalog::find ( CHAR *pHostName, CHAR *pServiceName,
                                    catDataNodeInfo **nodeInfo )
   {
      INT32 rc = SDB_CAT_NODE_NOT_FOUND ;
      SDB_ASSERT ( pHostName && pServiceName && nodeInfo,
                   "Input can't be NULL" )
      CATNETWORKCATALOG_SLOCK
      // should we use map instead of loop? Or it doesn't make much diff because
      // we won't have millions of replGroups in the system?
      rc = _catalog.find ( pHostName, pServiceName, nodeInfo ) ;
      if ( !rc )
      {
         // we found one
         rc = SDB_OK ;
         goto done ;
      }
      // if we can't find any after loop through all, let's see if we are
      // catalog
      if ( SDB_ROLE_CATALOG != pmdGetKRCB()->getDBRole() )
      {
         // fetch from remote
          
      }
   done :
      return rc ;
   }


   INT32 _catNetworkAuth::add ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKAUTH_SLOCK
      BSONElement be = obj.getField ( CAT_NODES_FIELDNAME ) ;
      if ( be.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field from %s", CAT_NODES_FIELDNAME,
                 obj.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      rc = _auth.set ( 0, be ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to add authenticator into catalog, rc = %d",
                 rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkAuth::update ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKAUTH_SLOCK
      BSONElement be = obj.getField ( CAT_NODES_FIELDNAME ) ;
      if ( be.eoo() )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Unable to find %s field from %s", CAT_NODES_FIELDNAME,
                 obj.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      rc = _auth.update ( 0, be ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to update authenticator into catalog, rc = %d",
                 rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }


   INT32 _catNetworkAuth::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORKAUTH_SLOCK
      try
      {
         BSONObjBuilder ob ;
         BSONObj objAuth ;
         rc = _auth.dumpInfo ( objAuth ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                   "Failed to dump info for authentication network topology" ) ;
            goto error ;
         }
         ob.append ( "Auth", objAuth ) ;
         obj = ob.obj() ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for authentication network topology: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkAuth::find ( CHAR *pHostName, CHAR *pServiceName,
                                 catDataNodeInfo **nodeInfo )
   {
      INT32 rc = SDB_CAT_NODE_NOT_FOUND ;
      SDB_ASSERT ( pHostName && pServiceName && nodeInfo,
                   "Input can't be NULL" )
      CATNETWORKAUTH_SLOCK
      // should we use map instead of loop? Or it doesn't make much diff because
      // we won't have millions of replGroups in the system?
      rc = _auth.find ( pHostName, pServiceName, nodeInfo ) ;
      if ( !rc )
      {
         // we found one
         rc = SDB_OK ;
         goto done ;
      }
      // if we can't find any after loop through all, let's see if we are
      // catalog
      if ( SDB_ROLE_CATALOG != pmdGetKRCB()->getDBRole () )
      {
         // fetch from remote
         
      }
   done :
      return rc ;
   }

   void _catNetwork::getCatalogReplGroup ( CAT_REPLGROUP_ID &replGroup )
   {
      replGroup = CAT_CATALOG_REPLGROUP_ID ;
   }

   INT32 _catNetwork::addupdate ( BSONObj &obj, BOOLEAN isUpdate )
   {
      INT32 rc = SDB_OK ;
      const CHAR *pType = NULL ;
      CATNETWORK_SLOCK
      BSONElement e = obj.getField ( CAT_TYPE_FIELDNAME ) ;
      if ( e.eoo () || e.type() != String )
      {
         pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
                 "Unexpected record, can't find %s field or it's not string",
                 CAT_TYPE_FIELDNAME ) ;
         rc = SDB_CORRUPTED_RECORD ;
         goto error ;
      }
      pType = e.valuestr () ;
      switch ( pType[0] )
      {
      case CAT_SHARD_VALUENAME :
      {
         BSONElement el = obj.getField ( CAT_ID_FIELDNAME ) ;
         if ( el.eoo() || el.type() != NumberInt )
         {
            pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
                    "Shard ID doesn't exist or not integer" ) ;
            rc = SDB_CORRUPTED_RECORD ;
            goto error ;
         }
         if ( isUpdate )
            rc = _data.update ( el.numberInt(), obj ) ;
         else
            rc = _data.add ( el.numberInt(), obj ) ;
         if ( rc )
         {
            pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
                    "Failed to add shard into catalog: %s",
                    obj.toString().c_str() ) ;
            goto error ;
         }
         break ;
      }
      case CAT_COORD_VALUENAME :
         if ( isUpdate )
            rc = _coord.update ( obj ) ;
         else
            rc = _coord.add ( obj ) ;
         if ( rc )
         {
            pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
                    "Failed to add coord into catalog: %s",
                    obj.toString().c_str() ) ;
            goto error ;
         }
         break ;
      case CAT_CATALOG_VALUENAME :
         // for catalog, always use "update" version instead of "insert"
         rc = _catalog.update ( obj ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to add catalog into catalog: %s",
                    obj.toString().c_str() ) ;
            goto error ;
         }
         break ;
      case CAT_AUTH_VALUENAME :
         if ( isUpdate )
            rc = _auth.update ( obj ) ;
         else
            rc = _auth.add ( obj ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to add auth into catalog: %s",
                    obj.toString().c_str() ) ;
            goto error ;
         }
         break ;
      default :
         pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
                 "Unexpected type, expects one of (%c)(%c)(%c)(%c)",
                 CAT_SHARD_VALUENAME, CAT_COORD_VALUENAME,
                 CAT_CATALOG_VALUENAME, CAT_AUTH_VALUENAME ) ;
         rc = SDB_CORRUPTED_RECORD ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetwork::dumpInfo ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORK_SLOCK
      try
      {
         BSONObjBuilder ob ;
         // authentication
         {
            BSONObj authObj ;
            rc = _auth.dumpInfo ( authObj ) ;
            if ( rc )
               goto error ;
            ob.append ( "Authentication", authObj ) ;
         }
         // catalog
         {
            BSONObj catalogObj ;
            rc = _catalog.dumpInfo ( catalogObj ) ;
            if ( rc )
               goto error ;
            ob.append ( "Catalog", catalogObj ) ;
         }
         // coord
         {
            BSONObj coordObj ;
            rc = _coord.dumpInfo ( coordObj ) ;
            if ( rc )
               goto error ;
            ob.append ( "Coordinators", coordObj ) ;
         }
         // data
         {
            BSONObj dataObj ;
            rc = _data.dumpInfo ( dataObj ) ;
            if ( rc )
               goto error ;
            ob.append ( "Data", dataObj ) ;
         }
         obj = ob.obj() ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to generate info for network topology: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
              "Failed to dump info for network topology" ) ;
      goto done ;
   }

   INT32 _catNetwork::getAllReplGroups ( set<CAT_REPLGROUP_ID> &replGroups )
   {
      CATNETWORK_SLOCK
      return _data.getAllReplGroups ( replGroups ) ;
   }

   // this should be called ONLY in CATALOG mode
   INT32 _catNetwork::init ()
   {
      INT32 rc                   = SDB_OK ;
      pmdKRCB *krcb              = pmdGetKRCB() ;
      SDB_ASSERT ( SDB_ROLE_CATALOG == krcb->getDBRole(),
                   "Database role must be catalog" )
      SDB_DMSCB *dmsCB           = krcb->getDMSCB() ;
      SDB_RTNCB *rtnCB           = krcb->getRTNCB() ;
      pmdEDUMgr *eduMgr          = krcb->getEDUMgr () ;
      CHAR *pBufferCurrent       = NULL ;
      SINT32 bufferLen           = 0 ;
      SINT32 numRecords          = 0 ;
      SINT64 startingPos         = 0 ;
      SINT64 contextID           = 0 ;
      BOOLEAN retried            = FALSE ;
      pmdEDUCB *cb               = NULL ;
      INT32 catalogNetworkLen    = 0 ;
      CHAR *collectionName       = NULL ;
      BSONObj dummyObj ;
      cb                         = new pmdEDUCB ( eduMgr, EDU_TYPE_AGENT ) ;
      if ( !cb )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Not able to allocate memory for EDUCB" ) ;
         goto error ;
      }
      catalogNetworkLen          = ossStrlen (
                                     CAT_COLLECTION_CATALOG_NETWORK ) ;
      collectionName             = (CHAR*)SDB_OSS_MALLOC (
                                   catalogNetworkLen + 1 ) ;
      if ( !collectionName )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Not able to allocate memory for collection name" ) ;
         goto error ;
      }
      ossStrncpy ( collectionName, CAT_COLLECTION_CATALOG_NETWORK,
                   catalogNetworkLen ) ;
      collectionName [ catalogNetworkLen ] = 0 ;
   retry:
      // then let's try to find the collection
      rc = rtnFindCollection ( CAT_COLLECTION_CATALOG_NETWORK, dmsCB ) ;
      // if we can't find
      if ( rc )
      {
         // let's see if we have tried to create new collection
         if ( retried || SDB_DMS_NOTEXIST != rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Cannot find collection %s, rc = %d",
                    CAT_COLLECTION_CATALOG_NETWORK, rc ) ;
            goto error ;
         }
         // if not tried, let's create one
         rc = rtnCreateCollectionCommand (
               CAT_COLLECTION_CATALOG_NETWORK,
               NULL,
               dmsCB, NULL ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to create %s collection, rc = %d",
                    CAT_COLLECTION_CATALOG_NETWORK, rc ) ;
            goto error ;
         }
         // mark we have tried
         retried = TRUE ;
         goto retry ;
      }

      // then we should read everything in CAT_COLLECTION_CATALOG_NETWORK into
      // memory
      rc = rtnQuery ( collectionName, dummyObj, dummyObj,
                      dummyObj, dummyObj, 0, cb, 0, -1, dmsCB, rtnCB,
                      contextID ) ;
      if ( rc )
      {
         // if we hit end of collection, let's return SDB_OK, otherwise return
         // error
         if ( SDB_DMS_EOC != rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to query %s collection",
                    CAT_COLLECTION_CATALOG_NETWORK ) ;
            goto error ;
         }
         rc = SDB_OK ;
         goto done ;
      }

      do
      {
         rc = rtnGetMore ( contextID, 1, &pBufferCurrent, bufferLen,
                           numRecords, startingPos, cb, rtnCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               goto done ;
            }
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to fetch from %s collection",
                    CAT_COLLECTION_CATALOG_NETWORK ) ;
            goto error ;
         }
         if ( pBufferCurrent )
         {
            try
            {
               BSONObj obj ( pBufferCurrent ) ;
               rc = addupdate ( obj, FALSE ) ;
               if ( rc )
               {
                  pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                          "Failed to add member into network catalog: %s",
                          obj.toString().c_str() ) ;
                  goto error ;
               }
            }
            catch ( std::exception &e )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Invalid data is read from context buffer: %s",
                       e.what() ) ;
               rc = SDB_CORRUPTED_RECORD ;
               goto error ;
            }
         }
      } while ( TRUE ) ;
   done :
      if ( cb )
         delete cb ;
      if ( collectionName )
         SDB_OSS_FREE ( collectionName ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _catDataNodeInfo::obj ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATDATANODEINFO_SLOCK
      try
      {
         BSONObjBuilder ob ;
         ob.append ( CAT_HOST_FIELDNAME, _node._hostName ) ;
         ob.append ( CAT_SERVICE_FIELDNAME, _node._service ) ;
         ob.append ( CAT_STATUS_FIELDNAME, _node._status ) ;
         ob.append ( CAT_ROLE_FIELDNAME, _role ) ;
         obj = ob.obj () ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to generate data node object: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catReplGroup::obj ( BSONObjBuilder &ob )
   {
      INT32 rc = SDB_OK ;
      BSONArrayBuilder ba ;
      map<CAT_NODE_ID, catDataNodeInfo*>::const_iterator it ;
      CATREPLGROUP_SLOCK
      try
      {
         ob.append ( CAT_ID_FIELDNAME, _replGroupID ) ;
         for ( it = _nodesMap.begin(); it != _nodesMap.end(); ++it )
         {
            BSONObj objNode ;
            rc = (*it).second->obj ( objNode ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to dump info for replGroup" ) ;
               goto error ;
            }
            ba.append ( objNode ) ;
         }
         ob.append ( CAT_NODES_FIELDNAME, ba.arr() ) ;
      }
      catch( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for replGroup: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 _catCoordNodeInfo::obj ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CATCOORDNODEINFO_SLOCK
      try
      {
         BSONObjBuilder ob ;
         ob.append ( CAT_HOST_FIELDNAME, _node._hostName ) ;
         ob.append ( CAT_SERVICE_FIELDNAME, _node._service ) ;
         ob.append ( CAT_STATUS_FIELDNAME, _node._status ) ;
         obj = ob.obj () ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to generate data node object: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkData::obj ( vector<BSONObj> &obj )
   {
      INT32 rc = SDB_OK ;
      map<CAT_REPLGROUP_ID, catReplGroup*>::const_iterator it ;
      BSONObjBuilder ob ;
      CATNETWORKDATA_SLOCK
      for ( it = _replGroupMap.begin () ;
            it != _replGroupMap.end () ;
            ++it )
      {
         BSONObj tempObj ;
         ob.append ( CAT_TYPE_FIELDNAME, std::string(""+CAT_SHARD_VALUENAME) ) ;
         rc = (*it).second->obj ( ob ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to generate obj for network data" ) ;
            goto error ;
         }
         tempObj = ob.obj () ;
         obj.push_back ( tempObj ) ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCoord::obj ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder ob ;
      BSONArrayBuilder ba ;
      map<CAT_NODE_ID, catCoordNodeInfo*>::const_iterator it ;
      CATNETWORKCOORD_SLOCK
      try
      {
         ob.append ( CAT_TYPE_FIELDNAME,
                     std::string ( "" + CAT_COORD_VALUENAME ) ) ;
         for ( it = _coordMap.begin() ;
               it != _coordMap.end() ;
               ++it )
         {
            BSONObj objNode ;
            rc = (*it).second->obj ( objNode ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to dump info for catNetworkCoord" ) ;
               goto error ;
            }
            ba.append ( objNode ) ;
         }
         ob.append ( CAT_NODES_FIELDNAME, ba.arr() ) ;
         obj = ob.obj() ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for catNetworkCoord: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkCatalog::obj ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder ob ;
      BSONArrayBuilder ba ;
      map<CAT_NODE_ID, catDataNodeInfo*>::const_iterator it ;
      CATNETWORKCATALOG_SLOCK
      try
      {
         ob.append ( CAT_TYPE_FIELDNAME,
                     std::string ( "" + CAT_CATALOG_VALUENAME ) ) ;
         for ( it = _catalog._nodesMap.begin();
               it != _catalog._nodesMap.end(); ++it )
         {
            BSONObj objNode ;
            rc = (*it).second->obj ( objNode ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to dump info for catNetworkCatalog" ) ;
               goto error ;
            }
            ba.append ( objNode ) ;
         }
         ob.append ( CAT_NODES_FIELDNAME, ba.arr() ) ;
         obj = ob.obj() ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for catNetworkCatalog: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetworkAuth::obj ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder ob ;
      BSONArrayBuilder ba ;
      map<CAT_NODE_ID, catDataNodeInfo*>::const_iterator it ;
      CATNETWORKAUTH_SLOCK
      try
      {
         ob.append ( CAT_TYPE_FIELDNAME,
                     std::string ( "" + CAT_AUTH_VALUENAME ) ) ;
         for ( it = _auth._nodesMap.begin();
               it != _auth._nodesMap.end(); ++it )
         {
            BSONObj objNode ;
            rc = (*it).second->obj ( objNode ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to dump info for catNetworkAuth" ) ;
               goto error ;
            }
            ba.append ( objNode ) ;
         }
         ob.append ( CAT_NODES_FIELDNAME, ba.arr() ) ;
         obj = ob.obj() ;
      }
      catch ( std::exception &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to dump info for catNetworkAuth: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _catNetwork::obj ( BSONObj &coordObj, BSONObj &catalogObj,
                            BSONObj &authObj, vector<BSONObj> &dataObj )
   {
      INT32 rc = SDB_OK ;
      CATNETWORK_SLOCK
      rc = _coord.obj ( coordObj ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to get coord obj" ) ;
         goto error ;
      }
      rc = _catalog.obj ( catalogObj ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to get catalog obj" ) ;
         goto error ;
      }
      rc = _auth.obj ( authObj ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to get auth obj" ) ;
         goto error ;
      }
      rc = _data.obj ( dataObj ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to get data obj" ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 _catNetwork::resync ( BOOLEAN isInitial )
   {
      INT32 rc = SDB_OK ;
      vector<BSONObj> result ;
      //rc = catCatalog::findAllNetwork ( result ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to find all network" ) ;
         goto error ;
      }
      for ( UINT32 i = 0; i < result.size(); ++i )
      {
         // we use "add" mode ( FALSE ) when initialize network
         // otherwise we use "update" mode ( TRUE )
         rc = addupdate ( result[i], !isInitial ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to update, rc = %d", rc ) ;
            goto error ;
         }
      }
   done :
      return rc ;
   error :
      goto done ;
   }
}
