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

   Source File Name = mongodef.hpp

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
#ifndef _SDB_MONGO_DEFINITION_HPP_
#define _SDB_MONGO_DEFINITION_HPP_

#include "util.hpp"
#include "../../bson/bson.hpp"
#include "dms.hpp"

enum mongoOption
{
   dbReply       = 1,
   dbMsg         = 1000,
   dbUpdate      = 2001,
   dbInsert      = 2002,
   dbQuery       = 2004,
   dbGetMore     = 2005,
   dbDelete      = 2006,
   dbKillCursors = 2007,
} ;

enum insertOption
{
   INSERT_CONTINUE_ON_ERROR = 1 << 0,
} ;

enum removeOption
{
   REMOVE_JUSTONE   = 1 << 0,
   REMOVE_BROADCASE = 1 << 1,
} ;

enum updateOption
{
   UPDATE_UPSERT    = 1 << 0,
   UPDATE_MULTI     = 1 << 1,
   UPDATE_BROADCAST = 1 << 2,
} ;

enum queryOption
{
   QUERY_CURSOR_TAILABLE   = 1 << 1,
   QUERY_SLAVE_OK          = 1 << 2,
   QUERY_OPLOG_REPLAY      = 1 << 3,
   QUERY_NO_CURSOR_TIMEOUT = 1 << 4,
   QUERY_AWAIT_DATA        = 1 << 5,
   QUERY_EXHAUST           = 1 << 6,
   QUERY_PARTIAL_RESULTS   = 1 << 7,
   QUERY_ALL_SUPPORTED     = QUERY_CURSOR_TAILABLE | QUERY_SLAVE_OK |
                             QUERY_OPLOG_REPLAY | QUERY_NO_CURSOR_TIMEOUT |
                             QUERY_AWAIT_DATA | QUERY_EXHAUST |
   QUERY_PARTIAL_RESULTS,
} ;

enum ResultFlag
{
   RESULT_CURSOR_NOT_FOUND   = 1,
   RESULT_ERRSET             = 2,
   RESULT_SHARD_CONFIG_STALE = 4,
   RESULT_AWAIT_CAPABLE      = 8,
};

enum authState
{
   AUTH_NONE  = 0,
   AUTH_NONCE = 1,

   AUTH_FINISHED = 1 << 31,
} ;

#define SDB_AUTH_SOURCE_FAP "fap-mongo"

#pragma pack(1)
struct mongoMsgHeader
{
   INT32 msgLen ;
   INT32 requestId ;
   INT32 responseTo ;
   INT32 opCode ;
   INT32 reservedFlags ;
};
#pragma pack()

#pragma pack(1)
struct mongoMsgReply
{
   mongoMsgHeader header ;
   SINT64 cursorId ;
   INT32 startingFrom ;
   INT32 nReturned ;
};
#pragma pack()

#define CS_NAME_SIZE       DMS_COLLECTION_SPACE_NAME_SZ
#define CL_FULL_NAME_SIZE  DMS_COLLECTION_SPACE_NAME_SZ +   \
                           DMS_COLLECTION_NAME_SZ + 1

enum
{
   OP_INVALID           = -1,

   OP_INSERT            = 0,
   OP_REMOVE            = 1,
   OP_UPDATE            = 2,
   OP_QUERY             = 3,
   OP_GETMORE           = 4,
   OP_KILLCURSORS       = 5,
   OP_ENSURE_INDEX      = 6,
   OP_CMD_CREATE        = 7,     // create collection
   OP_CMD_DROP          = 8,    // drop collection
   OP_CMD_DROP_DATABASE = 9,
   OP_CMD_GETLASTERROR  = 10,    // will not process msg
   OP_CMD_DROP_INDEX    = 11,
   OP_CMD_GET_INDEX     = 12,
   OP_CMD_GET_CLS       = 13,    // list collections
   OP_CMD_COUNT         = 14,
   OP_CMD_AGGREGATE     = 15,
   OP_CMD_DISTINCT      = 16,
   OP_CMD_AUTH          = 17,
   OP_CMD_CRTUSER       = 18,
   OP_CMD_DELUSER       = 19,
   OP_CMD_LISTUSER      = 20,
   OP_CMD_ISMASTER      = 21,
   OP_CMD_PING          = 22,
   OP_CMD_NOT_SUPPORTED = 23,
   OP_CMD_WHATSMYURI    = 24,
   OP_CMD_BUILDINFO     = 25,
   OP_CMD_GETLOG        = 26,
   OP_CMD_GET_DBS       = 27,    // list databases
} ;

#endif
