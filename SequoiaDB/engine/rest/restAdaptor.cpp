#include "core.hpp"
#include "restAdaptor.hpp"
#include "http_parser.hpp"
#include "ossMem.h"
#include "ossUtil.h"
#include "../util/fromjson.hpp"
#include "pdTrace.hpp"
#include "restTrace.hpp"

namespace engine
{
   INT32 restAdaptor::on_message_begin( void *pData )
   {
      return 0 ;
   }

   INT32 restAdaptor::on_headers_complete( void *pData )
   {
      return 0 ;
   }

   INT32 restAdaptor::on_message_complete( void *pData )
   {
      http_parser *pParser = (http_parser *)pData ;
      httpConnection *pHttpConnection = (httpConnection *)pParser->data ;
      pHttpConnection->_recvComplete = TRUE ;
      return 0 ;
   }

   INT32 restAdaptor::on_url( void *pData,
                              const CHAR* at, size_t length )
   {
      http_parser *pParser = (http_parser *)pData ;
      httpConnection *pHttpConnection = (httpConnection *)pParser->data ;
      INT32 i = 0 ;
      CHAR *pPath = NULL ;

      pHttpConnection->_pPath = at ;
      for( ; i < length && at[i] != '?'; ++i ) ;

      pPath = pHttpConnection->_pRecvBuffer +
         ( at - pHttpConnection->_pRecvBuffer ) ;
      pPath[i] = 0 ;

      printf( "path: %s\n", pHttpConnection->_pPath ) ;

      if( i + 1 < length )
      {
         ++i ;
         _parse_http_query( pHttpConnection,
                            pPath + i, length - i ) ;
      }
      return 0 ;
   }

   INT32 restAdaptor::on_header_field( void *pData,
                                       const CHAR* at, size_t length )
   {
      http_parser *pParser = (http_parser *)pData ;
      httpConnection *pHttpConnection = (httpConnection *)pParser->data ;
      /*
      if ( pHttpConnection->_isKey )
      {
         _requestHeader httpHeader ;
         httpHeader.length  = length ;
         httpHeader.pBuffer = at ;
         pHttpConnection->_requestKey.push_back( httpHeader ) ;
         pHttpConnection->_isKey = FALSE ;
      }
      else
      {
         INT32 vectorLen = pHttpConnection->_requestKey.size() ;
         if( vectorLen <= 0 )
         {
            return 1 ;
         }
         pHttpConnection->_requestKey[vectorLen-1].length += length ;
      }*/
      return 0 ;
   }

   INT32 restAdaptor::on_header_value( void *pData,
                                       const CHAR* at, size_t length )
   {
      http_parser *pParser = (http_parser *)pData ;
      httpConnection *pHttpConnection = (httpConnection *)pParser->data ;
      /*
      if ( !pHttpConnection->_isKey )
      {
         _requestHeader httpHeader ;
         httpHeader.length  = length ;
         httpHeader.pBuffer = at ;
         pHttpConnection->_requestValue.push_back( httpHeader ) ;
         pHttpConnection->_isKey = TRUE ;
      }
      else
      {
         INT32 vectorLen = pHttpConnection->_requestValue.size() ;
         if( vectorLen <= 0 )
         {
            return 1 ;
         }
         pHttpConnection->_requestValue[vectorLen-1].length += length ;
      }*/
      return 0 ;
   }

   INT32 restAdaptor::on_body( void *pData,
                               const CHAR* at, size_t length )
   {
      http_parser *pParser = (http_parser *)pData ;
      httpConnection *pHttpConnection = (httpConnection *)pParser->data ;
      /*
      INT32 i = length ;
      CHAR *pPostQuery = NULL ;

      pPostQuery = pHttpConnection->_pRecvBuffer +
            ( at - pHttpConnection->_pRecvBuffer ) ;

      printf( "pPostQuery: %s\n", pPostQuery ) ;

      _parse_http_query( pHttpConnection,
                         pPostQuery, length ) ;
      */
      return 0 ;
   }

   restAdaptor::restAdaptor() : _maxHttpHeaderSize(0),
                                _maxHttpBodySize(0),
                                _timeout(0),
                                _pSettings(NULL)
   {
   }

   restAdaptor::~restAdaptor()
   {
      if ( _pSettings )
      {
         SDB_OSS_FREE( _pSettings ) ;
         _pSettings = NULL ;
      }
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__RESTADP_PARQUERY, "restAdaptor::_parse_http_query" )
   INT32 restAdaptor::_parse_http_query( httpConnection *pHttpConnection,
                                         CHAR *pBuffer, INT32 length )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RESTADP_PARQUERY ) ;
      INT32 keyOffset   = 0 ;
      INT32 valueOffset = 0 ;

      for ( INT32 i = 0; i < length; ++i )
      {
         if ( pBuffer[i] == '=' )
         {
            pBuffer[i] = 0 ;
            valueOffset = i + 1 ;
            continue ;
         }
         else if ( pBuffer[i] == '&' || ( i + 1 == length ) )
         {
            if ( i + 1 == length )
            {
               pBuffer[i+1] = 0 ;
            }
            else
            {
               pBuffer[i] = 0 ;
            }

            printf("%s = %s\n", pBuffer + keyOffset, pBuffer + valueOffset ) ;
            pHttpConnection->_requestQuery.insert(
                  std::make_pair(pBuffer + keyOffset, pBuffer + valueOffset) ) ;
            keyOffset = i + 1 ;
            continue ;
         }
      }
   done:
      PD_TRACE_EXITRC ( SDB__RESTADP_PARQUERY, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   OSS_INLINE const CHAR *restAdaptor::_getResourceFileName( const CHAR *pPath )
   {
      INT32 pathLen = ossStrlen( pPath ) ;
      for ( INT32 i = pathLen - 1; i >= 0; --i )
      {
         if( pPath[i] == '/' )
         {
            if ( i + 1  >= pathLen )
            {
               return NULL ;
            }
            else
            {
               return ( pPath + i + 1 ) ;
            }
         }
      }
      return NULL ;
   }

   OSS_INLINE const CHAR *restAdaptor::_getFileExtension(
         const CHAR *pFileName )
   {
      INT32 fileNameLen = ossStrlen( pFileName ) ;
      for ( INT32 i = fileNameLen - 1; i >= 0; --i )
      {
         if( pFileName[i] == '.' )
         {
            if ( i + 1  >= fileNameLen )
            {
               return NULL ;
            }
            else
            {
               return ( pFileName + i + 1 ) ;
            }
         }
      }
      return NULL ;
   }

   INT32 restAdaptor::_convertMsg( HTTP_PARSE_COMMON &common,
                                   CHAR **pMsg,
                                   INT32 &msgSize )
   {
      INT32 rc = SDB_OK ;
      const CHAR *pFileName = NULL ;
      const CHAR *pExtension = NULL ;
      INT32 fileNameSize = 0 ;
      /*
      pFileName = _getResourceFileName( _pHttpConnection->_pPath ) ;
      if ( pFileName )
      {
         //there are resource file
         pExtension = _getFileExtension( pFileName ) ;
         if ( pExtension )
         {
            //get pFileName's file
            common = COM_GETFILE ;
         }
         else
         {
            //get sequoiadb resource
            if ( _pHttpConnection->_requestQuery.size() > 0 )
            {
            }
            else
            {
            }
         }
      }
      else
      {
         if ( _pHttpConnection->_requestQuery.size() > 0 )
         {
            //run common
         }
         else
         {
            //get default file
         }
      }
      */
   done:
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__RESTADP_INIT, "restAdaptor::init" )
   INT32 restAdaptor::init( INT32 maxHttpHeaderSize,
                            INT32 maxHttpBodySize,
                            INT32 timeout )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RESTADP_INIT );

      http_parser_settings *pSettings = NULL ;

      pSettings = (http_parser_settings *)SDB_OSS_MALLOC(
            sizeof( http_parser_settings ) ) ;
      if ( !pSettings )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "Unable to allocate %d bytes memory",
                  sizeof( http_parser_settings ) ) ;
         goto error ;
      }

      ossMemset( pSettings, 0, sizeof( http_parser_settings ) ) ;
      pSettings->on_message_begin    = restAdaptor::on_message_begin ;
      pSettings->on_url              = restAdaptor::on_url ;
      pSettings->on_header_field     = restAdaptor::on_header_field ;
      pSettings->on_header_value     = restAdaptor::on_header_value ;
      pSettings->on_headers_complete = restAdaptor::on_headers_complete ;
      pSettings->on_body             = restAdaptor::on_body ;
      pSettings->on_message_complete = restAdaptor::on_message_complete ;

      _maxHttpHeaderSize = maxHttpHeaderSize ;
      _maxHttpBodySize = maxHttpHeaderSize ;
      _timeout = timeout ;
      _pSettings = pSettings ;
   done:
      PD_TRACE_EXITRC ( SDB__RESTADP_INIT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__RESTADP_GETREQHE, "restAdaptor::getRequestHeader" )
   INT32 restAdaptor::getRequestHeader( pmdRestSession *pSession )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RESTADP_GETREQHE ) ;
      SDB_ASSERT ( pSession, "pSession is NULL" )
      //httpConnection *pHttpCon = pSession->getRestConn() ;
      //CHAR *pBuffer = pSession->getFixBuff() ;
      //INT32 bufSize = pSession->getFixBuffSize() ;
/*
      http_parser_init( pParser, HTTP_REQUEST ) ;

      while( true )
      {
         rc = pSocket->recv( pRecvBuffer + receivedSize,
                             REST_ONCE_RECV_SIZE,
                             curRecvSize,
                             timeout,
                             0,
                             FALSE ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to recv, rc=%d", rc ) ;
            goto error ;
         }

         if ( receivedSize + curRecvSize > maxRecvSize )
         {
            rc = SDB_REST_RECV_SIZE ;
            PD_LOG ( PDERROR, "Recv size greater than max recv size" ) ;
            goto error ;
         }

         http_parser_execute( pParser, (http_parser_settings *)_pSettings,
                              pRecvBuffer + receivedSize, curRecvSize ) ;
         receivedSize += curRecvSize ;
         if ( _pHttpConnection->_recvComplete )
         {
            break ;
         }
      }
*/
   done:
      PD_TRACE_EXITRC( SDB__RESTADP_GETREQHE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__RESTADP_GETREQBO, "restAdaptor::getRequestBody" )
   INT32 restAdaptor::getRequestBody( pmdRestSession *pSession,
                                      HTTP_PARSE_COMMON &common,
                                      CHAR **pMsg,
                                      INT32 &msgSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RESTADP_GETREQBO ) ;
      SDB_ASSERT ( pSession, "pSession is NULL" )
/*
      http_parser *pParser     = (http_parser *)_pHttpConnection->_pHttpParser ;
      ossSocket   *pSocket     = _pHttpConnection-> _pSocket ;
      CHAR        *pRecvBuffer = _pHttpConnection->_pRecvBuffer ;
      SDB_ASSERT ( pParser,     "pParser is NULL" )
      SDB_ASSERT ( pSocket,     "pSocket is NULL" )
      SDB_ASSERT ( pRecvBuffer, "pRecvBuffer is NULL" )

      INT32 timeout      = (INT32)_pHttpConnection->_timeout ;
      INT32 maxRecvSize  = (INT32)_pHttpConnection->_maxHttpSize ;
      INT32 curRecvSize  = 0 ;
      INT32 receivedSize = 0 ;

      //init
      ossMemset ( pRecvBuffer, 0, maxRecvSize ) ;
      _pHttpConnection->_recvComplete  = FALSE ;
      _pHttpConnection->_isKey         = TRUE ;
      _pHttpConnection->_requestKey.clear() ;
      _pHttpConnection->_requestValue.clear() ;

      http_parser_init( pParser, HTTP_REQUEST ) ;

      while( true )
      {
         rc = pSocket->recv( pRecvBuffer + receivedSize,
                             REST_ONCE_RECV_SIZE,
                             curRecvSize,
                             timeout,
                             0,
                             FALSE ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to recv, rc=%d", rc ) ;
            goto error ;
         }

         if ( receivedSize + curRecvSize > maxRecvSize )
         {
            rc = SDB_REST_RECV_SIZE ;
            PD_LOG ( PDERROR, "Recv size greater than max recv size" ) ;
            goto error ;
         }

         http_parser_execute( pParser, (http_parser_settings *)_pSettings,
                              pRecvBuffer + receivedSize, curRecvSize ) ;
         receivedSize += curRecvSize ;
         if ( _pHttpConnection->_recvComplete )
         {
            break ;
         }
      }

      for ( INT32 i = 0; i < _pHttpConnection->_requestKey.size(); ++i )
      {
         printf( "%.*s : %.*s \n",
                 _pHttpConnection->_requestKey[i].length,
                 _pHttpConnection->_requestKey[i].pBuffer,
                 _pHttpConnection->_requestValue[i].length,
                 _pHttpConnection->_requestValue[i].pBuffer ) ;
      }
*/
   done:
      PD_TRACE_EXITRC( SDB__RESTADP_GETREQBO, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__RESTADP_SENDRE, "restAdaptor::sendResponse" )
   INT32 restAdaptor::sendResponse( pmdRestSession *pSession )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RESTADP_SENDRE ) ;
   done:
      PD_TRACE_EXITRC( SDB__RESTADP_SENDRE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__RESTADP_APPENDHEADER, "restAdaptor::appendHttpHeader" )
   INT32 restAdaptor::appendHttpHeader( pmdRestSession *pSession,
                                        const CHAR *pKey,
                                        const CHAR *pValue )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RESTADP_APPENDHEADER ) ;
   done:
      PD_TRACE_EXITRC( SDB__RESTADP_APPENDHEADER, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__RESTADP_GETHEADER, "restAdaptor::getHttpHeader" )
   INT32 restAdaptor::getHttpHeader( pmdRestSession *pSession,
                                     const CHAR *pKey,
                                     const CHAR **pValue )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RESTADP_GETHEADER ) ;
      //httpConnection *pHttpCon = pSession->getRestConn() ;

   done:
      PD_TRACE_EXITRC( SDB__RESTADP_GETHEADER, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__RESTADP_APPENDBODY, "restAdaptor::appendHttpBody" )
   INT32 restAdaptor::appendHttpBody( pmdRestSession *pSession,
                                      const CHAR *pBuffer,
                                      INT32 length,
                                      INT32 number )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RESTADP_APPENDBODY ) ;
   done:
      PD_TRACE_EXITRC( SDB__RESTADP_APPENDBODY, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}
