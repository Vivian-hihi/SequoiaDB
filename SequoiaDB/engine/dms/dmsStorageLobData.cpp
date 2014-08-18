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

#define DMS_LOBD_EYECATCHER "SDBLOBD"
#define DMS_LOBD_EYECATCHER_LEN 8

namespace engine
{
   _dmsStorageLobData::_dmsStorageLobData( const CHAR *fileName )
   :_fileSz( 0 ),
    _pageSz( 0 ),
    _logarithmic( 0 )
   {
      _fileName.assign( fileName ) ;
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
      UINT32 mode = OSS_READWRITE | OSS_EXCLUSIVE  ; 
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

      _fullPath.append( path ) ;
      _fullPath.append( OSS_FILE_SEP ) ;
      _fullPath.append( _fileName ) ;
      if ( OSS_MAX_PATHSIZE < _fullPath.size() )
      {
         PD_LOG ( PDERROR, "fullpath is too long: %s",
                  _fullPath.c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = ossOpen( _fullPath.c_str(), mode,
                    OSS_RU|OSS_WU|OSS_RG, _file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }

      if ( createNew )
      {
         PD_LOG( PDEVENT, "create lobd file[%s] succeed, mode: %x",
                 _fileName.c_str(), mode ) ;
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
                 _fullPath.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      close() ;
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
                    _fullPath.c_str(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::truncate( INT64 len )
   {
      INT32 rc = SDB_OK ;
      rc = ossTruncateFile( &_file, len ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to truncate file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
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
      SDB_ASSERT( 0 == (getSeek( page, offset ) - sizeof( _dmsStorageUnitHeader )) % (256 * 1024 ), "impossible" ) ;
      rc = ossSeek( &_file, getSeek( page, offset ), OSS_SEEK_SET ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek file:%d", rc ) ;
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

      rc = ossSeek( &_file, getSeek( page, offset ), OSS_SEEK_SET ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek file:%d", rc ) ;
         goto error ;
      }

      rc = ossReadN( &_file, len, buf, readFromFile ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read page:%d, rc:%d",
                 page, rc ) ;
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
      SINT64 oldSz = _fileSz ;
      rc = ossExtendFile( &_file, len ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extend file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         INT32 rcTmp = ossTruncateFile( &_file, _fileSz ) ;
         if ( SDB_OK != rcTmp )
         {
            PD_LOG( PDSEVERE, "failed to revert the increase of segment:%d"
                    ", we will panic the db",
                    rcTmp ) ;
            ossPanic() ;
         }
         goto error ;
      }
#ifdef _DEBUG
      {
      SINT64 sizeAfterExtend = 0 ;
      rc = ossGetFileSize( &_file, &sizeAfterExtend ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get size of file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }

      if ( ( sizeAfterExtend - sizeof( _dmsStorageUnitHeader ) ) % DMS_SEGMENT_SZ != 0 )
      {
         PD_LOG( PDERROR, "invalid file size:%lld, file:%s",
                 sizeAfterExtend, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      }
#endif

      _fileSz += len ;
   done:
      return rc ;
   truncate:
      {
      INT32 rcTmp = SDB_OK ;
      rcTmp = truncate( oldSz ) ;
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
      INT32 rcTmp = ossGetFileSize( &_file, &nowSize ) ;
      if ( SDB_OK != rcTmp )
      {
         PD_LOG( PDERROR, "failed to get file size:%d", rcTmp ) ;
         goto truncate ;
      }
      else if ( nowSize != oldSz )
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
      SDB_ASSERT( 0 < _fullPath.size(), "file path is empty" ) ;
      rc = close() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to close file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
         goto error ;
      }

      rc = ossDelete( _fullPath.c_str() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to remove file:%s, rc:%d",
                 _fullPath.c_str(), rc ) ;
         goto error ;
      }

      PD_LOG( PDEVENT, "remove file:%s", _fullPath.c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageLobData::_initFileHeader( const dmsStorageInfo &info )
   {
      /// TODO:add virtual functions to init and check file header.
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
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extend file:%s, rc:%d",
                 _fileName.c_str(), rc ) ;
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
         PD_LOG( PDERROR, "failed to get file header:%s, rc:%d", rc ) ;
         goto error ;
      }

      if ( 0 != ossStrncmp( header._eyeCatcher, DMS_LOBD_EYECATCHER,
                            DMS_LOBD_EYECATCHER_LEN ) )
      {
         PD_LOG( PDERROR, "invalid eye catcher:%s, file:%s",
                 header._eyeCatcher, _fileName.c_str() ) ;
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
         PD_LOG( PDERROR, "invalid su name:%s in file:%s",
                 header._name, _fileName.c_str() ) ;
         rc = SDB_SYS ;
      }

      if ( info._sequence != header._sequence )
      {
         PD_LOG( PDERROR, "invalid sequence:%d in file:%s",
                 info._sequence, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( info._secretValue != header._secretValue )
      {
         PD_LOG( PDERROR, "invalid secret value:%lld, file:%s",
                 info._secretValue, _fileName.c_str() ) ;
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

      if ( ( _fileSz - sizeof( header ) ) % DMS_SEGMENT_SZ != 0 )
      {
         PD_LOG( PDERROR, "invalid file size:%lld, file:%s",
                 _fileSz, _fileName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _pageSz = DMS_DEFAULT_LOB_PAGE_SZ ;
      ossIsPowerOf2( _pageSz, &_logarithmic ) ;
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

