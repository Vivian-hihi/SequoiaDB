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
#include "msg.h"
#include "rtnCommandDef.hpp"
#include "rtn.hpp"
#include "pmd.hpp"
#include "sdbInterface.hpp"
#include "mongoReplyHelper.hpp"

/////////////////////////////////////////////////////////////////
// implement for mongo processor
_mongoSession::_mongoSession( SOCKET fd, engine::IResource *resource )
   : engine::pmdSession( fd ), _resource( resource )
{
   _converter = SDB_OSS_NEW mongoConverter() ;
}

_mongoSession::~_mongoSession()
{
   if ( NULL != _converter )
   {
      SDB_OSS_DEL( _converter ) ;
      _converter = NULL ;
   }

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

   std::vector< msgBuffer * >::iterator it = _inBufferVec.begin() ;
   for ( ; it != _inBufferVec.end(); ++it )
   {
      SDB_OSS_DEL (*it) ;
      (*it) = NULL ;
   }

   _inBufferVec.clear() ;
}

UINT64 _mongoSession::identifyID()
{
   return ossPack32To64( _socket.getLocalIP(), _socket.getLocalPort() ) ;
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
               PD_LOG( PDERROR, "Session failed to recv rest msg, rc: %d",
                       sessionName(), rc ) ;
            }
            break ;
         }
         pBuff[ msgSize ] = 0 ;
         {
            // make sure buffers are empty for coming msg
            _resetBuffers() ;
            // convert msg first
            _converter->loadFrom( pBuff, msgSize ) ;
            rc = _converter->convert( _inBuffer ) ;
            if ( SDB_OK != rc && SDB_OPTION_NOT_SUPPORT != rc)
            {
               goto error ;
            }

            // handle commands before dispatched
            if ( _preProcessMsg( _converter->getParser(),
                                 _resource, _contextBuff ) )
            {
               _pEDUCB->incEventCount() ;
               goto reply ;
            }

            _pEDUCB->incEventCount() ;
            // activate edu
            if ( SDB_OK != ( rc = pmdEDUMgr->activateEDU( _pEDUCB ) ) )
            {
               PD_LOG( PDERROR, "Session[%s] activate edu failed, rc: %d",
                       sessionName(), rc ) ;
               goto error ;
            }

            pInMsg = _inBuffer.data() ;
            while ( NULL != pInMsg )
            {
               // a new loop
               _needReply = FALSE ;
               // process msg
               rc = _processMsg( pInMsg ) ;
               rc = _converter->reConvert( _inBuffer, &_replyHeader ) ;
               if ( SDB_OK != rc )
               {
                  goto reply ;
               }
               else
               {
                  // when rc == SDB_OK && _inBuffer is not empty, shoul retry
                  // to process msg
                  if ( !_inBuffer.empty() )
                  {
                     _contextBuff.release() ;
                     pInMsg = _inBuffer.data() ;
                  }
                  else
                  {
                     // should exit while loop
                     pInMsg = NULL ;
                  }
               }
            }
         reply:
            handleResponse( _converter->getOpType(), _contextBuff ) ;
            pBody = _contextBuff.data() ;
            bodyLen = _contextBuff.size() ;
            // send response
            INT32 rcTmp = _reply( &_replyHeader, pBody, bodyLen ) ;
            if ( rcTmp )
            {
               PD_LOG( PDERROR, "Session[%s] failed to send response, rc: %d",
                       sessionName(), rcTmp ) ;
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

INT32 _mongoSession::_processMsg( const CHAR *pMsg )
{
   INT32 rc  = SDB_OK ;
   INT32 tmp = SDB_OK ;
   INT32 bodyLen = 0 ;
   bson::BSONObjBuilder bob ;

   rc = _onMsgBegin( (MsgHeader *) pMsg ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

   {
      rc = getProcessor()->processMsg( (MsgHeader *) pMsg,
                                       _contextBuff, _replyHeader.contextID,
                                       _needReply ) ;
      bodyLen = _contextBuff.size() ;
      _replyHeader.numReturned = _contextBuff.recordNum() ;
      _replyHeader.startFrom = (INT32)_contextBuff.getStartFrom() ;
      _replyHeader.flags = rc ;
   }

   // when SDB_OK != rc, or msg is with $cmd, need to reply
   // so value of bodyLen cannot be 0
   if ( ( rc || _converter->getParser().withCmd ) && ( 0 == bodyLen ) )
   {
      _errorInfo = engine::utilGetErrorBson( rc,
                   _pEDUCB->getInfo( engine::EDU_INFO_ERROR ) ) ;

      tmp = _errorInfo.getIntField( OP_ERRNOFIELD ) ;
      if ( SDB_OK != rc )
      {
         bob.append( "ok", FALSE ) ;
         bob.append( "code",  tmp ) ;
         bob.append( "errmsg", _errorInfo.getStringField( OP_ERRDESP_FIELD) ) ;
      }
      else
      {
         bob.append( "ok", TRUE ) ;
      }
      _contextBuff = engine::rtnContextBuf( bob.obj() ) ;

      _replyHeader.numReturned = 1 ;
      _replyHeader.startFrom = 0 ;
      _replyHeader.flags = rc ;
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

   if ( MSG_BS_INTERRUPTE == msg->opCode ||
        MSG_BS_INTERRUPTE_SELF == msg->opCode ||
        MSG_BS_DISCONNECT == msg->opCode  )
   {
      _needReply = FALSE ;
   }
   else if ( MSG_BS_INSERT_REQ == msg->opCode ||
             MSG_BS_DELETE_REQ == msg->opCode ||
             MSG_BS_UPDATE_REQ == msg->opCode )
   {
      _needReply = FALSE ;
   }
   else
   {
      _needReply = TRUE ;
   }

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

   // id
   reply.header.id = 0 ;
   // responseTo, cast UINT64 to INT32
   reply.header.responseTo = replyHeader->header.requestID ;
   // opCode
   reply.header.opCode = dbReply ;
   // _flags
   reply.header._flags = 0 ;
   // _version
   reply.header._version = 0 ;
   // reservedFlag
   reply.header.reservedFlags = 0 ;
   //cursorID
   if ( _converter->getParser().withCmd )
   {
      reply.cursorId = 0 ;
   }
   else
   {
      reply.cursorId = replyHeader->contextID + 1 ;
   }
   // startingFrom
   reply.startingFrom = replyHeader->startFrom ;
   // nReturn
   if ( _converter->getParser().withCmd )
   {
      reply.nReturned = replyHeader->numReturned > 0 ? replyHeader->numReturned : 1 ;
   }
   else
   {
      reply.nReturned = replyHeader->numReturned ;
   }

   if ( !_converter->getParser().withCmd )// && reply.nReturned > 0 )
   {
      while ( offset < len )
      {
         bsonBody.init( pBody + offset ) ;
         _outBuffer.write( bsonBody.objdata(), bsonBody.objsize() ) ;
         offset += ossRoundUpToMultipleX( bsonBody.objsize(), 4 ) ;
      }
      pBody = _outBuffer.data() ;
      reply.header.len = sizeof( mongoMsgReply ) + _outBuffer.size() ;
   }
   else
   {
      if ( pBody )
      {
         bsonBody.init( pBody ) ;
         if ( !bsonBody.hasField( "ok" ) )
         {
            bob.append( "ok", 0 == replyHeader->flags ? TRUE : FALSE ) ;
            bob.append( "code", replyHeader->flags ) ;
            bob.appendElements( bsonBody ) ;
            objToSend = bob.obj() ;
            pBody = objToSend.objdata() ;
            reply.header.len = sizeof( mongoMsgReply ) + objToSend.objsize() ;
         }
         else
         {
            reply.header.len = sizeof( mongoMsgReply ) + len ;
         }
      }
      else
      {
         bob.append( "ok", 1.0 ) ;
         objToSend = bob.obj() ;
         pBody = objToSend.objdata() ;
         reply.header.len = sizeof( mongoMsgReply ) + objToSend.objsize() ;
      }
   }

   rc = sendData( (CHAR *)&reply, sizeof( mongoMsgReply ) ) ;
   if ( rc )
   {
      PD_LOG( PDERROR, "Session[%s] failed to send response header, rc: %d",
              sessionName(), rc ) ;
      goto error ;
   }

   if ( pBody )
   {
      rc = sendData( pBody, reply.header.len - sizeof( mongoMsgReply ) ) ;
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

BOOLEAN _mongoSession::_preProcessMsg( const mongoParser &parser,
                                       engine::IResource *resource,
                                       engine::rtnContextBuf &buff )
{
   BOOLEAN handled = FALSE ;

   if ( OP_CMD_ISMASTER == parser.opType )
   {
      handled = TRUE ;
      // build ismaster msg
      fap::mongo::buildIsMasterReplyMsg( resource, buff ) ;
   }
   else if ( OP_CMD_GETNONCE == parser.opType )
   {
      handled = TRUE ;
      // build getnonce msg
      fap::mongo::buildGetNonceReplyMsg( buff ) ;
   }
   else if ( OP_CMD_GETLASTERROR == parser.opType )
   {
      handled = TRUE ;
      // build getlasterror msg
      fap::mongo::buildGetLastErrorReplyMsg( _errorInfo, buff ) ;
   }
   else if ( OP_CMD_NOT_SUPPORTED == parser.opType )
   {
      handled = TRUE ;
      fap::mongo::buildNotSupportReplyMsg( _contextBuff ) ;
   }

   if ( handled )
   {
      // make _relpyHeader
      _replyHeader.contextID            = -1 ;
      _replyHeader.numReturned          = 1 ;
      _replyHeader.startFrom            = 0 ;
      _replyHeader.header.opCode        = MAKE_REPLY_TYPE(parser.opCode) ;
      _replyHeader.header.requestID     = parser.id ;
      _replyHeader.header.TID           = 0 ;
      _replyHeader.header.routeID.value = 0 ;
   }

   return handled ;
}

void _mongoSession::handleResponse( const INT32 opType,
                                    engine::rtnContextBuf &buff )
{
   if ( OP_CMD_COUNT_MORE == opType )
   {
      bson::BSONObjBuilder bob ;
      bson::BSONObj obj( buff.data() ) ;
      bob.append( "n", obj.getIntField( "Total" ) ) ;
      buff = engine::rtnContextBuf( bob.obj() ) ;
   }
}
