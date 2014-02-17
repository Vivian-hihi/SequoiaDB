#include "client_internal.h"
#include "bson/bson.h"
#include "ossUtil.h"
#include "ossMem.h"
#include "msg.h"
#include "msgDef.h"
#include "network.h"
#include "common.h"
#include "pmdOptions.h"
#include "msgCatalogDef.h"
#include "openssl/md5.h"
#include "fmpDef.hpp"

#define SDB_IS_EMPTY_CHAR(p) \
( (p) == ' ' || \
  (p) == '\t' || \
  (p) == '\n' || \
  (p) == '\r' )

static BOOLEAN _sdbIsSrand = FALSE ;
#if defined (_LINUX)
static UINT32 _sdbRandSeed = 0 ;
#endif
static void _sdbSrand ()
{
   if ( !_sdbIsSrand )
   {
#if defined (_WINDOWS)
      srand ( (UINT32) time ( NULL ) ) ;
#elif defined (_LINuX)
      _sdbRandSeed = time ( NULL ) ;
#endif
      _sdbIsSrand = TRUE ;
   }
}
static UINT32 _sdbRand ()
{
   UINT32 randVal = 0 ;
   if ( !_sdbIsSrand )
      _sdbSrand () ;
#if defined (_WINDOWS)
   rand_s ( &randVal ) ;
#elif defined (_LINUX)
   randVal = rand_r ( &_sdbRandSeed ) ;
#endif
   return randVal ;
}

//#define SDB_CLIENT_DFT_NETWORK_TIMEOUT 1000000
#define SDB_CLIENT_DFT_NETWORK_TIMEOUT -1
static INT32 _setRGName ( sdbShardHandle handle,
                          const CHAR *pShardName )
{
   INT32 rc       = SDB_OK ;
   INT32 len      = 0 ;
   sdbRGStruct *r = (sdbRGStruct*)handle ;
   if ( !r || !pShardName )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   if ( r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( ( len = ossStrlen ( pShardName ) ) > CLIENT_RG_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossMemset ( r->_replicaGroupName, 0, sizeof(r->_replicaGroupName) ) ;
   ossMemcpy ( r->_replicaGroupName, pShardName, len ) ;
   r->_isCatalog = ( ossStrcmp ( pShardName, CAT_CATALOG_GROUPNAME ) == 0 ) ;
done :
   return rc ;
error :
   goto done ;
}

static INT32 _setCSName ( sdbCSHandle handle,
                          const CHAR *pCollectionSpaceName )
{
   INT32 rc       = SDB_OK ;
   INT32 len      = 0 ;
   sdbCSStruct *s = (sdbCSStruct*)handle ;
   if ( !s || !pCollectionSpaceName )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   if ( s->_handleType != SDB_HANDLE_TYPE_CS )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( ( len = ossStrlen ( pCollectionSpaceName) ) > CLIENT_CS_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossMemset ( s->_CSName, 0, sizeof(s->_CSName) ) ;
   ossMemcpy ( s->_CSName, pCollectionSpaceName, len ) ;
done :
   return rc ;
error :
   goto done ;
}
static INT32 _setCollectionName ( sdbCollectionHandle handle,
                                  const CHAR *pCollectionFullName )
{
   INT32 rc                 = SDB_OK ;
   INT32 collectionSpaceLen = 0 ;
   INT32 collectionLen      = 0 ;
   INT32 fullLen            = 0 ;
   CHAR *pDot               = NULL ;
   CHAR *pDot1              = NULL ;
   CHAR collectionFullName [ CLIENT_COLLECTION_NAMESZ +
                             CLIENT_CS_NAMESZ +
                             1 ] ;
   sdbCollectionStruct *s  = (sdbCollectionStruct*)handle ;
   if ( !s || !pCollectionFullName )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   if ( s->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   ossMemset ( s->_CSName, 0, sizeof ( s->_CSName ) );
   ossMemset ( s->_collectionName, 0, sizeof ( s->_collectionName ) ) ;
   ossMemset ( s->_collectionFullName, 0, sizeof ( s->_collectionFullName ) ) ;
   if ( (fullLen = ossStrlen ( pCollectionFullName )) >
        CLIENT_COLLECTION_NAMESZ + CLIENT_CS_NAMESZ + 1 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossMemset ( collectionFullName, 0, sizeof ( collectionFullName ) ) ;
   ossStrncpy ( collectionFullName, pCollectionFullName, fullLen ) ;
   pDot = (CHAR*)ossStrchr ( (CHAR*)collectionFullName, '.' ) ;
   pDot1 = (CHAR*)ossStrrchr ( (CHAR*)collectionFullName, '.' ) ;
   if ( !pDot || (pDot != pDot1) )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   *pDot = 0 ;
   ++pDot ;

   collectionSpaceLen = ossStrlen ( collectionFullName ) ;
   collectionLen      = ossStrlen ( pDot ) ;
   if ( collectionSpaceLen <= CLIENT_CS_NAMESZ &&
        collectionLen <= CLIENT_COLLECTION_NAMESZ )
   {
      ossMemcpy ( s->_CSName, collectionFullName,
                  collectionSpaceLen ) ;
      ossMemcpy ( s->_collectionName, pDot, collectionLen ) ;
      ossMemcpy ( s->_collectionFullName, pCollectionFullName, fullLen ) ;
   }
   else
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}
static INT32 _reallocBuffer ( CHAR **ppBuffer, INT32 *buffersize,
                              INT32 newSize )
{
   INT32 rc = SDB_OK ;
   CHAR *pOriginalBuffer = NULL ;
   if ( !ppBuffer || !buffersize )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   pOriginalBuffer = *ppBuffer ;
   if ( *buffersize < newSize )
   {
      *ppBuffer = (CHAR*)SDB_OSS_REALLOC ( *ppBuffer, sizeof(CHAR)*newSize);
      if ( !*ppBuffer )
      {
         *ppBuffer = pOriginalBuffer ;
         rc = SDB_OOM ;
         goto error ;
      }
      *buffersize = newSize ;
   }
done :
   return rc ;
error :
   goto done ;
}

static INT32 _send1 ( SOCKET sock, const CHAR *pMsg,
                      INT32 len )
{
   INT32 rc = SDB_OK ;
   if ( !pMsg )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientSend ( sock, pMsg, len, SDB_CLIENT_DFT_NETWORK_TIMEOUT ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error :
   goto done ;
}

static INT32 _send ( SOCKET sock, const MsgHeader *msg,
                     BOOLEAN endianConvert )
{
   INT32 rc = SDB_OK ;
   INT32 len = 0 ;
   if ( !msg )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossEndianConvertIf4 ( msg->messageLength, len, endianConvert ) ;
   rc = _send1 ( sock, (const CHAR*)msg, len ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error :
   goto done ;
}

static INT32 _recv ( SOCKET sock, MsgHeader **msg, INT32 *size,
                     BOOLEAN endianConvert )
{
   INT32 rc = SDB_OK ;
   INT32 len = 0 ;
   INT32 realLen = 0 ;
   CHAR **ppBuffer = (CHAR**)msg ;
   while ( TRUE )
   {
      // get length first
      rc = clientRecv ( sock, (CHAR*)&len, sizeof(len),
                        SDB_CLIENT_DFT_NETWORK_TIMEOUT ) ;
      if ( rc == SDB_TIMEOUT )
         continue ;
      if ( rc )
      {
         goto error ;
      }
      break ;
   }
   ossEndianConvertIf4 ( len, realLen, endianConvert ) ;
   rc = _reallocBuffer ( ppBuffer, size, realLen+1 ) ;
   if ( rc )
   {
      goto error ;
   }
   // use the original len before convert
   *(SINT32*)(*ppBuffer) = len ;
   while ( TRUE )
   {
      rc = clientRecv ( sock, &(*ppBuffer)[sizeof(realLen)],
                        realLen - sizeof(realLen),
                        SDB_CLIENT_DFT_NETWORK_TIMEOUT ) ;
      if ( SDB_TIMEOUT == rc )
         continue ;
      if ( rc )
      {
         goto error ;
      }
      break ;
   }
done :
   return rc ;
error :
   if ( SDB_NETWORK_CLOSE == rc )
   {
      clientDisconnect ( sock ) ;
   }
   goto done ;
}

static INT32 _recvExtract ( SOCKET sock, MsgHeader **msg, INT32 *size,
                            SINT64 *contextID, BOOLEAN *result,
                            BOOLEAN endianConvert )
{
   INT32 rc          = SDB_OK ;
   INT32 replyFlag   = -1 ;
   INT32 numReturned = -1 ;
   INT32 startFrom   = -1 ;
   CHAR **ppBuffer   = (CHAR**)msg ;
   if ( !msg || !size || !contextID || !result )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = _recv ( sock, msg, size, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = clientExtractReply ( *ppBuffer, &replyFlag, contextID,
                             &startFrom, &numReturned, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   if ( replyFlag != SDB_OK )
   {
      *result = FALSE ;
      rc = replyFlag ;
      goto done ;
   }

   *result = TRUE ;
done :
   return rc ;
error :
   goto done ;
}

static INT32 _recvExtractEval ( SOCKET sock, MsgHeader **msg, INT32 *size,
                                SINT64 *contextID, SDB_SPD_RES_TYPE *type,
                                BOOLEAN *result, bson *errmsg,
                                BOOLEAN endianConvert )
{
   INT32 rc          = SDB_OK ;
   INT32 replyFlag   = -1 ;
   INT32 startFrom   = -1 ;
   CHAR **ppBuffer   = (CHAR**)msg ;
   MsgOpReply *replyHeader = NULL ;
   if ( !msg || !size || !contextID || !result )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = _recv ( sock, msg, size, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = clientExtractReply ( *ppBuffer, &replyFlag, contextID,
                             &startFrom, (SINT32 *)type, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   if ( replyFlag != SDB_OK )
   {
      *result = FALSE ;
      rc = replyFlag ;
      replyHeader = (MsgOpReply *)(*ppBuffer) ;
      if ( sizeof( MsgOpReply ) != replyHeader->header.messageLength )
      {
         bson_init_finished_data( errmsg, *ppBuffer + sizeof(MsgOpReply) ) ;
      }
      goto done ;
   }

   *result = TRUE ;
done :
   return rc ;
error :
   goto done ;
}

static void _killCursor ( SOCKET sock, CHAR **ppSendBuffer,
                          INT32 *sendBufferSize, CHAR **ppReceiveBuffer,
                          INT32 *receiveBufferSize, SINT64 contextID,
                          BOOLEAN endianConvert )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result = FALSE ;
   SINT64 lcontextID = contextID ;
   rc = clientBuildKillContextsMsg ( ppSendBuffer, sendBufferSize, 0, 1,
                                     &lcontextID, endianConvert ) ;
   if ( rc )
   {
      return ;
   }
   rc = _send ( sock, (MsgHeader*)(*ppSendBuffer), endianConvert ) ;
   if ( rc )
   {
      return ;
   }
   rc = _recvExtract ( sock, (MsgHeader**)ppReceiveBuffer,
                       receiveBufferSize, &lcontextID,
                       &result, endianConvert ) ;
   if ( rc )
   {
      return ;
   }
   return ;
}

static INT32 _readNextBuffer ( SOCKET sock, CHAR **ppSendBuffer,
                               INT32 *sendBufferSize, CHAR **ppReceiveBuffer,
                               INT32 *receiveBufferSize, SINT64 contextID,
                               BOOLEAN endianConvert )
{
   INT32 rc = SDB_OK ;
   BOOLEAN lresult = FALSE ;
   SINT64 lcontextID = 0 ;
   rc = clientBuildGetMoreMsg ( ppSendBuffer, sendBufferSize, -1, contextID,
                                0, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( sock, (MsgHeader*)(*ppSendBuffer), endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( sock, (MsgHeader**)ppReceiveBuffer,
                       receiveBufferSize, &lcontextID,
                       &lresult, endianConvert ) ;
   if ( rc || lcontextID != contextID )
   {
      goto error ;
   }
done :
   return rc ;
error :
   if ( SDB_DMS_EOC != rc )
   {
      _killCursor ( sock, ppSendBuffer, sendBufferSize,
                    ppReceiveBuffer, receiveBufferSize, contextID,
                    endianConvert ) ;
   }
   goto done ;
}


static INT32 _runCommand ( SOCKET sock, CHAR **ppSendBuffer,
                           INT32 *sendBufferSize, CHAR **ppReceiveBuffer,
                           INT32 *receiveBufferSize, BOOLEAN endianConvert,
                           const CHAR *pString, BOOLEAN *result, bson *arg1,
                           bson *arg2, bson *arg3, bson *arg4 )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID = 0 ;
   rc = clientBuildQueryMsg ( ppSendBuffer,
                              sendBufferSize,
                              pString, 0, 0, -1, -1,
                              arg1, arg2, arg3, arg4, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( sock, (MsgHeader*)(*ppSendBuffer), endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( sock,
                       (MsgHeader**)ppReceiveBuffer,
                       receiveBufferSize,
                       &contextID, result, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

static INT32 requestSysInfo ( SOCKET sock, BOOLEAN *endianConvert )
{
   INT32 rc = SDB_OK ;
   MsgSysInfoRequest request ;
   MsgSysInfoReply reply ;
   MsgSysInfoRequest *prq = &request ;
   INT32 requestSize = sizeof(request) ;
   if ( endianConvert )
      *endianConvert = FALSE ;
   rc = clientBuildSysInfoRequest ( (CHAR**)&prq, &requestSize ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send1 ( sock, (const CHAR*)prq, requestSize ) ;
   if ( rc )
   {
      goto error ;
   }
   while ( TRUE )
   {
      rc = clientRecv ( sock, (CHAR*)&reply, sizeof(MsgSysInfoReply),
                        SDB_CLIENT_DFT_NETWORK_TIMEOUT ) ;
      if ( SDB_TIMEOUT == rc )
         continue ;
      if ( rc )
      {
         goto error ;
      }
      break ;
   }
   rc = clientExtractSysInfoReply ( (CHAR*)&reply, endianConvert, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbConnect ( const CHAR *pHostName, const CHAR *pServiceName,
                              const CHAR *pUsrName, const CHAR *pPasswd ,
                              sdbConnectionHandle *handle )
{
   INT32 rc = SDB_OK ;
   //for the encryted password
   CHAR md5[MD5_DIGEST_LENGTH*2+1] ;
   BOOLEAN r ;
   SINT64 contextID = 0 ;
   sdbConnectionStruct *connection = NULL ;
   BOOLEAN endianConvert = FALSE ;
   if ( !handle || !pHostName || !pServiceName ||!pUsrName ||!pPasswd )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   connection = (sdbConnectionStruct*)SDB_OSS_MALLOC ( sizeof (
                sdbConnectionStruct ) ) ;
   if ( !connection )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( connection, 0, sizeof(sdbConnectionStruct) ) ;
   connection->_handleType = SDB_HANDLE_TYPE_CONNECTION ;

   rc = clientConnect ( pHostName, pServiceName, &connection->_sock ) ;
   if ( rc )
   {
      goto error ;
   }
   *handle = (sdbConnectionHandle)connection ;

   // request system information
   rc = requestSysInfo ( connection->_sock, &endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   connection->_endianConvert = endianConvert ;

   //encryt the password
   //do we need to take care of endianess?
   rc = md5Encrypt( pPasswd, md5, MD5_DIGEST_LENGTH*2+1) ;
   if ( rc )
   {
      goto error ;
   }
   //build checking message
   rc = clientBuildAuthMsg( &connection->_pSendBuffer,
                            &connection->_sendBufferSize,
                            pUsrName, md5, 0, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   //send to engine
   rc = _send (connection->_sock, (MsgHeader*)connection->_pSendBuffer,
               endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize,
                       &contextID, &r,
                       endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT void sdbDisconnect ( sdbConnectionHandle handle )
{
   sdbConnectionStruct *connection = (sdbConnectionStruct*)handle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      return ;
   }
   if ( !clientBuildDisconnectMsg ( &connection->_pSendBuffer,
                                    &connection->_sendBufferSize, 0, connection->_endianConvert ))
   {
      _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
              connection->_endianConvert ) ;
   }
   clientDisconnect ( connection->_sock ) ;
}

SDB_EXPORT INT32 sdbGetDataBlocks ( sdbCollectionHandle cHandle,
                            bson *condition,
                            bson *select,
                            bson *orderBy,
                            bson *hint,
                            INT64 numToSkip,
                            INT64 numToReturn,
                            sdbCursorHandle *handle )
{
   INT32 rc                        = SDB_OK ;
   CHAR *p                         = NULL ;
   sdbCursorStruct *cursor         = NULL ;
   SINT64 contextID                = 0 ;
   BOOLEAN result                  = FALSE ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs ||
         cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == 0 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   p = CMD_ADMIN_PREFIX CMD_NAME_GET_DATABLOCKS ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              p, 0, 0,
                              numToSkip, numToReturn, condition,
                              select, orderBy, hint, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, ( MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock,
                       (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock = cs->_sock ;
   cursor->_contextID = contextID ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset = -1 ;
   cursor->_endianConvert = cs->_endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbGetQueryMeta ( sdbCollectionHandle cHandle,
                                   bson *condition,
                                   bson *orderBy,
                                   bson *hint,
                                   INT64 numToSkip,
                                   INT64 numToReturn,
                                   sdbCursorHandle *handle )
{
   INT32 rc                        = SDB_OK ;
   CHAR *p                         = NULL ;
   sdbCursorStruct *cursor         = NULL ;
   SINT64 contextID                = 0 ;
   BOOLEAN result                  = FALSE ;
   bson hint1 ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   bson_init ( &hint1 ) ;
   bson_append_string ( &hint1, FIELD_NAME_COLLECTION, cs->_collectionFullName ) ;
   rc = bson_finish ( &hint1 ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   p = CMD_ADMIN_PREFIX CMD_NAME_GET_QUERYMETA ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer,
                              &cs->_sendBufferSize,
                              p, 0, 0, numToSkip, numToReturn, condition,
                              hint, orderBy, &hint1,
                              cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, ( MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock,
                       (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock = cs->_sock ;
   cursor->_contextID = contextID ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset = -1 ;
   cursor->_endianConvert = cs->_endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done:
   bson_destroy ( &hint1 ) ;
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbGetSnapshot ( sdbConnectionHandle cHandle,
                                  INT32 snapType,
                                  bson *condition,
                                  bson *selector,
                                  bson *orderBy,
                                  sdbCursorHandle *handle )
{
   INT32 rc                        = SDB_OK ;
   CHAR *p                         = NULL ;
   sdbCursorStruct *cursor         = NULL ;
   SINT64 contextID                = -1 ;
   BOOLEAN result                  = FALSE ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   switch ( snapType )
   {
   case SDB_SNAP_CONTEXTS :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_CONTEXTS ;
      break ;
   case SDB_SNAP_CONTEXTS_CURRENT :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_CONTEXTS_CURRENT ;
      break ;
   case SDB_SNAP_SESSIONS :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_SESSIONS ;
      break ;
   case SDB_SNAP_SESSIONS_CURRENT :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_SESSIONS_CURRENT ;
      break ;
   case SDB_SNAP_COLLECTIONS :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_COLLECTIONS ;
      break ;
   case SDB_SNAP_COLLECTIONSPACES :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_COLLECTIONSPACES ;
      break ;
   case SDB_SNAP_DATABASE :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_DATABASE ;
      break ;
   case SDB_SNAP_SYSTEM :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_SYSTEM ;
      break ;
   case SDB_SNAP_CATALOG :
      p = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_CATA ;
      break;
   default :
      return SDB_INVALIDARG ;
   }
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer,
                              &connection->_sendBufferSize,
                              p, 0, 0, 0, -1, condition, selector, orderBy,
                              NULL, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,
                       (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock = connection->_sock ;
   cursor->_contextID = contextID ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset = -1 ;
   cursor->_endianConvert = connection->_endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbCreateUsr( sdbConnectionHandle cHandle, const CHAR *pUsrName,
                               const CHAR *pPasswd )
{
   INT32 rc = SDB_OK ;
   CHAR md5[MD5_DIGEST_LENGTH*2+1] ;
   BOOLEAN r ;
   SINT64 contextID = 0 ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( NULL == pUsrName ||NULL == pPasswd )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = md5Encrypt( pPasswd, md5, MD5_DIGEST_LENGTH*2+1) ;
   if ( rc )
   {
      goto error ;
   }
   rc = clientBuildAuthCrtMsg( &connection->_pSendBuffer,
                               &connection->_sendBufferSize,
                               pUsrName, md5, 0, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,
                      (MsgHeader**)&connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize, &contextID, &r,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
  return rc ;
error:
  goto done ;
}

SDB_EXPORT INT32 sdbRemoveUsr( sdbConnectionHandle cHandle, const CHAR *pUsrName,
                               const CHAR *pPasswd )
{
   INT32 rc = SDB_OK ;
   CHAR md5[MD5_DIGEST_LENGTH*2+1] ;
   BOOLEAN r ;
   SINT64 contextID = 0 ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( NULL == pUsrName ||NULL == pPasswd )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = md5Encrypt( pPasswd, md5, MD5_DIGEST_LENGTH*2+1) ;
   if ( rc )
   {
      goto error ;
   }
   rc = clientBuildAuthDelMsg( &connection->_pSendBuffer,
                               &connection->_sendBufferSize,
                               pUsrName, md5, 0, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,
                      (MsgHeader**)&connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize, &contextID, &r,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
  return rc ;
error:
  goto done ;
}

SDB_EXPORT INT32 sdbResetSnapshot ( sdbConnectionHandle cHandle,
                                    bson *condition )
{
   INT32 rc                = SDB_OK ;
   BOOLEAN result          = FALSE ;
   CHAR *p                 = CMD_ADMIN_PREFIX CMD_NAME_SNAPSHOT_RESET ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      p, &result, condition,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

static INT32 _sdbGetList ( SOCKET _sock, CHAR **_pSendBuffer,
                           INT32 *_sendBufferSize, CHAR **_pReceiveBuffer,
                           INT32 *_receiveBufferSize, BOOLEAN _endianConvert,
                           INT32 listType,
                           bson *condition, bson *selector, bson *orderBy,
                           sdbCursorHandle *handle )
{
   INT32 rc                        = SDB_OK ;
   CHAR *p                         = NULL ;
   sdbCursorStruct *cursor         = NULL ;
   SINT64 contextID                = -1 ;
   BOOLEAN result                  = FALSE ;
   switch ( listType )
   {
   case SDB_LIST_CONTEXTS :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_CONTEXTS ;
      break ;
   case SDB_LIST_CONTEXTS_CURRENT :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_CONTEXTS_CURRENT ;
      break ;
   case SDB_LIST_SESSIONS :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_SESSIONS ;
      break ;
   case SDB_LIST_SESSIONS_CURRENT :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_SESSIONS_CURRENT ;
      break ;
   case SDB_LIST_COLLECTIONS :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_COLLECTIONS ;
      break ;
   case SDB_LIST_COLLECTIONSPACES :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_COLLECTIONSPACES ;
      break ;
   case SDB_LIST_STORAGEUNITS :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_STORAGEUNITS ;
      break ;
   case SDB_LIST_SHARDS :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_GROUPS ;
      break ;
   case SDB_LIST_STOREPROCEDURES :
      p = CMD_ADMIN_PREFIX CMD_NAME_LIST_PROCEDURES ;
      break ;
   default :
      return SDB_INVALIDARG ;
   }
   rc = clientBuildQueryMsg ( _pSendBuffer,
                              _sendBufferSize,
                              p, 0, 0, 0, -1, condition, selector, orderBy,
                              NULL, _endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( _sock, (MsgHeader*)(*_pSendBuffer), _endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( _sock,
                       (MsgHeader**)_pReceiveBuffer,
                       _receiveBufferSize, &contextID, &result,
                       _endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock = _sock ;
   cursor->_contextID = contextID ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset = -1 ;
   cursor->_endianConvert = _endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetList ( sdbConnectionHandle cHandle,
                              INT32 listType,
                              bson *condition,
                              bson *selector,
                              bson *orderBy,
                              sdbCursorHandle *handle )
{
   INT32 rc                        = SDB_OK ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = _sdbGetList ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      listType,
                      condition, selector, orderBy, handle ) ;
   if ( rc )
   {
      goto done ;
   }
done :
   return rc ;
error :
   goto done ;
}


SDB_EXPORT INT32 sdbGetCollection ( sdbConnectionHandle cHandle,
                                   const CHAR *pCollectionFullName,
                                   sdbCollectionHandle *handle )
{
   INT32 rc               = SDB_OK ;
   BOOLEAN result         = FALSE ;
   INT32 nameLength       = 0 ;
   bson newObj ;
   CHAR *pTestCollection  = CMD_ADMIN_PREFIX CMD_NAME_TEST_COLLECTION ;
   CHAR *pName            = FIELD_NAME_NAME ;
   sdbCollectionStruct *s = NULL ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pCollectionFullName || !handle ||
        (nameLength = ossStrlen ( pCollectionFullName) ) >
        CLIENT_CS_NAMESZ + CLIENT_COLLECTION_NAMESZ +1 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, pName, pCollectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      pTestCollection, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
   s = (sdbCollectionStruct*) SDB_OSS_MALLOC ( sizeof( sdbCollectionStruct) ) ;
   if ( !s )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( s, 0, sizeof( sdbCollectionStruct) ) ;
   s->_handleType    = SDB_HANDLE_TYPE_COLLECTION ;
   s->_sock          = connection->_sock ;
   s->_endianConvert = connection->_endianConvert ;
   rc = _setCollectionName ( (sdbCollectionHandle)s, pCollectionFullName ) ;
   if ( rc )
   {
      SDB_OSS_FREE ( s ) ;
      goto error ;
   }
   *handle = (sdbCollectionHandle)s ;
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetCollectionSpace ( sdbConnectionHandle cHandle,
                                         const CHAR *pCollectionSpaceName,
                                         sdbCSHandle *handle )
{
   INT32 rc               = SDB_OK ;
   BOOLEAN result         = FALSE ;
   INT32 nameLength       = 0 ;
   bson newObj ;
   CHAR *pTestCollection  = CMD_ADMIN_PREFIX CMD_NAME_TEST_COLLECTIONSPACE ;
   CHAR *pName            = FIELD_NAME_NAME ;
   sdbCSStruct *s         = NULL ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pCollectionSpaceName || !handle ||
        (nameLength = ossStrlen ( pCollectionSpaceName) ) >
        CLIENT_CS_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, pName, pCollectionSpaceName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;

   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      pTestCollection, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
   s = (sdbCSStruct*) SDB_OSS_MALLOC ( sizeof( sdbCSStruct) ) ;
   if ( !s )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( s, 0, sizeof( sdbCSStruct) ) ;
   s->_handleType    = SDB_HANDLE_TYPE_CS ;
   s->_sock          = connection->_sock ;
   s->_endianConvert = connection->_endianConvert ;
   rc = _setCSName ( (sdbCSHandle)s, pCollectionSpaceName ) ;
   if ( rc )
   {
      SDB_OSS_FREE ( s ) ;
      goto error ;
   }
   *handle = (sdbCSHandle)s ;
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetShard ( sdbConnectionHandle cHandle,
                                      const CHAR *pShardName,
                                      sdbShardHandle *handle )
{
   INT32 rc                 = SDB_OK ;
   INT32 nameLength         = 0 ;
   sdbCursorHandle   cursor = SDB_INVALID_HANDLE ;
   bson newObj ;
   bson result ;
   CHAR *pName              = CAT_GROUPNAME_NAME ;
   sdbRGStruct *r           = NULL ;
   BOOLEAN found            = FALSE ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;
   bson_init ( &result ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pShardName || !handle ||
        (nameLength = ossStrlen ( pShardName ) ) >
        CLIENT_RG_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   rc = bson_append_string ( &newObj, pName, pShardName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = sdbGetList ( cHandle, SDB_LIST_SHARDS, &newObj, NULL, NULL,
                     &cursor ) ;
   if ( rc )
   {
      goto error ;
   }
   if ( SDB_OK == ( rc = sdbNext ( cursor, &result ) ) )
   {
      r = (sdbRGStruct*) SDB_OSS_MALLOC ( sizeof( sdbRGStruct ) ) ;
      if ( !r )
      {
         rc = SDB_OOM ;
         goto error ;
      }
      ossMemset ( r, 0, sizeof( sdbRGStruct ) ) ;
      r->_handleType    = SDB_HANDLE_TYPE_REPLICAGROUP ;
      r->_sock          = connection->_sock ;
      r->_endianConvert = connection->_endianConvert ;
      rc = _setRGName ( (sdbShardHandle)r, pShardName ) ;
      if ( rc )
      {
         SDB_OSS_FREE ( r ) ;
         goto error ;
      }
      if ( ossStrcmp ( pShardName, CATALOG_GROUPNAME ) == 0 )
      {
         r->_isCatalog = TRUE ;
      }
      *handle = (sdbShardHandle)r ;
      found = TRUE ;
   }
   else if ( SDB_DMS_EOC != rc )
   {
      goto error ;
   }
   if ( !found )
   {
      rc = SDB_CLS_GRP_NOT_EXIST ;
      goto error ;
   }
done :
   if ( SDB_INVALID_HANDLE != cursor )
   {
      sdbReleaseCursor ( cursor ) ;
   }
   bson_destroy ( &newObj ) ;
   bson_destroy ( &result ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetShard1 ( sdbConnectionHandle cHandle,
                                       UINT32 id,
                                       sdbShardHandle *handle )
{
   INT32 rc                 = SDB_OK ;
   sdbCursorHandle   cursor = SDB_INVALID_HANDLE ;
   bson newObj ;
   bson result ;
   CHAR *pName              = CAT_GROUPID_NAME ;
   sdbRGStruct *r           = NULL ;
   BOOLEAN found            = FALSE ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;
   bson_init ( &result ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !handle )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   rc = bson_append_int ( &newObj, pName, (INT32)id ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = sdbGetList ( cHandle, SDB_LIST_SHARDS, &newObj, NULL, NULL,
                     &cursor ) ;
   if ( rc )
   {
      goto error ;
   }
   if ( SDB_OK == ( rc = sdbNext ( cursor, &result ) ) )
   {
      bson_iterator it ;
      const CHAR *pShardName = NULL ;
      r = (sdbRGStruct*) SDB_OSS_MALLOC ( sizeof( sdbRGStruct ) ) ;
      if ( !r )
      {
         rc = SDB_OOM ;
         goto error ;
      }
      if ( BSON_STRING != bson_find ( &it, &result, CAT_GROUPNAME_NAME ) )
      {
         rc = SDB_SYS ;
         goto error ;
      }
      pShardName = bson_iterator_string ( &it ) ;
      ossMemset ( r, 0, sizeof( sdbRGStruct ) ) ;
      r->_handleType    = SDB_HANDLE_TYPE_REPLICAGROUP ;
      r->_sock          = connection->_sock ;
      r->_endianConvert = connection->_endianConvert ;
      rc = _setRGName ( (sdbShardHandle)r, pShardName ) ;
      if ( rc )
      {
         SDB_OSS_FREE ( r ) ;
         goto error ;
      }
      if ( ossStrcmp ( pShardName, CATALOG_GROUPNAME ) == 0 )
      {
         r->_isCatalog = TRUE ;
      }
      *handle = (sdbShardHandle)r ;
      found = TRUE ;
   }
   else if ( SDB_DMS_EOC != rc )
   {
      goto error ;
   }
   if ( !found )
   {
      rc = SDB_CLS_GRP_NOT_EXIST ;
      goto error ;
   }
done :
   if ( SDB_INVALID_HANDLE != cursor )
   {
      sdbReleaseCursor ( cursor ) ;
   }
   bson_destroy ( &newObj ) ;
   bson_destroy ( &result ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetShardName ( sdbShardHandle cHandle,
                                          CHAR **ppShardName )
{
   INT32 rc                 = SDB_OK ;
   sdbRGStruct *r           = (sdbRGStruct*)cHandle ;
   if ( !r ||
        r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( ppShardName )
   {
      *ppShardName = r->_replicaGroupName ;
   }
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT BOOLEAN sdbIsShardCatalog ( sdbShardHandle cHandle )
{
   sdbRGStruct *r = (sdbRGStruct*)cHandle ;
   if ( !r ||
        r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      return FALSE ;
   }
   return r->_isCatalog ;
}

SDB_EXPORT INT32 sdbCreateCataShard ( sdbConnectionHandle cHandle,
                                        const CHAR *pHostName,
                                        const CHAR *pServiceName,
                                        const CHAR *pDatabasePath,
                                        bson *configure )
{
   INT32 rc = SDB_OK ;
   bson configuration ;
   BOOLEAN result = FALSE ;
   CHAR *pCataShard = CMD_ADMIN_PREFIX CMD_NAME_CREATE_CATA_GROUP ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &configuration ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pHostName || !pServiceName || !pDatabasePath )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   // HostName is required
   rc = bson_append_string ( &configuration,
                             CAT_HOST_FIELD_NAME, pHostName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // ServiceName is required
   rc = bson_append_string ( &configuration,
                             PMD_OPTION_SVCNAME, pServiceName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // database path is required
   rc = bson_append_string ( &configuration,
                             PMD_OPTION_DBPATH, pDatabasePath ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // append all other parameters
   if ( configure )
   {
      bson_iterator it ;
      bson_iterator_init ( &it, configure ) ;
      while ( BSON_EOO != bson_iterator_next ( &it ) )
      {
         const CHAR *key = bson_iterator_key ( &it ) ;
         if ( ossStrcmp ( key, PMD_OPTION_DBPATH ) == 0 ||
              ossStrcmp ( key, PMD_OPTION_SVCNAME ) == 0  ||
              ossStrcmp ( key, CAT_HOST_FIELD_NAME ) == 0 )
         {
            // skip the ones we already created
            continue ;
         }
         switch ( (signed int)bson_iterator_type ( &it ) )
         {
         case BSON_INT :
         {
            CHAR temp[32] = {0} ;
            ossSnprintf ( temp, sizeof(temp),
                          "%d", bson_iterator_int ( &it ) ) ;
            rc = bson_append_string ( &configuration,
                                      key, temp ) ;
            break ;
         }
         case BSON_LONG :
         {
            CHAR temp[32] = {0} ;
            ossSnprintf ( temp, sizeof(temp),
                          "%lld", (INT64)bson_iterator_long ( &it ) ) ;
            rc = bson_append_string ( &configuration,
                                      key, temp ) ;
            break ;
         }
         case BSON_STRING :
            rc = bson_append_string ( &configuration, key,
                                      bson_iterator_string ( &it ) ) ;
            break ;
         case BSON_DOUBLE :
         {
            CHAR temp[64] = {0} ;
            ossSnprintf ( temp, sizeof(temp),
                          "%f", bson_iterator_double ( &it ) ) ;
            rc = bson_append_string ( &configuration,
                                      key, temp ) ;
            break ;
         }
         default :
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         if ( rc )
         {
            rc = SDB_SYS ;
            goto error ;
         }
      } // while
   } // if ( configure )
   bson_finish ( &configuration ) ;

   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      pCataShard, &result, &configuration,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &configuration ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbCreateNode ( sdbShardHandle cHandle,
                                 const CHAR *pHostName,
                                 const CHAR *pServiceName,
                                 const CHAR *pDatabasePath,
                                 bson *configure )
{
   INT32 rc = SDB_OK ;
   bson configuration ;
   BOOLEAN result = FALSE ;
   CHAR *pCreateNode = CMD_ADMIN_PREFIX CMD_NAME_CREATE_NODE ;
   sdbRGStruct *r = (sdbRGStruct*)cHandle ;
   bson_init ( &configuration ) ;

   if ( !r ||
        r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pHostName || !pServiceName || !pDatabasePath )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   // GroupName is required
   rc = bson_append_string ( &configuration,
                             CAT_GROUPNAME_NAME, r->_replicaGroupName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // HostName is required
   rc = bson_append_string ( &configuration,
                             CAT_HOST_FIELD_NAME, pHostName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // ServiceName is required
   rc = bson_append_string ( &configuration,
                             PMD_OPTION_SVCNAME, pServiceName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // database path is required
   rc = bson_append_string ( &configuration,
                             PMD_OPTION_DBPATH, pDatabasePath ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // append all other parameters
   if ( configure )
   {
      bson_iterator it ;
      bson_iterator_init ( &it, configure ) ;
      while ( BSON_EOO != bson_iterator_next ( &it ) )
      {
         const CHAR *key = bson_iterator_key ( &it ) ;
         if ( ossStrcmp ( key, PMD_OPTION_DBPATH ) == 0 ||
              ossStrcmp ( key, PMD_OPTION_SVCNAME ) == 0  ||
              ossStrcmp ( key, CAT_HOST_FIELD_NAME ) == 0 ||
              ossStrcmp ( key, CAT_GROUPNAME_NAME ) == 0 )
         {
            // skip the ones we already created
            continue ;
         }
         switch ( (signed int)bson_iterator_type ( &it ) )
         {
         case BSON_INT :
         {
            CHAR temp[32] = {0} ;
            ossSnprintf ( temp, sizeof(temp),
                          "%d", bson_iterator_int ( &it ) ) ;
            rc = bson_append_string ( &configuration,
                                      key, temp ) ;
            break ;
         }
         case BSON_LONG :
         {
            CHAR temp[32] = {0} ;
            ossSnprintf ( temp, sizeof(temp),
                          "%lld", (INT64)bson_iterator_long ( &it ) ) ;
            rc = bson_append_string ( &configuration,
                                      key, temp ) ;
            break ;
         }
         case BSON_STRING :
            rc = bson_append_string ( &configuration, key,
                                      bson_iterator_string ( &it ) ) ;
            break ;
         case BSON_DOUBLE :
         {
            CHAR temp[64] = {0} ;
            ossSnprintf ( temp, sizeof(temp),
                          "%f", bson_iterator_double ( &it ) ) ;
            rc = bson_append_string ( &configuration,
                                      key, temp ) ;
            break ;
         }
         default :
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         if ( rc )
         {
            rc = SDB_SYS ;
            goto error ;
         }
      } // while
   } // if ( configure )
   bson_finish ( &configuration ) ;

   rc = _runCommand ( r->_sock, &r->_pSendBuffer,
                      &r->_sendBufferSize,
                      &r->_pReceiveBuffer,
                      &r->_receiveBufferSize,
                      r->_endianConvert,
                      pCreateNode, &result, &configuration,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &configuration ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbRemoveNode ( sdbShardHandle cHandle,
                                 const CHAR *pHostName,
                                 const CHAR *pServiceName,
                                 bson *configure )
{
   INT32 rc = SDB_OK ;
   bson removeInfo ;
   CHAR *pRemoveNode = CMD_ADMIN_PREFIX CMD_NAME_REMOVE_NODE ;
   sdbRGStruct *r = (sdbRGStruct*)cHandle ;
   BOOLEAN result = FALSE ;
   bson_init( &removeInfo ) ;

   if ( !r ||
        r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pHostName || !pServiceName )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   // GroupName is required
   rc = bson_append_string ( &removeInfo,
                             CAT_GROUPNAME_NAME, r->_replicaGroupName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // HostName is required
   rc = bson_append_string ( &removeInfo,
                             FIELD_NAME_HOST, pHostName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   // ServiceName is required
   rc = bson_append_string ( &removeInfo,
                             PMD_OPTION_SVCNAME, pServiceName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   if ( configure )
   {
      bson_iterator it ;
      bson_iterator_init ( &it, configure ) ;
      while ( BSON_EOO != bson_iterator_next ( &it ) )
      {
         const CHAR *key = bson_iterator_key ( &it ) ;
         if ( ossStrcmp ( key, FIELD_NAME_HOST ) == 0  ||
              ossStrcmp ( key, FIELD_NAME_SERVICE_NAME ) == 0 ||
              ossStrcmp ( key, CAT_GROUPNAME_NAME ) == 0 )
         {
            // skip the ones we already created
            continue ;
         }
         else
         {
            rc = bson_append_element( &removeInfo,
                                      NULL, &it ) ;
            if ( rc )
            {
               rc = SDB_SYS ;
               goto error ;
            }
         }
      }
   }

   bson_finish( &removeInfo ) ;

   rc = _runCommand ( r->_sock, &r->_pSendBuffer,
                      &r->_sendBufferSize,
                      &r->_pReceiveBuffer,
                      &r->_receiveBufferSize,
                      r->_endianConvert,
                      pRemoveNode, &result, &removeInfo,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   bson_destroy( &removeInfo ) ;
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbCreateCollectionSpace ( sdbConnectionHandle cHandle,
                                            const CHAR *pCollectionSpaceName,
                                            INT32 iPageSize,
                                            sdbCSHandle *handle )
{
   INT32 rc               = SDB_OK ;
   BOOLEAN result         = FALSE ;
   INT32 nameLength       = 0 ;
   bson newObj ;
   CHAR *pCreateCollection  = CMD_ADMIN_PREFIX CMD_NAME_CREATE_COLLECTIONSPACE ;
   CHAR *pName              = FIELD_NAME_NAME ;
   CHAR *pPageSize          = FIELD_NAME_PAGE_SIZE ;
   sdbCSStruct *s           = NULL ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pCollectionSpaceName || !handle ||
        (nameLength = ossStrlen ( pCollectionSpaceName) ) >
        CLIENT_CS_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   if ( iPageSize != SDB_PAGESIZE_4K &&
        iPageSize != SDB_PAGESIZE_8K &&
        iPageSize != SDB_PAGESIZE_16K &&
        iPageSize != SDB_PAGESIZE_32K &&
        iPageSize != SDB_PAGESIZE_64K )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, pName, pCollectionSpaceName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_int ( &newObj, pPageSize, iPageSize ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   bson_finish ( &newObj ) ;
   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      pCreateCollection, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
   s = (sdbCSStruct*) SDB_OSS_MALLOC ( sizeof( sdbCSStruct) ) ;
   if ( !s )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( s, 0, sizeof( sdbCSStruct) ) ;
   s->_handleType    = SDB_HANDLE_TYPE_CS ;
   s->_sock          = connection->_sock ;
   s->_endianConvert = connection->_endianConvert ;
   rc = _setCSName ( (sdbCSHandle)s, pCollectionSpaceName ) ;
   if ( rc )
   {
      SDB_OSS_FREE ( s ) ;
      goto error ;
   }
   *handle = (sdbCSHandle)s ;
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbDropCollectionSpace ( sdbConnectionHandle cHandle,
                                         const CHAR *pCollectionSpaceName )
{
   INT32 rc               = SDB_OK ;
   BOOLEAN result         = FALSE ;
   INT32 nameLength       = 0 ;
   bson newObj ;
   CHAR *pDropCollection  = CMD_ADMIN_PREFIX CMD_NAME_DROP_COLLECTIONSPACE ;
   CHAR *pName            = FIELD_NAME_NAME ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pCollectionSpaceName ||
        (nameLength = ossStrlen ( pCollectionSpaceName) ) >
        CLIENT_CS_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, pName, pCollectionSpaceName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      pDropCollection, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbCreateShard ( sdbConnectionHandle cHandle,
                                         const CHAR *pShardName,
                                         sdbShardHandle *handle )
{
   INT32 rc         = SDB_OK ;
   BOOLEAN result   = FALSE ;
   INT32 nameLength = 0 ;
   bson newObj ;
   CHAR *pCreateShard = CMD_ADMIN_PREFIX CMD_NAME_CREATE_GROUP ;
   CHAR *pName               = FIELD_NAME_GROUPNAME ;
   sdbRGStruct *r            = NULL ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pShardName || !handle ||
        (nameLength = ossStrlen ( pShardName ) ) >
        CLIENT_RG_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, pName, pShardName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;

   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      pCreateShard, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
   r = (sdbRGStruct*) SDB_OSS_MALLOC ( sizeof( sdbRGStruct ) ) ;
   if ( !r )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( r, 0, sizeof( sdbRGStruct ) ) ;
   r->_handleType    = SDB_HANDLE_TYPE_REPLICAGROUP ;
   r->_sock          = connection->_sock ;
   r->_endianConvert = connection->_endianConvert ;
   rc = _setRGName ( (sdbShardHandle)r, pShardName ) ;
   if ( rc )
   {
      SDB_OSS_FREE ( r ) ;
      goto error ;
   }
   *handle = (sdbShardHandle)r ;
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbRemoveShard ( sdbConnectionHandle cHandle,
                                         const CHAR *pShardName )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result   = FALSE ;
   INT32 nameLength = 0 ;
   bson newObj ;
   CHAR *pCommand = CMD_ADMIN_PREFIX CMD_NAME_REMOVE_GROUP ;
   CHAR *pName = FIELD_NAME_GROUPNAME ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( !pShardName )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   nameLength = ossStrlen( pShardName ) ;
   if ( 0 == nameLength || CLIENT_RG_NAMESZ < nameLength )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   rc = bson_append_string ( &newObj, pName, pShardName ) ;
      if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;

   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      pCommand, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   bson_destroy ( &newObj ) ;
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbStartShard ( sdbShardHandle cHandle )
{
   INT32 rc         = SDB_OK ;
   BOOLEAN result   = FALSE ;
   bson newObj ;
   CHAR *pActivateShard = CMD_ADMIN_PREFIX CMD_NAME_ACTIVE_GROUP ;
   CHAR *pName                 = FIELD_NAME_GROUPNAME ;
   sdbRGStruct *r              = (sdbRGStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   rc = bson_append_string ( &newObj, pName, r->_replicaGroupName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;

   rc = _runCommand ( r->_sock, &r->_pSendBuffer,
                      &r->_sendBufferSize,
                      &r->_pReceiveBuffer,
                      &r->_receiveBufferSize,
                      r->_endianConvert,
                      pActivateShard, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbStopShard ( sdbShardHandle cHandle )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result = FALSE ;
   bson configuration ;
   sdbRGStruct *r              = (sdbRGStruct*)cHandle ;
   bson_init ( &configuration ) ;

   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = bson_append_string ( &configuration,
                             FIELD_NAME_GROUPNAME, r->_replicaGroupName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &configuration ) ;
   rc = _runCommand ( r->_sock, &r->_pSendBuffer,
                      &r->_sendBufferSize,
                      &r->_pReceiveBuffer,
                      &r->_receiveBufferSize,
                      r->_endianConvert,
                      CMD_ADMIN_PREFIX CMD_NAME_SHUTDOWN_GROUP,
                      &result, &configuration,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &configuration ) ;
   return rc ;
error :
   goto done ;
}

static INT32 _sdbGetShardDetail ( sdbShardHandle cHandle,
                                         bson *result )
{
   INT32 rc               = SDB_OK ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   bson newObj ;
   CHAR *pName            = FIELD_NAME_GROUPNAME ;
   sdbRGStruct *r         = (sdbRGStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, pName, r->_replicaGroupName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = _sdbGetList ( r->_sock, &r->_pSendBuffer, &r->_sendBufferSize,
                      &r->_pReceiveBuffer, &r->_receiveBufferSize,
                      r->_endianConvert,
                      SDB_LIST_SHARDS, &newObj, NULL, NULL, &cursor ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = sdbNext ( cursor, result ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   if ( SDB_INVALID_HANDLE != cursor )
   {
      sdbReleaseCursor ( cursor ) ;
   }
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

static INT32 _sdbShardExtractNode ( SOCKET sock,
                                    sdbNodeHandle *handle,
                                    const CHAR *data,
                                    BOOLEAN endianConvert )
{
   INT32 rc = SDB_OK ;
   sdbRNStruct *r = NULL ;

   r = (sdbRNStruct*) SDB_OSS_MALLOC ( sizeof( sdbRNStruct ) ) ;
   if ( !r )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( r, 0, sizeof( sdbRNStruct ) ) ;
   r->_handleType = SDB_HANDLE_TYPE_REPLICANODE ;
   r->_sock = sock ;
   r->_endianConvert = endianConvert ;
   rc = clientReplicaGroupExtractNode ( data,
                                 r->_hostName,
                                 CLIENT_MAX_HOSTNAME,
                                 r->_serviceName,
                                 CLIENT_MAX_SERVICENAME,
                                 &r->_nodeID ) ;
   if ( rc )
   {
      goto error ;
   }
   ossStrncpy ( r->_nodeName, r->_hostName, CLIENT_MAX_HOSTNAME ) ;
   ossStrncat ( r->_nodeName, NODE_NAME_SERVICE_SEP, 1 ) ;
   ossStrncat ( r->_nodeName, r->_serviceName,
                CLIENT_MAX_SERVICENAME ) ;

   *handle = (sdbNodeHandle)r ;
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetNodeMaster ( sdbShardHandle cHandle,
                                    sdbNodeHandle *handle )
{
   INT32 rc                = SDB_OK ;
   bson_iterator it ;
   bson result ;
   const CHAR *primaryData = NULL ;
   INT32 primaryNode       = -1 ;
   sdbRGStruct *r          = (sdbRGStruct*)cHandle ;
   bson_init ( &result ) ;

   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = _sdbGetShardDetail ( cHandle, &result ) ;
   if ( rc )
   {
      goto error ;
   }
   if ( BSON_INT != bson_find ( &it, &result, CAT_PRIMARY_NAME ) )
   {
      // cannot find primary
      rc = SDB_CLS_NODE_NOT_EXIST ;
      goto error ;
   }
   primaryNode = bson_iterator_int ( &it ) ;
   // extract the primary node and find out the node id
   if ( BSON_ARRAY != bson_find ( &it, &result, CAT_GROUP_NAME ) )
   {
      // the Group is not array
      rc = SDB_SYS ;
      goto error ;
   }
   // walk through Group and find out the NodeID
   {
      const CHAR *groupList = bson_iterator_value ( &it ) ;
      bson_iterator i ;
      bson_iterator_from_buffer ( &i, groupList ) ;
      // loop for all elements in Group
      while ( bson_iterator_next ( &i ) )
      {
         bson intObj ;
         bson_init ( &intObj ) ;
         // make sure each element is object and construct intObj object
         // bson_init_finished_data does not accept const CHAR*,
         // however since we are NOT going to perform any change, it's afe to
         // cast const CHAR* to CHAR* here
         if ( BSON_OBJECT == (signed int)bson_iterator_type ( &i ) &&
              BSON_OK == bson_init_finished_data ( &intObj,
                                       (CHAR*)bson_iterator_value ( &i ) ) )
         {
            bson_iterator k ;
            // look for "NodeID" in each object
            if ( BSON_INT != bson_find ( &k, &intObj, CAT_NODEID_NAME ) )
            {
               rc = SDB_SYS ;
               goto error ;
            }
            // if we find the master, let's record the pointer and jump out
            if ( primaryNode == bson_iterator_int ( &k ) )
            {
               primaryData = intObj.data ;
               break ;
            }
         }
      }
   }
   if ( primaryData )
   {
      rc = _sdbShardExtractNode ( r->_sock, handle, primaryData,
                                         r->_endianConvert ) ;
      if ( rc )
      {
         goto error ;
      }
   }
   else
   {
      // if we find primary id but cannot find primary node in list, return
      // primary not found
      rc = SDB_CLS_NODE_NOT_EXIST ;
      goto error ;
   }
done :
   bson_destroy ( &result ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetNodeSlave ( sdbShardHandle cHandle,
                                   sdbNodeHandle *handle )
{
   INT32 rc                = SDB_OK ;
   bson_iterator it ;
   bson result ;
   const CHAR *primaryData = NULL ;
   INT32 primaryNode       = -1 ;
   sdbRGStruct *r          = (sdbRGStruct*)cHandle ;
   bson_init ( &result ) ;

   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   rc = _sdbGetShardDetail ( cHandle, &result ) ;
   if ( rc )
   {
      goto error ;
   }
   if ( BSON_INT == bson_find ( &it, &result, CAT_PRIMARY_NAME ) )
   {
      // get the primary node and skip it later
      primaryNode = bson_iterator_int ( &it ) ;
   }
   // walk through Group and skip primary node, and pickup a random one
   if ( BSON_ARRAY != bson_find ( &it, &result, CAT_GROUP_NAME ) )
   {
      // the Group is not array
      rc = SDB_SYS ;
      goto error ;
   }
   {
      const CHAR *groupList = bson_iterator_value ( &it ) ;
      bson_iterator i ;
      BOOLEAN first = TRUE ;
      INT32 totalNum = -1 ;
      INT32 fetchNum = -1 ;
retry :
      totalNum = 0 ;
      bson_iterator_from_buffer ( &i, groupList ) ;
      // loop for all elements in Group
      while ( bson_iterator_next ( &i ) )
      {
         bson intObj ;
         bson_init ( &intObj ) ;
         // make sure each element is object and construct intObj object
         // bson_init_finished_data does not accept const CHAR*,
         // however since we are NOT going to perform any change, it's safe
         // to cast const CHAR* to CHAR* here
         if ( BSON_OBJECT == (signed int)bson_iterator_type ( &i ) &&
              BSON_OK == bson_init_finished_data ( &intObj,
                                       (CHAR*)bson_iterator_value ( &i ) ) )
         {
            bson_iterator k ;
            // look for "NodeID" in each object
            if ( BSON_INT != bson_find ( &k, &intObj, CAT_NODEID_NAME ) )
            {
               rc = SDB_SYS ;
               goto error ;
            }
            // if we find the master, let's skip it, otherwise let's push to
            // vector
            if ( primaryNode != bson_iterator_int ( &k ) )
            {
               if ( !first && totalNum == fetchNum )
               {
                  // if it's second time we get here, let's compare whether we
                  // want to take this one
                  primaryData = intObj.data ;
                  break ;
               }
               ++totalNum ;
            }
            else
            {
               // if this is master
               if ( !first && -1 == fetchNum )
               {
                  // if it's second time we get here and there's no slave found
                  // in previous run, let's get the primary
                  primaryData = intObj.data ;
                  break ;
               }
            }
         } // if ( BSON_OBJECT == (signed int)bson_iterator_type ( &i ) &&
      } // while ( bson_iterator_next ( &i ) )
      if ( first )
      {
         // if it's first run, let's mark it already run and randomly pick a
         // slave
         first = FALSE ;
         if ( totalNum )
         {
            fetchNum = _sdbRand() % totalNum ;
         }
         goto retry ;
      }
   }
   if ( primaryData )
   {
      rc = _sdbShardExtractNode ( r->_sock, handle, primaryData,
                                         r->_endianConvert ) ;
      if ( rc )
      {
         goto error ;
      }
   }
   // if we can't find any slave nor primary, something wrong
   else
   {
      rc = SDB_CLS_NODE_NOT_EXIST ;
      goto error ;
   }
done :
   bson_destroy ( &result ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetNodeByName ( sdbShardHandle cHandle,
                                    const CHAR *pNodeName,
                                    sdbNodeHandle *handle )
{
   INT32 rc = SDB_OK ;
   CHAR *pHostName = NULL ;
   CHAR *pServiceName = NULL ;
   sdbRGStruct *r          = (sdbRGStruct*)cHandle ;
   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      handle = NULL ;
      goto error ;
   }
   if ( !pNodeName || !handle )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   pHostName = (CHAR*)SDB_OSS_MALLOC ( ossStrlen ( pNodeName + 1 ) ) ;
   if ( !pHostName )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   pServiceName = ossStrchr ( pHostName, NODE_NAME_SERVICE_SEPCHAR ) ;
   if ( !pServiceName )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   *pServiceName = '\0' ;
   pServiceName ++ ;
   rc = sdbGetNodeByHost ( cHandle, pHostName, pServiceName, handle ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   if ( pHostName )
   {
      SDB_OSS_FREE ( pHostName ) ;
   }
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetNodeByHost ( sdbShardHandle cHandle,
                                           const CHAR *pHostName,
                                           const CHAR *pServiceName,
                                           sdbNodeHandle *handle )
{
   INT32 rc = SDB_OK ;
   const CHAR *hostName = NULL ;
   const CHAR *serviceName = NULL ;
   const CHAR *nodeName = NULL ;
   bson result ;
   bson_iterator it ;
   sdbRGStruct *r          = (sdbRGStruct*)cHandle ;
   bson_init ( &result ) ;

   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      handle = NULL ;
      goto error ;
   }
   if ( !pHostName || !pServiceName || !handle )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   *handle = SDB_INVALID_HANDLE ;

   rc = _sdbGetShardDetail ( cHandle, &result ) ;
   if ( rc )
   {
      goto error ;
   }
   // walk through Group and find out the NodeID
   if ( BSON_ARRAY != bson_find ( &it, &result, CAT_GROUP_NAME ) )
   {
      // the Group is not array
      rc = SDB_SYS ;
      goto error ;
   }
   {
      INT32 nodeID = 0 ;
      const CHAR *groupList = bson_iterator_value ( &it ) ;
      bson_iterator i ;
      bson_iterator_from_buffer ( &i, groupList ) ;
      // loop for all elements in Group
      while ( bson_iterator_next ( &i ) )
      {
         rc = _sdbShardExtractNode ( r->_sock, handle,
               (CHAR*)bson_iterator_value ( &i ),
               r->_endianConvert ) ;
         if ( rc )
         {
            goto error ;
         }
         rc = sdbGetNodeAddr ( *handle, &hostName,
                                      &serviceName, &nodeName,
                                      &nodeID ) ;
         if ( rc )
         {
            goto error ;
         }
         if ( ossStrcmp ( hostName, pHostName ) == 0 &&
              ossStrcmp ( serviceName, pServiceName ) == 0 )
         {
            break ;
         }
         sdbReleaseNode ( *handle ) ;
         *handle = SDB_INVALID_HANDLE ;
      }
   }
   if ( (*handle) == SDB_INVALID_HANDLE )
   {
      // if we can't find the given id
      rc = SDB_CLS_NODE_NOT_EXIST ;
      goto error ;
   }
done :
   bson_destroy ( &result ) ;
   return rc ;
error :
   if ( handle && (*handle!=SDB_INVALID_HANDLE) )
   {
      sdbReleaseNode ( *handle ) ;
   }
   goto done ;
}

SDB_EXPORT INT32 sdbGetNodeAddr ( sdbNodeHandle cHandle,
                                          const CHAR **ppHostName,
                                          const CHAR **ppServiceName,
                                          const CHAR **ppNodeName,
                                          INT32 *pNodeID )
{
   INT32 rc = SDB_OK ;
   sdbRNStruct *r          = (sdbRNStruct*)cHandle ;
   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICANODE )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( ppHostName )
   {
      *ppHostName = r->_hostName ;
   }
   if ( ppServiceName )
   {
      *ppServiceName = r->_serviceName ;
   }
   if ( ppNodeName )
   {
      *ppNodeName = r->_nodeName ;
   }
   if ( pNodeID )
   {
      *pNodeID = r->_nodeID ;
   }
done :
   return rc ;
error :
   goto done ;
}

static INT32 _sdbStartStopNode ( sdbNodeHandle cHandle,
                                        BOOLEAN start )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result = FALSE ;
   bson configuration ;
   sdbRNStruct *r          = (sdbRNStruct*)cHandle ;
   bson_init ( &configuration ) ;

   if ( !r || r->_handleType != SDB_HANDLE_TYPE_REPLICANODE )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = bson_append_string ( &configuration,
                             CAT_HOST_FIELD_NAME, r->_hostName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &configuration,
                             PMD_OPTION_SVCNAME, r->_serviceName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &configuration ) ;
   rc = _runCommand ( r->_sock, &r->_pSendBuffer,
                      &r->_sendBufferSize,
                      &r->_pReceiveBuffer,
                      &r->_receiveBufferSize,
                      r->_endianConvert,
                      start?
                         (CMD_ADMIN_PREFIX CMD_NAME_STARTUP_NODE) :
                         (CMD_ADMIN_PREFIX CMD_NAME_SHUTDOWN_NODE),
                      &result, &configuration,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &configuration ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbStartNode ( sdbNodeHandle cHandle )
{
   return _sdbStartStopNode ( cHandle, TRUE ) ;
}

SDB_EXPORT INT32 sdbStopNode ( sdbNodeHandle cHandle )
{
   return _sdbStartStopNode ( cHandle, FALSE ) ;
}

SDB_EXPORT INT32 sdbListCollectionSpaces ( sdbConnectionHandle cHandle,
                                           sdbCursorHandle *handle )
{
   return sdbGetList ( cHandle, SDB_LIST_COLLECTIONSPACES, NULL, NULL, NULL,
                       handle ) ;
}

SDB_EXPORT INT32 sdbListCollections ( sdbConnectionHandle cHandle,
                                      sdbCursorHandle *handle )
{
   return sdbGetList ( cHandle, SDB_LIST_COLLECTIONS, NULL, NULL, NULL,
                       handle ) ;
}

SDB_EXPORT INT32 sdbListShards ( sdbConnectionHandle cHandle,
                                        sdbCursorHandle *handle )
{
   return sdbGetList ( cHandle, SDB_LIST_SHARDS, NULL, NULL, NULL,
                       handle ) ;
}

SDB_EXPORT INT32 sdbFlushConfigure(sdbConnectionHandle cHandle,
                                   bson *options )
{
   INT32 rc = SDB_OK ;
   BOOLEAN r ;
   SINT64 contextID = 0 ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   rc = clientBuildQueryMsg( &(connection->_pSendBuffer),
                             &(connection->_sendBufferSize),
                             (CMD_ADMIN_PREFIX CMD_NAME_EXPORT_CONFIG),
                             0, 0, 0, -1, options, NULL, NULL, NULL,
                             connection->_endianConvert ) ;
   if ( SDB_OK != rc )
   {
      ossPrintf ( "Failed to build flush msg, rc = %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }

   rc = _send ( connection->_sock, (MsgHeader*)(connection->_pSendBuffer),
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,
                      (MsgHeader**)&connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize, &contextID, &r,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbCrtJSProcedure(sdbConnectionHandle cHandle,
                                   const CHAR *code )
{
   INT32 rc = SDB_OK ;
   BOOLEAN r ;
   SINT64 contextID = 0 ;
   bson bs ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   bson_init( &bs ) ;
   bson_append_code( &bs, FIELD_NAME_FUNC, code ) ;
   bson_append_int( &bs, FMP_FUNC_TYPE, FMP_FUNC_TYPE_JS ) ;
   bson_finish( &bs ) ;

   rc = clientBuildQueryMsg( &(connection->_pSendBuffer),
                             &(connection->_sendBufferSize),
                             (CMD_ADMIN_PREFIX CMD_NAME_CRT_PROCEDURES),
                             0, 0, 0, -1, &bs, NULL, NULL, NULL,
                             connection->_endianConvert ) ;
   if ( SDB_OK != rc )
   {
      ossPrintf ( "Failed to build crt procedures msg, rc = %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }

   rc = _send ( connection->_sock, (MsgHeader*)(connection->_pSendBuffer),
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,
                      (MsgHeader**)&connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize, &contextID, &r,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   bson_destroy( &bs ) ;
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbRmProcedures(sdbConnectionHandle cHandle,
                                 const CHAR *spName )
{
   INT32 rc = SDB_OK ;
   BOOLEAN r ;
   SINT64 contextID = 0 ;
   bson bs ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   bson_init( &bs ) ;
   bson_append_string( &bs, FIELD_NAME_FUNC, spName ) ;
   bson_finish( &bs ) ;
   rc = clientBuildQueryMsg( &(connection->_pSendBuffer),
                             &(connection->_sendBufferSize),
                             (CMD_ADMIN_PREFIX CMD_NAME_RM_PROCEDURES),
                             0, 0, 0, -1, &bs, NULL, NULL, NULL,
                             connection->_endianConvert ) ;
   bson_destroy( &bs ) ;
   if ( SDB_OK != rc )
   {
      ossPrintf ( "Failed to build rm procedues msg, rc = %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }

   rc = _send ( connection->_sock, (MsgHeader*)(connection->_pSendBuffer),
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,
                      (MsgHeader**)&connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize, &contextID, &r,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbListProcedures( sdbConnectionHandle cHandle,
                                    bson *condition,
                                    sdbCursorHandle *handle )
{
   INT32 rc = SDB_OK ;
   rc = sdbGetList( cHandle, SDB_LIST_STOREPROCEDURES, condition, NULL, NULL,
                    handle ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbEvalJS(sdbConnectionHandle cHandle,
                           const CHAR *code,
                           SDB_SPD_RES_TYPE *type,
                           sdbCursorHandle *handle,
                           bson *errmsg )
{
   INT32 rc = SDB_OK ;
   BOOLEAN r ;
   bson bs ;
   SINT64 contextID = 0 ;
   sdbCursorStruct *cursor = NULL ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   bson_init( &bs ) ;
   bson_append_code( &bs, FIELD_NAME_FUNC, code ) ;
   bson_append_int( &bs, FIELD_NAME_FUNCTYPE, FMP_FUNC_TYPE_JS ) ;
   bson_finish( &bs ) ;
   rc = clientBuildQueryMsg( &(connection->_pSendBuffer),
                             &(connection->_sendBufferSize),
                             (CMD_ADMIN_PREFIX CMD_NAME_EVAL),
                             0, 0, 0, -1, &bs, NULL, NULL, NULL,
                             connection->_endianConvert ) ;
   if ( SDB_OK != rc )
   {
      ossPrintf ( "Failed to build flush msg, rc = %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }

   rc = _send ( connection->_sock, (MsgHeader*)(connection->_pSendBuffer),
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtractEval ( connection->_sock,
                          (MsgHeader**)&connection->_pReceiveBuffer,
                           &connection->_receiveBufferSize, &contextID,
                           type, &r, errmsg, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }

   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock = connection->_sock ;
   cursor->_contextID = contextID ;
   cursor->_offset = -1 ;
   cursor->_endianConvert = connection->_endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done:
   bson_destroy( &bs ) ;
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbGetCollection1 ( sdbCSHandle cHandle,
                                     const CHAR *pCollectionName,
                                     sdbCollectionHandle *handle )
{
   INT32 rc                        = SDB_OK ;
   BOOLEAN result                  = FALSE ;
   INT32 nameLength                = 0 ;
   bson newObj ;
   CHAR *pTestCollection           = CMD_ADMIN_PREFIX CMD_NAME_TEST_COLLECTION ;
   CHAR *pName                     = FIELD_NAME_NAME ;
   sdbCollectionStruct *s          = NULL ;
   sdbCSStruct *cs                 = (sdbCSStruct*)cHandle ;
   CHAR fullCollectionName [ CLIENT_COLLECTION_NAMESZ + CLIENT_CS_NAMESZ + 2 ];
   bson_init ( &newObj ) ;

   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_CS )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( !pCollectionName || !handle ||
        (nameLength = ossStrlen ( pCollectionName) ) >
        CLIENT_COLLECTION_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossMemset ( fullCollectionName, 0, sizeof(fullCollectionName) ) ;
   ossStrncpy ( fullCollectionName, cs->_CSName, sizeof(cs->_CSName) ) ;
   ossStrncat ( fullCollectionName, ".", 1 ) ;
   ossStrncat ( fullCollectionName, pCollectionName, CLIENT_COLLECTION_NAMESZ );
   rc = bson_append_string ( &newObj, pName, fullCollectionName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = _runCommand ( cs->_sock, &cs->_pSendBuffer,
                      &cs->_sendBufferSize,
                      &cs->_pReceiveBuffer,
                      &cs->_receiveBufferSize,
                      cs->_endianConvert,
                      pTestCollection, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
   s = (sdbCollectionStruct*) SDB_OSS_MALLOC ( sizeof( sdbCollectionStruct) ) ;
   if ( !s )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( s, 0, sizeof( sdbCollectionStruct) ) ;
   s->_handleType    = SDB_HANDLE_TYPE_COLLECTION ;
   s->_sock          = cs->_sock ;
   s->_endianConvert = cs->_endianConvert ;
   rc = _setCollectionName ( (sdbCollectionHandle)s, fullCollectionName ) ;
   if ( rc )
   {
      SDB_OSS_FREE ( s ) ;
      goto error ;
   }
   *handle = (sdbCollectionHandle)s ;
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbCreateCollection1 ( sdbCSHandle cHandle,
                                        const CHAR *pCollectionName,
                                        bson *options,
                                        sdbCollectionHandle *handle )
{
   INT32 rc                        = SDB_OK ;
   BOOLEAN result                  = FALSE ;
   INT32 nameLength                = 0 ;
   bson newObj ;
   CHAR *pTestCollection           = CMD_ADMIN_PREFIX CMD_NAME_CREATE_COLLECTION ;
   CHAR *pName                     = FIELD_NAME_NAME ;
   sdbCollectionStruct *s          = NULL ;
   sdbCSStruct *cs                 = (sdbCSStruct*)cHandle ;
   bson_iterator it ;
   CHAR fullCollectionName [ CLIENT_COLLECTION_NAMESZ + CLIENT_CS_NAMESZ + 2 ];
   bson_init ( &newObj ) ;

   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_CS )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( !pCollectionName || !handle ||
        (nameLength = ossStrlen ( pCollectionName) ) >
        CLIENT_COLLECTION_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossMemset ( fullCollectionName, 0, sizeof(fullCollectionName) ) ;
   ossStrncpy ( fullCollectionName, cs->_CSName, sizeof(cs->_CSName) ) ;
   ossStrncat ( fullCollectionName, ".", 1 ) ;
   ossStrncat ( fullCollectionName, pCollectionName, CLIENT_COLLECTION_NAMESZ );
   rc = bson_append_string ( &newObj, pName, fullCollectionName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   if ( options )
   {
      bson_iterator_init ( &it, options ) ;
      while ( BSON_EOO != bson_iterator_next ( &it ) )
      {
         bson_append_element ( &newObj, NULL, &it ) ;
      }
   }
   bson_finish ( &newObj ) ;
   rc = _runCommand ( cs->_sock, &cs->_pSendBuffer,
                      &cs->_sendBufferSize,
                      &cs->_pReceiveBuffer,
                      &cs->_receiveBufferSize,
                      cs->_endianConvert,
                      pTestCollection, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
   s = (sdbCollectionStruct*) SDB_OSS_MALLOC ( sizeof( sdbCollectionStruct) ) ;
   if ( !s )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( s, 0, sizeof( sdbCollectionStruct) ) ;
   s->_handleType    = SDB_HANDLE_TYPE_COLLECTION ;
   s->_sock          = cs->_sock ;
   s->_endianConvert = cs->_endianConvert ;
   rc = _setCollectionName ( (sdbCollectionHandle)s, fullCollectionName ) ;
   if ( rc )
   {
      SDB_OSS_FREE ( s ) ;
      goto error ;
   }
   *handle = (sdbCollectionHandle)s ;
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbCreateCollection ( sdbCSHandle cHandle,
                                       const CHAR *pCollectionName,
                                       sdbCollectionHandle *handle )
{
   return sdbCreateCollection1 ( cHandle, pCollectionName, NULL, handle ) ;
}

SDB_EXPORT INT32 sdbAlterCollection ( sdbCollectionHandle cHandle,
                                      bson *options  )
{
   INT32 rc                        = SDB_OK ;
   BOOLEAN result                  = FALSE ;
   bson newObj ;
   SINT64 contextID                = 0 ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( !options ||
        cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_COLLECTION_NAME,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_bson ( &newObj, FIELD_NAME_OPTIONS, options ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_ALTER_COLLECTION,
                              0, 0, -1, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbDropCollection ( sdbCSHandle cHandle,
                                     const CHAR *pCollectionName )
{
   INT32 rc                        = SDB_OK ;
   BOOLEAN result                  = FALSE ;
   INT32 nameLength                = 0 ;
   bson newObj ;
   CHAR *pTestCollection           = CMD_ADMIN_PREFIX CMD_NAME_DROP_COLLECTION ;
   CHAR *pName                     = FIELD_NAME_NAME ;
   sdbCSStruct *cs                 = (sdbCSStruct*)cHandle ;
   CHAR fullCollectionName [ CLIENT_COLLECTION_NAMESZ + CLIENT_CS_NAMESZ + 2 ];
   bson_init ( &newObj ) ;

   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_CS )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( !pCollectionName ||
        (nameLength = ossStrlen ( pCollectionName) ) >
        CLIENT_COLLECTION_NAMESZ )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossMemset ( fullCollectionName, 0, sizeof(fullCollectionName) ) ;
   ossStrncpy ( fullCollectionName, cs->_CSName, sizeof(cs->_CSName) ) ;
   ossStrncat ( fullCollectionName, ".", 1 ) ;
   ossStrncat ( fullCollectionName, pCollectionName, CLIENT_COLLECTION_NAMESZ );
   rc = bson_append_string ( &newObj, pName, fullCollectionName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = _runCommand ( cs->_sock, &cs->_pSendBuffer,
                      &cs->_sendBufferSize,
                      &cs->_pReceiveBuffer,
                      &cs->_receiveBufferSize,
                      cs->_endianConvert,
                      pTestCollection, &result, &newObj,
                      NULL, NULL, NULL ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}


SDB_EXPORT INT32 sdbGetCSName ( sdbCSHandle cHandle,
                                CHAR **ppCSName )
{
   INT32 rc                        = SDB_OK ;
   sdbCSStruct *cs                 = (sdbCSStruct*)cHandle ;
   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_CS )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   *ppCSName = &cs->_CSName[0] ;
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbSplitCollection ( sdbCollectionHandle cHandle,
                                      const CHAR *pSourceGroup,
                                      const CHAR *pTargetGroup,
                                      const bson *pSplitCondition,
                                      const bson *pSplitEndCondition )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   bson newObj ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &newObj ) ;
   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( !pSourceGroup || !pTargetGroup || !pSplitCondition ||
        cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_COLLECTION_NAME,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_SOURCE_NAME,
                             pSourceGroup ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_TARGET_NAME,
                             pTargetGroup ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_bson ( &newObj, CAT_SPLITQUERY_NAME,
                           pSplitCondition ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   if ( NULL != pSplitEndCondition )
   {
      rc = bson_append_bson( &newObj, CAT_SPLITENDQUERY_NAME,
                             pSplitEndCondition ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }

   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_SPLIT,
                              0, 0, -1, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbSplitCLAsync ( sdbCollectionHandle cHandle,
                                   const CHAR *pSourceGroup,
                                   const CHAR *pTargetGroup,
                                   const bson *pSplitCondition,
                                   const bson *pSplitEndCondition,
                                   SINT64 *taskID )
{
   INT32 rc = SDB_OK ;
   BOOLEAN bresult ;
   SINT64 contextID = 0 ;
   bson newObj ;
   bson result ;
   sdbCursorStruct *cursor ;
   bson_iterator it ;
   sdbCollectionStruct *cs = (sdbCollectionStruct *)cHandle ;
   bson_init ( &newObj ) ;
   bson_init ( &result ) ;
   // check handle
   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // check arguments
   if ( !pSourceGroup || !pTargetGroup || !pSplitCondition ||
        cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // append
   rc = bson_append_string ( &newObj, CAT_COLLECTION_NAME,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_SOURCE_NAME,
                             pSourceGroup ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_TARGET_NAME,
                             pTargetGroup ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_bson ( &newObj, CAT_SPLITQUERY_NAME,
                           pSplitCondition ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   if ( NULL != pSplitEndCondition )
   {
      rc = bson_append_bson ( &newObj, CAT_SPLITENDQUERY_NAME,
                              pSplitEndCondition ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }
   // async:true
   rc = bson_append_bool ( &newObj, FIELD_NAME_ASYNC, TRUE ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   // build message
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_SPLIT,
                              0, 0, -1, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   // send message to engine
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   // receive message from engine and then extract info
   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &bresult,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   // build a cursor
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = cs->_sock ;
   cursor->_contextID       = contextID ;
   ossMemcpy ( cursor->_collectionFullName, cs->_collectionFullName,
               sizeof(cursor->_collectionFullName) ) ;
   cursor->_offset          = -1 ;
   // get the taskid
   rc = sdbNext ( (sdbCursorHandle)cursor, &result ) ;
   if ( rc )
   {
      sdbReleaseCursor ( (sdbCursorHandle)cursor ) ;
      goto error ;
   }
   if ( BSON_LONG == bson_find ( &it, &result, FIELD_NAME_TASKID ) )
   {
      *taskID = bson_iterator_long ( &it ) ;
   }
   else
   {
      rc = SDB_SYS ;
   }
   sdbReleaseCursor ( (sdbCursorHandle)cursor ) ;
done :
   bson_destroy ( &newObj ) ;
   bson_destroy ( &result ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbSplitCollectionByPercent( sdbCollectionHandle cHandle,
                                              const CHAR * pSourceGroup,
                                              const CHAR * pTargetGroup,
                                              double percent )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   bson newObj ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   if ( percent <= 0.0 || percent > 100.0 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   if ( !pSourceGroup || !pTargetGroup || cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_COLLECTION_NAME,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_SOURCE_NAME,
                             pSourceGroup ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_TARGET_NAME,
                             pTargetGroup ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_double( &newObj, CAT_SPLITPERCENT_NAME,
                            percent ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_SPLIT,
                              0, 0, -1, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}


SDB_EXPORT INT32 sdbSplitCLByPercentAsync ( sdbCollectionHandle cHandle,
                                            const CHAR *pSourceGroup,
                                            const CHAR *pTargetGroup,
                                            FLOAT64 percent,
                                            SINT64 *taskID )
{
   INT32 rc = SDB_OK ;
   BOOLEAN bresult ;
   SINT64 contextID = 0 ;
   bson newObj ;
   bson result ;
   sdbCursorStruct *cursor ;
   bson_iterator it ;
   sdbCollectionStruct *cs = (sdbCollectionStruct *)cHandle ;
   bson_init ( &newObj ) ;
   bson_init ( &result ) ;
   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( percent <= 0.0 || percent > 100.0 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   if ( !pSourceGroup || !pTargetGroup ||
        cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_COLLECTION_NAME,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_SOURCE_NAME,
                             pSourceGroup ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, CAT_TARGET_NAME,
                           pTargetGroup ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_double ( &newObj, CAT_SPLITPERCENT_NAME,
                             percent ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   // async:true
   rc = bson_append_bool ( &newObj, FIELD_NAME_ASYNC, TRUE ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_SPLIT,
                              0, 0, -1, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &bresult,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = cs->_sock ;
   cursor->_contextID       = contextID ;
   ossMemcpy ( cursor->_collectionFullName, cs->_collectionFullName,
               sizeof(cursor->_collectionFullName) ) ;
   cursor->_offset          = -1 ;
   rc = sdbNext ( (sdbCursorHandle)cursor, &result ) ;
   if ( rc )
   {
      sdbReleaseCursor ( (sdbCursorHandle)cursor ) ;
      goto error ;
   }
   if ( BSON_LONG == bson_find ( &it, &result, FIELD_NAME_TASKID ) )
   {
      *taskID = bson_iterator_long ( &it ) ;
   }
   else
   {
      rc = SDB_SYS ;
   }
   sdbReleaseCursor ( (sdbCursorHandle)cursor ) ;
done :
   bson_destroy ( &newObj ) ;
   bson_destroy ( &result ) ;
   return rc ;
error :
   goto done ;
}

/*
SDB_EXPORT INT32 sdbRenameCollection ( sdbCollectionHandle cHandle,
                                       const CHAR *pNewName )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   bson obj ;
   bson_init ( &obj ) ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( !pNewName )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &obj, FIELD_NAME_COLLECTIONSPACE, cs->_CSName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &obj, FIELD_NAME_OLDNAME, cs->_collectionName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &obj, FIELD_NAME_NEWNAME, pNewName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &obj ) ;

   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_RENAME_COLLECTION,
                              0, 0, -1, -1, &obj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &obj ) ;
   return rc ;
error :
   goto done ;
}*/

SDB_EXPORT INT32 sdbCreateIndex ( sdbCollectionHandle cHandle,
                                  bson *indexDef,
                                  const CHAR *pIndexName,
                                  BOOLEAN isUnique,
                                  BOOLEAN isEnforced )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   bson indexObj ;
   bson newObj ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &indexObj ) ;
   bson_init ( &newObj ) ;
   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' || !indexDef ||
        !pIndexName )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_bson ( &indexObj, IXM_FIELD_NAME_KEY, indexDef ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &indexObj, IXM_FIELD_NAME_NAME, pIndexName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_bool ( &indexObj, IXM_FIELD_NAME_UNIQUE, isUnique ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_bool ( &indexObj, IXM_FIELD_NAME_ENFORCED, isEnforced ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &indexObj ) ;

   rc = bson_append_string ( &newObj, FIELD_NAME_COLLECTION,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_bson ( &newObj, FIELD_NAME_INDEX,  &indexObj ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_CREATE_INDEX,
                              0, 0, -1, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &indexObj ) ;
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetIndexes ( sdbCollectionHandle cHandle,
                                 const CHAR *pIndexName,
                                 sdbCursorHandle *handle )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   bson queryCond ;
   bson newObj ;
   sdbCursorStruct *cursor ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &queryCond ) ;
   bson_init ( &newObj ) ;
   if ( !cs || !handle ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   /* build query condition */
   if ( pIndexName )
   {
      rc = bson_append_string ( &queryCond, IXM_FIELD_NAME_INDEX_DEF "."
                                IXM_FIELD_NAME_NAME, pIndexName ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
      bson_finish ( &queryCond ) ;
   }
   /* build collection name */
   rc = bson_append_string ( &newObj, FIELD_NAME_COLLECTION,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;

   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_GET_INDEXES,
                              0, 0, -1, -1,
                              pIndexName?&queryCond:NULL,
                              NULL, NULL, &newObj, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = cs->_sock ;
   cursor->_contextID       = contextID ;
   ossMemcpy ( cursor->_collectionFullName, cs->_collectionFullName,
               sizeof(cursor->_collectionFullName) ) ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset          = -1 ;
   cursor->_endianConvert   = cs->_endianConvert ;
   *handle                  = (sdbCursorHandle)cursor ;
done :
   bson_destroy ( &queryCond ) ;
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbDropIndex ( sdbCollectionHandle cHandle,
                               const CHAR *pIndexName )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   bson indexObj ;
   bson newObj ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &indexObj ) ;
   bson_init ( &newObj ) ;

   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' ||
        !pIndexName )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &indexObj, "", pIndexName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &indexObj ) ;

   rc = bson_append_string ( &newObj, FIELD_NAME_COLLECTION,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_bson ( &newObj, FIELD_NAME_INDEX,  &indexObj ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_DROP_INDEX,
                              0, 0, -1, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &indexObj ) ;
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetCount ( sdbCollectionHandle cHandle,
                               bson *condition,
                               SINT64 *count )
{
   INT32 rc = SDB_OK ;
   BOOLEAN bresult ;
   SINT64 contextID = 0 ;
   bson newObj ;
   bson result ;
   sdbCursorStruct *cursor ;
   bson_iterator it ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &newObj ) ;
   bson_init ( &result ) ;

   if ( !cs || !count ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   /* build collection name */
   rc = bson_append_string ( &newObj, FIELD_NAME_COLLECTION,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_GET_COUNT,
                              0, 0, -1, -1,
                              condition,
                              NULL, NULL, &newObj, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &bresult,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = cs->_sock ;
   cursor->_contextID       = contextID ;
   ossMemcpy ( cursor->_collectionFullName, cs->_collectionFullName,
               sizeof(cursor->_collectionFullName) ) ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset          = -1 ;
   cursor->_endianConvert   = cs->_endianConvert ;
   rc = sdbNext ( (sdbCursorHandle)cursor, &result ) ;
   if ( rc )
   {
      sdbReleaseCursor ( (sdbCursorHandle)cursor ) ;
      goto error ;
   }
   if ( BSON_LONG == bson_find ( &it, &result, FIELD_NAME_TOTAL ) )
   {
      *count = bson_iterator_long ( &it ) ;
   }
   else
   {
      rc = SDB_SYS ;
   }
   sdbReleaseCursor ( (sdbCursorHandle)cursor ) ;
done :
   bson_destroy ( &newObj ) ;
   bson_destroy ( &result ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbGetCount1 ( sdbCollectionHandle cHandle,
                                bson *condition,
                                bson *hint,
                                SINT64 *count )
{
   INT32 rc = SDB_OK ;
   BOOLEAN bresult ;
   SINT64 contextID = 0 ;
   bson newObj ;
   bson result ;
   sdbCursorStruct *cursor ;
   bson_iterator it ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &newObj ) ;
   bson_init ( &result ) ;

   if ( !cs || !count ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   /* build collection name */
   rc = bson_append_string ( &newObj, FIELD_NAME_COLLECTION,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   // add the hint when it's need
   if ( hint )
   {
      rc = bson_append_bson ( &newObj, FIELD_NAME_HINT, hint ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_GET_COUNT,
                              0, 0, -1, -1,
                              condition,
                              NULL, NULL, &newObj, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &bresult,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = cs->_sock ;
   cursor->_contextID       = contextID ;
   ossMemcpy ( cursor->_collectionFullName, cs->_collectionFullName,
               sizeof(cursor->_collectionFullName) ) ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset          = -1 ;
   cursor->_endianConvert   = cs->_endianConvert ;
   rc = sdbNext ( (sdbCursorHandle)cursor, &result ) ;
   if ( rc )
   {
      sdbReleaseCursor ( (sdbCursorHandle)cursor ) ;
      goto error ;
   }
   if ( BSON_LONG == bson_find ( &it, &result, FIELD_NAME_TOTAL ) )
   {
      *count = bson_iterator_long ( &it ) ;
   }
   else
   {
      rc = SDB_SYS ;
   }
   sdbReleaseCursor ( (sdbCursorHandle)cursor ) ;
done :
   bson_destroy ( &newObj ) ;
   bson_destroy ( &result ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbInsert ( sdbCollectionHandle cHandle,
                             bson *obj )
{
   return sdbInsert1 ( cHandle, obj, NULL ) ;
}

SDB_EXPORT INT32 sdbInsert1 ( sdbCollectionHandle cHandle,
                              bson *obj, bson_iterator *id )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID ;
   BOOLEAN result ;
   bson_iterator tempid ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientAppendOID ( obj, &tempid ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = clientBuildInsertMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                               cs->_collectionFullName, 0, 0, obj, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   if ( id )
   {
      ossMemcpy ( id, &tempid, sizeof(bson_iterator) ) ;
   }
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbBulkInsert ( sdbCollectionHandle cHandle,
                                 SINT32 flags, bson **obj, SINT32 num )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID ;
   BOOLEAN result ;
   SINT32 count = 0 ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   if ( num < 0)
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   else if ( num == 0 )
   {
      // in this case, prevent use the cs->_pSendBuffer to send thing to engine
      goto done ;
   }
   for ( count = 0; count < num; ++count )
   {
      if ( !obj[count] )
         break ;
      rc = clientAppendOID ( obj[count], NULL ) ;
      if ( rc )
      {
         goto error ;
      }
      if ( 0 == count )
         rc = clientBuildInsertMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                                     cs->_collectionFullName, flags, 0,
                                     obj[count], cs->_endianConvert ) ;
      else
         rc = clientAppendInsertMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                                      obj[count], cs->_endianConvert ) ;
      if ( rc )
      {
         goto error ;
      }
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}
/*
static INT32 _sdbUpdate ( SOCKET sock, CHAR *pCollectionFullName,
                          CHAR **ppSendBuffer, INT32 *sendBufferSize,
                          CHAR **ppReceiveBuffer, INT32 *receiveBufferSize,
                          BOOLEAN endianConvert,
                          bson *rule, bson *condition, bson *hint )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID ;
   BOOLEAN result ;
   if ( !pCollectionFullName || !ppSendBuffer || !sendBufferSize ||
        !ppReceiveBuffer || !receiveBufferSize || !rule )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = clientBuildUpdateMsg ( ppSendBuffer, sendBufferSize,
                               pCollectionFullName, 0, 0, condition,
                               rule, hint, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( sock, (MsgHeader*)(*ppSendBuffer), endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( sock, (MsgHeader**)ppReceiveBuffer,
                       receiveBufferSize, &contextID, &result,
                       endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}
*/
static INT32 __sdbUpdate ( sdbCollectionHandle cHandle,
                           SINT32 flag,
                           bson *rule,
                           bson *condition,
                           bson *hint )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID ;
   BOOLEAN result ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientBuildUpdateMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                                cs->_collectionFullName, flag, 0, condition,
                                rule, hint, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbUpdate ( sdbCollectionHandle cHandle,
                             bson *rule,
                             bson *condition,
                             bson *hint )
{
   return __sdbUpdate ( cHandle, 0, rule, condition, hint ) ;
}

SDB_EXPORT INT32 sdbUpsert ( sdbCollectionHandle cHandle,
                             bson *rule,
                             bson *condition,
                             bson *hint )
{
   return __sdbUpdate ( cHandle, FLG_UPDATE_UPSERT, rule, condition, hint ) ;
}
/*
static INT32 _sdbDelete ( SOCKET sock, CHAR *pCollectionFullName,
                          CHAR **ppSendBuffer, INT32 *sendBufferSize,
                          CHAR **ppReceiveBuffer, INT32 *receiveBufferSize,
                          BOOLEAN endianConvert,
                          bson *condition, bson *hint )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID ;
   BOOLEAN result ;
   if ( !pCollectionFullName || !ppSendBuffer || !sendBufferSize ||
        !ppReceiveBuffer || !receiveBufferSize )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = clientBuildDeleteMsg ( ppSendBuffer, sendBufferSize,
                               pCollectionFullName, 0, 0, condition,
                               hint, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( sock, (MsgHeader*)(*ppSendBuffer), endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( sock, (MsgHeader**)ppReceiveBuffer,
                       receiveBufferSize, &contextID, &result,
                       endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}
*/

SDB_EXPORT INT32 sdbDelete ( sdbCollectionHandle cHandle,
                             bson *condition,
                             bson *hint )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID ;
   BOOLEAN result ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientBuildDeleteMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                                cs->_collectionFullName, 0, 0, condition,
                                hint, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbQuery ( sdbCollectionHandle cHandle,
                            bson *condition,
                            bson *select,
                            bson *orderBy,
                            bson *hint,
                            INT64 numToSkip,
                            INT64 numToReturn,
                            sdbCursorHandle *handle )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID ;
   BOOLEAN result ;
   sdbCursorStruct *cursor = NULL ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              cs->_collectionFullName, 0, 0,
                              numToSkip, numToReturn, condition,
                              select, orderBy, hint, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer,
                cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = cs->_sock ;
   cursor->_contextID       = contextID ;
   ossMemcpy ( cursor->_collectionFullName, cs->_collectionFullName,
               sizeof(cursor->_collectionFullName) ) ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset          = -1 ;
   cursor->_endianConvert   = cs->_endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done :
   return rc ;
error :
   goto done ;
}
/*
static INT32 _sdbQuery ( SOCKET sock, CHAR *pCollectionFullName,
                         CHAR **ppSendBuffer, INT32 *sendBufferSize,
                         CHAR **ppReceiveBuffer, INT32 *receiveBufferSize,
                         BOOLEAN endianConvert,
                         bson *condition,
                         bson *select,
                         bson *orderBy,
                         bson *hint,
                         INT64 numToSkip,
                         INT64 numToReturn,
                         sdbCursorHandle *handle )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID ;
   BOOLEAN result ;
   sdbCursorStruct *cursor = NULL ;
   if ( !pCollectionFullName || !ppSendBuffer || !sendBufferSize ||
        !ppReceiveBuffer || !receiveBufferSize  )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   if ( pCollectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientBuildQueryMsg ( ppSendBuffer, sendBufferSize,
                              pCollectionFullName, 0, 0,
                              numToSkip, numToReturn, condition,
                              select, orderBy, hint, endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( sock, (MsgHeader*)(*ppSendBuffer),
                endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( sock, (MsgHeader**)ppReceiveBuffer,
                       receiveBufferSize, &contextID, &result,
                       endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = sock ;
   cursor->_contextID       = contextID ;
   ossMemcpy ( cursor->_collectionFullName, pCollectionFullName,
               sizeof(cursor->_collectionFullName) ) ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset          = -1 ;
   cursor->_endianConvert   = endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done :
   return rc ;
error :
   goto done ;
}
*/
SDB_EXPORT INT32 sdbNext ( sdbCursorHandle cHandle,
                           bson *obj )
{
   INT32 rc = SDB_OK ;
   MsgOpReply *pReply = NULL ;
   bson localobj ;
   sdbCursorStruct *cs = (sdbCursorStruct*)cHandle ;
   bson_init ( &localobj ) ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_CURSOR )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   /*
   if ( cs->_modifiedCurrent )
   {
      bson_destroy ( cs->_modifiedCurrent ) ;
      SDB_OSS_FREE ( cs->_modifiedCurrent ) ;
      cs->_modifiedCurrent = NULL ;
   }
   */
   if ( !cs->_pReceiveBuffer )
   {
      cs->_offset = -1 ;
      rc = _readNextBuffer ( cs->_sock, &cs->_pSendBuffer, &cs->_sendBufferSize,
                             &cs->_pReceiveBuffer, &cs->_receiveBufferSize,
                             cs->_contextID, cs->_endianConvert ) ;
      if ( rc )
      {
         goto error ;
      }
   }
retry :
   pReply = (MsgOpReply*)cs->_pReceiveBuffer ;
   if ( -1 == cs->_offset )
   {
      cs->_offset = ossRoundUpToMultipleX ( sizeof(MsgOpReply), 4 ) ;
   }
   else
   {
      cs->_offset += ossRoundUpToMultipleX
            ( *(INT32*)&cs->_pReceiveBuffer[cs->_offset], 4 ) ;
   }
   if ( cs->_offset >= pReply->header.messageLength ||
        cs->_offset >= cs->_receiveBufferSize )
   {
      cs->_offset = -1 ;
      rc = _readNextBuffer ( cs->_sock, &cs->_pSendBuffer, &cs->_sendBufferSize,
                             &cs->_pReceiveBuffer, &cs->_receiveBufferSize,
                             cs->_contextID, cs->_endianConvert ) ;
      if ( rc )
      {
         goto error ;
      }
      goto retry ;
   }
   rc = bson_init_finished_data ( &localobj, &cs->_pReceiveBuffer [ cs->_offset] ) ;
   if ( rc )
   {
      rc = SDB_CORRUPTED_RECORD ;
      goto done ;
   }
   // copy to output result
   rc = bson_copy ( obj, &localobj ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto done ;
   }
   ++ cs->_totalRead ;
done :
   bson_destroy ( &localobj ) ;
//   cs->_isDeleteCurrent = FALSE ;
   return rc ;
error :
   if ( SDB_DMS_EOC == rc )
   {
      cs->_contextID = -1 ;
   }
   goto done ;
}

SDB_EXPORT INT32 sdbCurrent ( sdbCursorHandle cHandle,
                              bson *obj )
{
   INT32 rc            = SDB_OK ;
   MsgOpReply *pReply  = NULL ;
   sdbCursorStruct *cs = (sdbCursorStruct*)cHandle ;
   bson localobj ;
   bson_init ( &localobj ) ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_CURSOR )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   /*
   //we can't get the current record when it was deleted
   if(cs->_isDeleteCurrent)
   {
      rc = SDB_CURRENT_RECORD_DELETED ;
      goto error ;
   }
   */
   //invalid parameter
   if ( !obj )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   //make sure the obj has been initialized
   bson_init( obj ) ;
   /*
   if ( cs->_modifiedCurrent )
   {
      //deep copy,never use ossmencpy() which is shallow copy
      bson_copy ( obj, cs->_modifiedCurrent ) ;
      goto done ;
   }
   */
   if ( !cs->_pReceiveBuffer )
   {
      cs->_offset = -1 ;
      rc = _readNextBuffer ( cs->_sock, &cs->_pSendBuffer, &cs->_sendBufferSize,
                             &cs->_pReceiveBuffer, &cs->_receiveBufferSize,
                             cs->_contextID, cs->_endianConvert ) ;
      if ( rc )
      {
         goto error ;
      }
   }
retry :
   pReply = (MsgOpReply*)cs->_pReceiveBuffer ;
   if ( -1 == cs->_offset )
   {
      cs->_offset = ossRoundUpToMultipleX ( sizeof(MsgOpReply), 4 ) ;
   }

   if ( cs->_offset >= pReply->header.messageLength ||
        cs->_offset >= cs->_receiveBufferSize )
   {
      cs->_offset = -1 ;
      rc = _readNextBuffer ( cs->_sock, &cs->_pSendBuffer, &cs->_sendBufferSize,
                             &cs->_pReceiveBuffer, &cs->_receiveBufferSize,
                             cs->_contextID, cs->_endianConvert ) ;
      if ( rc )
      {
         goto error ;
      }
      goto retry ;
   }
   rc = bson_init_finished_data ( &localobj, &cs->_pReceiveBuffer [ cs->_offset] ) ;
   if ( rc )
   {
      rc = SDB_CORRUPTED_RECORD ;
      goto done ;
   }
   rc = bson_copy ( obj, &localobj ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   ++ cs->_totalRead ;
done :
   bson_destroy ( &localobj ) ;
   return rc ;
error :
   if ( SDB_DMS_EOC == rc )
   {
      cs->_contextID = -1 ;
   }
   goto done ;
}
/*
SDB_EXPORT INT32 sdbUpdateCurrent ( sdbCursorHandle cHandle,
                                    bson *rule )
{
   INT32 rc = SDB_OK ;
   // declare variables
   bson obj ;
   bson_init ( &obj ) ;
   bson updateCondition ;
   bson_init ( &updateCondition ) ;
   bson_iterator it ;
   bson hintObj ;
   bson_init ( &hintObj ) ;
   // create index scan hint in order to reduce cost for optimizer
   bson_append_string ( &hintObj, "", CLIENT_RECORD_ID_INDEX ) ;
   bson_finish ( &hintObj ) ;

   // declare buffer variables
   CHAR *pSendBuffer       = NULL ;
   INT32 sendBufferSize    = 0 ;
   CHAR *pReceiveBuffer    = NULL ;
   INT32 receiveBufferSize = 0 ;

   bson modifiedObj ;
   bson_init ( &modifiedObj ) ;

   // declare cursor handle
   sdbCursorHandle tempQuery = SDB_INVALID_HANDLE ;
   // convert handle to struct
   sdbCursorStruct *cs = (sdbCursorStruct*)cHandle ;
   // make sure the handle is valid and the type is correct
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_CURSOR )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // validate collection got valid name
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   //we can't update the current record when it was deleted
//   if( cs->_isDeleteCurrent )
//   {
//      rc = SDB_CURRENT_RECORD_DELETED ;
//      goto error ;
//   }

   // get the current record from handl
   rc = sdbCurrent ( cHandle, &obj ) ;
   if ( rc )
   {
      goto error ;
   }
   // extract the record id field from object
   if ( BSON_EOO != bson_find ( &it, &obj, CLIENT_RECORD_ID_FIELD ) )
   {
      // construct update condition using the id field
      rc = bson_append_element ( &updateCondition, NULL, &it ) ;
      // sanity check
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
      // finish building update condition
      rc = bson_finish ( &updateCondition ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }
   else
   {
      // all record must have _id field
      rc = SDB_CORRUPTED_RECORD ;
      goto error ;
   }
   // perform update command and validate the return value
   rc = _sdbUpdate ( cs->_sock, cs->_collectionFullName,
                     &pSendBuffer, &sendBufferSize,
                     &pReceiveBuffer, &receiveBufferSize,
                      cs->_endianConvert,
                      rule,
                     &updateCondition, &hintObj ) ;
   if ( rc )
   {
      goto error ;
   }
   // after update, we have to perform another query, note the cursor object is
   // stored in tempQuery
   rc = _sdbQuery ( cs->_sock, cs->_collectionFullName, &pSendBuffer,
                    &sendBufferSize, &pReceiveBuffer, &receiveBufferSize,
                    cs->_endianConvert,
                    &updateCondition, NULL, NULL,
                    &hintObj, 0, 1, &tempQuery ) ;
   if ( rc )
   {
      goto error ;
   }
   // extract the record from cursor
   rc = sdbNext ( tempQuery, &modifiedObj ) ;
   if ( rc )
   {
      // we should not hit error even for SDB_DMS_EOC, since the record supposed
      // to be in database, unless the record is deleted before the query
      goto error ;
   }
   // now the new modified data is stored in modifiedObj
   // then let's delete the current one if there's exist
   if ( !cs->_modifiedCurrent )
   {
      // allocate memory for a temporary bson object and save it in the cursor
      // the memory is freed when the cursor move to next record ( call sdbNext
      // ) or destroyed. We need to be careful here to avoid memory leak
      cs->_modifiedCurrent = (bson*)SDB_OSS_MALLOC ( sizeof(bson) ) ;
      if ( !cs->_modifiedCurrent )
      {
         rc = SDB_OOM ;
         goto error ;
      }
      bson_init ( cs->_modifiedCurrent ) ;
   }
   // perform a copy because modifiedObj is local variable, the memory will
   // be freed once release tempQuery handle
   rc = bson_copy ( cs->_modifiedCurrent, &modifiedObj ) ;
   if ( BSON_OK != rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
done :
   // release the temporary cursor handle
   if ( SDB_INVALID_HANDLE != tempQuery )
   {
      sdbReleaseCursor ( tempQuery ) ;
      tempQuery = SDB_INVALID_HANDLE ;
   }
   // release send buffer for temporary update request
   if ( pSendBuffer )
   {
      SDB_OSS_FREE ( pSendBuffer ) ;
   }
   // release receive buffer
   if ( pReceiveBuffer )
   {
      SDB_OSS_FREE ( pReceiveBuffer ) ;
   }
   // destroy local bson objects
   bson_destroy ( &updateCondition ) ;
   bson_destroy ( &hintObj ) ;
   bson_destroy ( &modifiedObj ) ;
   return rc ;
error :
   goto done ;
}
*/
/*
SDB_EXPORT INT32 sdbDeleteCurrent ( sdbCursorHandle cHandle )
{
   INT32 rc = SDB_OK ;
   // declare variables
   bson obj ;
   bson_init ( &obj ) ;
   bson updateCondition ;
   bson_init ( &updateCondition ) ;
   bson_iterator it ;
   bson hintObj ;
   bson_init ( &hintObj ) ;
   // create index scan hint in order to reduce cost for optimizer
   bson_append_string ( &hintObj, "", CLIENT_RECORD_ID_INDEX ) ;
   bson_finish ( &hintObj ) ;
   // declare buffer variables
   CHAR *pSendBuffer = NULL ;
   INT32 sendBufferSize = 0 ;
   CHAR *pReceiveBuffer = NULL ;
   INT32 receiveBufferSize = 0 ;
   // convert handle to struct
   sdbCursorStruct *cs = (sdbCursorStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_CURSOR )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // make sure the handle is valid and the type is correct
   if ( cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   //we can't delete current record twice
   if(cs->_isDeleteCurrent)
   {
      rc = SDB_CURRENT_RECORD_DELETED ;
      goto error ;
   }
   // get the current record from handl
   rc = sdbCurrent ( cHandle, &obj ) ;
   if ( rc )
   {
      goto error ;
   }
   // extract the record id field from object
   if ( BSON_EOO != bson_find ( &it, &obj, CLIENT_RECORD_ID_FIELD ) )
   {
      // construct update condition using the id field
      rc = bson_append_element ( &updateCondition, NULL, &it ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
      // finish building update condition
      rc = bson_finish ( &updateCondition ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }
   else
   {
      // all record must have _id field
      rc = SDB_CORRUPTED_RECORD ;
      goto error ;
   }
   // perform delete command and validate the return value
   rc = _sdbDelete ( cs->_sock, cs->_collectionFullName,
                     &pSendBuffer, &sendBufferSize,
                     &pReceiveBuffer, &receiveBufferSize,
                     cs->_endianConvert,
                     &updateCondition, &hintObj ) ;
   if ( rc )
   {
      goto error ;
   }
   if ( cs->_modifiedCurrent )
   {
      bson_destroy ( cs->_modifiedCurrent ) ;
      SDB_OSS_FREE ( cs->_modifiedCurrent ) ;
      cs->_modifiedCurrent = NULL ;
   }
done :
   if ( pSendBuffer )
   {
      SDB_OSS_FREE ( pSendBuffer ) ;
   }
   if ( pReceiveBuffer )
   {
      SDB_OSS_FREE ( pReceiveBuffer ) ;
   }
   bson_destroy ( &updateCondition ) ;
   bson_destroy ( &hintObj ) ;
   cs->_isDeleteCurrent = TRUE ;
   return rc ;
error :
   goto done ;
}
*/

SDB_EXPORT INT32 sdbCloseCursor ( sdbCursorHandle cHandle )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result = FALSE ;
   SINT64 contextID = -1 ;
   sdbCursorStruct *cs = (sdbCursorStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_CURSOR )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = clientBuildKillContextsMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize, 0, 1,
                                     &cs->_contextID, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID,
                       &result, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cs->_contextID = -1 ;
done :
   return rc ;
error :
   goto done ;
}

#define TRACE_FIELD_SEP ','
static INT32 sdbTraceStrtok ( bson *obj, CHAR *pLine )
{
   INT32 rc     = SDB_OK ;
   INT32 len    = 0 ;
   CHAR *pStart = pLine ;
   CHAR *pStop  = pLine ;
   if ( pLine == NULL )
   {
      goto done ;
   }
   len = ossStrlen ( pLine ) ;
   while ( pStart - pLine <= len &&
           pStop  - pLine <= len )
   {
      // skip all empty chars in front
      if ( ( pStart == pStop ) &&
           SDB_IS_EMPTY_CHAR ( *pStop ) )
         ++pStart ;
      // when we hit separator
      else if ( *pStop == TRACE_FIELD_SEP ||
                *pStop == '\0' )
      {
         // set to newline char
         *pStop = '\0' ;
         // we only process if it's not empty string
         if ( pStart != pStop )
         {
            // scan back to remove all empty chars
            CHAR *pTmp = pStop - 1 ;
            while ( ( pTmp > pStart ) &&
                    SDB_IS_EMPTY_CHAR ( *pTmp ) )
            {
               *pTmp = '\0' ;
               --pTmp ;
            }
            // append query object
            rc = bson_append_string ( obj, "", pStart ) ;
            if ( rc )
            {
               rc = SDB_SYS ;
               goto error ;
            }
         }
         // set pstart to stop + 1
         pStart = pStop + 1 ;
      }
      // increase pstop, keep pstart remains
      ++pStop ;
   }
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbTraceStart ( sdbConnectionHandle cHandle,
                                 UINT32 traceBufferSize,
                                 CHAR * comp,
                                 CHAR * breakPoint )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   bson obj ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   // init obj
   bson_init( &obj );
   rc = bson_append_long ( &obj, FIELD_NAME_SIZE, (INT64)traceBufferSize ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   if ( comp )
   {
      rc = bson_append_start_array ( &obj, FIELD_NAME_COMPONENTS ) ;
      if( rc )
      {
         rc = SDB_SYS;
         goto error;
      }
      rc = sdbTraceStrtok ( &obj, comp ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = bson_append_finish_array( &obj );
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }

   if ( breakPoint )
   {
      rc = bson_append_start_array( &obj, FIELD_NAME_BREAKPOINTS );
      if( rc )
      {
         rc = SDB_SYS;
         goto error;
      }
      rc = sdbTraceStrtok ( &obj, breakPoint ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = bson_append_finish_array( &obj );
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }
   bson_finish ( &obj ) ;
   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      CMD_ADMIN_PREFIX CMD_NAME_TRACE_START, &result, &obj,
                      NULL, NULL, NULL ) ;
done :
   bson_destroy ( &obj ) ;
   return rc ;
error :
   goto done ;
}


SDB_EXPORT INT32 sdbTraceResume ( sdbConnectionHandle cHandle )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      CMD_ADMIN_PREFIX CMD_NAME_TRACE_RESUME, &result, NULL,
                      NULL, NULL, NULL ) ;
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbTraceStop ( sdbConnectionHandle cHandle,
                                const CHAR *pDumpFileName )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   bson obj ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &obj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( pDumpFileName )
   {
      rc = bson_append_string ( &obj, FIELD_NAME_FILENAME,
                                pDumpFileName ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }
   bson_finish ( &obj ) ;
   rc = _runCommand ( connection->_sock, &connection->_pSendBuffer,
                      &connection->_sendBufferSize,
                      &connection->_pReceiveBuffer,
                      &connection->_receiveBufferSize,
                      connection->_endianConvert,
                      CMD_ADMIN_PREFIX CMD_NAME_TRACE_STOP, &result, &obj,
                      NULL, NULL, NULL ) ;
done :
   bson_destroy ( &obj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbTraceStatus ( sdbConnectionHandle cHandle,
                                  sdbCursorHandle *handle )
{
   INT32 rc = SDB_OK ;
   BOOLEAN r = FALSE;
   SINT64 contextID = 0 ;
   sdbCursorStruct *cursor = NULL;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer,
                              &connection->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_TRACE_STATUS,
                              0, 0, 0, -1, NULL, NULL, NULL,
                              NULL, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _send (connection->_sock, (MsgHeader*)connection->_pSendBuffer,
               connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize,
                       &contextID, &r,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = connection->_sock ;
   cursor->_contextID       = contextID ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset          = -1 ;
   cursor->_endianConvert   = connection->_endianConvert ;
   *handle                  = (sdbCursorHandle)cursor ;
done:
   return rc ;
error:
   goto done ;
}


SDB_EXPORT INT32 sdbExecUpdate( sdbConnectionHandle cHandle,
                                const CHAR *sql )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   rc = clientValidateSql( sql, FALSE ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = clientBuildSqlMsg( &connection->_pSendBuffer,
                           &connection->_sendBufferSize, sql, 0, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send (connection->_sock,(MsgHeader*)connection->_pSendBuffer,
               connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,(MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize,
                       &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbExec( sdbConnectionHandle cHandle,
                          const CHAR *sql,
                          sdbCursorHandle *result )
{
   INT32 rc = SDB_OK ;
   BOOLEAN r ;
   SINT64 contextID = 0 ;
   sdbCursorStruct *cursor = NULL;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }

   rc = clientValidateSql( sql, TRUE ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = clientBuildSqlMsg( &connection->_pSendBuffer,
                           &connection->_sendBufferSize, sql, 0, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send (connection->_sock, (MsgHeader*)connection->_pSendBuffer,
               connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize,
                       &contextID, &r,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType      = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock            = connection->_sock ;
   cursor->_contextID       = contextID ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset          = -1 ;
   cursor->_endianConvert   = connection->_endianConvert ;
   *result                  = (sdbCursorHandle)cursor ;
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbTransactionBegin( sdbConnectionHandle cHandle )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = clientBuildTransactionBegMsg( &connection->_pSendBuffer,
                           &connection->_sendBufferSize, 0, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send (connection->_sock,(MsgHeader*)connection->_pSendBuffer,
               connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,(MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize,
                       &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbTransactionCommit( sdbConnectionHandle cHandle )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = clientBuildTransactionCommitMsg( &connection->_pSendBuffer,
                           &connection->_sendBufferSize, 0, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send (connection->_sock,(MsgHeader*)connection->_pSendBuffer,
               connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,(MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize,
                       &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT INT32 sdbTransactionRollback( sdbConnectionHandle cHandle )
{
   INT32 rc = SDB_OK ;
   BOOLEAN result ;
   SINT64 contextID = 0 ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION)
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = clientBuildTransactionRollbackMsg( &connection->_pSendBuffer,
                           &connection->_sendBufferSize, 0, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send (connection->_sock,(MsgHeader*)connection->_pSendBuffer,
               connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock,(MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize,
                       &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

SDB_EXPORT void sdbReleaseConnection ( sdbConnectionHandle cHandle )
{
   sdbConnectionStruct *cs = (sdbConnectionStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      return ;
   }
   if ( cs->_pSendBuffer )
   {
      SDB_OSS_FREE (cs->_pSendBuffer ) ;
   }
   if ( cs->_pReceiveBuffer )
   {
      SDB_OSS_FREE ( cs->_pReceiveBuffer ) ;
   }
   SDB_OSS_FREE ( (sdbConnectionStruct*)cHandle ) ;
}

SDB_EXPORT void sdbReleaseCollection ( sdbCollectionHandle cHandle )
{
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      return ;
   }
   if ( cs->_pSendBuffer )
   {
      SDB_OSS_FREE (cs->_pSendBuffer ) ;
   }
   if ( cs->_pReceiveBuffer )
   {
      SDB_OSS_FREE ( cs->_pReceiveBuffer ) ;
   }
   SDB_OSS_FREE ( (sdbCollectionStruct*)cHandle ) ;
}

SDB_EXPORT void sdbReleaseCS ( sdbCSHandle cHandle )
{
   sdbCSStruct *cs = (sdbCSStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_CS )
   {
      return ;
   }
   if ( cs->_pSendBuffer )
   {
      SDB_OSS_FREE (cs->_pSendBuffer ) ;
   }
   if ( cs->_pReceiveBuffer )
   {
      SDB_OSS_FREE ( cs->_pReceiveBuffer ) ;
   }
   SDB_OSS_FREE ( (sdbCSStruct*)cHandle ) ;
}

SDB_EXPORT void sdbReleaseShard ( sdbShardHandle cHandle )
{
   sdbRGStruct *rg = (sdbRGStruct*)cHandle ;
   if ( !rg || rg->_handleType != SDB_HANDLE_TYPE_REPLICAGROUP )
   {
      return ;
   }
   if ( rg->_pSendBuffer )
   {
      SDB_OSS_FREE ( rg->_pSendBuffer ) ;
   }
   if ( rg->_pReceiveBuffer )
   {
      SDB_OSS_FREE ( rg->_pReceiveBuffer ) ;
   }
   SDB_OSS_FREE ( (sdbRGStruct*)cHandle ) ;
}

SDB_EXPORT void sdbReleaseNode ( sdbNodeHandle cHandle )
{
   sdbRNStruct *rn = (sdbRNStruct*)cHandle ;
   if ( !rn || rn->_handleType != SDB_HANDLE_TYPE_REPLICANODE )
   {
      return ;
   }
   if ( rn->_pSendBuffer )
   {
      SDB_OSS_FREE ( rn->_pSendBuffer ) ;
   }
   if ( rn->_pReceiveBuffer )
   {
      SDB_OSS_FREE ( rn->_pReceiveBuffer ) ;
   }
   SDB_OSS_FREE ( (sdbRNStruct*)cHandle ) ;
}

SDB_EXPORT void sdbReleaseCursor ( sdbCursorHandle cHandle )
{
   sdbCursorStruct *cs = (sdbCursorStruct*)cHandle ;
   if ( !cs || cs->_handleType != SDB_HANDLE_TYPE_CURSOR )
   {
      return ;
   }
   if ( cs->_sock && -1 != cs->_contextID )
   {
      _killCursor ( cs->_sock, &cs->_pSendBuffer, &cs->_sendBufferSize,
                    &cs->_pReceiveBuffer, &cs->_receiveBufferSize,
                    cs->_contextID, cs->_endianConvert ) ;
   }
   if ( cs->_pSendBuffer )
   {
      SDB_OSS_FREE (cs->_pSendBuffer ) ;
   }
   if ( cs->_pReceiveBuffer )
   {
      SDB_OSS_FREE ( cs->_pReceiveBuffer ) ;
   }
   /*
   if ( cs->_modifiedCurrent )
   {
      bson_destroy ( cs->_modifiedCurrent ) ;
      SDB_OSS_FREE ( cs->_modifiedCurrent ) ;
      cs->_modifiedCurrent = NULL ;
   }
   */
   /*
   if( cs->_isDeleteCurrent )
   {
     cs->_isDeleteCurrent = FALSE ;
   }
   */
   SDB_OSS_FREE ( (sdbCursorStruct*)cHandle ) ;
}

SDB_EXPORT INT32 sdbAggregate ( sdbCollectionHandle cHandle,
                                 bson **obj, SINT32 num,
                                 sdbCursorHandle *handle )
{
   INT32 rc = SDB_OK ;
   SINT64 contextID = -1;
   BOOLEAN result = FALSE;
   SINT32 count = 0 ;
   sdbCursorStruct *cursor = NULL;
   sdbCollectionStruct *sdbCL = (sdbCollectionStruct *)cHandle ;
   if ( !sdbCL || sdbCL->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error;
   }
   if ( sdbCL->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG;
      goto error;
   }
   for ( count = 0; count < num; ++count )
   {
      if ( !obj[count] )
         break ;
      if ( 0 == count )
         rc = clientBuildAggrRequest ( &sdbCL->_pSendBuffer, &sdbCL->_sendBufferSize,
                                      sdbCL->_collectionFullName, obj[count],
                                      sdbCL->_endianConvert ) ;
      else
         rc = clientAppendAggrRequest ( &sdbCL->_pSendBuffer,
                                        &sdbCL->_sendBufferSize,
                                        obj[count], sdbCL->_endianConvert ) ;
      if ( rc )
      {
         goto error ;
      }
   }
   rc = _send ( sdbCL->_sock, (MsgHeader *)sdbCL->_pSendBuffer,
               sdbCL->_endianConvert );
   if ( rc )
   {
      goto error;
   }
   rc = _recvExtract( sdbCL->_sock, (MsgHeader **)&sdbCL->_pReceiveBuffer,
                     &sdbCL->_receiveBufferSize, &contextID, &result,
                     sdbCL->_endianConvert );
   if ( rc )
   {
      goto error;
   }
   cursor = (sdbCursorStruct *) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) );
   if ( !cursor )
   {
      rc = SDB_OOM;
      goto error;
   }
   ossMemset( cursor, 0, sizeof(sdbCursorStruct) );
   cursor->_handleType = SDB_HANDLE_TYPE_CURSOR;
   cursor->_sock = sdbCL->_sock;
   cursor->_contextID = contextID;
   cursor->_offset = -1;
   cursor->_endianConvert = sdbCL->_endianConvert ;
   *handle = (sdbCursorHandle)cursor;
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbAttachCollection ( sdbCollectionHandle cHandle,
                             const CHAR *subClFullName,
                             bson *options )
{
   INT32 rc                      = SDB_OK ;
   BOOLEAN result                = FALSE ;
   SINT64 contextID              = 0 ;
   INT32 nameLength              = 0 ;
   bson newObj ;
   bson_iterator it ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( !subClFullName || !options ||
        (nameLength = ossStrlen ( subClFullName) ) >
         CLIENT_COLLECTION_NAMESZ ||
         cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, FIELD_NAME_NAME,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, FIELD_NAME_SUBCLNAME, subClFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_iterator_init ( &it, options ) ;
   while ( BSON_EOO != ( bson_iterator_next ( &it ) ) )
   {
      rc = bson_append_element ( &newObj, NULL, &it ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_LINK_CL,
                              0, 0, 0, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbDetachCollection( sdbCollectionHandle cHandle,
                              const CHAR *subClFullName)
{
   INT32 rc                      = SDB_OK ;
   BOOLEAN result                = FALSE ;
   SINT64 contextID              = 0 ;
   INT32 nameLength              = 0 ;
   bson newObj ;
   sdbCollectionStruct *cs = (sdbCollectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !cs ||
        cs->_handleType != SDB_HANDLE_TYPE_COLLECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   if ( !subClFullName ||
        (nameLength = ossStrlen ( subClFullName) ) >
         CLIENT_COLLECTION_NAMESZ ||
         cs->_collectionFullName[0] == '\0' )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, FIELD_NAME_NAME,
                             cs->_collectionFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_string ( &newObj, FIELD_NAME_SUBCLNAME, subClFullName ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &cs->_pSendBuffer, &cs->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_UNLINK_CL,
                              0, 0, 0, -1, &newObj,
                              NULL, NULL, NULL, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( cs->_sock, (MsgHeader*)cs->_pSendBuffer, cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( cs->_sock, (MsgHeader**)&cs->_pReceiveBuffer,
                       &cs->_receiveBufferSize, &contextID, &result,
                       cs->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbBackupOffline ( sdbConnectionHandle cHandle,
                bson *options)
{
   INT32 rc                      = SDB_OK ;
   BOOLEAN result                = FALSE ;
   SINT64 contextID              = 0 ;
//   const CHAR *key               = NULL ;
   bson newObj ;
   bson_iterator it ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // options is optional
   if ( options )
   {
      bson_iterator_init ( &it, options ) ;
      while ( BSON_EOO != bson_iterator_next ( &it ) )
      {
/*
         key = bson_iterator_key ( &it ) ;
         if ( ossStrcmp ( key, FIELD_NAME_GROUPNAME ) &&
              ossStrcmp ( key, FIELD_NAME_NAME ) &&
              ossStrcmp ( key, FIELD_NAME_PATH ) &&
              ossStrcmp ( key, FIELD_NAME_DESP ) &&
              ossStrcmp ( key, FIELD_NAME_ENSURE_INC ) &&
              ossStrcmp ( key, FIELD_NAME_OVERWRITE ) )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
*/
         rc = bson_append_element ( &newObj, NULL, &it ) ;
         if ( rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
   }
   rc = bson_finish ( &newObj ) ;
   if ( rc )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer, &connection->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_BACKUP_OFFLINE,
                              0, 0, 0, -1, &newObj,
                              NULL, NULL, NULL, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbListBackup ( sdbConnectionHandle cHandle,
                              bson *options,
                              bson *condition,
                              bson *selector,
                              bson *orderBy,
                              sdbCursorHandle *handle )
{
   INT32 rc                      = SDB_OK ;
   BOOLEAN result                = FALSE ;
   SINT64 contextID              = 0 ;
   sdbCursorStruct *cursor         = NULL ;
   bson newObj ;
   bson_iterator it ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // options is optional
   if ( options )
   {
      bson_iterator_init ( &it, options ) ;
      while ( BSON_EOO != bson_iterator_next ( &it ) )
      {
         bson_append_element ( &newObj, NULL, &it ) ;
      }
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer, &connection->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_LIST_BACKUP,
                              0, 0, 0, -1, condition,
                              selector, orderBy, &newObj, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock = connection->_sock ;
   cursor->_contextID = contextID ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset = -1 ;
   cursor->_endianConvert = connection->_endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbRemoveBackup ( sdbConnectionHandle cHandle,
                                   bson* options )
{
   INT32 rc                      = SDB_OK ;
   BOOLEAN result                = FALSE ;
   SINT64 contextID              = 0 ;
   bson newObj ;
   bson_iterator it ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   bson_init ( &newObj ) ;

   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // options is optional
   if ( options )
   {
      bson_iterator_init ( &it, options ) ;
      while ( BSON_EOO != bson_iterator_next ( &it ) )
      {
         bson_append_element ( &newObj, NULL, &it ) ;
      }
   }
   bson_finish ( &newObj ) ;
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer, &connection->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_REMOVE_BACKUP,
                              0, 0, 0, -1, &newObj,
                              NULL, NULL, NULL, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbListTasks ( sdbConnectionHandle cHandle,
                              bson *condition,
                              bson *selector,
                              bson *orderBy,
                              bson *hint,
                              sdbCursorHandle *handle )
{
   INT32 rc                      = SDB_OK ;
   BOOLEAN result                = FALSE ;
   SINT64 contextID              = 0 ;
   sdbCursorStruct *cursor         = NULL ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*)cHandle ;
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer, &connection->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_LIST_TASK,
                              0, 0, 0, -1, condition,
                              selector, orderBy, hint, connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   cursor = (sdbCursorStruct*) SDB_OSS_MALLOC ( sizeof(sdbCursorStruct) ) ;
   if ( !cursor )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cursor, 0, sizeof(sdbCursorStruct) ) ;
   cursor->_handleType = SDB_HANDLE_TYPE_CURSOR ;
   cursor->_sock = connection->_sock ;
   cursor->_contextID = contextID ;
//   cursor->_isDeleteCurrent = FALSE ;
   cursor->_offset = -1 ;
   cursor->_endianConvert = connection->_endianConvert ;
   *handle = (sdbCursorHandle)cursor ;
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbWaitTasks ( sdbConnectionHandle cHandle,
                                const SINT64 *taskIDs,
                                SINT32 num )
{
   INT32 rc                      = SDB_OK ;
   BOOLEAN result                = FALSE ;
   SINT64 contextID              = 0 ;
   SINT32 i                      = 0 ;
   bson newObj ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*) cHandle ;
   bson_init ( &newObj ) ;
   // check handle
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // check argument
   if ( !taskIDs || num < 0 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // append argument
   rc = bson_append_start_object ( &newObj, FIELD_NAME_TASKID ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_start_array ( &newObj, "$in" ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   for ( i = 0; i < num; i++ )
   {
      rc = bson_append_long ( &newObj, "", taskIDs[i] ) ;
      if ( rc )
      {
         rc = SDB_SYS ;
         goto error ;
      }
   }
   rc = bson_append_finish_array ( &newObj ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_finish_object ( &newObj ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_finish ( &newObj ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   // build msg
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer, &connection->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_WAITTASK,
                              0, 0, 0, -1,
                              &newObj, NULL, NULL, NULL,
                              connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   // send to engine
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   // receive and extract
   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbCancelTask ( sdbConnectionHandle cHandle,
                                SINT64 taskID,
                                BOOLEAN isAsync )
{
   INT32 rc                      = SDB_OK ;
   BOOLEAN result                = FALSE ;
   SINT64 contextID              = 0 ;
   bson newObj ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*) cHandle ;
   bson_init ( &newObj ) ;
   // check handle
   if ( !connection ||
        connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // check argument
   if ( taskID <= 0 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // append argument
   rc = bson_append_long ( &newObj, FIELD_NAME_TASKID, taskID ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_append_bool ( &newObj, FIELD_NAME_ASYNC, isAsync ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   rc = bson_finish ( &newObj ) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   // build msg
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer, &connection->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_CANCEL_TASK,
                              0, 0, 0, -1,
                              &newObj, NULL, NULL, NULL,
                              connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   // send to engine
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   // receive and extract
   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   bson_destroy ( &newObj ) ;
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 sdbSetSessionAttr ( sdbConnectionHandle cHandle,
                                     bson *options )
{
   INT32 rc              = SDB_OK ;
   BOOLEAN result        = FALSE ;
   SINT64 contextID      = 0 ;
   const CHAR *key       = NULL ;
   INT32 value           = PREFER_REPL_TYPE_MAX ;
   const CHAR *str_value = NULL ;
   INT32 int_value       = 0 ;
   bson newObj ;
   bson_iterator it ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*) cHandle ;
   if ( !connection || connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   // check handle
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // check argument
   if ( options == NULL )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // build obj
   bson_init ( &newObj ) ;
   bson_iterator_init ( &it, options ) ;
   while ( BSON_EOO != bson_iterator_next( &it ) )
   {
      // get key
      key = bson_iterator_key( &it ) ;
      // get value
      if ( strcmp( FIELD_NAME_PREFERED_REPLICA, key ) == 0 )
      {
         switch ( bson_iterator_type( &it ) )
         {
            case BSON_STRING :
               str_value = bson_iterator_string ( &it ) ;
               if ( strcmp( "M", str_value ) == 0 ) // master
                  value = PREFER_REPL_MASTER ;
               else if ( strcmp( "S", str_value ) == 0 ) // slave
                  value = PREFER_REPL_SLAVE ;
               else if ( strcmp( "A", str_value ) == 0 ) // anyone
                  value = PREFER_REPL_ANYONE ;
               else
               {
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               break ;
            case BSON_INT :
               int_value = bson_iterator_int ( &it ) ;
               if ( 1 <= int_value && int_value <= 7 )
                  value = int_value ;
               else
               {
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               break ;
            default :
               rc = SDB_INVALIDARG ;
               goto error ;
         }
         // append element
         rc = bson_append_int ( &newObj, key, value ) ;
         if ( rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         break ;
      }
   }
   rc = bson_finish ( &newObj ) ;
   if ( rc )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientBuildQueryMsg ( &connection->_pSendBuffer, &connection->_sendBufferSize,
                              CMD_ADMIN_PREFIX CMD_NAME_SETSESS_ATTR,
                              0, 0, 0, -1,
                              &newObj, NULL, NULL, NULL,
                              connection->_endianConvert) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

SDB_EXPORT INT32 _sdbMsg ( sdbConnectionHandle cHandle, const CHAR *msg )
{
   INT32 rc              = SDB_OK ;
   BOOLEAN result        = FALSE ;
   SINT64 contextID      = 0 ;
   sdbConnectionStruct *connection = (sdbConnectionStruct*) cHandle ;
   if ( !connection || connection->_handleType != SDB_HANDLE_TYPE_CONNECTION )
   // check handle
   {
      rc = SDB_CLT_INVALID_HANDLE ;
      goto error ;
   }
   // check argument
   if ( !msg )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   rc = clientBuildTestMsg ( &connection->_pSendBuffer, &connection->_sendBufferSize,
                              msg, 0, connection->_endianConvert) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _send ( connection->_sock, (MsgHeader*)connection->_pSendBuffer,
                connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
   rc = _recvExtract ( connection->_sock, (MsgHeader**)&connection->_pReceiveBuffer,
                       &connection->_receiveBufferSize, &contextID, &result,
                       connection->_endianConvert ) ;
   if ( rc )
   {
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}
