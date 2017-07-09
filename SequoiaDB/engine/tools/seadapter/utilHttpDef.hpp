#ifndef UTIL_HTTP_DEF_HPP_
#define UTIL_HTTP_DEF_HPP_

// Maximum size of the http header and body.
// Note: HTTP_MAX_HEADER_SIZE should be defined before including http_parser.hpp
//       with exactly this name. It will be used in http_parser.
//       If not defined here, the default value of 80k defined in http_parser
//       itself will be used.
#define HTTP_MAX_HEADER_SIZE        ( 64 * 1024 )
#define HTTP_MAX_BODY_SIZE          ( 64 * 1024 * 1024 - HTTP_MAX_HEADER_SIZE )

#define _TEXT_PLAIN              "text/plain"
#define _APPLICATION_JSON        "application/json"
#define _APPLICATION_URLENCODED  "application/x-www-form-urlencoded"

#define HTTP_URI_PREFIX          "http://"
#define HTTP_LINE_END_STR        "\r\n"
#define HTTP_BLOCK_END_STR       "\r\n\r\n"
#define HTTP_BLOCK_END_STR_LEN   ossStrlen( HTTP_BLOCK_END_STR )
#define HTTP_URI_PREFIX_LEN      ossStrlen( HTTP_URI_PREFIX )
#define HTTP_DEFAULT_PORT        80
#define HTTP_REQ_TIMEOUT         ( 10 * 1000 )
#define HTTP_REQ_HEAD_STR        "HEAD"
#define HTTP_REQ_GET_STR         "GET"
#define HTTP_REQ_PUT_STR         "PUT"
#define HTTP_REQ_POST_STR        "POST"
#define HTTP_REQ_DELETE_STR      "DELETE"

   // Default send and receive buffer size: 1MB.
#define HTTP_DEF_BUF_SIZE        ( 1024 * 1024 )

#endif /* UTIL_HTTP_DEF_HPP_ */

