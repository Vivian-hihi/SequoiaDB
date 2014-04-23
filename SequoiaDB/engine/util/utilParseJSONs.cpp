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

   Source File Name = utilParseJSONs.cpp

   Descriptive Name =

   When/how to use: parse Jsons util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/05/2013  JW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "pd.hpp"
#include "ossMem.hpp"
#include "ossUtil.hpp"
#include "utilParseData.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"

PD_TRACE_DECLARE_FUNCTION ( SDB__UTILJSONPS__INIT, "_utilJSONParser::initialize" )
INT32 _utilJSONParser::initialize ( _utilParserParamet *parserPara )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__UTILJSONPS__INIT );
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
   _bufferSize  = parserPara->bufferSize ;
   _blockNum    = parserPara->blockNum ;
   _blockSize   = _bufferSize / _blockNum ;
   _accessModel = parserPara->accessModel ;
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
done:
   PD_TRACE_EXITRC ( SDB__UTILJSONPS__INIT, rc );
   return rc ;
error:
   goto done ;
}

CHAR *_utilJSONParser::getBuffer()
{
   return _buffer ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__UTILJSONPS__GETNEXTRECORD, "_utilJSONParser::getNextRecord" )
INT32 _utilJSONParser::getNextRecord ( UINT32 &startOffset,
                                       UINT32 &size,
                                       UINT32 *line,
                                       UINT32 *column,
                                       _bucket **ppBucket )
{
   INT32       rc             = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__UTILJSONPS__GETNEXTRECORD );
   UINT32      blockSize      = 0      ;
   UINT32       level          = 0      ;
   BOOLEAN     isESC          = FALSE  ;
   BOOLEAN     isString       = FALSE  ;
   BOOLEAN     isRecordFirst  = TRUE   ;
   CHAR        *pCursor       = NULL   ;
   CHAR        *curBuffer     = NULL   ;
   blockSize = _blockSize ;
   pCursor = _curBuffer ;

   do
   {
      if ( 0 == _unreadSpace )
      {
         if ( _pBlock >= _blockNum )
         {
            UINT32 recordLeftSize = 0 ;
            blockSize = _blockSize ;
            _pBlock = 0 ;
            if ( ppBucket )
            {
               ppBucket[_pBlock]->wait_to_get_exclusive_lock() ;
            }
            ossMemset ( _buffer, 0, _blockSize ) ;
            recordLeftSize = pCursor - _curBuffer ;
            if ( recordLeftSize >= ( _blockNum * _blockSize ) )
            {
               recordLeftSize = 0 ;
               PD_LOG ( PDWARNING, "Data size larger than the bucket size,\
clear bucket data" ) ;
            }
            else if ( recordLeftSize > 0 )
            {
               ossMemmove ( _buffer, _curBuffer, recordLeftSize ) ;
            }
            curBuffer = _buffer + recordLeftSize ;
            _curBuffer = _buffer ;
            pCursor = curBuffer ;
            while ( recordLeftSize > blockSize )
            {
               recordLeftSize -= blockSize ;
               ++_pBlock ;
               if ( ppBucket )
               {
                  ppBucket[_pBlock]->wait_to_get_exclusive_lock() ;
               }
            }
            blockSize -= recordLeftSize ;
         }
         else
         {
            blockSize = _blockSize ;
            curBuffer = _buffer + _pBlock * _blockSize ;
            if ( ppBucket )
            {
               ppBucket[_pBlock]->wait_to_get_exclusive_lock() ;
            }
            ossMemset ( curBuffer, 0, _blockSize ) ;
         }
         rc = _pAccessData->readNextBuffer ( curBuffer, blockSize ) ;
         if ( rc )
         {
            if ( rc == SDB_EOF && !blockSize )
            {
               if ( 0 < level )
               {
                  rc = SDB_OK ;
                  break ;
               }
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
         _unreadSpace = blockSize ;
         ++_pBlock ;
         if ( _unreadSpace == 0 )
         {
            continue ;
         }
         if ( _lackLF )
         {
            while ( *pCursor != '{' )
            {
               if ( !_unreadSpace )
               {
                  rc = SDB_UTIL_PARSE_JSON_INVALID ;
                  PD_LOG ( PDERROR, "Failed to read next buffer rc = %d", rc ) ;
                  goto error ;
               }
               if ( *pCursor == '\n' )
               {
                  ++_line ;
                  _column = 1 ;
               }
               if ( *pCursor != '\r' &&
                    *pCursor != '\n' &&
                    *pCursor != '\t' &&
                    *pCursor != 32 &&
                    *pCursor != _delRecord[0] )
               {
                   rc = SDB_UTIL_PARSE_JSON_INVALID ;
                   PD_LOG ( PDERROR, "Failed json" ) ;
                   goto error ;
               }
               ++pCursor ;
               --_unreadSpace ;
            }
            _lackLF = FALSE ;
         }
      }
      if ( isESC )
      {
         isESC = FALSE ;
      }
      else
      {
         if ( isRecordFirst &&
              '{' == *pCursor )
         {
            isRecordFirst = FALSE ;
         }
         switch ( *pCursor )
         {
         case '{':
         case '[':
            if ( !isString )
            {
               ++level ;
            }
            break ;
         case '}':
         case ']':
            if ( !isString )
            {
               --level ;
            }
            break ;
         case '\"':
            if ( !isRecordFirst )
            {
               if ( isString )
               {
                  isString = FALSE ;
               }
               else
               {
                  isString = TRUE ;
               }
            }
            break ;
         case '\\':
            if ( !isRecordFirst )
            {
               isESC = TRUE ;
            }
            break ;
         default:
            break ;
         }
      }
      --_unreadSpace ;
      ++pCursor ;
      if ( _linePriority &&
           _delRecord[0] == *pCursor )
      {
         break ;
      }
   }while ( level > 0 || isRecordFirst ) ;
   if ( line )
   {
      *line = _line ;
   }
   if ( column )
   {
      *column = _column ;
   }
   _column += size ;
   startOffset = _curBuffer - _buffer ;
   size   = pCursor - _curBuffer ;

   do
   {
      if ( !_unreadSpace )
      {
         _lackLF = TRUE ;
         break ;
      }
      if ( *pCursor == '\n' )
      {
         ++_line ;
         _column = 1 ;
      }
      if ( *pCursor == '{' )
      {
         break ;
      }
      if ( *pCursor != '\r' &&
           *pCursor != '\n' &&
           *pCursor != '\t' &&
           *pCursor != 32 &&
           *pCursor != _delRecord[0] )
      {
         rc = SDB_UTIL_PARSE_JSON_INVALID ;
         if ( line )
         {
            *line = _line ;
         }
         if ( column )
         {
            *column = _column ;
         }
         PD_LOG ( PDERROR, "Failed json" ) ;
         goto error ;
      }
      ++pCursor ;
      --_unreadSpace ;
   }while ( *pCursor != '{' ) ;
   _curBuffer =  pCursor ;
done:
   PD_TRACE_EXITRC ( SDB__UTILJSONPS__GETNEXTRECORD, rc );
   return rc ;
error:
   goto done ;
}

_utilJSONParser::_utilJSONParser() : _curBuffer(NULL),
                                     _pBlock(0),
                                     _unreadSpace(0),
                                     _lackLF(FALSE)
{
}

_utilJSONParser::~_utilJSONParser()
{
}
