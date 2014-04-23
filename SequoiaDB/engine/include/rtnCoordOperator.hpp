#ifndef RTNCOORDOPERATOR_HPP__
#define RTNCOORDOPERATOR_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "pmdEDU.hpp"
#include "msg.hpp"
#include "coordCB.hpp"
#include "rtnCoordCommon.hpp"
#include "msgCatalog.hpp"
#include "../bson/bson.h"

namespace engine
{
   class rtnCoordOperator : public SDBObject
   {
   public:
      virtual ~rtnCoordOperator(){}

      virtual INT32 execute( CHAR *pReceiveBuffer, SINT32 packSize,
                             CHAR **ppResultBuffer, pmdEDUCB *cb,
                             MsgOpReply &replyHeader,
                             BSONObj **ppErrorObj ) = 0;

   };

   class rtnCoordOperatorDefault : public rtnCoordOperator
   {
   public:
      virtual INT32 execute( CHAR *pReceiveBuffer, SINT32 packSize,
                           CHAR **ppResultBuffer, pmdEDUCB *cb,
                           MsgOpReply &replyHeader,
                           BSONObj **ppErrorObj );
   };

   class rtnCoordTransOperator : public rtnCoordOperator
   {
   protected:
      virtual INT32 buildTransSession( CoordGroupList &groupLst,
                                       netMultiRouteAgent *pRouteAgent,
                                       pmdEDUCB *cb );
      virtual INT32 releaseTransSession( CoordGroupList &groupLst,
                                       netMultiRouteAgent *pRouteAgent,
                                       pmdEDUCB *cb );
      virtual void adjustTransSession( CoordGroupList &transGroupLst,
                                       netMultiRouteAgent *pRouteAgent,
                                       pmdEDUCB *cb );
      virtual INT32 modifyOpOnMainCL( CoordCataInfoPtr &cataInfo,
                                    const CoordSubCLlist &subCLList,
                                    MsgHeader *pSrcMsg,
                                    netMultiRouteAgent *pRouteAgent,
                                    pmdEDUCB *cb,
                                    BOOLEAN isNeedRefresh,
                                    std::set<INT32> &ignoreRCList,
                                    CoordGroupList &sendGroupLst,
                                    INT64 *modifyNum = NULL );
      virtual INT32 buildOpMsg( const CoordCataInfoPtr &cataInfo,
                              const CoordSubCLlist &subCLList,
                              CHAR *pSrcMsg, CHAR *&pDstMsg,
                              INT32 &bufferSize ){ return SDB_SYS ; }
   };

   class rtnCoordKillContext : public rtnCoordOperator
   {
   public :
      virtual INT32 execute ( CHAR *pReceiveBuffer, SINT32 packSize,
                              CHAR **ppResultBuffer, pmdEDUCB *cb,
                              MsgOpReply &replHeader,
                              BSONObj **ppErrorObj ) ;
   } ;

   class rtnCoordMsg : public rtnCoordOperator
   {
   public:
      virtual INT32 execute ( CHAR *pReceiveBuffer, SINT32 packSize,
                              CHAR **ppResultBuffer, pmdEDUCB *cb,
                              MsgOpReply &replyHeader,
                              BSONObj **ppErrorObj ) ;
   } ;

}

#endif //RTNCOORDOPERATOR_HPP__

