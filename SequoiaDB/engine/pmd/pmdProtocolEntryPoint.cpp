#include <stdio.h>
#include "pd.hpp"
#include "ossMem.hpp"
#include "pmd.hpp"
#include "pmdEDUMgr.hpp"
#include "ossSocket.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "pmdProcessor.hpp"
#include "pmdAccessProtocolBase.hpp"

namespace engine {

   INT32 pmdFapListenerEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDTCPLSTNENTPNT ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      monDBCB *mondbcb = krcb->getMonDBCB () ;
      pmdEDUMgr * eduMgr = cb->getEDUMgr() ;
      EDUID agentEDU = PMD_INVALID_EDUID ;
      ossSocket *pListerner = ( ossSocket* )pData ;

      // let's set the state of EDU to RUNNING
      if ( SDB_OK != ( rc = eduMgr->activateEDU ( cb ) ) )
      {
         goto error ;
      }

      // master loop for tcp listener
      while ( ! cb->isDisconnected() )
      {
         SOCKET s ;
         // timeout in 10ms, so we won't hold global bind latch for too long
         // and it's only held at first time into the loop
         rc = pListerner->accept ( &s, NULL, NULL ) ;
         // if we don't get anything for a period of time, let's loop
         if ( SDB_TIMEOUT == rc || SDB_TOO_MANY_OPEN_FD == rc )
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
            PD_LOG ( PDERROR, "Failed to accept socket in TcpListener(rc=%d)",
               rc ) ;
            if ( pListerner->isClosed() )
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

         if ( !krcb->isActive() )
         {
            ossSocket newsock ( &s ) ;
            newsock.close () ;
            continue ;
         }

         // now we have a tcp socket for a new connection, let's get an 
         // agent, Note the new new socket sent passing to startEDU
         rc = eduMgr->startEDU ( EDU_TYPE_FAPAGENT, pData, &agentEDU ) ;
         if ( rc )
         {
            PD_LOG( ( rc == SDB_QUIESCED ? PDWARNING : PDERROR ),
               "Failed to start edu, rc: %d", rc ) ;

            // close remote connection if we can't create new thread
            ossSocket newsock ( &s ) ;
            newsock.close () ;
            continue ;
         }
         // Now EDU is started and posted with the new socket, let's
         // get back to wait for another request
      } //while ( ! cb->isDisconnected() )

      if ( SDB_OK != ( rc = eduMgr->waitEDU ( cb ) ) )
      {
         goto error ;
      }

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
      goto done ;
   }

   INT32 pmdFapAgentEntryPoint( pmdEDUCB *cb, void *arg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDLOCALAGENTENTPNT );
      //TODO: make sure the destructor execute after 'localSession.detach() ;'
      pmdSession *session = NULL ;
      SOCKET s = *(( SOCKET *) &arg ) ;
      IPmdAccessProtocol* protocol = NULL ;

      if ( pmdGetDBRole() == SDB_ROLE_COORD )
      {
         pmdCoordProcessor coordProcessor ;
         session = protocol->getSession( s, &coordProcessor ) ;
         if ( NULL == session )
         {
            PD_LOG( PDERROR, "Failed to get Session of protocol" ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         session->attach( cb ) ;
         coordProcessor.attachSession( session ) ;
         rc = session->run() ;
         coordProcessor.detachSession() ;
         session->detach() ;
         protocol->releaseSession( session ) ;
      }
      else
      {
         pmdDataProcessor dataProcessor ;
         session = protocol->getSession( s, &dataProcessor ) ;
         if ( NULL == session )
         {
            PD_LOG( PDERROR, "Failed to get Session of protocol" ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         session->attach( cb ) ;
         dataProcessor.attachSession( session ) ;
         rc = session->run() ;
         dataProcessor.detachSession() ;
         session->detach() ;
         protocol->releaseSession( session ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_PMDLOCALAGENTENTPNT, rc );
      return rc ;
   error:
      goto done ;
   }

}
