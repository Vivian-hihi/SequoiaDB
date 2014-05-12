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

   Source File Name = utilParseCSV.cpp

   Descriptive Name =

   When/how to use: parse Jsons util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2013  JW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "pd.hpp"
#include "ossMem.hpp"
#include "ossUtil.hpp"
#include "utilParseData.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"
#include "text.h"

#define UTL_WORKER_CTJSIZE    512
#define UTL_WORKER_FIELD      "field"
#define UTL_DEFAULT_DELCHAR   '"'
#define UTL_CHAR_ESC          '\\'

PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCSV__INIT, "_utilCSVParser::initialize" )
INT32 _utilCSVParser::initialize ( _utilParserParamet *parserPara )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__UTILCSV__INIT );
   //INT32 size = 0 ;
   //check error
   if ( parserPara->blockNum <= 0 )
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "blockNum must be big than 0!" ) ;
      goto error ;
   }

   if ( parserPara->bufferSize < UTIL_DATA_BUFFER_SIZE ||
        parserPara->bufferSize % parserPara->blockNum )
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "buffer space is too small 32MB!" ) ;
      goto error ;
   }

   _linePriority = parserPara->linePriority ;
   _bufferSize   = parserPara->bufferSize ;
   _blockNum     = parserPara->blockNum ;
   _blockSize    = _bufferSize / _blockNum ;
   _headerline   = parserPara->readHeader ;
   _accessModel  = parserPara->accessModel ;
   //_buffer
   //_buffer = (CHAR *)SDB_OSS_MALLOC ( _bufferSize ) ;
   mallocBufer ( _bufferSize ) ;
   if ( !_buffer )
   {
      rc = SDB_SYS ;
      PD_LOG ( PDERROR, "malloc error" ) ;
      goto error ;
   }
   _curBuffer = _buffer ;
   ossMemset ( _buffer, 0, _bufferSize ) ;
   //IO
   if ( UTIL_GET_IO == _accessModel )
   {
      utilAccessParametLocalIO accessData ;
      accessData.pFileName = parserPara->fileName ;
      _pAccessData = SDB_OSS_NEW utilAccessDataLocalIO() ;
      if ( !_pAccessData )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "malloc error" ) ;
         goto error ;
      }
      rc = _pAccessData->initialize( (void*)&accessData ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to init IO,rc = %d", rc ) ;
         goto error ;
      }
   }
   else if ( UTIL_GET_HDFS == _accessModel )
   {
      utilAccessParametHdfs accessData ;
      accessData.pFileName = parserPara->fileName ;
      accessData.pPath     = parserPara->path ;
      accessData.pHostName = parserPara->hostName ;
      accessData.pUser     = parserPara->user ;
      accessData.port      = parserPara->port ;
      _pAccessData = SDB_OSS_NEW utilAccessDataHdfs() ;
      if ( !_pAccessData )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "malloc error" ) ;
         goto error ;
      }
      rc = _pAccessData->initialize( (void*)&accessData ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to init IO,rc = %d", rc ) ;
         goto error ;
      }
   }
   else
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "Failed to access model,rc = %d", rc ) ;
      goto error ;
   }
   if ( !_headerline )
   {
      _fieldBuffer   = parserPara->headerBuffer ;
      _fieldSize     = ossStrlen ( _fieldBuffer ) ;
      _readFreeSpace = _fieldSize ;
   }
   /*rc = _readFirstField( ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to read header, rc = %d", rc ) ;
      goto error ;
   }*/

   for ( INT32 i = 0; i < UTL_PARSER_CSV_CHAR_SPACE_NUM; ++i )
   {
      if ( _charSpace[i] == _delChar[0] ||
           _charSpace[i] == _delField[0] ||
           _charSpace[i] == _delRecord[0] )
      {
         _charSpace[i] = 0 ;
      }
   }

   /*size = _vField.size() ;
   for ( INT32 i = 0; i < size; ++i )
   {
      PD_LOG ( PDERROR, "%s", _vField[i]->fieldBuf ) ;
   }*/
done:
   PD_TRACE_EXITRC ( SDB__UTILCSV__INIT, rc );
   return rc ;
error:
   goto done ;
}

CHAR *_utilCSVParser::getBuffer()
{
   return _buffer ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCSV__GETNEXTRECORD, "_utilCSVParser::getNextRecord" )
INT32 _utilCSVParser::getNextRecord ( UINT32 &startOffset,
                                      UINT32 &size,
                                      UINT32 *line,
                                      UINT32 *column,
                                      _bucket **ppBucket )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__UTILCSV__GETNEXTRECORD );
   BOOLEAN     isString       = FALSE ;
   UINT32      delCharNum     = 0 ;
   UINT32      newReadSize    = 0 ;
   UINT32      isReadSize     = 0 ;
   UINT32      useBlockNum    = 0 ;
   CHAR       *pCursor        = _curBuffer ;
   CHAR       *curBuffer      = NULL ;


   do
   {
      if ( 0 == _unreadSpace )
      {
         if ( _pBlock >= _blockNum )
         {
            isReadSize = pCursor - _curBuffer ;
            if ( isReadSize > _blockSize && isReadSize < _bufferSize )
            {
               //is read size use block number
               useBlockNum = ( (UINT32)( isReadSize / _blockSize ) ) + 1 ;
               while ( useBlockNum > 0 )
               {
                  if ( ppBucket )
                  {
                     ppBucket[_pBlock]->wait_to_get_exclusive_lock() ;
                  }
                  ++_pBlock ;
                  --useBlockNum ;
               }
               ossMemmove ( _buffer, _curBuffer, isReadSize ) ;
               newReadSize = isReadSize % _blockSize ;
               if ( newReadSize == 0 )
               {
                  ++_pBlock ;
                  continue ;
               }
               //ossMemset ( _buffer + isReadSize, 0, newReadSize ) ;
            }
            else
            {
               if ( isReadSize == _bufferSize )
               {
                  isReadSize = 0 ;
                  PD_LOG ( PDWARNING, "Data size larger than the bucket \
size %d, clear bucket data", _bufferSize ) ;
               }
               _pBlock = 0 ;
               if ( ppBucket )
               {
                  ppBucket[_pBlock]->wait_to_get_exclusive_lock() ;
               }
               ossMemmove ( _buffer, _curBuffer, isReadSize ) ;
               newReadSize = _blockSize - isReadSize ;
               if ( newReadSize == 0 )
               {
                  ++_pBlock ;
                  continue ;
               }
               //ossMemset ( _buffer + isReadSize, 0, newReadSize ) ;
            }
            curBuffer = _buffer + isReadSize ;
            pCursor = curBuffer ;
         }
         else
         {
            if ( ppBucket )
            {
               ppBucket[_pBlock]->wait_to_get_exclusive_lock() ;
            }
            newReadSize = _blockSize ;
            curBuffer = _buffer + _pBlock * _blockSize ;
            //ossMemset ( curBuffer, 0, _blockSize ) ;
         }
         rc = _pAccessData->readNextBuffer ( curBuffer, newReadSize ) ;
         if ( rc )
         {
            if ( rc == SDB_EOF && newReadSize == 0 )
            {
               startOffset = _curBuffer - _buffer ;
               size = pCursor - _curBuffer ;
               goto done ;
            }
            else if ( rc != SDB_EOF )
            {
               PD_LOG ( PDERROR, "Failed to read next buffer rc = %d", rc ) ;
               goto error ;
            }
            else
            {
               rc = SDB_OK ;
            }
         }
         _unreadSpace = newReadSize ;
         ++_pBlock ;
      }
      if ( _delChar[0] == *pCursor )
      {
         ++delCharNum ;
      }
      else if ( _delRecord[0] == *pCursor )
      {
         if ( delCharNum > 0 && delCharNum % 2 != 0 )
         {
            isString = !isString ;
         }
         delCharNum = 0 ;
         if ( !isString || _linePriority )
         {
            ++pCursor ;
            --_unreadSpace ;
            startOffset = _curBuffer - _buffer ;
            size = pCursor - _curBuffer ;
            _curBuffer = pCursor ;
            ++_line ;
            goto done ;
         }
      }
      else
      {
         if ( delCharNum > 0 && delCharNum % 2 != 0 )
         {
            isString = !isString ;
         }
         delCharNum = 0 ;
      }
      ++pCursor ;
      --_unreadSpace ;
   }while( TRUE ) ;
done:
   PD_TRACE_EXITRC ( SDB__UTILCSV__GETNEXTRECORD, rc );
   return rc ;
error:
   goto done ;
}

_utilCSVParser::_utilCSVParser() : _pBlock(0),
                                   _unreadSpace(0),
                                   _fieldSize(0),
                                   _readNumStr(0),
                                   _readFreeSpace(0),
                                   _leftFieldSize(0),
                                   _curBuffer(NULL),
                                   _fieldBuffer(NULL),
                                   _nextFieldCursor(NULL),
                                   _isString(FALSE),
                                   _delCharNum(0)
{
}

_utilCSVParser::~_utilCSVParser()
{
   if ( _headerline )
   {
      SAFE_OSS_FREE( _fieldBuffer ) ;
      _vField.clear() ;
   }
}



_convertCSV::_convertCSV( BOOLEAN stringType ) : _pJsonBuffer(NULL),
                                                 _jsonBufSize(0),
                                                 _jsonBufFreeSpace(0)
{
   _stringType = stringType ;
}
_convertCSV::~_convertCSV()
{
   SAFE_OSS_FREE ( _pJsonBuffer ) ;
}

#define JSON_BUF_APPEND( buffer, size ) \
{ \
   rc = _jsonBufAppend ( buffer, size ) ; \
   if ( rc ) \
   { \
      goto error ; \
   } \
}

#define JSON_BUF_CHECK_APPEND( buffer, size ) \
{ \
   rc = _transferredCSV ( buffer, size ) ; \
   if ( rc ) \
   { \
      goto error ; \
   } \
}

/*#define CREATE_TEMP_FIELD_NAME ( num ) \
{ \
   ossMemset ( _fieldName, 0, UTL_WORKER_FIELD_SIZE ) ; \
   ossSnprintf ( _fieldName, \
                 UTL_WORKER_FIELD_SIZE, \
                 UTL_WORKER_FIELD "%d", \
                 num ) ; \
}*/

PD_TRACE_DECLARE_FUNCTION ( SDB__UTILPARCSV__CTCSVTOJSON,"_convertCSV::_convertCSVToJson" )
INT32 _convertCSV::_convertCSVToJson ( CHAR *pBuffer, UINT32 size,
                                       BOOLEAN autoAddField,
                                       BOOLEAN autoCompletion,
                                       _utilDataParser *parser, CHAR **pOut )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__UTILPARCSV__CTCSVTOJSON );
   SDB_ASSERT ( pBuffer, "pBuffer is NULL" )
   SDB_ASSERT ( parser, "parser is NULL" )
   SDB_ASSERT ( pOut, "pOut is NULL" )
   _parser = parser ;
   CHAR    *pCursor    = pBuffer ;
   CHAR    *pTemp      = NULL ;
   //string read number
   UINT32   strLeft    = 0 ;
   //field start number
   UINT32   fieldLeft  = 0 ;
   //field sum
   INT32 fieldSumNum = _parser->_vField.size() ;
   //field number
   INT32    fieldNum   = 0 ;
   //auto add field number
   INT32    autoFieldNum = 1 ;
   
   BOOLEAN  isFirst    = TRUE ;

   BOOLEAN  isString   = FALSE ;
   INT32    tmpCharNum = 0 ;

   _jsonBufFreeSpace = _jsonBufSize ;
   ossMemset ( _pJsonBuffer, 0, _jsonBufSize ) ;

   JSON_BUF_APPEND ( "{", 1 ) ;

   while ( strLeft < size )
   {
      if ( _parser->_delChar[0] == *pCursor )
      {
         ++tmpCharNum ;
         ++strLeft ;
         ++pCursor ;
      }
      else if ( ( _parser-> _delField[0] == *pCursor ||
                  _parser-> _delRecord[0] == *pCursor ) )
      {
         if ( tmpCharNum > 0 && tmpCharNum % 2 != 0 )
         {
            tmpCharNum = 0 ;
            isString = !isString ;
         }
         if ( !isString )
         {
            if ( fieldNum < fieldSumNum )
            {
               if ( !isFirst )
               {
                  JSON_BUF_APPEND ( ",", 1 ) ;
               }
               pTemp = _parser->_vField[fieldNum] ;
               if ( pTemp )
               {
                  //JSON_BUF_APPEND ( pTemp,
                  //                  ossStrlen(pTemp) ) ;
                  JSON_BUF_CHECK_APPEND ( pTemp,
                                          ossStrlen(pTemp) ) ;
               }
               else
               {
                  if ( autoAddField )
                  {
                     ossMemset ( _fieldName, 0, UTL_WORKER_FIELD_SIZE ) ;
                     ossSnprintf ( _fieldName,
                                   UTL_WORKER_FIELD_SIZE,
                                   UTL_WORKER_FIELD "%d",
                                   autoFieldNum ) ;
                     JSON_BUF_APPEND ( _fieldName, ossStrlen ( _fieldName ) ) ;
                     ++autoFieldNum ;
                  }
                  else
                  {
                     //error
                     rc = SDB_UTIL_PARSE_JSON_INVALID ;
                     PD_LOG ( PDERROR, "CSV format error, this field is \
empty field, you can set \"--spare true\" to add the field, rc = %d", rc ) ;
                     goto error ;
                  }
               }
               JSON_BUF_APPEND ( ":", 1 ) ;
               JSON_BUF_CHECK_APPEND ( pBuffer + fieldLeft,
                                       strLeft - fieldLeft ) ;
            }
            else
            {
               //if field list less than field number, then auto add field
               if ( autoAddField )
               {
                  if ( !isFirst )
                  {
                     JSON_BUF_APPEND ( ",", 1 ) ;
                  }
                  ossMemset ( _fieldName, 0, UTL_WORKER_FIELD_SIZE ) ;
                  ossSnprintf ( _fieldName,
                                UTL_WORKER_FIELD_SIZE,
                                UTL_WORKER_FIELD "%d",
                                autoFieldNum ) ;
                  ++autoFieldNum ;
                  JSON_BUF_APPEND ( _fieldName, ossStrlen ( _fieldName ) ) ;
                  JSON_BUF_APPEND ( ":", 1 ) ;
                  JSON_BUF_APPEND ( "null", 4 ) ;
               }
            }

            if ( isFirst )
            {
               isFirst = !isFirst ;
            }

            ++fieldNum ;
            if ( _parser-> _delRecord[0] == *pCursor )
            {
               break ;
            }
            else
            {
               ++strLeft ;
               ++pCursor ;
               fieldLeft = strLeft ;
            }
         }
         else
         {
            if ( tmpCharNum > 0 && tmpCharNum % 2 != 0 )
            {
               isString = !isString ;
            }
            tmpCharNum = 0 ;
            ++strLeft ;
            ++pCursor ;
         }
      }
      else
      {
         if ( tmpCharNum > 0 && tmpCharNum % 2 != 0 )
         {
            isString = !isString ;
         }
         tmpCharNum = 0 ;
         ++strLeft ;
         ++pCursor ;
      }

      if ( strLeft >= size )
      {
         tmpCharNum = 0 ;

         if ( fieldNum < fieldSumNum )
         {
            if ( !isFirst )
            {
               JSON_BUF_APPEND ( ",", 1 ) ;
            }
            pTemp = _parser->_vField[fieldNum] ;
            if ( pTemp )
            {
               //JSON_BUF_APPEND ( pTemp,
               //                  ossStrlen(pTemp) ) ;
               JSON_BUF_CHECK_APPEND( pTemp,
                                      ossStrlen(pTemp) ) ;
            }
            else
            {
               if ( autoAddField )
               {
                  ossMemset ( _fieldName, 0, UTL_WORKER_FIELD_SIZE ) ;
                  ossSnprintf ( _fieldName,
                                UTL_WORKER_FIELD_SIZE,
                                UTL_WORKER_FIELD "%d",
                                autoFieldNum ) ;
                  JSON_BUF_APPEND ( _fieldName, ossStrlen ( _fieldName ) ) ;
                  ++autoFieldNum ;
               }
               else
               {
                  //error
                  rc = SDB_UTIL_PARSE_JSON_INVALID ;
                  PD_LOG ( PDERROR, "CSV format error, this field is \
empty field, you can set \"--spare true\" to add the field, rc = %d", rc ) ;
                  goto error ;
               }
            }
            JSON_BUF_APPEND ( ":", 1 ) ;
            JSON_BUF_CHECK_APPEND ( pBuffer + fieldLeft,
                                    strLeft - fieldLeft ) ;
         }
         else
         {
            if ( autoAddField )
            {
               if ( !isFirst )
               {
                  JSON_BUF_APPEND ( ",", 1 ) ;
               }
               ossMemset ( _fieldName, 0, UTL_WORKER_FIELD_SIZE ) ;
               ossSnprintf ( _fieldName,
                             UTL_WORKER_FIELD_SIZE,
                             UTL_WORKER_FIELD "%d",
                             autoFieldNum ) ;
               ++autoFieldNum ;
               JSON_BUF_APPEND ( _fieldName, ossStrlen ( _fieldName ) ) ;
               JSON_BUF_APPEND ( ":", 1 ) ;
               JSON_BUF_APPEND ( "null", 4 ) ;
            }
         }
         
         if ( isFirst )
         {
            isFirst = !isFirst ;
         }
      }
   }//while
   
   if ( autoCompletion )
   {
      for ( ; fieldNum < fieldSumNum; ++fieldNum )
      {
         if ( !isFirst )
         {
            JSON_BUF_APPEND ( ",", 1 ) ;
         }
         pTemp = _parser->_vField[fieldNum] ;
         if ( pTemp )
         {
            //JSON_BUF_APPEND ( pTemp,
            //                  ossStrlen(pTemp) ) ;
            JSON_BUF_CHECK_APPEND ( pTemp,
                                    ossStrlen(pTemp) ) ;
         }
         else
         {
            if ( autoAddField )
            {
               ossMemset ( _fieldName, 0, UTL_WORKER_FIELD_SIZE ) ;
               ossSnprintf ( _fieldName,
                             UTL_WORKER_FIELD_SIZE,
                             UTL_WORKER_FIELD "%d",
                             autoFieldNum ) ;
               JSON_BUF_APPEND ( _fieldName, ossStrlen ( _fieldName ) ) ;
               ++autoFieldNum ;
            }
            else
            {
               //error
               rc = SDB_UTIL_PARSE_JSON_INVALID ;
               PD_LOG ( PDERROR, "CSV format error, this field is \
empty field, you can set \"--spare true\" to add the field, rc = %d", rc ) ;
               goto error ;
            }
         }
         JSON_BUF_APPEND ( ":", 1 ) ;
         JSON_BUF_APPEND ( "null", 4 ) ;
      }
   }
   JSON_BUF_APPEND ( "}", 1 ) ;
done:
   *pOut = _pJsonBuffer ;
   PD_TRACE_EXITRC ( SDB__UTILPARCSV__CTCSVTOJSON, rc );
   return rc ;
error:
   goto done ;
}

INT32 _convertCSV::_transferredCSV ( CHAR *buffer, UINT32 size )
{
   INT32 rc = SDB_OK ;
   BOOLEAN  isString = FALSE ;

   buffer = _parser->_trimLeft ( buffer, size ) ;
   if ( !buffer )
   {
      JSON_BUF_APPEND ( "null", 4 ) ;
      goto done ;
   }
   _parser->_trimRight ( buffer + size - 1, size ) ;
   if ( 0 == size )
   {
      JSON_BUF_APPEND ( "null", 4 ) ;
      goto done ;
   }

   //is string "xxxxxx"
   if ( _parser->_delChar[0] == *buffer &&
        _parser->_delChar[0] == *(buffer + size - 1) )
   {
      ++buffer ;
      size -= 2 ;
      isString = TRUE ;
   }
   //is string xxxxx"
   else if ( _parser->_delChar[0] != *buffer &&
             _parser->_delChar[0] == *(buffer + size - 1) )
   {
      isString = TRUE ;
   }
   //not string  xxxxx
   else if ( _parser->_delChar[0] != *buffer &&
             _parser->_delChar[0] != *(buffer + size - 1) )
   {
      //is number?
      if ( size == 4 &&
           buffer[0] == 't' &&
           buffer[1] == 'r' &&
           buffer[2] == 'u' &&
           buffer[3] == 'e' )
      {
         isString = FALSE ;
      }
      else if ( size == 4 &&
                buffer[0] == 'n' &&
                buffer[1] == 'u' &&
                buffer[2] == 'l' &&
                buffer[3] == 'l' )
      {
         isString = FALSE ;
      }
      else if ( size == 5 &&
                buffer[0] == 'f' &&
                buffer[1] == 'a' &&
                buffer[2] == 'l' &&
                buffer[3] == 's' &&
                buffer[4] == 'e' )
      {
         isString = FALSE ;
      }
      else
      {
         isString = !_parser->parse_number ( buffer, size ) ;
      }
   }
   else
   {
      //is error csv
      rc = SDB_UTIL_PARSE_JSON_INVALID ;
      PD_LOG ( PDERROR, "CSV format error, only one side of the field appears \
delChar, rc = %d", rc ) ;
      goto error ;
   }

   if ( isString || _stringType )
   {
      JSON_BUF_APPEND ( "\"", 1 ) ;
      for ( UINT32 i = 0; i < size; ++i )
      {
         // xxx"xxxx
         if ( _parser->_delChar[0] == *(buffer + i) )
         {
            ++i ;
            // xx""xxx
            if ( i < size && _parser->_delChar[0] == *(buffer + i) )
            {
               if ( _parser->_delChar[0] == '"' )
               {
                  JSON_BUF_APPEND ( "\\\"", 2 ) ;
               }
               else
               {
                  JSON_BUF_APPEND ( (buffer + i), 1 ) ;
               }
            }
            else
            {
               //error csv, not double delChar
               rc = SDB_UTIL_PARSE_JSON_INVALID ;
               PD_LOG ( PDERROR, "CSV format error, field appears delChar, \
must double delChar, rc = %d", rc ) ;
               goto error ;
            }
         }
         else
         {
            switch( *(buffer + i) )
            {
            case '\b':
               JSON_BUF_APPEND ( "\\\b", 2 ) ;
               break ;
            case '\f':
               JSON_BUF_APPEND ( "\\\f", 2 ) ;
               break ;
            case '\n':
               JSON_BUF_APPEND ( "\\\n", 2 ) ;
               break ;
            case '\r':
               JSON_BUF_APPEND ( "\\\r", 2 ) ;
               break ;
            case '\t':
               JSON_BUF_APPEND ( "\\\t", 2 ) ;
               break ;
            case '\\':
               JSON_BUF_APPEND ( "\\\\", 2 ) ;
               break ;
            case '"':
               JSON_BUF_APPEND ( "\\\"", 2 ) ;
               break ;
            default:
               JSON_BUF_APPEND ( buffer + i, 1 ) ;
               break ;
            }
         }
      }
      JSON_BUF_APPEND ( "\"", 1 ) ;
   }
   //is number, true, false, null
   else
   {
     JSON_BUF_APPEND ( buffer, size ) ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 _convertCSV::_jsonBufAppend ( CHAR *buffer, UINT32 size )
{
   INT32 rc = SDB_OK ;
   while ( _jsonBufFreeSpace < size )
   {
      rc = _allocJsonBuffer() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to call allocJsonBuffer, rc = %d",
                  rc ) ;
         goto error ;
      }
   }
   ossStrncpy ( _pJsonBuffer + ( _jsonBufSize - _jsonBufFreeSpace ),
                buffer, size ) ;
   _jsonBufFreeSpace -= size ;
done:
   return rc ;
error:
   goto done ;
}

INT32 _convertCSV::_allocJsonBuffer()
{
   INT32  rc      = SDB_OK ;
   CHAR  *newBuf  = NULL ;

   _jsonBufSize += UTL_WORKER_CTJSIZE ;
   _jsonBufFreeSpace += UTL_WORKER_CTJSIZE ;
   //free in ~migWorker()
   newBuf = (CHAR *)SDB_OSS_REALLOC ( _pJsonBuffer,
                                      _jsonBufSize ) ;
   if ( !newBuf )
   {
      rc = SDB_OOM ;
      PD_LOG ( PDERROR, "Unable to allocate %d bytes memory",
               _jsonBufSize ) ;
      goto error ;
   }
   _pJsonBuffer = newBuf ;
   ossMemset ( _pJsonBuffer + ( _jsonBufSize - _jsonBufFreeSpace ),
               0, _jsonBufFreeSpace ) ;
done:
   return rc ;
error:
   goto done ;
}
