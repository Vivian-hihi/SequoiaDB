#ifndef CATNET_HPP__
#define CATNET_HPP__
#include <map>
#include <set>
#include "core.hpp"
#include "cat.hpp"
#include "ossSocket.hpp"
#include "ossASIO.hpp"
#include "ossLatch.hpp"
#include "../bson/bson.h"
using namespace std ;
using namespace bson ;
namespace engine
{
#define CAT_TYPE_FIELDNAME    "Type"
#define CAT_SHARD_VALUENAME   'S'
#define CAT_COORD_VALUENAME   'O'
#define CAT_CATALOG_VALUENAME 'C'
#define CAT_AUTH_VALUENAME    'A'
#define CAT_ID_FIELDNAME      "ID"
#define CAT_NODES_FIELDNAME   "Nodes"
#define CAT_HOST_FIELDNAME    "Host"
#define CAT_SERVICE_FIELDNAME "Service"
#define CAT_STATUS_FIELDNAME  "Status"
#define CAT_ROLE_FIELDNAME    "Role"

#define CAT_STATUS_RC         2
#define CAT_STATUS_PEER       1
#define CAT_STATUS_OFFLINE    0
#define CAT_ROLE_PRIMARY      1
#define CAT_ROLE_SECONDARY    0

   typedef INT32 CAT_NODE_ID ;
   typedef INT32 CAT_REPLGROUP_ID ;
#define CAT_CATALOG_REPLGROUP_ID -1
   // information for each single node
   class _catNodeInfo
   {
   private:
#ifdef CATNODEINFO_XLOCK
#undef CATNODEINFO_XLOCK
#endif
#define CATNODEINFO_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATNODEINFO_SLOCK
#undef CATNODEINFO_SLOCK
#endif
#define CATNODEINFO_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
   public:
      CAT_NODE_ID _nodeID ;
      CHAR _hostName [ OSS_MAX_HOSTNAME+1 ] ;
      CHAR _service [ OSS_MAX_SERVICENAME+1 ] ;
      INT32 _status ;
      // the element is only used by catalog node
      ossTick _lastHeartBeatTick ;
      _catNodeInfo ()
      {
         ossMemset ( _hostName, 0, sizeof(_hostName) ) ;
         ossMemset ( _service, 0, sizeof(_service) ) ;
         _lastHeartBeatTick.clear() ;
      }
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 set ( INT32 id, BSONElement &be ) ;
      BOOLEAN operator==(const _catNodeInfo &rhs)
      {
         return
                0 == ossStrncmp ( _hostName, rhs._hostName, OSS_MAX_HOSTNAME) &&
                0 == ossStrncmp ( _service, rhs._service, OSS_MAX_SERVICENAME) ;
      }
   } ;
   typedef class _catNodeInfo catNodeInfo ;

   // for master node type and slave
   enum CAT_DATA_ROLE
   {
      CAT_DATA_PRIMARY = 0,
      CAT_DATA_SECONDARY
   } ;
   class _catReplGroup ;
   // data node
   class _catDataNodeInfo
   {
   private:
#ifdef CATDATANODEINFO_XLOCK
#undef CATDATANODEINFO_XLOCK
#endif
#define CATDATANODEINFO_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATDATANODEINFO_SLOCK
#undef CATDATANODEINFO_SLOCK
#endif
#define CATDATANODEINFO_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
      _catReplGroup *_parent ;
   public:
      explicit _catDataNodeInfo ( _catReplGroup *parent )
      {
         _parent = parent ;
      }
      catNodeInfo _node ;
      CAT_DATA_ROLE _role ;
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 set ( INT32 id, BSONElement &be, CAT_DATA_ROLE &role ) ;
      INT32 obj ( BSONObj &obj ) ;
      BOOLEAN match ( CHAR *pHostName, CHAR *pServiceName ) ;
      friend class _catReplGroup ;
   } ;
   typedef class _catDataNodeInfo catDataNodeInfo ;

   class _catNetworkCoord ;
   // coord node info
   class _catCoordNodeInfo
   {
   private:
#ifdef CATCOORDNODEINFO_XLOCK
#undef CATCOORDNODEINFO_XLOCK
#endif
#define CATCOORDNODEINFO_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATCOORDNODEINFO_SLOCK
#undef CATCOORDNODEINFO_SLOCK
#endif
#define CATCOORDNODEINFO_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
   public:
      catNodeInfo _node ;
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 set ( INT32 id, BSONElement &e ) ;
      INT32 obj ( BSONObj &obj ) ;
      BOOLEAN match ( CHAR *pHostName, CHAR *pServiceName ) ;
      friend class _catNetworkCoord ;
   } ;
   typedef class _catCoordNodeInfo catCoordNodeInfo ;

   // replication group is only for data type
   // this is also called "shard"
#define CAT_REPLGROUP_MASTER_NONE -1
   class _catReplGroup
   {
   private:
#ifdef CATREPLGROUP_XLOCK
#undef CATREPLGROUP_XLOCK
#endif
#define CATREPLGROUP_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATREPLGROUP_SLOCK
#undef CATREPLGROUP_SLOCK
#endif
#define CATREPLGROUP_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
   public:
      CAT_REPLGROUP_ID _replGroupID ;
      // this one is the master id
      // there's no master when this value = CAT_REPLGROUP_MASTER_NONE
      CAT_NODE_ID _currentMasterID ;
      map<CAT_NODE_ID, catDataNodeInfo*> _nodesMap ;
      vector<catDataNodeInfo*> _removedNodes ;
      _catReplGroup()
      {
         _currentMasterID = CAT_REPLGROUP_MASTER_NONE ;
      }
      ~_catReplGroup ()
      {
         map<CAT_NODE_ID, catDataNodeInfo*>::iterator it ;
         for ( it = _nodesMap.begin(); it != _nodesMap.end(); ++it )
         {
            delete (*it).second ;
         }
         _nodesMap.clear() ;
         for ( UINT32 i = 0 ; i < _removedNodes.size(); ++i )
         {
            delete _removedNodes[i] ;
         }
         _removedNodes.clear() ;
      }
      INT32 set ( INT32 id, BSONElement &e ) ;
      INT32 update ( INT32 id, BSONElement &be ) ;
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 obj ( BSONObjBuilder &ob ) ;
      INT32 find ( CHAR *pHostName, CHAR *pServiceName,
                   catDataNodeInfo **nodeInfo ) ;
      void demotePrimary ( CAT_NODE_ID &oldPrimaryNodeID ) ;
      void promotePrimary ( CAT_NODE_ID primaryNodeID,
                            CAT_NODE_ID &oldPrimaryNodeID ) ;
      INT32 size ()
      {
         CATREPLGROUP_SLOCK
         return _nodesMap.size() ;
      }
   } ;
   typedef class _catReplGroup catReplGroup ;

   struct _pmdReplGroup ;
   // all data partition info
   class _catNetworkData
   {
   private:
#ifdef CATNETWORKDATA_XLOCK
#undef CATNETWORKDATA_XLOCK
#endif
#define CATNETWORKDATA_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATNETWORKDATA_SLOCK
#undef CATNETWORKDATA_SLOCK
#endif
#define CATNETWORKDATA_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
      INT32 _add ( INT32 id, BSONObj &obj ) ;
      INT32 _update ( INT32 id, BSONObj &obj, catReplGroup* replGroup ) ;
   public:
      map<CAT_REPLGROUP_ID, catReplGroup*> _replGroupMap ;
      ~_catNetworkData ()
      {
         map<CAT_REPLGROUP_ID, catReplGroup*>::iterator it ;
         for ( it = _replGroupMap.begin(); it != _replGroupMap.end(); ++it )
         {
            delete (*it).second ;
         }
         _replGroupMap.clear() ;
      }
      INT32 update ( INT32 id, BSONObj &obj ) ;
      INT32 add ( INT32 id, BSONObj &obj ) ;
      // given replication group id, build replGroup structure
      INT32 find ( CAT_REPLGROUP_ID id, _pmdReplGroup *group ) ;
      // given hostname and servicename, return node info pointer
      INT32 find ( CHAR *pHostName, CHAR *pServiceName,
                   catDataNodeInfo **nodeInfo ) ;
      INT32 getAllReplGroups ( set<CAT_REPLGROUP_ID> &replGroups ) ;
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 obj ( vector<BSONObj> &obj ) ;
   } ;
   typedef class _catNetworkData catNetworkData ;

   // coord information
   class _catNetworkCoord
   {
   private:
#ifdef CATNETWORKCOORD_XLOCK
#undef CATNETWORKCOORD_XLOCK
#endif
#define CATNETWORKCOORD_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATNETWORKCOORD_SLOCK
#undef CATNETWORKCOORD_SLOCK
#endif
#define CATNETWORKCOORD_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
   public:
      map<CAT_NODE_ID, catCoordNodeInfo*> _coordMap ;
      vector<catCoordNodeInfo*> _removedNodes ;
      ~_catNetworkCoord ()
      {
         map<CAT_NODE_ID, catCoordNodeInfo*>::iterator it ;
         for ( it = _coordMap.begin(); it != _coordMap.end(); ++it )
         {
            delete (*it).second ;
         }
         _coordMap.clear() ;
         for ( UINT32 i = 0 ; i < _removedNodes.size(); ++i )
         {
            delete _removedNodes[i] ;
         }
         _removedNodes.clear() ;
      }
      // given hostname and servicename, return node info pointer
      INT32 find ( CHAR *pHostName, CHAR *pServiceName,
                   catCoordNodeInfo **nodeInfo ) ;
      INT32 add ( BSONObj &obj ) ;
      INT32 update ( BSONObj &obj ) ;
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 obj ( BSONObj &obj ) ;
   } ;
   typedef class _catNetworkCoord catNetworkCoord ;

   // catalog information
   class _catNetworkCatalog
   {
   private:
#ifdef CATNETWORKCATALOG_XLOCK
#undef CATNETWORKCATALOG_XLOCK
#endif
#define CATNETWORKCATALOG_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATNETWORKCATALOG_SLOCK
#undef CATNETWORKCATALOG_SLOCK
#endif
#define CATNETWORKCATALOG_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
   public:
      catReplGroup _catalog ;
      CAT_NODE_ID _previousAttemptNode ;
      INT32 add ( BSONObj &obj ) ;
      INT32 update ( BSONObj &obj ) ;
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 obj ( BSONObj &obj ) ;
      INT32 get ( _pmdReplGroup *group ) ;
      void demotePrimary ( CAT_NODE_ID &oldPrimaryNodeID ) ;
      void promotePrimary ( CAT_NODE_ID primaryNodeID,
                            CAT_NODE_ID &oldPrimaryNodeID ) ;
      INT32 find ( CHAR *pHostName, CHAR *pServiceName,
                   catDataNodeInfo **nodeInfo ) ;
      INT32 size ()
      {
         return _catalog.size() ;
      }
      CAT_NODE_ID getNextAttemptNode () ;
   } ;
   typedef class _catNetworkCatalog catNetworkCatalog ;

   // authentication information
   class _catNetworkAuth
   {
   private:
#ifdef CATNETWORKAUTH_XLOCK
#undef CATNETWORKAUTH_XLOCK
#endif
#define CATNETWORKAUTH_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATNETWORKAUTH_SLOCK
#undef CATNETWORKAUTH_SLOCK
#endif
#define CATNETWORKAUTH_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
   public:
      catReplGroup _auth ;
      INT32 find ( CHAR *pHostName, CHAR *pServiceName,
                   catDataNodeInfo **nodeInfo ) ;
      INT32 add ( BSONObj &obj ) ;
      INT32 update ( BSONObj &obj ) ;
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 obj ( BSONObj &obj ) ;
   } ;
   typedef class _catNetworkAuth catNetworkAuth ;

   // for entire network topology, we have data, coord, catalog and
   // authentication components
   // in data, we have a map of replication groups, in each replication group we
   // have one or more data nodes. One of them is master and others are
   // secondary
   // in coord, we have a map of coordNodeInfo, which contains the ID and host
   // for each node
   // Catalog and Authentication can be both considered as a replication group
   struct _catNetwork
   {
   public:
      catNetworkData *getData ()
      {
         return &_data ;
      }
      catNetworkCoord *getCoord ()
      {
         return &_coord ;
      }
      catNetworkCatalog *getCatalog ()
      {
         return &_catalog ;
      }
      catNetworkAuth *getAuth ()
      {
         return &_auth ;
      }
      INT32 addupdate ( BSONObj &obj, BOOLEAN isUpdate ) ;
      INT32 dumpInfo ( BSONObj &obj ) ;
      INT32 getAllReplGroups ( set<CAT_REPLGROUP_ID> &replGroups ) ;
      void getCatalogReplGroup ( CAT_REPLGROUP_ID &replGroup ) ;
      INT32 init () ;
      INT32 obj ( BSONObj &coordObj, BSONObj &catalogObj, BSONObj &authObj,
                  vector<BSONObj> &dataObj ) ;
      INT32 resync ( BOOLEAN isInitial ) ;
   private:
#ifdef CATNETWORK_XLOCK
#undef CATNETWORK_XLOCK
#endif
#define CATNETWORK_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATNETWORK_SLOCK
#undef CATNETWORK_SLOCK
#endif
#define CATNETWORK_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;

      catNetworkData _data ;
      catNetworkCoord _coord ;
      catNetworkCatalog _catalog ;
      catNetworkAuth _auth ;
   } ;
   typedef struct _catNetwork catNetwork ;
}

#endif
