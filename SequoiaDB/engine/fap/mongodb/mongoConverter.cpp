/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

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
   _parser.init( _msgdata, _msglen ) ;

   // convert mongodb msg to sequoiadb msg
   // for all kinds of requests available
   // 
   if ( dbInsert == _parser.opCode )
   {
      _cmd = commandMgr::instance()->findCommand( "insert" ) ;
   }   
   else if ( dbDelete == _parser.opCode )
   {
      _cmd = commandMgr::instance()->findCommand( "delete" ) ;
   }
   else if ( dbUpdate == _parser.opCode )
   {
      _cmd = commandMgr::instance()->findCommand( "update" ) ;
   }
   else if ( dbQuery == _parser.opCode )
   {
      _cmd = commandMgr::instance()->findCommand( "query" ) ;
   }
   else if ( dbGetMore == _parser.opCode )
   {
      _cmd = commandMgr::instance()->findCommand( "getMore" ) ;
   }
   else if ( dbKillCursors == _parser.opCode )
   {
      _cmd = commandMgr::instance()->findCommand( "killCursors" ) ;
   }

   if ( NULL == _cmd )
   {
      goto error ;
   }

   rc = _cmd->convertRequest( _parser, out ) ;
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
   INT32 numToReturn = -1 ;

   // create collection failed
   if ( OP_CMD_CREATE == _parser.opType )
   {
      // here mean mongo msg was converted to multi sdb msg
      // like create collection command msg
      // those msg convert to more than one sdb msg
      // that time cs may be not existed, should skip the error
      // and create collection space first
      if ( SDB_OK != reply->flags && SDB_DMS_CS_NOTEXIST == reply->flags )
      {
         _parser.reparse() ;
         _cmd = commandMgr::instance()->findCommand( "createCS" ) ;
         if ( NULL != _cmd )
         {
            out.zero() ;
            rc = _cmd->convertRequest( _parser, out ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
            goto done ;
         }
      }
      else
      {
         rc = reply->flags ;
         goto error ;
      }
   }

   // if is create collection space msg
   if ( OP_CMD_CREATE_CS == _parser.opType )
   {
      if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         goto error ;
      }

      // then, try to create collection again
      _parser.reparse() ;
      _cmd = commandMgr::instance()->findCommand( "create" ) ;
      if ( NULL != _cmd )
      {
         out.zero() ;
         rc = _cmd->convertRequest( _parser, out ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         goto done ;
      }
   }

   if ( OP_CMD_COUNT == _parser.opType || OP_QUERY == _parser.opType )
   {
      if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         goto done ;
      }

      if ( OP_QUERY == _parser.opType )
      {
         MsgOpQuery *query = (MsgOpQuery*)out.data() ;
         numToReturn = query->numToReturn ;
         _parser.opType = OP_GETMORE ;
         if ( numToReturn <= 0 && 0 == reply->numReturned && SDB_OK == reply->flags )
         {
            out.zero() ;
            goto done;
         }
      }
      else if ( OP_CMD_COUNT == _parser.opType )
      {
         numToReturn = 1 ;
         _parser.opType = OP_CMD_COUNT_MORE ;
      }

      out.zero() ;
      fap::mongo::buildGetMoreMsg( out ) ;
      MsgOpReply *msg = ( MsgOpReply *)out.data() ;
      msg->header.requestID = reply->header.requestID ;
      msg->contextID = reply->contextID ;
      msg->numReturned = numToReturn ;
      goto done ;
   }
   // when not handled above, assigned the reply flags to rc for return
   rc = reply->flags ;
   out.zero() ;

done:
   return rc ;
error:
   goto done ;
}
