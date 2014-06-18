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
#include "../omsvc/omGetFileCommand.hpp"

#include "../bson/bson.h"

using namespace bson ;

namespace engine
{

   #define REST_COMMAND_KEY     "cmd"

   /*
      _pmdRestSession implement
   */
   _pmdRestSession::_pmdRestSession( SOCKET fd )
   :_pmdLocalSession( fd )
   {
      _pFixBuff         = NULL ;
      _pSessionInfo     = NULL ;

      _wwwRootPath      = "./www" ;
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
      CHAR *pFilePath               = NULL ;
      INT32 bodySize                   = 0 ;
      BOOLEAN needReply                = FALSE ;

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

         _pEDUCB->resetInterrupt() ;
         _pEDUCB->resetInfo( EDU_INFO_ERROR ) ;
         needReply = TRUE ;

         // recv rest header
         rc = pAdptor->recvRequestHeader( this ) ;
         if ( rc )
         {
            if ( SDB_APP_FORCED != rc )
            {
               PD_LOG( PDERROR, "Session[%s] failed to recv rest header, "
                       "rc: %d", sessionName(), rc ) ;
            }
            else
            {
               needReply = FALSE ;
            }
            break ;
         }
         // session is not exist
         if ( !_pSessionInfo )
         {
            // find session id
            rc = pAdptor->getHttpHeader( this, FIELD_NAME_SESSIONID,
                                         &pSessionID ) ;
            // if 'SessionID' exist, attach the sessionInfo
            if ( pSessionID )
            {
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
         rc = pAdptor->recvRequestBody( this, httpCommon, &pFilePath, bodySize ) ;
         if ( rc )
         {
            if ( SDB_APP_FORCED != rc )
            {
               PD_LOG( PDERROR, "Session[%s] failed to recv rest body, "
                       "rc: %d", sessionName(), rc ) ;
            }
            else
            {
               needReply = FALSE ;
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
         needReply = FALSE ;
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

      if ( needReply && _socket.isConnected() )
      {
         _errorInfo = pmdGetErrorBson( rc, _pEDUCB->getInfo(
                                       EDU_INFO_ERROR ) ) ;
         pAdptor->setOPResult( this, rc, _errorInfo ) ;
         rc = pAdptor->sendResponse( this, HTTP_BADREQ ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Session[%s] send rest response failed, rc: %d",
                    sessionName(), rc ) ;
         }
      }

      disconnect() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRestSession::_processRestMsg( HTTP_PARSE_COMMON command, 
                                           const CHAR *pFilePath )
   {
      restAdaptor *pAdptor = NULL ;

      pAdptor = sdbGetOMManager()->getRestAdptor() ;
      switch(command)
      {
         case COM_GETFILE :
         {
            PD_LOG( PDEVENT, "getfile command:file=%s", pFilePath ) ;
            omGetFileCommand *pGetFileCommand = NULL ;
            pGetFileCommand = new omGetFileCommand(pAdptor, this, 
                                                   _wwwRootPath.c_str(), 
                                                   pFilePath) ;
            pGetFileCommand->init( _pEDUCB ) ;
            pGetFileCommand->doCommand() ;
            delete pGetFileCommand ;
            break ;
         }

         case COM_CMD :
         {
            const CHAR *pSubCommand = NULL ;
            pAdptor->getQuery( this, REST_COMMAND_KEY, &pSubCommand ) ;
            if ( NULL == pSubCommand )
            {
               // TODO: response
            }
            PD_LOG( PDEVENT, "CMD command:command=%s", pSubCommand ) ;
            if ( ossStrcmp( pSubCommand, OM_LOGIN_REQ ) == 0 )
            {
               omAuthCommand *pAuthCommand = NULL ; 
               pAuthCommand = new omAuthCommand (pAdptor, this, 
                                                 _wwwRootPath.c_str() ) ;
               pAuthCommand->init( _pEDUCB ) ;
               pAuthCommand->doCommand() ;
               delete pAuthCommand ;
            }
            
            break ;
         }

         default :
         {

            break ;
         }
      }

      return SDB_OK ;
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
   }

   void _pmdRestSession::saveSession( restSessionInfo &sessionInfo )
   {
   }

   bool _pmdRestSession::isAuthOK()
   {
      if ( NULL != _pSessionInfo )
      {
         if ( _pSessionInfo->_authOK != 0 )
         {
            return true ;
         }
      }

      return false ;
   }

}

