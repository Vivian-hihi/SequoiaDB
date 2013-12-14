/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = msgAuth.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef MSGAUTH_HPP_
#define MSGAUTH_HPP_
#include "core.hpp"
#include "msg.h"
#include "authDef.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   struct _MsgAuthReply
   {
      MsgInternalReplyHeader header ;
      _MsgAuthReply()
      {
         header.res = 0 ;
         header.header.messageLength = sizeof( _MsgAuthReply ) ;
         header.header.opCode = MSG_AUTH_VERIFY_RES ;
         header.header.routeID.value = MSG_INVALID_ROUTEID ;
         header.header.TID = 0 ;
         header.header.requestID = 0 ;
      }
   } ;
   typedef struct _MsgAuthReply MsgAuthReply ;

   struct _MsgAuthCrtReply
   {
      MsgInternalReplyHeader header ;
      _MsgAuthCrtReply()
      {
         header.res = 0 ;
         header.header.messageLength = sizeof(_MsgAuthCrtReply ) ;
         header.header.opCode = MSG_AUTH_CRTUSR_RES ;
         header.header.routeID.value = MSG_INVALID_ROUTEID ;
         header.header.TID = 0 ;
         header.header.requestID = 0 ;
      }
   } ;
   typedef struct _MsgAuthCrtReply MsgAuthCrtReply ;

   struct _MsgAuthDelReply
   {
      MsgInternalReplyHeader header ;
      _MsgAuthDelReply()
      {
         header.res = 0 ;
         header.header.messageLength = sizeof(_MsgAuthCrtReply ) ;
         header.header.opCode = MSG_AUTH_DELUSR_RES ;
         header.header.routeID.value = MSG_INVALID_ROUTEID ;
         header.header.TID = 0 ;
         header.header.requestID = 0 ;
      }
   } ;
   typedef struct _MsgAuthDelReply MsgAuthDelReply ;

   INT32 extractAuthMsg( MsgHeader *header, BSONObj &obj ) ;

}

#endif

