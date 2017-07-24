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

   Source File Name = omagentRemoteUsrFile.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/03/2016  WJM Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentRemoteUsrFile.hpp"
#include "omagentMgr.hpp"
#include "omagentDef.hpp"
#include "ossCmdRunner.hpp"
#include "ossPrimitiveFileOp.hpp"
#include "sptUsrFileCommon.hpp"
#include <boost/algorithm/string.hpp>
#include "../bson/lib/md5.hpp"
#if defined(_LINUX)
#include <sys/stat.h>
#endif
using namespace bson ;
#define SPT_READ_LEN 1024
#define SPT_MD5_READ_LEN 1024

namespace engine
{
   /*
      _remoteFileOpen implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileOpen )

   _remoteFileOpen::_remoteFileOpen()
   {
   }

   _remoteFileOpen::~_remoteFileOpen()
   {
   }

   const CHAR* _remoteFileOpen::name()
   {
      return OMA_REMOTE_FILE_OPEN ;
   }

   INT32 _remoteFileOpen::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string filename ;
      _sptUsrFileCommon fileCommon ;
      string err ;

      // get argument
      if ( FALSE == _valueObj.hasField( "filename" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filename must be config" ) ;
         goto error ;
      }
      if ( String != _valueObj.getField( "filename" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filename must be string" ) ;
         goto error ;
      }
      filename = _valueObj.getStringField( "filename" ) ;

      rc = fileCommon.open( filename, _optionObj, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }

      rc = fileCommon.close( err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileRead implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileRead )

   _remoteFileRead::_remoteFileRead()
   {
   }

   _remoteFileRead::~_remoteFileRead()
   {
   }

   INT32 _remoteFileRead::init( const CHAR* pInfomation )
   {
      INT32 rc = SDB_OK ;

      rc = _remoteExec::init( pInfomation ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get argument, rc: %d", rc ) ;

      if ( FALSE == _valueObj.hasField( "filename" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filename must be config" ) ;
         goto error;
      }
      if ( String != _valueObj.getField( "filename" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filename must be string" ) ;
         goto error ;
      }
      _filename = _valueObj.getStringField( "filename" ) ;

      if ( FALSE == _valueObj.hasField( "location" ) )
      {
         _location = 0 ;
      }
      else
      {
         BSONElement element  = _valueObj.getField( "location" ) ;
         if( NumberInt != element.type() &&
             NumberLong != element.type() )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG_MSG( PDERROR, "location must be number" ) ;
            goto error ;
         }
         else
         {
            _location = element.numberLong() ;
         }
      }

      if ( FALSE == _valueObj.hasField( "size" ) )
      {
         _size = SPT_READ_LEN ;
      }
      else
      {
         BSONElement element  = _valueObj.getField( "size" ) ;
         if( NumberInt != element.type() &&
             NumberLong != element.type() )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG_MSG( PDERROR, "size must be number" ) ;
            goto error ;
         }
         else
         {
            _size = element.numberLong() ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   const CHAR* _remoteFileRead::name()
   {
      return OMA_REMOTE_FILE_READ ;
   }

   INT32 _remoteFileRead::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      CHAR *buf = NULL ;
      SINT64 readLen = 0 ;
      BSONObjBuilder builder ;
      _sptUsrFileCommon fileCommon ;
      string err ;

      // open file
      rc = fileCommon.open( _filename, _optionObj, err ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }

      rc = fileCommon.seek( _location, BSONObj(), err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }

      // read content
      rc = fileCommon.read( BSON( "size" << _size ), err,
                           &buf, readLen ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
      builder.append( "readContent", buf, readLen + 1 ) ;
      builder.append( "readLen", readLen) ;

      retObj = builder.obj() ;
   done:
      if ( NULL != buf )
      {
         SDB_OSS_FREE( buf ) ;
      }
      fileCommon.close( err ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileWrite implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileWrite )

   _remoteFileWrite::_remoteFileWrite()
   {
      _location = 0 ;
      _size = 0 ;
      _content = NULL ;
   }

   _remoteFileWrite::~_remoteFileWrite()
   {
   }

   INT32 _remoteFileWrite::init( const CHAR * pInfomation )
   {
      INT32 rc = SDB_OK ;

      rc = _remoteExec::init( pInfomation ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get argument, rc: %d", rc ) ;

      _content = _valueObj.getStringField( "content" ) ;

      if ( FALSE == _valueObj.hasField( "filename" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filename must be config" ) ;
         goto error;
      }
      if ( String != _valueObj.getField( "filename" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filename must be string" ) ;
         goto error ;
      }
      _filename = _valueObj.getStringField( "filename" ) ;

      if ( FALSE == _valueObj.hasField( "location" ) )
      {
         _location = 0 ;
      }
      else if ( NumberInt != _valueObj.getField( "location" ).type() &&
                NumberLong != _valueObj.getField( "location" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "location must be number" ) ;
         goto error ;
      }
      else
      {
         _location = _valueObj.getIntField( "location" ) ;
      }

      if( FALSE == _valueObj.hasField( "size" ) )
      {
         _size = ossStrlen( _content ) ;
      }
      else if( NumberInt != _valueObj.getField( "size" ).type() &&
               NumberLong != _valueObj.getField( "size" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "size must be number" ) ;
         goto error ;
      }
      else
      {
         _size = _valueObj.getIntField( "size" ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   const CHAR* _remoteFileWrite::name()
   {
      return OMA_REMOTE_FILE_WRITE ;
   }

   INT32 _remoteFileWrite::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      OSSFILE file ;
      _sptUsrFileCommon fileCommon ;
      string err ;

      // open file
      rc = fileCommon.open( _filename, _optionObj, err ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }

      rc = fileCommon.seek( _location, BSONObj(), err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }

      // write content
      rc = fileCommon.write( _content, _size, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      fileCommon.close( err ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileRemove implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileRemove )

   _remoteFileRemove::_remoteFileRemove()
   {
   }

   _remoteFileRemove::~_remoteFileRemove()
   {
   }

   const CHAR* _remoteFileRemove::name()
   {
      return OMA_REMOTE_FILE_REMOVE ;
   }

   INT32 _remoteFileRemove::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string filepath ;
      string err ;

      // get pathname
      if ( FALSE == _valueObj.hasField( "filepath" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filepath must be config" ) ;
         goto error ;
      }
      if ( String != _valueObj.getField( "filepath" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filepath must be string" ) ;
         goto error ;
      }
      filepath = _valueObj.getStringField( "filepath" ) ;

      rc = _sptUsrFileCommon::remove( filepath, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileExist implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileExist )

   _remoteFileExist::_remoteFileExist()
   {
   }

   _remoteFileExist::~_remoteFileExist()
   {
   }

   const CHAR* _remoteFileExist::name()
   {
      return OMA_REMOTE_FILE_ISEXIST ;
   }

   INT32 _remoteFileExist::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string filepath ;
      BOOLEAN fileExist = FALSE ;
      string err ;
      BSONObjBuilder builder ;

      if ( FALSE == _valueObj.hasField( "filepath" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filepath must be config" ) ;
         goto error ;
      }
      if ( String != _valueObj.getField( "filepath" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filepath must be string" ) ;
         goto error ;
      }
      filepath = _valueObj.getStringField( "filepath" ) ;

      rc = _sptUsrFileCommon::exist( filepath, err, fileExist ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
      builder.appendBool( "isExist", fileExist ) ;
      retObj = builder.obj() ;
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileCopy implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileCopy )

   _remoteFileCopy::_remoteFileCopy()
   {
   }

   _remoteFileCopy::~_remoteFileCopy()
   {
   }

   const CHAR* _remoteFileCopy::name()
   {
      return OMA_REMOTE_FILE_COPY ;
   }

   INT32 _remoteFileCopy::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string src ;
      string dst ;
      string err ;

      // get argument
      if ( FALSE == _matchObj.hasField( "src" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "src is required" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "src" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "src must be string" ) ;
         goto error;
      }
      src = _matchObj.getStringField( "src" ) ;

      if ( FALSE == _valueObj.hasField( "dst" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "dst is required" ) ;
         goto error ;
      }
      if ( String != _valueObj.getField( "dst" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "dst must be string" ) ;
         goto error;
      }
      dst = _valueObj.getStringField( "dst" ) ;

      rc = _sptUsrFileCommon::copy( src, dst, _optionObj, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileMove implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileMove )

   _remoteFileMove::_remoteFileMove()
   {
   }

   _remoteFileMove::~_remoteFileMove()
   {
   }

   const CHAR* _remoteFileMove::name()
   {
      return OMA_REMOTE_FILE_MOVE ;
   }

   INT32 _remoteFileMove::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string src ;
      string dst ;
      string err ;

      // get argument
      if ( FALSE == _matchObj.hasField( "src" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "src is required" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "src" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "src must be string" ) ;
         goto error ;
      }
      src = _matchObj.getStringField( "src" ) ;

      if ( FALSE == _valueObj.hasField( "dst" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "dst is required" ) ;
         goto error ;
      }
      if ( String != _valueObj.getField( "dst" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "dst must be string" ) ;
         goto error ;
      }
      dst = _valueObj.getStringField( "dst" ) ;

      rc = _sptUsrFileCommon::move( src, dst, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileMkdir implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileMkdir )

   _remoteFileMkdir::_remoteFileMkdir()
   {
   }

   _remoteFileMkdir::~_remoteFileMkdir()
   {
   }

   const CHAR* _remoteFileMkdir::name()
   {
      return OMA_REMOTE_FILE_MKDIR ;
   }

   INT32 _remoteFileMkdir::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string name ;
      string err ;

      // get argument
      if ( FALSE == _valueObj.hasField( "name" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "name is required" ) ;
         goto error ;
      }
      if ( String != _valueObj.getField( "name" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "name must be string" ) ;
         goto error ;
      }
      name = _valueObj.getStringField( "name" ) ;

      rc = _sptUsrFileCommon::mkdir( name, _optionObj, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileFind implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileFind )

   _remoteFileFind::_remoteFileFind()
   {
   }

   _remoteFileFind::~_remoteFileFind()
   {
   }

   const CHAR* _remoteFileFind::name()
   {
      return OMA_REMOTE_FILE_FIND ;
   }

   INT32 _remoteFileFind::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string err ;

      // get argument
      if ( TRUE == _optionObj.isEmpty() )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "optionObj must be config") ;
         goto error ;
      }

      rc = _sptUsrFileCommon::find( _optionObj, err, retObj ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileChmod implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileChmod )

   _remoteFileChmod::_remoteFileChmod()
   {
   }

   _remoteFileChmod::~_remoteFileChmod()
   {
   }

   const CHAR* _remoteFileChmod::name()
   {
      return OMA_REMOTE_FILE_CHMOD ;
   }

   INT32 _remoteFileChmod::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string pathname ;
      INT32 mode ;
      string err ;

      // get argument
      if ( FALSE == _matchObj.hasField( "pathname" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filename must be config" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "pathname" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filename must be string" ) ;
         goto error ;
      }
      pathname = _matchObj.getStringField( "pathname" ) ;

      if ( FALSE == _valueObj.hasField( "mode" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "mode must be config" ) ;
         goto error ;
      }
      if ( NumberInt != _valueObj.getField( "mode" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "mode must be INT32" ) ;
         goto error ;
      }
      mode = _valueObj.getIntField( "mode" ) ;

      rc = _sptUsrFileCommon::chmod( pathname, mode, _optionObj, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileChown implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileChown )

   _remoteFileChown::_remoteFileChown()
   {
   }

   _remoteFileChown::~_remoteFileChown()
   {
   }

   const CHAR* _remoteFileChown::name()
   {
      return OMA_REMOTE_FILE_CHOWN ;
   }

   INT32 _remoteFileChown::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string pathname ;
      string err ;

      // get argument
      if ( FALSE == _matchObj.hasField( "filename" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filename must be config" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "filename" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filename must be string" ) ;
         goto error ;
      }
      pathname = _matchObj.getStringField( "filename" ) ;

      rc = _sptUsrFileCommon::chown( pathname, _valueObj, _optionObj, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileChgrp implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileChgrp )

   _remoteFileChgrp::_remoteFileChgrp()
   {
   }

   _remoteFileChgrp::~_remoteFileChgrp()
   {
   }

   const CHAR* _remoteFileChgrp::name()
   {
      return OMA_REMOTE_FILE_CHGRP ;
   }

   INT32 _remoteFileChgrp::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string pathname ;
      string groupname ;
      string err ;
      // get argument
      if ( FALSE == _matchObj.hasField( "filename" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filename must be config" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "filename" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filename must be string" ) ;
         goto error ;
      }
      pathname = _matchObj.getStringField( "filename" ) ;

      if ( FALSE == _valueObj.hasField( "groupname" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "groupname must be config" ) ;
         goto error ;
      }
      if ( String != _valueObj.getField( "groupname" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "groupname must be string" ) ;
         goto error ;
      }
      groupname = _valueObj.getStringField( "groupname" ) ;

      rc = _sptUsrFileCommon::chgrp( pathname, groupname, _optionObj, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileGetUmask implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileGetUmask )

   _remoteFileGetUmask::_remoteFileGetUmask()
   {

   }

   _remoteFileGetUmask::~_remoteFileGetUmask()
   {
   }

   const CHAR* _remoteFileGetUmask::name()
   {
      return OMA_REMOTE_FILE_GET_UMASK ;
   }

   INT32 _remoteFileGetUmask::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string outStr ;
      string err ;

      rc = _sptUsrFileCommon::getUmask( err, outStr ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
      retObj = BSON( "mask" << outStr.c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileSetUmask implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileSetUmask )

   _remoteFileSetUmask::_remoteFileSetUmask()
   {
   }

   _remoteFileSetUmask::~_remoteFileSetUmask()
   {
   }

   const CHAR* _remoteFileSetUmask::name()
   {
      return OMA_REMOTE_FILE_SET_UMASK ;
   }

   INT32 _remoteFileSetUmask::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string err ;
      INT32 mask ;

      // get argument
      if ( FALSE == _valueObj.hasField( "mask" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "mask must be config" ) ;
         goto error ;
      }
      if ( NumberInt != _valueObj.getField( "mask" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "mask must be INT32" ) ;
         goto error ;
      }
      mask = _valueObj.getIntField( "mask" ) ;

      rc = _sptUsrFileCommon::setUmask( mask, err ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileList implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileList )

   _remoteFileList::_remoteFileList()
   {
   }

   _remoteFileList::~_remoteFileList()
   {
   }

   const CHAR* _remoteFileList::name()
   {
      return OMA_REMOTE_FILE_LIST ;
   }

   INT32 _remoteFileList::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string err ;

      rc = _sptUsrFileCommon::list( _optionObj, err, retObj ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileGetPathType implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileGetPathType )

   _remoteFileGetPathType::_remoteFileGetPathType()
   {
   }

   _remoteFileGetPathType::~_remoteFileGetPathType()
   {
   }

   const CHAR* _remoteFileGetPathType::name()
   {
      return OMA_REMOTE_FILE_GET_PATH_TYPE ;
   }

   INT32 _remoteFileGetPathType::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string pathname ;
      string pathType ;
      string err ;

      // get argument
      if ( FALSE == _matchObj.hasField( "pathname" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "pathname must be config" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "pathname" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "pathname must be string" ) ;
         goto error ;
      }
      pathname = _matchObj.getStringField( "pathname" ) ;

      rc = _sptUsrFileCommon::getPathType( pathname, err, pathType ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
      retObj = BSON( "pathType" << pathType ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileIsEmptyDir implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileIsEmptyDir )

   _remoteFileIsEmptyDir::_remoteFileIsEmptyDir()
   {
   }

   _remoteFileIsEmptyDir::~_remoteFileIsEmptyDir()
   {
   }

   const CHAR* _remoteFileIsEmptyDir::name()
   {
      return OMA_REMOTE_FILE_IS_EMPTYDIR ;
   }

   INT32 _remoteFileIsEmptyDir::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isEmpty = FALSE ;
      string pathname ;
      string err ;

      // get pathname
      if ( FALSE == _matchObj.hasField( "pathname" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "pathname must be config" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "pathname" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "pathname must be string" ) ;
         goto error ;
      }
      pathname = _matchObj.getStringField( "pathname" ) ;

      rc = _sptUsrFileCommon::isEmptyDir( pathname, err, isEmpty ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
      retObj = BSON( "isEmpty" << isEmpty ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileStat implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileStat )

   _remoteFileStat::_remoteFileStat()
   {
   }

   _remoteFileStat::~_remoteFileStat()
   {
   }

   const CHAR* _remoteFileStat::name()
   {
      return OMA_REMOTE_FILE_STAT ;
   }

   INT32 _remoteFileStat::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string pathname ;
      string err ;

      // get argument
      if ( FALSE == _matchObj.hasField( "filename" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filename must be config" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "filename" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filename must be string" ) ;
         goto error ;
      }
      pathname = _matchObj.getStringField( "filename" ) ;

      rc = _sptUsrFileCommon::getStat( pathname, err, retObj ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteFileMd5 implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileMd5 )

   _remoteFileMd5::_remoteFileMd5()
   {
   }

   _remoteFileMd5::~_remoteFileMd5()
   {
   }

   const CHAR* _remoteFileMd5::name()
   {
      return OMA_REMOTE_FILE_MD5 ;
   }

   INT32 _remoteFileMd5::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string filename ;
      string err ;
      string code ;

      // check, we need 1 argument filename
      if ( FALSE == _matchObj.hasField( "filename" ) )
      {
         rc = SDB_INVALIDARG  ;
         err = "filename must be config" ;
         goto error ;
      }
      if ( String != _matchObj.getField( "filename" ).type() )
      {
         rc = SDB_INVALIDARG ;
         err = "filename must be string" ;
         goto error ;
      }
      filename = _matchObj.getStringField( "filename" ) ;

      rc = _sptUsrFileCommon::md5( filename, err, code ) ;
      if( SDB_OK != rc )
      {
         goto error ;
      }
      retObj = BSON( "md5" << code.c_str() ) ;
   done:
      return rc ;
   error:
      PD_LOG_MSG( PDERROR, err.c_str() ) ;
      goto done ;
   }


   /*
      _remoteFileGetSize implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileGetSize )

   _remoteFileGetSize::_remoteFileGetSize()
   {
   }

   _remoteFileGetSize::~_remoteFileGetSize()
   {
   }

   const CHAR* _remoteFileGetSize::name()
   {
      return OMA_REMOTE_FILE_GET_CONTENT_SIZE ;
   }

   INT32 _remoteFileGetSize::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      INT64 size = 0 ;
      string name ;
      string err ;

      // get filename
      if ( FALSE == _matchObj.hasField( "filename" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "filename must be config" ) ;
         goto error ;
      }
      if ( String != _matchObj.getField( "filename" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "filename must be string" ) ;
         goto error ;
      }
      name = _matchObj.getStringField( "filename" ) ;

      rc = _sptUsrFileCommon::getFileSize( name, err, size ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
      retObj = BSON( "size" << size ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

    /*
      _remoteFileGetPermission implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _remoteFileGetPermission )

   _remoteFileGetPermission::_remoteFileGetPermission()
   {
   }

   _remoteFileGetPermission::~_remoteFileGetPermission()
   {
   }

   const CHAR* _remoteFileGetPermission::name()
   {
      return OMA_REMOTE_FILE_GET_PERMISSION ;
   }

   INT32 _remoteFileGetPermission::doit( BSONObj &retObj )
   {
      INT32 rc = SDB_OK ;
      string pathname ;
      string err ;
      INT32 permission = 0 ;

      // get pathname
      if( FALSE == _valueObj.hasField( "pathname" ) )
      {
         rc = SDB_OUT_OF_BOUND ;
         PD_LOG_MSG( PDERROR, "pathname must be config" ) ;
         goto error ;
      }
      else if( String != _valueObj.getField( "pathname" ).type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "pathname must be string" ) ;
         goto error ;
      }
      pathname = _valueObj.getStringField( "pathname" ) ;

      rc = _sptUsrFileCommon::getPermission( pathname, err, permission ) ;
      if( SDB_OK != rc )
      {
         PD_LOG_MSG( PDERROR, err.c_str() ) ;
         goto error ;
      }
      retObj = BSON( "permission" << permission ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}
