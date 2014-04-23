#ifndef RTNCOORDAGGREGATE_HPP__
#define RTNCOORDAGGREGATE_HPP__

#include "rtnCoordOperator.hpp"

namespace engine
{
   class rtnCoordAggregate : virtual public rtnCoordOperator
   {
   public:
      virtual INT32 execute( CHAR *pReceiveBuffer, SINT32 packSize,
                           CHAR **ppResultBuffer, pmdEDUCB *cb,
                           MsgOpReply &replyHeader,
                           BSONObj **ppErrorObj );
   };
}

#endif
