#ifndef RTNCOORDTRANSACTION_HPP__
#define RTNCOORDTRANSACTION_HPP__

#include "rtnCoordOperator.hpp"

namespace engine
{
   class rtnCoordTransBegin : public rtnCoordOperator
   {
   public:
      virtual INT32 execute( CHAR * pReceiveBuffer, SINT32 packSize,
                           CHAR * * ppResultBuffer, pmdEDUCB * cb,
                           MsgOpReply & replyHeader,
                           BSONObj **ppErrorObj );
   };

   class rtnCoord2PhaseCommit : public rtnCoordOperator
   {
   public:
      virtual INT32 execute( CHAR * pReceiveBuffer, SINT32 packSize,
                           CHAR * * ppResultBuffer, pmdEDUCB * cb,
                           MsgOpReply & replyHeader,
                           BSONObj **ppErrorObj );

   private:
      virtual INT32 doPhase1( CHAR * pReceiveBuffer, SINT32 packSize,
                           CHAR * * ppResultBuffer, pmdEDUCB * cb,
                           MsgOpReply & replyHeader );

      virtual INT32 doPhase2( CHAR * pReceiveBuffer, SINT32 packSize,
                           CHAR * * ppResultBuffer, pmdEDUCB * cb,
                           MsgOpReply & replyHeader );

      virtual INT32 cancelOp( CHAR * pReceiveBuffer, SINT32 packSize,
                           CHAR * * ppResultBuffer, pmdEDUCB * cb,
                           MsgOpReply & replyHeader );

      virtual INT32 buildPhase1Msg( CHAR * pReceiveBuffer, CHAR **pMsg ) = 0;

      virtual INT32 buildPhase2Msg( CHAR * pReceiveBuffer, CHAR **pMsg ) = 0;

      virtual INT32 executeOnDataGroup ( CHAR *pMsg,
                                         netMultiRouteAgent *pRouteAgent,
                                         pmdEDUCB *cb ) = 0;
   };

   class rtnCoordTransCommit : public rtnCoord2PhaseCommit
   {
   public:
      virtual INT32 execute( CHAR * pReceiveBuffer, SINT32 packSize,
                           CHAR * * ppResultBuffer, pmdEDUCB * cb,
                           MsgOpReply & replyHeader,
                           BSONObj **ppErrorObj );

   private:
      virtual INT32 buildPhase1Msg( CHAR * pReceiveBuffer, CHAR **pMsg );

      virtual INT32 buildPhase2Msg( CHAR * pReceiveBuffer, CHAR **pMsg );

      virtual INT32 executeOnDataGroup ( CHAR *pMsg,
                                         netMultiRouteAgent *pRouteAgent,
                                         pmdEDUCB *cb );
   };

   class rtnCoordTransRollback : public rtnCoordOperator
   {
   public:
      virtual INT32 execute( CHAR * pReceiveBuffer, SINT32 packSize,
                           CHAR * * ppResultBuffer, pmdEDUCB * cb,
                           MsgOpReply & replyHeader,
                           BSONObj **ppErrorObj );

   private:
      virtual INT32 executeOnDataGroup ( CHAR *pMsg,
                                         netMultiRouteAgent *pRouteAgent,
                                         pmdEDUCB *cb );
   };
}

#endif
