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

   Source File Name = coordInterruptOperator.cpp

   Descriptive Name = Coord Interrupt

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   interrupt processing on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "coordInterruptOperator.hpp"
#include "pmd.hpp"
#include "rtnCB.hpp"
#include "msgMessageFormat.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

namespace engine
{

   /*
      _coordInterrupt implement
   */
   _coordInterrupt::_coordInterrupt()
   {
      const static string s_name( "Interrupt" ) ;
      setName( s_name ) ;
   }

   _coordInterrupt::~_coordInterrupt()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_INTERRUPT_EXE, "_coordInterrupt::execute" )
   INT32 _coordInterrupt::execute( MsgHeader *pMsg,
                                   pmdEDUCB *cb,
                                   INT64 &contextID,
                                   rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_INTERRUPT_EXE ) ;
      SDB_RTNCB *pRtncb = pmdGetKRCB()->getRTNCB() ;
      pmdRemoteSessionSite *pSite = NULL ;
      pmdRemoteSession *pSession = NULL ;
      coordNoSessionInitHandler interruptHandler ;
      INT64 tmpContextID = -1 ;

      contextID = -1 ;


      /// send interrupt to all nodes associate with the session,
      /// and kill all context
      if ( cb->getRemoteSite() )
      {
         SET_ROUTEID routeSet ;
         pSite = (pmdRemoteSessionSite*)cb->getRemoteSite() ;
         pSite->getAllNodeID( routeSet ) ;

         pSession = pSite->addSession( getTimeout(), &interruptHandler ) ;
         if ( !pSession )
         {
            PD_LOG( PDERROR, "Create session[%s] failed", cb->getName() ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         _sendInterrupt( pSession, cb, routeSet ) ;
      }

      // delete all opened contexts when received the interrupt message
      while ( -1 != ( tmpContextID = cb->contextPeek() ) )
      {
         pRtncb->contextDelete( tmpContextID, NULL ) ;
      }

   done:
      if ( pSession )
      {
         pSite->removeSession( pSession->sessionID() ) ;
      }
      PD_TRACE_EXITRC ( COORD_INTERRUPT_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_INTERRUPT_SENDINTERRUPT, "_coordInterrupt::_sendInterrupt" )
   void _coordInterrupt::_sendInterrupt( pmdRemoteSession *pSession,
                                         pmdEDUCB *cb,
                                         SET_ROUTEID &routeMap )
   {
      PD_TRACE_ENTRY ( COORD_INTERRUPT_SENDINTERRUPT ) ;

      INT32 rc = SDB_OK ;
      pmdSubSession *pSub           = NULL ;
      SET_ROUTEID::iterator it ;

      MsgHeader interruptMsg ;
      interruptMsg.messageLength = sizeof( MsgHeader ) ;
      interruptMsg.opCode = MSG_BS_INTERRUPTE ;

      /// send msg
      it = routeMap.begin() ;
      while( it != routeMap.end() )
      {
         pSub = pSession->addSubSession( *it ) ;
         pSub->setReqMsg( &interruptMsg, PMD_EDU_MEM_NONE ) ;

         rc = pSession->sendMsg( pSub ) ;
         ++it ;

         if ( rc )
         {
            PD_LOG( PDWARNING, "Send interrupt message to node[%s] failed, "
                    "rc: %d", routeID2String( *it ).c_str(), rc ) ;
         }
      }

      PD_TRACE_EXIT ( COORD_INTERRUPT_SENDINTERRUPT ) ;
   }

}

