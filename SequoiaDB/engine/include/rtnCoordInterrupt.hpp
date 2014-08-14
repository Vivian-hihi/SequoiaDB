#ifndef RTNCOORDINTERRUPT_HPP__
#define RTNCOORDINTERRUPT_HPP__

#include "rtnCoordOperator.hpp"

namespace engine
{
   class rtnCoordInterrupt : public rtnCoordOperator
   {
   public:
      virtual INT32 execute( CHAR * pReceiveBuffer, SINT32 packSize,
                        CHAR * * ppResultBuffer, pmdEDUCB * cb,
                        MsgOpReply & replyHeader,
                        BSONObj **ppErrorObj );
   private:
      void SendInterrupt( pmdEDUCB *cb, ROUTE_SET &routeMap );
   };
}

#endif // RTNCOORDINTERRUPT_HPP__
