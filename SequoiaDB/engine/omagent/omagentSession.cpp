#include "omagentSession.hpp"
#include "omagentHelper.hpp"
#include "msgMessage.hpp"
#include "omagentTest.hpp"
#include "../bson/bson.h"

using namespace engine ;

namespace engine
{
   _omaSession::_omaSession( SOCKET fd )
//   :pmdSession( fd )
   {
//ossPrintf("1, in omagent session constructor.\n");
      ossMemset( (void*)&_replyHeader, 0, sizeof(_replyHeader) ) ;
   }

   _omaSession::~_omaSession()
   {
   }

   SDB_SESSION_TYPE _omaSession::sessionType() const
   {
      return SDB_SESSION_OMAGENT ;
   }

   INT32 _omaSession::run()
   {
      INT32 rc          = SDB_OK ;
      UINT32 msgSize    = 0 ;
      CHAR *pBuff       = NULL ;
      INT32 buffSize    = 0 ;
      CHAR *pBuffer     = NULL ;
      INT32 bufferSize  = 0 ;

//      rc = testScanHost ( &pBuffer, &bufferSize ) ;
//      rc = testInstallRemoteAgent ( &pBuffer, &bufferSize ) ;
//      rc = testInstallAgentProcess ( &pBuffer, &bufferSize ) ;
//      rc = testRemoveAgentProcess ( &pBuffer, &bufferSize ) ;
//      rc = testStopAgentProcess ( &pBuffer, &bufferSize ) ;
//      rc = testGetHostInfo ( &pBuffer, &bufferSize ) ;
//      rc = testRegHosts ( &pBuffer, &bufferSize ) ;
//      rc = testGetHostName ( &pBuffer, &bufferSize ) ;
        rc = testInstallDBBusiness ( &pBuffer, &bufferSize ) ;
/**********************************/

      rc = _processMsg( (MsgHeader *)pBuffer ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to process omsvc's message, rc = %d", rc ) ;
         goto error ;
      }


   done:
      if ( pBuffer )
      {
         SDB_OSS_FREE ( pBuffer ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaSession::_processMsg( MsgHeader *msg )
   {
      INT32 rc          = SDB_OK ;
//      omaObjBuff objBuff ;
      CHAR* pBody = NULL ;
      INT32 bodyLen     = 0 ;

      // build reply msg header
      rc = _buildReplyHeader( msg ) ;
      if ( rc )
      {

      }
      // process msg
      rc = _processOPMsg( msg, &pBody, bodyLen,
                          _replyHeader.numReturned ) ;
      if ( rc )
      {
         ossPrintf("rc is : %d\n", rc) ;
      }

      BSONObj temp( pBody ) ;
      ossPrintf( "result :\n" ) ;
      ossPrintf( "pBody is : %s\n", temp.toString().c_str() ) ;
      ossPrintf( "bodyLen is : %d\n", bodyLen ) ;
      ossPrintf( "numReturned is : %d\n", _replyHeader.numReturned ) ;

/***************************************************/

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
            PD_LOG ( PDERROR, "Session[%s] failed to send response for omsvc, rc: %d",
                     sessionName(), rcTmp ) ;
         }
      }
*/
   done:
      if ( NULL  != pBody )
      {
         SDB_OSS_FREE( pBody ) ;
         pBody = NULL ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaSession::_reply( MsgOpReply *responseMsg,
                                  const CHAR *pBody,
                                  INT32 bodyLen )
   {
      INT32 rc = SDB_OK ;

      // response header
//      rc = sendData( (const CHAR*)responseMsg, sizeof(MsgOpReply) ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Session[%s] failed to send response header for omsvc, rc: %d",
                 sessionName(), rc ) ;
         goto error ;
      }
      // response body
      if ( pBody )
      {
//         rc = sendData( pBody, bodyLen ) ;
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


   INT32 _omaSession::_processOPMsg( MsgHeader *msg, CHAR **ppBody,
                                         INT32 &bodyLen, INT32 &returnNum )
//   INT32 _omaSession::_processOPMsg( MsgHeader *msg, omaObjBuff &objBuff )
   {
      INT32 rc = SDB_OK ;
//      omaObjBuff objBuff ;

      if ( NULL == ppBody )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      switch( msg->opCode )
      {
         case MSG_BS_QUERY_REQ :
            rc = _onQueryReqMsg( msg, ppBody, bodyLen, returnNum ) ;
            break ;
         default :
            PD_LOG( PDERROR, "Session[%s] recv unknow msg from omsvc[type:[%d]%d, "
                    "len: %d, tid: %d, routeID: %d.%d.%d, reqID: %lld]",
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
/*
      bodyLen = objBuff.size() ;
      returnNum = objBuff.recordNum() ;
      if ( bodyLen > 0 )
         ossMemcpy( *ppBody, objBuff.data(), bodyLen ) ;
*/
   done:
      return rc ;
   error:
      if ( _needRollback )
      {
//         INT32 rcTmp = _omaRollbak() ;
         PD_LOG( PDEVENT, "Something wrong, need to rollback" ) ;
         _needRollback = FALSE ;
      }
      goto done ;
   }

   INT32 _omaSession::_buildReplyHeader( MsgHeader *msg )
   {
      // set reply header ( except flags, length )
      _replyHeader.contextID          = -1 ;
      _replyHeader.numReturned        = 0 ;
      _replyHeader.startFrom          = 0 ;
      _replyHeader.header.opCode      = MAKE_REPLY_TYPE(msg->opCode) ;
      _replyHeader.header.requestID   = msg->requestID ;
      _replyHeader.header.TID         = msg->TID ;
      _replyHeader.header.routeID.value     = 0 ;

      return SDB_OK ;
   }

/*
   INT32 _omaSession::_onQueryReqMsg( MsgHeader *msg,
                                      omaObjBuff &objBuff )
*/
   INT32 _omaSession::_onQueryReqMsg( MsgHeader *msg, CHAR **ppBody,
                                          INT32 &bodyLen, INT32 &returnNum )
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
      _omaCommand *pCommand = NULL ;

      PD_LOG ( PDEVENT, "omsvc request received" ) ;
      // extract command
      rc = msgExtractQuery ( (CHAR *)msg, &flags, &pCollectionName,
                             &numToSkip, &numToReturn, &pQuery,
                             &pFieldSelector, &pOrderByBuffer,
                             &pHintBuffer ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Session[%s] extract omsvc's command msg failed, rc: %d",
                  sessionName(), rc ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // is command
      if ( omaIsCommand ( pCollectionName ) )
      {
         PD_LOG( PDEVENT, "omagent receive command: %s", pCollectionName ) ;
         rc = omaParseCommand ( pCollectionName, &pCommand ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to parse omsvc's command[%s] [rc:%d]",
                    pCollectionName, rc ) ;
            goto error ;
         }
         rc = omaInitCommand( pCommand, flags, numToSkip, numToReturn,
                                  pQuery, pFieldSelector, pOrderByBuffer,
                                  pHintBuffer ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to init omsvc's command for omagent, rc = %d", rc ) ;
            goto error ;
         }
         rc = omaRunCommand( pCommand, ppBody, bodyLen, returnNum ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to run omsvc's command, rc = %d", rc ) ;
            goto error ;
         }
      }
   done:
      if ( pCommand )
      {
         omaReleaseCommand( &pCommand ) ;
      }
      return rc ;
   error:
    goto done ;
   }

} // namespace CLSMGR
