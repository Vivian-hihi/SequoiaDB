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
          05/22/2014  JWH
   Last Changed =

*******************************************************************************/

#include "migExport.hpp"
#include "ossMem.hpp"
#include "ossUtil.hpp"
#include "ossIO.hpp"
#include "pdTrace.hpp"
#include "migTrace.hpp"
#include "../util/json2rawbson.h"
#include "../client/jstobs.h"
#include "msgDef.h"
#include "msg.h"

#define CSV_STR_TABLE   '\t'
#define CSV_STR_CR      '\r'
#define CSV_STR_LF      '\n'
#define CSV_STR_COMMA   ','
#define CSV_STR_SPACE   32
#define CSV_STR_QUOTES  '"'
#define CSV_STR_SLASH   '\\'

migExport::migExport () : _gConnection(0),
                          _gCollectionSpace(0),
                          _gCollection(0),
                          _gCSList(0),
                          _gCLList(0),
                          _gCursor(0),
                          _bufferSize(0),
                          _isOpen(FALSE),
                          _pMigArg(NULL),
                          _pBuffer(NULL)
{
}

migExport::~migExport ()
{
   if ( _isOpen )
   {
      ossClose ( _file ) ;
   }
   if ( _gCursor )
   {
      sdbCloseCursor( _gCursor ) ;
      sdbReleaseCursor( _gCursor ) ;
   }
   if ( _gCSList )
   {
      sdbCloseCursor( _gCSList ) ;
      sdbReleaseCursor( _gCSList ) ;
   }
   if ( _gCLList )
   {
      sdbCloseCursor( _gCLList ) ;
      sdbReleaseCursor( _gCLList ) ;
   }
   if ( _gCollection )
   {
      sdbReleaseCollection ( _gCollection ) ;
   }
   if ( _gCollectionSpace )
   {
      sdbReleaseCS ( _gCollectionSpace ) ;
   }
   if ( _gConnection )
   {
      sdbDisconnect ( _gConnection ) ;
   }
   SAFE_OSS_FREE( _pBuffer ) ;
}

CHAR *migExport::_trimLeft( CHAR *pCursor, INT32 &size )
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

CHAR *migExport::_trimRight ( CHAR *pCursor, INT32 &size )
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

CHAR *migExport::_trim ( CHAR *pCursor, INT32 &size )
{
   pCursor = _trimLeft( pCursor, size ) ;
   pCursor = _trimRight( pCursor, size ) ;
   return pCursor ;
}

INT32 migExport::_filterString( CHAR **pField, INT32 &size )
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

BOOLEAN lessFields( const CHAR *pField1, const CHAR *pField2 )
{
   return ( ossStrcmp( pField1, pField2 ) < 0 ) ;
}

INT32 migExport::_parseFields( CHAR *pFields, INT32 size, bson &obj )
{
   INT32   rc         = SDB_OK ;
   INT32   tempRc     = SDB_OK ;
   INT32   fieldSize  = 0 ;
   BOOLEAN isString   = FALSE;
   CHAR   *pCursor    = pFields ;
   CHAR   *leftField  = pFields ;
   bson_init( &obj ) ;
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
               bson_append_undefined( &obj, leftField ) ;
               _vFields.push_back( leftField ) ;
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
            bson_append_undefined( &obj, leftField ) ;
            _vFields.push_back( leftField ) ;
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
   bson_finish ( &obj ) ;
   std::sort( _vFields.begin(), _vFields.end(), lessFields ) ;
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_connectDB()
{
   INT32 rc = SDB_OK ;
   // connection is established
   rc = sdbConnect ( _pMigArg->pHostname, _pMigArg->pSvcname,
                     _pMigArg->pUser, _pMigArg->pPassword,
                     &_gConnection ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to connect database %s:%s, rc = %d",
               _pMigArg->pHostname, _pMigArg->pSvcname, rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_getCSList()
{
   INT32 rc = SDB_OK ;
   rc = sdbListCollectionSpaces( _gConnection, &_gCSList ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection space list, rc = %d", rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_getCLList()
{
   INT32 rc = SDB_OK ;
   rc = sdbListCollections( _gConnection, &_gCLList ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection list, rc = %d", rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_getCS( const CHAR *pCSName )
{
   INT32 rc = SDB_OK ;
   // get collection space
   rc = sdbGetCollectionSpace ( _gConnection, pCSName,
                                &_gCollectionSpace ) ;
   if ( SDB_DMS_CS_NOTEXIST == rc )
   {
      PD_LOG ( PDERROR, "Collection space %s does not exist",
               pCSName ) ;
      goto error ;
   }
   else if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection space %s, rc = %d",
               pCSName, rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_getCL( const CHAR *pCLName )
{
   INT32 rc = SDB_OK ;
   // get collection
   rc = sdbGetCollection1 ( _gCollectionSpace, pCLName,
                            &_gCollection ) ;
   if ( SDB_DMS_NOTEXIST == rc )
   {
      PD_LOG ( PDERROR, "Collection %s does not exist"OSS_NEWLINE,
               pCLName ) ;
      goto error ;
   }
   else if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection %s, rc = %d",
               pCLName, rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_writeInclude()
{
   INT32 rc = SDB_OK ;
   INT32 fieldsNum = 0 ;
   SINT64 writedSize = 0 ;
   SINT64 curWriteSize = 0 ;
   SINT64 fieldSize = 0 ;
   CHAR *pField = NULL ;

   if ( _pMigArg->type == MIGEXPRT_CSV &&
        _pMigArg->include == TRUE )
   {
      fieldsNum = _vFields.size() ;
      for ( INT32 i = 0; i < fieldsNum; ++i )
      {
         pField = _vFields.at( i ) ;
         fieldSize = (SINT64)ossStrlen( pField ) ;
         if ( i + 1 == fieldsNum )
         {
            pField[ fieldSize ] = _pMigArg->delRecord ;
         }
         else
         {
            pField[ fieldSize ] = _pMigArg->delField ;
         }
         ++fieldSize ;
         writedSize = 0 ;
         while ( fieldSize > 0 )
         {
            rc = ossWrite ( &_file, pField + writedSize,
                            fieldSize, &curWriteSize ) ;
            if ( rc && SDB_INTERRUPT != rc )
            {
               PD_LOG ( PDERROR, "Failed to write to file, rc = %d", rc ) ;
               goto error ;
            }
            fieldSize -= curWriteSize ;
            writedSize += curWriteSize ;
            rc = SDB_OK ;
         }
      }
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_query()
{
   INT32 rc = SDB_OK ;
   bson obj ;
   if ( _pMigArg->pFields )
   {
      rc = _parseFields ( _pMigArg->pFields,
                          ossStrlen( _pMigArg->pFields ),
                          obj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to parse Header, rc=%d", rc ) ;
         goto error ;
      }
      rc = _writeInclude() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to call _writeInclude, rc=%d", rc ) ;
         goto error ;
      }
   }
   else
   {
      bson_init( &obj ) ;
      bson_empty( &obj ) ;
   }
   // start creating cursor
   rc = sdbQuery ( _gCollection, NULL, &obj, NULL, NULL, 0, -1,
                   &_gCursor ) ;
   if ( rc )
   {
      if ( SDB_DMS_EOC != rc )
      {
         PD_LOG ( PDERROR, "Failed to query collection, rc = %d", rc ) ;
         goto error ;
      }
      else
      {
         goto done ;
      }
   }
done:
   bson_destroy( &obj ) ;
   return rc ;
error:
   goto done ;
}

INT32 migExport::_reallocBuffer( CHAR **ppBuffer, INT32 size, INT32 newSize )
{
   INT32 rc = SDB_OK ;
   CHAR *pTemp = NULL ;
   if ( newSize > size )
   {
      pTemp = (CHAR *)SDB_OSS_REALLOC( *ppBuffer, newSize ) ;
      if ( !pTemp )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "out of memory, rc=%d", rc ) ;
         goto error ;
      }
      *ppBuffer = pTemp ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_writeFile( bson *pbson )
{
   INT32  rc = SDB_OK ;
   INT32  bufferSize = 0 ;
   INT32  tempSize = 0 ;
   SINT64 writedSize = 0 ;
   SINT64 curWriteSize = 0 ;
   SINT64 bufferSize2 = 0 ;
   CHAR *pTemp = NULL ;

   if ( _pMigArg->type == MIGEXPRT_CSV )
   {
      rc = getCSVSize( _pMigArg->delChar,
                       _pMigArg->delField,
                       _pMigArg->delRecord,
                       pbson->data, &bufferSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get csv size, rc=%d", rc ) ;
         goto error ;
      }
      if ( _bufferSize < bufferSize )
      {
         rc = _reallocBuffer( &_pBuffer, _bufferSize, bufferSize ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to realloc memory, rc=%d", rc ) ;
            goto error ;
         }
         _bufferSize = bufferSize ;
      }
      pTemp = _pBuffer ;
      tempSize = bufferSize ;
      rc = bson2csv( _pMigArg->delChar,
                     _pMigArg->delField,
                     _pMigArg->delRecord,
                     pbson->data, &pTemp, &tempSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to convert bson to csv, rc=%d", rc ) ;
         goto error ;
      }
      bufferSize -= tempSize ;
   }
   else if ( _pMigArg->type == MIGEXPRT_JSON )
   {
      bufferSize = bson_sprint_length ( pbson ) + 1 ;
      if ( bufferSize == 0 )
      {
         PD_LOG ( PDERROR, "Failed to get json size, rc=%d", rc ) ;
         goto error ;
      }
      if ( _bufferSize < bufferSize )
      {
         rc = _reallocBuffer( &_pBuffer, _bufferSize, bufferSize ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to realloc memory, rc=%d", rc ) ;
            goto error ;
         }
         _bufferSize = bufferSize ;
      }
      ossMemset( _pBuffer, 0, _bufferSize ) ;
      if ( !bsonToJson ( _pBuffer, _bufferSize, pbson, FALSE, TRUE ) )
      {
         PD_LOG ( PDERROR, "Failed to convert bson to json, rc=%d", rc ) ;
         goto error ;
      }
      bufferSize = ossStrlen( _pBuffer ) ;
      _pBuffer[ bufferSize ] = _pMigArg->delRecord ;
      ++bufferSize ;
   }

   if ( bufferSize < 0 )
   {
      rc = SDB_SYS ;
      PD_LOG ( PDERROR, "csv size can not be less than 0, rc=%d", rc ) ;
      goto error ;
   }

   bufferSize2 = (SINT64)bufferSize ;

   while ( bufferSize2 > 0 )
   {
      rc = ossWrite ( &_file, _pBuffer + writedSize,
                      bufferSize2, &curWriteSize ) ;
      if ( rc && SDB_INTERRUPT != rc )
      {
         PD_LOG ( PDERROR, "Failed to write to file, rc = %d", rc ) ;
         goto error ;
      }
      bufferSize2 -= curWriteSize ;
      writedSize += curWriteSize ;
      rc = SDB_OK ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_exportCL( const CHAR *pCSName,
                            const CHAR *pCLName,
                            INT32 &total )
{
   INT32 rc = SDB_OK ;
   SINT64 writedSize = 0 ;
   SINT64 curWriteSize = 0 ;
   INT32 clTotal = 0 ;
   bson obj ;

   bson_init( &obj ) ;
   _gCollection = 0 ;
   _gCollectionSpace = 0 ;

   rc = _getCS( pCSName ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection space, rc = %d",
               rc ) ;
      goto error ;
   }
   rc = _getCL( pCLName ) ;
   if ( rc )
   {
      if ( rc == SDB_DMS_NOTEXIST )
      {
         rc = SDB_OK ;
         goto done ;
      }
      else
      {
         PD_LOG ( PDERROR, "Failed to get collection, rc = %d",
                  rc ) ;
         goto error ;
      }
   }
   rc = _query() ;
   if ( rc )
   {
      if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_OK ;
         goto done ;
      }
      else
      {
         PD_LOG ( PDERROR, "Failed to get record, rc = %d",
                  rc ) ;
         goto error ;
      }
   }
   while ( TRUE )
   {
      rc = sdbNext( _gCursor, &obj ) ;
      if ( rc )
      {
         if ( SDB_DMS_EOC != rc )
         {
            PD_LOG ( PDERROR, "Failed to get collection list, rc = %d",
                     rc ) ;
            goto error ;
         }
         else
         {
            rc = SDB_OK ;
            goto done ;
         }
      }
      rc = _writeFile( &obj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to write record to file, rc = %d",
                  rc ) ;
         goto error ;
      }
      bson_destroy ( &obj ) ;
      ++clTotal ;
   }
done:
   total += clTotal ;
   bson_destroy ( &obj ) ;
   if ( _gCollection )
   {
      sdbReleaseCollection ( _gCollection ) ;
   }
   if ( _gCollectionSpace )
   {
      sdbReleaseCS ( _gCollectionSpace ) ;
   }
   PD_LOG ( PDEVENT, "%s.%s export record %d in file",pCSName, pCLName, clTotal ) ;
   return rc ;
error:
   goto done ;
}

INT32 migExport::_run( const CHAR *pCSName, const CHAR *pCLName, INT32 &total )
{
   INT32 rc = SDB_OK ;
   const CHAR *pTemp = NULL ;
   bson obj ;
   bson_iterator it ;
   bson_type type ;

   bson_init( &obj ) ;
   if ( ( pCSName == NULL && pCLName == NULL ) ||
        ( pCSName == NULL && pCLName != NULL ) )
   {
      //never cs cl
      rc = _getCSList() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get collection space list, rc = %d",
                  rc ) ;
         goto error ;
      }
      while( TRUE )
      {
         rc = sdbNext( _gCSList, &obj ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC != rc )
            {
               PD_LOG ( PDERROR, "Failed to get collection space list, rc = %d",
                        rc ) ;
               goto error ;
            }
            else
            {
               rc = SDB_OK ;
               goto done ;
            }
         }
         type = bson_find( &it, &obj, "Name" ) ;
         if ( type != BSON_STRING )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDERROR, "List collection space does not string, rc = %d",
                     rc ) ;
            goto error ;
         }
         pTemp = bson_iterator_string( &it ) ;
         rc = _run( pTemp, pCLName, total ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Faild to call _run, rc = %d", rc ) ;
            goto error ;
         }
      }
   }
   else if ( pCSName != NULL && pCLName == NULL )
   {
      //cs
      rc = _getCLList() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get collection list, rc = %d",
                  rc ) ;
         goto error ;
      }
      while ( TRUE )
      {
         rc = sdbNext( _gCLList, &obj ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC != rc )
            {
               PD_LOG ( PDERROR, "Failed to get collection list, rc = %d",
                        rc ) ;
               goto error ;
            }
            else
            {
               rc = SDB_OK ;
               goto done ;
            }
         }
         type = bson_find( &it, &obj, "Name" ) ;
         if ( type != BSON_STRING )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDERROR, "List collection does not string, rc = %d",
                     rc ) ;
            goto error ;
         }
         pTemp = bson_iterator_string( &it ) ;
         rc = _run( pCSName, pTemp, total ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Faild to call _run, rc = %d", rc ) ;
            goto error ;
         }
      }
   }
   else
   {
      //cs and cl
      rc = _exportCL( pCSName, pCLName, total ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Faild to call _export, rc = %d", rc ) ;
         goto error ;
      }
   }
done:
   bson_destroy ( &obj ) ;
   return rc ;
error:
   goto done ;
}

INT32 migExport::init( migExprtArg *pMigArg )
{
   INT32 rc = SDB_OK ;

   _pMigArg = pMigArg ;

   rc = _connectDB() ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to connect database, rc=%d", rc ) ;
      goto error ;
   }

   // open output file
   rc = ossOpen ( _pMigArg->pFile,
                  OSS_REPLACE | OSS_WRITEONLY | OSS_EXCLUSIVE,
                  OSS_RU | OSS_WU | OSS_RG,
                  _file ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to open file %s, rc = %d",
               _pMigArg->pFile, rc ) ;
      goto error ;
   }
   _isOpen = TRUE ;

done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::run( INT32 &total )
{
   INT32 rc = SDB_OK ;
   rc = _run( _pMigArg->pCSName, _pMigArg->pCLName, total ) ;
   if ( rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}
