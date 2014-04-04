#ifndef CATALOGUECB_HPP_
#define CATALOGUECB_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "pmdDef.hpp"
#include "msg.hpp"
#include "netRouteAgent.hpp"
#include "msgCatalog.hpp"

namespace engine
{

   class sdbCatalogueCB : public SDBObject
   {
      friend class catMainController ;

      typedef std::map<UINT32, UINT32>    GRP_ID_MAP;
      typedef std::map<UINT16, UINT16>    NODE_ID_MAP;

      public:
         sdbCatalogueCB()
         {
            _routeID.value = 0;
            _strHostName = "";
            _strCatServiceName = "";
            _pNetWork = NULL;
            _iCurNodeId = CAT_DATA_NODE_ID_BEGIN;
            _iCurGrpId = CAT_DATA_GROUP_ID_BEGIN;
            _curCataNodeId = CATA_NODE_ID_BEGIN;
         }
         ~sdbCatalogueCB()
         {
            if ( _pNetWork != NULL )
            {
               SDB_OSS_DEL _pNetWork;
               _pNetWork = NULL;
            }
         }
         void     insertGroupID( UINT32 grpID, BOOLEAN isActive = TRUE ) ;
         void     removeGroupID( UINT32 grpID ) ;
         void     insertNodeID( UINT16 nodeID ) ;
         void     activeGroup( UINT32 groupID ) ;
         UINT32   AllocGroupID() ;
         INT32    getAGroupRand( INT32 &groupID) ;
         UINT16   AllocNodeID();
         void     releaseNodeID( UINT16 nodeID );
         void     setAddr( const _MsgRouteID &id,
                           const CHAR *host,
                           const CHAR *service );
         UINT16   AllocCataNodeID();
         void     insertCataNodeID( UINT16 nodeID );
         void     releaseCataNodeID( UINT16 nodeID );
         void     updateRouteID ( const _MsgRouteID &id ) ;

         _netRouteAgent* netWork()
         {
            return _pNetWork;
         }
         BOOLEAN isPrimary();

      protected:
         INT32 init( _netMsgHandler *handler );

      private:
         _netRouteAgent       *_pNetWork;
         _MsgRouteID          _routeID;
         std::string          _strHostName;
         std::string          _strCatServiceName;
         NODE_ID_MAP          _nodeIdMap;
         NODE_ID_MAP          _cataNodeIdMap;
         GRP_ID_MAP           _grpIdMap;
         GRP_ID_MAP           _deactiveGrpIdMap;
         UINT16               _iCurNodeId;
         UINT16               _curCataNodeId;
         ossSpinSLatch        _GrpIDMutex;
         UINT32               _iCurGrpId;
   };

}
#endif
