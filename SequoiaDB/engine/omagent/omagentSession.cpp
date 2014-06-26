#include "omagentSession.cpp"

using namespace engine ;

namespace CLSMGR
{
   _omagentSession::_omagentSession( SOCKET fd )
   :pmdSession( fd )
   {
      ossMemset( (void*)&_replyHeader, 0, sizeof(_replyHeader) ) ;
   }

   _omagentSession:~_omagentSession()
   {
   }

   INT32 _omagentSession::run()
   {
      INT32 rc          = SDB_OK ;
      UINT32 msgSize    = 0 ;
      CHAR *pBuff       = NULL ;
      INT32 buffSize    = 0 ;
   ossPrintf( "in omagentSession, run"OSS_NEWLINE ) ;

      const CHAR* cmd = "$scan host" ;
      MsgOpQuery qmsg ;
      qmsg.header.messageLength = sizeof(MsgOpQuery) + ossStrlen( cmd ) + 1 ;
      ossMemcpy ( qmsg.name, cmd, ossStrlen( cmd ) + 1 ) ;

      rc = _processMsg( (MsgHeader *)qmsg ) ;
      ossPrintf( "rc is: %d\n", rc ) ;


   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omagentSession::_processMsg( MsgHeader *msg )
   {
      INT32 rc          = SDB_OK ;
//      const CHAR *pBody = NULL ;
      CHAR pBody[4096] = {0} ;
      INT32 bodyLen     = 0 ;

      // build reply msg header
      rc = _buildReplyHeader( msg ) ;
      if ( rc )
      {

      }
      // process msg
      rc = _processOPMsg( msg, &pBody, bodyLen,
                          _replyHeader.numReturned,
                          _replyHeader.startFrom ) ;
      if ( rc )
      {
         ossPrintf("rc is : %d\n", rc) ;
      }
      ossPrintf( "result :\n" ) ;
      ossPrintf( "pBody is : %s\n", pBody ) ;
      ossPrintf( "bodyLen is : %d\n", bodyLen ) ;
/*
      if ( _needReply )
      {
         if ( rc && bodyLen == 0 )
         {
            _errorInfo = BSON( "errmsg" << "something wrong for process omsvc reques"
                               << "errcode" << rc ) ;
            _replyHeader.numReturned = -1 ;
         }
         _replyHeader.flags = rc ;
         _replayHeader.header.messageLength = sizeof( _replyHeader ) + bodyLen ;

         // send response
         INT32 rcTmp = _reply( &_replyHeader, pBody, bodyLen ) ;
         if ( rcTmp )
         {
            ossPrintf ( "Session[%s] failed to send response for omsvc, rc: %d"OSS_NEWLINE,
                        sessionName(), rcTmp ) ;
         }
      }
*/
      rc = SDB_OK ;
      return rc ;
   }

   INT32 _omagentSession::_reply( MsgOpReply *responseMsg,
                                  const CHAR *pBody,
                                  INT32 bodyLen )
   {
      INT32 rc = SDB_OK ;

      // response header
      rc = sendData( (const CHAR*)responseMsg, sizeof(MsgOpReply) ) ;
      if ( rc )
      {
         ossPrintf( "Session[%s] failed to send response header for omsvc, rc: %d"OSS_NEWLINE,
                 sessionName(), rc ) ;
         goto error ;
      }
      // response body
      if ( pBody )
      {
         rc = sendData( pBody, bodyLen ) ;
         if ( rc )
         {
            ossPrintf( "Session[%s] failed to send response body, rc: %d"OSS_NEWLINE,
                    sessionName(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omagentSession::_processOPMsg( MsgHeader *msg, const CHAR **ppBody,
                                         INT32 &bodyLen, INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;
      omagentObjBuff objBuff ;

      if ( NULL == ppBody )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      switch( msg->opCode )
      {
         case MSG_BS_QUERY_REQ :
            rc = _onQueryReqMsg( msg, objBuff ) ;
            break ;
         default :
            ossPrintf( "Session[%s] recv unknow msg from omsvc[type:[%d]%d, "
                    "len: %d, tid: %d, routeID: %d.%d.%d, reqID: %lld]"OSS_NEWLINE,
                    sessionName(), IS_REPLY_TYPE(msg->opCode),
                    GET_REQUEST_TYPE(msg->opCode), msg->messageLength, msg->TID,
                    msg->routeID.columns.groupID, msg->routeID.columns.nodeID,
                    msg->routeID.columns.serviceID, msg->requestID ) ;
            rc = SDB_INVALIDARG ;
            break ;
      }

      if  ( rc )
      {
         goto error ;
      }

      bodyLen = objBuff.size() ;
      recordNum = objBuff.recordNum() ;
      if ( bodyLen > 0 )
         ossMemcpy( *ppBody, objBuff.data(), bodyLen ) ;
   done:
      return rc ;
   error:
      if ( _needRollbak )
      {
//         INT32 rcTmp = _omagentRollbak() ;
         ossPrintf( "Something wrong, need to rollback"OSS_NEWLINE ) ;
         _needRollback = FALSE ;
      }
      goto done ;
   }

   INT32 _omagentSession::_onQueryReqMsg( MsgHeader *msg,
                                          omagentObjBuff &objBuff )
   {
      INT32 rc                  = SDB_OK ;
      INT32 flags               = 0 ;
      CHAR *pCollectionName     = NULL ;
      CHAR *pQuery              = NULL ;
      CHAR *pFieldSelector      = NULL ;
      CHAR *pOrderByBuffer      = NULL ;
      CHAR *pHintBuffer         = NULL ;
      SINT64 numToSkip          = -1 ;
      SINT64 numToReturn        = -1 ;
      _omagentCommand *pCommand = NULL ;

      ossPrintf ( "omsvc request received"OSS_NEWLINE ) ;
      // extract command
      rc = msgExtractQuery ( (CHAR *)msg, &flags, &pCollectionName,
                             &numToSkip, &numToReturn, &pQuery,
                             &pFieldSelector, &pOrderByBuffer,
                             &pHintBuffer ) ;
      if ( rc )
      {
         ossPrintf ( "Session[%s] extract omsvc's command msg failed, rc: %d"OSS_NEWLINE,
                     sessionName(), rc ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // is command
      if ( omagentIsCommand ( pCollectionName ) )
      {
         ossPrintf( "omagent receive command: %s"OSS_NEWLINE, pCollectionName ) ;
         rc = omagentParseCommand ( pCollectionName, &pCommand ) ;
         if ( SDB_OK != rc )
         {
            ossPrintf( "Failed to parse omsvc's command[%s] [rc:%d]"OSS_NEWLINE,
                       pCollectionName, rc ) ;
            goto error ;
         }
         rc = omagentInitCommand( pCommand, flags, numToSkip, numToReturn,
                                  pQuery, pFieldSelector, pOrderByBuffer,
                                  pHintBuffer ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         rc = rtnRunCommand( pCommand, objBuff ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }
   done:
      if ( pCommand )
      {
         omagentReleaseCommand( &pCommand ) ;
      }
      ossPrintf( "Making reply package for omsvc, err = %d"OSS_NEWLINE, rc ) ;

      return rc ;
   error:
      goto error ;
   }

} // namespace CLSMGR
