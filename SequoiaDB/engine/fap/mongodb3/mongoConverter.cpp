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
   baseCommand *cmd = NULL ;
   commandMgr *cmdMgr = commandMgr::instance() ;

   if ( NULL == cmdMgr )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   _parser.extractMsg( _msgdata, _msglen ) ;

   if ( dbQuery == _parser.dataPacket().opCode )
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

   try
   {
      baseCommand* newCmd = NULL ;

      rc = cmd->convert( _parser, &newCmd ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to convert command[%s], rc: %d",
                   cmd->name(), rc ) ;
      if ( newCmd )
      {
         cmd = newCmd ;
      }

      rc = cmd->buildMsg( _parser, out ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to build message for command[%s], rc: %d",
                   cmd->name(), rc ) ;

      _parser.setCommand( cmd ) ;
   }
   catch ( std::exception &e )
   {
      PD_RC_CHECK( SDB_SYS, PDERROR,
                   "Failed to process command: %s, exception occurred: %s",
                   cmd->name(), e.what() ) ;
   }

done:
   return rc ;
error:
   goto done ;
}

INT32 mongoConverter::convertReply( MsgOpReply &replyHeader,
                                    engine::rtnContextBuf &replyBuf )
{
   INT32 rc = SDB_OK ;
   baseCommand *cmd = _parser.command() ;

   try
   {
      rc = cmd->handleReply( _parser, replyHeader, replyBuf ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to build response for command[%s], rc: %d",
                   cmd->name(), rc ) ;
   }
   catch ( std::exception &e )
   {
      PD_RC_CHECK( SDB_SYS, PDERROR,
                   "Failed to process command: %s, exception occurred: %s",
                   cmd->name(), e.what() ) ;
   }

done:
   return rc ;
error:
   goto done ;
}

