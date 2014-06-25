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

   Source File Name = omMsgEventHandler.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          25/06/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "omMsgEventHandler.hpp"
#include "pmdEDU.hpp"
#include "pmdRemoteSession.hpp"

namespace engine
{

   /*
      _omMsgHandler implement
   */
   _omMsgHandler::_omMsgHandler( _pmdRemoteSessionMgr *pRSManager )
   {
      _pRSManager       = pRSManager ;
      _pMainCB          = NULL ;
      _pMsgAssist       = NULL ;
   }

   _omMsgHandler::~_omMsgHandler()
   {
      _pRSManager       = NULL ;
   }

   void _omMsgHandler::attach( _pmdEDUCB * cb, _netMsgAssister *pMsgAssist )
   {
      _pMainCB    = cb ;
      _pMsgAssist = pMsgAssist ;
   }

   void _omMsgHandler::detach()
   {
      _pMainCB    = NULL ;
      _pMsgAssist = NULL ;
   }

   INT32 _omMsgHandler::handleMsg( const NET_HANDLE &handle,
                                   const _MsgHeader *header,
                                   const CHAR *msg )
   {
      INT32 rc = SDB_OK ;
      CHAR *pNewMsg = NULL ;
      INT32 newBuffLen = 0 ;

      // main cb msg
      if ( header->TID == 0 )
      {
         SDB_ASSERT( _pMainCB && _pMsgAssist, "Main cb can't be NULL" ) ;
         if ( !_pMainCB || !_pMsgAssist )
         {
            PD_LOG( PDERROR, "Main cb or MsgAssit handler is null when recv "
                    "msg[opCode:%d]", header->opCode ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         // copy msg
         rc = _pMainCB->allocBuff( header->messageLength, &pNewMsg,
                                   newBuffLen ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to alloc memory for msg[opCode: %d, "
                    "len: %d], rc: %d", header->opCode, header->messageLength,
                    rc ) ;
            goto error ;
         }
         ossMemcpy( pNewMsg, msg, header->messageLength ) ;
         // store handle
         _pMsgAssist->pushMsgHandle( pNewMsg, handle ) ;
         // push event
         _pMainCB->postEvent( pmdEDUEvent(PMD_EDU_EVENT_MSG, TRUE, pNewMsg) ) ;
      }
      // session msg
      else
      {
         SDB_ASSERT( _pRSManager, "Remote Session Manager can't be NULL" ) ;
         rc = _pRSManager->pushMessage( handle, header ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Push message[opCode: %d, len] failed, rc: %d",
                    header->opCode, header->messageLength, rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      if ( pNewMsg )
      {
         _pMainCB->releaseBuff( pNewMsg ) ;
      }
      goto done ;
   }

   void _omMsgHandler::handleClose( const NET_HANDLE &handle, _MsgRouteID id )
   {
      SDB_ASSERT( _pRSManager, "Remote session manager can't be NULL" ) ;

      _pRSManager->handleClose( handle, id ) ;
   }

}


