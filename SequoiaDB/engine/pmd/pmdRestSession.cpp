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
      INT32 rc = SDB_OK ;
      restAdaptor *pAdptor = sdbGetOMManager()->getRestAdptor() ;
      pmdEDUMgr *pEDUMgr = NULL ;
      const CHAR *pSessionID = NULL ;

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
            else
            {
            }
         }
         
      } // end while
      

   done:
      return rc ;
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

   void _pmdRestSession::restoreSession( restSessionInfo &sessionInfo )
   {
   }

   void _pmdRestSession::saveSession( restSessionInfo &sessionInfo )
   {
   }

}

