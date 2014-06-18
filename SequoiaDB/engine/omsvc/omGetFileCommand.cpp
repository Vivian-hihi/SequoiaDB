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

   Source File Name = omGetFileCommand.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/12/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#include "omGetFileCommand.hpp"
#include "omDef.hpp"
#include "rtn.hpp"
#include "omManager.hpp"


using namespace bson;

namespace engine
{
   omAuthCommand::omAuthCommand( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession, 
                                 const CHAR *pRootPath )
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
      _rootPath    = pRootPath ;
   }

   omAuthCommand::~omAuthCommand()
   {
   }

   INT32 omAuthCommand::_verifyUser( const CHAR *pUserName, const CHAR *pPasswd, 
                                     const CHAR *pTimestamp ) 
   {
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj bsonQuery ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID   = -1 ;
      SINT64 startingPos = 0 ;
      rtnContextBuf buffObj ;
      INT32 rc = SDB_OK ;
      
      bsonBuilder.append( OM_USER_FIELD_NAME, pUserName ) ;
      bsonBuilder.append( OM_USER_FIELD_PASSWD, pPasswd ) ;
      bsonQuery = bsonBuilder.obj() ;
      rc = rtnQuery( OM_CS_AUTH_CL_USER, selector, bsonQuery, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to query:%d", rc ) ;
         goto error ;
      }
      
      rc = rtnGetMore( contextID, -1, buffObj, startingPos, _cb, _pRTNCB ) ;
      if ( SDB_OK != rc && SDB_DMS_EOC != rc )
      {
         PD_LOG( PDERROR, "failed to getmore:%d",rc ) ;
         rc = SDB_AUTH_AUTHORITY_FORBIDDEN ;
         goto error ;
      }
      else if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_AUTH_AUTHORITY_FORBIDDEN ;
         goto error ;
      }
      else if ( 0 == buffObj.recordNum() )
      {
         rc = SDB_AUTH_AUTHORITY_FORBIDDEN ;
         goto error ;
      }
      else if ( 1 == buffObj.recordNum() )
      {
         rc = SDB_OK ;
      }
      else
      {
         PD_LOG( PDERROR, "get more than one record, impossible" ) ;
         rc = SDB_SYS ;
         SDB_ASSERT( FALSE, "impossible" ) ;
         goto error ;
      }

   done:

      if ( -1 != contextID )
      {
         rtnKillContexts( 1, &contextID, _cb, _pRTNCB ) ;
      }
      return rc ;
      
   error:
      goto done ;
   }

   INT32 omAuthCommand::doCommand()
   {
      const CHAR *pUserName        = NULL ;
      const CHAR *pPasswd          = NULL ;
      CHAR *pTimestamp             = NULL ;
      INT32 rc                     = SDB_OK ;
      ossSocket *socket            = NULL ;
      restSessionInfo *sessionInfo = NULL ;
      BSONObjBuilder bsonBuilder ;
      BSONObj bsonRes ;

      _restAdaptor->getQuery(_restSession, OM_LOGIN_USERNAME, &pUserName ) ;
      _restAdaptor->getQuery(_restSession, OM_LOGIN_PASSWD, &pPasswd ) ;

      if ( ( NULL == pUserName ) || ( NULL == pPasswd ) )
      {
         rc = SDB_INVALIDARG ;
         bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
         bsonBuilder.append( OM_REST_RES_DETAIL, 
                             "username or passwd is null" ) ;
         bsonRes = bsonBuilder.obj() ;
         goto error ;
      }

      rc = _verifyUser( pUserName, pPasswd, pTimestamp ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_AUTH_AUTHORITY_FORBIDDEN == rc )
         {
            bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
            bsonBuilder.append( OM_REST_RES_DETAIL, 
                                "username or passwd is wrong" ) ;
            bsonRes = bsonBuilder.obj() ;
         }
         else
         {
            bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
            bsonBuilder.append( OM_REST_RES_DETAIL, "system error" ) ;
            bsonRes = bsonBuilder.obj() ;
         }
         
         goto error ;
      }
      
      socket = _restSession->socket() ;
      if ( NULL == socket )
      {
         PD_LOG( PDERROR, "socket is null, impossible" ) ;
         rc = SDB_SYS ;
      }
      
      sessionInfo = sdbGetOMManager()->newSessionInfo(pUserName, 
                                                      socket->getLocalIP() ) ;
      if ( NULL == sessionInfo )
      {
         PD_LOG( PDERROR, "new session failed:user=%s, ip=%u", pUserName,
                 socket->getLocalIP() ) ;
         rc = SDB_SYS ;
      }

      // login in success here;
//      if ( ossStrcmp(pPasswd, OM_DEFAULT_LOGIN_PASSWD) == 0 )
//      {
//         rc = SDB_OM_PASSWD_CHANGE_SUGGUEST ;
//         bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
//         bsonBuilder.append( OM_REST_RES_DETAIL, 
//                             "passwd is never changed" ) ;
//      }

      sessionInfo->_authOK = TRUE ;
      bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      bsonBuilder.append( OM_REST_RES_LOCAL, "/"OM_REST_INDEX_HTML ) ;
      bsonRes = bsonBuilder.obj() ;
      _restAdaptor->appendHttpHeader( _restSession, FIELD_NAME_SESSIONID, 
                                      sessionInfo->_id.c_str() ) ;

   done:
      _restAdaptor->setOPResult( _restSession, rc, bsonRes ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      
      return SDB_OK ;

   error:
      goto done ;
   }

   omCheckSessionCommand::omCheckSessionCommand( restAdaptor *pRestAdaptor, 
                                                 pmdRestSession *pRestSession )
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
   }

   omCheckSessionCommand::~omCheckSessionCommand()
   {
      
   }

   INT32 omCheckSessionCommand::doCommand()
   {
      BSONObjBuilder bsonBuilder ;
      const CHAR* sessionID = NULL ;
      _restAdaptor->getHttpHeader( _restSession, FIELD_NAME_SESSIONID, 
                                   &sessionID ) ;
      if ( NULL != sessionID && _restSession->isAuthOK() )
      {
         bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
         _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      }
      else
      {
         PD_LOG( PDEVENT, "OM: redirect to:%s", OM_REST_LOGIN_HTML ) ;
         bsonBuilder.append( OM_REST_RES_RETCODE, 
                             SDB_AUTH_AUTHORITY_FORBIDDEN ) ;
         bsonBuilder.append( OM_REST_RES_LOCAL, "/"OM_REST_LOGIN_HTML ) ;
         _restAdaptor->setOPResult( _restSession, SDB_AUTH_AUTHORITY_FORBIDDEN, 
                                    bsonBuilder.obj() ) ;
         _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      }

      return SDB_OK ;
   }

   omCreateClusterCommand::omCreateClusterCommand( restAdaptor *pRestAdaptor, 
                                                  pmdRestSession *pRestSession )
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
   }

   omCreateClusterCommand::~omCreateClusterCommand()
   {
   }

   INT32 omCreateClusterCommand::doCommand()
   {
      return SDB_OK ;
   }


   omGetFileCommand::omGetFileCommand( restAdaptor *pRestAdaptor, 
                                       pmdRestSession *pRestSession, 
                                       const CHAR *pRootPath,
                                       const CHAR *pSubPath)
   {
      _restAdaptor = pRestAdaptor ;
      _restSession = pRestSession ;
      _rootPath    = pRootPath ;
      _subPath     = pSubPath ;
   }
   
   omGetFileCommand::~omGetFileCommand()
   {
   }
   
   INT32 omGetFileCommand::doCommand() 
   {
      INT32 rc                      = SDB_OK ;
      CHAR *pContent                = NULL ;
      INT32 contentLength           = 0 ;
      restFileController* transfer = NULL ;
      string realSubPath            = _subPath ;

      transfer = restFileController::getTransferInstance() ;
      transfer->getTransferedPath(_subPath.c_str(), realSubPath) ;
      
      rc = _getFileContent( _rootPath + realSubPath, &pContent, 
                            contentLength ) ;
      if ( SDB_OK != rc )
      {
         HTTP_FILE_TYPE file_type = _restAdaptor->getFileType( _restSession ) ;
         if ( HTTP_FILE_HTML == file_type || HTTP_FILE_DEFAULT == file_type )
         {
            PD_LOG( PDEVENT, "OM: 2file no found:%s", realSubPath.c_str() ) ;
            _restAdaptor->appendHttpBody( _restSession, 
                                          OM_REST_REDIRECT_LOGIN, 
                                          ossStrlen(OM_REST_REDIRECT_LOGIN) ) ;
            _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
         }
         else
         {
            PD_LOG( PDEVENT, "OM: file no found:%s", realSubPath.c_str() ) ;
            _restAdaptor->sendResponse( _restSession, HTTP_NOTFOUND ) ;
         }
         
         goto error ;
      }

      _restAdaptor->appendHttpBody( _restSession, pContent, contentLength ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;

      _restSession->releaseBuff( pContent, contentLength ) ;

   done:
      return SDB_OK ;
   error:
      goto done ;
   }
   
   INT32 omGetFileCommand::undoCommand()
   {
      return SDB_OK ;
   }

   INT32 omGetFileCommand::_getFileContent( string filePath, 
                                            CHAR **pFileContent, 
                                            INT32 &fileContentLen )
   {
      OSSFILE file ;
      INT32 rc               = SDB_OK ;
      INT64 fileSize         = 0 ;
      SINT64 realFileSize    = 0 ;
      INT32 buffSize         = 0 ;
      bool isFileOpened      = false ;
      bool isAllocBuff       = false ;

      rc = ossOpen( filePath.c_str(), OSS_READONLY, OSS_RWXU, file ) ;
      PD_RC_CHECK ( rc, PDERROR, 
                    "Failed to open file:file=%s, rc = %d", 
                    filePath.c_str(), rc ) ;

      isFileOpened = true ;
      rc           = ossGetFileSize( &file, &fileSize ) ;
      PD_RC_CHECK ( rc, PDERROR, 
                    "Failed to get file size:file=%s, rc = %d", 
                    filePath.c_str(), rc ) ;

      rc = _restSession->allocBuff( fileSize, pFileContent, buffSize ) ;
      PD_RC_CHECK ( rc, PDERROR, 
                    "Failed to alloc buff:buff_size=%I64d, rc = %d", 
                    fileSize, rc ) ;

      isAllocBuff = true ;
      rc          = ossRead( &file, *pFileContent, fileSize, &realFileSize ) ;
      PD_RC_CHECK ( rc, PDERROR, 
                    "Failed to read file content:file=%s, rc = %d", 
                    filePath.c_str(), rc ) ;

      fileContentLen = realFileSize ;

   done:
      if ( isFileOpened )
      {
         ossClose( file ) ;
      }

      if ( isAllocBuff )
      {
         _restSession->releaseBuff( *pFileContent, buffSize ) ;
      }

      return rc ;
   error:
      goto done ;
   }


   restFileController::restFileController()
   {
      INT32 pairCount   = 0 ;
      INT32 i           = 0 ;
      INT32 publicCount = 0 ;

      static char* filePathTable[][2] = {
         {"/",    "/"OM_REST_INDEX_HTML}
      } ;

      // only html file, other file is all public now(from jiawen)
      static char *fileAuthorityPublic[] = {
         "/"OM_REST_LOGIN_HTML ,
      } ;

      pairCount = sizeof( filePathTable ) / ( 2 * sizeof ( char * ) ) ;

      for ( i = 0 ; i < pairCount ; i++ )
      {
         _transfer.insert( mapValueType( filePathTable[i][0], 
                                                    filePathTable[i][1])) ; 
      }

      publicCount = sizeof( fileAuthorityPublic ) / ( sizeof ( char * ) ) ;
      for ( i = 0 ; i < publicCount ; i++ )
      {
         _publicAccessFiles.insert( mapValueType( fileAuthorityPublic[i], 
                                                  fileAuthorityPublic[i])) ; 
      }
   }

   restFileController* restFileController::getTransferInstance()
   {
      static restFileController instance ;
      return &instance ;
   }

   INT32 restFileController::getTransferedPath( const char *src, 
                                                   string &transfered )
   {
      INT32 rc = SDB_OK ;
      mapIteratorType ite = _transfer.find( src ) ;
      if ( ite == _transfer.end() )
      {
         goto error ;
      }

      transfered = ite->second ;

   done:
      return rc ; 
   error:
      rc = -1 ;
      goto done ;
   }

   bool restFileController::isFileAuthorPublic( const char *file ) 
   {
      mapIteratorType ite = _publicAccessFiles.find( file ) ;
      if ( ite == _publicAccessFiles.end() )
      {
         return false ;
      }

      return true ;
   }

}

