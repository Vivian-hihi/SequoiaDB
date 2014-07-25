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

   Source File Name = pmdRestSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdRestSession.hpp"
#include "omManager.hpp"
#include "pmdEDUMgr.hpp"
#include "msgDef.h"
#include "pmdCommon.hpp"
#include "ossMem.hpp"
#include "../omsvc/omGetFileCommand.hpp"

#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   static void _sendOpError2Web ( INT32 rc, restAdaptor *pAdptor, 
                                  pmdRestSession *pRestSession,
                                  pmdEDUCB* pEduCB ) ;

   void _sendOpError2Web ( INT32 rc, restAdaptor *pAdptor, 
                           pmdRestSession *pRestSession,
                           pmdEDUCB* pEduCB )
   {
      BSONObj _errorInfo = pmdGetErrorBson( rc, pEduCB->getInfo( 
                                            EDU_INFO_ERROR ) ) ;
      pAdptor->setOPResult( pRestSession, rc, _errorInfo ) ;
      pAdptor->sendResponse( pRestSession, HTTP_OK ) ;
   }

   /*
      _pmdRestSession implement
   */
   _pmdRestSession::_pmdRestSession( SOCKET fd )
   :_pmdLocalSession( fd )
   {
      _pFixBuff         = NULL ;
      _pSessionInfo     = NULL ;

      _wwwRootPath      = pmdGetOptionCB()->getWWWPath() ;
   }

   _pmdRestSession::~_pmdRestSession()
   {
      if ( _pFixBuff )
      {
         sdbGetOMManager()->releaseFixBuf( _pFixBuff ) ;
         _pFixBuff = NULL ;
      }
   }

   UINT64 _pmdRestSession::identifyID()
   {
      // TODO:XUJIANHUI
      return 0 ;
   }

   INT32 _pmdRestSession::run()
   {
      INT32 rc                         = SDB_OK ;
      restAdaptor *pAdptor             = sdbGetOMManager()->getRestAdptor() ;
      pmdEDUMgr *pEDUMgr               = NULL ;
      const CHAR *pSessionID           = NULL ;
      HTTP_PARSE_COMMON httpCommon     = COM_GETFILE ;
      CHAR *pFilePath                  = NULL ;
      INT32 bodySize                   = 0 ;

      if ( !_pEDUCB )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      pEDUMgr = _pEDUCB->getEDUMgr() ;

      while ( !_pEDUCB->isDisconnected() && !_socket.isClosed() )
      {
         // sniff wether has data
         rc = sniffData() ;
         if ( SDB_TIMEOUT == rc )
         {
            continue ;
         }
         else if ( rc < 0 )
         {
            break ;
         }

         // if interrupted, kill all context
         if ( _pEDUCB->isInterrupted( TRUE ) )
         {
            // delete all context
            INT64 contextID = -1 ;
            while ( -1 != ( contextID = _pEDUCB->contextPeek() ) )
            {
               _pRTNCB->contextDelete( contextID, NULL ) ;
            }
         }

         _pEDUCB->resetInterrupt() ;
         _pEDUCB->resetInfo( EDU_INFO_ERROR ) ;

         // recv rest header
         rc = pAdptor->recvRequestHeader( this ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to recv rest header, "
                    "rc: %d", sessionName(), rc ) ;
            if ( SDB_REST_EHS == rc )
            {
               pAdptor->sendResponse( this, HTTP_BADREQ ) ;
            }
            else if ( SDB_APP_FORCED != rc )
            {
               _sendOpError2Web( rc, pAdptor, this, _pEDUCB ) ;
            }
            
            break ;
         }
         // session is not exist
         if ( !_pSessionInfo )
         {
            // find session id
            pAdptor->getHttpHeader( this, FIELD_NAME_SESSIONID, &pSessionID ) ;
            // if 'SessionID' exist, attach the sessionInfo
            if ( pSessionID )
            {
               PD_LOG( PDINFO, "Rest session: %s", pSessionID ) ;
               _pSessionInfo = sdbGetOMManager()->attachSessionInfo(
                                  pSessionID ) ;
            }

            // if session exist, restore
            if ( _pSessionInfo )
            {
               restoreSession( _pSessionInfo ) ;
            }
         }
         // recv body
         rc = pAdptor->recvRequestBody( this, httpCommon, &pFilePath, 
                                        bodySize ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] failed to recv rest body, "
                    "rc: %d", sessionName(), rc ) ;
            if ( SDB_REST_EHS == rc )
            {
               pAdptor->sendResponse( this, HTTP_BADREQ ) ;
            }
            else if ( SDB_APP_FORCED != rc )
            {
               _sendOpError2Web( rc, pAdptor, this, _pEDUCB ) ;
            }
            
            break ;
         }
         
         // increase process event count
         _pEDUCB->incEventCount() ;
         
         // activate edu
         if ( SDB_OK != ( rc = pEDUMgr->activateEDU( _pEDUCB ) ) )
         {
            PD_LOG( PDERROR, "Session[%s] activate edu failed, rc: %d",
                    sessionName(), rc ) ;
            break ;
         }

         // process msg
         rc = _processRestMsg( httpCommon, pFilePath ) ;
         if ( rc )
         {
            break ;
         }
         
         // wait edu
         if ( SDB_OK != ( rc = pEDUMgr->waitEDU( _pEDUCB ) ) )
         {
            PD_LOG( PDERROR, "Session[%s] wait edu failed, rc: %d",
                    sessionName(), rc ) ;
            break ;
         }
         
         // release body msg
         releaseBuff( pFilePath, bodySize ) ;
         rc = SDB_OK ;
      } // end while

      disconnect() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRestSession::_processRestMsg( HTTP_PARSE_COMMON command, 
                                           const CHAR *pFilePath )
   {
      restAdaptor *pAdptor           = NULL ;
      omCommandInterface *pOmCommand = NULL ;
      pAdptor = sdbGetOMManager()->getRestAdptor() ;

      pOmCommand = _createCommand( command, pFilePath ) ;
      if ( NULL == pOmCommand )
      {
         goto error ;
      }
      
      pOmCommand->init( _pEDUCB ) ;
      pOmCommand->doCommand() ;

   done:
      if ( NULL != pOmCommand )
      {
         SDB_OSS_DEL pOmCommand ;
      }
      return SDB_OK ;

   error:
      goto done ;
   }

   omCommandInterface *_pmdRestSession::_createCommand( 
                                HTTP_PARSE_COMMON command, 
                                const CHAR *pFilePath )
   {
      omCommandInterface *commandIf = NULL ;
      restAdaptor *pAdptor          = NULL ;

      pAdptor = sdbGetOMManager()->getRestAdptor() ;

      if ( COM_GETFILE == command )
      {
         PD_LOG( PDEVENT, "OM: getfile command:file=%s", pFilePath ) ;
         commandIf = SDB_OSS_NEW omGetFileCommand( pAdptor, this,
                                                   _wwwRootPath.c_str(),
                                                   pFilePath ) ;
      }
      else 
      {
         const CHAR *pSubCommand = NULL ;
         pAdptor->getQuery( this, OM_REST_FIELD_COMMAND, &pSubCommand ) ;
         if ( NULL == pSubCommand )
         {
            BSONObjBuilder builder ;
            builder.append( OM_REST_RES_RETCODE, SDB_INVALIDARG ) ;
            builder.append( OM_REST_RES_DETAIL, "command is null" ) ;
            pAdptor->setOPResult( this, SDB_INVALIDARG, builder.obj() ) ;
            pAdptor->sendResponse( this, HTTP_OK ) ;
            goto error ;
         }

         PD_LOG( PDEVENT, "OM: command:command=%s", pSubCommand ) ;
         //TODO temperately close authrity!!!!!!!!!!!
//         if ( ossStrcmp( pSubCommand, OM_LOGIN_REQ ) != 0
//              && ossStrcmp( pSubCommand, OM_CHECK_SESSION_REQ ) != 0
//              && !isAuthOK() )
//         {
//            // except login_rep and check_seesion_req, other commands can only 
//            // execute in authrity status
//            BSONObjBuilder builder ;
//            builder.append( OM_REST_RES_RETCODE, 
//                            SDB_AUTH_AUTHORITY_FORBIDDEN ) ;
//            builder.append( OM_REST_RES_LOCAL, "/"OM_REST_LOGIN_HTML ) ;
//            pAdptor->setOPResult( this, SDB_AUTH_AUTHORITY_FORBIDDEN, 
//                                  builder.obj() ) ;
//            pAdptor->sendResponse( this, HTTP_OK ) ;
//            PD_LOG( PDEVENT, "OM: redirect to:%s", OM_REST_LOGIN_HTML ) ;
//            goto error ;
//         }
         
         if ( ossStrcasecmp( pSubCommand, OM_LOGIN_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omAuthCommand( pAdptor, this, 
                                                   _wwwRootPath.c_str() ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_CHECK_SESSION_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omCheckSessionCommand( pAdptor, this ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_CREATE_CLUSTER_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omCreateClusterCommand( pAdptor, this ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_QUERY_CLUSTER_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omQueryClusterCommand( pAdptor, this ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_SCAN_HOST_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omScanHostCommand( pAdptor, this, 
                                                       OM_DEFAULT_LOCAL_HOST, 
                                                       OM_AGENT_DEFAULT_PORT ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_CHECK_HOST_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omCheckHostCommand( pAdptor, this, 
                                                        OM_DEFAULT_LOCAL_HOST, 
                                                        OM_AGENT_DEFAULT_PORT ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_ADD_HOST_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omAddHostCommand( pAdptor, this, 
                                                      OM_DEFAULT_LOCAL_HOST, 
                                                      OM_AGENT_DEFAULT_PORT ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_QUERY_HOST_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omQueryHostCommand( pAdptor, this ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_QUERY_BUSINESS_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omQueryBusinessCommand( pAdptor, this, 
                                                            _wwwRootPath.c_str(), 
                                                            pFilePath ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, 
                                  OM_QUERY_BUSINESS_TEMPLATE_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omQueryBusinessTemplateCommand( pAdptor, 
                                                                    this, 
                                                                    _wwwRootPath.c_str(), 
                                                                    pFilePath ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_CONFIG_BUSINESS_REQ ) == 0 )
         {
            commandIf = SDB_OSS_NEW omConfigBusinessCommand( pAdptor, this, 
                                                             _wwwRootPath.c_str(), 
                                                             pFilePath ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, OM_INSTALL_BUSINESS_REQ) == 0 )
         {
            commandIf = SDB_OSS_NEW omInstallBusinessReq( pAdptor, this, 
                                                          _wwwRootPath.c_str(), 
                                                          pFilePath, 
                                                          OM_DEFAULT_LOCAL_HOST, 
                                                          OM_AGENT_DEFAULT_PORT ) ;
         }
         else if ( ossStrcasecmp( pSubCommand, 
                                  OM_QUERY_INSTALL_PROGRESS ) == 0 )
         {
            commandIf = SDB_OSS_NEW omQueryInstallProgress( pAdptor, this ) ;
         }
         else
         {
            BSONObjBuilder builder ;
            string errorInfo = string("command is unreconigzed:") + pSubCommand ;
            builder.append( OM_REST_RES_RETCODE, SDB_INVALIDARG ) ;
            builder.append( OM_REST_RES_DETAIL, errorInfo.c_str() ) ;
            pAdptor->setOPResult( this, SDB_INVALIDARG, builder.obj() ) ;
            pAdptor->sendResponse( this, HTTP_OK ) ;
            goto error ;
         }
      }

   done:
      return commandIf ;
   error:
      goto done ;
   }

   void _pmdRestSession::_onAttach()
   {
   }

   void _pmdRestSession::_onDetach()
   {
   }

   INT32 _pmdRestSession::_onAuth( MsgHeader * msg )
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   INT32 _pmdRestSession::getFixBuffSize() const
   {
      return sdbGetOMManager()->getFixBufSize() ;
   }

   CHAR* _pmdRestSession::getFixBuff ()
   {
      if ( !_pFixBuff )
      {
         _pFixBuff = sdbGetOMManager()->allocFixBuf() ;
      }
      return _pFixBuff ;
   }

   void _pmdRestSession::restoreSession( restSessionInfo *pSessionInfo )
   {
      _pSessionInfo = pSessionInfo ;
   }

   void _pmdRestSession::saveSession( restSessionInfo &sessionInfo )
   {
   }

   bool _pmdRestSession::isAuthOK()
   {
      if ( NULL != _pSessionInfo )
      {
         if ( _pSessionInfo->_authOK )
         {
            return true ;
         }
      }

      return false ;
   }

   const CHAR* _pmdRestSession::getSessionID()
   {
      if ( NULL != _pSessionInfo )
      {
         if ( _pSessionInfo->_authOK )
         {
            return _pSessionInfo->_id.c_str();
         }
      }

      return NULL ;
   }

}

