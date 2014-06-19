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
   // *****************omAuthCommand *****************************
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

   void omAuthCommand::_sendErrorRes2Web( INT32 rc, const CHAR* detail )
   {
      BSONObjBuilder bsonBuilder ;
      
      bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      if ( NULL != detail )
      {
         bsonBuilder.append( OM_REST_RES_DETAIL, detail ) ;
      }

      _restAdaptor->setOPResult( _restSession, rc, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
   }

   INT32 omAuthCommand::doCommand()
   {
      const CHAR *pUserName        = NULL ;
      const CHAR *pPasswd          = NULL ;
      INT32 rc                     = SDB_OK ;
      ossSocket *socket            = NULL ;
      restSessionInfo *sessionInfo = NULL ;
      BSONObjBuilder authBuilder ;
      BSONObjBuilder resBuilder ;
      BSONObj bsonRes ;
      BSONObj bsonAuth ;

      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_LOGIN_NAME, &pUserName ) ;
      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_LOGIN_PASSWD, &pPasswd ) ;

      if ( ( NULL == pUserName ) || ( NULL == pPasswd ) )
      {
         _sendErrorRes2Web( SDB_INVALIDARG, "username or passwd is null" ) ;
         goto error ;
      }

      authBuilder.append( SDB_AUTH_USER, pUserName ) ;
      authBuilder.append( SDB_AUTH_PASSWD, pPasswd ) ;
      bsonAuth = authBuilder.obj() ;
      rc = sdbGetOMManager()->authenticate( bsonAuth, _cb ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_AUTH_AUTHORITY_FORBIDDEN == rc )
         {
            _sendErrorRes2Web( rc, "username or passwd is wrong" ) ;
         }
         else
         {
            _sendErrorRes2Web( rc, "system error" ) ;
         }
         
         goto error ;
      }

      socket = _restSession->socket() ;
      if ( NULL == socket )
      {
         PD_LOG( PDERROR, "socket is null, impossible" ) ;
         _sendErrorRes2Web( SDB_SYS, "system error" ) ;
         goto error ;
      }

      if ( _restSession->isAuthOK() )
      {
         sdbGetOMManager()->releaseSessionInfo( _restSession->getSessionID() ) ;
      }
      
      sessionInfo = sdbGetOMManager()->newSessionInfo(pUserName, 
                                                      socket->getLocalIP() ) ;
      if ( NULL == sessionInfo )
      {
         PD_LOG( PDERROR, "new session failed:user=%s, ip=%u", pUserName,
                 socket->getLocalIP() ) ;
         _sendErrorRes2Web( SDB_SYS, "system error" ) ;
      }

      sessionInfo->_authOK = TRUE ;
      resBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      resBuilder.append( OM_REST_RES_LOCAL, "/"OM_REST_INDEX_HTML ) ;
      _restAdaptor->appendHttpHeader( _restSession, FIELD_NAME_SESSIONID, 
                                      sessionInfo->_id.c_str() ) ;
      _restAdaptor->setOPResult( _restSession, rc, resBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      
   done:
      return SDB_OK ;

   error:
      goto done ;
   }

   // *****************omCheckSessionCommand *****************************
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

   // *****************omCreateClusterCommand *****************************
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
      const CHAR *pClusterName = NULL ;
      const CHAR *pDesc        = NULL ;
      BSONObjBuilder bsonBuilder ;
      BSONObj bsonCluster ;
      INT32 rc                 = SDB_OK ;

      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_CLUSTER, 
                             &pClusterName ) ;
      if ( NULL == pClusterName )
      {
         _sendErrorRes2Web( SDB_INVALIDARG, "cluster name is null" ) ;
         goto error ;
      }

      // desc is not necessary
      _restAdaptor->getQuery(_restSession, OM_REST_FIELD_CLUSTER_DESC, 
                             &pDesc ) ;
      bsonBuilder.append( OM_CLUSTER_FIELD_NAME, pClusterName ) ;
      bsonBuilder.append( OM_CLUSTER_FIELD_DESC, pDesc ) ;
      // duplicate check depends on the unique index of table(OM_CS_DEPLOY_CL_CLUSTERIDX1)
      bsonCluster = bsonBuilder.obj() ;
      rc = rtnInsert( OM_CS_DEPLOY_CL_CLUSTER, bsonCluster, 1, 0, _cb );
      if ( rc )
      {
         if ( SDB_IXM_DUP_KEY == rc )
         {
            string errorInfo = string(pClusterName) + " is already exist" ;
            _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         }
         else
         {
            string errorInfo = string("failed to insert cluster:") 
                               + pClusterName ;
            PD_LOG( PDERROR, "OM: failed to insert cluster:%s", pClusterName ) ;
            _sendErrorRes2Web( rc, errorInfo.c_str() ) ;
         }

         goto error ;
      }

      bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      bsonBuilder.append( OM_REST_FIELD_CLUSTER, pClusterName ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      
   done:
      return SDB_OK ;
   error:
      goto done ;
   }

   void omCreateClusterCommand::_sendErrorRes2Web( INT32 rc, const CHAR* detail )
   {
      BSONObjBuilder bsonBuilder ;
      
      bsonBuilder.append( OM_REST_RES_RETCODE, rc ) ;
      if ( NULL != detail )
      {
         bsonBuilder.append( OM_REST_RES_DETAIL, detail ) ;
      }

      _restAdaptor->setOPResult( _restSession, rc, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
   }

   // *****************omQueryClusterCommand *****************************
   omQueryClusterCommand::omQueryClusterCommand( restAdaptor *pRestAdaptor, 
                                                 pmdRestSession *pRestSession )
                         : omCreateClusterCommand( pRestAdaptor, pRestSession )
   {
   }

   omQueryClusterCommand::~omQueryClusterCommand()
   {
   }

   INT32 omQueryClusterCommand::doCommand()
   {
      const CHAR *pClusterName = NULL ;
      BSONObjBuilder bsonBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      SINT64 contextID ;
      INT32 rc                 = SDB_OK ;

      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 0, 
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
      }

      bsonBuilder.append( OM_REST_RES_RETCODE, SDB_OK ) ;
      bsonBuilder.append( OM_REST_FIELD_CLUSTER, pClusterName ) ;
      _restAdaptor->setOPResult( _restSession, SDB_OK, bsonBuilder.obj() ) ;
      _restAdaptor->sendResponse( _restSession, HTTP_OK ) ;
      
   done:
      return SDB_OK ;
   error:
      goto done ;
   }

   // *****************omGetFileCommand *****************************
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
                                          OM_REST_REDIRECT_INDEX, 
                                          ossStrlen(OM_REST_REDIRECT_INDEX) ) ;
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

