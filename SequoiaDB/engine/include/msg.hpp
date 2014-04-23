#ifndef MSG_HPP__
#define MSG_HPP__
#include "msg.h"

struct _MsgRouteIDComp
{
   BOOLEAN operator()( const _MsgRouteID &id1,
                       const _MsgRouteID &id2 )
   {
      return id1.value < id2.value ;
   }
} ;

typedef _MsgRouteIDComp MsgRouteIDComp ;


#endif
