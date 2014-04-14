#ifndef RESTADAPTOR_HPP__
#define RESTADAPTOR_HPP__
#include "core.hpp"
#include "oss.hpp"
#include "ossSocket.hpp"
#include "msg.hpp"
#include "../bson/bson.h"
#include "msgMessage.hpp"
#include "pmdSession.hpp"
#include <vector>
#include <map>

//recv once size 1KB
#define REST_ONCE_RECV_SIZE 100
//http size 2MB
#define REST_MAX_HTTP_SIZE 2097152
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
      struct _requestHeader
      {
         //string size
         UINT32 length ;
         const CHAR *pBuffer ;
      } ;

      struct _responseHeader
      {
         const CHAR *pKey ;
         const CHAR *pValue ;
      } ;

      struct _httpConnection : public SDBObject
      {
         //Max http recv size
         UINT32 _maxHttpSize ;
         //recv and send timeout
         UINT32 _timeout ;
         //flag recv complete
         BOOLEAN _recvComplete ;
         //flag is parser key or value, true: key, false: value
         BOOLEAN _isKey ;
         //recv buffer
         CHAR *_pRecvBuffer ;
         //send buffer
         CHAR *_pSendBuffer ;
         //socket
         ossSocket *_pSocket ;
         //http parser
         void *_pHttpParser ;
         //path
         const CHAR *_pPath ;

         vector<_requestHeader> _requestKey ;
         vector<_requestHeader> _requestValue ;
         map<CHAR *,CHAR *> _requestQuery ;
         vector<_responseHeader> _responseField ;
      } ;

      _httpConnection *_pHttpConnection ;
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
      static INT32 _parse_http_query( _httpConnection *pHttpConnection,
                                      CHAR *pBuffer, INT32 length ) ;
      inline const CHAR *_getResourceFileName( const CHAR *pPath ) ;
      inline const CHAR *_getFileExtension( const CHAR *pFileName ) ;
      INT32 _convertMsg( HTTP_PARSE_COMMON &common,
                         CHAR **pMsg,
                         UINT32 &msgSize ) ;
   public:
      restAdaptor() ;
      ~restAdaptor() ;
      INT32 init( ossSocket *socket,
                  UINT32 maxHttpSize = REST_MAX_HTTP_SIZE,
                  UINT32 timeout = REST_TIMEOUT ) ;

      INT32 getRequest( HTTP_PARSE_COMMON &common,
                        CHAR **pMsg,
                        UINT32 &msgSize ) ;
      INT32 sendResponse() ;

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
