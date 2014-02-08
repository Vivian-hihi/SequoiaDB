/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdTcpListener.cpp

   Descriptive Name = Process MoDel TCP Listener

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main entry point for TCP
   Listener.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

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
   #define PMD_TCPLISTENER_RETRY 5
   // Main function to handle new connection request
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDTCPLSTNENTPNT, "pmdTcpListenerEntryPoint" )
   INT32 pmdTcpListenerEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDTCPLSTNENTPNT );
      pmdKRCB *krcb = pmdGetKRCB() ;
      monDBCB *mondbcb = krcb->getMonDBCB () ;
      SDB_ROLE dbrole = krcb->getDBRole () ;
      EDUID myEDUID = cb->getID () ;
      pmdEDUMgr * eduMgr = cb->getEDUMgr() ;
      UINT32 retry = 0 ;
      BOOLEAN isLatched = FALSE ;
      EDUID agentEDU = PMD_INVALID_EDUID ;

      while ( retry <= PMD_TCPLISTENER_RETRY && !PMD_IS_DB_DOWN )
      {
         retry ++ ;
         // here we read from TCP socket
         UINT16 port = pmdGetKRCB()->getServicePort () ;

         PD_LOG ( PDEVENT, "Listening on port %d\n", port ) ;

         ossSocketBindListenMutexGet() ;
         isLatched = TRUE ;
         // no need to set timeout since tcp listener only accept, it never recv
         // or send
         ossSocket sock ( port ) ;
         rc = sock.initSocket () ;
         SDB_VALIDATE_GOTOERROR ( SDB_OK==rc, rc, "Failed initialize socket" )

         rc = sock.bind_listen () ;
         SDB_VALIDATE_GOTOERROR ( SDB_OK==rc, rc,
                                  "Failed to bind/listen socket");
         // once bind is successful, let's set the state of EDU to RUNNING
         if ( SDB_OK != ( rc = eduMgr->activateEDU ( myEDUID )) )
         {
            goto error ;
         }
         // master loop for tcp listener
         while ( ! cb->isDisconnected() )
         {
            SOCKET s ;
            // timeout in 10ms, so we won't hold global bind latch for too long
            // and it's only held at first time into the loop
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
            if ( SDB_ROLE_STANDALONE == dbrole ||
                 SDB_ROLE_DATA       == dbrole ||
                 SDB_ROLE_CATALOG    == dbrole ||
                 SDB_ROLE_AUTH       == dbrole )
            {
               rc = eduMgr->startEDU ( EDU_TYPE_AGENT, pData, &agentEDU ) ;
            }
            else if ( SDB_ROLE_COORD == dbrole )
            {
               rc = eduMgr->startEDU ( EDU_TYPE_COORDAGENT, pData, &agentEDU ) ;
            }
            else
            {
               PD_LOG ( PDERROR, "Invalid database role" ) ;
               rc = SDB_SYS ;
               continue ;
            }

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
         if ( SDB_OK != ( rc = eduMgr->waitEDU ( myEDUID )) )
         {
            goto error ;
         }
      } // while ( retry <= PMD_TCPLISTENER_RETRY )
   done :
      PD_TRACE_EXITRC ( SDB_PMDTCPLSTNENTPNT, rc );
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
