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
   class _restConvertMsg : public SDBObject
   {
   public:
      INT32 buildInsertMsg ( CHAR **ppBuffer,
                             INT32 *pBufferSize,
                             const CHAR *pCollectionName,
                             const CHAR *pInsertor ) ;
      INT32 buildQueryMsg( CHAR **ppBuffer,
                           INT32 *pBufferSize,
                           const CHAR *pCollectionName,
                           SINT64 numToSkip,
                           SINT64 numToReturn,
                           const CHAR *pCondition,
                           const CHAR *pSelector,
                           const CHAR *pOrderby,
                           const CHAR *pHint ) ;
   } ;

   class restAdaptor : public SDBObject
   {
   private:
      INT32 _maxHttpHeaderSize ;
      INT32 _maxHttpBodySize ;
      INT32 _timeout ;
      void *_pSettings ;
      _restConvertMsg _convertObj ;
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
      OSS_INLINE BOOLEAN _checkEndOfHeader( httpConnection *pHttpCon,
                                            CHAR *pBuffer,
                                            INT32 bufferSize,
                                            INT32 &bodyOffset ) ;
      INT32 _switchMsg( httpConnection *pHttpCon,
                        HTTP_PARSE_COMMON common,
                        CHAR **ppMsg,
                        INT32 &msgSize ) ;
      INT32 _convertMsg( pmdRestSession *pSession,
                         HTTP_PARSE_COMMON &common,
                         CHAR **ppMsg,
                         INT32 &msgSize ) ;
      void _getQuery( httpConnection *pHttpCon,
                      const CHAR *pKey,
                      const CHAR **ppValue ) ;
      INT32 _query2Msg( httpConnection *pHttpCon,
                        HTTP_PARSE_COMMON &common,
                        CHAR **ppMsg,
                        INT32 &msgSize ) ;
      INT32 _getStringLen( httpConnection *pHttpCon,
                           const CHAR *pBuff ) ;
      void _paraInit( httpConnection *pHttpCon ) ;
   public:
      restAdaptor() ;
      ~restAdaptor() ;
      INT32 init( INT32 maxHttpHeaderSize,
                  INT32 maxHttpBodySize,
                  INT32 timeout = REST_TIMEOUT ) ;

      INT32 getRequestHeader( pmdRestSession *pSession ) ;
      INT32 getRequestBody( pmdRestSession *pSession,
                            HTTP_PARSE_COMMON &common,
                            CHAR **ppMsg,
                            INT32 &msgSize ) ;
      INT32 setOPResult( pmdRestSession *pSession,
                         INT32 result,
                         const BSONObj &info ) ;
      INT32 sendResponse( pmdRestSession *pSession,
                          HTTP_RESPONSE_CODE rspCode ) ;

      INT32 appendHttpHeader( pmdRestSession *pSession,
                              const CHAR *pKey,
                              const CHAR *pValue ) ;
      INT32 getHttpHeader( pmdRestSession *pSession,
                           const CHAR *pKey,
                           const CHAR **ppValue ) ;
      INT32 appendHttpBody( pmdRestSession *pSession,
                            const CHAR *pBuffer,
                            INT32 length,
                            INT32 number ) ;
   } ;
}
#endif
