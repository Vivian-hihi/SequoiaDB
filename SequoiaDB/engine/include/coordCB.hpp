
#ifndef COORDCB_HPP__
#define COORDCB_HPP__

#include "core.hpp"
#include "oss.hpp"
#include <map>
#include <boost/shared_ptr.hpp>
#include "netRouteAgent.hpp"
#include "netMultiRouteAgent.hpp"
#include "rtnCoord.hpp"
#include "ossUtil.h"
#include "rtnPredicate.hpp"
#include "msgCatalog.hpp"
#include "clsCatalogAgent.hpp"
#include "coordDef.hpp"
#include "sdbInterface.hpp"

namespace engine
{
   /*
      _CoordCB define
   */
   class _CoordCB : public _IControlBlock
   {
      typedef std::map<std::string, UINT32>     GROUP_NAME_MAP ;
      typedef GROUP_NAME_MAP::iterator          GROUP_NAME_MAP_IT ;

   public:
      _CoordCB() ;
      virtual ~_CoordCB() ;

      virtual SDB_CB_TYPE cbType() const { return SDB_CB_COORD ; }
      virtual const CHAR* cbName() const { return "COORDCB" ; }

      virtual INT32  init () ;
      virtual INT32  active () ;
      virtual INT32  deactive () ;
      virtual INT32  fini () ;
      virtual void   onConfigChange() ;

      _netRouteAgent* netWork()
      {
         return _pNetWork;
      }

      netMultiRouteAgent* getRouteAgent()
      {
         return &_multiRouteAgent;
      }

      rtnCoordProcesserFactory *getProcesserFactory()
      {
         return &_processerFactory;
      }

      INT32 addCatNodeAddr( const _MsgRouteID &id,
                            const CHAR *pHost,
                            const CHAR *pService );

      void getCatNodeAddrList ( CoordVecNodeInfo &catNodeLst );

      void clearCatNodeAddrList();

      void updateCatGroupInfo( CoordGroupInfoPtr &groupInfo );

      CoordGroupInfoPtr getCatGroupInfo()
      {
         CoordGroupInfoPtr catGroupInfoTmp;
         {
         ossScopedLock _lock(&_mutex, SHARED) ;
         catGroupInfoTmp = _catGroupInfo;
         }
         return catGroupInfoTmp;
      }

      MsgRouteID getPrimaryCat()
      {
         ossScopedLock _lock(&_mutex, SHARED) ;
         return _catGroupInfo->getPrimary();
      }

      void setSlaveCat( MsgRouteID slave )
      {
         ossScopedLock _lock(&_mutex, EXCLUSIVE) ;
         _catGroupInfo->setSlave( slave );
      }

      void setPrimaryCat( MsgRouteID primary )
      {
         ossScopedLock _lock(&_mutex, EXCLUSIVE) ;
         _catGroupInfo->setPrimary( primary );
      }

      INT32 groupID2Name ( UINT32 id, std::string &name )
      {
         ossScopedLock _lock( &_nodeGroupMutex, SHARED ) ;

         CoordGroupMap::iterator it = _nodeGroupInfo.find( id ) ;
         if ( it == _nodeGroupInfo.end() )
         {
            return SDB_COOR_NO_NODEGROUP_INFO ;
         }
         name = it->second->groupName() ;

         return SDB_OK ;
      }

      INT32 groupName2ID ( const CHAR* name, UINT32 &id )
      {
         ossScopedLock _lock( &_nodeGroupMutex, SHARED ) ;

         GROUP_NAME_MAP::iterator it = _groupNameMap.find( name ) ;
         if ( it == _groupNameMap.end() )
         {
            return SDB_COOR_NO_NODEGROUP_INFO ;
         }
         id = it->second ;

         return SDB_OK ;
      }

      void  addGroupInfo ( CoordGroupInfoPtr &groupInfo )
      {
         // TODO:delete the outTime groupInfo
         // TODO:check version
         ossScopedLock _lock( &_nodeGroupMutex, EXCLUSIVE ) ;

         _nodeGroupInfo[groupInfo->getGroupID()] = groupInfo ;

         // clear group name map
         _clearGroupName( groupInfo->getGroupID() ) ;

         // add to group name map
         _addGroupName( groupInfo->groupName(), groupInfo->getGroupID() ) ;
      }

      void  removeGroupInfo( UINT32 groupID )
      {
         ossScopedLock _lock(&_nodeGroupMutex, EXCLUSIVE) ;
         _nodeGroupInfo.erase( groupID ) ;

         // clear group name map
         _clearGroupName( groupID ) ;
      }

      INT32 getGroupInfo ( UINT32 groupID, CoordGroupInfoPtr &groupInfo )
      {
         INT32 rc = SDB_OK;
         ossScopedLock _lock( &_nodeGroupMutex, SHARED ) ;
         CoordGroupMap::iterator iter = _nodeGroupInfo.find ( groupID );
         if ( _nodeGroupInfo.end() == iter )
         {
            rc = SDB_COOR_NO_NODEGROUP_INFO;
         }
         else
         {
            groupInfo = iter->second;
         }
         return rc;
      }

      INT32 getGroupInfo ( const CHAR *groupName, CoordGroupInfoPtr &groupInfo )
      {
         UINT32 groupID = 0 ;
         INT32 rc = groupName2ID( groupName, groupID ) ;
         if ( SDB_OK == rc )
         {
            rc = getGroupInfo( groupID, groupInfo ) ;
         }
         return rc ;
      }

      void updateCataInfo ( const std::string &collectionName,
                            CoordCataInfoPtr &cataInfo )
      {
         // TODO:update catalogue info
         // TODO:delete the outTime groupInfo
         ossScopedLock _lock( &_cataInfoMutex, EXCLUSIVE );
         _cataInfoMap[collectionName] = cataInfo ;
      }

      INT32 getCataInfo ( const std::string &strCollectionName,
                          CoordCataInfoPtr &cataInfo )
      {
         INT32 rc = SDB_CAT_NO_MATCH_CATALOG;
         ossScopedLock _lock( &_cataInfoMutex, SHARED );
         CoordCataMap::iterator iter
                              = _cataInfoMap.find( strCollectionName );
         if ( iter != _cataInfoMap.end() )
         {
            rc = SDB_OK;
            cataInfo = iter->second;
         }
         return rc;
      }

      void delCataInfo ( const std::string &collectionName )
      {
         ossScopedLock _lock( &_cataInfoMutex, EXCLUSIVE );
         _cataInfoMap.erase( collectionName );
      }

   protected:
      INT32         _addGroupName ( const std::string& name, UINT32 id ) ;
      INT32         _clearGroupName ( UINT32 id ) ;

   private:
      _netRouteAgent                      *_pNetWork;
      ossSpinSLatch                       _mutex;
      netMultiRouteAgent                  _multiRouteAgent;
      CoordGroupInfoPtr                   _catGroupInfo;//don't modify while runtime
      rtnCoordProcesserFactory            _processerFactory;

      ossSpinSLatch                       _nodeGroupMutex;
      CoordGroupMap                       _nodeGroupInfo;
      GROUP_NAME_MAP                      _groupNameMap ;

      ossSpinSLatch                       _cataInfoMutex;
      CoordCataMap                        _cataInfoMap;

      CoordVecNodeInfo                    _cataNodeAddrList;

   };
   typedef _CoordCB CoordCB ;

   /*
      get global coord cb
   */
   CoordCB* sdbGetCoordCB() ;

}

#endif // COORDCB_HPP__

