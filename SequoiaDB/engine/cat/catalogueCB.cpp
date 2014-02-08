
#include "catalogueCB.hpp"
#include "msgCatalog.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "catTrace.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include <stdlib.h>

namespace engine
{

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_INIT, "sdbCatalogueCB::init" )
   INT32 sdbCatalogueCB::init( _netMsgHandler *handler )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGCB_INIT ) ;
      do
      {
         if ( _pNetWork != NULL )
         {
            SDB_OSS_DEL _pNetWork;
            _pNetWork = NULL;
         }
         // free in destructor
         _pNetWork = SDB_OSS_NEW _netRouteAgent( handler );
         if ( !_pNetWork )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for netRouteAgent" ) ;
            rc = SDB_OOM ;
            break ;
         }
         PD_TRACE1 ( SDB_CATALOGCB_INIT,
                     PD_PACK_ULONG ( _routeID.value ) ) ;
         _pNetWork->setLocalID( _routeID );
         rc = _pNetWork->updateRoute( _routeID,
                                      _strHostName.c_str(),
                                      _strCatServiceName.c_str() );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR, "Failed to update route(routeID=%lld, host=%s, "
                     "service=%s, rc=%d)", _routeID.value, _strHostName.c_str(),
                     _strCatServiceName.c_str(), rc);
            break;
         }
         rc = _pNetWork->listen( _routeID );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR, "Failed to open listen-port(host=%s, service=%s, "
                     "rc=%d)", _strHostName.c_str(), _strCatServiceName.c_str(),
                     rc );
            break;
         }
      }while ( FALSE );
      PD_TRACE_EXITRC ( SDB_CATALOGCB_INIT, rc ) ;
      return rc;
   }

   BOOLEAN sdbCatalogueCB::isPrimary()
   {
      return pmdGetKRCB()->getClsCB()->isPrimary();
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_INSERTGROUPID, "sdbCatalogueCB::insertGroupID" )
   void sdbCatalogueCB::insertGroupID( UINT32 grpID, BOOLEAN isActive )
   {
      PD_TRACE_ENTRY ( SDB_CATALOGCB_INSERTGROUPID ) ;
      PD_TRACE2 ( SDB_CATALOGCB_INSERTGROUPID,
                  PD_PACK_UINT ( grpID ),
                  PD_PACK_UINT ( isActive ) ) ;
      if ( grpID >= CAT_DATA_GROUP_ID_BEGIN )
      {
         ossScopedLock _lock(&_GrpIDMutex, EXCLUSIVE) ;
         if ( isActive )
         {
            _grpIdMap.insert( GRP_ID_MAP::value_type(grpID, grpID) );
         }
         else
         {
            _deactiveGrpIdMap.insert( GRP_ID_MAP::value_type(grpID, grpID) );
         }
         _iCurGrpId = _iCurGrpId > grpID ? _iCurGrpId : ++grpID ;
      }
      PD_TRACE_EXIT ( SDB_CATALOGCB_INSERTGROUPID ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_REMOVEGROUPID, "sdbCatalogueCB::removeGroupID" )
   void sdbCatalogueCB::removeGroupID( UINT32 grpID )
   {
      PD_TRACE_ENTRY ( SDB_CATALOGCB_REMOVEGROUPID ) ;
      PD_TRACE1 ( SDB_CATALOGCB_REMOVEGROUPID,
                  PD_PACK_UINT ( grpID ) ) ;
      if ( grpID >= CAT_DATA_GROUP_ID_BEGIN )
      {
         ossScopedLock _lock(&_GrpIDMutex, EXCLUSIVE) ;
         _grpIdMap.erase(grpID);
         _deactiveGrpIdMap.erase( grpID );
      }
      PD_TRACE_EXIT ( SDB_CATALOGCB_REMOVEGROUPID ) ;
      return ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_ACTIVEGROUP, "sdbCatalogueCB::activeGroup" )
   void sdbCatalogueCB::activeGroup( UINT32 groupID )
   {
      PD_TRACE_ENTRY ( SDB_CATALOGCB_ACTIVEGROUP ) ;
      ossScopedLock _lock(&_GrpIDMutex, EXCLUSIVE) ;
      _deactiveGrpIdMap.erase( groupID );
      _grpIdMap.insert( GRP_ID_MAP::value_type(groupID, groupID) );
      PD_TRACE_EXIT ( SDB_CATALOGCB_ACTIVEGROUP ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_INSERTNODEID, "sdbCatalogueCB::insertNodeID" )
   void sdbCatalogueCB::insertNodeID( UINT16 nodeID )
   {
      PD_TRACE_ENTRY ( SDB_CATALOGCB_INSERTNODEID ) ;
      PD_TRACE1 ( SDB_CATALOGCB_INSERTNODEID, PD_PACK_USHORT ( nodeID ) ) ;
      if ( nodeID >= CAT_DATA_NODE_ID_BEGIN )
      {
         _nodeIdMap.insert( NODE_ID_MAP::value_type(nodeID, nodeID) );
         _iCurNodeId = _iCurNodeId > nodeID ? _iCurNodeId : ++nodeID ;
      }
      else
      {
         _cataNodeIdMap.insert( NODE_ID_MAP::value_type(nodeID, nodeID) );
         _curCataNodeId = _curCataNodeId > nodeID ? _curCataNodeId : ++nodeID ;
      }
      PD_TRACE_EXIT ( SDB_CATALOGCB_INSERTNODEID ) ;
   }

   void sdbCatalogueCB::insertCataNodeID( UINT16 nodeID )
   {
      _cataNodeIdMap.insert( NODE_ID_MAP::value_type(nodeID, nodeID) );
      _curCataNodeId = _curCataNodeId > nodeID ? _curCataNodeId : ++nodeID ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_GETAGROUPRAND, "sdbCatalogueCB::getAGroupRand" )
   INT32 sdbCatalogueCB::getAGroupRand( INT32 &groupID )
   {
      INT32 rc = SDB_CAT_NO_NODEGROUP_INFO ;
      PD_TRACE_ENTRY ( SDB_CATALOGCB_GETAGROUPRAND ) ;
      ossScopedLock _lock( &_GrpIDMutex, EXCLUSIVE ) ;
      UINT32 mapSize = _grpIdMap.size();
      PD_TRACE1 ( SDB_CATALOGCB_GETAGROUPRAND,
                  PD_PACK_UINT ( mapSize ) ) ;
      if ( mapSize > 0 )
      {
         UINT32 randNum = ossRand() % mapSize;
         UINT32 i = 0;
         GRP_ID_MAP::iterator iterMap = _grpIdMap.begin();
         for ( ; i < randNum && iterMap!=_grpIdMap.end(); i++ )
         {
            ++iterMap;
         }
         if ( iterMap != _grpIdMap.end() )
         {
            groupID = iterMap->first;
            rc = SDB_OK ;
         }
      }
      PD_TRACE_EXITRC ( SDB_CATALOGCB_GETAGROUPRAND, rc ) ;
      return rc;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_ALLOCGROUPID, "sdbCatalogueCB::AllocGroupID" )
   UINT32 sdbCatalogueCB::AllocGroupID ()
   {
      INT32 i = 0;
      UINT32 id = 0 ;
      PD_TRACE_ENTRY ( SDB_CATALOGCB_ALLOCGROUPID ) ;
      ossScopedLock _lock(&_GrpIDMutex, EXCLUSIVE) ;
      while ( i++ < CAT_DATA_NODE_MAX_NUM )
      {
         if ( _iCurGrpId < CAT_DATA_GROUP_ID_BEGIN )
         {
            _iCurGrpId = CAT_DATA_GROUP_ID_BEGIN;
         }
         GRP_ID_MAP::const_iterator it = _grpIdMap.find( _iCurGrpId );
         if ( it != _grpIdMap.end() )
         {
            _iCurGrpId++;
            continue;
         }
         it = _deactiveGrpIdMap.find( _iCurGrpId );
         if ( it != _deactiveGrpIdMap.end() )
         {
            _iCurGrpId++;
            continue;
         }
         id = _iCurGrpId ;
         goto done ;
      }
      id = CAT_INVALID_GROUPID ;
   done :
      PD_TRACE1 ( SDB_CATALOGCB_ALLOCGROUPID, PD_PACK_UINT ( id ) ) ;
      PD_TRACE_EXIT ( SDB_CATALOGCB_ALLOCGROUPID ) ;
      return id ;
   }

   void sdbCatalogueCB::releaseNodeID( UINT16 nodeID )
   {
      _nodeIdMap.erase( nodeID );
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_ALLOCCATANODEID, "sdbCatalogueCB::AllocCataNodeID" )
   UINT16 sdbCatalogueCB::AllocCataNodeID()
   {
      INT32 i = 0;
      UINT16 id = 0 ;
      PD_TRACE_ENTRY ( SDB_CATALOGCB_ALLOCCATANODEID ) ;
      while ( i++ < CATA_NODE_MAX_NUM )
      {
         if ( _curCataNodeId >= CAT_DATA_NODE_ID_BEGIN )
         {
            _curCataNodeId = CATA_NODE_ID_BEGIN;
         }
         NODE_ID_MAP::const_iterator it
                           = _cataNodeIdMap.find( _curCataNodeId );
         if ( _cataNodeIdMap.end() == it )
         {
            id = _curCataNodeId ;
            insertCataNodeID( _curCataNodeId );
            goto done ;
         }
         _curCataNodeId++;
      }
      id = CAT_INVALID_NODEID ;
   done :
      PD_TRACE1 ( SDB_CATALOGCB_ALLOCCATANODEID, PD_PACK_USHORT ( id ) ) ;
      PD_TRACE_EXIT ( SDB_CATALOGCB_ALLOCCATANODEID ) ;
      return id ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_ALLOCNODEID, "sdbCatalogueCB::AllocNodeID" )
   UINT16 sdbCatalogueCB::AllocNodeID()
   {
      INT32 i = 0;
      UINT16 id = 0 ;
      PD_TRACE_ENTRY ( SDB_CATALOGCB_ALLOCNODEID ) ;
      while ( i++ < CAT_DATA_NODE_MAX_NUM )
      {
         if ( _iCurNodeId < CAT_DATA_NODE_ID_BEGIN )
         {
            _iCurNodeId = CAT_DATA_NODE_ID_BEGIN;
         }
         NODE_ID_MAP::const_iterator it = _nodeIdMap.find( _iCurNodeId );
         if ( _nodeIdMap.end() == it )
         {
            id = _iCurNodeId ;
            insertNodeID( _iCurNodeId );
            goto done ;
         }
         _iCurNodeId++;
      }
      id = CAT_INVALID_NODEID ;
   done :
      PD_TRACE1 ( SDB_CATALOGCB_ALLOCNODEID, PD_PACK_USHORT ( id ) ) ;
      PD_TRACE_EXIT ( SDB_CATALOGCB_ALLOCNODEID ) ;
      return id;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_SETADDR, "sdbCatalogueCB::setAddr" )
   void sdbCatalogueCB::setAddr( const _MsgRouteID &id,
                                 const CHAR *host,
                                 const CHAR *service)
   {
      PD_TRACE_ENTRY ( SDB_CATALOGCB_SETADDR ) ;
      PD_TRACE3 ( SDB_CATALOGCB_SETADDR, PD_PACK_ULONG ( id.value ),
                  PD_PACK_STRING ( host ), PD_PACK_STRING ( service ) ) ;
      _routeID = id;
      _strHostName = host;
      _strCatServiceName = service;
      PD_TRACE_EXIT ( SDB_CATALOGCB_SETADDR ) ;
   }

   // The caller must make sure id has the correct serviceID
   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGCB_UPDATEROUTEID, "sdbCatalogueCB::updateRouteID" )
   void sdbCatalogueCB::updateRouteID ( const _MsgRouteID &id )
   {
      INT32 rc = SDB_OK ;
      if ( !_pNetWork )
         return ;
      PD_TRACE_ENTRY ( SDB_CATALOGCB_UPDATEROUTEID ) ;
      PD_TRACE1 ( SDB_CATALOGCB_UPDATEROUTEID, PD_PACK_ULONG ( id.value ) ) ;
      rc = _pNetWork->updateRoute( _routeID, id ) ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Failed to update route(routeID=%lld, host=%s, "
                  "service=%s, rc=%d)", _routeID.value, _strHostName.c_str(),
                 _strCatServiceName.c_str(), rc);
      }
      _pNetWork->setLocalID( id );
      _routeID = id ;
      PD_TRACE_EXIT ( SDB_CATALOGCB_UPDATEROUTEID ) ;
   }
}
