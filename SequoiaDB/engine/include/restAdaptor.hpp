#ifndef RESTADAPTOR_HPP__
#define RESTADAPTOR_HPP__
#include "core.hpp"
#include "oss.hpp"
#include "msg.hpp"
#include "../bson/bson.h"
#include "msgMessage.hpp"
#include "pmdSession.hpp"
#include "restdefine.hpp"
#include <map>

//recv and send timeout
#define REST_TIMEOUT  1000

namespace engine
{
   enum HTTP_PARSE_COMMON
   {
      COM_INSERT = 0,
      COM_DELETE,
      COM_UPDATE,
      COM_QUERY,
      COM_SQL,
      COM_LOGIN,
      COM_GETFILE
   } ;

   class restAdaptor : public SDBObject
   {
   private:
      UINT32 _maxHttpHeaderSize ;
      UINT32 _maxHttpBodySize ;
      UINT32 _timeout ;
      void *_pSettings ;
   private:
      static INT32 on_message_begin( void *pData ) ;
      static INT32 on_headers_complete( void *pData ) ;
      static INT32 on_message_complete( void *pData ) ;
      static INT32 on_url( void *pData, const CHAR* at, size_t length ) ;
      static INT32 on_header_field( void *pData, const CHAR* at,
                                    size_t length ) ;
      static INT32 on_header_value( void *pData, const CHAR* at,
                                    size_t length ) ;
      static INT32 on_body( void *pData, const CHAR* at,
                            size_t length ) ;
      static INT32 _parse_http_query( httpConnection *pHttpConnection,
                                      CHAR *pBuffer, INT32 length ) ;
      inline const CHAR *_getResourceFileName( const CHAR *pPath ) ;
      inline const CHAR *_getFileExtension( const CHAR *pFileName ) ;
      INT32 _convertMsg( HTTP_PARSE_COMMON &common,
                         CHAR **pMsg,
                         UINT32 &msgSize ) ;
   public:
      restAdaptor() ;
      ~restAdaptor() ;
      INT32 init( UINT32 maxHttpHeaderSize,
                  UINT32 maxHttpBodySize,
                  UINT32 timeout = REST_TIMEOUT ) ;

      INT32 getRequestHeader( pmdSession *pSession ) ;
      INT32 getRequestBody( pmdSession *pSession,
                            HTTP_PARSE_COMMON &common,
                            CHAR **pMsg,
                            UINT32 &msgSize ) ;
      INT32 sendResponse( pmdSession *pSession ) ;

      INT32 appendHttpHeader( const CHAR *pKey,
                              const CHAR *pValue ) ;
      INT32 getHttpHeader( const CHAR *pKey,
                           const CHAR **pValue ) ;
      INT32 appendHttpBody( const CHAR *pBuffer,
                            UINT32 length,
                            UINT32 number ) ;
   } ;
}
#endif
