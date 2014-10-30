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

   Source File Name = dmsStorageLobData.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          17/07/2014  YW Initial Draft

   Last Changed =

*******************************************************************************/

#include "dmsStorageLobData.hpp"
#include "ossUtil.hpp"
#include "utilStr.hpp"

namespace engine
{
   #define DMS_LOBD_EYECATCHER            "SDBLOBD"
   #define DMS_LOBD_EYECATCHER_LEN        8

   /*
      _dmsStorageLobData implement
   */
   _dmsStorageLobData::_dmsStorageLobData( const CHAR *fileName )
   :_fileSz( 0 ),
    _lastSz( 0 ),
    _pageSz( 0 ),
    _logarithmic( 0 )
   {
      _fileName.assign( fileName ) ;
      _segmentPages = 0 ;
      _segmentPagesSquare = 0 ;
      ossMemset( _fullPath, 0, sizeof( _fullPath ) ) ;
   }

   _dmsStorageLobData::~_dmsStorageLobData()
   {
      close() ;   
   }

   INT32 _dmsStorageLobData::open( const CHAR *path,
                                   BOOLEAN createNew,
                                   BOOLEAN delWhenExist,
                                   const dmsStorageInfo &info )
   {
      INT32 rc = SDB_OK ;
      UINT32 mode = OSS_READWRITE | OSS_SHAREREAD ; 
      SDB_ASSERT( path, "path can't be NULL" ) ;
      INT64 fileSize = 0 ;

      if ( createNew )
      {
         if ( delWhenExist )
         {
            mode |= OSS_REPLACE ;
         }
         else
         {
            mode |= OSS_CREATEONLY ;
         }
      }

      rc = utilBuildFullPath( path, _fileName.c_str(), OSS_MAX_PATHSIZE,
                              _fullPath ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "path + name is too long: %s, %s",
                  path, _fileName.c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = ossOpen( _fullPath, mode, OSS_RU|OSS_WU|OSS_RG, _file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open file:%s, rc:%d",
                 _fullPath, rc ) ;
         goto error ;
      }

      if ( createNew )
      {
         PD_LOG( PDEVENT, "create lobd file[%s] succeed, mode: %x",
                 _fullPath, mode ) ;
      }

      rc = ossGetFileSize( &_file, &fileSize ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get size of file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }

      if ( 0 == fileSize )
      {
         if ( !createNew )
         {
            PD_LOG ( PDERROR, "lobd file is empty: %s", _fileName.c_str() ) ;
            rc = SDB_DMS_INVALID_SU ;
            goto error ;   
         }

         rc = _initFileHeader( info ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to init file header:%s, rc:%d",
                    _fileName.c_str(), rc ) ;
            goto error ;
         }
      }

      rc = _validateFile( info ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to validate file:%s, rc:%d",
                 _fullPath, rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      close() ;
      goto done ;
   }

   INT32 _dmsStorageLobData::_reopen()
   {
      INT32 rc = SDB_OK ;
      INT64 tmpSz = _fileSz ;

      rc = close() ;
      if( rc )
      {
         PD_LOG( PDERROR, "Close file[%s] failed, rc: %d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }

      rc = ossOpen( _fullPath, OSS_READWRITE | OSS_SHAREREAD,
                    OSS_RU|OSS_WU|OSS_RG, _file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to reopen file:%s, rc:%d",
                 _fullPath, rc ) ;
         goto error ;
      }
      _lastSz = tmpSz ;

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _dmsStorageLobData::isOpened()const
   {
      return _file.isOpened() ;
   }

   INT32 _dmsStorageLobData::close()
   {
      INT32 rc = SDB_OK ;
      if ( _file.isOpened() )
      {
         rc = ossClose( _file ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to close file:%s, rc:%d",
                    _fullPath, rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::write( DMS_LOB_PAGEID page,
                                    const CHAR *data,
                                    UINT32 len,
                                    UINT32 offset )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( DMS_LOB_INVALID_PAGEID != page &&
                  NULL != data &&
                  0 == offset &&
                  len <= _pageSz, "invalid operation" ) ;

      INT64 writeOffset = getSeek( page, offset ) ;
      if ( writeOffset + len > _fileSz )
      {
         PD_LOG( PDERROR, "Offset[%lld] grater than file size[%lld] in "
                 "file[%s]", writeOffset, _fileSz, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // reopen when the offset grater than lastSz
      if ( writeOffset + len > _lastSz )
      {
         rc = _reopen() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to reopen file[%s], rc: %d",
                    _fileName.c_str(), rc ) ;
            goto error ;
         }
      }

      rc = ossSeek( &_file, writeOffset, OSS_SEEK_SET ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek file[%lld], rc: %d",
                 writeOffset, rc ) ;
         goto error ;
      }

      rc = ossWriteN( &_file, data, len ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to write data, page:%d, rc:%d",
                 page, rc ) ;
         goto error ; 
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::read( DMS_LOB_PAGEID page,
                                   UINT32 len,
                                   UINT32 offset,
                                   CHAR *buf,
                                   UINT32 &readLen )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( DMS_LOB_INVALID_PAGEID != page &&
                  NULL != buf &&
                  len + offset <= _pageSz, "invalid operation" ) ;
      SINT64 readFromFile = 0 ;
      INT64 readOffset = getSeek( page, offset ) ;

      if ( readOffset + len > _fileSz )
      {
         PD_LOG( PDERROR, "Offset[%lld] grater than file size[%lld] in "
                 "file[%s]", readOffset, _fileSz, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( readOffset + len > _lastSz )
      {
         PD_LOG( PDERROR, "Offset[%lld] grater than last size[%lld] in"
                 "file[%s]", readOffset, _lastSz, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = ossSeek( &_file, readOffset, OSS_SEEK_SET ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek file[%lld], rc: %d",
                 readOffset, rc ) ;
         goto error ;
      }

      rc = ossReadN( &_file, len, buf, readFromFile ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read page[%d], rc: %d",
                 page, rc ) ;
         goto error ;
      }

      readLen = readFromFile ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::readRaw( UINT64 offset, UINT32 len,
                                      CHAR * buf, UINT32 &readLen )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != buf && offset <= _fileSz, "invalid operation" ) ;
      SINT64 readFromFile = 0 ;

      if ( offset + len > _fileSz )
      {
         PD_LOG( PDERROR, "Offset[%lld] grater than file size[%lld] in "
                 "file[%s]", offset, _fileSz, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // reopen when the offset grater than lastSz
      if ( offset + len > _lastSz )
      {
         rc = _reopen() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to reopen file[%s], rc: %d",
                    _fileName.c_str(), rc ) ;
            goto error ;
         }
      }

      rc = ossSeek( &_file, offset, OSS_SEEK_SET ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek file[%lld], rc: %d",
                 offset, rc ) ;
         goto error ;
      }

      rc = ossReadN( &_file, len, buf, readFromFile ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read data[offset: %lld, len: %d], rc: %d",
                 offset, len, rc ) ;
         goto error ;
      }

      readLen = readFromFile ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::extend( INT64 len )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( 0 < len, "invalid extend size" ) ;
      OSSFILE file ;
      UINT32 mode = OSS_READWRITE | OSS_SHAREREAD  ;

      rc = ossOpen( _fullPath, mode, OSS_RU|OSS_WU|OSS_RG, file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open file when extend:%d", rc ) ;
         goto error ;
      }

#ifdef _DEBUG
      {
         SINT64 sizeBeforeExtend = 0 ;
         rc = ossGetFileSize( &file, &sizeBeforeExtend ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get size of file:%s, rc:%d",
                    _fileName.c_str(), rc ) ;
            goto error ;
         }

         if ( 0 != _fileSz &&
              ( sizeBeforeExtend - sizeof( _dmsStorageUnitHeader ) ) %
              DMS_SEGMENT_SZ != 0 )
         {
            PD_LOG( PDERROR, "invalid file size:%lld, file:%s",
                    sizeBeforeExtend, _fileName.c_str() ) ;
            rc = SDB_SYS ;
            SDB_ASSERT( FALSE, "impossible" ) ;
            goto error ;
         }
      }
#endif // _DEBUG

      rc = ossExtendFile( &file, len ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extend file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }

#ifdef _DEBUG
      {
         SINT64 sizeAfterExtend = 0 ;
         rc = ossGetFileSize( &file, &sizeAfterExtend ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get size of file:%s, rc:%d",
                    _fileName.c_str(), rc ) ;
            goto error ;
         }

         if ( ( sizeAfterExtend - sizeof( _dmsStorageUnitHeader ) ) %
              DMS_SEGMENT_SZ != 0 )
         {
            PD_LOG( PDERROR, "invalid file size:%lld, file:%s",
                    sizeAfterExtend, _fileName.c_str() ) ;
            rc = SDB_SYS ;
            SDB_ASSERT( FALSE, "impossible" ) ;
            goto error ;
         }
      }
#endif // _DEBUG

      _fileSz += len ;

   done:
      if ( file.isOpened() )
      {
         INT32 rcTmp = SDB_OK ;
         rcTmp = ossClose( file ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to close file after extend:%d", rcTmp ) ;
         }
      }
      return rc ;
   truncate:
      {
         INT32 rcTmp = SDB_OK ;
         rcTmp = ossTruncateFile( &file, _fileSz ) ;
         if ( SDB_OK != rcTmp )
         {
            PD_LOG( PDSEVERE, "Failed to revert the increase of segment, "
                     "rc = %d", rcTmp ) ;
            ossPanic() ;
         }
         goto done ;
      }
   error:
      {
         SINT64 nowSize = 0 ;
         INT32 rcTmp = ossGetFileSize( &file, &nowSize ) ;
         if ( SDB_OK != rcTmp )
         {
            PD_LOG( PDERROR, "failed to get file size:%d", rcTmp ) ;
            goto truncate ;
         }
         else if ( nowSize != _fileSz )
         {
            goto truncate ;
         }
         else
         {
            goto done ;
         }
      }
   }

   INT32 _dmsStorageLobData::remove()
   {
      INT32 rc = SDB_OK ;

      if ( _fullPath[ 0 ] == 0 )
      {
         goto done ;
      }

      rc = close() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to close file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }

      rc = ossDelete( _fullPath ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to remove file:%s, rc:%d",
                 _fullPath, rc ) ;
         goto error ;
      }

      PD_LOG( PDEVENT, "remove file:%s", _fullPath ) ;
      _fullPath[ 0 ] = 0 ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::_initFileHeader( const dmsStorageInfo &info )
   {
      INT32 rc = SDB_OK ;
      _dmsStorageUnitHeader header ;
      ossStrncpy( header._eyeCatcher, DMS_LOBD_EYECATCHER,
                  DMS_LOBD_EYECATCHER_LEN ) ;
      header._version = DMS_LOB_CUR_VERSION ;
      header._pageSize = 0 ;
      header._storageUnitSize = 0 ;
      ossStrncpy ( header._name, info._suName, DMS_SU_NAME_SZ ) ;
      header._sequence = info._sequence ;
      header._numMB    = 0 ;
      header._MBHWM    = 0 ;
      header._pageNum  = 0 ;
      header._secretValue = info._secretValue ;

      rc = extend( sizeof( header ) ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to extend header, rc: %d", rc ) ;
         goto error ;
      }

      rc = ossSeek ( &_file, 0, OSS_SEEK_SET ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "failed to seek to beginning of the file, rc: %d",
                  rc ) ;
         goto error ;
      }

      rc = ossWriteN( &_file, ( const CHAR *)(&header), sizeof( header ) ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to write file header:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::_validateFile( const dmsStorageInfo &info )
   {
      INT32 rc = SDB_OK ;
      _dmsStorageUnitHeader header ;
      rc = _getFileHeader( header ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get file header, rc:%d", rc ) ;
         goto error ;
      }

      if ( 0 != ossStrncmp( header._eyeCatcher, DMS_LOBD_EYECATCHER,
                            DMS_LOBD_EYECATCHER_LEN ) )
      {
         CHAR szTmp[ DMS_HEADER_EYECATCHER_LEN + 1 ] = {0} ;
         ossStrncpy( szTmp, header._eyeCatcher, DMS_HEADER_EYECATCHER_LEN ) ;
         PD_LOG( PDERROR, "invalid eye catcher:%s, file:%s",
                 szTmp, _fileName.c_str() ) ;
         rc = SDB_INVALID_FILE_TYPE ;
         goto error ;
      }

      if ( DMS_LOB_CUR_VERSION != header._version )
      {
         PD_LOG( PDERROR, "invalid version of header:%d, file:%s",
                 header._version, _fileName.c_str() ) ;
         rc = SDB_DMS_INCOMPATIBLE_VERSION ;
         goto error ;
      }

      if ( 0 != header._pageSize ||
           0 != header._storageUnitSize ||
           0 != header._numMB ||
           0 != header._MBHWM ||
           0 != header._pageNum )
      {
         PD_LOG( PDERROR, "invalid field value which not in used" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( 0 != ossStrncmp ( info._suName, header._name,
                             DMS_SU_NAME_SZ ) )
      {
         CHAR szTmp[ DMS_SU_NAME_SZ + 1 ] = {0} ;
         ossStrncpy( szTmp, header._name, DMS_SU_NAME_SZ ) ;
         PD_LOG( PDERROR, "invalid su name:%s in file:%s",
                 szTmp, _fileName.c_str() ) ;
         rc = SDB_SYS ;
      }

      if ( info._sequence != header._sequence )
      {
         PD_LOG( PDERROR, "invalid sequence:%d != %d in file:%s",
                 header._sequence, info._sequence, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( info._secretValue != header._secretValue )
      {
         PD_LOG( PDERROR, "invalid secret value: %lld, self: %lld, file:%s",
                 info._secretValue, header._secretValue, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = ossGetFileSize( &_file, &_fileSz ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get size of file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }
      _lastSz = _fileSz ;

      if ( ( _fileSz - sizeof( header ) ) % DMS_SEGMENT_SZ != 0 )
      {
         PD_LOG( PDERROR, "invalid file size:%lld, file:%s",
                 _fileSz, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _pageSz = info._lobdPageSize ;
      if ( !ossIsPowerOf2( _pageSz, &_logarithmic ) )
      {
         PD_LOG( PDERROR, "Page size[%d] is not power of 2", _pageSz ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _segmentPages = getSegmentSize() >> _logarithmic ;
      if ( !ossIsPowerOf2( _segmentPages, &_segmentPagesSquare ) )
      {
         PD_LOG( PDERROR, "Segment pages[%u] must be the power of 2",
                 _segmentPages ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::_getFileHeader( _dmsStorageUnitHeader &header )
   {
      INT32 rc = SDB_OK ;
      INT64 fileLen = 0 ;
      SINT64 readLen = 0 ;
      rc = ossGetFileSize( &_file, &fileLen ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to file len:%d", rc ) ;
         goto error ;
      }

      if ( fileLen < ( INT64 )sizeof( _dmsStorageUnitHeader ) )
      {
         PD_LOG( PDERROR, "invalid length of file:%lld", fileLen ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = ossSeek( &_file, 0, OSS_SEEK_SET ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek file:%d", rc ) ;
         goto error ;
      }

      rc = ossReadN( &_file, sizeof( _dmsStorageUnitHeader ),
                     ( CHAR * )( &header ), readLen ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read file:%d", rc ) ;
         goto error ;
      }

      SDB_ASSERT( sizeof( _dmsStorageUnitHeader ) == readLen, "impossible" ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}

