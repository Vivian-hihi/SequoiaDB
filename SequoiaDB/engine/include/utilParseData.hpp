/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = utilParseData.hpp

   Descriptive Name =

   When/how to use: parse Data util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/05/2013  JW  Initial Draft

   Last Changed =

******************************************************************************/

#ifndef UTIL_PARSE_DATA_HPP_
#define UTIL_PARSE_DATA_HPP_

#include "core.hpp"
#include "ossLatch.hpp"
#include "utilAccessData.hpp"

#include <vector>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

//32MB buffer size must be multiple of 33554432
#define UTIL_DATA_BUFFER_SIZE 33554432
#define UTIL_DATA_HEADER_SIZE 1024

class _bucket : public SDBObject
{
private:
   boost::mutex _mutex ;
   boost::condition_variable condVar ;
   INT32 lockCounter ;
public:
   void wait_to_get_exclusive_lock() ;
   void inc() ;
   void dec() ;
   _bucket() ;
} ;

struct _fieldBuf : public SDBObject
{
   CHAR *buffer ;
   CHAR *fieldBuf ;
} ;

enum SDB_UTIL_ACCESS_MODEL
{
   UTIL_GET_IO = 0,
   UTIL_GET_HDFS
} ;

struct _utilParserParamet : public SDBObject
{
   UINT32                  bufferSize ;
   UINT32                  blockNum ;
   BOOLEAN                 readHeader ;
   BOOLEAN                 autoAddField ;
   BOOLEAN                 autoCompletion ;
   BOOLEAN                 linePriority ;
   SDB_UTIL_ACCESS_MODEL   accessModel ;
   UINT16                  port ;
   const CHAR             *fileName ;
   const CHAR             *path ;
   const CHAR             *hostName ;
   const CHAR             *user ;
   CHAR                   *headerBuffer ;

   _utilParserParamet() : bufferSize(0),
                          blockNum(0),
                          readHeader(FALSE),
                          autoAddField(TRUE),
                          autoCompletion(FALSE),
                          linePriority(TRUE),
                          accessModel(UTIL_GET_IO),
                          port(0),
                          fileName(NULL),
                          path(NULL),
                          hostName(NULL),
                          user(NULL),
                          headerBuffer(NULL)
   {
   }
} ;

#define UTL_PARSER_CSV_CHAR_SPACE_NUM 4

class _utilDataParser : public SDBObject
{
protected:
   UINT32                  _bufferSize   ;
   UINT32                  _line         ;
   UINT32                  _column       ;
   UINT32                  _blockNum     ;
   UINT32                  _blockSize    ;
   BOOLEAN                 _headerline   ;
   BOOLEAN                 _linePriority ;
   SDB_UTIL_ACCESS_MODEL   _accessModel  ;
   CHAR                   *_buffer       ;
   utilAccessData         *_pAccessData  ;
   CHAR                    _charSpace[UTL_PARSER_CSV_CHAR_SPACE_NUM] ;
public:
   CHAR        _delChar[2]   ;
   CHAR        _delField[2]  ;
   CHAR        _delRecord[2] ;
   //field vector
   std::vector< _fieldBuf * > _vField ;
public:
   _utilDataParser() ;
   CHAR *_trimLeft ( CHAR *pCursor, UINT32 &size ) ;
   CHAR *_trimRight ( CHAR *pCursor, UINT32 &size ) ;
   BOOLEAN parse_number ( const CHAR *buffer, UINT32 size ) ;
   void mallocBufer ( UINT32 size ) ;
   void setDel ( CHAR delChar, CHAR delField, CHAR delRecord ) ;
   virtual ~_utilDataParser() ;

   virtual INT32 initialize ( _utilParserParamet *parserPara ) = 0 ;
   virtual INT32 getNextRecord ( UINT32 &startOffset,
                                 UINT32 &size,
                                 UINT32 *line = NULL,
                                 UINT32 *column = NULL,
                                 _bucket **ppBucket = NULL ) = 0 ;
   virtual CHAR *getBuffer() = 0 ;
} ;

class _utilJSONParser : public _utilDataParser
{
private :
   CHAR       *_curBuffer   ;
   UINT32      _pBlock      ;
   UINT32      _unreadSpace ;
   BOOLEAN     _lackLF      ;
public:
   virtual INT32 initialize ( _utilParserParamet *parserPara ) ;
   virtual INT32 getNextRecord ( UINT32 &startOffset,
                                 UINT32 &size,
                                 UINT32 *line = NULL,
                                 UINT32 *column = NULL,
                                 _bucket **ppBucket = NULL ) ;
   virtual CHAR *getBuffer() ;
   _utilJSONParser() ;
   ~_utilJSONParser() ;
} ;

class _utilCSVParser : public _utilDataParser
{
private :
   //record cursor
   CHAR       *_curBuffer    ;
   //record cursor in block number
   UINT32      _pBlock       ;
   //not read buffer size
   UINT32      _unreadSpace  ;

   //buffer sum size
   UINT32      _fieldSize   ;
   //read buffer size
   UINT32      _readNumStr   ;
   //not read buffer size
   UINT32      _readFreeSpace ;
   //field buffer
   CHAR       *_fieldBuffer ;
   //field cursor
   CHAR       *_leftField ;
   //next field cursor
   CHAR       *_nextFieldCursor ;
   // is string ?
   BOOLEAN     _isString ;
   // delChar number
   INT32       _delCharNum ;
   // not auto add filed
private:
   INT32 _findDelField () ;
   INT32 _allocField ( UINT32 size ) ;
   INT32 _readFirstField ( ) ;
public:
   virtual INT32 initialize ( _utilParserParamet *parserPara ) ;
   virtual INT32 getNextRecord ( UINT32 &startOffset,
                                 UINT32 &size,
                                 UINT32 *line = NULL,
                                 UINT32 *column = NULL,
                                 _bucket **ppBucket = NULL ) ;
   virtual CHAR *getBuffer() ;
   _utilCSVParser() ;
   ~_utilCSVParser() ;
} ;


#define UTL_WORKER_FIELD_SIZE 1024

class _convertCSV : public SDBObject
{
private:
   CHAR      *_pJsonBuffer ;
   UINT32     _jsonBufSize ;
   UINT32     _jsonBufFreeSpace ;
   _utilDataParser *_parser ;
   CHAR       _fieldName[UTL_WORKER_FIELD_SIZE] ;
private:
   INT32 _transferredCSV ( CHAR *buffer, UINT32 size ) ;
   INT32 _jsonBufAppend ( CHAR *buffer, UINT32 size ) ;
   INT32 _allocJsonBuffer() ;
public:
   _convertCSV() ;
   ~_convertCSV() ;
   INT32 _convertCSVToJson ( CHAR *pBuffer,
                             UINT32 size,
                             BOOLEAN autoAddField,
                             BOOLEAN autoCompletion,
                             _utilDataParser *parser,
                             CHAR **pOut ) ;
} ;

#endif