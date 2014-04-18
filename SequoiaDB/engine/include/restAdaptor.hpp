#ifndef RESTADAPTOR_HPP__
#define RESTADAPTOR_HPP__
#include "core.hpp"
#include "oss.hpp"
#include "msg.hpp"
#include "../bson/bson.h"
#include "msgMessage.hpp"
#include "pmdRestSession.hpp"
#include "restDefine.hpp"
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
      INT32 _maxHttpHeaderSize ;
      INT32 _maxHttpBodySize ;
      INT32 _timeout ;
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
      OSS_INLINE const CHAR *_getResourceFileName( const CHAR *pPath ) ;
      OSS_INLINE const CHAR *_getFileExtension( const CHAR *pFileName ) ;
      INT32 _convertMsg( HTTP_PARSE_COMMON &common,
                         CHAR **pMsg,
                         INT32 &msgSize ) ;
   public:
      restAdaptor() ;
      ~restAdaptor() ;
      INT32 init( INT32 maxHttpHeaderSize,
                  INT32 maxHttpBodySize,
                  INT32 timeout = REST_TIMEOUT ) ;

      INT32 getRequestHeader( pmdRestSession *pSession ) ;
      INT32 getRequestBody( pmdRestSession *pSession,
                            HTTP_PARSE_COMMON &common,
                            CHAR **pMsg,
                            INT32 &msgSize ) ;
      INT32 sendResponse( pmdRestSession *pSession ) ;

      INT32 appendHttpHeader( pmdRestSession *pSession,
                              const CHAR *pKey,
                              const CHAR *pValue ) ;
      INT32 getHttpHeader( pmdRestSession *pSession,
                           const CHAR *pKey,
                           const CHAR **pValue ) ;
      INT32 appendHttpBody( pmdRestSession *pSession,
                            const CHAR *pBuffer,
                            INT32 length,
                            INT32 number ) ;
   } ;
}
#endif
