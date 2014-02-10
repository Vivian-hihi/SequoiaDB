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
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGPARSER_RUN, "_migParser::run" )
INT32 _migParser::run ( INT32 &total, INT32 &succeed )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGPARSER_RUN );
   INT32 count = 0, succ = 0;

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

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGCSVPS_INIT, "_migCSVParser::init" )
INT32 _migCSVParser::init ( sdbCollectionHandle collection,
                            const CHAR *pInputFile,
                            CHAR *fields,
                            BOOLEAN isHeaderline,
                            BOOLEAN autoAddField,
                            BOOLEAN autoCompletion,
                            BOOLEAN linePriority,
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
      else if ( delCharSize > 1 &&
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
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "delchar must be 1 char of 16 hex format ( e.g. 0x09 )" ) ;
         goto error ;
      }
   }

   if ( pDelField )
   {
      INT32 delFieldSize = ossStrlen ( pDelField ) ;
      if ( delFieldSize == 1 )
      {
         delField = pDelField[0] ;
      }
      else if ( delFieldSize > 1 &&
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
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "delfield must be 1 char of 16 hex format ( e.g. 0x09 )" ) ;
         goto error ;
      }
   }

   if ( pDelRecord )
   {
      INT32 delRecordSize = ossStrlen ( pDelRecord ) ;
      if ( delRecordSize == 1 )
      {
         delRecord = pDelRecord[0] ;
      }
      else if ( delRecordSize > 1 &&
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
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "delrecord must be 1 char of 16 hex format ( e.g. 0x09 )" ) ;
         goto error ;
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
   parserPara.linePriority = linePriority ;
   _autoAddField = autoAddField ;
   _autoCompletion = autoCompletion ;

   rc = _parser->initialize( &parserPara ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to _parser initialize, rc=%d", rc ) ;
      goto error ;
   }
done :
   PD_TRACE_EXITRC ( SDB__MIGCSVPS_INIT, rc );
   return rc ;
error :
   goto done ;
}

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

done :
   PD_TRACE_EXITRC ( SDB__MIGCSVPS__GETRCD, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGJSONPS_INIT, "_migJSONParser::init" )
INT32 _migJSONParser::init ( sdbCollectionHandle collection,
                             const CHAR *pInputFile,
                             BOOLEAN linePriority,
                             BOOLEAN bMongoCompatible,
                             const CHAR *pDelRecord )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGJSONPS_INIT );
   SDB_ASSERT ( collection, "collection can't be NULL" )
   SDB_ASSERT ( pInputFile, "input file can't be NULL" )
   CHAR delChar = '"' ;
   CHAR delField = ',' ;
   CHAR delRecord = '\n' ;
   _utilParserParamet parserPara ;

   if ( pDelRecord )
   {
      INT32 delRecordSize = ossStrlen ( pDelRecord ) ;
      if ( delRecordSize == 1 )
      {
         delRecord = pDelRecord[0] ;
      }
      else if ( delRecordSize > 1 &&
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
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "delrecord must be 1 char of 16 hex format ( e.g. 0x09 )" ) ;
         goto error ;
      }
   }

   _parser = SDB_OSS_NEW _utilJSONParser() ;
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
   parserPara.readHeader = FALSE ;
   parserPara.linePriority = linePriority ;

   rc = _parser->initialize( &parserPara ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to _parser initialize, rc=%d", rc ) ;
      goto error ;
   }

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
