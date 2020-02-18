/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = mongoConverter.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/27/2015  LZ  Initial Draft

   Last Changed =

*******************************************************************************/
#include "mongoConverter.hpp"
#include "mongodef.hpp"
#include "commands.hpp"
#include "mongoReplyHelper.hpp"

INT32 mongoConverter::convert( msgBuffer &out )
{
   INT32 rc = SDB_OK ;
   baseCommand *&cmd = _parser.command() ;
   _parser.extractMsg( _msgdata, _msglen ) ;

   // convert mongodb msg to sequoiadb msg
   // for all kinds of requests available
   commandMgr *cmdMgr = commandMgr::instance() ;
   if ( NULL == cmdMgr )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   if ( dbInsert == _parser.dataPacket().opCode )
   {
      cmd = cmdMgr->findCommand( "insert" ) ;
   }
   else if ( dbDelete == _parser.dataPacket().opCode )
   {
      cmd = cmdMgr->findCommand( "delete" ) ;
   }
   else if ( dbUpdate == _parser.dataPacket().opCode )
   {
      cmd = cmdMgr->findCommand( "update" ) ;
   }
   else if ( dbQuery == _parser.dataPacket().opCode )
   {
      cmd = cmdMgr->findCommand( "query" ) ;
   }
   else if ( dbGetMore == _parser.dataPacket().opCode )
   {
      cmd = cmdMgr->findCommand( "getMore" ) ;
   }
   else if ( dbKillCursors == _parser.dataPacket().opCode )
   {
      cmd = cmdMgr->findCommand( "killCursors" ) ;
   }

   if ( NULL == cmd )
   {
      rc = SDB_OPTION_NOT_SUPPORT ;
      goto error ;
   }

   rc = cmd->convert( _parser ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

   rc = cmd->buildMsg( _parser, out ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

done:
   return rc ;
error:
   goto done ;
}

INT32 mongoConverter::reConvert( msgBuffer &out, MsgOpReply *reply )
{
   INT32 rc = SDB_OK ;
   UINT32 curOp = _parser.currentOperation() ;

   if ( OP_CMD_COUNT    == curOp || OP_CMD_GET_INDEX == curOp ||
        OP_CMD_GET_CLS  == curOp || OP_CMD_AGGREGATE == curOp ||
        OP_CMD_DISTINCT == curOp )
   {
      if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         goto done ;
      }
      if ( 0 == reply->numReturned && -1 != reply->contextID )
      {
         out.zero() ;
         fap::mongo::buildGetMoreMsg( out ) ;
         MsgOpGetMore *msg = ( MsgOpGetMore *)out.data() ;
         msg->header.requestID = reply->header.requestID ;
         msg->contextID = reply->contextID ;
         msg->numToReturn = -1 ;
         goto done ;
      }
   }

   out.zero() ;

   // when not handled above, assigned the reply flags to rc for return
   rc = reply->flags ;

done:
   return rc ;
}
