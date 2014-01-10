/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = migImport.cpp

   Descriptive Name = Migration Import Implementation

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains implementation for import
   operation

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/06/2013  HTL  Initial Draft

   Last Changed =

*******************************************************************************/

#include "migImport.hpp"
#include "ossMem.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "migTrace.hpp"
#include "../util/json2rawbson.h"
#include "../util/text.h"

_migParser::_migParser() : _parser(NULL),
                           _collection(NULL)
{
   _delChar[0]     = MIG_DEFAULT_DELCHAR ;
   _delChar[1]     = 0 ;
   _delField[0]    = MIG_DEFAULT_DELFIELD ;
   _delField[1]    = 0 ;
   _delRecord[0]   = MIG_DEFAULT_DELRECORD ;
   _delRecord[1]   = 0 ;
}

_migParser::~_migParser()
{
   SAFE_OSS_DELETE ( _parser ) ;
   /*
   if ( _isOpened )
   {
      ossClose ( _file ) ;
   }
   if ( _pReadBuffer )
   {
      SDB_OSS_FREE ( _pReadBuffer ) ;
   }
   if ( _pJsonBuffer )
   {
      SDB_OSS_FREE ( _pJsonBuffer ) ;
   }
   */
}
/*
static inline std::string &ltrim ( std::string &s )
{
   s.erase ( s.begin(), std::find_if ( s.begin(), s.end(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace)))) ;
   return s ;
}

static inline std::string &rtrim ( std::string &s )
{
   s.erase ( std::find_if ( s.rbegin(), s.rend(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace))).base(),
             s.end() ) ;
   return s ;
}

static inline std::string &trim ( std::string &s )
{
   return ltrim ( rtrim ( s ) ) ;
}

// format hex string to delimiter
PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER__HEXSTR2DEL, "_migParser::_hexStrToDel" )
BOOLEAN _migParser::_hexStrToDel ( const CHAR *pDelStr,
                                   CHAR &del )
{
   PD_TRACE_ENTRY ( SDB__MIGPARSER__HEXSTR2DEL );
   INT32 strLength = 0 ;
   const CHAR *pCurStr = pDelStr ;
   BOOLEAN success = FALSE ;
   UINT32 value = 0 ;
   // at least we need to include '0x' and another byte
   if ( !pDelStr || ( strLength = ossStrlen ( pDelStr ) ) < 3 ||
        pDelStr[0] != '0' || pDelStr[1] != 'x' )
   {
      goto error ;
   }
   // first two char must be 0x
   pCurStr = &pDelStr[2] ;
   while ( *pCurStr != '\0' )
   {
      value *= 16 ;
      // we accept 0x000000AB, but we need to make sure the value must be less
      // than 255, otherwise the delimiter doesn't make sense
      if ( value > 0xFF )
         break ;
      CHAR curChar = *pCurStr ;
      if ( ( curChar >= '0' && curChar <= '9' ) )
      {
         value += curChar - '0' ;
      }
      else if ( curChar >= 'a' && curChar <= 'f' )
      {
         value += curChar - 'a' + 10 ;
      }
      else if ( curChar >= 'A' && curChar <= 'F' )
      {
         value += curChar - 'A' + 10 ;
      }
      else
      {
         break ;
      }
      ++pCurStr ;
   }
   // we consider it's success only when all characters are parsed
   if ( *pCurStr == '\0' )
   {
      success = TRUE ;
      del = (CHAR)value ;
   }
done :
   PD_TRACE_EXIT ( SDB__MIGPARSER__HEXSTR2DEL );
   return success ;
error :
   goto done ;
}

// convert string to delimiter
PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER__STR2DEL, "_migParser::_strToDel" )
BOOLEAN _migParser::_strToDel ( const CHAR *pDelStr,
                                   CHAR &del )
{
   PD_TRACE_ENTRY ( SDB__MIGPARSER__STR2DEL );
   INT32 strLength = 0 ;
   BOOLEAN success = FALSE ;
   if ( !pDelStr || ( strLength = ossStrlen ( pDelStr ) ) == 0 )
   {
      goto error ;
   }
   // check how many chars in buffer
   if ( strLength == 1 )
   {
      // if only provided one char, then that's it
      del = pDelStr[0] ;
      success = TRUE ;
   }
   else if ( pDelStr[0] == '\'' &&
             pDelStr[strLength] == '\'' )
   {
      // if we are in single quote
      if ( strLength == 3 )
      {
         // if there's a single char like 'x', then that's it
         del = pDelStr[1] ;
         success = TRUE ;
      }
      else if ( strLength == 4 && pDelStr[1] == '\\' )
      {
         // if there are two char like '\n', then we need to check
         if ( pDelStr[2] == 'n' )
         {
            del = '\n' ;
            success = TRUE ;
         }
         else if ( pDelStr[2] == 't' )
         {
            del = '\t' ;
            success = TRUE ;
         }
      }
   }
   else
   {
      // if we were provided as 0xYY
      if ( strLength >= 3 &&
           pDelStr[0] == '0' &&
           pDelStr[1] == 'x' )

      {
         success = _hexStrToDel ( pDelStr, del ) ;
      }
   }
done :
   PD_TRACE_EXIT ( SDB__MIGPARSER__STR2DEL );
   return success ;
error :
   goto done ;
}
*/
PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER_RUN, "_migParser::run" )
INT32 _migParser::run ( INT32 &total, INT32 &succeed )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGPARSER_RUN );
   INT32 count = 0, succ = 0;
   /*if ( !_init )
   {
      PD_LOG ( PDERROR, "Failed: initialize required!" ) ;
      rc = SDB_SYS ;
      goto error ;
   }*/
   while ( TRUE )
   {
      bson bsonObj ;
      bson_init ( &bsonObj ) ;
      rc = _getRecord ( bsonObj ) ;
      if ( rc == SDB_EOF )
      {
         bson_destroy ( &bsonObj ) ;
         //PD_LOG ( PDEVENT, "Import Successfully" ) ;
         rc = SDB_OK ;
         break ;
      }
      else if ( rc == SDB_UTIL_PARSE_JSON_INVALID )
      {
         bson_destroy ( &bsonObj ) ;
         count++ ;
         PD_LOG ( PDERROR, "Bad record in %d", count ) ;
         continue ;
      }
      else if ( rc )
      {
         bson_destroy ( &bsonObj ) ;
         PD_LOG ( PDERROR, "Import Failed, rc = %d", rc ) ;
         goto error ;
      }
      if ( bsonObj.dataSize <= 5 )
      {
         bson_destroy ( &bsonObj ) ;
         // empty bson
         continue ;
      }
      count++ ;
      bson *obj = &bsonObj ;
      rc = _importRecord ( &obj, 1 ) ;
      bson_destroy ( &bsonObj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to import record in %d", count ) ;
         continue ;
      }
      succ++ ;
   }
   total = count ;
   succeed = succ ;
done :
   PD_TRACE_EXITRC ( SDB__MIGPARSER_RUN, rc );
   return rc ;
error :
   goto done ;
}
/*
PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER__REALLOCRB, "_migParser::_reallocReadBuf" )
INT32 _migParser::_reallocReadBuf ( UINT32 requiredSize )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGPARSER__REALLOCRB );
   // make sure we don't allocate too much, round up to 4MB
   requiredSize = ossRoundUpToMultipleX ( requiredSize,
                                          MIG_INC_READ_BUFFER ) ;
   if ( requiredSize > MIG_MAX_READ_BUFFER )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   if ( requiredSize > _bufSize )
   {
      UINT32 diff = _bufParsed () ;
      // if we need more memory, try to realloc
      CHAR *pOld = _pReadBuffer ;
      _pReadBuffer = (CHAR*)SDB_OSS_REALLOC ( _pReadBuffer, requiredSize ) ;
      if ( !_pReadBuffer )
      {
         rc = SDB_OOM ;
         _pReadBuffer = pOld ;
         goto error ;
      }
      _pCurPtr = _pReadBuffer + diff ;
      _bufSize = requiredSize ;
   }
done :
   PD_TRACE_EXITRC ( SDB__MIGPARSER__REALLOCRB, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER__REALLOCJB, "_migParser::_reallocJsonBuf" )
INT32 _migParser::_reallocJsonBuf ( UINT32 requiredSize )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGPARSER__REALLOCJB );
   // make sure we don't allocate too much, round up to 4MB
   requiredSize = ossRoundUpToMultipleX ( requiredSize,
                                          MIG_INC_READ_BUFFER ) ;
   if ( requiredSize > MIG_MAX_READ_BUFFER )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   if ( requiredSize > _jsonSize )
   {
      UINT32 diff = _jsonBufOccupied () ;
      // if we need more memory, try to realloc
      CHAR *pOld = _pJsonBuffer ;
      _pJsonBuffer = (CHAR*)SDB_OSS_REALLOC ( _pJsonBuffer, requiredSize ) ;
      if ( !_pJsonBuffer )
      {
         rc = SDB_OOM ;
         _pJsonBuffer = pOld ;
         goto error ;
      }
      _pJsonCurPtr = _pJsonBuffer + diff ;
      _jsonSize = requiredSize ;
   }
done :
   PD_TRACE_EXITRC ( SDB__MIGPARSER__REALLOCJB, rc );
   return rc ;
error :
   goto done ;
}
*/
/*PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER__RDNXT, "_migParser::_readNext" )
INT32 _migParser::_readNext ( CHAR *pBuffer, UINT32 size )
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( pBuffer, "pBuffer can't be NULL" ) ;
   PD_TRACE_ENTRY ( SDB__MIGPARSER__RDNXT );
   SINT64 iLenRead = 0 ;
   do
   {
      rc = ossRead ( &_file, pBuffer, size, &iLenRead ) ;
   } while ( rc == SDB_INTERRUPT ) ;
   if ( rc == SDB_EOF )
   {
      PD_LOG ( PDEVENT, "Hit the end of file" ) ;
   }
   else if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to read from file, rc = %d", rc ) ;
   }
   else
   {
      _bufUsed += iLenRead ;
   }

   PD_TRACE_EXITRC ( SDB__MIGPARSER__RDNXT, rc );
   return rc ;
}*/

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER__IMPRCD, "_imgParser::_importRecord" )
INT32 _migParser::_importRecord ( bson **bsonObj,  SINT32 num )
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( bsonObj, "bsonObj can't be NULL" ) ;
   PD_TRACE_ENTRY ( SDB__MIGPARSER__IMPRCD );
   rc = sdbInsert ( _collection, *bsonObj ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to insert record, rc = %d", rc ) ;
      goto error ;
   }

done :
   PD_TRACE_EXITRC ( SDB__MIGPARSER__IMPRCD, rc );
   return rc ;
error :
   goto done ;
}
/*
PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER__APPJSONSTR, "_migParser::_appendJsonStr" )
INT32 _migParser::_appendJsonStr ( const CHAR *pStr, UINT32 num )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGPARSER__APPJSONSTR );
   // +1 for '\0'
   UINT32 totalSize = num + _jsonBufOccupied () + 1 ;
   rc = _reallocJsonBuf ( totalSize ) ;
   if ( rc )
      goto error ;
   ossMemcpy ( _pJsonCurPtr, pStr, num ) ;
   _pJsonCurPtr += num ;
   *_pJsonCurPtr = '\0' ;
done :
   PD_TRACE_EXITRC ( SDB__MIGPARSER__APPJSONSTR, rc );
   return rc ;
error :
   goto done ;
}*/

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGCSVPS_INIT, "_migCSVParser::init" )
INT32 _migCSVParser::init ( sdbCollectionHandle collection,
                            const CHAR *pInputFile,
                            CHAR *fields,
                            BOOLEAN isHeaderline,
                            BOOLEAN autoAddField,
                            BOOLEAN autoCompletion,
                            const CHAR *pDelChar,
                            const CHAR *pDelField,
                            const CHAR *pDelRecord )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGCSVPS_INIT );
   SDB_ASSERT ( collection, "collection can't be NULL" )
   SDB_ASSERT ( pInputFile, "input file can't be NULL" )
   CHAR delChar = '"' ;
   CHAR delField = ',' ;
   CHAR delRecord = '\n' ;
   _utilParserParamet parserPara ;

   if ( pDelChar )
   {
      INT32 delCharSize = ossStrlen ( pDelChar ) ;
      if ( delCharSize == 1 )
      {
         delChar = pDelChar[0] ;
      }
      if ( delCharSize > 1 &&
           pDelChar[0] == '0' &&
           pDelChar[1] == 'x' )
      {
         delChar = 0 ;
         if ( delCharSize > 4 )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         for ( INT32 i = 3; i <= delCharSize; ++i )
         {
            delChar *= 16 ;
            CHAR c = pDelChar[i-1] ;
            if ( c >= '0' && c <= '9' )
            {
               delChar += c - '0' ;
            }
            else if ( c >= 'a' && c <= 'f' )
            {
               delChar += c - 'a' + 10 ;
            }
            else if ( c >= 'A' && c <= 'F' )
            {
               delChar += c - 'A' + 10 ;
            }
         }
      }
   }

   if ( pDelField )
   {
      INT32 delFieldSize = ossStrlen ( pDelField ) ;
      if ( delFieldSize == 1 )
      {
         delField = pDelField[0] ;
      }
      if ( delFieldSize > 1 &&
           pDelField[0] == '0' &&
           pDelField[1] == 'x' )
      {
         delField = 0 ;
         if ( delFieldSize > 4 )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         for ( INT32 i = 3; i <= delFieldSize; ++i )
         {
            delField *= 16 ;
            CHAR c = pDelField[i-1] ;
            if ( c >= '0' && c <= '9' )
            {
               delField += c - '0' ;
            }
            else if ( c >= 'a' && c <= 'f' )
            {
               delField += c - 'a' + 10 ;
            }
            else if ( c >= 'A' && c <= 'F' )
            {
               delField += c - 'A' + 10 ;
            }
         }
      }
   }

   if ( pDelRecord )
   {
      INT32 delRecordSize = ossStrlen ( pDelRecord ) ;
      if ( delRecordSize == 1 )
      {
         delRecord = pDelRecord[0] ;
      }
      if ( delRecordSize > 1 &&
           pDelRecord[0] == '0' &&
           pDelRecord[1] == 'x' )
      {
         delRecord = 0 ;
         if ( delRecordSize > 4 )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         for ( INT32 i = 3; i <= delRecordSize; ++i )
         {
            delRecord *= 16 ;
            CHAR c = pDelRecord[i-1] ;
            if ( c >= '0' && c <= '9' )
            {
               delRecord += c - '0' ;
            }
            else if ( c >= 'a' && c <= 'f' )
            {
               delRecord += c - 'a' + 10 ;
            }
            else if ( c >= 'A' && c <= 'F' )
            {
               delRecord += c - 'A' + 10 ;
            }
         }
      }
   }

   _parser = SDB_OSS_NEW _utilCSVParser() ;
   if ( !_parser )
   {
      rc = SDB_OOM ;
      PD_LOG ( PDERROR, "memory error" ) ;
      goto error ;
   }
   _parser->setDel ( delChar,
                     delField,
                     delRecord ) ;
   _collection = collection ;

   parserPara.fileName = pInputFile ;
   parserPara.bufferSize = 33554432 ;
   parserPara.blockNum = 32768 ;
   parserPara.readHeader = isHeaderline ;
   parserPara.headerBuffer = fields ;
   _autoAddField = autoAddField ;
   _autoCompletion = autoCompletion ;

   rc = _parser->initialize( &parserPara ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to _parser initialize, rc=%d", rc ) ;
      goto error ;
   }
   // parse string delimiter
   /*if ( pDelChar && !_strToDel ( pDelChar, _delChar[0] ))
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // parse field delimiter
   if ( pDelField && !_strToDel ( pDelField, _delField[0] ))
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // parse record delimiter
   if ( pDelRecord && !_strToDel ( pDelRecord, _delRecord[0] ))
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   
   // open input file
   rc = ossOpen ( pInputFile,
                  OSS_READONLY | OSS_SHAREREAD,
                  OSS_DEFAULTFILE,
                  _file ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to open input file %s, rc = %d",
               pInputFile, rc ) ;
      goto error ;
   }
   _isOpened = TRUE ;
   // allocate buffer
   _pJsonBuffer = (CHAR*)SDB_OSS_MALLOC ( MIG_INC_READ_BUFFER ) ;
   if ( !_pJsonBuffer )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   _pJsonCurPtr = _pJsonBuffer ;
   _jsonSize = MIG_INC_READ_BUFFER ;

   _pReadBuffer = (CHAR*)SDB_OSS_MALLOC ( MIG_INC_READ_BUFFER ) ;
   if ( !_pReadBuffer )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   _pCurPtr = _pReadBuffer ;
   _bufSize = MIG_INC_READ_BUFFER ;
   rc = _getHead () ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get CSV head, rc = %d", rc ) ;
      goto error ;
   }
   _init = TRUE ;*/
done :
   PD_TRACE_EXITRC ( SDB__MIGCSVPS_INIT, rc );
   return rc ;
error :
   goto done ;
}
/*
PD_TRACE_DECLARE_FUNCTION ( SDB__MIGCSVPS__GETHEAD, "_migCSVParser::_getHead" )
INT32 _migCSVParser::_getHead ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGCSVPS__GETHEAD );
   string curField = "" ;

   while ( TRUE )
   { // read first field of csv file
      if ( ( _pCurPtr - _pReadBuffer ) >= _bufUsed  )
      { // hit the end of buffer
         UINT32 readNextSize = 0 ;
         if ( _pCurPtr == _pReadBuffer && _bufUsed == _bufSize )
         { // the size of this record bigger than buffer size
            rc = _reallocReadBuf ( _bufUsed + MIG_INC_READ_BUFFER ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to reallocate memory, rc = %d", rc ) ;
               goto error ;
            }
            readNextSize = MIG_INC_READ_BUFFER ;
         }
         else if ( _pCurPtr > _pReadBuffer && _bufUsed == _bufSize )
         {
            _bufUsed = _bufNotParsed() ;
            ossMemmove ( _pReadBuffer, _pCurPtr, _bufUsed ) ;
            _pCurPtr = _pReadBuffer + _bufUsed ;
            readNextSize = MIG_INC_READ_BUFFER - _bufUsed % MIG_INC_READ_BUFFER ;
         }
         else
         { // _bufUsed < _bufSize
            readNextSize = MIG_INC_READ_BUFFER ;
         }
         rc = _readNext ( _pCurPtr, readNextSize ) ;
         if ( rc )
            goto error ;
      }

      if ( *_pCurPtr == _delField[0] || *_pCurPtr == _delRecord[0] )
      {
         trim ( curField ) ;
         _fieldAll.push_back ( curField ) ;
         if ( *_pCurPtr == _delRecord[0] )
         {
            _pCurPtr++ ;
            break ;
         }
         curField = "" ;
      }
      else if ( *_pCurPtr == '.' )
      {
         PD_LOG ( PDERROR, "Key field can't include '.'" ) ;
         rc = SDB_INVALIDARG;
         goto error ;
      }
      else
         curField += *_pCurPtr ;
      _pCurPtr++ ;
   }
done :
   PD_TRACE_EXITRC ( SDB__MIGCSVPS__GETHEAD, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGCSVPS__GETFLDS, "_migCSVParser::_getFields" )
INT32 _migCSVParser::_getFields ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGCSVPS__GETFLDS );
   if ( _isEOF )
   { // end of file
      rc = SDB_EOF ;
      goto error ;
   }
   // json begin
   _pJsonCurPtr = _pJsonBuffer ;
   rc = _appendJsonStr ( "{", 1 ) ;
   if ( rc )
      goto error ;
   for ( UINT32 i = 0; i < _fieldAll.size(); ++i )
   {
      BOOLEAN inQuotes = FALSE ;
      BOOLEAN preWasQuote = FALSE ;
      BOOLEAN isEOR = FALSE ;
      CHAR *pNext = _pCurPtr ;
      const CHAR* key = _fieldAll[i].c_str() ;
      UINT32 keyLen = ossStrlen(key) ;
      rc = _appendJsonStr ( key, keyLen ) ;
      if ( rc )
         goto error ;
      rc = _appendJsonStr ( ":", 1 ) ;
      if ( rc )
         goto error ;

      while ( TRUE )
      { // read every field
         if ( ( pNext - _pReadBuffer ) >= _bufUsed  )
         { // hit the end of buffer
            UINT32 readNextSize = 0 ;
            if ( _pCurPtr == _pReadBuffer && _bufUsed == _bufSize )
            { // the size of this record bigger than buffer size
               rc = _reallocReadBuf ( _bufUsed + MIG_INC_READ_BUFFER ) ;
               if ( rc )
               {
                  PD_LOG ( PDERROR, "Failed to reallocate memory, rc = %d", rc ) ;
                  goto error ;
               }
               pNext = _pReadBuffer + _bufUsed ;
               readNextSize = MIG_INC_READ_BUFFER ;
            }
            else if ( _pCurPtr > _pReadBuffer && _bufUsed == _bufSize )
            {
               _bufUsed = _bufNotParsed() ;
               ossMemmove ( _pReadBuffer, _pCurPtr, _bufUsed ) ;
               _pCurPtr = _pReadBuffer ;
               pNext = _pCurPtr + _bufUsed ;
               readNextSize = MIG_INC_READ_BUFFER - _bufUsed % MIG_INC_READ_BUFFER ;
            }
            else
            { // _bufUsed < _bufSize
               readNextSize = MIG_INC_READ_BUFFER ;
            }
            rc = _readNext ( pNext, readNextSize ) ;
            // hit the end of file
            if ( SDB_EOF == rc )
            {
               UINT32 valLen = pNext - _pCurPtr ;
               if ( valLen > 0 && !isEOR )
               { // field not empty
                  rc = _appendJsonStr ( _pCurPtr, valLen ) ;
                  if ( rc )
                     goto error ;
               }
               else if ( !isEOR )
               { // clear the key name and ':'
                  _pJsonCurPtr -= ( keyLen + 1 ) ;
                  if ( i > 0 )
                  // if not the first field, clear ','
                     _pJsonCurPtr -= 1 ;
               }
                _isEOF = TRUE ;
               goto ok ;
            }
            else if ( rc )
               goto error ;
         }
         if ( *pNext == _delChar[0] )
         { // start with "
            if ( !inQuotes )
            { // starting string
               *pNext = '"' ;
               inQuotes = TRUE ;
            }
            else
            { // " after starting string
               if ( preWasQuote )
               { // " in string
                  *(pNext-1) = '\\' ;
                  preWasQuote = FALSE ;
               }
               else
                  preWasQuote = TRUE ;
            }
         }
         else
         {
            // a quote may represents end of string, so let's always record
            // "preWasQuote" when hitting quote, and if the next is not another
            // ", we consider the previous one as real end of string
            if ( inQuotes && preWasQuote )
            { // stopping string
               *(pNext-1) = '"' ;
               inQuotes = FALSE ;
               preWasQuote = FALSE ;
            }
            if ( isEOR )
            {
               // ignore the extra fields of current record
               if ( !inQuotes && *pNext == _delRecord[0] )
               {// hit the end of record
                  ++pNext ;
                  _pCurPtr = pNext ;
                  goto ok ;
               }
               else
               {
                  ++pNext ;
                  continue ;
               }
            }
            // if there's slash in the original string, we need to append it to
            // two slash chars in json, so that json won't consider string "\n"
            // as newline, instead it will be read as "\\n"
            if ( inQuotes && *pNext == '\\' )
            {
               rc = _appendJsonStr ( _pCurPtr, pNext - _pCurPtr ) ;
               if ( rc )
                  goto error ;
               _pCurPtr = pNext + 1 ;
               rc = _appendJsonStr ( "\\\\", 2 ) ;
               if ( rc )
                  goto error ;
            }
            if ( !inQuotes && ( *pNext == _delField[0] || *pNext == _delRecord[0] ) )
            {
               UINT32 valLen = pNext - _pCurPtr ;
               if ( valLen > 0 )
               { // field not empty
                  rc = _appendJsonStr ( _pCurPtr, valLen ) ;
                  if ( rc )
                     goto error ;
               }
               else
               { // clear the key name and ':'
                  _pJsonCurPtr -= ( keyLen + 1 ) ;
                  if ( i > 0 )
                  // not the first field, clear the ','
                     _pJsonCurPtr -= 1 ;
               }
               _pCurPtr = pNext + 1 ;
               if ( *pNext == _delRecord[0] )
                // hit the end of record
                  goto ok ;
               if ( i == _fieldAll.size() - 1 )
               { // end record
                  isEOR = TRUE ;
                  ++pNext ;
                  continue ;
               }
               else
               { // finish read one field, prepare to read next field
                  rc = _appendJsonStr ( ",", 1 ) ;
                  if ( rc )
                     goto error ;
               }
               break ;
            }
         }
         ++pNext ;
      }
   }
ok :
   // json end
   rc = _appendJsonStr ( "}", 1 ) ;
   if ( rc )
      goto error ;
done :
   PD_TRACE_EXITRC ( SDB__MIGCSVPS__GETFLDS, rc );
   return rc ;
error :
   goto done ;
}*/

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGCSVPS__GETRCD, "_migCSVParser::_getRecord" )
INT32 _migCSVParser::_getRecord ( bson &record )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGCSVPS__GETRCD );
   UINT32  startOffset = 0 ;
   UINT32  size        = 0 ;
   _convertCSV ccsv ;
   bson obj ;
   CHAR *buffer = _parser->getBuffer() ;
   CHAR *pJsonBuffer = NULL ;
   rc = _parser->getNextRecord ( startOffset, size ) ;
   if ( rc )
   {
      if ( rc == SDB_EOF )
      {
         if ( 0 == size )
         {
            goto done ;
         }
      }
      else
      {
         PD_LOG ( PDERROR, "Failed to _parser getNextRecord, rc=%d", rc ) ;
         goto error ;
      }
   }
   rc = ccsv._convertCSVToJson ( buffer + startOffset, size,
                                 _autoAddField, _autoCompletion,
                                 _parser, &pJsonBuffer ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to convert CSV to Json, rc=%d", rc ) ;
      goto error ;
   }
   pJsonBuffer = json2rawcbson ( pJsonBuffer ) ;
   if ( !pJsonBuffer )
   {
      rc = SDB_UTIL_PARSE_JSON_INVALID ;
      PD_LOG ( PDERROR, "Failed json" ) ;
      goto error ;
   }
   obj.ownmem = 0 ;
   obj.data = NULL ;
   bson_init_finished_data ( &obj, pJsonBuffer ) ;
   bson_copy ( &record, &obj ) ;
   free ( pJsonBuffer ) ;
   /*rc = _getFields () ;
   if ( rc )
      goto error ;
   if ( !jsonToBson ( &record, _pJsonBuffer ) )
      rc = SDB_MIG_IMP_BAD_RECORD ;
   */
done :
   PD_TRACE_EXITRC ( SDB__MIGCSVPS__GETRCD, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGJSONPS_INIT, "_migJSONParser::init" )
INT32 _migJSONParser::init ( sdbCollectionHandle collection,
                             const CHAR *pInputFile,
                             BOOLEAN bMongoCompatible )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGJSONPS_INIT );
   SDB_ASSERT ( collection, "collection can't be NULL" )
   SDB_ASSERT ( pInputFile, "input file can't be NULL" )
   _utilParserParamet parserPara ;
   _parser = SDB_OSS_NEW _utilJSONParser() ;
   if ( !_parser )
   {
      rc = SDB_OOM ;
      PD_LOG ( PDERROR, "memory error" ) ;
      goto error ;
   }
   _collection = collection ;
   /*
   _bMongoCompatible = bMongoCompatible ;
   // open input file
   rc = ossOpen ( pInputFile,
                  OSS_READONLY | OSS_SHAREREAD,
                  OSS_DEFAULTFILE,
                  _file ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to open input file %s, rc = %d",
               pInputFile, rc ) ;
      goto error ;
   }
   _isOpened = TRUE ;*/
   parserPara.fileName = pInputFile ;
   parserPara.bufferSize = 33554432 ;
   parserPara.blockNum = 32768 ;
   parserPara.readHeader = FALSE ;

   rc = _parser->initialize( &parserPara ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to _parser initialize, rc=%d", rc ) ;
      goto error ;
   }
   /*_pReadBuffer = (CHAR*)SDB_OSS_MALLOC ( MIG_INC_READ_BUFFER ) ;
   if ( !_pReadBuffer )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   _pCurPtr = _pReadBuffer ;
   _bufSize = MIG_INC_READ_BUFFER ;
   _init = TRUE ;*/
done :
   PD_TRACE_EXITRC ( SDB__MIGJSONPS_INIT, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGJSONPS__GETRCD, "_migJSONParser::_getRecord" )
INT32 _migJSONParser::_getRecord ( bson &record )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGJSONPS__GETRCD );
   UINT32  startOffset = 0 ;
   UINT32  size        = 0 ;
   bson obj ;
   CHAR *buffer = _parser->getBuffer() ;
   CHAR *buffer2 = NULL ;
   rc = _parser->getNextRecord ( startOffset, size ) ;
   if ( rc )
   {
      if ( rc == SDB_EOF )
      {
         goto done ;
      }
      PD_LOG ( PDERROR, "Failed to _parser getNextRecord, rc=%d", rc ) ;
      goto error ;
   }
   if ( !isValidUTF8WSize ( buffer + startOffset, size ) )
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "It is not utf-8 file, rc=%d", rc ) ;
      goto error ;
   }
   buffer2 = json2rawcbson ( buffer + startOffset ) ;
   if ( !buffer2 )
   {
      rc = SDB_UTIL_PARSE_JSON_INVALID ;
      PD_LOG ( PDERROR, "Failed json" ) ;
      goto error ;
   }
   obj.ownmem = 0 ;
   obj.data = NULL ;
   bson_init_finished_data ( &obj, buffer2 ) ;
   bson_copy ( &record, &obj ) ;
   free ( buffer2 ) ;
   /*
   BOOLEAN inQuotes = FALSE ;
   BOOLEAN inJson   = FALSE ;
   BOOLEAN isEscape = FALSE ;
   BOOLEAN isEOR    = FALSE ;
   INT32 level      = 0 ;
   CHAR tmpSaved    = 0 ;
   CHAR *pNext      = _pCurPtr ;
   if ( _isEOF )
   { // end of file
      rc = SDB_EOF ;
      goto error ;
   }

   while ( TRUE )
   { // read every field
      if ( ( pNext - _pReadBuffer ) >= _bufUsed  )
      { // hit the end of buffer
         UINT32 readNextSize = 0 ;
         if ( _pCurPtr == _pReadBuffer && _bufUsed == _bufSize )
         { // the size of this record bigger than buffer size
            rc = _reallocReadBuf ( _bufUsed + MIG_INC_READ_BUFFER ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to reallocate memory, rc = %d", rc ) ;
               goto error ;
            }
            pNext = _pReadBuffer + _bufUsed ;
            readNextSize = MIG_INC_READ_BUFFER ;
         }
         else if ( _pCurPtr > _pReadBuffer && _bufUsed == _bufSize )
         {
            _bufUsed = _bufNotParsed() ;
            ossMemmove ( _pReadBuffer, _pCurPtr, _bufUsed ) ;
            _pCurPtr = _pReadBuffer ;
            pNext = _pCurPtr + _bufUsed ;
            readNextSize = MIG_INC_READ_BUFFER - _bufUsed % MIG_INC_READ_BUFFER ;
         }
         else
         { // _bufUsed < _bufSize
            readNextSize = MIG_INC_READ_BUFFER ;
         }
         rc = _readNext ( pNext, readNextSize ) ;
         // hit the end of file
         if ( SDB_EOF == rc )
         { // need next loop to allocate one byte for fill with '\0'
            _isEOF = TRUE ;
            if ( !inJson )
               goto done ;
            isEOR = TRUE ;
            rc = SDB_OK ;
         }
         else if ( rc )
            goto error ;
      }
      if ( isEOR )
      { // end of json record, end with '\0'
         if ( level != 0 )
         {
            rc = SDB_MIG_IMP_BAD_RECORD ;
            _pCurPtr = pNext ;
            goto error ;
         }
         tmpSaved = *pNext ;
         *pNext = '\0' ;
         break ;
      }
      if ( isEscape )
      {// ignore this char
         isEscape = FALSE ;
         pNext++ ;
         continue ;
      }
      switch ( *pNext )
      {
      case '{' :
         if ( !inQuotes && ++level == 1 )
         { // json begin
            inJson = TRUE ;
            _pCurPtr = pNext ;
         }
         break ;
      case '}' :
         if ( !inQuotes && --level <= 0 )
         // json stop, need next loop to allocate one byte for fill with '\0'
            isEOR = TRUE ;
         break ;
      case '"' :
         inQuotes = !inQuotes ;
         break ;
      case '\\' :
         // escape next char
         isEscape = TRUE ;
         break ;
      default :
         // skip to next char
         break ;
      }
      pNext++ ;
   } // while ( TRUE )

   if ( !jsonToBson2 ( &record, _pCurPtr, _bMongoCompatible ) )
      rc = SDB_MIG_IMP_BAD_RECORD ;
   *pNext = tmpSaved ;
   _pCurPtr = pNext ;
   */
done :
   PD_TRACE_EXITRC ( SDB__MIGJSONPS__GETRCD, rc );
   return rc ;
error :
   goto done ;
}
