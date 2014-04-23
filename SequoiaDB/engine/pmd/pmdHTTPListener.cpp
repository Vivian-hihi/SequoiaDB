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

   Source File Name = pmdHTTPListener.cpp

   Descriptive Name = Process MoDel HTTP Listener ( REST requests )

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main entry point for HTTP
   Listener.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/04/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include <stdio.h>
#include "pd.hpp"
#include "ossMem.hpp"
#include "pmd.hpp"
#include "pmdEDUMgr.hpp"
#include "ossSocket.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   #define PMD_HTTPLISTENER_RETRY 5
   // Main function to handle new connection request
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDHTTPLSTNENPPNT, "pmdHTTPListenerEntryPoint" )
   INT32 pmdHTTPListenerEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDHTTPLSTNENPPNT );
      pmdKRCB *krcb = pmdGetKRCB() ;
      monDBCB *mondbcb = krcb->getMonDBCB () ;
      EDUID myEDUID = cb->getID () ;
      pmdEDUMgr * eduMgr = cb->getEDUMgr() ;
      UINT32 retry = 0 ;
      BOOLEAN isLatched = FALSE ;
      const CHAR *restService = pmdGetKRCB()->getRestAddr () ;
      EDUID agentEDU = PMD_INVALID_EDUID ;
      UINT16 port = 0 ;
      if ( SDB_OK != ( rc = eduMgr->activateEDU ( myEDUID )) )
      {
         goto error ;
      }

      rc = ossSocket::getPort ( restService, port ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get port from service name: %s, rc = %d",
                  restService, rc ) ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "HTTP Listening on port %d\n", port ) ;

      while ( retry <= PMD_HTTPLISTENER_RETRY && !PMD_IS_DB_DOWN )
      {
         retry ++ ;
         ossSocketBindListenMutexGet() ;
         isLatched = TRUE ;
         // http listener only perform accept, it never do recv or send, so
         // don't need to set timeout
         ossSocket sock ( port ) ;
         rc = sock.initSocket () ;
         SDB_VALIDATE_GOTOERROR ( SDB_OK==rc, rc, "Failed initialize socket" )

         rc = sock.bind_listen () ;
         SDB_VALIDATE_GOTOERROR ( SDB_OK==rc, rc, "Failed to bind/listen socket");

         // master loop for tcp listener
         while ( ! cb->isDisconnected() )
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
               // if we fail due to error, let's restart socket
               PD_LOG ( PDERROR, "Failed to accept socket in TcpListener" ) ;
               PD_LOG ( PDEVENT, "Restarting socket to listen" ) ;
               break ;
            }

            cb->incEventCount() ;
            ++mondbcb->numConnects ;

            // assign the socket to the arg
            void *pData = NULL ;
            *((SOCKET *) &pData) = s ;

            // now we have a tcp socket for a new connection, let's get an agent
            // Note the new new socket sent passing to startEDU
            rc = eduMgr->startEDU ( EDU_TYPE_HTTPAGENT, pData, &agentEDU ) ;

            if ( rc )
            {
               if ( rc == SDB_QUIESCED )
               {
                  // we cannot start EDU due to quiesced
                  PD_LOG ( PDWARNING, "Reject new connection due to quiesced database" ) ;
               }
               else
               {
                  PD_LOG ( PDERROR, "Failed to start EDU agent" ) ;
               }
               // close remote connection if we can't create new thread
               ossSocket newsock ( &s ) ;
               newsock.close () ;
               continue ;
            }

            // Now EDU is started and posted with the new socket, let's
            // get back to wait for another request
         } //while ( ! cb->isDisconnected() )
      } // while ( retry <= PMD_TCPLISTENER_RETRY )
   done :
      PD_TRACE_EXITRC ( SDB_PMDHTTPLSTNENPPNT, rc );
      return rc;

   error :
      switch ( rc )
      {
      case SDB_SYS :
         PD_LOG ( PDSEVERE, "System error occured" ) ;
         break ;
      default :
         PD_LOG ( PDSEVERE, "Internal error" ) ;
         break ;
      }
      if ( isLatched )
         ossSocketBindListenMutexRelease() ;
      goto done ;
   }
}
