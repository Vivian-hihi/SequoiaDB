#ifndef RTNCOORDUPDATE_HPP__
#define RTNCOORDUPDATE_HPP__

#include "rtnCoordOperator.hpp"
#include "../bson/bson.h"

namespace engine
{
   class rtnCoordUpdate : public rtnCoordTransOperator
   {
   public:
      virtual INT32 execute( CHAR *pReceiveBuffer, SINT32 packSize,
                           CHAR **ppResultBuffer, pmdEDUCB *cb,
                           MsgOpReply &replyHeader, BSONObj **ppErrorObj );

   protected:
      INT32 buildOpMsg( const CoordCataInfoPtr &cataInfo,
                              const CoordSubCLlist &subCLList,
                              CHAR *pSrcMsg, CHAR *&pDstMsg,
                              INT32 &bufferSize );
      virtual INT32 checkModifierForSubCL ( const CoordSubCLlist &subCLList,
                                       const CHAR *pUpdator,
                                       pmdEDUCB *cb );
   private:
      INT32 getNodeGroups( const CoordCataInfoPtr &cataInfo,
                           bson::BSONObj &selectObj,
                           CoordGroupList &sendGroupLst,
                           CoordGroupList &groupLst );
      INT32 updateToDataNodeGroup( CHAR *pBuffer,
                                 CoordGroupList &groupLst,
                                 CoordGroupList &sendGroupLst,
                                 netMultiRouteAgent *pRouteAgent,
                                 pmdEDUCB *cb,
                                 INT64 *updateNum = NULL );
      INT32 checkIfIncludeShardingKey ( const CoordCataInfoPtr &cataInfo,
                                       const CHAR *pUpdator,
                                       BOOLEAN &isInclude,
                                       pmdEDUCB *cb );
      INT32 updateNormalCL( CoordCataInfoPtr cataInfo,
                           bson::BSONObj &boSelector,
                           MsgOpUpdate *pUpdateMsg,
                           netMultiRouteAgent *pRouteAgent,
                           pmdEDUCB *cb,
                           CoordGroupList &sendGroupLst,
                           INT64 *updateNum = NULL );
      INT32 buildOpMsg( const CoordCataInfoPtr &cataInfo,
                              const CoordSubCLlist &subCLList,
                              const CHAR *pSrcMsg, CHAR *&pDstMsg,
                              INT32 &bufferSize );
      INT32 kickShardingKey( const CoordCataInfoPtr &cataInfo,
                             const bson::BSONObj &boUpdator,
                             bson::BSONObj &boNewUpdator,
                             BOOLEAN &hasShardingKey );
      INT32 kickShardingKeyForSubCL( const CoordSubCLlist &subCLList,
                             const bson::BSONObj &boUpdator,
                             bson::BSONObj &boNewUpdator,
                             BOOLEAN &hasShardingKey,
                             pmdEDUCB *cb );
   };
}

#endif


