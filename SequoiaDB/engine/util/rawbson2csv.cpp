/*******************************************************************************

   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = csv2rawbson.cpp

   Descriptive Name = CSV To Raw BSON

   When/how to use: this program may be used on binary and text-formatted
   versions of UTIL component. This file contains declare of json2rawbson. Note
   this function should NEVER be directly called other than fromjson.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/04/2014  JWH Initial Draft

   Last Changed =

*******************************************************************************/

#include "rawbson2csv.hpp"
#include "ossUtil.h"
#include "pd.hpp"
#include "../client/bson/bson.h"
#include "time.h"
#include <math.h>
#include "../client/base64c.h"

#define CSV_STR_TABLE   '\t'
#define CSV_STR_CR      '\r'
#define CSV_STR_LF      '\n'
#define CSV_STR_COMMA   ','
#define CSV_STR_QUOTES  '"'
#define CSV_STR_SPACE   32
#define CSV_STR_SLASH   '\\'

#define TIME_FORMAT "%d-%d-%d-%d.%d.%d.%d"
#define DATE_FORMAT "%d-%d-%d"
#define INT32_LAST_YEAR 2038
#define RELATIVE_YEAR 1900
#define RELATIVE_MOD 12
#define RELATIVE_DAY 31
#define RELATIVE_HOUR 24
#define RELATIVE_MIN_SEC 60

#define ENCODE_DEFAULT_SIZE      512
#define ENCODE_ESTIMATE_MULTIPLE 1.5

CHAR *csvEncode::_trimLeft ( CHAR *pCursor, INT32 &size )
{
   for ( INT32 i = 0; i < size; ++i )
   {
      switch( *pCursor )
      {
      case CSV_STR_TABLE:
      case CSV_STR_CR:
      case CSV_STR_LF:
      case CSV_STR_SPACE:
         ++pCursor ;
         break ;
      case 0:
      default:
         size -= i ;
         return pCursor ;
      }
   }
   return pCursor ;
}

CHAR *csvEncode::_trimRight ( CHAR *pCursor, INT32 &size )
{
   for ( INT32 i = 1; i <= size; ++i )
   {
      switch( *( pCursor + ( size - i ) ) )
      {
      case CSV_STR_TABLE:
      case CSV_STR_CR:
      case CSV_STR_LF:
      case CSV_STR_SPACE:
         break ;
      case 0:
      default:
         size -= ( i - 1 ) ;
         return pCursor ;
      }
   }
   return pCursor ;
}

CHAR *csvEncode::_trim ( CHAR *pCursor, INT32 &size )
{
   pCursor = _trimLeft( pCursor, size ) ;
   pCursor = _trimRight( pCursor, size ) ;
   return pCursor ;
}

static void local_time ( time_t *Time, struct tm *TM )
{
   if ( !Time || !TM )
      return ;
#if defined (__linux__ )
   localtime_r( Time, TM ) ;
#elif defined (_WIN32)
   // The Time represents the seconds elapsed since midnight (00:00:00),
   // January 1, 1970, UTC. This value is usually obtained from the time
   // function.
   localtime_s( TM, Time ) ;
#endif
}

#define APPENDSTRING(a,b,c)\
{\
   rc = _appendString( a, b, c ) ;\
   if ( rc )\
   {\
      PD_LOG ( PDERROR, "Failed to append string \"%.s\"",\
               c, b ) ;\
      goto error ;\
   }\
}

INT32 csvEncode::_parseField( CHAR **pField, INT32 &size )
{
   INT32 rc = SDB_OK ;
   CHAR *pBuffer = *pField ;
   if ( pBuffer[0] == CSV_STR_QUOTES &&
        pBuffer[size-1] == CSV_STR_QUOTES )
   {
      ++pBuffer ;
      size -= 2 ;
   }
   *pField = pBuffer ;
   return rc ;
}

INT32 csvEncode::_getEncodeObj( INT32 ID, csvEncodeObj **ppCSVObj )
{
   INT32 rc = SDB_OK ;
   if ( _vObjList.size() == 0 )
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "This ID %d does not exist" ) ;
      goto error ;
   }
   *ppCSVObj = _vObjList.at( ID ) ;
done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::_estimateCSVSize( csvEncodeObj *pCSVObj, INT32 bsonSize )
{
   INT32 rc = SDB_OK ;
   INT32 estimateCSVSize = bsonSize * ENCODE_ESTIMATE_MULTIPLE ;

   if ( estimateCSVSize == 0 )
   {
      estimateCSVSize = ENCODE_DEFAULT_SIZE ;
   }

   if ( estimateCSVSize > pCSVObj->csvBufferSize )
   {
      rc = _rallocCSV( &pCSVObj->pCSVBuffer, estimateCSVSize ) ;
      pCSVObj->csvBufferSize = estimateCSVSize ;
      if ( rc )
      {
         goto error ;
      }
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::_rallocCSV( CHAR **pBuffer, INT32 newSize )
{
   INT32 rc = SDB_OK ;
   CHAR *pNewBuffer = NULL ;

   pNewBuffer = (CHAR *)SDB_OSS_REALLOC( (*pBuffer), newSize ) ;
   if ( !pNewBuffer )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
               newSize ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   *pBuffer = pNewBuffer ;
done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::_appendString( csvEncodeObj *pCSVObj,
                                const CHAR *pBuffer, INT32 size )
{
   INT32 rc = SDB_OK ;
   INT32 unusedSpace = 0 ;
   INT32 newSize = 0 ;
   INT32 isDoubleChar = FALSE ;

   unusedSpace = pCSVObj->csvBufferSize - pCSVObj->csvSize ;

   for ( INT32 i = 0; i < size; )
   {
      if ( unusedSpace == 0 )
      {
         newSize = pCSVObj->csvBufferSize + ENCODE_DEFAULT_SIZE ;
         rc = _rallocCSV( &pCSVObj->pCSVBuffer, newSize ) ;
         if ( rc )
         {
            goto error ;
         }
         pCSVObj->csvBufferSize = newSize ;
         unusedSpace = pCSVObj->csvBufferSize - pCSVObj->csvSize ;
      }
      if ( isDoubleChar )
      {
         *(pCSVObj->pCSVBuffer + pCSVObj->csvSize) = _delChar ;
         isDoubleChar = FALSE ;
      }
      else
      {
         if ( *(pBuffer + i) == _delChar )
         {
            isDoubleChar = TRUE ;
         }
         *(pCSVObj->pCSVBuffer + pCSVObj->csvSize) = *(pBuffer + i) ;
         ++i ;
      }
      ++(pCSVObj->csvSize) ;
      --unusedSpace ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::_appendObj( csvEncodeObj *pCSVObj, void *pBson_iterator )
{
   INT32 rc = SDB_OK ;
   INT32 newSize = 0 ;
   INT32 size    = 0 ;
   bson_iterator *pIt = (bson_iterator *)pBson_iterator ;

   size = bson_sprint_length_iterator ( pIt ) ;

   if ( size > pCSVObj->tempBufferSize )
   {
      newSize = pCSVObj->tempBufferSize +
            ( ENCODE_DEFAULT_SIZE > size ? ENCODE_DEFAULT_SIZE : size ) ;
      rc = _rallocCSV( &pCSVObj->pTempBuffer, newSize ) ;
      pCSVObj->tempBufferSize = newSize ;
      if ( rc )
      {
         goto error ;
      }
   }

   if( !bson_sprint_iterator ( &pCSVObj->pTempBuffer,
                               &pCSVObj->tempBufferSize,
                               pIt, '"' ) )
   {
      PD_LOG ( PDERROR, "Failed to sprint iterator" ) ;
      rc = SDB_CORRUPTED_RECORD ;
      goto error ;
   }
   APPENDSTRING( pCSVObj, pCSVObj->pTempBuffer, size ) ;
done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::_appendNonString( csvEncodeObj *pCSVObj,
                                   void *pBson_iterator )
{
   INT32 rc = SDB_OK ;
   INT32 unusedSpace = 0 ;
   INT32 newSize     = 0 ;
   INT32 size        = 0 ;
   CHAR *pTempBuf    = NULL ;
   bson_iterator *pIt = (bson_iterator *)pBson_iterator ;

   size = bson_sprint_length_iterator( pIt ) ;
   unusedSpace = pCSVObj->csvBufferSize - pCSVObj->csvSize ;

   if ( size > unusedSpace )
   {
      if ( size - unusedSpace >= ENCODE_DEFAULT_SIZE )
      {
         newSize = pCSVObj->csvBufferSize +
               ( size - unusedSpace + ENCODE_DEFAULT_SIZE ) ;
      }
      else
      {
         newSize = pCSVObj->csvBufferSize + ENCODE_DEFAULT_SIZE ;
      }
      rc = _rallocCSV( &pCSVObj->pCSVBuffer, newSize ) ;
      if ( rc )
      {
         goto error ;
      }
      pCSVObj->csvBufferSize = newSize ;
   }
   pTempBuf = pCSVObj->pCSVBuffer + pCSVObj->csvSize ;
   unusedSpace = pCSVObj->csvBufferSize - pCSVObj->csvSize ;
   if ( !bson_sprint_iterator ( &pTempBuf, &unusedSpace,
                                pIt, _delChar ) )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
               newSize ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   pCSVObj->csvSize = ( pCSVObj->csvBufferSize - unusedSpace ) ;
done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::_appendValue( csvEncodeObj *pCSVObj, void *pBson_iterator )
{
   INT32 rc = SDB_OK ;
   bson_iterator *pIt = (bson_iterator *)pBson_iterator ;
   bson_type type = bson_iterator_type( pIt ) ;
   INT32 tempSize = 0 ;
   INT32 base64Size = 0 ;
   CHAR temp[128] = { 0 } ;
   const CHAR *pTemp = NULL ;
   CHAR *pBase64 = NULL ;
   bson_timestamp_t ts;
   time_t timer ;
   struct tm psr;

   if ( type == BSON_DOUBLE || type == BSON_BOOL ||
        type == BSON_NULL || type == BSON_INT ||
        type == BSON_LONG )
   {
      rc = _appendNonString( pCSVObj, pBson_iterator ) ;
      if ( rc )
      {
         goto error ;
      }
   }
   else
   {
      APPENDSTRING( pCSVObj, &_delChar, 1 ) ;
      if ( type == BSON_TIMESTAMP )
      {
         ts = bson_iterator_timestamp( pIt );
         timer = (time_t)ts.t;
         local_time( &timer, &psr ) ;
         tempSize = ossSnprintf ( temp, 64,
                                  "%04d-%02d-%02d-%02d.%02d.%02d.%06d",
                                  psr.tm_year + 1900,
                                  psr.tm_mon + 1,
                                  psr.tm_mday,
                                  psr.tm_hour,
                                  psr.tm_min,
                                  psr.tm_sec,
                                  ts.i ) ;
         APPENDSTRING( pCSVObj, temp, tempSize ) ;
      }
      else if ( type == BSON_DATE )
      {
         timer = bson_iterator_date( pIt );
         local_time( &timer, &psr ) ;
         tempSize = ossSnprintf ( temp, 64, "%04d-%02d-%02d",
                                  psr.tm_year + 1900,
                                  psr.tm_mon + 1,
                                  psr.tm_mday ) ;
         APPENDSTRING( pCSVObj, temp, tempSize ) ;
      }
      else if ( type == BSON_UNDEFINED )
      {
         APPENDSTRING( pCSVObj, CSV_STR_UNDEFINED, CSV_STR_UNDEFINED_SIZE ) ;
      }
      else if ( type == BSON_MINKEY )
      {
         APPENDSTRING( pCSVObj, CSV_STR_MINKEY, CSV_STR_MINKEY_SIZE ) ;
      }
      else if ( type == BSON_MAXKEY )
      {
         APPENDSTRING( pCSVObj, CSV_STR_MAXKEY, CSV_STR_MAXKEY_SIZE ) ;
      }
      else if ( type == BSON_CODE )
      {
         pTemp = bson_iterator_code( pIt ) ;
         APPENDSTRING( pCSVObj, pTemp, ossStrlen( pTemp ) ) ;
      }
      else if ( type == BSON_STRING || type == BSON_SYMBOL )
      {
         pTemp = bson_iterator_string( pIt ) ;
         APPENDSTRING( pCSVObj, pTemp, ossStrlen( pTemp ) ) ;
      }
      else if ( type == BSON_BINDATA )
      {
         pTemp = bson_iterator_bin_data( pIt ) ;
         tempSize = bson_iterator_bin_len ( pIt ) - 1 ;
         base64Size = getEnBase64Size ( tempSize ) ;
         pBase64 = (CHAR *)SDB_OSS_MALLOC( base64Size ) ;
         ossMemset( pBase64, 0, base64Size ) ;
         if ( !base64Encode( pTemp, tempSize, pBase64, base64Size ) )
         {
            SAFE_OSS_FREE( pBase64 ) ;
            PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
                     base64Size ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         APPENDSTRING( pCSVObj, pBase64, base64Size - 1 ) ;
         SAFE_OSS_FREE( pBase64 ) ;
      }
      else if ( type == BSON_REGEX )
      {
         pTemp = bson_iterator_regex( pIt ) ;
         APPENDSTRING( pCSVObj, pTemp, ossStrlen( pTemp ) ) ;
      }
      else if ( type == BSON_OID )
      {
         bson_oid_to_string( bson_iterator_oid( pIt ), temp ) ;
         APPENDSTRING( pCSVObj, temp, 24 ) ;
      }
      else
      {
         rc = _appendObj( pCSVObj, pBson_iterator ) ;
         if ( rc )
         {
            goto error ;
         }
      }
      APPENDSTRING( pCSVObj, &_delChar, 1 ) ;
   }
done:
   return rc ;
error:
   goto done ;
}

csvEncode::csvEncode() : _delChar(0),
                         _delField(0),
                         _delRecord(0)
{
}

csvEncode::~csvEncode()
{
   std::vector<csvEncodeObj *>::iterator it ;
   csvEncodeObj *pCSVObj = NULL ;

   for( it = _vObjList.begin(); it != _vObjList.end(); ++it )
   {
      pCSVObj = *it ;
      if ( pCSVObj )
      {
         SAFE_OSS_FREE( pCSVObj->pCSVBuffer ) ;
         SAFE_OSS_FREE( pCSVObj->pTempBuffer ) ;
         SAFE_OSS_DELETE( pCSVObj ) ;
      }
   }
}

INT32 csvEncode::parseHeader( CHAR *pFields, INT32 size )
{
   INT32   rc         = SDB_OK ;
   INT32   tempRc     = SDB_OK ;
   INT32   fieldSize  = 0 ;
   BOOLEAN isString   = FALSE;
   CHAR   *pCursor    = pFields ;
   CHAR   *leftField  = pFields ;

   do
   {
      if ( 0 == size )
      {
         if ( !isString )
         {
            fieldSize = pCursor - leftField ;
            leftField = _trim( leftField, fieldSize ) ;
            if ( fieldSize == 0 )
            {
               PD_LOG ( PDERROR, "The field can not be an empty string, \
if need the space string field, please use \"\"" ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            else
            {
               rc = _parseField( &leftField, fieldSize ) ;
               if ( rc )
               {
                  PD_LOG ( PDERROR, "Faild to call _parseField, rc=%d",
                           rc ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               leftField[ fieldSize ] = 0 ;
               _vField.push_back ( leftField ) ;
            }
         }
         break ;
      }

      if ( CSV_STR_QUOTES == *pCursor )
      {
         --size ;
         ++pCursor ;
         isString = !isString ;
      }
      else if ( !isString &&
                ( CSV_STR_COMMA == *pCursor || CSV_STR_LF == *pCursor ) )
      {
         fieldSize = pCursor - leftField ;
         leftField = _trim( leftField, fieldSize ) ;
         if ( CSV_STR_LF == *pCursor )
         {
            tempRc = SDB_UTIL_CSV_FIELD_END ;
         }
         if ( fieldSize == 0 )
         {
            PD_LOG ( PDERROR, "The field can not be an empty string, \
if need the space string field, please use \"" ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         else
         {
            rc = _parseField( &leftField, fieldSize ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Faild to call _parseField, rc=%d",
                        rc ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            leftField[ fieldSize ] = 0 ;
            _vField.push_back ( leftField ) ;
         }

         if ( tempRc == SDB_UTIL_CSV_FIELD_END )
         {
            break ;
         }
         else
         {
            --size ;
            ++pCursor ;
            leftField = pCursor ;
         }
      }
      else
      {
         --size ;
         ++pCursor ;
      }
   }while ( TRUE ) ;

done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::init( CHAR delChar,
                       CHAR delField,
                       CHAR delRecord )
{
   INT32 rc = SDB_OK ;

   if ( delChar == delField )
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "delchar does not like delfield" ) ;
      goto error ;
   }
   if ( delChar == delRecord )
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "delchar does not like delrecord" ) ;
      goto error ;
   }
   if ( delField == delRecord )
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "delfield does not like delrecord" ) ;
      goto error ;
   }

   _delChar      = delChar ;
   _delField     = delField ;
   _delRecord    = delRecord ;
done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::getID( INT32 &ID )
{
   INT32 rc = SDB_OK ;
   csvEncodeObj *pCSVObj = NULL ;

   pCSVObj = SDB_OSS_NEW csvEncodeObj() ;
   if ( !pCSVObj )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory" ) ;
      rc = SDB_OOM ;
      goto error ;
   }

   pCSVObj->pCSVBuffer = (CHAR *)SDB_OSS_MALLOC( ENCODE_DEFAULT_SIZE ) ;
   if ( !pCSVObj->pCSVBuffer )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
               ENCODE_DEFAULT_SIZE ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   pCSVObj->csvBufferSize = ENCODE_DEFAULT_SIZE ;

   pCSVObj->pTempBuffer = (CHAR *)SDB_OSS_MALLOC( ENCODE_DEFAULT_SIZE ) ;
   if ( !pCSVObj->pTempBuffer )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
               ENCODE_DEFAULT_SIZE ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   pCSVObj->tempBufferSize = ENCODE_DEFAULT_SIZE ;

   ID = _vObjList.size() ;
   _vObjList.push_back( pCSVObj ) ;

done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::getCSV( INT32 ID, CHAR **ppBuffer, INT32 &size )
{
   INT32 rc = SDB_OK ;
   csvEncodeObj *pCSVObj = NULL ;

   rc = _getEncodeObj( ID, &pCSVObj ) ;
   if ( rc )
   {
      goto error ;
   }

   *ppBuffer = pCSVObj->pCSVBuffer ;
   size = pCSVObj->csvSize ;

done:
   return rc ;
error:
   goto done ;
}

INT32 csvEncode::bson2csv( INT32 ID, CHAR *pbson ) 
{
   INT32 rc = SDB_OK ;
   BOOLEAN isFirst = TRUE ;
   INT32 fieldSumNum = _vField.size() ;
   csvEncodeObj *pCSVObj = NULL ;
   bson_type fieldType ;
   bson_iterator it ;
   bson obj ;

   bson_init ( &obj ) ;
   obj.ownmem = 0 ;
   obj.data = NULL ;
   bson_init_finished_data ( &obj, pbson ) ;

   rc = _getEncodeObj( ID, &pCSVObj ) ;
   if ( rc )
   {
      goto error ;
   }

   pCSVObj->csvSize = 0 ;

   rc = _estimateCSVSize( pCSVObj, obj.dataSize ) ;
   if ( rc )
   {
      goto error ;
   }

   for ( INT32 i = 0; i < fieldSumNum; ++i )
   {
      fieldType = bson_find ( &it, &obj, _vField.at( i ) ) ;
      // if BSON_EOO == fieldType ( which is 0 ), that means we hit end of object
      if ( BSON_EOO == fieldType )
      {
         break ;
      }
      // do NOT concat "," for first entrance
      if ( isFirst )
      {
         isFirst = FALSE ;
      }
      else
      {
         APPENDSTRING( pCSVObj, &_delField, 1 ) ;
      }
      //then we check the data type
      rc = _appendValue( pCSVObj, &it ) ;
      if ( rc )
      {
         goto error ;
      }
   }
   APPENDSTRING( pCSVObj, &_delRecord, 1 ) ;
done:
   return rc ;
error:
   goto done ;
}
