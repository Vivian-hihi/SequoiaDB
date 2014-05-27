#ifndef RTNCOORDQUERY_HPP__
#define RTNCOORDQUERY_HPP__

#include "rtnCoordOperator.hpp"
#include "rtnContext.hpp"
#include "../bson/bson.h"

namespace engine
{
   class rtnCoordQuery : virtual public rtnCoordOperator
   {
   public:
      virtual INT32 execute( CHAR *pReceiveBuffer, SINT32 packSize,
                           CHAR **ppResultBuffer, pmdEDUCB *cb,
                           MsgOpReply &replyHeader,
                           BSONObj** ppErrorObj );
      INT32 queryToDataNodeGroup( CHAR *pBuffer,
                                 CoordGroupList &groupLst,
                                 CoordGroupList &sendGroupLst,
                                 netMultiRouteAgent *pRouteAgent,
                                 pmdEDUCB *cb,
                                 rtnContextCoord *pContext,
                                 BOOLEAN sendToPrimary = FALSE,
                                 std::set<INT32> *ignoreRCList = NULL );
      virtual INT32 executeQuery( CHAR *pSrc ,
                                 bson::BSONObj &boQuery ,
                                 bson::BSONObj &boOrderBy ,
                                 const CHAR * pCollectionName ,
                                 netMultiRouteAgent *pRouteAgent ,
                                 pmdEDUCB *cb ,
                                 rtnContextCoord *&pContext );
   private:
      INT32 getNodeGroups( const CoordCataInfoPtr &cataInfo,
                           bson::BSONObj &queryObj,
                           const CoordGroupList &sendGroupLst,
                           CoordGroupList &groupLst );
      INT32 queryOnMainCL( CoordGroupSubCLMap &groupSubCLMap,
                           MsgOpQuery *pSrc,
                           pmdEDUCB *cb,
                           netMultiRouteAgent *pRouteAgent,
                           CoordGroupList &sendGroupList,
                           rtnContextCoord *pContext );
   };

}

#endif

