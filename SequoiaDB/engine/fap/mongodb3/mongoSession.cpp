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

namespace fap
{

static BOOLEAN checkBigEndian()
{
   BOOLEAN bigEndian = FALSE ;
   union
   {
      unsigned int i ;
      unsigned char s[4] ;
   } c ;

   c.i = 0x12345678 ;
   if ( 0x12 == c.s[0] )
   {
      bigEndian = TRUE ;
   }

   return bigEndian ;
}

static void buildGetMoreMsg( UINT64 requestID, INT64 contextID, msgBuffer &out )
{
   if ( !out.empty() )
   {
      out.zero() ;
   }
   out.reserve( sizeof( MsgOpGetMore ) ) ;
   out.advance( sizeof( MsgOpGetMore ) ) ;

   MsgOpGetMore *getmore = (MsgOpGetMore *)out.data() ;
   getmore->header.messageLength = sizeof( MsgOpGetMore ) ;
   getmore->header.opCode = MSG_BS_GETMORE_REQ ;
   getmore->header.requestID = requestID ;
   getmore->header.routeID.value = 0 ;
   getmore->header.TID = 0 ;
   getmore->contextID = contextID ;
   getmore->numToReturn = -1 ;
}

/////////////////////////////////////////////////////////////////
// implement for mongo processor

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
   UINT32 headerLen             = sizeof( mongoMsgHeader ) ;
   engine::pmdEDUMgr *pmdEDUMgr = NULL ;
   CHAR *pBuff                  = NULL ;
   const CHAR *pInMsg           = NULL ;
   INT32 orgOpCode              = 0 ;
   BOOLEAN hasBuildGetMore      = FALSE ;
   _mongoCommand* pCommand      = NULL ;
   engine::monDBCB *mondbcb     = engine::pmdGetKRCB()->getMonDBCB() ;
   mongoSessionCtx sessCtx ;

   if ( !_pEDUCB )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   pmdEDUMgr = _pEDUCB->getEDUMgr() ;

   bigEndian = checkBigEndian() ;
   PD_CHECK( !bigEndian, SDB_SYS, error, PDERROR,
             "Big endian is not support " ) ;

   while ( !_pEDUCB->isDisconnected() && !_socket.isClosed() )
   {
      // clear interrupt flag
      _pEDUCB->resetInterrupt() ;
      _pEDUCB->resetInfo( engine::EDU_INFO_ERROR ) ;
      _pEDUCB->resetLsn() ;

      // receive msgLen
      rc = recvData( (CHAR*)&msgSize, sizeof(UINT32) ) ;
      if ( rc )
      {
         if ( SDB_APP_FORCED != rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to recv msg size, "
                    "rc: %d", sessionName(), rc ) ;
         }
         goto error ;
      }

      if ( msgSize < headerLen || msgSize > SDB_MAX_MSG_LENGTH )
      {
         PD_LOG( PDERROR, "Session[%s] recv msg size[%d] is less than "
                 "mongoMsgHeader size[%d] or more than max msg size[%d]",
                 sessionName(), msgSize, headerLen, SDB_MAX_MSG_LENGTH ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // receive rest of message
      pBuff = getBuff( msgSize + 1 ) ;
      PD_CHECK( pBuff, SDB_OOM, error, PDERROR, "Out of memory" ) ;

      *(UINT32*)pBuff = msgSize ;

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

      // activate edu
      _pEDUCB->incEventCount() ;
      mondbcb->addReceiveNum() ;

      rc = pmdEDUMgr->activateEDU( _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Session[%s] activate edu failed, rc: %d",
                   sessionName(), rc ) ;

      // convert message to command
      _resetBuffers() ;
      rc = mongoGetAndInitCommand( pBuff, &pCommand, sessCtx, _inBuffer ) ;
      PD_CHECK( SDB_OK == rc, rc, reply, PDERROR,
                "Session[%s] failed to get and init command, rc: %d",
                sessionName(), rc ) ;

      if ( !pCommand->needProcessByEngine() )
      {
         goto reply ;
      }

      // process message
      pInMsg = _inBuffer.data() ;
      orgOpCode = ((MsgHeader*)pInMsg)->opCode ;
      hasBuildGetMore = FALSE ;
      while ( TRUE )
      {
         rc = _processMsg( pInMsg ) ;

         // auto create cs/cl
         if ( SDB_DMS_CS_NOTEXIST == _replyHeader.flags )
         {
            if ( CMD_COLLECTION_CREATE == pCommand->type() )
            {
               if ( SDB_OK == _autoCreateCS( pCommand->csName() ) )
               {
                  ((MsgHeader*)pInMsg)->opCode = orgOpCode ;
                  continue ;
               }
            }
         }
         else if ( SDB_DMS_NOTEXIST == _replyHeader.flags )
         {
            if ( CMD_INSERT == pCommand->type() ||
                 CMD_INDEX_CREATE == pCommand->type() ||
                 ( CMD_UPDATE == pCommand->type() &&
                 ((_mongoUpdateCommand*)pCommand)->isUpsert()) )
            {
               if ( SDB_OK == _autoCreateCL( pCommand->clFullName() ) )
               {
                  ((MsgHeader*)pInMsg)->opCode = orgOpCode ;
                  continue ;
               }
            }
         }
         else if ( SDB_OK == _replyHeader.flags )
         {
            if ( !hasBuildGetMore && _needGetMore( pCommand->type() ) )
            {
               buildGetMoreMsg( _replyHeader.header.requestID,
                                _replyHeader.contextID, _inBuffer ) ;
               hasBuildGetMore = TRUE ;
               continue ;
            }
         }

         break ;
      }

      // reply to mongo client
   reply:
      rc = _reply( rc, pCommand, _replyHeader, _contextBuff ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Session[%s] failed to reply, rc: %d",
                   sessionName(), rc ) ;

      mongoReleaseCommand( &pCommand ) ;
      _contextBuff.release() ;

      // wait edu
      rc = pmdEDUMgr->waitEDU( _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Session[%s] wait edu failed, rc: %d",
                   sessionName(), rc ) ;
   }

done:
   if ( pCommand )
   {
      mongoReleaseCommand( &pCommand ) ;
   }
   disconnect() ;
   return rc ;
error:
   goto done ;
}

INT32 _mongoSession::_autoCreateCS( const CHAR* csName )
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_CREATE_COLLECTIONSPACE ;
   bson::BSONObj obj       = BSON( FIELD_NAME_NAME << csName <<
                                   FIELD_NAME_PAGE_SIZE << 65536 ) ;
   bson::BSONObj empty ;

   _tmpBuffer.zero() ;
   _tmpBuffer.reserve( sizeof( MsgOpQuery ) ) ;
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

   if ( SDB_OK == rc )
   {
      PD_LOG( PDEVENT,
              "Session[%s]: Create collection space[%s] automatically",
              sessionName(), csName ) ;
   }
   else
   {
      PD_LOG( PDWARNING,
              "Session[%s]: failed to create collection space[%s] automatically"
              ", rc: %d", sessionName(), csName, rc ) ;
      if ( SDB_DMS_CS_EXIST == rc )
      {
         rc = SDB_OK ;
      }
   }

   return rc ;
}

INT32 _mongoSession::_autoCreateCL( const CHAR* clFullName )
{
   INT32 rc                = SDB_OK ;
   MsgOpQuery *query       = NULL ;
   const CHAR *cmdName     = CMD_ADMIN_PREFIX CMD_NAME_CREATE_COLLECTION ;
   bson::BSONObj obj       = BSON( FIELD_NAME_NAME << clFullName );
   bson::BSONObj empty ;

   while( TRUE )
   {
      _tmpBuffer.zero() ;
      _tmpBuffer.reserve( sizeof( MsgOpQuery ) ) ;
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
         string csName ;
         csName.assign( clFullName, ossStrstr( clFullName, "." ) - clFullName ) ;
         rc = _autoCreateCS( csName.c_str() ) ;
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

   if ( SDB_OK == rc )
   {
      PD_LOG( PDEVENT,
              "Session[%s]: Create collection[%s] automatically",
              sessionName(), clFullName ) ;
   }
   else
   {
      PD_LOG( PDWARNING,
              "Session[%s]: failed to create collection[%s] automatically"
              ", rc: %d", sessionName(), clFullName, rc ) ;
      if ( SDB_DMS_EXIST == rc )
      {
         rc = SDB_OK ;
      }
   }

   return rc ;
}

BOOLEAN _mongoSession::_needGetMore( MONGO_CMD_TYPE opType )
{
   if ( CMD_COUNT           == opType ||
        CMD_LIST_INDEX      == opType ||
        CMD_LIST_COLLECTION == opType ||
        CMD_AGGREGATE       == opType ||
        CMD_LIST_DATABASE   == opType ||
        CMD_DISTINCT        == opType )
   {
      if ( -1 != _replyHeader.contextID )
      {
         return TRUE ;
      }
   }

   return FALSE ;
}

INT32 _mongoSession::_processMsg( const CHAR *pMsg )
{
   INT32 rc  = SDB_OK ;
   BOOLEAN needReply = FALSE ;
   BOOLEAN needRollback = FALSE ;
   bson::BSONObjBuilder retBuilder ;

   rc = _onMsgBegin( (MsgHeader *) pMsg ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

   _contextBuff.release() ;
   rc = getProcessor()->processMsg( (MsgHeader *) pMsg, _contextBuff,
                                    _replyHeader.contextID,
                                    needReply, needRollback, retBuilder ) ;
   _errorInfo = engine::utilGetErrorBson( rc,
                              _pEDUCB->getInfo( engine::EDU_INFO_ERROR ) ) ;
   if ( rc && needRollback )
   {
      PD_LOG( PDDEBUG,
              "Session rolling back operation[opCode: %d], rc: %d",
              ((MsgHeader*)pMsg)->opCode, rc ) ;

      INT32 rcTmp = getProcessor()->doRollback() ;
      PD_RC_CHECK( rcTmp, PDERROR,
                   "Session failed to rollback trans info, rc: %d",
                   rcTmp ) ;
   }

   _replyHeader.numReturned = _contextBuff.recordNum() ;
   _replyHeader.startFrom = (INT32)_contextBuff.getStartFrom() ;
   _replyHeader.flags = rc ;

   if ( rc )
   {
      bson::BSONObjBuilder bob ;
      bob.append( "ok", 0 ) ;
      bob.append( "code",  _errorInfo.getIntField( OP_ERRNOFIELD ) ) ;
      bob.append( "errmsg", _errorInfo.getStringField( OP_ERRDESP_FIELD ) ) ;
      _contextBuff = engine::rtnContextBuf( bob.obj() ) ;
      _replyHeader.numReturned = 1 ;
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

INT32 _mongoSession::_reply( INT32 errCode,
                             _mongoCommand* pCommand,
                             MsgOpReply &replyHeader,
                             engine::rtnContextBuf &_contextBuff )
{
   INT32 rc = SDB_OK ;
   _mongoResponseBuffer headerBuf ;

   // build response
   if ( errCode )
   {
      bson::BSONObjBuilder bob ;
      bob.append( "ok", 0 ) ;
      bob.append( "code",  errCode ) ;
      bob.append( "errmsg", getErrDesp( errCode ) ) ;
      _contextBuff = engine::rtnContextBuf( bob.obj() ) ;
      _replyHeader.numReturned = 1 ;
      _replyHeader.flags = errCode ;
   }

   rc = mongoPostRunCommand( pCommand, _replyHeader, _contextBuff, headerBuf ) ;
   PD_RC_CHECK( rc, PDERROR,
                "Session[%s] failed to build response, rc: %d",
                sessionName(), rc ) ;

   // send response
   if ( headerBuf.usedSize > 0 )
   {
      INT32 rcTmp = SDB_OK ;
      rcTmp = sendData( (CHAR *)&headerBuf, headerBuf.usedSize ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Session[%s] failed to send response header, rc: %d",
                   sessionName(), rcTmp ) ;

      if ( _contextBuff.data() )
      {
         rcTmp = sendData( _contextBuff.data(), _contextBuff.size() ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Session[%s] failed to send response body, rc: %d",
                      sessionName(), rcTmp ) ;
      }
   }

done:
   return rc ;
error:
   goto done ;
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

   msgSetAttr.reserve( sizeof( MsgOpQuery ) ) ;
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

}
