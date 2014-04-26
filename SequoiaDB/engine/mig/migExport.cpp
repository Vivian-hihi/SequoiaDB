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

   Source File Name = migExport.cpp

   Descriptive Name = Migration Export Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains implementation for export
   operation

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "../client/jstobs.h"
#include "migExport.hpp"
#include "ossIO.hpp"
#include "ossMem.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "migTrace.hpp"

_migExtractor::_migExtractor()
{
   _delChar[0]     = MIG_DEFAULT_DELCHAR ;
   _delChar[1]     = 0 ;
   _delField[0]    = MIG_DEFAULT_DELFIELD ;
   _delField[1]    = 0 ;
   _delRecord[0]   = MIG_DEFAULT_DELRECORD ;
   _delRecord[1]   = 0 ;
   _collection     = SDB_INVALID_HANDLE ;
   _isOpened       = FALSE ;
   _init           = FALSE ;
   _pExtractBuffer = NULL ;
   _pCurPtr        = NULL ;
   _bufSize        = 0 ;
}

_migExtractor::~_migExtractor()
{
   if ( _isOpened )
   {
      ossClose ( _file ) ;
   }
   if ( _pExtractBuffer )
   {
      SDB_OSS_FREE ( _pExtractBuffer ) ;
   }
}

// format hex string to delimiter
// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGEXTR__HEXSTR2DEL, "_migExtractor::_hexStrToDel" )
BOOLEAN _migExtractor::_hexStrToDel ( const CHAR *pDelStr,
                                      CHAR &del )
{
   PD_TRACE_ENTRY ( SDB__MIGEXTR__HEXSTR2DEL );
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
   PD_TRACE_EXIT ( SDB__MIGEXTR__HEXSTR2DEL );
   return success ;
error :
   goto done ;
}

// convert string to delimiter
// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGEXTR__STR2DEL, "_migExtractor::_strToDel" )
BOOLEAN _migExtractor::_strToDel ( const CHAR *pDelStr,
                                   CHAR &del )
{
   PD_TRACE_ENTRY ( SDB__MIGEXTR__STR2DEL );
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
   PD_TRACE_EXIT ( SDB__MIGEXTR__STR2DEL );
   return success ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGEXTR__REALLOCMEM, "_migExtractor::_reallocMem" )
INT32 _migExtractor::_reallocMem ( UINT32 requiredSize )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGEXTR__REALLOCMEM );
   // make sure we don't allocate too much, round up to 4MB
   requiredSize = ossRoundUpToMultipleX ( requiredSize,
                                          MIG_INC_EXTRACT_BUFFER ) ;
   if ( requiredSize > MIG_MAX_EXTRACT_BUFFER )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   if ( requiredSize > _bufSize )
   {
      ossValuePtr diff = _bufOccupied() ;
      CHAR *pOld = _pExtractBuffer ;
      // if we need more memory, try to realloc
      _pExtractBuffer = (CHAR*)SDB_OSS_REALLOC ( _pExtractBuffer,
                                                 requiredSize ) ;
      if ( !_pExtractBuffer )
      {
         rc = SDB_OOM ;
         _pExtractBuffer = pOld ;
         goto error ;
      }
      _pCurPtr = _pExtractBuffer + diff ;
      _bufSize = requiredSize ;
   }

done :
   PD_TRACE_EXITRC ( SDB__MIGEXTR__REALLOCMEM, rc );
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGEXTR__APPSTR, "_migExtractor::_appendStr" )
INT32 _migExtractor::_appendStr ( const CHAR *pStr )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGEXTR__APPSTR );
   INT32 strSize = ossStrlen ( pStr ) ;
   // +1 for '\0'
   INT32 totalSize = strSize + ( _bufOccupied () ) + 1 ;
   rc = _reallocMem ( totalSize ) ;
   if ( rc )
   {
      goto error ;
   }
   ossMemcpy ( _pCurPtr, pStr, strSize ) ;
   _pCurPtr += strSize ;
   *_pCurPtr = '\0' ;
done :
   PD_TRACE_EXITRC ( SDB__MIGEXTR__APPSTR, rc );
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGEXTR__FLHBUF, "_migExtractor::_flushBuf" )
INT32 _migExtractor::_flushBuf ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGEXTR__FLHBUF );
   SINT64 len = 0 ;
   SINT64 iLenToWrite = 0 ;
   SINT64 writePos = 0 ;
   SDB_ASSERT ( _init, "extractor must be initialized" )
   rc = _appendStr ( _delRecord ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to append del record, rc = %d", rc ) ;
      goto error ;
   }
   len = _bufOccupied() ;
   while ( len > 0 )
   {
      rc = ossWrite ( &_file, _pExtractBuffer + writePos, len, &iLenToWrite ) ;
      if ( rc && SDB_INTERRUPT != rc )
      {
         PD_LOG ( PDERROR, "Failed to write to file, rc = %d", rc ) ;
         goto error ;
      }
      len -= iLenToWrite ;
      writePos += iLenToWrite ;
      rc = SDB_OK ;
   }

done :
   PD_TRACE_EXITRC ( SDB__MIGEXTR__FLHBUF, rc );
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGEXTR_RUN, "_migExtractor::run" )
INT32 _migExtractor::run ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGEXTR_RUN );
   SDB_ASSERT ( _init, "extractor must be initialized" )

   while ( TRUE )
   {
      rc = _extractRecord() ;
      if ( rc )
      {
         if ( SDB_DMS_EOC != rc )
         {
            PD_LOG ( PDERROR, "Failed to extract record, rc = %d", rc ) ;
            goto error ;
         }
         rc = SDB_OK ;
         goto done ;
      }
      rc = _flushBuf () ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to flush buf to file, rc = %d", rc ) ;
         goto error ;
      }
      // reset current pointer
      _pCurPtr = _pExtractBuffer ;
   }
done :
   PD_TRACE_EXITRC ( SDB__MIGEXTR_RUN, rc );
   return rc ;
error :
   goto done ;
}

_migCSVExtractor::_migCSVExtractor ( vector<string> &fieldList,
                                     BOOLEAN incHead )
{
   vector<string>::iterator it ;
   for ( it = fieldList.begin(); it != fieldList.end(); ++it )
   {
      _fieldList.push_back ( *it ) ;
   }
   _incHead = incHead ;
}

_migCSVExtractor::~_migCSVExtractor ()
{
   _fieldList.clear () ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGCSVEXTR_INIT, "_migCSVExtractor::init" )
INT32 _migCSVExtractor::init ( sdbCollectionHandle collection,
                               const CHAR *pOutputFile,
                               const CHAR *pDelChar,
                               const CHAR *pDelField,
                               const CHAR *pDelRecord )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGCSVEXTR_INIT );
   SDB_ASSERT ( collection, "collection can't be NULL" )
   SDB_ASSERT ( pOutputFile, "output file can't be NULL" )
   // parse string delimiter
   if ( pDelChar && !_strToDel ( pDelChar, _delChar[0] ))
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
   _collection = collection ;
   // open output file
   rc = ossOpen ( pOutputFile,
                  OSS_REPLACE | OSS_WRITEONLY | OSS_EXCLUSIVE,
                  OSS_RU | OSS_WU | OSS_RG,
                  _file ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to open output file %s, rc = %d",
               pOutputFile, rc ) ;
      goto error ;
   }
   _isOpened = TRUE ;

   // append head
   if ( _incHead )
   {
      for ( UINT32 i = 0; i < _fieldList.size(); ++i )
      {
         rc = _appendStr ( _delChar ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to append del char, rc = %d", rc ) ;
            goto error ;
         }
         rc = _appendStr ( _fieldList[i].c_str() ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to append del char, rc = %d", rc ) ;
            goto error ;
         }
         rc = _appendStr ( _delChar ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to append del char, rc = %d", rc ) ;
            goto error ;
         }
         if ( i < _fieldList.size() - 1 )
         {
            rc = _appendStr ( _delField ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to append del char, rc = %d", rc ) ;
               goto error ;
            }
         }
      }
      rc = _appendStr ( _delRecord ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to append del char, rc = %d", rc ) ;
         goto error ;
      }
   }

   // start creating cursor
   rc = sdbQuery ( _collection, NULL, NULL, NULL, NULL, 0, -1,
                   &_cursor ) ;
   if ( rc )
   {
      if ( SDB_DMS_EOC != rc )
         PD_LOG ( PDERROR, "Failed to query collection, rc = %d", rc ) ;
      goto error ;
   }
   _init = TRUE ;
done :
   PD_TRACE_EXITRC ( SDB__MIGCSVEXTR_INIT, rc );
   return rc ;
error :
   goto done ;
}
// { " b " : 1 }
// 0 1 2 3 4 5 6
// { " " b " " : 1 }
// 0 1 2 3 4 5 6 7 8
INT32 _migCSVExtractor::_extractString ( CHAR *str, INT32 strLen, BOOLEAN trim,
                                         CHAR delChar )
{
   INT32 rc = SDB_OK ;
   INT32 tempSize = 0 ;
   CHAR *temp = NULL ;
   INT32 tempStart = 0 ;
   INT32 strStart = 0 ;

   if ( trim )
   {
      tempSize = ossStrlen ( str ) - 1 ;
   }
   else
   {
      tempSize = ossStrlen ( str ) + 1 ;
   }
   temp = new CHAR [ tempSize ] ;
   if ( !temp )
   {
      PD_LOG ( PDERROR, "Failed to new memory" ) ;
      rc = SDB_CORRUPTED_RECORD ;
      goto error ;
   }
   temp[ tempSize - 1 ] = '\0' ;
   if ( trim )
   {
      str[tempSize] = '\0' ;
      ossStrcpy ( temp, str + 1 ) ;
   }
   else
   {
      ossStrcpy ( temp, str ) ;
   }
   ossMemset ( str, 0, strLen ) ;
   for ( INT32 i = 0; i < tempSize; ++i )
   {
      if ( delChar == temp[i] || i == tempSize - 1 )
      {
         ossStrncpy ( str + strStart, temp + tempStart , i - tempStart ) ;
         strStart += ( i - tempStart + 1 ) ;
         tempStart = i ;
         if ( i < tempSize - 1 )
         {
            str[ strStart - 1 ] = _delChar[0] ;
         }
      }
   }
done:
   delete[] temp ;
   return rc ;
error:
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGCSVEXTR__EXTRRCD, "_migCSVExtractor::_extractRecord" )
INT32 _migCSVExtractor::_extractRecord ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGCSVEXTR__EXTRRCD );
   SDB_ASSERT ( _init, "extractor must be initialized first" )
   bson obj ;
   bson_init ( &obj ) ;
   bson_type t;
   INT32 objectStrSize = 0 ;
   CHAR *objectStr = NULL ;
   CHAR *objectStrCur = NULL ;
   BOOLEAN trim = FALSE ;
   // get next record from engine
   rc = sdbNext ( _cursor, &obj ) ;
   if ( rc )
   {
      if ( SDB_DMS_EOC != rc )
         PD_LOG ( PDERROR, "Failed to get next from collection, rc = %d", rc ) ;
      goto error ;
   }
   // loop for each field
   for ( UINT32 i = 0; i < _fieldList.size(); ++i )
   {
      string str = _fieldList[i] ;
      bson_iterator it ;
      // get field type
      bson_type fieldType = bson_find ( &it, &obj, str.c_str() ) ;
      // if we can't find the field, let's append null
      if ( BSON_EOO != fieldType )
      {
         // if we can find the field, first let's get free buffer size and
         // compare with field size
         INT32 leftSize = _bufFree () ;
         INT32 estLen = bson_sprint_length_iterator ( &it ) ;
         if ( 0 == estLen )
         {
            PD_LOG ( PDERROR, "Unable to estimate size of iterator" ) ;
            rc = SDB_CORRUPTED_RECORD ;
            goto error ;
         }
         // if buffer is not large enough to hold the object, let's increase it
         if ( estLen >= leftSize )
         {
            rc = _reallocMem ( _bufOccupied() +  estLen ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
                        _bufOccupied() + estLen ) ;
               goto error ;
            }
            leftSize = _bufFree () ;
         }
         // let's print the iterator into buffer using string delimiter
         t = bson_iterator_type( &it ) ;
         if ( BSON_OID == t || BSON_OBJECT == t ||
              BSON_ARRAY == t || BSON_STRING == t ||
              BSON_MINKEY == t || BSON_MAXKEY == t ||
              BSON_BINDATA == t || BSON_DATE == t ||
              BSON_REGEX == t || BSON_TIMESTAMP == t  )
         {
            if ( BSON_STRING == t )
            {
               trim = TRUE ;
            }
            else
            {
               trim = FALSE ;
            }
            objectStrSize = bson_sprint_length_iterator ( &it ) * 2 ;
            objectStr = new CHAR [ objectStrSize ] ;
            if ( !objectStr )
            {
               rc = SDB_CORRUPTED_RECORD ;
               PD_LOG ( PDERROR, "Failed to new memory" ) ;
               goto error ;
            }
            ossMemset ( objectStr, 0, objectStrSize ) ;
            objectStrCur = objectStr ;
            if ( BSON_STRING == t )
            {
               estLen = bson_sprint_iterator ( &objectStrCur, &objectStrSize,
                                               &it, _delChar[0] ) ;
            }
            else
            {
               estLen = bson_sprint_iterator ( &objectStrCur, &objectStrSize,
                                               &it, '"' ) ;
            }
            if ( 0 == estLen )
            {
               delete[] objectStr ;
               PD_LOG ( PDERROR, "Failed to sprint iterator" ) ;
               rc = SDB_CORRUPTED_RECORD ;
               goto error ;
            }
            rc = _extractString( objectStr, objectStrSize, trim, _delChar[0] ) ;
            if ( rc )
            {
               delete[] objectStr ;
               PD_LOG ( PDERROR, "Failed to extract string, rc = %d",
                        rc ) ;
               goto error ;
            }
            rc = _appendStr ( _delChar ) ;
            if ( rc )
            {
               delete[] objectStr ;
               PD_LOG ( PDERROR, "Failed to append field delimiter, rc = %d",
                        rc ) ;
               goto error ;
            }
            rc = _appendStr ( objectStr ) ;
            delete[] objectStr ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to append field delimiter, rc = %d",
                        rc ) ;
               goto error ;
            }
            rc = _appendStr ( _delChar ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to append field delimiter, rc = %d",
                        rc ) ;
               goto error ;
            }
         }
         else
         {
            estLen = bson_sprint_iterator ( &_pCurPtr, &leftSize, &it, _delChar[0] );
            if ( 0 == estLen )
            {
               PD_LOG ( PDERROR, "Failed to sprint iterator" ) ;
               rc = SDB_CORRUPTED_RECORD ;
               goto error ;
            }
         }
         // set '\0' to end of string
         *_pCurPtr = 0 ;
      }
      // append field delimiter
      if ( i < _fieldList.size() - 1 )
      {
         rc = _appendStr ( _delField ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to append field delimiter, rc = %d",
                     rc ) ;
            goto error ;
         }
      }
   }
done :
   bson_destroy ( &obj ) ;
   PD_TRACE_EXITRC ( SDB__MIGCSVEXTR__EXTRRCD, rc );
   return rc ;
error :
   goto done ;
}



//json

_migJSONExtractor::_migJSONExtractor ( vector<string> &fieldList )
{
   _hasID = TRUE ;
   if ( fieldList.size() )
   {
      vector<string>::iterator it ;
      for ( it = fieldList.begin(); it != fieldList.end(); ++it )
      {
         _fieldList.push_back ( *it ) ;
      }
   }
}

_migJSONExtractor::~_migJSONExtractor ()
{
   if ( _fieldList.size() )
   {
      _fieldList.clear () ;
   }
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGJSONEXTR_INIT, "_migJSONExtractor::init" )
INT32 _migJSONExtractor::init ( sdbCollectionHandle collection,
                                const CHAR *pOutputFile,
                                const CHAR *pDelRecord,
                                BOOLEAN hasID )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGJSONEXTR_INIT );
   SDB_ASSERT ( collection, "collection can't be NULL" )
   SDB_ASSERT ( pOutputFile, "output file can't be NULL" )

   _hasID = hasID ;
   // parse record delimiter
   if ( pDelRecord && !_strToDel ( pDelRecord, _delRecord[0] ))
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   _collection = collection ;
   // open output file
   rc = ossOpen ( pOutputFile,
                  OSS_REPLACE | OSS_WRITEONLY | OSS_EXCLUSIVE,
                  OSS_RU | OSS_WU | OSS_RG,
                  _file ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to open output file %s, rc = %d",
               pOutputFile, rc ) ;
      goto error ;
   }
   _isOpened = TRUE ;

   // start creating cursor
   rc = sdbQuery ( _collection, NULL, NULL, NULL, NULL, 0, -1,
                   &_cursor ) ;
   if ( rc )
   {
      if ( SDB_DMS_EOC != rc )
         PD_LOG ( PDERROR, "Failed to query collection, rc = %d", rc ) ;
      goto error ;
   }
   _init = TRUE ;
done :
   PD_TRACE_EXITRC ( SDB__MIGJSONEXTR_INIT, rc );
   return rc ;
error :
   goto done ;
}

BOOLEAN _migJSONExtractor::_reallocateBuffer ( CHAR **pBuf, INT32 *bufSize )
{
   INT32 currentSize = *bufSize ;
   CHAR *pOldBuf     = *pBuf ;
   if ( 0 == currentSize )
      currentSize = INIT_BUF_SIZE ;
   else
      currentSize += currentSize>INC_BUF_SIZE?INC_BUF_SIZE:currentSize ;

   // allocate memory
   *pBuf = (CHAR*)SDB_OSS_MALLOC(sizeof(CHAR) * currentSize) ;
   // if allocate failed, we have to restore the original buffer pointer
   // and return FALSE
   if ( !(*pBuf) )
   {
      *pBuf = pOldBuf ;
      return FALSE ;
   }
   // otherwise let's free the original memory and set bufSize
   SAFE_OSS_FREE ( pOldBuf ) ;
   *bufSize = currentSize ;
   return TRUE ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__MIGJSONEXTR__EXTRRCD, "_migJSONExtractor::_extractRecord" )
INT32 _migJSONExtractor::_extractRecord ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__MIGJSONEXTR__EXTRRCD );
   SDB_ASSERT ( _init, "extractor must be initialized first" )
   bson obj ;
   bson_init ( &obj ) ;
   bson *newObj = NULL ;
   CHAR *jsonStr = NULL ;
   INT32 jsonSize = 0 ;
   bson_type fieldType = BSON_EOO ;
   CHAR *key = NULL ;
   CHAR *keyCursor = NULL ;
   BOOLEAN hasID = FALSE ;
   string fieldID = "_id" ;
   UINT32 filedIDLen = ossStrlen ( fieldID.c_str() ) ;

   newObj = bson_create () ;
   if ( !newObj )
   {
      PD_LOG ( PDERROR, "Failed to create bson object" ) ;
      goto error ;
   }
   bson_init( newObj ) ;
   // get next record from engine
   rc = sdbNext ( _cursor, &obj ) ;
   if ( rc )
   {
      if ( SDB_DMS_EOC != rc )
         PD_LOG ( PDERROR, "Failed to get next from collection, rc = %d", rc ) ;
      goto error ;
   }
   if ( _fieldList.size() )
   {
      // loop for each field
      for ( UINT32 i = 0; i < _fieldList.size(); ++i )
      {
         string str = _fieldList[i] ;
         if ( ossStrlen ( str.c_str() ) == filedIDLen )
         {
            if ( 0 == ossStrncmp ( fieldID.c_str(), str.c_str(), filedIDLen ) )
            {
               hasID = TRUE ;
            }
         }
      }
      if ( !hasID && _hasID )
      {
         _fieldList.push_back( fieldID ) ;
      }
      for ( UINT32 i = 0; i < _fieldList.size(); ++i )
      {
         string str = _fieldList[i] ;
         bson_iterator it ;

         // get field type
         fieldType = bson_find ( &it, &obj, str.c_str() ) ;
         // if we can't find the field, let's append null
         if ( BSON_EOO != fieldType )
         {
            if ( ossStrrchr ( str.c_str(), '.' ) )
            {
               key = (CHAR *)SDB_OSS_MALLOC ( str.length() + 1 ) ;
               if ( !key )
               {
                  PD_LOG ( PDERROR, "Can not to malloc memory" ) ;
                  goto error ;
               }
               key[str.length()] = 0 ;
               ossStrcpy ( key, str.c_str() ) ;
               keyCursor = ossStrrchr ( key, '.' ) + 1 ;
               bson_append_element ( newObj, keyCursor, &it ) ;
               SAFE_OSS_FREE ( key ) ;
            }
            else
            {
               bson_append_element ( newObj, str.c_str(), &it ) ;
            }
         }
      }
   }
   else if( !_hasID )
   {
      const CHAR *pKey = NULL ;
      bson_iterator *it = bson_iterator_create() ;
      bson_iterator_init( it, &obj ) ;
      while( BSON_EOO != bson_iterator_next ( it ) )
      {
         pKey = bson_iterator_key( it ) ;
         if ( ossStrlen( pKey ) != 3 )
         {
            bson_append_element ( newObj, pKey, it ) ;
         }
         else
         {
            if ( pKey[0] != '_' || pKey[1] != 'i' || pKey[2] != 'd' )
            {
               bson_append_element ( newObj, pKey, it ) ;
            }
         }
      }
      bson_iterator_dispose( it ) ;
   }
   else
   {
      bson_copy ( newObj, &obj ) ;
   }

   bson_finish ( newObj ) ;
   if ( !jsonStr && !_reallocateBuffer( &jsonStr, &jsonSize ) )
   {
      PD_LOG ( PDERROR, "Can not to malloc memory" ) ;
      goto error ;
   }
   do
   {
      rc = bsonToJson ( jsonStr, jsonSize, newObj, FALSE ) ;
      if ( !rc && !_reallocateBuffer ( &jsonStr, &jsonSize ) )
      {
         PD_LOG ( PDERROR, "Failed to convert json string, rc = %d", rc ) ;
         goto error ;
      }
   } while ( !rc ) ;

   rc = _appendStr ( jsonStr ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to append json string, rc = %d", rc ) ;
      goto error ;
   }
done :
   bson_destroy ( &obj ) ;
   bson_dispose ( newObj ) ;
   SAFE_OSS_FREE ( jsonStr ) ;
   PD_TRACE_EXITRC ( SDB__MIGJSONEXTR__EXTRRCD, rc );
   return rc ;
error :
   goto done ;
}
