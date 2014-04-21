/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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

namespace engine
{

   /*
      _pmdRestSession implement
   */
   _pmdRestSession::_pmdRestSession( SOCKET fd )
   :_pmdLocalSession( fd )
   {
      _pFixBuff         = NULL ;
      _pSessionInfo     = NULL ;
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
      CHAR *pBody                      = NULL ;
      INT32 bodySize                   = 0 ;

      if ( !_pEDUCB )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      pEDUMgr = _pEDUCB->getEDUMgr() ;

      while ( !_pEDUCB->isDisconnected() && !_socket.isClosed() )
      {
         _pEDUCB->resetInterrupt() ;
         _pEDUCB->resetInfo( EDU_INFO_ERROR ) ;

         // recv rest header
         rc = pAdptor->getRequestHeader( this ) ;
         if ( rc )
         {
            if ( SDB_APP_FORCED != rc )
            {
               PD_LOG( PDERROR, "Session[%s] failed to recv rest header, "
                       "rc: %d", sessionName(), rc ) ;
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
         rc = pAdptor->getRequestBody( this, httpCommon, &pBody, bodySize ) ;
         if ( rc )
         {
            if ( SDB_APP_FORCED != rc )
            {
               PD_LOG( PDERROR, "Session[%s] failed to recv rest body, "
                       "rc: %d", sessionName(), rc ) ;
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
         rc = _processRestMsg( pBody, bodySize ) ;
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
         releaseBuff( pBody, bodySize ) ;
         rc = SDB_OK ;
      } // end while

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdRestSession::_processRestMsg( const CHAR *pData, INT32 dataLen )
   {
      // TODO:XUJIANHUI
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

}

