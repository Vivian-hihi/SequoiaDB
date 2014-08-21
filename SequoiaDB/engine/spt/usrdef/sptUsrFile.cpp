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

   Source File Name = sptUsrFile.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptUsrFile.hpp"
#include "pd.hpp"
#include "ossMem.hpp"

using namespace std ;
using namespace bson ;

namespace engine
{
JS_MEMBER_FUNC_DEFINE( _sptUsrFile, read )
JS_MEMBER_FUNC_DEFINE( _sptUsrFile, seek )
JS_MEMBER_FUNC_DEFINE( _sptUsrFile, write )
JS_MEMBER_FUNC_DEFINE( _sptUsrFile, close )
JS_MEMBER_FUNC_DEFINE( _sptUsrFile, toString )
JS_CONSTRUCT_FUNC_DEFINE( _sptUsrFile, construct )
JS_DESTRUCT_FUNC_DEFINE( _sptUsrFile, destruct )
JS_STATIC_FUNC_DEFINE( _sptUsrFile, remove )
JS_STATIC_FUNC_DEFINE( _sptUsrFile, help )

JS_BEGIN_MAPPING( _sptUsrFile, "File" )
   JS_ADD_MEMBER_FUNC( "read", read )
   JS_ADD_MEMBER_FUNC( "write", write )
   JS_ADD_MEMBER_FUNC( "close", close )
   JS_ADD_STATIC_FUNC( "remove", remove )
   JS_ADD_STATIC_FUNC( "help", help )
   JS_ADD_MEMBER_FUNC( "seek", seek )
   JS_ADD_MEMBER_FUNC( "toString", toString )
   JS_ADD_CONSTRUCT_FUNC( construct )
   JS_ADD_DESTRUCT_FUNC( destruct )
JS_MAPPING_END()

   _sptUsrFile::_sptUsrFile()
   {

   }

   _sptUsrFile::~_sptUsrFile()
   {
      if ( _file.isOpened() )
      {
         ossClose( _file ) ;
      }
   }

   INT32 _sptUsrFile::construct( const _sptArguments &arg,
                                 _sptReturnVal &rval,
                                 bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;

      rc = arg.getString( 0, _filename ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "filename must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "filename must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get filename, rc: %d", rc ) ;

      rc = ossOpen( _filename.c_str(),
                    OSS_READWRITE | OSS_CREATE,
                    OSS_RWXU,
                    _file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open file:%s, rc:%d",
                 _filename.c_str(), rc ) ;
         goto error ;
      }

      rval.setUsrObjectVal( "", this, SPT_CLASS_DEF( this ) ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrFile::toString( const _sptArguments & arg,
                                _sptReturnVal & rval,
                                bson::BSONObj & detail )
   {
      rval.setStringVal( "", _filename.c_str() ) ;
      return SDB_OK ;
   }

   INT32 _sptUsrFile::destruct()
   {
      if ( _file.isOpened() )
      {
         ossClose( _file ) ;
      }
      return SDB_OK ;
   }

   INT32 _sptUsrFile::read( const _sptArguments &arg,
                            _sptReturnVal &rval,
                            bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
#define SPT_READ_LEN 1024
      SINT64 len = SPT_READ_LEN ;
      CHAR stackBuf[SPT_READ_LEN + 1] = { 0 } ;
      CHAR *buf = NULL ;
      SINT64 read = 0 ;

      if ( !_file.isOpened() )
      {
         PD_LOG( PDERROR, "the file is not opened." ) ;
         detail = BSON( SPT_ERR << "file is not opened" ) ;
         rc = SDB_IO ;
         goto error ;
      }
      rc = arg.getNative( 0, &len ) ;
      if ( rc && SDB_OUT_OF_BOUND != rc )
      {
         detail = BSON( SPT_ERR << "size must be native type" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get size, rc: %d", rc ) ;

      if ( SPT_READ_LEN < len )
      {
         buf = ( CHAR * )SDB_OSS_MALLOC( len + 1 ) ;
         if ( NULL == buf )
         {
            PD_LOG( PDERROR, "failed to allocate mem." ) ;
            rc = SDB_OOM ;
            goto error ;
         }
      }
      else
      {
         buf = ( CHAR * )stackBuf ;
      }

      rc = ossReadN( &_file, len, buf, read ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read file:%d", rc ) ;
         goto error ;
      }
      buf[read] = '\0' ;

      rval.setStringVal( "", buf ) ;
   done:
      if ( SPT_READ_LEN < len && NULL != buf )
      {
         SDB_OSS_FREE( buf ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrFile::write( const _sptArguments &arg,
                             _sptReturnVal &rval,
                             bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      string content ;
      if ( !_file.isOpened() )
      {
         PD_LOG( PDERROR, "the file is not opened." ) ;
         detail = BSON( SPT_ERR << "file is not opened" ) ;
         rc = SDB_IO ;
         goto error ;
      }
      rc = arg.getString( 0, content ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "content must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "content must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get content, rc: %d", rc ) ;

      rc = ossWriteN( &_file, content.c_str(), content.size() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to write to file:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrFile::seek( const _sptArguments &arg,
                            _sptReturnVal &rval,
                            bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      INT32 seekSize = 0 ;
      OSS_SEEK whence ;
      string whenceStr = "b" ;

      if ( !_file.isOpened() )
      {
         PD_LOG( PDERROR, "the file is not opened." ) ;
         detail = BSON( SPT_ERR << "file is not opened" ) ;
         rc = SDB_IO ;
         goto error ;
      }

      rc = arg.getNative( 0, &seekSize ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "offset must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "offset must be native type" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get offset, rc: %d", rc ) ;

      rc = arg.getString( 1, whenceStr ) ;
      if ( rc && SDB_OUT_OF_BOUND != rc )
      {
         detail = BSON( SPT_ERR << "where must be string(b/c/e)" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get where, rc: %d", rc ) ;

      if ( "b" == whenceStr )
      {
         whence = OSS_SEEK_SET ;
      }
      else if ( "c" == whenceStr )
      {
         whence = OSS_SEEK_CUR ;
      }
      else if ( "e" == whenceStr )
      {
         whence = OSS_SEEK_END ;
      }
      else
      {
         detail = BSON( SPT_ERR << "where must be string(b/c/e)" ) ;
         PD_LOG( PDERROR, "invalid arg whence:%s", whenceStr.c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = ossSeek( &_file, seekSize, whence ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to seek:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrFile::remove( const _sptArguments &arg,
                             _sptReturnVal &rval,
                             bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      string fullPath ;
      rc = arg.getString( 0, fullPath ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "filepath must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "filepath must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get filepath, rc: %d", rc ) ;

      rc = ossDelete( fullPath.c_str() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to remove file:%s, rc:%d",
                 fullPath.c_str(), rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrFile::close( const _sptArguments &arg,
                             _sptReturnVal &rval,
                             bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      if ( _file.isOpened() )
      {
         rc = ossClose( _file ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to close file:%d", rc ) ;
            goto error ;
         }
      } 
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrFile::help( const _sptArguments & arg,
                            _sptReturnVal & rval,
                            BSONObj & detail )
   {
      stringstream ss ;
      ss << "File functions:" << endl
         << "var file = new File( filename )" << endl
         << "   read( [size] )" << endl
         << "   write( content )" << endl
         << "   seek( offset, [where] ) " << endl
         << "   close()" << endl
         << "File.remove( filepath )" << endl ;
      rval.setStringVal( "", ss.str().c_str() ) ;
      return SDB_OK ;
   }

}

