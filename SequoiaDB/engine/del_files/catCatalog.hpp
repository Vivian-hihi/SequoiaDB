#ifndef CATCATALOG_HPP__
#define CATCATALOG_HPP__

#include "core.hpp"
#include "ossLatch.hpp"
#include "catNet.hpp"
#include "catCollection.hpp"
#include "ossASIO.hpp"
#include "../bson/bson.h"
#include <vector>
struct _MsgHeader ;
namespace engine
{
   struct _pmdEDUCB ;
   struct _SDB_SHARDCB ;
   struct _pmdSessionID ;
   // this structure is used by COORD node ONLY except those static functions
   struct _catCatalog
   {
   private :
#ifdef CATCATALOG_XLOCK
#undef CATCATALOG_XLOCK
#endif
#define CATCATALOG_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
#ifdef CATCATALOG_SLOCK
#undef CATCATALOG_SLOCK
#endif
#define CATCATALOG_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
      ossSpinSLatch _mutex ;
      // Network Topology
      map<tcp::socket*, catDataNodeInfo*> _dataNodesMap ;
      map<tcp::socket*, catDataNodeInfo*> _catalogNodesMap ;
      map<tcp::socket*, catDataNodeInfo*> _authNodesMap ;
      map<tcp::socket*, catCoordNodeInfo*> _coordNodesMap ;
      template<class T>
      void _findTimeoutNodes ( vector<T> &dataNodes,
                               map<tcp::socket*, T> &nodes ) ;
      catNetwork *_network ;
      catCollection *_collection ;

      INT32 _handShakeNode ( CHAR *pHostName, CHAR *pServiceName,
                             INT32 dbRole, tcp::socket *sock ) ;
      INT32 _handShakeAuth ( CHAR *pHostName, CHAR *pServiceName,
                             tcp::socket *sock ) ;
      INT32 _handShakeCoord ( CHAR *pHostName, CHAR *pServiceName,
                              tcp::socket *sock ) ;
      INT32 _handShakeData ( CHAR *pHostName, CHAR *pServiceName,
                             tcp::socket *sock ) ;
      INT32 _handShakeCatalog ( CHAR *pHostName, CHAR *pServiceName,
                                tcp::socket *sock ) ;
   public :
      INT32 onHeartBeat ( tcp::socket * sock ) ;
      INT32 onHandShake ( CHAR *pHostName, CHAR *pServiceName,
                          INT32 dbRole, tcp::socket *sock ) ;
      void addDataNode ( catDataNodeInfo *dataNode, tcp::socket *sock ) ;
      void addCatalogNode ( catDataNodeInfo *dataNode, tcp::socket *sock ) ;
      void addAuthNode ( catDataNodeInfo *dataNode, tcp::socket *sock ) ;
      void addCoordNode ( catCoordNodeInfo *coordNode, tcp::socket *sock ) ;
      void findTimeoutNodes ( vector<catDataNodeInfo*> &dataNodes,
                              vector<catDataNodeInfo*> &catalogNodes,
                              vector<catDataNodeInfo*> &authNodes,
                              vector<catCoordNodeInfo*> &coordNodes ) ;
      void init ( catNetwork *network, catCollection *collection ) ;


   public :
      // these static functions are used by both catalog and non-catalog, they
      // are used to send communication messages ( we may want to use RTN
      // component later when they are implemented )
      static INT32 findReplGroup ( CAT_REPLGROUP_ID replGroupID,
                                   BSONObj &result ) ;
      static INT32 findAllReplGroup ( vector<BSONObj> &result ) ;
      static INT32 findAllNetwork ( vector<BSONObj> &result ) ;
   private :
      static INT32 runCatalogSimpleQuery ( BSONObj &matcher,
                                           const CHAR *pCollectionName,
                                           vector<BSONObj> result ) ;
      static INT32 _receiveReply ( CHAR *reply, vector<BSONObj> &resultList,
                                   INT64 &contextID ) ;
      static INT32 _readCatalog ( _MsgHeader *pMessage, _SDB_SHARDCB *shardcb,
                                  _pmdSessionID *sessionID, _pmdEDUCB *cb,
                                  vector<BSONObj> &resultList ) ;

   } ;
   typedef struct _catCatalog catCatalog ;
}

#endif
