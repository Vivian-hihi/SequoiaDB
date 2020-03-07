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

   Source File Name = mongoSession.cpp

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
#include "util.hpp"
#include "mongodef.hpp"
#include "mongoConverter.hpp"
#include "mongoSession.hpp"
#include "pmdEDUMgr.hpp"
#include "pmdEDU.hpp"
#include "pmdEnv.hpp"
#include "monCB.hpp"
#include "msg.hpp"
#include "../../bson/bson.hpp"
#include "rtnCommandDef.hpp"
#include "rtn.hpp"
#include "pmd.hpp"
#include "sdbInterface.hpp"
#include "mongoReplyHelper.hpp"

/////////////////////////////////////////////////////////////////
// implement for mongo processor

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

_mongoSession::_mongoSession( SOCKET fd, engine::IResource *resource )
   : engine::pmdSession( fd ), _masterRead( FALSE ),
     _resource( resource )
{
}

_mongoSession::~_mongoSession()
{
   _resource = NULL ;
}

void _mongoSession::_resetBuffers()
{
   // release buff context
   if ( 0 != _contextBuff.size() )
   {
      _contextBuff.release() ;
   }

   if ( !_inBuffer.empty() )
   {
      _inBuffer.zero() ;
   }

   if ( !_outBuffer.empty() )
   {
      _outBuffer.zero() ;
   }
}

INT32 _mongoSession::getServiceType() const
{
   return CMD_SPACE_SERVICE_LOCAL ;
}

engine::SDB_SESSION_TYPE _mongoSession::sessionType() const
{
   return engine::SDB_SESSION_PROTOCOL ;
}

INT32 _mongoSession::run()
{
   INT32 rc                     = SDB_OK ;
   BOOLEAN bigEndian            = FALSE ;
   UINT32 msgSize               = 0 ;
   // reservedFlag should not included in msg header len
   UINT32  headerLen            = sizeof( mongoMsgHeader ) - sizeof( INT32 ) ;
   INT32 bodyLen                = 0 ;
   engine::pmdEDUMgr *pmdEDUMgr = NULL ;
   CHAR *pBuff                  = NULL ;
   const CHAR *pBody            = NULL ;
   const CHAR *pInMsg           = NULL ;
   INT32 orgOpCode              = 0 ;
   engine::monDBCB *mondbcb     = engine::pmdGetKRCB()->getMonDBCB() ;

   if ( !_pEDUCB )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   pmdEDUMgr = _pEDUCB->getEDUMgr() ;
   bigEndian = checkBigEndian() ;
   while ( !_pEDUCB->isDisconnected() && !_socket.isClosed() )
   {
      // clear interrupt flag
      _pEDUCB->resetInterrupt() ;
      _pEDUCB->resetInfo( engine::EDU_INFO_ERROR ) ;
      _pEDUCB->resetLsn() ;

      // recv msg
      rc = recvData( (CHAR*)&msgSize, sizeof(UINT32) ) ;
      if ( rc )
      {
         if ( SDB_APP_FORCED != rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to recv msg size, "
                    "rc: %d", sessionName(), rc ) ;
         }
         break ;
      }

      // if big endian, need to convert len to little endian
      if ( bigEndian )
      {
         // build an incompatible msg
         // UINT32 tmp = msgSize ;
         // ossEndianConvert4( tmp, msgSize) ;
      }

      if ( msgSize < headerLen || msgSize > SDB_MAX_MSG_LENGTH )
      {
         PD_LOG( PDERROR, "Session[%s] recv msg size[%d] is less than "
                 "mongoMsgHeader size[%d] or more than max msg size[%d]",
                 sessionName(), msgSize, sizeof( mongoMsgHeader ),
                 SDB_MAX_MSG_LENGTH ) ;
         rc = SDB_INVALIDARG ;
         break ;
      }
      // other msg
      else
      {
         pBuff = getBuff( msgSize + 1 ) ;
         if ( !pBuff )
         {
            rc = SDB_OOM ;
            break ;
         }
         *(UINT32*)pBuff = msgSize ;
         // recv the rest msg
         rc = recvData( pBuff + sizeof(UINT32), msgSize - sizeof(UINT32) ) ;
         if ( rc )
         {
            if ( SDB_APP_FORCED != rc )
            {
               PD_LOG( PDERROR, "Session[%s] failed to recv rest msg, rc: %d",
                       sessionName(), rc ) ;
            }
            break ;
         }
         pBuff[ msgSize ] = 0 ;
         {
            // make sure buffers are empty for coming msg
            _resetBuffers() ;
            // convert msg first
            _converter.loadFrom( pBuff, msgSize ) ;
            rc = _converter.convert( _inBuffer ) ;
            if ( SDB_OK != rc && SDB_OPTION_NOT_SUPPORT != rc)
            {
               goto error ;
            }

            _pEDUCB->incEventCount() ;
            mondbcb->addReceiveNum() ;
            // activate edu
            if ( SDB_OK != ( rc = pmdEDUMgr->activateEDU( _pEDUCB ) ) )
            {
               PD_LOG( PDERROR, "Session[%s] activate edu failed, rc: %d",
                       sessionName(), rc ) ;
               goto error ;
            }

            // handle commands before dispatched
            if ( _preProcessMsg( _converter.getParser(),
                                 _resource, _contextBuff ) )
            {
               goto reply ;
            }

            pInMsg = _inBuffer.data() ;
            orgOpCode = ((MsgHeader*)pInMsg)->opCode ;
            while ( NULL != pInMsg )
            {
               // process msg
               rc = _processMsg( pInMsg ) ;

               // auto create cs/cl
               if ( SDB_DMS_CS_NOTEXIST == _replyHeader.flags )
               {
                  if ( SDB_OK == _autoCreateCS() )
                  {
                     ((MsgHeader*)pInMsg)->opCode = orgOpCode ;
                     continue ;
                  }
               }
               else if ( SDB_DMS_NOTEXIST == _replyHeader.flags )
               {
                  if ( OP_INSERT == _converter.getOpType() ||
                       OP_ENSURE_INDEX == _converter.getOpType() ||
                       ( OP_UPDATE == _converter.getOpType() &&
                         ( ((MsgOpUpdate*)_inBuffer.data())->flags & FLG_UPDATE_UPSERT ) ) )
                  {
                     if ( SDB_OK == _autoCreateCL() )
                     {
                        ((MsgHeader*)pInMsg)->opCode = orgOpCode ;
                        continue ;
                     }
                  }
               }

               rc = _converter.reConvert( _inBuffer, &_replyHeader ) ;
               if ( SDB_OK != rc )
               {
                  goto reply ;
               }
               else if ( _inBuffer.empty() )
               {
                  // exit while loop
                  pInMsg = NULL ;
               }
            }

         reply:
            _handleResponse( _converter.getOpType(), _contextBuff ) ;
            pBody = _contextBuff.data() ;
            bodyLen = _contextBuff.size() ;
            // send response
            INT32 rcTmp = _reply( &_replyHeader, pBody, bodyLen ) ;
            if ( rcTmp )
            {
               PD_LOG( PDERROR, "Session[%s] failed to send response,"
                       "rc: %d", sessionName(), rcTmp ) ;
               goto error ;
            }
            pBody = NULL ;
            bodyLen = 0 ;
            _contextBuff.release() ;

            // wait edu
            if ( SDB_OK != ( rc = pmdEDUMgr->waitEDU( _pEDUCB ) ) )
            {
               PD_LOG( PDERROR, "Session[%s] wait edu failed, rc: %d",
                       sessionName(), rc ) ;
               goto error ;
            }
         }
      }
   } // end while
done:
   disconnect() ;
   return rc ;
error:
   goto done ;
}

INT32 _mongoSession::_autoCreateCS()
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   mongoDataPacket &packet = _converter.getParser().dataPacket() ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_CREATE_COLLECTIONSPACE ;
   bson::BSONObj obj       = BSON( FIELD_NAME_NAME << packet.csName <<
                                   FIELD_NAME_PAGE_SIZE << 65536 ) ;
   bson::BSONObj empty ;

   _tmpBuffer.zero() ;
   _tmpBuffer.reverse( sizeof( MsgOpQuery ) ) ;
   _tmpBuffer.advance( sizeof( MsgOpQuery ) - 4 ) ;

   query = ( MsgOpQuery * )_tmpBuffer.data() ;
   query->header.opCode = MSG_BS_QUERY_REQ ;
   query->header.TID = 0 ;
   query->header.routeID.value = 0 ;
   query->header.requestID = 0 ;
   query->version = 0 ;
   query->w = 0 ;
   query->padding = 0 ;
   query->flags = 0 ;
   query->numToSkip = 0 ;
   query->numToReturn = -1 ;
   query->nameLength = ossStrlen( cmdName ) ;

   _tmpBuffer.write( cmdName, query->nameLength + 1, TRUE ) ;
   _tmpBuffer.write( obj, TRUE ) ;
   _tmpBuffer.write( empty, TRUE ) ;
   _tmpBuffer.write( empty, TRUE ) ;
   _tmpBuffer.write( empty, TRUE ) ;
   _tmpBuffer.doneLen() ;

   rc = _processMsg( (CHAR*)query ) ;

   return rc ;
}

INT32 _mongoSession::_autoCreateCL()
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   mongoDataPacket &packet = _converter.getParser().dataPacket() ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_CREATE_COLLECTION ;
   bson::BSONObj obj       = BSON( FIELD_NAME_NAME << packet.fullName.c_str() );
   bson::BSONObj empty ;

   while( TRUE )
   {
      _tmpBuffer.zero() ;
      _tmpBuffer.reverse( sizeof( MsgOpQuery ) ) ;
      _tmpBuffer.advance( sizeof( MsgOpQuery ) - 4 ) ;

      query = ( MsgOpQuery * )_tmpBuffer.data() ;
      query->header.opCode = MSG_BS_QUERY_REQ ;
      query->header.TID = 0 ;
      query->header.routeID.value = 0 ;
      query->header.requestID = 0 ;
      query->version = 0 ;
      query->w = 0 ;
      query->padding = 0 ;
      query->flags = 0 ;
      query->nameLength = ossStrlen( cmdName ) ;
      query->numToSkip = 0 ;
      query->numToReturn = -1 ;

      _tmpBuffer.write( cmdName, query->nameLength + 1, TRUE ) ;
      _tmpBuffer.write( obj, TRUE ) ;
      _tmpBuffer.write( empty, TRUE ) ;
      _tmpBuffer.write( empty, TRUE ) ;
      _tmpBuffer.write( empty, TRUE ) ;
      _tmpBuffer.doneLen() ;

      rc = _processMsg( (CHAR*)query ) ;
      if ( SDB_DMS_CS_NOTEXIST == rc )
      {
         rc = _autoCreateCS() ;
         if ( rc )
         {
            break ;
         }
      }
      else
      {
         break ;
      }
   }

   return rc ;
}

INT32 _mongoSession::_processMsg( const CHAR *pMsg )
{
   INT32 rc  = SDB_OK ;
   INT32 errCode = SDB_OK ;
   INT32 bodyLen = 0 ;
   BOOLEAN needReply = FALSE ;
   BOOLEAN needRollback = FALSE ;
   bson::BSONObjBuilder bob ;
   bson::BSONObjBuilder retBuilder ;
   mongoDataPacket &packet = _converter.getParser().dataPacket() ;

   rc = _onMsgBegin( (MsgHeader *) pMsg ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

   _contextBuff.release() ;
   rc = getProcessor()->processMsg( (MsgHeader *) pMsg,
                                    _contextBuff, _replyHeader.contextID,
                                    needReply,
                                    needRollback,
                                    retBuilder ) ;
   _errorInfo = engine::utilGetErrorBson( rc,
                _pEDUCB->getInfo( engine::EDU_INFO_ERROR ) ) ;
   if ( SDB_OK != rc )
   {
      if ( needRollback )
      {
         PD_LOG( PDDEBUG, "Session rolling back operation "
                 "(opCode=%d, rc=%d)", ((MsgHeader*)pMsg)->opCode, rc ) ;

         INT32 rcTmp = getProcessor()->doRollback() ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Session failed to rollback trans "
                    "info, rc: %d", rcTmp ) ;
         }
      }

      errCode = _errorInfo.getIntField( OP_ERRNOFIELD ) ;
      // build error msg
      bob.append( "ok", FALSE ) ;
      if ( SDB_IXM_DUP_KEY == errCode )
      {
         // for assert in testcase of c driver for mongodb
         errCode = 11000 ;
      }
      bob.append( "code",  errCode ) ;
      bob.append( "errmsg", _errorInfo.getStringField( OP_ERRDESP_FIELD) ) ;
      bob.append( "err", _errorInfo.getStringField( OP_ERRDESP_FIELD) ) ;
      _contextBuff = engine::rtnContextBuf( bob.obj() ) ;
   }

   bodyLen = _contextBuff.size() ;
   _replyHeader.numReturned = _contextBuff.recordNum() ;
   _replyHeader.startFrom = (INT32)_contextBuff.getStartFrom() ;
   _replyHeader.flags = rc ;

   // when msg is with $cmd, need to reply
   // so value of bodyLen cannot be 0
   if ( packet.with( OPTION_CMD ) && OP_FIND != _converter.getOpType() )
   {
      if ( 0 == bodyLen )
      {
         errCode = _errorInfo.getIntField( OP_ERRNOFIELD ) ;
         if ( SDB_OK != rc )
         {
            // build error msg
            bob.append( "ok", FALSE ) ;
            bob.append( "code",  errCode ) ;
            bob.append( "errmsg", _errorInfo.getStringField( OP_ERRDESP_FIELD) ) ;
            bob.append( "err", _errorInfo.getStringField( OP_ERRDESP_FIELD) ) ;
         }
         else
         {
            bob.append( "ok", TRUE ) ;
         }

         _contextBuff = engine::rtnContextBuf( bob.obj() ) ;
         _replyHeader.flags = rc ;
      }
   }

   _onMsgEnd( rc, (MsgHeader *) pMsg ) ;

done:
   return rc ;
error:
   goto done ;
}

INT32 _mongoSession::_onMsgBegin( MsgHeader *msg )
{
   // set reply header ( except flags, length )
   _replyHeader.contextID          = -1 ;
   _replyHeader.numReturned        = 0 ;
   _replyHeader.startFrom          = 0 ;
   _replyHeader.header.opCode      = MAKE_REPLY_TYPE(msg->opCode) ;
   _replyHeader.header.requestID   = msg->requestID ;
   _replyHeader.header.TID         = msg->TID ;
   _replyHeader.header.routeID     = engine::pmdGetNodeID() ;

   // start operator
   MON_START_OP( _pEDUCB->getMonAppCB() ) ;

   return SDB_OK ;
}

INT32 _mongoSession::_onMsgEnd( INT32 result, MsgHeader *msg )
{
   // release buff context
   //_contextBuff.release() ;

   if ( result && SDB_DMS_EOC != result )
   {
      PD_LOG( PDWARNING, "Session[%s] process msg[opCode=%d, len: %d, "
              "TID: %d, requestID: %llu] failed, rc: %d",
              sessionName(), msg->opCode, msg->messageLength, msg->TID,
              msg->requestID, result ) ;
   }

   // end operator
   MON_END_OP( _pEDUCB->getMonAppCB() ) ;

   return SDB_OK ;
}

INT32 _mongoSession::_reply( MsgOpReply *replyHeader,
                             const CHAR *pBody,
                             const INT32 len )
{
   INT32 rc         = SDB_OK ;
   INT32 offset     = 0 ;
   mongoMsgReply reply ;
   bson::BSONObjBuilder bob ;
   bson::BSONObj bsonBody ;
   bson::BSONObj objToSend ;
   mongoDataPacket &packet = _converter.getParser().dataPacket() ;

   if ( OP_KILLCURSORS == _converter.getOpType() ||
        dbInsert == packet.opCode ||
        dbUpdate == packet.opCode ||
        dbDelete == packet.opCode )
   {
      // should not send any msg
      goto done;
   }
   // id
   reply.header.requestId = 0 ;//replyHeader->header.requestID ;
   // responseTo, cast UINT64 to INT32
   reply.header.responseTo = packet.requestId ;
   // opCode
   reply.header.opCode = dbReply ;
   // _flags
   reply.header.flags = 0 ;
   // _version
   reply.header.version = 0 ;
   // reservedFlag
   reply.header.reservedFlags = 0 ;
   if ( SDB_AUTH_AUTHORITY_FORBIDDEN == replyHeader->flags )
   {
      reply.header.reservedFlags |= 2 ;
   }
   // startingFrom
   if ( -1 != replyHeader->contextID )
   {
      _cursorStartFrom.cursorId = reply.cursorId ;
      reply.startingFrom = replyHeader->startFrom ;
      _cursorStartFrom.startFrom = reply.startingFrom
                                   + replyHeader->numReturned ;
   }
   else
   {
      if ( replyHeader->contextID == _cursorStartFrom.cursorId )
      {
         reply.startingFrom = _cursorStartFrom.startFrom ;
      }
      else
      {
         reply.startingFrom = 0;
      }
      // reset cursorStartFrom
      _cursorStartFrom.cursorId = 0 ;
      _cursorStartFrom.startFrom = 0 ;
   }
   //cursorID
   if ( SDB_OK != replyHeader->flags )
   {
      reply.cursorId = 0 ;
   }
   else
   {
      reply.cursorId = replyHeader->contextID + 1 ;
   }

   // nReturn
   if ( packet.with( OPTION_CMD ) &&
        OP_GETMORE != _converter.getOpType() )
   {
      reply.nReturned = ( replyHeader->numReturned > 0 ?
                          replyHeader->numReturned : 1 ) ;
   }
   else
   {
      reply.nReturned = replyHeader->numReturned ;
   }

   if ( reply.nReturned > 1 )
   {
      while ( offset < len )
      {
         bsonBody.init( pBody + offset ) ;
         _outBuffer.write( bsonBody.objdata(), bsonBody.objsize() ) ;
         offset += ossRoundUpToMultipleX( bsonBody.objsize(), 4 ) ;
      }
   }
   else
   {
      if ( pBody )
      {
         if ( 0 == reply.cursorId &&
             ( SDB_OK == _replyHeader.flags &&
               OP_QUERY != _converter.getOpType() ) )
         {
            // error or command
            bsonBody.init( pBody ) ;
            if ( !bsonBody.hasField( "ok" ) )
            {
               bob.append( "ok", 0 == replyHeader->flags ? TRUE : FALSE ) ;
               bob.append( "code", replyHeader->flags ) ;
               bob.append( "err", _errorInfo.getStringField( OP_ERRDESP_FIELD) ) ;
               bob.appendElements( bsonBody ) ;
               objToSend = bob.obj() ;
               _outBuffer.write( objToSend ) ;
               //pBody = objToSend.objdata() ;
               //reply.header.len = sizeof( mongoMsgReply ) + objToSend.objsize() ;
            }
            else
            {
               _outBuffer.write( bsonBody ) ;
            }
         }
         else
         {
            bsonBody.init( pBody ) ;
            _outBuffer.write( bsonBody ) ;
         }
      }
      else
      {
         if ( OP_GETMORE != _converter.getOpType() &&
              OP_CMD_GET_INDEX == _converter.getOpType()  )
         {
            bob.append( "ok", 1.0 ) ;
            objToSend = bob.obj() ;
            _outBuffer.write( objToSend ) ;
         }
      }
   }

   if ( !_outBuffer.empty() )
   {
      pBody = _outBuffer.data() ;
   }
   reply.header.msgLen = sizeof( mongoMsgReply ) + _outBuffer.size() ;

   rc = sendData( (CHAR *)&reply, sizeof( mongoMsgReply ) ) ;
   if ( rc )
   {
      PD_LOG( PDERROR, "Session[%s] failed to send response header, rc: %d",
              sessionName(), rc ) ;
      goto error ;
   }

   if ( pBody )
   {
      rc = sendData( pBody, reply.header.msgLen - sizeof( mongoMsgReply ) ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Session[%s] failed to send response body, rc: %d",
                          sessionName(), rc ) ;
         goto error ;
      }
   }

done:
   return rc ;
error:
   goto done ;
}

BOOLEAN _mongoSession::_preProcessMsg( msgParser &parser,
                                       engine::IResource *resource,
                                       engine::rtnContextBuf &buff )
{
   BOOLEAN handled = FALSE ;
   mongoDataPacket &packet = parser.dataPacket() ;

   if ( OP_CMD_ISMASTER == parser.currentOperation() )
   {
      handled = TRUE ;
      // build ismaster msg
      fap::mongo::buildIsMasterReplyMsg( resource, buff ) ;
   }
   else if ( OP_CMD_GETNONCE == parser.currentOperation() )
   {
      handled = TRUE ;
      // build getnonce msg
      bson::BSONObj obj ;
      obj.init( _inBuffer.data() ) ;
      buff = engine::rtnContextBuf( obj ) ;
      //fap::mongo::buildGetNonceReplyMsg( buff ) ;
   }
   else if ( OP_CMD_GETLASTERROR == parser.currentOperation() )
   {
      handled = TRUE ;
      // build getlasterror msg
      fap::mongo::buildGetLastErrorReplyMsg( _errorInfo, buff ) ;
   }
   else if ( OP_CMD_NOT_SUPPORTED == parser.currentOperation() )
   {
      handled = TRUE ;
      fap::mongo::buildNotSupportReplyMsg( buff,
                                           packet.all.firstElementFieldName() );
   }
   else if ( OP_CMD_PING == parser.currentOperation() )
   {
       handled = TRUE ;
       fap::mongo::buildPingReplyMsg( buff ) ;
   }
   else if ( OP_CMD_WHATSMYURI == parser.currentOperation() )
   {
       handled = TRUE ;
       fap::mongo::buildWhatsmyuriReplyMsg( buff ) ;
   }
   else if ( OP_CMD_BUILDINFO == parser.currentOperation() )
   {
       handled = TRUE ;
       fap::mongo::buildBuildinfoReplyMsg( buff ) ;
   }
   else if ( OP_CMD_GETLOG == parser.currentOperation() )
   {
       handled = TRUE ;
       fap::mongo::buildGetLogReplyMsg( buff ) ;
   }

   if ( handled )
   {
      // make _relpyHeader
      _replyHeader.contextID            = -1 ;
      _replyHeader.numReturned          = 1 ;
      _replyHeader.startFrom            = 0 ;
      _replyHeader.header.opCode        = MAKE_REPLY_TYPE(packet.opCode) ;
      _replyHeader.header.requestID     = packet.requestId ;
      _replyHeader.header.TID           = 0 ;
      _replyHeader.header.routeID.value = 0 ;
      _replyHeader.flags         = SDB_OK ;
   }

   return handled ;
}

void _mongoSession::_handleResponse( const INT32 opType,
                                     engine::rtnContextBuf &buff )
{
   bson::BSONObjBuilder bob ;
   mongoDataPacket packet = _converter.getParser().dataPacket() ;

   if ( SDB_AUTH_AUTHORITY_FORBIDDEN == _replyHeader.flags )
   {
      bson::BSONObj resObj( buff.data() ) ;
      bob.append( "ok", resObj.getIntField( "ok" ) ) ;
      bob.append( "$err", resObj.getStringField( "err" ) ) ;
      bob.append( "code", resObj.getIntField( "code" ) ) ;
      buff = engine::rtnContextBuf( bob.obj() ) ;
      goto done ;
   }

   if ( OP_CMD_COUNT == opType && SDB_OK == _replyHeader.flags )
   {
      // reply: { n: 1 }
      bson::BSONObj resObj( buff.data() ) ;
      bob.append( "n", resObj.getIntField( "Total" ) ) ;
      buff = engine::rtnContextBuf( bob.obj() ) ;
      _replyHeader.contextID = -1 ;
      _replyHeader.startFrom = 0 ;
   }
   else if ( OP_CMD_DISTINCT == opType )
   {
      // reply: { values: [ 1, 3, 4 ], ok: 1 }
      if ( SDB_OK == _replyHeader.flags )
      {
         bob.appendElements( BSONObj( buff.data() ) ) ;
         bob.append( "ok", 1 ) ;
         buff = engine::rtnContextBuf( bob.obj() ) ;
         _replyHeader.contextID = -1 ;
      }
      else if ( SDB_DMS_EOC == _replyHeader.flags )
      {
         bson::BSONArrayBuilder arr( bob.subarrayStart( "values" ) ) ;
         arr.done() ;
         bob.append( "ok", 1 ) ;
         buff = engine::rtnContextBuf( bob.obj() ) ;
         _replyHeader.contextID = -1 ;
      }
   }
   else if ( OP_INSERT == opType && SDB_OK == _replyHeader.flags )
   {
      // reply: { n: 1, ok: 1 }
      bson::BSONObj resObj( buff.data() ) ;
      bob.append( "ok", 1 ) ;
      if ( resObj.hasField( "InsertedNum" ) )
      {
         bob.append( "n", resObj.getIntField( "InsertedNum" ) ) ;
      }
      buff = engine::rtnContextBuf( bob.obj() ) ;
   }
   else if ( OP_REMOVE == opType && SDB_OK == _replyHeader.flags )
   {
      // reply: { n: 1, ok: 1 }
      bson::BSONObj resObj( buff.data() ) ;
      bob.append( "ok", 1 ) ;
      if ( resObj.hasField( "DeletedNum" ) )
      {
         bob.append( "n", resObj.getIntField( "DeletedNum" ) ) ;
      }
      buff = engine::rtnContextBuf( bob.obj() ) ;
   }
   else if ( OP_UPDATE == opType && SDB_OK == _replyHeader.flags )
   {
      // update reply: { ok: 1, n: 1, nModified: 1 }
      // upsert reply: { ok: 1, n: 1, nModified: 0,
      //                 upserted: [ { index: 0, _id: xxx } ] }
      bson::BSONObj resObj( buff.data() ) ;
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
         bson::BSONArrayBuilder sub( bob.subarrayStart( "upserted" ) ) ;
         sub.append( BSON( "index" << 0 <<
                           "_id" << packet.dataInfo.getField( "_id" ) ) ) ;
         sub.done() ;
      }
      buff = engine::rtnContextBuf( bob.obj() ) ;
   }
   else if ( OP_CMD_GET_DBS == opType )
   {
      if ( SDB_OK == _replyHeader.flags )
      {
         bson::BSONArrayBuilder arr( bob.subarrayStart( "databases" ) ) ;
         INT32 offset = 0 ;
         while ( offset < buff.size() )
         {
            bson::BSONObj obj( buff.data() + offset ) ;
            // { Name: "cs" } => { name: "cs" }
            arr.append( BSON( "name" << obj.getStringField( "Name" ) ) ) ;
            offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
         }
         arr.done() ;
         bob.append( "ok", 1 ) ;

         buff = engine::rtnContextBuf( bob.obj() ) ;
         _replyHeader.contextID = -1 ;
         _replyHeader.numReturned = 1 ;
      }
      else if ( SDB_DMS_EOC == _replyHeader.flags )
      {
         bson::BSONArrayBuilder arr( bob.subarrayStart( "databases" ) ) ;
         arr.done() ;
         bob.append( "ok", 1 ) ;
         buff = engine::rtnContextBuf( bob.obj() ) ;
         _replyHeader.contextID = -1 ;
         _replyHeader.numReturned = 1 ;
      }
   }
   else if ( OP_CMD_GET_INDEX == opType && packet.with( OPTION_IDX ) &&
             SDB_OK == _replyHeader.flags )
   {
      INT32 offset = 0 ;
      _tmpBuffer.zero() ;
      while ( offset < buff.size() )
      {
         bson::BSONObj obj( buff.data() + offset ) ;
         bson::BSONObj newObj = _convertIndexObj( obj ) ;

         _tmpBuffer.write( newObj.objdata(), newObj.objsize(), TRUE ) ;
         offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
      }

      buff = engine::rtnContextBuf( _tmpBuffer.data(), _tmpBuffer.size(),
                                    buff.recordNum() ) ;
   }
   else if ( OP_FIND == opType        || OP_CMD_AGGREGATE == opType ||
             OP_CMD_GET_CLS == opType || OP_CMD_GET_INDEX == opType )
   {
      if ( packet.with( OPTION_CMD ) )
      {
         if ( SDB_OK      == _replyHeader.flags ||
              SDB_DMS_EOC == _replyHeader.flags )
         {
            _buildFirstBatch( buff ) ;
         }
      }
      else
      {
         if ( SDB_DMS_EOC == _replyHeader.flags )
         {
            buff = engine::rtnContextBuf() ;
            _replyHeader.numReturned = 0 ;
            _replyHeader.startFrom = _cursorStartFrom.startFrom ;
         }
      }
   }
   else if ( OP_GETMORE == opType )
   {
      if ( packet.with( OPTION_CMD ) && dbQuery == packet.opCode )
      {
         if ( SDB_OK      == _replyHeader.flags ||
              SDB_DMS_EOC == _replyHeader.flags )
         {
            _buildNextBatch( buff ) ;
         }
      }
      else
      {
         if ( SDB_DMS_EOC == _replyHeader.flags )
         {
            buff = engine::rtnContextBuf() ;
            _replyHeader.numReturned = 0 ;
            _replyHeader.startFrom = _cursorStartFrom.startFrom ;
         }
      }
   }

done:
   return ;
}

void _mongoSession::_buildFirstBatch( engine::rtnContextBuf &buff )
{
   // {xxx}, {xxx}... =>
   // { cursor: { firstBatch: [ {xxx}, {xxx}... ], id: 0, ns: "foo.bar" },
   //   ok: 1 }
   mongoDataPacket packet = _converter.getParser().dataPacket() ;
   bson::BSONObjBuilder resultBuilder ;
   bson::BSONObjBuilder cursorBuilder ;

   bson::BSONArrayBuilder arr( cursorBuilder.subarrayStart( "firstBatch" ) ) ;
   INT32 offset = 0 ;
   if ( SDB_DMS_EOC == _replyHeader.flags )
   {
      // do nothing
   }
   else
   {
      while ( offset < buff.size() )
      {
         bson::BSONObj obj( buff.data() + offset ) ;
         offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;

         if ( OP_CMD_GET_INDEX == _converter.getOpType() &&
              packet.with( OPTION_CMD ) )
         {
            bson::BSONObj newObj = _convertIndexObj(obj) ;
            arr.append( newObj ) ;
         }
         else if ( OP_CMD_GET_CLS == _converter.getOpType() )
         {
            // { Name: "foo.bar" } => { name: "bar" }
            const CHAR* clFullName = obj.getStringField( "Name" ) ;
            const CHAR* dotPos = ossStrstr( clFullName, "." ) + 1 ;
            string clShortName = dotPos ;
            unescapeDot( clShortName ) ;
            bson::BSONObj newObj = BSON( "name" << clShortName.c_str() ) ;
            arr.append( newObj ) ;
         }
         else
         {
            arr.append( obj ) ;
         }
      }
   }
   arr.done() ;

   if ( OP_CMD_GET_INDEX == _converter.getOpType() &&
        packet.with( OPTION_CMD ) )
   {
      // index request: { listIndexes: "bar" }
      // index reply:   { ... ns: "foo.$cmd.listIndexes.bar" ... }
      std::string ns = packet.csName ;
      ns += ".$cmd.listIndexes." ;
      ns += packet.all.firstElement().valuestrsafe() ;
      cursorBuilder.append( "ns", ns.c_str() ) ;
   }
   else if ( OP_CMD_GET_CLS == _converter.getOpType() )
   {
      // listCL reply:   { ... ns: "foo.$cmd.listCollections" ... }
      string ns = _converter.getParser().dataPacket().csName ;
      ns += ".$cmd.listCollections" ;
      cursorBuilder.append( "ns", ns.c_str() ) ;
   }
   else
   {
      cursorBuilder.append( "ns", packet.fullName.c_str() ) ;
   }

   cursorBuilder.append( "id", (long long)( _replyHeader.contextID + 1 ) ) ;
   resultBuilder.append( "cursor", cursorBuilder.obj() ) ;
   resultBuilder.append( "ok", 1 ) ;
   buff = engine::rtnContextBuf( resultBuilder.obj() ) ;

   _replyHeader.contextID = -1 ;
   _replyHeader.numReturned = 1 ;
}

void _mongoSession::_buildNextBatch( engine::rtnContextBuf &buff )
{
   // {xxx}, {xxx}... =>
   // { cursor: { nextBatch: [ {xxx}, {xxx}... ],
   //             id: 0,
   //             ns: "foo.$cmd.listIndexes.bar" },
   //   ok: 1 }
   mongoDataPacket packet = _converter.getParser().dataPacket() ;
   bson::BSONObjBuilder resultBuilder ;
   bson::BSONObjBuilder cursorBuilder ;

   bson::BSONArrayBuilder arr( cursorBuilder.subarrayStart( "nextBatch" ) ) ;
   INT32 offset = 0 ;
   if ( SDB_DMS_EOC == _replyHeader.flags )
   {
      // do nothing
   }
   else
   {
      while ( offset < buff.size() )
      {
         bson::BSONObj obj( buff.data() + offset ) ;
         offset += ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
         arr.append( obj ) ;
      }
   }
   arr.done() ;

   // getMore request:
   //  { getMore: <ctxID>, collection: "$cmd.listIndexes.bar" }
   packet.fullName = packet.csName ;
   packet.fullName += "." ;
   packet.fullName += packet.all.getStringField( "collection" ) ;
   cursorBuilder.append( "ns", packet.fullName.c_str() ) ;

   cursorBuilder.append( "id", (long long)( _replyHeader.contextID + 1 ) ) ;
   resultBuilder.append( "cursor", cursorBuilder.obj() ) ;
   resultBuilder.append( "ok", 1 ) ;
   buff = engine::rtnContextBuf( resultBuilder.obj() ) ;

   _replyHeader.contextID = -1 ;
   _replyHeader.numReturned = 1 ;
}

BSONObj _mongoSession::_convertIndexObj( const BSONObj& sdbIdxFmt )
{
   bson::BSONObjBuilder builder ;
   bson::BSONObj sdbIdxDef = sdbIdxFmt.getObjectField( "IndexDef" ) ;

   builder.append( "v", sdbIdxDef.getIntField( "v" ) ) ;
   if ( sdbIdxDef.getBoolField( "unique" ) &&
        sdbIdxDef.getBoolField( "enforced" ) )
   {
      builder.append( "unique", true ) ;
   }
   builder.append( "key", sdbIdxDef.getObjectField( "key" ) ) ;
   builder.append( "name", sdbIdxDef.getStringField( "name" ) ) ;
   builder.append( "ns", _converter.getParser().dataPacket().fullName.c_str() ) ;

   return builder.obj() ;
}

INT32 _mongoSession::_setSeesionAttr()
{
   INT32 rc = SDB_OK ;
   engine::pmdEDUMgr *pmdEDUMgr = _pEDUCB->getEDUMgr() ;
   const CHAR *cmd = CMD_ADMIN_PREFIX CMD_NAME_SETSESS_ATTR ;
   MsgOpQuery *set = NULL ;

   bson::BSONObj obj ;
   bson::BSONObj emptyObj ;

   msgBuffer msgSetAttr ;
   if ( _masterRead )
   {
      goto done ;
   }

   msgSetAttr.reverse( sizeof( MsgOpQuery ) ) ;
   msgSetAttr.advance( sizeof( MsgOpQuery ) - 4 ) ;
   obj = BSON( FIELD_NAME_PREFERED_INSTANCE << PREFER_REPL_MASTER ) ;
   set = (MsgOpQuery *)msgSetAttr.data() ;

   set->header.opCode = MSG_BS_QUERY_REQ ;
   set->header.TID = 0 ;
   set->header.routeID.value = 0 ;
   set->header.requestID = 0 ;
   set->version = 0 ;
   set->w = 0 ;
   set->padding = 0 ;
   set->flags = 0 ;
   set->nameLength = ossStrlen(cmd) ;
   set->numToSkip = 0 ;
   set->numToReturn = -1 ;

   msgSetAttr.write( cmd, set->nameLength + 1, TRUE ) ;
   msgSetAttr.write( obj, TRUE ) ;
   msgSetAttr.write( emptyObj, TRUE ) ;
   msgSetAttr.write( emptyObj, TRUE ) ;
   msgSetAttr.write( emptyObj, TRUE ) ;
   msgSetAttr.doneLen() ;

   // activate edu
   if ( SDB_OK != ( rc = pmdEDUMgr->activateEDU( _pEDUCB ) ) )
   {
      PD_LOG( PDERROR, "Session[%s] activate edu failed, rc: %d",
              sessionName(), rc ) ;
      goto error ;
   }

   rc = _processMsg( msgSetAttr.data() ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   _masterRead = TRUE ;

   // wait edu
   if ( SDB_OK != ( rc = pmdEDUMgr->waitEDU( _pEDUCB ) ) )
   {
      PD_LOG( PDERROR, "Session[%s] wait edu failed, rc: %d",
              sessionName(), rc ) ;
      goto error ;
   }

done:
   return rc ;
error:
   goto done ;
}
