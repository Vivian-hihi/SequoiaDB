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

migExport::migExport () : _gConnection(0),
                          _gCollectionSpace(0),
                          _gCollection(0),
                          _gCSList(0),
                          _gCLList(0),
                          _gCursor(0),
                          _ID(0),
                          _isOpen(FALSE),
                          _pMigArg(NULL)
{
}

migExport::~migExport ()
{
   if ( _isOpen )
   {
      ossClose ( _file ) ;
   }
   if ( _gConnection )
   {
      sdbDisconnect ( _gConnection ) ;
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
      sdbReleaseConnection ( _gConnection ) ;
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

INT32 migExport::_query()
{
   INT32 rc = SDB_OK ;
   // start creating cursor
   rc = sdbQuery ( _gCollection, NULL, NULL, NULL, NULL, 0, -1,
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
   return rc ;
error:
   goto done ;
}

INT32 migExport::_writeFile( bson *pbson )
{
   INT32  rc = SDB_OK ;
   INT32  bufferSize = 0 ;
   SINT64 writedSize = 0 ;
   SINT64 curWriteSize = 0 ;
   SINT64 tempSize = 0 ;
   CHAR *pBuffer = NULL ;

   if ( _pMigArg->type == MIGEXPRT_CSV )
   {
      rc = _csvEncode.bson2csv( _ID, pbson->data ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to convert bson to csv, rc=%d", rc ) ;
         goto error ;
      }
      rc = _csvEncode.getCSV( _ID, &pBuffer, bufferSize ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to convert bson to csv, rc=%d", rc ) ;
         goto error ;
      }
   }
   else if ( _pMigArg->type == MIGEXPRT_JSON )
   {
      
   }

   if ( bufferSize < 0 )
   {
      rc = SDB_SYS ;
      PD_LOG ( PDERROR, "csv size can not be less than 0, rc=%d", rc ) ;
      goto error ;
   }

   tempSize = (SINT64)bufferSize ;

   while ( tempSize > 0 )
   {
      rc = ossWrite ( &_file, pBuffer + writedSize,
                      tempSize, &curWriteSize ) ;
      if ( rc && SDB_INTERRUPT != rc )
      {
         PD_LOG ( PDERROR, "Failed to write to file, rc = %d", rc ) ;
         goto error ;
      }
      tempSize -= curWriteSize ;
      writedSize += curWriteSize ;
      rc = SDB_OK ;
   }

done:
   return rc ;
error:
   goto done ;
}

INT32 migExport::_run( const CHAR *pCSName, const CHAR *pCLName, INT32 &total )
{
   INT32 rc = SDB_OK ;
   INT32 clTotal = 0 ;
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
   }
done:
   total += clTotal ;
   PD_LOG ( PDEVENT, "%s.%s export record %d in file",pCSName, pCLName, clTotal ) ;
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

   if ( _pMigArg->type == MIGEXPRT_CSV )
   {
      rc = _csvEncode.init( _pMigArg->delChar,
                            _pMigArg->delField,
                            _pMigArg->delRecord ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to csv parser initialize, rc=%d", rc ) ;
         goto error ;
      }
      rc = _csvEncode.parseHeader ( _pMigArg->pFields,
                                    ossStrlen( _pMigArg->pFields ) ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to parse Header, rc=%d", rc ) ;
         goto error ;
      }
      rc = _csvEncode.getID( _ID ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get ID, rc=%d", rc ) ;
         goto error ;
      }
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