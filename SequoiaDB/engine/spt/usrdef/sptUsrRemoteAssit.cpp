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

   Source File Name = sptUsrRemoteAssit.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          18/07/2016  WJM Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptUsrRemoteAssit.hpp"
#include "client.h"
#include "client_internal.h"
#include "pd.hpp"
#include "msgDef.h"
#include "ossUtil.h"
#include "ossTypes.h"
#include "omagentDef.hpp"
#include "network.h"
#include "common.h"

#if defined( _LINUX ) || defined (_AIX)
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#define SDB_CLIENT_DFT_NETWORK_TIMEOUT 10000

namespace engine
{

/*
 define macro
*/


#define HANDLE_CHECK( handle, interhandle, handletype ) \
do                                                      \
{                                                       \
   if ( SDB_INVALID_HANDLE == handle )                  \
   {                                                    \
      rc = SDB_INVALIDARG ;                             \
      goto error ;                                      \
   }                                                    \
   if ( !interhandle ||                                 \
        handletype != interhandle->_handleType )        \
   {                                                    \
      rc = SDB_CLT_INVALID_HANDLE ;                     \
      goto error ;                                      \
   }                                                    \
}while( FALSE )


#define CHECK_RET_MSGHEADER( pSendBuf, pRecvBuf, connHandle )               \
do                                                                          \
{                                                                           \
   sdbConnectionStruct *db = (sdbConnectionStruct*)connHandle ;             \
   HANDLE_CHECK( connHandle, db, SDB_HANDLE_TYPE_CONNECTION ) ;             \
   rc = clientCheckRetMsgHeader( pSendBuf, pRecvBuf, db->_endianConvert ) ; \
   if ( SDB_OK != rc )                                                      \
   {                                                                        \
      if ( SDB_UNEXPECTED_RESULT == rc )                                    \
      {                                                                     \
         sdbDisconnect( connHandle ) ;                                      \
      }                                                                     \
      goto error ;                                                          \
   }                                                                        \
}while( FALSE )



/*
   define member functions
*/

   _sptUsrRemoteAssit::_sptUsrRemoteAssit()
   {
      _handle = 0 ;
   }

   _sptUsrRemoteAssit::~_sptUsrRemoteAssit()
   {
      disconnect() ;
   }

   INT32 _sptUsrRemoteAssit::connect( const CHAR *pHostName,
                                      const CHAR *pServiceName )
   {
      INT32 rc = SDB_OK ;

      // disconnect before connect
      rc = disconnect() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to disconnect" ) ;

      rc = sdbConnect( pHostName, pServiceName, SDB_OMA_USER,
                       SDB_OMA_USERPASSWD, &_handle ) ;
      PD_RC_CHECK( rc, PDERROR, "Connect to %s:%s failed, rc: %d",
                   pHostName, pServiceName, rc ) ;

   done:
      return rc ;
   error:
      disconnect() ;
      goto done ;
   }

   INT32 _sptUsrRemoteAssit::disconnect()
   {
      if ( 0 != _handle )
      {
         sdbDisconnect ( (sdbConnectionHandle)_handle ) ;
         sdbReleaseConnection( (sdbConnectionHandle) _handle ) ;
         _handle = 0 ;
      }

      return SDB_OK ;
   }

   INT32 _sptUsrRemoteAssit::runCommand( const CHAR *pString,
                                         SINT32 flag,
                                         UINT64 reqID,
                                         SINT64 numToSkip,
                                         SINT64 numToReturn,
                                         const CHAR *arg1,
                                         const CHAR *arg2,
                                         const CHAR *arg3,
                                         const CHAR *arg4,
                                         CHAR **ppRetBuffer )
   {
      INT32 rc          = SDB_OK ;
      BOOLEAN result    = FALSE ;
      SINT64 contextID  = 0 ;
      sdbConnectionStruct *connection = 0;

      if( 0 == _handle )
      {
         rc = SDB_NETWORK ;
         PD_LOG( PDERROR, "network is closed, rc = %d", rc ) ;
         goto error ;
      }
      connection = (sdbConnectionStruct *) _handle ;

      // build message
      rc = clientBuildQueryMsgCpp ( &( connection->_pSendBuffer ),
                                    &( connection->_sendBufferSize ),
                                    pString, flag, reqID,
                                    numToSkip, numToReturn,
                                    arg1, arg2, arg3, arg4,
                                    connection->_endianConvert ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to build query msg, rc = %d", rc ) ;


      // send and recv msg
      rc = _sendAndRecv( ( const MsgHeader* )connection->_pSendBuffer,
                         ( MsgHeader** )&( connection->_pReceiveBuffer ),
                         &( connection->_receiveBufferSize ),
                         TRUE,
                         connection->_endianConvert ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to build send and recv msg, rc = %d", rc ) ;

      // extract message
      rc = _extract( (MsgHeader *)connection->_pReceiveBuffer,
                     connection->_receiveBufferSize,
                     &contextID, &result,
                     connection->_endianConvert ) ;
      if ( SDB_OK != rc )
      {
         if ( TRUE == result )
         {
            PD_LOG( PDERROR, "Failed to extract msg in client, rc = %d", rc ) ;
            goto error ;
         }
         else
         {
            INT32 rcSave = rc ;
            PD_LOG( PDERROR, "Failed to run command in engine, rc = %d", rc ) ;
            CHECK_RET_MSGHEADER( connection->_pSendBuffer,
                                 connection->_pReceiveBuffer,
                                 _handle ) ;
            _getRetBuffer( connection->_pReceiveBuffer, ppRetBuffer ) ;
            rc = rcSave ;
            goto error ;
         }
      }


      // check whether the return message is what we want or not
      CHECK_RET_MSGHEADER( connection->_pSendBuffer,
                           connection->_pReceiveBuffer,
                           _handle ) ;

      // try to get retObj
      rc = _getRetBuffer( connection->_pReceiveBuffer, ppRetBuffer ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get retObjArray, rc = %d", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrRemoteAssit::_sendAndRecv( const MsgHeader *sendMsg,
                                           MsgHeader **recvMsg, INT32 *size,
                                           BOOLEAN needRecv,
                                           BOOLEAN endianConvert )
   {
      INT32 rc          = SDB_OK ;
      BOOLEAN hasLock   = FALSE ;
      sdbConnectionStruct *connection = (sdbConnectionStruct*)_handle ;

      // check arguments
      if( NULL == connection->_sock )
      {
         rc = SDB_NOT_CONNECTED ;
         goto error ;
      }
      if( NULL == sendMsg )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      ossMutexLock( &connection->_sockMutex ) ;
      hasLock = TRUE ;

      // send
      rc = _sendMsg ( sendMsg, endianConvert ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to send msg, rc: %d", rc ) ;

      // recv
      if ( TRUE == needRecv )
      {
         rc = _recvMsg ( recvMsg, size, endianConvert ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to recv msg, rc: %d", rc ) ;
      }

   done:
      if ( TRUE == hasLock )
      {
         ossMutexUnlock( &connection->_sockMutex ) ;
      }
      return rc ;
   error:
      if ( SDB_NETWORK_CLOSE == rc || SDB_NETWORK == rc )
      {
         clientDisconnect ( &connection->_sock ) ;
      }
      goto done ;
   }

   INT32 _sptUsrRemoteAssit::_sendMsg( const MsgHeader *msg,
                                       BOOLEAN endianConvert )
   {
      INT32 rc          = SDB_OK ;
      INT32 msgLength   = 0 ;
      ossEndianConvertIf4 ( msg->messageLength, msgLength, endianConvert ) ;

      rc = _send ( (const CHAR*)msg, msgLength ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to send msg, rc: %d", rc) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrRemoteAssit::_recvMsg( MsgHeader **msg, INT32 *msgLength,
                                       BOOLEAN endianConvert )
   {
      INT32 rc         = SDB_OK ;
      INT32 recvLength = 0 ;
      INT32 realLength = 0 ;
      CHAR **ppBuffer  = (CHAR**)msg ;
      sdbConnectionStruct *connection = (sdbConnectionStruct*)_handle ;

      if ( NULL == connection->_sock )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      while ( TRUE )
      {
         // get length first
         rc = clientRecv ( connection->_sock, (CHAR*)&recvLength,
                           sizeof( recvLength ),
                           SDB_CLIENT_DFT_NETWORK_TIMEOUT ) ;
         if ( SDB_TIMEOUT == rc )
         {
            continue ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to get length, rc: %d", rc) ;

#if defined( _LINUX ) || defined (_AIX)
      #if defined (_AIX)
         #define TCP_QUICKACK TCP_NODELAYACK
      #endif
      // quick ack
      {
         INT32 i = 0 ;
         setsockopt( clientGetRawSocket ( connection->_sock ),
                     IPPROTO_TCP, TCP_QUICKACK, (void*)&i, sizeof(i) ) ;
      }
#endif
         break ;
      }
      ossEndianConvertIf4 ( recvLength, realLength, endianConvert ) ;
      rc = _reallocBuffer ( ppBuffer, msgLength , realLength+1 ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to realloc buffer, rc: %d", rc) ;

      // use the original recvLength before convert
      *(SINT32*)(*ppBuffer) = recvLength ;
      while ( TRUE )
      {
         // get residual message
         rc = clientRecv ( connection->_sock,
                           &( *ppBuffer )[sizeof( realLength )],
                           realLength - sizeof( realLength ),
                           SDB_CLIENT_DFT_NETWORK_TIMEOUT ) ;
         if ( SDB_TIMEOUT == rc )
            continue ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to get residual message, rc: %d", rc) ;
         break ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrRemoteAssit::_send( const CHAR *pMsg,
                                    INT32 msgLength )
   {
      INT32 rc = SDB_OK ;
      sdbConnectionStruct *connection = (sdbConnectionStruct*)_handle ;

      if ( NULL == connection->_sock )
      {
         rc = SDB_INVALIDARG ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get valid sock, rc: %d", rc) ;

      rc = clientSend ( connection->_sock, pMsg, msgLength,
                        SDB_CLIENT_DFT_NETWORK_TIMEOUT ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to send, rc: %d", rc) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrRemoteAssit::_reallocBuffer( CHAR **ppBuffer, INT32 *bufferSize,
                                             INT32 newSize)
   {
      INT32 rc              = SDB_OK ;
      CHAR *pOriginalBuffer = NULL ;

      if ( *bufferSize < newSize )
      {
         pOriginalBuffer = *ppBuffer ;
         *ppBuffer = (CHAR*)SDB_OSS_REALLOC( *ppBuffer,
                                             sizeof(CHAR) *newSize ) ;
         if( !*ppBuffer )
         {
            *ppBuffer = pOriginalBuffer ;
            rc = SDB_OOM ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to realloc ppBuffer, rc: %d", rc) ;
         *bufferSize = newSize ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrRemoteAssit::_extract( MsgHeader * msg, INT32 msgSize,
                                       SINT64 * contextID,
                                       BOOLEAN * result, BOOLEAN endianConvert )
   {
      INT32 rc = SDB_OK ;
      INT32 replyFlag = -1 ;
      INT32 numReturned = -1 ;
      INT32 startFrom = -1 ;
      CHAR *pBuffer = ( CHAR* )msg ;

      rc = clientExtractReply ( pBuffer, &replyFlag, contextID,
                                &startFrom, &numReturned, endianConvert ) ;
      if ( SDB_OK != replyFlag )
      {
         *result = FALSE ;
         rc = replyFlag ;
      }
      else
      {
         *result = TRUE ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to extract reply, rc: %d", rc) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrRemoteAssit::_getRetBuffer( CHAR *pRetMsg, CHAR **ppRetBuffer )
   {
      INT32 rc     = SDB_OK ;
      INT32 offset = ossRoundUpToMultipleX( sizeof( MsgOpReply ), 4 ) ;

      MsgOpReply * msgReply = (MsgOpReply*)pRetMsg ;

      if ( NULL == ppRetBuffer )
      {
         rc = SDB_INVALIDARG ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get ppRetBuffer, rc: %d", rc) ;

      // init retBuffer
      if ( offset < msgReply->header.messageLength )
      {
         *ppRetBuffer = &pRetMsg[offset] ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to init ppRetBuffer, rc: %d", rc) ;

   done:
      return rc ;
   error:
      goto done ;
   }

}
