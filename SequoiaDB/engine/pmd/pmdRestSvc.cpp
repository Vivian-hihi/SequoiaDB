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

   Source File Name = pmdRestSvc.cpp

   Descriptive Name = Process MoDel HTTP Listener ( REST requests )

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main entry point for HTTP
   Listener.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          14/04/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "pmdEDUMgr.hpp"
#include "ossSocket.hpp"
#include "pmdRestSession.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{

   /*
      rest service entry point
   */
   INT32 pmdRestSvcEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc                = SDB_OK ;
      pmdKRCB *krcb           = pmdGetKRCB() ;
      monDBCB *mondbcb        = krcb->getMonDBCB () ;
      pmdEDUMgr *eduMgr       = cb->getEDUMgr() ;

      BOOLEAN isLatched       = FALSE ;
      EDUID agentEDU          = PMD_INVALID_EDUID ;

      UINT16 port             = 0 ;
      const CHAR *restService = pmdGetKRCB()->getRestAddr () ;

      if ( SDB_OK != ( rc = eduMgr->activateEDU ( cb )) )
      {
         goto error ;
      }

      rc = ossGetPort ( restService, port ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get port from service name: %s, rc = %d",
                  restService, rc ) ;
         goto error ;
      }

      PD_LOG ( PDEVENT, "Rest Service Listening on port %d\n", port ) ;

      // create listen
      {
         ossSocketBindListenMutexGet() ;
         isLatched = TRUE ;

         ossSocket sock ( port ) ;
         rc = sock.initSocket () ;
         PD_RC_CHECK( rc, PDERROR, "Init rest service socket failed, rc: %d",
                      rc ) ;

         rc = sock.bind_listen () ;
         PD_RC_CHECK( rc, PDERROR, "Bind rest service socket failed, rc: %d",
                      rc ) ;

         while ( !cb->isDisconnected() )
         {
            SOCKET s ;
            rc = sock.accept ( &s, NULL, NULL ) ;
            if ( isLatched )
            {
               ossSocketBindListenMutexRelease() ;
               isLatched = FALSE ;
            }
            // if we don't get anything for a period of time, let's loop
            if ( SDB_TIMEOUT == rc )
            {
               rc = SDB_OK ;
               continue ;
            }
            // if we receive error due to database down, we finish
            if ( rc && PMD_IS_DB_DOWN )
            {
               rc = SDB_OK ;
               goto done ;
            }
            else if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to accept rest socket, rc: %d",
                        rc ) ;
               if ( sock.isClosed() )
               {
                  break ;
               }
               else
               {
                  continue ;
               }
            }

            cb->incEventCount() ;
            ++mondbcb->numConnects ;

            // assign the socket to the arg
            void *pData = NULL ;
            *((SOCKET *) &pData) = s ;

            // now we have a tcp socket for a new connection, let's get an agent
            // Note the new new socket sent passing to startEDU
            rc = eduMgr->startEDU ( EDU_TYPE_RESTAGENT, pData, &agentEDU ) ;

            if ( rc )
            {
               PD_LOG( ( rc == SDB_QUIESCED ? PDWARNING : PDERROR ),
                       "Failed to start edu, rc: %d", rc ) ;

               // close remote connection if we can't create new thread
               ossSocket newsock ( &s ) ;
               newsock.close () ;
               continue ;
            }
         } //while ( ! cb->isDisconnected() )
      }

   done :
      return rc ;
   error :
      if ( isLatched )
      {
         ossSocketBindListenMutexRelease() ;
      }
      goto done ;
   }

   /*
      rest agent entry point
   */
   INT32 pmdRestAgentEntryPoint( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;

      SOCKET s = *(( SOCKET *) &pData ) ;

      pmdRestSession restSession( s ) ;
      restSession.attach( cb ) ;
      rc = restSession.run() ;
      restSession.detach() ;

      return rc ;
   }

}

