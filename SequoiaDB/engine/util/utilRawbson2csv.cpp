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

   Source File Name = utilRawbson2csv.cpp

   Descriptive Name = BSON TO CSV

   When/how to use: this program may be used on binary and text-formatted
   versions of UTIL component. This file contains declare of json2rawbson. Note
   this function should NEVER be directly called other than fromjson.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/03/2014  JWH Initial Draft

   Last Changed =

*******************************************************************************/

#include "utilRawbson2csv.hpp"
#include "rawbson2csv.h"
#include "../client/bson/bson.h"

#define CSV_STR_TABLE   '\t'
#define CSV_STR_CR      '\r'
#define CSV_STR_LF      '\n'
#define CSV_STR_COMMA   ','
#define CSV_STR_SPACE   32
#define CSV_STR_QUOTES  '"'
#define CSV_STR_SLASH   '\\'

CHAR *utilConvertCSV::_trimLeft( CHAR *pCursor, INT32 &size )
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

CHAR *utilConvertCSV::_trimRight ( CHAR *pCursor, INT32 &size )
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

CHAR *utilConvertCSV::_trim ( CHAR *pCursor, INT32 &size )
{
   pCursor = _trimLeft( pCursor, size ) ;
   pCursor = _trimRight( pCursor, size ) ;
   return pCursor ;
}

INT32 utilConvertCSV::_filterString( CHAR **pField, INT32 &size )
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

INT32 utilConvertCSV::init( CHAR delChar, CHAR delField )
{
   INT32 rc = SDB_OK ;
   if ( delChar == delField )
   {
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "delchar does not like delfield" ) ;
      goto error ;
   }
   _delChar = delChar ;
   _delField = delField ;
done:
   return rc ;
error:
   goto done ;
}

utilConvertCSV::utilConvertCSV() : _delChar(0),
                                   _delField(0)
{
}

utilConvertCSV::~utilConvertCSV()
{
   _freeFieldList( NULL ) ;
}

void utilConvertCSV::_freeFieldList( fieldResolve *pFieldRe )
{
   if ( NULL == pFieldRe )
   {
      fieldResolve *pTemp = NULL ;
      INT32 fieldsNum = _vFields.size() ;
      for ( INT32 i = 0; i < fieldsNum; ++i )
      {
         pTemp = _vFields.at( i ) ;
         _freeFieldList( pTemp ) ;
      }
   }
   else
   {
      if ( pFieldRe->pSubField )
      {
         _freeFieldList( pFieldRe->pSubField ) ;
         SAFE_OSS_DELETE( pFieldRe ) ;
      }
      else
      {
         SAFE_OSS_DELETE( pFieldRe ) ;
      }
   }
}

INT32 utilConvertCSV::_parseSubField( CHAR *pField, fieldResolve *pParent )
{
   INT32 rc = SDB_OK ;
   CHAR *pSubField = NULL ;
   fieldResolve *pFieldRe = NULL ;
   pFieldRe = SDB_OSS_NEW fieldResolve() ;
   if ( !pFieldRe )
   {
      rc = SDB_OOM ;
      PD_LOG ( PDERROR, "Failed to malloc memory", rc ) ;
      goto error ;
   }

   if ( pParent )
   {
      pParent->pSubField = pFieldRe ;
   }
   else
   {
      _vFields.push_back( pFieldRe ) ;
   }

   pSubField = ossStrchr( pField, '.' ) ;
   if ( pSubField )
   {
      rc = SDB_INVALIDARG ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Field does not has the\".\" symbol", rc ) ;
         goto error ;
      }
      //off export a.b
      *pSubField = 0 ;
      ++pSubField ;
      pFieldRe->pField = pField ;
      rc = _parseSubField( pSubField, pFieldRe ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to call _parseSubField", rc ) ;
         goto error ;
      }
   }
   else
   {
      pFieldRe->pField = pField ;
      pFieldRe->pSubField = NULL ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 utilConvertCSV::parseFields( CHAR *pFields, INT32 size )
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
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            else
            {
               rc = _filterString( &leftField, fieldSize ) ;
               if ( rc )
               {
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               leftField[ fieldSize ] = 0 ;
               rc = _parseSubField( leftField, NULL ) ;
               if ( rc )
               {
                  PD_LOG ( PDERROR, "Failed to call _parseSubField", rc ) ;
                  goto error ;
               }
            }
         }
         else
         {
            rc = SDB_INVALIDARG ;
            PD_LOG ( PDERROR, "field format error, only one side of \
the field appears \", rc = %d", rc ) ;
            goto error ;
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
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         else
         {
            rc = _filterString( &leftField, fieldSize ) ;
            if ( rc )
            {
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            leftField[ fieldSize ] = 0 ;
            rc = _parseSubField( leftField, NULL ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to call _parseSubField", rc ) ;
               goto error ;
            }
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

INT32 utilConvertCSV::parseCSVSize( CHAR *pbson, INT32 *pCSVSize )
{
   INT32 rc = SDB_OK ;
   rc = getCSVSize( _delChar, _delField, pbson, pCSVSize ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get csv size, rc = %d", rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 utilConvertCSV::_appendBsonElement( void *pObj,
                                          fieldResolve *pFieldRe,
                                          const CHAR *pData )
{
   INT32 rc = SDB_OK ;
   bson *obj = (bson *)pObj ;
   bson subObj ;
   bson_iterator it ;
   bson_type fieldType ;
   bson_init( &subObj ) ;
   bson_init_finished_data( &subObj, pData ) ;

   fieldType = bson_find( &it, &subObj, pFieldRe->pField ) ;
   if ( BSON_EOO == fieldType || BSON_UNDEFINED == fieldType )
   {
      if ( bson_append_undefined( obj, pFieldRe->pField ) )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Failed to call bson_append_undefined, rc = %d", rc ) ;
         goto error ;
      }
      goto done ;
   }

   if ( pFieldRe->pSubField )
   {
      if ( BSON_OBJECT == fieldType || BSON_ARRAY == fieldType )
      {
         rc = _appendBsonElement( obj, pFieldRe->pSubField, bson_iterator_value( &it ) ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to append bson element, rc = %d", rc ) ;
            goto error ;
         }
      }
      else
      {
         if ( bson_append_undefined( obj, pFieldRe->pField ) )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Failed to call bson_append_undefined, rc = %d", rc ) ;
            goto error ;
         }
         goto done ;
      }
   }
   else
   {
      if ( bson_append_element( obj, pFieldRe->pField, &it ) )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Failed to call bson_append_element, rc = %d", rc ) ;
         goto error ;
      }
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 utilConvertCSV::bsonCovertCSV( CHAR *pbson,
                                     CHAR **ppBuffer,
                                     INT32 *pCSVSize )
{
   INT32 rc = SDB_OK ;
   INT32 fieldsNum = 0 ;
   fieldResolve *pFieldRc = NULL ;
   bson obj ;
   bson_init( &obj ) ;

   fieldsNum = _vFields.size() ;
   for ( INT32 i = 0; i < fieldsNum; ++i )
   {
      pFieldRc = _vFields.at( i ) ;
      rc = _appendBsonElement( &obj, pFieldRc, pbson ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to append bson element, rc = %d", rc ) ;
         goto error ;
      }
   }
   bson_finish ( &obj ) ;
   rc = bson2csv( _delChar, _delField, obj.data, ppBuffer, pCSVSize ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to bson convert csv, rc = %d", rc ) ;
      goto error ;
   }
done:
   bson_destroy ( &obj ) ;
   return rc ;
error:
   goto done ;
}