#ifndef RTNCOORDINSERT_HPP__
#define RTNCOORDINSERT_HPP__

#include "rtnCoordOperator.hpp"
#include "../bson/bson.h"

namespace engine
{
   class rtnCoordInsert : public rtnCoordTransOperator
   {
   typedef std::vector< CHAR * > ObjQueue;
   typedef std::map< UINT32, ObjQueue > GroupObjsMap;
   typedef std::map< std::string, ObjQueue > SubCLObjsMap;
   typedef std::map< UINT32, SubCLObjsMap >  GroupSubCLMap;
   typedef struct _CoordInsertMsg
   {
      SINT32      dataLen;
      netIOVec    dataList;
      _CoordInsertMsg()
      {
         dataLen = 0;
      }
   }CoordInsertMsg;
   typedef std::map< UINT32, CoordInsertMsg > GroupInsertMsgMap;
   public:
      virtual INT32 execute( CHAR *pReceiveBuffer, SINT32 packSize,
                           CHAR **ppResultBuffer, pmdEDUCB *cb,
                           MsgOpReply &replyHeader, BSONObj **ppErrorObj );
   private:
      INT32 shardDataByGroup( const CoordCataInfoPtr &cataInfo,
                              INT32 count,
                              CHAR *pInsertor,
                              GroupObjsMap &groupObjsMap );
      INT32 shardDataByGroup( const CoordCataInfoPtr &cataInfo,
                              INT32 count,
                              CHAR *pInsertor,
                              pmdEDUCB *cb,
                              GroupSubCLMap &groupSubCLMap );
      INT32 reshardData( const CoordCataInfoPtr &cataInfo,
                        GroupObjsMap &groupObjsMap );
      INT32 reshardData( const CoordCataInfoPtr &cataInfo,
                        pmdEDUCB *cb,
                        GroupSubCLMap &groupSubCLMap );
      INT32 shardAnObj( CHAR *pInsertor,
                        const CoordCataInfoPtr &cataInfo,
                        GroupObjsMap &groupObjsMap );
      INT32 shardAnObj( CHAR *pInsertor,
                        const CoordCataInfoPtr &cataInfo,
                        pmdEDUCB * cb,
                        GroupSubCLMap &groupSubCLMap );
      INT32 insertToAGroup( CHAR *pBuffer,
                           UINT32 grpID,
                           netMultiRouteAgent *pRouteAgent,
                           pmdEDUCB *cb );
      INT32 processReply( REPLY_QUE &replyQue,
                        CoordGroupList &successGroupList,
                        pmdEDUCB *cb );
      INT32 insertToGroups( const GroupInsertMsgMap &groupMsgMap,
                           MsgOpInsert *pSrcMsg,
                           netMultiRouteAgent *pRouteAgent,
                           pmdEDUCB *cb,
                           CoordGroupList &successGroupList );
      INT32 sendToGroups( const GroupInsertMsgMap &groupObjsMap,
                           MsgOpInsert *pSrcMsg,
                           netMultiRouteAgent *pRouteAgent,
                           pmdEDUCB *cb,
                           REQUESTID_MAP &sendNodes );
      INT32 insertToNormalCL( const CoordCataInfoPtr &cataInfo,
                              CHAR *pReceiveBuffer, CHAR *pInsertor,
                              INT32 count, netMultiRouteAgent *pRouteAgent,
                              pmdEDUCB *cb, GroupObjsMap &groupObjsMap,
                              BOOLEAN &hasSendSomeData);
      INT32 insertToMainCL( const CoordCataInfoPtr &cataInfo,
                           CHAR *pReceiveBuffer,
                           CHAR *pInsertor, INT32 count,
                           netMultiRouteAgent *pRouteAgent,
                           pmdEDUCB *cb,
                           GroupSubCLMap &groupSubCLMap );
      INT32 buildInsertMsg( CHAR *pHeadRemain, INT32 remainLen,
                           const GroupSubCLMap &groupSubCLMap,
                           std::vector< bson::BSONObj > &subClInfoLst,
                           void *pFiller,
                           GroupInsertMsgMap &groupMsgMap );
      INT32 buildInsertMsg( CHAR *pHeadRemain, INT32 remainLen,
                           const GroupObjsMap &groupObjsMap,
                           void *pFiller,
                           GroupInsertMsgMap &groupMsgMap );
   };
}

#endif
