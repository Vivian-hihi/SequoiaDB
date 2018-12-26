/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = utilRenameLogger.cpp

   Descriptive Name = util rename logger

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== ======== ==============================================
          12/17/2018  Ting YU  Initial Draft

   Last Changed =

*******************************************************************************/

#include "utilRenameLogger.hpp"
#include "utilStr.hpp"
#include "pmd.hpp"

using namespace std ;

namespace engine
{
   BOOLEAN utilStr2RenameLog( const string& str, utilRenameLog& log )
   {
      BOOLEAN isOk = TRUE ;
      INT32 fieldSize = 0 ;
      const CHAR* pLine = NULL ;
      const CHAR *pSep = NULL ;

      vector<string> lines = utilStrSplit( str, OSS_NEWLINE ) ;
      if( UTIL_RENAME_LOG_FIELD_NUM != lines.size() )
      {
         goto error ;
      }

      // first line
      pLine = lines.at( 0 ).c_str() ;
      pSep = ossStrchr( pLine, UTIL_RENAME_LOG_SEP ) ;
      if ( NULL == pSep )
      {
         goto error ;
      }
      fieldSize = pSep - pLine ;
      if (    ( sizeof( UTIL_RENAME_LOG_OLDNAME ) - 1 ) == fieldSize
           && 0 == ossStrncmp( pLine, UTIL_RENAME_LOG_OLDNAME, fieldSize ) )
      {
         // it is ok
      }
      else
      {
         goto error ;
      }
      ossStrncpy( log.oldName, pSep + 1, DMS_COLLECTION_SPACE_NAME_SZ ) ;

      // second line
      pLine = lines.at( 1 ).c_str() ;
      pSep = ossStrchr( pLine, UTIL_RENAME_LOG_SEP ) ;
      if ( NULL == pSep )
      {
         goto error ;
      }
      fieldSize = pSep - pLine ;
      if (    ( sizeof( UTIL_RENAME_LOG_NEWNAME ) - 1 ) == fieldSize
           && 0 == ossStrncmp( pLine, UTIL_RENAME_LOG_NEWNAME, fieldSize ) )
      {
         // it is ok
      }
      else
      {
         goto error ;
      }
      ossStrncpy( log.newName, pSep + 1, DMS_COLLECTION_SPACE_NAME_SZ ) ;


      /*
      vector<string> line1 = utilStrSplit( lines.at(0),
                                           UTIL_RENAME_LOG_SEP ) ;
      vector<string> line2 = utilStrSplit( lines.at(1),
                                           UTIL_RENAME_LOG_SEP ) ;
      if ( 0 != ossStrcmp( line1.at(0).c_str(), UTIL_RENAME_LOG_OLDNAME ) )
      {
         goto error ;
      }
      if ( 0 != ossStrcmp( line2.at(0).c_str(), UTIL_RENAME_LOG_NEWNAME ) )
      {
         goto error ;
      }
      ossStrncpy( log.oldName,
                  line1.at(1).c_str(),
                  DMS_COLLECTION_SPACE_NAME_SZ ) ;
      ossStrncpy( log.newName,
                  line2.at(1).c_str(),
                  DMS_COLLECTION_SPACE_NAME_SZ ) ;*/

      isOk = TRUE ;

   done :
      return isOk ;
   error :
      isOk = FALSE ;
      goto done ;
   }

   _utilRenameLogger::_utilRenameLogger()
   : _isOpened( FALSE ), _fileExist( FALSE )
   {
      ossMemset( _fileName, 0, OSS_MAX_PATHSIZE + 1 ) ;
   }

   _utilRenameLogger::~_utilRenameLogger()
   {
      if ( _isOpened )
      {
         ossClose( _file ) ;
      }
   }

   INT32 _utilRenameLogger::init( BOOLEAN* fileExist )
   {
      INT32 rc = SDB_OK ;

      rc = utilBuildFullPath( pmdGetOptionCB()->getDbPath(),
                              UTIL_RENAME_LOG_FILE_NAME,
                              OSS_MAX_PATHSIZE,
                              _fileName ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to build full file path , rc: %d", rc  );

      rc = ossFile::exists( _fileName, _fileExist ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to check existence of file[%s], rc: %d",
                    _fileName, rc ) ;

      if ( _fileExist )
      {
         INT32 mode = OSS_READWRITE ;
         INT32 permission = OSS_RU | OSS_WU | OSS_RG | OSS_RO ;
         rc = ossOpen( _fileName, mode, permission, _file ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to open file[%s], rc: %d", _fileName, rc ) ;
         _isOpened = TRUE ;
      }

      if ( fileExist )
      {
         *fileExist = _fileExist ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilRenameLogger::log( const utilRenameLog& log )
   {
      INT32 rc = SDB_OK ;
      SINT64 written = 0 ;
      INT32 mode = OSS_READWRITE | OSS_CREATE ;
      INT32 permission = OSS_RU | OSS_WU | OSS_RG | OSS_RO ;

      PD_CHECK ( !_fileExist, SDB_FE, error, PDERROR,
                 "File[%s] already exists, rc: %d", _fileName, rc ) ;

      // create file
      rc = ossOpen( _fileName, mode, permission, _file ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to open file[%s], rc: %d", _fileName, rc ) ;
      _isOpened = TRUE ;

      // write to file
      {
         CHAR strLog[ UTIL_RENAME_LOG_MAXLEN ] = { 0 } ;
         log.toString( strLog, UTIL_RENAME_LOG_MAXLEN ) ;
         rc = ossSeekAndWriteN( &_file, 0, strLog, ossStrlen( strLog ),
                                written ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to write start info into file[%s], rc: %d",
                       _fileName, rc ) ;
      }

      ossFsync( &_file ) ;

   done :
      if ( _isOpened )
      {
         ossClose( _file ) ;
      }
      return rc ;
   error :
      if ( _isOpened )
      {
         ossClose( _file ) ;
      }
      rc = ossDelete( _fileName ) ;
      rc = SDB_FNE == rc ? SDB_OK : rc ;
      goto done ;
   }

   INT32 _utilRenameLogger::load( utilRenameLog& log )
   {
      INT32 rc = SDB_OK ;

      INT64 fileSize = 0 ;
      SINT64 readSize = 0 ;
      CHAR* readBuf = NULL ;

      // expect file exists
      PD_CHECK ( _fileExist, SDB_FNE, error, PDERROR,
                 "File[%s] not exists, rc: %d", _fileName, rc ) ;

      // if the file too large, it is abnormal
      rc = ossGetFileSize( &_file, &fileSize ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to get size of file[%s], rc: %d", _fileName, rc ) ;

      if ( fileSize > UTIL_RENAME_LOG_MAXLEN )
      {
         rc = SDB_SYS ;
         PD_RC_CHECK ( rc, PDERROR,
                       "File size is too large[%s], rc: %d", _fileName, rc ) ;
         goto error ;
      }

      // read all string
      readBuf = ( CHAR* )SDB_OSS_MALLOC( fileSize + 1 ) ;
      if ( !readBuf )
      {
         rc = SDB_OOM ;
         goto error ;
      }
      ossMemset( readBuf, 0, fileSize + 1 ) ;

      rc = ossSeekAndReadN( &_file, 0, fileSize, readBuf, readSize ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to read file[%s], rc: %d", _fileName, rc ) ;

      // convert
      if ( !utilStr2RenameLog( readBuf, log ) )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR,
                 "Failed to convert string to rename info, rc: %d", rc ) ;
         goto error ;
      }

   done :
      if ( readBuf )
      {
         SDB_OSS_FREE( readBuf ) ;
         readBuf = NULL ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _utilRenameLogger::clear()
   {
      INT32 rc = SDB_OK ;

      if ( _isOpened )
      {
         ossClose( _file ) ;
         _isOpened = FALSE ;
      }
      rc = ossDelete( _fileName ) ;
      if ( SDB_FNE == rc )
      {
         rc = SDB_OK ;
      }
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to delete file[%s], rc: %d", _fileName, rc ) ;

   done :
      return rc ;
   error :
      goto done ;
   }

   const CHAR* _utilRenameLogger::fileName()
   {
      return _fileName ;
   }
}


