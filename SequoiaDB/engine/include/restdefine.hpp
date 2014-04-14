/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = restdefine.h

   Descriptive Name =

   When/how to use: parse Jsons util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2014  JW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef REST_DEFINE_H
#define REST_DEFINE_H

#include "core.hpp"
#include <sys/types.h>
#if defined(_WIN32) && !defined(__MINGW32__) && (!defined(_MSC_VER) || _MSC_VER<1600)
#include <BaseTsd.h>
#include <stddef.h>
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
#include <map>


struct http_parser {
  /** PRIVATE **/
  unsigned int type : 2;         /* enum http_parser_type */
  unsigned int flags : 6;        /* F_* values from 'flags' enum; semi-public */
  unsigned int state : 8;        /* enum state from http_parser.c */
  unsigned int header_state : 8; /* enum header_state from http_parser.c */
  unsigned int index : 8;        /* index into current matcher */

  uint32_t nread;          /* # bytes read in various scenarios */
  uint64_t content_length; /* # bytes in body (0 if no Content-Length header) */

  /** READ-ONLY **/
  unsigned short http_major;
  unsigned short http_minor;
  unsigned int status_code : 16; /* responses only */
  unsigned int method : 8;       /* requests only */
  unsigned int http_errno : 7;

  /* 1 = Upgrade header was present and the parser has exited because of that.
   * 0 = No upgrade header present.
   * Should be checked when http_parser_execute() returns in addition to
   * error checking.
   */
  unsigned int upgrade : 1;

  /** PUBLIC **/
  void *data; /* A pointer to get hook to the "connection" or "socket" object */
};

struct httpConnection
{
   //Max http recv size
   UINT32 _maxHttpSize ;
   //recv and send timeout
   UINT32 _timeout ;
   //flag headers complete
   BOOLEAN _recvHeaderComplete ;
   //flag recv complete
   BOOLEAN _recvComplete ;
   //flag is parser key or value, true: key, false: value
   BOOLEAN _isKey ;
   //recv buffer
   CHAR *_pRecvBuffer ;
   //send buffer
   CHAR *_pSendBuffer ;
   //path
   const CHAR *_pPath ;

   //http parser
   http_parser _httpParser ;
   //header list
   std::map<CHAR *,CHAR *> _requestHeaders ;
   //query list
   std::map<CHAR *,CHAR *> _requestQuery ;

   httpConnection() : _maxHttpSize(0),
                      _timeout(0),
                      _recvHeaderComplete(FALSE),
                      _recvComplete(FALSE),
                      _isKey(TRUE),
                      _pRecvBuffer(NULL),
                      _pSendBuffer(NULL),
                      _pPath(NULL)
   {
   }
} ;

#endif
