/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = dpsArchiveInfo.cpp

   Descriptive Name = Data Protection Services Log Archive Info

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains code logic for log page
   operations

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/28/2016  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "dpsArchiveInfo.hpp"
#include "ossMem.hpp"
#include "../util/fromjson.hpp"

using namespace bson ;
using namespace std ;

namespace engine
{
   #define DPS_ARCHIVE_INFO_PREFIX ".archive."
   #define DPS_ARCHIVE_INFO_FILE1 DPS_ARCHIVE_INFO_PREFIX"1"
   #define DPS_ARCHIVE_INFO_FILE2 DPS_ARCHIVE_INFO_PREFIX"2"

   #define DPS_ARCHIVE_INFO_NEXTLSN "startLSN"
   #define DPS_ARCHIVE_INFO_COUNTER "count"

   dpsArchiveInfoMgr::dpsArchiveInfoMgr()
   {
      _count = 0 ;
   }

   dpsArchiveInfoMgr::~dpsArchiveInfoMgr()
   {
   }

   INT32 dpsArchiveInfoMgr::init( const CHAR* archivePath )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( NULL != archivePath, "archivePath can't be NULL" ) ;

      _path = string( archivePath ) ;
      string file1 = _path + OSS_FILE_SEP + DPS_ARCHIVE_INFO_FILE1 ;
      string file2 = _path + OSS_FILE_SEP + DPS_ARCHIVE_INFO_FILE2 ;

      rc = _open( file1, _file1 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to open file1, rc=%d", rc ) ;
         goto error ;
      }

      rc = _open( file2, _file2 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to open file2, rc=%d", rc ) ;
         goto error ;
      }

      rc = _initInfo() ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to init archive info, rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   dpsArchiveInfo dpsArchiveInfoMgr::getInfo()
   {
      dpsArchiveInfo info ;
      _mutex.get() ;
      info = _info ;
      _mutex.release() ;
      return info ;
   }

   INT32 dpsArchiveInfoMgr::updateInfo( dpsArchiveInfo& info )
   {
      BSONObj data ;
      INT32 rc = SDB_OK ;

      ossScopedLock lock( &_mutex ) ;

      _info = info ;
      _count++ ;

      rc = _toBson( data ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to get archive info bson, rc=%d", rc ) ;
         goto error ;
      }

      rc = _write( _file1, data ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to update archive info to file1, " \
                  "rc=%d", rc ) ;
         goto error ;
      }

      rc = _write( _file2, data ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to update archive info to file2, " \
                  "rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 dpsArchiveInfoMgr::_initInfo()
   {
      INT32 rc = SDB_OK ;
      BSONObj data1 ;
      BSONObj data2 ;
      dpsArchiveInfo info1 ;
      dpsArchiveInfo info2 ;
      INT64 count1 = 0 ;
      INT64 count2  = 0 ;
      INT32 rc1 = SDB_OK ;
      INT32 rc2 = SDB_OK ;

      rc1 = _read( _file1, data1 ) ;
      rc2 = _read( _file2, data2 ) ;
      if ( SDB_OK != rc1 && SDB_OK != rc2 )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to read data from both archive info file, " \
                 "rc1=%d, rc2=%d",
                 rc1, rc2 ) ;
         goto error ;
      }

      if ( SDB_OK == rc1 && !data1.isEmpty() )
      {
         rc1 = _fromBson( data1, info1, count1 ) ;
      }

      if ( SDB_OK ==  rc2 && !data2.isEmpty() )
      {
         rc2 = _fromBson( data2, info2, count2 ) ;
      }

      if ( SDB_OK == rc1 && SDB_OK == rc2 )
      {
         _info = ( count1 >= count2 ) ? info1 : info2 ;
         _count = ( count1 >= count2 ) ? count1 : count2 ;
      }
      else if ( SDB_OK == rc1 )
      {
         _info = info1 ;
         _count = count1 ;
      }
      else if ( SDB_OK == rc2 )
      {
         _info = info2 ;
         _count = count2 ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to get archive info from file" ) ;
         goto error ;
      }

      if ( count1 != count2 )
      {
         rc = updateInfo( _info ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG ( PDERROR, "Failed to update archive info when init, rc=%d",
                     rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 dpsArchiveInfoMgr::_open( const string& fileName, ossFile& file )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN exist = FALSE ;
      UINT32 mode = 0 ;

      rc = ossFile::exists( fileName, exist ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to check if file[%s] exists, rc=%d",
                 fileName.c_str(), rc ) ;
         goto error ;
      }

      if ( exist )
      {
         mode = OSS_READWRITE ;
      }
      else
      {
         mode = OSS_CREATEONLY |OSS_READWRITE ;
      }

      rc = file.open( fileName, mode, OSS_DEFAULTFILE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to open file [%s], rc=%d",
                 fileName.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 dpsArchiveInfoMgr::_read( ossFile& file, BSONObj& data )
   {
      INT64 fileSize = 0 ;
      INT64 readSize = 0 ;
      CHAR* buf = NULL ;
      INT32 rc = SDB_OK ;

      SDB_ASSERT( file.isOpened(), "file should be opened" ) ;

      rc = file.getFileSize( fileSize ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to get file size, rc=%d", rc ) ;
         goto error ;
      }

      SDB_ASSERT( fileSize >= 0, "file size should be >= 0" ) ;

      if ( 0 == fileSize )
      {
         // no data in file
         goto done ;
      }

      buf = (CHAR*)SDB_OSS_MALLOC( fileSize + 1 ) ; // one more byte for safe
      if ( NULL == buf )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Failed to malloc, mem size = %lld, rc=%d",
                 fileSize + 1, rc ) ;
         goto error ;
      }

      rc = file.seekAndReadN( 0, buf, fileSize, readSize ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to read from file, rc=%d", rc ) ;
         goto error ;
      }

      SDB_ASSERT( readSize == fileSize, "readSize != fileSize" ) ;
      buf[ readSize ] = '\0' ; // safe guard

      rc = bson::fromjson( buf, data ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to convert json to bson, rc=%d", rc ) ;
         goto error ;
      }

   done:
      SAFE_OSS_FREE( buf ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 dpsArchiveInfoMgr::_write( ossFile& file, BSONObj& data )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( file.isOpened(), "file must be opened" ) ;

      string json = data.toString() ;

      rc = file.truncate( 0 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to truncate archive info file, rc=%d", rc );
         goto error ;
      }

      rc = file.seekAndWriteN( 0, json.c_str(), json.size() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to write archive info to file, " \
                 "info size=%d, rc=%d",
                 json.size(), rc );
         goto error ;
      }

      rc = file.sync() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to sync archive info file, rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 dpsArchiveInfoMgr::_toBson( BSONObj& data )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder builder ;

      try
      {
         builder.append( DPS_ARCHIVE_INFO_NEXTLSN,
                         (INT64)_info.startLSN.offset ) ;
         builder.append( DPS_ARCHIVE_INFO_COUNTER, _count ) ;
         data = builder.obj() ;
      }
      catch ( std::exception& e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to create BSON object: %s",
                  e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 dpsArchiveInfoMgr::_fromBson( const BSONObj& data,
                                       dpsArchiveInfo& info,
                                       INT64& count )
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      dpsArchiveInfo _info ;
      INT64 _count = 0 ;

      ele = data.getField( DPS_ARCHIVE_INFO_COUNTER ) ;
      if ( NumberLong != ele.type() && NumberInt != ele.type() )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      _count = ele.numberLong() ;
      if ( _count <= 0 )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      ele = data.getField( DPS_ARCHIVE_INFO_NEXTLSN ) ;
      if ( NumberLong != ele.type() && NumberInt != ele.type() )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      _info.startLSN.offset = ele.numberLong() ;
      if ( _info.startLSN.offset == DPS_INVALID_LSN_OFFSET )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      info = _info ;
      count = _count ;

  done:
      return rc ;
   error:
      goto done ;
   }
}

