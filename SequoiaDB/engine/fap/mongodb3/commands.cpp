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

   Source File Name = commands.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/01/2015  LZ  Initial Draft

   Last Changed =

*******************************************************************************/
#include "commands.hpp"
#include "msgBuffer.hpp"
#include "parser.hpp"
#include "msg.hpp"
#include "msgDef.h"

using namespace bson ;

static void convertProjection( BSONObj &proj )
{
   BSONObjBuilder newBuilder ;
   BOOLEAN hasId = FALSE ;
   BOOLEAN addInclude = FALSE ;
   BOOLEAN addExclude = FALSE ;
   BOOLEAN addIdExclude = FALSE ;
   BOOLEAN hasOperator = FALSE ;
   BSONObjIterator i( proj ) ;

   if ( proj.isEmpty() )
   {
      goto done ;
   }

   while ( i.more() )
   {
      BSONElement e = i.next() ;
      if ( 0 == ossStrcmp( e.fieldName(), "_id" ) )
      {
         hasId = TRUE ;
      }
      if ( Object == e.type() && '$' == e.Obj().firstElementFieldName()[0] )
      {
         hasOperator = TRUE ;
         newBuilder.append( e ) ;
      }
      else
      {
         // { b: 1 } => { b: { $include: 1 } }
         newBuilder.append( e.fieldName(),
                            BSON( "$include" << ( e.trueValue() ? 1 : 0 ) ) ) ;
         if ( e.trueValue() )
         {
            addInclude = TRUE ;
         }
         else
         {
            addExclude = TRUE ;
            if ( 0 == ossStrcmp( e.fieldName(), "_id" ) )
            {
               addIdExclude = TRUE ;
            }
         }
      }
   }

   if ( !hasOperator && !hasId && !addExclude )
   {
      newBuilder.append( "_id", BSON( "$include" << 1 ) ) ;
   }

   proj = newBuilder.obj() ;

   if ( addIdExclude && addInclude )
   {
      proj = proj.filterFieldsUndotted( BSON( "_id" << 1 ), false ) ;
   }

done:
   return ;
}

static void escapeDot( string& collectionFullName )
{
   BOOLEAN firstLoop = TRUE ;
   string::size_type pos = 0 ;
   while( TRUE )
   {
      pos = collectionFullName.find( '.', pos ) ;
      if ( string::npos == pos )
      {
         break ;
      }
      else if ( firstLoop )
      {
         pos++ ;
         firstLoop = FALSE ;
         continue ;
      }
      else
      {
         collectionFullName.replace( pos, 1, "%2E" ) ;
         pos += 3 ;
      }
   }
}

static void unescapeDot( string& collectionName )
{
   string::size_type pos = 0 ;
   while( TRUE )
   {
      pos = collectionName.find( "%2E", pos ) ;
      if ( string::npos == pos )
      {
         break ;
      }
      else
      {
         collectionName.replace( pos, 3, "." ) ;
         pos++ ;
      }
   }
}

static void convertIndexObj( BSONObj& indexObj, string clFullName )
{
   BSONObjBuilder builder ;
   BSONObj sdbIdxDef = indexObj.getObjectField( "IndexDef" ) ;

   // listIndexes command may send getMore message, we should convert fullname:
   //     foo.$cmd.listIndexes.bar => foo.bar
   string::size_type pos = clFullName.find( "$cmd.listIndexes" ) ;
   if ( string::npos != pos )
   {
      clFullName.erase( pos, sizeof( "$cmd.listIndexes" ) ) ;
   }
   unescapeDot( clFullName ) ;

   // build
   builder.append( "v", sdbIdxDef.getIntField( "v" ) ) ;
   if ( sdbIdxDef.getBoolField( "unique" ) &&
        sdbIdxDef.getBoolField( "enforced" ) )
   {
      builder.append( "unique", true ) ;
   }
   builder.append( "key", sdbIdxDef.getObjectField( "key" ) ) ;
   builder.append( "name", sdbIdxDef.getStringField( "name" ) ) ;
   builder.append( "ns", clFullName.c_str() ) ;

   indexObj = builder.obj() ;
}

static void convertCollectionObj( BSONObj& collectionObj )
{
   // { Name: "foo.bar" } => { name: "bar" }
   const CHAR* clFullName = collectionObj.getStringField( "Name" ) ;
   const CHAR* dotPos = ossStrstr( clFullName, "." ) + 1 ;
   string clShortName = dotPos ;

   unescapeDot( clShortName ) ;

   collectionObj = BSON( "name" << clShortName.c_str() ) ;
}

static void buildFirstBatch( msgParser &parser,
                             INT32 errCode,
                             mongoMsgReply &replyHeader,
                             engine::rtnContextBuf &replyBuf )
{
   // {xxx}, {xxx}... =>
   // { cursor: { firstBatch: [ {xxx}, {xxx}... ], id: 0, ns: "foo.bar" },
   //   ok: 1 }
   mongoDataPacket packet = parser.dataPacket() ;
   BSONObjBuilder resultBuilder ;
   BSONObjBuilder cursorBuilder ;

   BSONArrayBuilder arr( cursorBuilder.subarrayStart( "firstBatch" ) ) ;
   if ( SDB_DMS_EOC == errCode )
   {
      // do nothing
   }
   else
   {
      INT32 offset = 0 ;
      while ( offset < replyBuf.size() )
      {
         BSONObj obj( replyBuf.data() + offset ) ;
         offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;

         if ( OP_CMD_GET_INDEX == parser.currentOperation() )
         {
            convertIndexObj( obj, packet.fullName ) ;
         }
         else if ( OP_CMD_GET_CLS == parser.currentOperation() )
         {
            convertCollectionObj( obj ) ;
         }
         arr.append( obj ) ;
      }
   }
   arr.done() ;

   if ( OP_CMD_GET_INDEX == parser.currentOperation() )
   {
      // listIndexes reply:   { ... ns: "foo.$cmd.listIndexes.bar" ... }
      std::string ns = packet.csName ;
      ns += ".$cmd.listIndexes." ;
      ns += packet.all.firstElement().valuestrsafe() ;
      cursorBuilder.append( "ns", ns.c_str() ) ;
   }
   else if ( OP_CMD_GET_CLS == parser.currentOperation() )
   {
      // listCL reply:   { ... ns: "foo.$cmd.listCollections" ... }
      string ns = packet.csName ;
      ns += ".$cmd.listCollections" ;
      cursorBuilder.append( "ns", ns.c_str() ) ;
   }
   else
   {
      // real query reply:   { ... ns: "foo.bar" ... }
      cursorBuilder.append( "ns", packet.fullName.c_str() ) ;
   }

   cursorBuilder.append( "id", (INT64)replyHeader.cursorId ) ;
   resultBuilder.append( "cursor", cursorBuilder.obj() ) ;
   resultBuilder.append( "ok", 1 ) ;
   replyBuf = engine::rtnContextBuf( resultBuilder.obj() ) ;

   replyHeader.cursorId = MONGO_INVALID_CURSORID ;
   replyHeader.nReturned = 1 ;
}

static void buildNextBatch( msgParser &parser,
                            INT32 errCode,
                            mongoMsgReply &replyHeader,
                            engine::rtnContextBuf &replyBuf )
{
   // {xxx}, {xxx}... =>
   // { cursor: { nextBatch: [{xxx}, {xxx}...], id: 0, ns: "foo.bar" },
   //   ok: 1 }
   mongoDataPacket packet = parser.dataPacket() ;
   bson::BSONObjBuilder resultBuilder ;
   bson::BSONObjBuilder cursorBuilder ;

   bson::BSONArrayBuilder arr( cursorBuilder.subarrayStart( "nextBatch" ) ) ;
   INT32 offset = 0 ;
   if ( SDB_DMS_EOC == errCode )
   {
      // do nothing
   }
   else
   {
      while ( offset < replyBuf.size() )
      {
         bson::BSONObj obj( replyBuf.data() + offset ) ;
         offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
         arr.append( obj ) ;
      }
   }
   arr.done() ;

   /* getMore message may come from three command:
    * 1. listIndexes
    *           request: { getMore: <>, collection: "$cmd.listIndexes.bar" }
    *           reply:   { ... ns: "foo.$cmd.listIndexes.bar" ... }
    * 2. listCL
    *           request: { getMore: <>, collection: "$cmd.listCollections" }
    *           reply:   { ... ns: "foo.$cmd.listCollections" ... }
    * 3. real query
    *           request: { getMore: <>, collection: bar" }
    *           reply:   { ... ns: "foo.bar" ... }
    */
   packet.fullName = packet.csName ;
   packet.fullName += "." ;
   packet.fullName += packet.all.getStringField( "collection" ) ;
   cursorBuilder.append( "ns", packet.fullName.c_str() ) ;

   cursorBuilder.append( "id", (INT64)replyHeader.cursorId ) ;
   resultBuilder.append( "cursor", cursorBuilder.obj() ) ;
   resultBuilder.append( "ok", 1 ) ;
   replyBuf = engine::rtnContextBuf( resultBuilder.obj() ) ;

   replyHeader.cursorId = MONGO_INVALID_CURSORID ;
   replyHeader.nReturned = 1 ;
}

static void convertAggrSumIfExist( const BSONElement& ele,
                                   BSONObjBuilder& builder )
{
   if ( ele.isABSONObj() && ele.Obj().getIntField( "$sum" ) == 1 )
   {
      // { $group: { _id: ..., total: { $sum: 1 } } } =>
      // { $group: { _id: ..., total: { $count: "$_id" } } }
      builder.append( ele.fieldName(), BSON( "$count" << "$_id" ) ) ;
   }
   else
   {
      builder.append( ele ) ;
   }
}

static void convertAggrGroup( const BSONObj& groupObj,
                              std::vector<BSONObj>& newStageList )
{
   /* cl.aggregate( { $group: { _id: "$a" ) )
    * MongoDB return { _id: <value of a> }, but SequoiaDB return all fields.
    * So we should convert:
    * { $group: { _id: "$a", total: { $sum: "$b" } } } ==>
    * { $group: { _id: "$a", total: { $sum: "$b" }, tmp_id_field: "$a" } },
    * { $project: { _id: "$tmp_id_field", total: 1 } }
    */
   if ( 0 != ossStrcmp( groupObj.firstElementFieldName(), "$group" ) )
   {
      return ;
   }

   BSONObj groupValue = groupObj.getObjectField( "$group" ) ;
   const CHAR* idValue = groupValue.getStringField( "_id" ) ;

   if ( '$' == idValue[0] )
   {
      BSONObjBuilder bobGroup, bobProj ;
      BSONObjIterator itr( groupValue ) ;
      while ( itr.more() )
      {
         BSONElement e = itr.next() ;
         convertAggrSumIfExist( e, bobGroup ) ;
         if ( 0 == ossStrcmp( e.fieldName(), "_id" ) )
         {
            bobProj.append( "_id", "$tmp_id_field" ) ;
         }
         else
         {
            bobProj.append( e.fieldName(), 1 ) ;
         }
      }
      bobGroup.append( "tmp_id_field", idValue ) ;

      newStageList.push_back( BSON( "$group" << bobGroup.obj() ) ) ;
      newStageList.push_back( BSON( "$project" << bobProj.obj() ) ) ;
   }
   else
   {
      BSONObjBuilder bobGroup ;
      BSONObjIterator itr( groupValue ) ;
      while ( itr.more() )
      {
         BSONElement e = itr.next() ;
         convertAggrSumIfExist( e, bobGroup ) ;
      }
      newStageList.push_back( BSON( "$group" << bobGroup.obj() ) ) ;
   }
}

static void convertAggrProject( BSONObj& projectObj )
{
   /* MongoDB return _id field by default, but SequoiaDB doesn't return _id
    * field by default. So we should convert:
    * { $project: { a: 1 } }  ==> { $project: { id: 1, a: 1 } }
    */
   if ( 0 != ossStrcmp( projectObj.firstElementFieldName(), "$project" ) )
   {
      return ;
   }

   BSONObj projValue = projectObj.getObjectField( "$project" ) ;
   BOOLEAN hasId = projValue.hasField( "_id" ) ;
   if ( !hasId )
   {
      BSONObjBuilder builder ;
      builder.append( "_id", 1 ) ;
      builder.appendElements( projValue ) ;
      projectObj = BSON( "$project" << builder.obj() ) ;
   }
}

DECLARE_COMMAND_VAR( insert )
DECLARE_COMMAND_VAR( delete )
DECLARE_COMMAND_VAR( update )
DECLARE_COMMAND_VAR( query )
DECLARE_COMMAND_VAR( getMore )
DECLARE_COMMAND_VAR( killCursors )
DECLARE_COMMAND_VAR( distinct )
DECLARE_COMMAND_VAR( createUser )
DECLARE_COMMAND_VAR( dropUser )
DECLARE_COMMAND_VAR( create )
DECLARE_COMMAND_VAR( listDatabases )
DECLARE_COMMAND_VAR( listCollections )
DECLARE_COMMAND_VAR( drop )
DECLARE_COMMAND_VAR( count )
DECLARE_COMMAND_VAR( aggregate )
DECLARE_COMMAND_VAR( dropDatabase )
DECLARE_COMMAND_VAR( createIndexes )
DECLARE_COMMAND_VAR( deleteIndexes )
DECLARE_COMMAND_VAR( listIndexes )
DECLARE_COMMAND_VAR( getlasterror )
DECLARE_COMMAND_VAR( ismaster )
DECLARE_COMMAND_VAR( whatsmyuri )
DECLARE_COMMAND_VAR( buildinfo )
DECLARE_COMMAND_VAR( getLog )
DECLARE_COMMAND_VAR( ping )
DECLARE_COMMAND_VAR( logout )

/// implement of commands

BOOLEAN insertCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 insertCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 insertCommand::buildMsg( msgParser& parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpInsert *insert     = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;

   parser.setCurrentOp( OP_INSERT ) ;

   sdbMsg.reserve( sizeof( MsgOpInsert ) ) ;
   sdbMsg.advance( sizeof( MsgOpInsert ) - 4 ) ;

   insert = ( MsgOpInsert *)sdbMsg.data() ;
   insert->header.opCode = MSG_BS_INSERT_REQ ;
   insert->header.TID = 0 ;
   insert->header.routeID.value = 0 ;
   insert->header.requestID = packet.requestId ;
   insert->version = 0 ;
   insert->w = 0 ;
   insert->padding = 0 ;
   insert->flags = FLG_INSERT_RETURNNUM ;

   if ( ( packet.reservedInt & INSERT_CONTINUE_ON_ERROR ) ||
        !packet.all.getBoolField( "ordered" ) )
   {
      insert->flags |= FLG_INSERT_CONTONDUP ;
   }

   insert->nameLength = packet.fullName.length() ;
   sdbMsg.write( packet.fullName.c_str(), insert->nameLength + 1, TRUE ) ;

   // mongo message:
   // { insert: "bar", documents: [ {xxx}, {xxx}, ... ] }
   BSONElement e = packet.all.getField( "documents" ) ;
   if ( bson::Array != e.type() )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   else
   {
      BSONObjIterator it( e.Obj() ) ;
      while( it.more() )
      {
         BSONElement be = it.next() ;
         sdbMsg.write( be.Obj(), TRUE ) ;
      }
   }

   sdbMsg.doneLen() ;

done:
   return rc ;
error:
   goto done ;
}

INT32 insertCommand::handleReply( msgParser &parser,
                                  INT32 errCode,
                                  mongoMsgReply &replyHeader,
                                  engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      // reply: { n: 1, ok: 1 }
      BSONObj resObj( replyBuf.data() ) ;
      BSONObjBuilder bob ;

      bob.append( "ok", 1 ) ;
      if ( resObj.hasField( "InsertedNum" ) )
      {
         bob.append( "n", resObj.getIntField( "InsertedNum" ) ) ;
      }
      replyBuf = engine::rtnContextBuf( bob.obj() ) ;
   }

   return SDB_OK ;
}

BOOLEAN deleteCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 deleteCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 deleteCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpDelete *del        = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   std::vector<BSONElement> objList ;

   parser.setCurrentOp( OP_REMOVE );

   sdbMsg.reserve( sizeof( MsgOpDelete ) ) ;
   sdbMsg.advance( sizeof( MsgOpDelete ) - 4 ) ;

   del = ( MsgOpDelete *)sdbMsg.data() ;
   del->header.opCode = MSG_BS_DELETE_REQ ;
   del->header.TID = 0 ;
   del->header.routeID.value = 0 ;
   del->header.requestID = packet.requestId ;
   del->version = 0 ;
   del->w = 0 ;
   del->padding = 0 ;
   del->flags |= FLG_DELETE_RETURNNUM ;

   if ( packet.reservedInt & REMOVE_JUSTONE )
   {
      del->flags |= FLG_DELETE_SINGLEREMOVE ;
   }

   del->nameLength = packet.fullName.length() ;
   sdbMsg.write( packet.fullName.c_str(), del->nameLength + 1, TRUE ) ;

   // mongo message:
   // { delete: "bar", deletes: [ { q: {xxx}, limit: 0 } ] }
   BSONElement e = packet.all.getField( "deletes" ) ;
   if ( bson::Array != e.type() )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   objList = e.Array() ;
   for ( std::vector<BSONElement>::const_iterator cit = objList.begin() ;
         cit != objList.end() ; cit++ )
   {
      BSONObj subObj = (*cit).Obj() ;
      BSONObj obj = subObj.getObjectField( "q" ) ;
      BSONObj cond = getQueryObj( obj ) ;
      BSONObj hint = getHintObj( obj ) ;
      sdbMsg.write( cond, TRUE ) ;
      sdbMsg.write( hint, TRUE ) ;
   }

   sdbMsg.doneLen() ;

done:
   return rc ;
error:
   goto done ;
}

INT32 deleteCommand::handleReply( msgParser &parser,
                                  INT32 errCode,
                                  mongoMsgReply &replyHeader,
                                  engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      // reply: { n: 1, ok: 1 }
      BSONObj resObj( replyBuf.data() ) ;
      BSONObjBuilder bob ;

      bob.append( "ok", 1 ) ;
      if ( resObj.hasField( "DeletedNum" ) )
      {
         bob.append( "n", resObj.getIntField( "DeletedNum" ) ) ;
      }
      replyBuf = engine::rtnContextBuf( bob.obj() ) ;
   }

   return SDB_OK ;
}

BOOLEAN updateCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 updateCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 updateCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpUpdate *update     = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   std::vector<BSONElement> objList ;

   parser.setCurrentOp( OP_UPDATE ) ;

   sdbMsg.reserve( sizeof( MsgOpUpdate ) ) ;
   sdbMsg.advance( sizeof( MsgOpUpdate ) - 4 ) ;

   update = ( MsgOpUpdate *)sdbMsg.data() ;
   update->header.opCode = MSG_BS_UPDATE_REQ ;
   update->header.TID = 0 ;
   update->header.routeID.value = 0 ;
   update->header.requestID = packet.requestId ;
   update->version = 0 ;
   update->w = 0 ;
   update->padding = 0 ;
   update->flags = FLG_UPDATE_RETURNNUM ;

   update->nameLength = packet.fullName.length() ;
   sdbMsg.write( packet.fullName.c_str(), update->nameLength + 1, TRUE ) ;

   // mongo message:
   // { update: "bar",
   //   updates: [ { q: {xxx}, u: {xxx}, upsert: false, multi: true } ] }
   BSONElement e = packet.all.getField( "updates" ) ;
   if ( bson::Array != e.type() )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   objList = e.Array() ;
   for ( std::vector<BSONElement>::const_iterator cit = objList.begin() ;
         cit != objList.end() ; cit++ )
   {
      BSONObj obj = (*cit).Obj() ;
      BSONObj query = obj.getObjectField( "q" ) ;
      BSONObj updator = obj.getObjectField( "u" ) ;
      BSONObj hint = getHintObj( query ) ;
      BSONObj setOnObj ;

      // set flag
      if ( obj.getBoolField( "multi" ) )
      {
         update->flags |= FLG_UPDATE_MULTIUPDATE ;
      }
      if ( obj.getBoolField( "upsert" ) )
      {
         update->flags |= FLG_UPDATE_UPSERT ;
      }

      // if updator without operator, convert to $replace
      if( 0 == updator.nFields() ||
          updator.firstElementFieldName()[0] != '$' )
      {
         updator = BSON( "$replace" << updator ) ;
      }

      // upsert operation requires _id to return
      if ( update->flags & FLG_UPDATE_UPSERT )
      {
         BOOLEAN hasId = FALSE ;

         // has _id or not
         BSONObjIterator i( updator ) ;
         while ( i.more() )
         {
            BSONElement ele = i.next() ;
            if ( ele.isABSONObj() && ele.Obj().hasField( "_id" ) )
            {
               packet.dataInfo = BSON( "_id" << ele.Obj().getField( "_id" ) ) ;
               hasId = TRUE ;
               break ;
            }
         }

         // filter $setOnInsert
         if ( updator.hasField( "$setOnInsert" ) )
         {
            setOnObj = updator.getObjectField( "$setOnInsert" ) ;
            updator = updator.filterFieldsUndotted( BSON( "$setOnInsert" << 1 ),
                                                    false ) ;
            if( 0 == updator.nFields() )
            {
               updator = BSON( "$set" << BSONObj() ) ;
            }
         }

         // add _id to $SetOnInsert if _id doesn't exist
         if ( !hasId )
         {
            OID oid = OID::gen() ;
            packet.dataInfo = BSON( "_id" << oid ) ;

            BSONObjBuilder bob ;
            bob.append( "_id", oid ) ;
            bob.appendElements( setOnObj ) ;
            hint = BSON( FIELD_NAME_SET_ON_INSERT << bob.obj() ) ;
         }
         else
         {
            hint = BSON( FIELD_NAME_SET_ON_INSERT << setOnObj ) ;
         }
      }

      sdbMsg.write( query, TRUE ) ;
      sdbMsg.write( updator, TRUE ) ;
      sdbMsg.write( hint, TRUE ) ;
   }

   sdbMsg.doneLen() ;

done:
   return rc ;
error:
   goto done ;
}

INT32 updateCommand::handleReply( msgParser &parser,
                                  INT32 errCode,
                                  mongoMsgReply &replyHeader,
                                  engine::rtnContextBuf &replyBuf )
{
   mongoDataPacket& packet = parser.dataPacket() ;

   if ( SDB_OK == errCode )
   {
      // update reply: { ok: 1, n: 1, nModified: 1 }
      // upsert reply: { ok: 1, n: 1, nModified: 0,
      //                 upserted: [ { index: 0, _id: xxx } ] }
      BSONObj resObj( replyBuf.data() ) ;
      BSONObjBuilder bob ;

      bob.append( "ok", 1 ) ;
      //n
      if ( resObj.hasField( "InsertedNum" ) &&
           resObj.getIntField( "InsertedNum" ) > 0 )
      {
         bob.append( "n", resObj.getIntField( "InsertedNum" ) ) ;
      }
      else if ( resObj.hasField( "UpdatedNum" ) )
      {
         bob.append( "n", resObj.getIntField( "UpdatedNum" ) ) ;
      }
      //nModified
      if ( resObj.hasField( "ModifiedNum" ) )
      {
         bob.append( "nModified", resObj.getIntField( "ModifiedNum" ) ) ;
      }
      //upserted
      if ( resObj.hasField( "InsertedNum" ) &&
           resObj.getIntField( "InsertedNum" ) > 0 )
      {
         BSONArrayBuilder sub( bob.subarrayStart( "upserted" ) ) ;
         sub.append( BSON( "index" << 0 <<
                           "_id" << packet.dataInfo.getField( "_id" ) ) ) ;
         sub.done() ;
      }
      replyBuf = engine::rtnContextBuf( bob.obj() ) ;
   }

   return SDB_OK ;
}

BOOLEAN queryCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 queryCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   INT32 rc                = SDB_OK ;
   mongoDataPacket &packet = parser.dataPacket() ;

   parser.readInt( sizeof( INT32 ), ( CHAR* )&packet.nToSkip ) ;
   parser.readInt( sizeof( INT32 ), ( CHAR* )&packet.nToReturn ) ;
   if ( 0 == packet.nToReturn )
   {
      packet.nToReturn = -1 ;
   }

   if ( !parser.more() )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   parser.readNextObj( packet.all ) ;

   if ( packet.with( OPTION_CMD ) )
   {
      // all operation messages are dbQuery + cs.$cmd, except query, getMore
      // and killCursors
      const CHAR *cmdName = packet.all.firstElementFieldName() ;
      baseCommand *pCmd = commandMgr::instance()->findCommand( cmdName ) ;

      if ( pCmd )
      {
         *ppNewCmd = pCmd ;
      }
      else
      {
         rc = SDB_OPTION_NOT_SUPPORT ;
         parser.setCurrentOp( OP_CMD_NOT_SUPPORTED ) ;
         goto error ;
      }

      packet.fullName = packet.csName ;
      packet.fullName += "." ;
      packet.fullName += packet.all.getStringField( cmdName ) ;
      escapeDot( packet.fullName ) ;
   }
   else
   {
      // it is real query
      escapeDot( packet.fullName ) ;
   }

done:
   return rc ;
error:
   goto done ;
}

INT32 queryCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;

   parser.setCurrentOp( OP_QUERY ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;
   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;

   setQueryFlags( packet.reservedInt, query->flags ) ;
   if ( packet.all.getBoolField( "$explain" ) )
   {
      query->flags |= FLG_QUERY_EXPLAIN ;
   }

   query->nameLength = packet.fullName.length() ;
   query->numToSkip = packet.nToSkip ;
   if ( packet.nToReturn < 1000 )
   {
      query->numToReturn = packet.nToReturn ;
   }
   else
   {
      query->numToReturn = -1 ;
   }
   sdbMsg.write( packet.fullName.c_str(), query->nameLength + 1, TRUE ) ;

   BSONObj cond, selector, orderby, hint ;
   if ( parser.more() )
   {
      parser.readNextObj( selector ) ;
   }
   convertProjection( selector ) ;
   cond = getQueryObj( packet.all ) ;
   orderby = getSortObj( packet.all ) ;
   hint = getHintObj( packet.all ) ;

   sdbMsg.write( cond, TRUE ) ;
   sdbMsg.write( selector, TRUE ) ;
   sdbMsg.write( orderby, TRUE ) ;
   sdbMsg.write( hint, TRUE ) ;

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 queryCommand::handleReply( msgParser &parser,
                                 INT32 errCode,
                                 mongoMsgReply &replyHeader,
                                 engine::rtnContextBuf &replyBuf )
{
   if ( errCode != SDB_OK && errCode != SDB_DMS_EOC )
   {
      // reply: { $err: "xxx", code: 1234 }
      BSONObj resObj( replyBuf.data() ) ;
      BSONObj newObj = BSON( "$err" << resObj.getStringField( "errmsg" )<<
                             "code" << resObj.getIntField( "code" ) ) ;
      replyBuf = engine::rtnContextBuf( newObj ) ;
      replyHeader.header.reservedFlags |= MONGO_REPLY_FLAG_QUERY_FAILURE ;
   }
   return SDB_OK ;
}

BOOLEAN getMoreCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 getMoreCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 getMoreCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpGetMore *more      = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   INT64 cursorId          = MONGO_INVALID_CURSORID ;

   parser.setCurrentOp( OP_GETMORE ) ;

   sdbMsg.reserve( sizeof( MsgOpGetMore ) ) ;
   sdbMsg.advance( sizeof( MsgOpGetMore ) ) ;

   more = ( MsgOpGetMore * )sdbMsg.data() ;
   more->header.opCode = MSG_BS_GETMORE_REQ ;
   more->header.TID = 0 ;
   more->header.routeID.value = 0 ;
   more->header.requestID = packet.requestId ;

   if ( dbQuery == packet.opCode )
   {
      // get more for aggregate at spring data java driver 3.2+
      // mongo message:
      // { getMore: <cursorID>, collection: "bar" }
      cursorId = packet.all.getField( "getMore" ).numberLong() ;
      more->contextID = MGCURSOID_TO_SDBCTXID( cursorId ) ;
      more->numToReturn = -1 ;
   }
   else if ( dbGetMore == packet.opCode )
   {
      parser.readInt( sizeof( INT32 ), ( CHAR* )&packet.nToReturn ) ;
      if ( 0 == packet.nToReturn )
      {
         packet.nToReturn = -1 ;
      }
      more->numToReturn = packet.nToReturn ;

      parser.readInt( sizeof( INT32 ), ( CHAR* )&cursorId  ) ;
      more->contextID = MGCURSOID_TO_SDBCTXID( cursorId ) ;
   }

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 getMoreCommand::handleReply( msgParser &parser,
                                   INT32 errCode,
                                   mongoMsgReply &replyHeader,
                                   engine::rtnContextBuf &replyBuf )
{
   mongoDataPacket& packet = parser.dataPacket() ;

   if ( SDB_RTN_CONTEXT_NOTEXIST == errCode )
   {
      replyHeader.header.reservedFlags |= MONGO_REPLY_FLAG_CURSOR_NOT_FOUND ;
      goto done ;
   }

   if ( dbQuery == packet.opCode )
   {
      if ( SDB_OK == errCode || SDB_DMS_EOC == errCode )
      {
         buildNextBatch( parser, errCode, replyHeader, replyBuf ) ;
      }
   }
   else if ( dbGetMore == packet.opCode )
   {
      if ( SDB_OK == errCode )
      {
         msgBuffer tmpBuffer ;
         INT32 offset = 0 ;
         tmpBuffer.zero() ;
         if ( string::npos != packet.fullName.find( "$cmd.listCollections" ) )
         {
            while ( offset < replyBuf.size() )
            {
               BSONObj obj( replyBuf.data() + offset ) ;
               offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
               convertIndexObj( obj, packet.fullName ) ;
               tmpBuffer.write( obj.objdata(), obj.objsize(), TRUE ) ;
            }
            replyBuf = engine::rtnContextBuf( tmpBuffer.data(),
                                              tmpBuffer.size(),
                                              replyBuf.recordNum() ) ;
         }
         else if ( string::npos != packet.fullName.find( "$cmd.listIndexes" ) )
         {
            while ( offset < replyBuf.size() )
            {
               BSONObj obj( replyBuf.data() + offset ) ;
               offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
               convertCollectionObj( obj ) ;
               tmpBuffer.write( obj.objdata(), obj.objsize(), TRUE ) ;
            }
            replyBuf = engine::rtnContextBuf( tmpBuffer.data(),
                                              tmpBuffer.size(),
                                              replyBuf.recordNum() ) ;
         }
      }
      else if ( SDB_DMS_EOC == errCode )
      {
         replyBuf = engine::rtnContextBuf() ;
         replyHeader.nReturned = 0 ;
      }
   }

done:
   return SDB_OK ;
}

BOOLEAN killCursorsCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 killCursorsCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 killCursorsCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   INT32 nContext          = 0 ;
   MsgOpKillContexts *kill = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;

   parser.setCurrentOp( OP_KILLCURSORS ) ;

   sdbMsg.reserve( sizeof( MsgOpKillContexts ) ) ;
   sdbMsg.advance( sizeof( MsgOpKillContexts ) - sizeof( SINT64 ) ) ;

   kill = ( MsgOpKillContexts * )sdbMsg.data() ;
   kill->header.opCode = MSG_BS_KILL_CONTEXT_REQ ;
   kill->header.TID = 0 ;
   kill->header.routeID.value = 0 ;
   kill->header.requestID = packet.requestId ;
   kill->ZERO = 0 ;

   parser.readInt( sizeof( INT32 ), ( CHAR * )&nContext ) ;
   kill->numContexts = nContext ;

   while ( 0 < nContext )
   {
      INT64 cursorId = 0 ;
      parser.readInt( sizeof( SINT64 ), ( CHAR *)&cursorId ) ;
      if ( 0 != cursorId )
      {
         cursorId -= 1 ;
         sdbMsg.write( ( CHAR * )&cursorId, sizeof( SINT64 ) ) ;
      }
      --nContext ;
   }

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 killCursorsCommand::handleReply( msgParser &parser,
                                       INT32 errCode,
                                       mongoMsgReply &replyHeader,
                                       engine::rtnContextBuf &replyBuf )
{
   return SDB_OK ;
}

BOOLEAN createUserCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 createUserCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 createUserCommand::buildMsg( msgParser& parser, msgBuffer &sdbMsg )
{
   // TODO
   return SDB_OK ;
}

INT32 createUserCommand::handleReply( msgParser &parser,
                                      INT32 errCode,
                                      mongoMsgReply &replyHeader,
                                      engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      replyBuf = engine::rtnContextBuf( BSON( "ok" << 1 ) ) ;
      replyHeader.nReturned = 1 ;
   }
   return SDB_OK ;
}

BOOLEAN dropUserCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 dropUserCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 dropUserCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   // TODO
   return SDB_OK ;
}

INT32 dropUserCommand::handleReply( msgParser &parser,
                                    INT32 errCode,
                                    mongoMsgReply &replyHeader,
                                    engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      replyBuf = engine::rtnContextBuf( BSON( "ok" << 1 ) ) ;
      replyHeader.nReturned = 1 ;
   }
   return SDB_OK ;
}


BOOLEAN createCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 createCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 createCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_CREATE_COLLECTION ;

   parser.setCurrentOp( OP_CMD_CREATE ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;
   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   setQueryFlags( packet.reservedInt, query->flags ) ;
   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = 0 ;
   query->numToReturn = -1 ;

   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   BSONObj cond, empty ;
   cond = BSON( FIELD_NAME_NAME << packet.fullName.c_str() ) ;

   sdbMsg.write( cond, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 createCommand::handleReply( msgParser &parser,
                                  INT32 errCode,
                                  mongoMsgReply &replyHeader,
                                  engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      replyBuf = engine::rtnContextBuf( BSON( "ok" << 1 ) ) ;
      replyHeader.nReturned = 1 ;
   }
   return SDB_OK ;
}

BOOLEAN listDatabasesCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 listDatabasesCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 listDatabasesCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_LIST_COLLECTIONSPACES ;

   parser.setCurrentOp( OP_CMD_GET_DBS ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;

   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   setQueryFlags( packet.reservedInt, query->flags ) ;

   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = 0 ;
   query->numToReturn = -1 ;

   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   BSONObj empty ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 listDatabasesCommand::handleReply( msgParser &parser,
                                         INT32 errCode,
                                         mongoMsgReply &replyHeader,
                                         engine::rtnContextBuf &replyBuf )
{
   BSONObjBuilder bob ;

   if ( SDB_OK == errCode )
   {
      BSONArrayBuilder arr( bob.subarrayStart( "databases" ) ) ;
      INT32 offset = 0 ;
      while ( offset < replyBuf.size() )
      {
         BSONObj obj( replyBuf.data() + offset ) ;
         // { Name: "cs" } => { name: "cs" }
         arr.append( BSON( "name" << obj.getStringField( "Name" ) ) ) ;
         offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
      }
      arr.done() ;
      bob.append( "ok", 1 ) ;

      replyBuf = engine::rtnContextBuf( bob.obj() ) ;
      replyHeader.cursorId = MONGO_INVALID_CURSORID ;
      replyHeader.nReturned = 1 ;
   }
   else if ( SDB_DMS_EOC == errCode )
   {
      bob.append( "databases", BSONArray() ) ;
      bob.append( "ok", 1 ) ;
      replyBuf = engine::rtnContextBuf( bob.obj() ) ;
      replyHeader.cursorId = MONGO_INVALID_CURSORID ;
      replyHeader.nReturned = 1 ;
   }

   return SDB_OK ;
}

BOOLEAN listCollectionsCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 listCollectionsCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 listCollectionsCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_LIST_COLLECTIONS ;

   parser.setCurrentOp( OP_CMD_GET_CLS ) ;
   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;

   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   setQueryFlags( packet.reservedInt, query->flags ) ;

   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = 0 ;
   query->numToReturn = -1 ;

   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   // filter cs[foo]'s collections
   // condition: { Name: { $gt: "foo.", $lt: "foo/" }, ... }
   BSONObj empty ;
   BSONObjBuilder builder ;

   string lowBound = packet.csName ;
   lowBound += "." ;
   string upBound = packet.csName ;
   upBound += "/" ;
   builder.appendElements( packet.all.getObjectField( "filter" ) ) ;
   builder.append( "Name", BSON( "$gt" << lowBound << "$lt" << upBound ) ) ;

   sdbMsg.write( builder.obj(), TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 listCollectionsCommand::handleReply( msgParser &parser,
                                           INT32 errCode,
                                           mongoMsgReply &replyHeader,
                                           engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK      == errCode ||
        SDB_DMS_EOC == errCode )
   {
      buildFirstBatch( parser, errCode, replyHeader, replyBuf ) ;
   }

   return SDB_OK ;
}

BOOLEAN dropCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 dropCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 dropCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc = SDB_OK ;
   MsgOpQuery *query = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName = CMD_ADMIN_PREFIX CMD_NAME_DROP_COLLECTION ;

   parser.setCurrentOp( OP_CMD_DROP ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;

   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   setQueryFlags( packet.reservedInt, query->flags ) ;

   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = 0 ;
   query->numToReturn = -1 ;

   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   BSONObj cond, empty ;
   cond = BSON( FIELD_NAME_NAME << packet.fullName.c_str() ) ;
   sdbMsg.write( cond, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 dropCommand::handleReply( msgParser &parser,
                                INT32 errCode,
                                mongoMsgReply &replyHeader,
                                engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      const CHAR* clName = parser.dataPacket().fullName.c_str() ;
      replyBuf = engine::rtnContextBuf( BSON( "ns" << clName <<
                                              "ok" << 1 ) ) ;
      replyHeader.nReturned = 1 ;
   }
   else if ( SDB_DMS_NOTEXIST == errCode )
   {
      replyBuf = engine::rtnContextBuf( BSON( "ok" << 0 <<
                                              "errmsg" << "ns not found" ) ) ;
      replyHeader.nReturned = 1 ;
   }
   return SDB_OK ;
}

BOOLEAN countCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 countCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 countCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc = SDB_OK ;
   MsgOpQuery *query = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName = CMD_ADMIN_PREFIX CMD_NAME_GET_COUNT ;

   parser.setCurrentOp( OP_CMD_COUNT ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;

   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   setQueryFlags( packet.reservedInt, query->flags ) ;

   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = 0 ;
   query->numToReturn = -1 ;

   if ( packet.all.hasField( "skip" ) )
   {
      query->numToSkip = packet.all.getIntField( "skip" ) ;
   }
   if ( packet.all.hasField( "limit" ) )
   {
      query->numToReturn = packet.all.getIntField( "limit" ) ;
   }

   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   // mongo command: db.bar.count( { a: 1 }, { hint: "aIdx" } )
   // sdb msg: matcher: { a: 1 }
   //          hint:    { Collection: "bar", Hint: { "": "aIdx" } }
   BSONObj empty, hint ;
   BSONObj matcher = packet.all.getObjectField( "query" ) ;

   if ( packet.all.hasField( "hint" ) )
   {
      hint = BSON( FIELD_NAME_COLLECTION <<
                   packet.fullName.c_str() <<
                   FIELD_NAME_HINT <<
                   BSON( "" << packet.all.getStringField( "hint" ) ) ) ;
   }
   else
   {
      hint = BSON( FIELD_NAME_COLLECTION << packet.fullName.c_str() ) ;
   }

   sdbMsg.write( matcher, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( hint, TRUE ) ;

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 countCommand::handleReply( msgParser &parser,
                                 INT32 errCode,
                                 mongoMsgReply &replyHeader,
                                 engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      // reply: { n: 1, ok: 1 }
      BSONObj resObj( replyBuf.data() ) ;
      BSONObjBuilder bob ;

      bob.append( "n", resObj.getIntField( "Total" ) ) ;
      bob.append( "ok", 1 ) ;
      replyBuf = engine::rtnContextBuf( bob.obj() ) ;
      replyHeader.cursorId = MONGO_INVALID_CURSORID ;
      replyHeader.startingFrom = 0 ;
   }

   return SDB_OK ;
}

BOOLEAN aggregateCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 aggregateCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 aggregateCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpAggregate *aggre   = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;

   parser.setCurrentOp( OP_CMD_AGGREGATE ) ;

   sdbMsg.reserve( sizeof( MsgOpAggregate ) ) ;
   sdbMsg.advance( sizeof( MsgOpAggregate ) - 4 ) ;

   aggre = ( MsgOpAggregate * )sdbMsg.data() ;
   aggre->header.opCode = MSG_BS_AGGREGATE_REQ ;
   aggre->header.TID = 0 ;
   aggre->header.routeID.value = 0 ;
   aggre->header.requestID = packet.requestId ;

   aggre->version = 0 ;
   aggre->w = 0 ;
   aggre->padding = 0 ;
   aggre->flags = 0 ;

   aggre->nameLength = packet.fullName.length() ;
   sdbMsg.write( packet.fullName.c_str(), aggre->nameLength + 1, TRUE ) ;

   /* eg: { pipeline: [ { $match: { b: 1 } },
                        { $group: { _id: "$a", b: { $sum: "$b" } } }
                      ] } */
   BSONObjIterator it( packet.all.getObjectField( "pipeline" ) ) ;
   while ( it.more() )
   {
      BSONElement ele = it.next() ;
      if ( ele.type() != Object )
      {
         sdbMsg.write( ele.rawdata(), ele.size(), TRUE ) ;
         continue ;
      }

      BSONObj oneStage = ele.Obj() ;
      if ( 0 == ossStrcmp( oneStage.firstElementFieldName(), "$project" ) )
      {
         convertAggrProject( oneStage ) ;
         sdbMsg.write( oneStage, TRUE ) ;
      }
      else if ( 0 == ossStrcmp( oneStage.firstElementFieldName(), "$group" ) )
      {
         std::vector<BSONObj> newStageList ;
         convertAggrGroup( oneStage, newStageList ) ;
         for( std::vector<BSONObj>::iterator it = newStageList.begin() ;
              it != newStageList.end() ; it++ )
         {
            sdbMsg.write( *it, TRUE ) ;
         }
      }
      else
      {
         sdbMsg.write( oneStage, TRUE ) ;
      }
   }
   sdbMsg.doneLen() ;

   return rc ;
}

INT32 aggregateCommand::handleReply( msgParser &parser,
                                     INT32 errCode,
                                     mongoMsgReply &replyHeader,
                                     engine::rtnContextBuf &replyBuf )
{
   mongoDataPacket& packet = parser.dataPacket() ;

   if ( packet.all.hasField( "cursor" ) )
   {
      // aggregate at spring data java driver 3.2+
      if ( SDB_OK      == errCode ||
           SDB_DMS_EOC == errCode )
      {
         buildFirstBatch( parser, errCode, replyHeader, replyBuf ) ;
      }
   }
   else
   {
      // reply: { result: [ {xxx}, {xxx}, ... ], ok: 1 }
      if ( SDB_OK == errCode )
      {
         BSONObjBuilder bob ;
         BSONArrayBuilder arr( bob.subarrayStart( "result" ) ) ;
         INT32 offset = 0 ;
         while ( offset < replyBuf.size() )
         {
            BSONObj obj( replyBuf.data() + offset ) ;
            arr.append( obj ) ;
            offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
         }
         arr.done() ;
         bob.append( "ok", 1 ) ;

         replyBuf = engine::rtnContextBuf( bob.obj() ) ;
         replyHeader.nReturned = 1 ;
      }
      else if ( SDB_DMS_EOC == errCode )
      {
         BSONObjBuilder bob ;
         bob.append( "result", BSONArray() ) ;
         bob.append( "ok", 1 ) ;
         replyBuf = engine::rtnContextBuf( bob.obj() ) ;
         replyHeader.cursorId = MONGO_INVALID_CURSORID ;
         replyHeader.nReturned = 1 ;
      }
   }

   return SDB_OK ;
}

BOOLEAN distinctCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 distinctCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 distinctCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpAggregate *aggre   = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;

   parser.setCurrentOp( OP_CMD_DISTINCT ) ;

   sdbMsg.reserve( sizeof( MsgOpAggregate ) ) ;
   sdbMsg.advance( sizeof( MsgOpAggregate ) - 4 ) ;

   aggre = ( MsgOpAggregate * )sdbMsg.data() ;
   aggre->header.opCode = MSG_BS_AGGREGATE_REQ ;
   aggre->header.TID = 0 ;
   aggre->header.routeID.value = 0 ;
   aggre->header.requestID = packet.requestId ;

   aggre->version = 0 ;
   aggre->w = 0 ;
   aggre->padding = 0 ;
   aggre->flags = 0 ;

   aggre->nameLength = packet.fullName.length() ;
   sdbMsg.write( packet.fullName.c_str(), aggre->nameLength + 1, TRUE ) ;

   std::string distinctfield = "$" ;
   distinctfield += packet.all.getStringField( "key" ) ;

   // distinct( "a", { b: 1 } ) =>
   // { $match: { b: 1 } },
   // { $group: { _id: "$a" } },
   // { $group: { _id: null, values: { $addtoset: "$a" } } }
   BSONObj match, group1, group2 ;
   BSONObjBuilder builder ;

   if ( packet.all.hasField( "query" ) )
   {
      match = BSON( "$match" << packet.all.getField( "query" ) ) ;
   }

   group1 = BSON( "$group" << BSON( "_id" << distinctfield ) ) ;

   builder.appendNull( "_id" ) ;
   builder.append( "values", BSON( "$addtoset" << distinctfield ) ) ;
   group2 = BSON( "$group" << builder.done() ) ;

   if ( !match.isEmpty() )
   {
      sdbMsg.write( match, TRUE ) ;
   }
   sdbMsg.write( group1, TRUE ) ;
   sdbMsg.write( group2, TRUE ) ;
   sdbMsg.doneLen() ;

   return rc ;
}

INT32 distinctCommand::handleReply( msgParser &parser,
                                    INT32 errCode,
                                    mongoMsgReply &replyHeader,
                                    engine::rtnContextBuf &replyBuf )
{
   BSONObjBuilder bob ;

   // reply: { values: [ 1, 3, 4 ], ok: 1 }
   if ( SDB_OK == errCode )
   {
      bob.appendElements( BSONObj( replyBuf.data() ) ) ;
      bob.append( "ok", 1 ) ;
      replyBuf = engine::rtnContextBuf( bob.obj() ) ;
      replyHeader.cursorId = MONGO_INVALID_CURSORID ;
   }
   else if ( SDB_DMS_EOC == errCode )
   {
      bob.append( "values", BSONArray() ) ;
      bob.append( "ok", 1 ) ;
      replyBuf = engine::rtnContextBuf( bob.obj() ) ;
      replyHeader.cursorId = MONGO_INVALID_CURSORID ;
   }

   return SDB_OK ;
}

BOOLEAN dropDatabaseCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 dropDatabaseCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 dropDatabaseCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_DROP_COLLECTIONSPACE ;

   parser.setCurrentOp( OP_CMD_DROP_DATABASE ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;

   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   setQueryFlags( packet.reservedInt, query->flags ) ;

   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = packet.nToSkip ;
   query->numToReturn = packet.nToReturn ;

   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   BSONObj obj, empty ;
   obj = BSON( FIELD_NAME_NAME << packet.csName ) ;

   sdbMsg.write( obj, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 dropDatabaseCommand::handleReply( msgParser &parser,
                                        INT32 errCode,
                                        mongoMsgReply &replyHeader,
                                        engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      const CHAR* csName = parser.dataPacket().csName.c_str() ;
      replyBuf = engine::rtnContextBuf( BSON( "dropped" << csName <<
                                              "ok" << 1 ) ) ;
      replyHeader.nReturned = 1 ;
   }
   else if ( SDB_DMS_CS_NOTEXIST == errCode )
   {
      replyBuf = engine::rtnContextBuf( BSON( "ok" << 1 ) ) ;
      replyHeader.nReturned = 1 ;
   }

   return SDB_OK ;
}

BOOLEAN createIndexesCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 createIndexesCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 createIndexesCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_CREATE_INDEX ;

   parser.setCurrentOp( OP_ENSURE_INDEX ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;

   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = packet.nToSkip ;
   query->numToReturn = packet.nToReturn ;

   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   BSONObj empty ;
   BSONObjBuilder bob ;
   std::vector< BSONElement > objList ;

   // mongo message:
   // { createIndex: "bar", indexs: [ { key: { a: 1 }, name: "aIdx" } ] }
   BSONElement e = packet.all.getField( "indexes" ) ;
   if( bson::Array != e.type() )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   objList = e.Array() ;
   for ( std::vector<BSONElement>::const_iterator cit = objList.begin() ;
         cit != objList.end() ; cit++ )
   {
      BSONObj obj = (*cit).Obj() ;

      // mongodb unique index => sequiadb unique enforced index
      BSONObj indexObj = BSON( IXM_FIELD_NAME_KEY <<
                               obj.getObjectField( "key" ) <<
                               IXM_FIELD_NAME_NAME <<
                               obj.getStringField( "name") <<
                               IXM_FIELD_NAME_UNIQUE <<
                               obj.getBoolField( "unique" ) <<
                               IXM_FIELD_NAME_ENFORCED <<
                               obj.getBoolField( "unique" ) );
      bob.append( FIELD_NAME_INDEX, indexObj ) ;
      bob.append( FIELD_NAME_COLLECTION, packet.fullName.c_str() ) ;
      sdbMsg.write( bob.obj(), TRUE ) ;
   }

   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;

   sdbMsg.doneLen() ;

done:
   return rc ;
error:
   goto done ;
}

INT32 createIndexesCommand::handleReply( msgParser &parser,
                                         INT32 errCode,
                                         mongoMsgReply &replyHeader,
                                         engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      replyBuf = engine::rtnContextBuf( BSON( "ok" << 1 ) ) ;
      replyHeader.nReturned = 1 ;
   }
   return SDB_OK ;
}

BOOLEAN deleteIndexesCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 deleteIndexesCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 deleteIndexesCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc = SDB_OK ;
   MsgOpQuery *query = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName = CMD_ADMIN_PREFIX CMD_NAME_DROP_INDEX ;

   parser.setCurrentOp( OP_CMD_DROP_INDEX ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;

   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   setQueryFlags( packet.reservedInt, query->flags ) ;

   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = packet.nToSkip ;
   query->numToReturn = packet.nToReturn ;

   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   // mongo message:
   // { deleteIndexes: "bar", index: "aIdx" } or
   // { dropIndexes: "bar", index: "aIdx" }
   BSONObj empty ;
   BSONObj cond = BSON( FIELD_NAME_COLLECTION <<
                        packet.fullName.c_str() <<
                        FIELD_NAME_INDEX <<
                        BSON( "" << packet.all.getStringField( "index" ) ) ) ;

   sdbMsg.write( cond, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;

   sdbMsg.doneLen() ;

   return rc ;
}

INT32 deleteIndexesCommand::handleReply( msgParser &parser,
                                         INT32 errCode,
                                         mongoMsgReply &replyHeader,
                                         engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK == errCode )
   {
      replyBuf = engine::rtnContextBuf( BSON( "ok" << 1 ) ) ;
      replyHeader.nReturned = 1 ;
   }
   return SDB_OK ;
}

BOOLEAN listIndexesCommand::needProcessByEngine()
{
   return TRUE ;
}

INT32 listIndexesCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 listIndexesCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   INT32 rc = SDB_OK ;
   MsgOpQuery *query = NULL ;
   mongoDataPacket &packet = parser.dataPacket() ;
   const CHAR *cmdName = CMD_ADMIN_PREFIX CMD_NAME_GET_INDEXES ;
   BSONObj hint, empty ;

   if ( !packet.with( OPTION_CMD ) )
   {
      rc = SDB_OPTION_NOT_SUPPORT ;
      parser.setCurrentOp( OP_CMD_NOT_SUPPORTED ) ;
      goto error ;
   }

   parser.setCurrentOp( OP_CMD_GET_INDEX ) ;

   sdbMsg.reserve( sizeof( MsgOpQuery ) ) ;
   sdbMsg.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )sdbMsg.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = packet.requestId ;

   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   setQueryFlags( packet.reservedInt, query->flags ) ;

   query->nameLength = ossStrlen( cmdName ) ;
   query->numToSkip = 0 ;
   query->numToReturn = -1 ;
   sdbMsg.write( cmdName, query->nameLength + 1, TRUE ) ;

   hint = BSON( FIELD_NAME_COLLECTION << packet.fullName.c_str() ) ;

   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( empty, TRUE ) ;
   sdbMsg.write( hint, TRUE ) ;

   sdbMsg.doneLen() ;

done:
   return rc ;
error:
   goto done ;
}

INT32 listIndexesCommand::handleReply( msgParser &parser,
                                       INT32 errCode,
                                       mongoMsgReply &replyHeader,
                                       engine::rtnContextBuf &replyBuf )
{
   if ( SDB_OK      == errCode ||
        SDB_DMS_EOC == errCode )
   {
      buildFirstBatch( parser, errCode, replyHeader, replyBuf ) ;
   }

   return SDB_OK ;
}

BOOLEAN getlasterrorCommand::needProcessByEngine()
{
   return FALSE ;
}

INT32 getlasterrorCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 getlasterrorCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   parser.setCurrentOp( OP_CMD_GETLASTERROR ) ;
   return SDB_OK ;
}

INT32 getlasterrorCommand::handleReply( msgParser &parser,
                                        INT32 errCode,
                                        mongoMsgReply &replyHeader,
                                        engine::rtnContextBuf &replyBuf )
{
   // handle in _mongoSession::_preProcessMsg()
   return SDB_OK ;
}

BOOLEAN pingCommand::needProcessByEngine()
{
   return FALSE ;
}

INT32 pingCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 pingCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   parser.setCurrentOp( OP_CMD_PING ) ;
   return SDB_OK ;
}

INT32 pingCommand::handleReply( msgParser &parser,
                                INT32 errCode,
                                mongoMsgReply &replyHeader,
                                engine::rtnContextBuf &replyBuf )
{
   replyBuf = engine::rtnContextBuf( BSON( "ok" << 1 ) ) ;
   replyHeader.nReturned = 1 ;
   return SDB_OK ;
}

BOOLEAN ismasterCommand::needProcessByEngine()
{
   return FALSE ;
}

INT32 ismasterCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 ismasterCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   parser.setCurrentOp( OP_CMD_ISMASTER ) ;
   return SDB_OK ;
}

INT32 ismasterCommand::handleReply( msgParser &parser,
                                    INT32 errCode,
                                    mongoMsgReply &replyHeader,
                                    engine::rtnContextBuf &replyBuf )
{
   BSONObjBuilder bob ;
   bob.append( "ismaster", TRUE ) ;
   bob.append( "maxBsonObjectSize", 16*1024*1024 ) ;
   bob.append( "maxMessageSizeBytes", SDB_MAX_MSG_LENGTH ) ;
   bob.append( "maxWriteBatchSize", 1000 ) ;
   bob.append( "maxWireVersion", 3 ) ; // correspions to mongodb3.0
   bob.append( "minWireVersion", 0 ) ;
   bob.append( "ok", 1 ) ;
   replyBuf = engine::rtnContextBuf( bob.obj() ) ;
   replyHeader.nReturned = 1 ;

   return SDB_OK ;
}

BOOLEAN whatsmyuriCommand::needProcessByEngine()
{
   return FALSE ;
}

INT32 whatsmyuriCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 whatsmyuriCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   parser.setCurrentOp( OP_CMD_WHATSMYURI ) ;
   return SDB_OK ;
}

INT32 whatsmyuriCommand::handleReply( msgParser &parser,
                                      INT32 errCode,
                                      mongoMsgReply &replyHeader,
                                      engine::rtnContextBuf &replyBuf )
{
   bson::BSONObjBuilder bob ;
   bob.append( "ok", 1 ) ;
   bob.append( "you", "0.0.0.0:00000" ) ;
   replyBuf = engine::rtnContextBuf( bob.obj() ) ;
   replyHeader.nReturned = 1 ;

   return SDB_OK ;
}

BOOLEAN buildinfoCommand::needProcessByEngine()
{
   return FALSE ;
}

INT32 buildinfoCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 buildinfoCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   parser.setCurrentOp( OP_CMD_BUILDINFO ) ;
   return SDB_OK ;
}

INT32 buildinfoCommand::handleReply( msgParser &parser,
                                     INT32 errCode,
                                     mongoMsgReply &replyHeader,
                                     engine::rtnContextBuf &replyBuf )
{
   bson::BSONObjBuilder bob ;
   bob.append( "version", "3.0.15" ) ;
   // versionArray is important, it affects the protocol of messages
   // sent by mongo client
   bson::BSONArrayBuilder sub( bob.subarrayStart( "versionArray" ) ) ;
   sub.append( 3 ) ;
   sub.append( 0 ) ;
   sub.append( 15 ) ;
   sub.append( 0 ) ;
   sub.done() ;
   bob.append( "maxBsonObjectSize", 16*1024*1024 ) ;
   bob.append( "ok", 1 ) ;
   replyBuf = engine::rtnContextBuf( bob.obj() ) ;
   replyHeader.nReturned = 1 ;

   return SDB_OK ;
}

BOOLEAN getLogCommand::needProcessByEngine()
{
   return FALSE ;
}

INT32 getLogCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 getLogCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   parser.setCurrentOp( OP_CMD_GETLOG ) ;
   return SDB_OK ;
}

INT32 getLogCommand::handleReply( msgParser &parser,
                                  INT32 errCode,
                                  mongoMsgReply &replyHeader,
                                  engine::rtnContextBuf &replyBuf )
{
   bson::BSONObjBuilder bob ;
   bob.append( "totalLinesWritten", 0 ) ;
   bson::BSONArrayBuilder sub( bob.subarrayStart( "log" ) ) ;
   sub.done() ;
   bob.append( "ok", 1 ) ;
   replyBuf = engine::rtnContextBuf( bob.obj() ) ;
   replyHeader.nReturned = 1 ;

   return SDB_OK ;
}

BOOLEAN logoutCommand::needProcessByEngine()
{
   return FALSE ;
}

INT32 logoutCommand::convert( msgParser &parser, baseCommand** ppNewCmd )
{
   return SDB_OK ;
}

INT32 logoutCommand::buildMsg( msgParser &parser, msgBuffer &sdbMsg )
{
   parser.setCurrentOp( OP_CMD_PING ) ;
   return SDB_OK ;
}

INT32 logoutCommand::handleReply( msgParser &parser,
                                  INT32 errCode,
                                  mongoMsgReply &replyHeader,
                                  engine::rtnContextBuf &replyBuf )
{
   replyBuf = engine::rtnContextBuf( BSON( "ok" << 1 ) ) ;
   replyHeader.nReturned = 1 ;
   return SDB_OK ;
}
