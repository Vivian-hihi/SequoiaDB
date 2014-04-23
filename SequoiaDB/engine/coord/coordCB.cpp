#include "coordCB.hpp"
#include "pmd.hpp"
#include "ossTypes.h"
#include "pdTrace.hpp"
#include "coordTrace.hpp"
#include "coordDef.hpp"

using namespace bson;
namespace engine
{

   /*
   note: _CoordGroupInfo implement
   */
   _CoordGroupInfo::_CoordGroupInfo ( UINT32 groupID )
   :_groupItem( groupID )
   {
   }

   _CoordGroupInfo::~_CoordGroupInfo ()
   {
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_COORDGI_FRMBSONOBJ, "CoordGroupInfo::fromBSONObj" )
   INT32 _CoordGroupInfo::fromBSONObj( const bson::BSONObj &boGroupInfo )
   {
      PD_TRACE_ENTRY ( SDB_COORDGI_FRMBSONOBJ ) ;

      INT32 rc = _groupItem.updateGroupItem( boGroupInfo ) ;
      PD_RC_CHECK( rc, PDERROR, "Update group info failed, rc: %d", rc ) ;

   done:
      PD_TRACE_EXITRC ( SDB_COORDGI_FRMBSONOBJ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   void _CoordGroupInfo::setPrimary( const MsgRouteID &ID )
   {
      ossScopedRWLock lock( &_primaryMutex, EXCLUSIVE ) ;
      _groupItem.updatePrimary( ID, TRUE ) ;
   }

   void _CoordGroupInfo::setSlave( const MsgRouteID & ID )
   {
      ossScopedRWLock lock( &_primaryMutex, EXCLUSIVE ) ;
      _groupItem.updatePrimary( ID, FALSE ) ;
   }

   INT32 CoordCataInfo::getMatchGroups( const bson::BSONObj &matcher,
                                       CoordGroupList &groupLst )
   {
      INT32 rc = SDB_OK ;
      UINT32 i = 0 ;
      UINT32 groupID = 0 ;
      VEC_GROUP_ID vecGroup;
      rc = _catlogSet.findGroupIDS( matcher, vecGroup );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to get match groups(rc=%d)",
                  rc );
      SDB_ASSERT( vecGroup.size() != 0, "no match groups!" )
      PD_CHECK( vecGroup.size() != 0, SDB_CAT_NO_MATCH_CATALOG, error,
               PDERROR, "couldn't find match group" );
      for ( i = 0; i < vecGroup.size(); i++ )
      {
         groupID = vecGroup[i] ;
         groupLst[ groupID ] = groupID ;
      }
   done:
      return rc;
   error:
      goto done;
   }

   /*
   note: _CoordCB implement
   */
   void _CoordCB::updateCatGroupInfo( CoordGroupInfoPtr &groupInfo )
   {
      ossScopedLock _lock(&_mutex, EXCLUSIVE) ;
      _catGroupInfo = groupInfo;
   }

   void _CoordCB::getCatNodeAddrList ( CoordVecNodeInfo &catNodeLst )
   {
      catNodeLst = _cataNodeAddrList;
   }

   void _CoordCB::clearCatNodeAddrList()
   {
      _cataNodeAddrList.clear();
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_COORDCB_ADDCATNDADDR, "CoordCB::addCatNodeAddr" )
   INT32 _CoordCB::addCatNodeAddr( const _MsgRouteID &id,
                                   const CHAR *pHost,
                                   const CHAR *pService )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_COORDCB_ADDCATNDADDR ) ;
      CoordNodeInfo nodeInfo ;
      nodeInfo._id = id ;
      nodeInfo._id.columns.groupID = 0 ;
      ossStrncpy( nodeInfo._host, pHost, OSS_MAX_HOSTNAME );
      nodeInfo._host[OSS_MAX_HOSTNAME] = 0 ;
      nodeInfo._service[MSG_ROUTE_CAT_SERVICE] = pService ;

      _cataNodeAddrList.push_back( nodeInfo ) ;
      _multiRouteAgent.updateRoute( nodeInfo._id,
                                    nodeInfo._host,
                                    nodeInfo._service[MSG_ROUTE_CAT_SERVICE].c_str() );
      PD_TRACE_EXIT ( SDB_COORDCB_ADDCATNDADDR );
      return rc;
   }

   INT32 _CoordCB::_addGroupName ( const std::string& name, UINT32 id )
   {
      INT32 rc = SDB_OK ;
      GROUP_NAME_MAP_IT it = _groupNameMap.find ( name ) ;
      if ( it != _groupNameMap.end() )
      {
         // the same
         if ( it->second == id )
         {
            rc = SDB_OK ;
            goto done ;
         }
         rc = SDB_CLS_GROUP_NAME_CONFLICT ;
         goto done ;
      }
      _groupNameMap[name] = id ;

   done :
      return rc ;
   }

   INT32 _CoordCB::_clearGroupName( UINT32 id )
   {
      GROUP_NAME_MAP_IT it = _groupNameMap.begin() ;
      while ( it != _groupNameMap.end() )
      {
         if ( it->second == id )
         {
            _groupNameMap.erase( it ) ;
            break ;
         }
         ++it ;
      }
      return SDB_OK ;
   }

}

