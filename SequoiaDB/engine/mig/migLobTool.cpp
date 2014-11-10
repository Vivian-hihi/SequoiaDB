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

   Source File Name = migLobTool.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/31/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "migLobTool.hpp"
#include "msgDef.hpp"

using namespace bson ;
using namespace sdbclient ;

namespace lobtool 
{
const UINT32 BUF_SIZE = 1024 * 1024 ;

   _migLobTool::_migLobTool()
   :_buf( NULL ),
    _bufSize( 0 ),
    _written( 0 )
   {
      _bufSize = BUF_SIZE ;
      _buf = ( CHAR * )ossAlignedAlloc( OSS_FILE_DIRECT_IO_ALIGNMENT,
                                        BUF_SIZE ) ;
   }

   _migLobTool::~_migLobTool()
   {
      if ( _file.isOpened() )
      {
         ossClose( _file ) ;
      }

      if ( NULL != _buf )
      {
         SDB_OSS_ORIGINAL_FREE( _buf ) ;
      }
   }

   INT32 _migLobTool::exec( const bson::BSONObj &options )
   {
      INT32 rc = SDB_OK ;
      sdbclient::sdb db ;
      sdbclient::sdbCollection cl ;
      migOptions ops ;
      BSONElement ele ;

      if ( NULL == _buf )
      {
         PD_LOG( PDERROR, "failed to allocate memory" ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      ele = options.getField( MIG_HOSTNAME ) ;
      if ( String != ele.type() )
      {
         PD_LOG( PDERROR, "invalid type of %s, options:%s",
                 MIG_HOSTNAME, options.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      ops.hostname = ele.valuestr() ;

      ele = options.getField( MIG_SERVICE ) ;
      if ( String != ele.type() )
      {
         PD_LOG( PDERROR, "invalid type of %s, options:%s",
                 MIG_SERVICE, options.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      ops.service = ele.valuestr() ;

      ele = options.getField( MIG_FILE ) ;
      if ( String != ele.type() )
      {
         PD_LOG( PDERROR, "invalid type of %s, options:%s",
                 MIG_FILE, options.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      ops.file = ele.valuestr() ;
      ele = options.getField( MIG_USRNAME ) ;
      if ( ele.eoo() )
      {
         ops.usrname = "" ;
      }
      else if ( String != ele.type() )
      {
         PD_LOG( PDERROR, "invalid type of %s, options:%s",
                 MIG_USRNAME, options.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         ops.usrname = ele.valuestr() ;
      }

      ele = options.getField( MIG_PASSWD ) ;
      if ( ele.eoo() )
      {
         ops.passwd = "" ;
      }
      else if ( String != ele.type() )
      {
         PD_LOG( PDERROR, "invalid type of %s, options:%s",
                 MIG_PASSWD, options.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         ops.passwd = ele.valuestr() ;
      }

      ele = options.getField( MIG_CL ) ;
      if ( String != ele.type() )
      {
         PD_LOG( PDERROR, "invalid type of %s, options:%s",
                 MIG_CL, options.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      ops.collection = ele.valuestr() ;

      ele = options.getField( MIG_OP ) ;
      if ( String != ele.type() )
      {
         PD_LOG( PDERROR, "invalid type of %s, options:%s",
                 MIG_OP, options.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( 0 == ossStrcmp( ele.valuestr(), MIG_OP_IMPRT ) )
      {
         ops.type = MIG_OP_TYPE_IMPRT ;
      }
      else if ( 0 == ossStrcmp( ele.valuestr(), MIG_OP_EXPRT ) )
      {
         ops.type = MIG_OP_TYPE_EXPRT ;
      }
      else
      {
         PD_LOG( PDERROR, "unknown operation type:%s", ele.valuestr() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = _initDB( ops ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init db:%s, rc:%d",
                 options.toString( FALSE, TRUE ).c_str(), rc ) ;
         goto error ;
      }

      if ( MIG_OP_TYPE_EXPRT == ops.type )
      {
         rc = _exportLob( ops ) ;
      }

      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to complete operation, rc:%d", rc ) ;
         goto error ;
      }

   done:
      _closeDB() ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _migLobTool::_exportLob( const migOptions &ops )
   {
      INT32 rc = SDB_OK ;
      bson::BSONObj obj ;
      sdbclient::sdbCursor cursor ;
      UINT64 totalNum = 0 ;

      PD_LOG( PDEVENT, "begin to export lob" ) ;
      
      rc = _createFile( ops.file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create file:%s, rc:%d",
                 ops.file, rc ) ;
         goto error ;
      }
      
      rc = _cl.listLobs( cursor ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to list lobs in collection:%s, rc:%d",
                 ops.collection, rc ) ;
         goto error ;
      }

      do
      {
         rc = cursor.next( obj ) ;
         if ( SDB_OK == rc )
         {
            BSONElement available = obj.getField( FIELD_NAME_LOB_AVAILABLE ) ;
            if ( Bool != available.type() )
            {
               PD_LOG( PDERROR, "invalid object:%s",
                       obj.toString( FALSE, TRUE ).c_str() ) ;
               rc = SDB_SYS ;
               goto error ;
            }

            if ( !available.Bool() )
            {
               continue ;
            }

            rc = _append2File( obj ) ;
            if ( SDB_FNE == rc ||
                 SDB_LOB_IS_NOT_AVAILABLE == rc ||
                 SDB_LOB_SEQUENCE_NOT_EXIST == rc )
            {
               PD_LOG( PDWARNING, "lob[%s] may be removed when export it, rc:%d",
                       obj.toString( FALSE, TRUE ).c_str(), rc ) ;
               rc = SDB_OK ;
            }
            else if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to append lob[%s] to file, rc:%d",
                       obj.toString( FALSE, TRUE ).c_str(), rc ) ;
               goto error ;
            }
            else
            {
               ++totalNum ;
            }
         }
         else if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         else
         {
            PD_LOG( PDERROR, "failed to fetch next record:%d", rc ) ;
            goto error ;
         }

      } while ( TRUE ) ;

      rc = _refreshHeader( totalNum ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to refresh header:%d", rc ) ;
         goto error ;
      }

      PD_LOG( PDEVENT, "lob exporting has been done, total num:%lld", totalNum ) ;
      cout << "lob exporting has been done, total num:" << totalNum << endl ;
   done:
      cursor.close() ;
      if ( _file.isOpened() )
      {
         ossClose( _file ) ;
      }
      return rc ;
   error:
      if ( _file.isOpened() )
      {
         ossClose( _file ) ;
         ossDelete( ops.file ) ;
      }
      goto done ;
   }

   INT32 _migLobTool::_refreshHeader( UINT64 totalNum )
   {
      INT32 rc = SDB_OK ;
      migFileHeader *header = NULL ;

      _initFileHeader( ( migFileHeader * )_buf ) ;
      header = ( migFileHeader * )_buf ;
      header->totalNum = totalNum ;
      rc = ossSeek( &_file, 0, OSS_SEEK_SET ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek to the beginning of file:%d", rc ) ;
         goto error ;
      }

      rc = ossWriteN( &_file, _buf, sizeof( migFileHeader ) ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to write file, rc:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _migLobTool::_append2File( const bson::BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      sdbLob lob ;
      bson::OID oid ;
      UINT64 totalWrite = 0 ;
      INT64 truncateSize = 0 ;
      BSONElement ele ;

      rc = ossGetFileSize( &_file, &truncateSize ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get file size:%d", rc ) ;
         goto error ;
      }

      ele = obj.getField( FIELD_NAME_LOB_OID ) ;
      if ( jstOID != ele.type() )
      {
         PD_LOG( PDERROR, "invalid lob obj:%s",
                 obj.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      oid = ele.OID() ;

      rc = _write( obj.objdata(), obj.objsize() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to write file:%d", rc ) ;
         goto error ;
      }

      totalWrite += obj.objsize() ;

      rc = _cl.openLob( lob, oid ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open lob[%s], rc:%d",
                 oid.str().c_str(), rc ) ;
         goto error ;
      }

      do
      {
         UINT32 read = 0 ;
         rc = lob.read( _bufSize - _written,
                        _buf + _written,
                        &read ) ;
         if ( SDB_EOF == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to read lob[%s], rc:%d",
                    oid.str().c_str(), rc ) ;
            goto error ;
         }

         _written += read ;
         if ( _bufSize == _written )
         {
            rc = ossWriteN( &_file, _buf, _bufSize ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to write file:%d", rc ) ;
               goto error ;
            }
            _written = 0 ;
         }
      } while ( TRUE ) ;

      if ( 0 < _written )
      {
         UINT32 aligned = ossRoundUpToMultipleX( _written,
                                                 OSS_FILE_DIRECT_IO_ALIGNMENT ) ;
         SDB_ASSERT( aligned <= _bufSize, "impossible" ) ;
         rc = ossWriteN( &_file, _buf, aligned ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to write file:%d", rc ) ;
            goto error ;
         }
         _written = 0 ;
      }

   done:
      lob.close() ;
      return rc ;
   error:
      if ( 0 < truncateSize )
      {
         INT32 rcTmp = _truncate( truncateSize ) ;
         if ( SDB_OK != rcTmp )
         {
            PD_LOG( PDERROR, "failed to truncate file:%d", rcTmp ) ;
         }
         _written = 0 ;
      }
      goto done ;
   }

   INT32 _migLobTool::_truncate( INT64 len )
   {
      INT32 rc = SDB_OK ;
      rc = ossTruncateFile( &_file, len ) ;
      if ( SDB_OK  != rc )
      {
         PD_LOG( PDERROR, "failed to truncate file:%d", rc ) ;
         goto error ;
      }

      rc = ossSeek( &_file, 0, OSS_SEEK_END ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek to the end of file:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _migLobTool::_write( const CHAR *data, UINT32 size )
   {
      INT32 rc = SDB_OK ;
      const CHAR *pos = data ;
      UINT32 totalLen = size ;

      do
      {
         UINT32 cpLen = totalLen <= _bufSize - _written ?
                        totalLen : _bufSize - _written ;
         ossMemcpy( _buf + _written, pos, cpLen ) ;
         _written += cpLen ;

         if ( _bufSize == _written )
         {
            rc = ossWriteN( &_file, _buf, _bufSize ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to write file:%d", rc ) ;
               goto error ;
            }
            _written = 0 ;
         }

         pos += cpLen ;
         totalLen -= cpLen ;
      } while ( 0 < totalLen ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _migLobTool::_initDB( const migOptions &ops )
   {
      INT32 rc = SDB_OK ;
      rc = _db.connect( ops.hostname,
                        ops.service,
                        ops.usrname,
                        ops.passwd ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to connect to specified db[%s:%s], rc:%d",
                 ops.hostname, ops.service, rc ) ;
         goto error ;
      }

      rc = _db.getCollection( ops.collection, _cl ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get collection[%s], rc:%d",
                  ops.collection, rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   void _migLobTool::_closeDB()
   {
      _db.disconnect() ;
      return ;
   }

   INT32 _migLobTool::_createFile( const CHAR *fullPath )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != fullPath, "can not be null" ) ;
      UINT32 mode = OSS_READWRITE |
                    OSS_SHAREREAD |
                    OSS_CREATEONLY |
                    OSS_DIRECTIO ;

      rc = ossOpen( fullPath, mode, OSS_RU|OSS_WU|OSS_RG, _file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open file:%s, rc:%d",
                 fullPath, rc ) ;
         goto error ;
      }

      _initFileHeader( ( migFileHeader * )_buf ) ;
      rc = ossWriteN( &_file, _buf, sizeof( migFileHeader ) ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to write file[%s], rc:%d",
                 fullPath, rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      if ( _file.isOpened() )
      {
         ossClose( _file ) ;
      }
      goto done ;
   }

   void _migLobTool::_initFileHeader( migFileHeader *header )
   {
      migFileHeader h ;
      ossMemcpy( header, &h, sizeof( h ) ) ;
   }
}

