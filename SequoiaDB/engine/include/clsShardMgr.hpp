/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsShardMgr.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          22/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_SHD_MGR_HPP_
#define CLS_SHD_MGR_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "clsObjBase.hpp"
#include "netRouteAgent.hpp"
#include "ossEvent.hpp"
#include "ossLatch.hpp"
#include "clsCatalogAgent.hpp"
#include <map>

using namespace bson ;

namespace engine
{
   struct _catlogServerInfo : public SDBObject
   {
      NodeID      nodeID ;
      std::string host ;
      std::string service ;
   } ;
   typedef std::vector<_catlogServerInfo>             VECCATLOG ;

   class _clsEventItem : public SDBObject
   {
   public :
      BOOLEAN        send ;
      BOOLEAN        reSend ;
      UINT32         waitNum ;
      ossEvent       event ;
      UINT64         requestID ;
      std::string    name ;
      UINT32         groupID ;

      _clsEventItem ()
      {
         send = FALSE ;
         reSend = FALSE ;
         waitNum = 0 ;
         requestID = 0 ;
         groupID = 0 ;
      }
   } ;
   typedef class _clsEventItem clsEventItem ;

   class _clsCSEventItem : public SDBObject
   {
      public:
         std::string    csName ;
         ossEvent       event ;
         UINT32         pageSize ;

         _clsCSEventItem()
         {
            pageSize = 0 ;
         }
   } ;
   typedef _clsCSEventItem clsCSEventItem ;

   class _clsShardMgr :  public _clsObjBase
   {
      typedef std::map<std::string, clsEventItem*>       MAP_CAT_EVENT ;
      typedef MAP_CAT_EVENT::iterator                    MAP_CAT_EVENT_IT ;

      typedef std::map<UINT32, clsEventItem*>            MAP_NM_EVENT ;
      typedef MAP_NM_EVENT::iterator                     MAP_NM_EVENT_IT ;

      typedef std::map<UINT64, clsCSEventItem*>          MAP_CS_EVENT ;
      typedef MAP_CS_EVENT::iterator                     MAP_CS_EVENT_IT ;

      DECLARE_OBJ_MSG_MAP()

      public:
         _clsShardMgr( _netRouteAgent *rtAgent );
         virtual ~_clsShardMgr();

         virtual INT32    initialize() ;
         virtual INT32    active () ;
         virtual INT32    final() ;
         virtual void     onTimer ( UINT32 timerID, UINT32 interval ) ;

         void setCatlogInfo ( const NodeID &id, const std::string& host,
                              const std::string& service ) ;
         void setNodeID ( const MsgRouteID& nodeID ) ;

         catAgent* getCataAgent () ;
         nodeMgrAgent* getNodeMgrAgent () ;

         INT32 getAndLockCataSet( const CHAR *name, clsCatalogSet **ppSet,
                                  BOOLEAN noWithUpdate = TRUE,
                                  INT64 waitMillSec = OSS_ONE_SEC,
                                  BOOLEAN *pUpdated = NULL ) ;
         INT32 unlockCataSet( clsCatalogSet *catSet ) ;

         INT32 getAndLockGroupItem( UINT32 id, clsGroupItem **ppItem,
                                     BOOLEAN noWithUpdate = TRUE,
                                     INT64 waitMillSec = OSS_ONE_SEC,
                                     BOOLEAN *pUpdated = NULL ) ;
         INT32 unlockGroupItem( clsGroupItem *item ) ;

         INT32 rGetCSPageSize( const CHAR *csName, UINT32 &pageSize,
                               INT64 waitMillSec = OSS_ONE_SEC ) ;

      public:
         INT32  sendToCatlog ( MsgHeader * msg ) ;
         INT32  syncSend( MsgHeader * msg, UINT32 groupID, BOOLEAN primary,
                          MsgHeader **ppRecvMsg, INT64 millisec = OSS_ONE_SEC ) ;
         INT32  updatePrimary ( const NodeID & id , BOOLEAN primary ) ;
         INT32  updateCatGroup ( BOOLEAN unsetPrimary = TRUE ) ;

         INT32 syncUpdateCatalog ( const CHAR *pCollectionName,
                                   INT64 millsec = OSS_ONE_SEC ) ;
         INT32 syncUpdateGroupInfo ( UINT32 groupID,
                                     INT64 millsec = OSS_ONE_SEC ) ;
         NodeID nodeID () const ;
         INT32 clearAllData () ;

      protected:

         INT32 _sendCataQueryReq( INT32 queryType, const BSONObj &query,
                                  UINT64 requestID ) ;

         INT32 _sendCatalogReq ( const CHAR *pCollectionName,
                                 UINT64 requestID = 0 ) ;
         INT32 _sendGroupReq ( UINT32 groupID, UINT64 requestID = 0 ) ;
         INT32 _sendCSInfoReq ( const CHAR *pCSName, UINT64 requestID = 0 ) ;

         clsEventItem *_findCatSyncEvent ( const CHAR *pCollectionName,
                                           BOOLEAN bCreate = FALSE ) ;
         clsEventItem *_findCatSyncEvent ( UINT64 requestID ) ;

         clsEventItem *_findNMSyncEvent ( UINT32 groupID,
                                          BOOLEAN bCreate = FALSE ) ;
         clsEventItem *_findNMSyncEvent ( UINT64 requestID ) ;

         INT32 _findCatNodeID ( const VECCATLOG &catNodes,
                                const std::string &host,
                                const std::string &service,
                                NodeID &id ) ;

      //msg functions
      protected:
         INT32 _onCatCatGroupRes ( NET_HANDLE handle, MsgHeader* msg ) ;
         INT32 _onCatalogReqMsg ( NET_HANDLE handle, MsgHeader* msg ) ;
         INT32 _onCatGroupRes ( NET_HANDLE handle, MsgHeader* msg ) ;
         INT32 _onQueryCSInfoRsp( NET_HANDLE handle, MsgHeader* msg ) ;

      private:
         _netRouteAgent                *_pNetRtAgent ;
         _clsCatalogAgent              *_pCatAgent ;
         _clsNodeMgrAgent              *_pNodeMgrAgent ;
         ossSpinXLatch                 _catLatch ;
         MAP_CAT_EVENT                 _mapSyncCatEvent ;
         MAP_NM_EVENT                  _mapSyncNMEvent ;
         MAP_CS_EVENT                  _mapSyncCSEvent ;
         UINT64                        _requestID ;

         VECCATLOG                     _vecCatlog ;
         INT32                         _primary ;
         UINT32                        _catVerion ;
         ossSpinSLatch                 _shardLatch ;

         MsgRouteID                    _nodeID ;

   };

   typedef _clsShardMgr shardCB ;
}

#endif //CLS_SHD_MGR_HPP_

