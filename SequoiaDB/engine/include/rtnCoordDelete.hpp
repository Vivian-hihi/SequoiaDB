#ifndef RTNCOORDDELETE_HPP__
#define RTNCOORDDELETE_HPP__

#include "rtnCoordOperator.hpp"
#include "../bson/bson.h"

namespace engine
{
   class rtnCoordDelete : public rtnCoordTransOperator
   {
   public:
      virtual INT32 execute( CHAR *pReceiveBuffer, SINT32 packSize,
                           CHAR **ppResultBuffer, pmdEDUCB *cb,
                           MsgOpReply &replyHeader, BSONObj **ppErrorObj );

   protected:
      
      INT32 deleteNormalCL( CoordCataInfoPtr cataInfo,
                           bson::BSONObj &boDelete,
                           MsgOpDelete *pDelMsg,
                           netMultiRouteAgent *pRouteAgent,
                           pmdEDUCB *cb,
                           CoordGroupList &sendGroupLst);

   private:
      INT32 getNodeGroups( const CoordCataInfoPtr &cataInfo,
                           bson::BSONObj &deleteObj,
                           CoordGroupList &sendGroupLst,
                           CoordGroupList &groupLst );
      INT32 deleteToDataNodeGroup( CHAR *pBuffer,
                                 CoordGroupList &groupLst,
                                 CoordGroupList &sendGroupLst,
                                 netMultiRouteAgent *pRouteAgent,
                                 pmdEDUCB *cb );
      INT32 buildOpMsg( const CoordCataInfoPtr &cataInfo,
                              const CoordSubCLlist &subCLList,
                              CHAR *pSrcMsg, CHAR *&pDstMsg,
                              INT32 &bufferSize );
   };
}

#endif

