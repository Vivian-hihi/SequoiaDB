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

#include "migImportLoad.hpp"
#include "ossMem.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "migTrace.hpp"
#include "../util/json2rawbson.h"

_migParser::_migParser() : _parser(NULL),
                           _collection(NULL)
{
}

_migParser::~_migParser()
{
   SAFE_OSS_DELETE ( _parser ) ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGLOADPARSER_RUN, "_migParser::run" )
INT32 _migParser::run ( INT32 &total, INT32 &succeed, INT32 insertNum )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGLOADPARSER_RUN );
   //PD_LOG ( PDERROR, "num %d", insertNum ) ;
   INT32 count = 0, succ = 0;
   INT32 maxInsert = insertNum ;
   INT32 bsonObjNum = 0 ;
   bson **bsonObjArray = new(std::nothrow) bson* [maxInsert] ;
   if ( !bsonObjArray )
   {
      rc = SDB_OOM ;
      PD_LOG ( PDERROR, "Memory Failed" ) ;
      goto error ;
   }

   for ( INT32 i = 0; i < maxInsert; ++i )
   {
      bsonObjArray[i] = new(std::nothrow) bson() ;
      if ( !bsonObjArray[i] )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "Memory Failed" ) ;
         goto error ;
      }
   }

   while ( TRUE )
   {
      bson *tempObj = bsonObjArray[ bsonObjNum ] ;
      bson_init ( tempObj ) ;
      rc = _getRecord ( *tempObj ) ;
      if ( rc == SDB_EOF )
      {
         bson_destroy ( tempObj ) ;
         if ( bsonObjNum >= 0 )
         {
            rc = _importRecord ( bsonObjArray, bsonObjNum ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to import record in %d", count ) ;
            }
            for ( INT32 i = 0; i < bsonObjNum; ++i )
            {
               tempObj = bsonObjArray[ i ] ;
               bson_destroy ( tempObj ) ;
            }
            bsonObjNum = 0 ;
         }
         PD_LOG ( PDEVENT, "Import Successfully" ) ;
         rc = SDB_OK ;
         break ;
      }
      else if ( rc == SDB_UTIL_PARSE_JSON_INVALID )
      {
         bson_destroy ( tempObj ) ;
         count++ ;
         PD_LOG ( PDERROR, "Bad record in %d", count ) ;
         continue ;
      }
      else if ( rc )
      {
         bson_destroy ( tempObj ) ;
         count++ ;
         PD_LOG ( PDERROR, "Import Failed, rc = %d", rc ) ;
         goto error ;
      }
      if ( tempObj->dataSize <= 5 )
      {
         bson_destroy ( tempObj ) ;
         count++ ;
         // empty bson
         continue ;
      }
      ++count ;
      ++succ ;
      ++bsonObjNum ;
      if ( maxInsert == bsonObjNum )
      {
         rc = _importRecord ( bsonObjArray, maxInsert ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to import record in %d", count ) ;
         }
         for ( INT32 i = 0; i < bsonObjNum; ++i )
         {
            tempObj = bsonObjArray[ i ] ;
            bson_destroy ( tempObj ) ;
         }
         bsonObjNum = 0 ;
      }
   }
   total = count ;
   succeed = succ ;
done :
   if ( bsonObjArray )
   {
      for ( INT32 i = 0; i < maxInsert; ++i )
      {
         if ( bsonObjArray[i] )
         {
            delete bsonObjArray[i] ;
         }
      }
      delete bsonObjArray ;
   }
   PD_TRACE_EXITRC ( SDB__MIGLOADPARSER_RUN, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGLOADPARSER__IMPRCD, "_imgParser::_importRecord" )
INT32 _migParser::_importRecord ( bson **bsonObj,  SINT32 num )
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( bsonObj, "bsonObj can't be NULL" ) ;
   PD_TRACE_ENTRY ( SDB__MIGLOADPARSER__IMPRCD );
   rc = sdbBulkInsert ( _collection, FLG_INSERT_CONTONDUP, bsonObj, num ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to insert record, rc = %d", rc ) ;
      goto error ;
   }

done :
   PD_TRACE_EXITRC ( SDB__MIGLOADPARSER__IMPRCD, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGLOADCSVPS_INIT, "_migCSVParser::init" )
INT32 _migCSVParser::init ( sdbCollectionHandle collection,
                            const CHAR *pInputFile,
                            const CHAR *pPath,
                            const CHAR *pDelChar,
                            const CHAR *pDelField,
                            const CHAR *pDelRecord,
                            const CHAR *sourceHost,
                            const CHAR *sourceUser,
                            UINT16 port,
                            CHAR *fields,
                            BOOLEAN isHeaderline,
                            BOOLEAN autoAddField,
                            BOOLEAN autoCompletion,
                            BOOLEAN linePriority,
                            migImportAccess accessModel )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGLOADCSVPS_INIT );
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

   if ( MIG_IMPORT_GET_IO == accessModel )
   {
      parserPara.accessModel = UTIL_GET_IO ;
   }
   else if ( MIG_IMPORT_GET_HDFS == accessModel )
   {
      parserPara.accessModel = UTIL_GET_HDFS ;
   }

   parserPara.fileName = pInputFile ;
   parserPara.path = pPath ;
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
   PD_TRACE_EXITRC ( SDB__MIGLOADCSVPS_INIT, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGLOADCSVPS__GETRCD, "_migCSVParser::_getRecord" )
INT32 _migCSVParser::_getRecord ( bson &record )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGLOADCSVPS__GETRCD );
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
   PD_TRACE_EXITRC ( SDB__MIGLOADCSVPS__GETRCD, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGLOADJSONPS_INIT, "_migJSONParser::init" )
INT32 _migJSONParser::init ( sdbCollectionHandle collection,
                             const CHAR *pInputFile,
                             const CHAR *pPath,
                             const CHAR *sourceHost,
                             const CHAR *sourceUser,
                             UINT16 port,
                             BOOLEAN bMongoCompatible,
                             BOOLEAN linePriority,
                             migImportAccess accessModel )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGLOADJSONPS_INIT );
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

   if ( MIG_IMPORT_GET_IO == accessModel )
   {
      parserPara.accessModel = UTIL_GET_IO ;
   }
   else if ( MIG_IMPORT_GET_HDFS == accessModel )
   {
      parserPara.accessModel = UTIL_GET_HDFS ;
   }

   parserPara.fileName = pInputFile ;
   parserPara.path = pPath ;
   parserPara.bufferSize = 33554432 ;
   parserPara.blockNum = 32768 ;
   parserPara.readHeader = FALSE ;
   parserPara.port = port ;
   parserPara.path = pPath ;
   parserPara.hostName = sourceHost ;
   parserPara.user = sourceUser ;
   parserPara.linePriority = linePriority ;
   

   rc = _parser->initialize( &parserPara ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to _parser initialize, rc=%d", rc ) ;
      goto error ;
   }
done :
   PD_TRACE_EXITRC ( SDB__MIGLOADJSONPS_INIT, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB__MIGLOADJSONPS__GETRCD, "_migJSONParser::_getRecord" )
INT32 _migJSONParser::_getRecord ( bson &record )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGLOADJSONPS__GETRCD );
   UINT32  startOffset = 0 ;
   UINT32  size        = 0 ;
   bson obj ;
   CHAR *buffer = _parser->getBuffer() ;
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
   buffer = json2rawcbson ( buffer + startOffset ) ;
   if ( !buffer )
   {
      rc = SDB_UTIL_PARSE_JSON_INVALID ;
      PD_LOG ( PDERROR, "Failed json" ) ;
      goto error ;
   }
   obj.ownmem = 0 ;
   obj.data = NULL ;
   bson_init_finished_data ( &obj, buffer ) ;
   bson_copy ( &record, &obj ) ;
   free ( buffer ) ;
done :
   PD_TRACE_EXITRC ( SDB__MIGLOADJSONPS__GETRCD, rc );
   return rc ;
error :
   goto done ;
}
