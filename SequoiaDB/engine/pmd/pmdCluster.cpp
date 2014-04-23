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

   Source File Name = pmdCluster.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          30/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCLSENTPNT, "pmdClusterEntryPoint" )
   INT32 pmdClusterEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_PMDCLSENTPNT );

      pmdKRCB *pKRCB = pmdGetKRCB() ;
      clsCB *pClsCB = pKRCB->getClsCB() ;
      pmdEDUMgr *pEDUMgr = cb->getEDUMgr() ;
      pmdEDUEvent eventData;

      rc = pEDUMgr->activateEDU( cb->getID() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU" ) ;
         goto error ;
      }

      pEDUMgr->regSystemEDU ( cb->getType(), cb->getID() ) ;

      rc = pClsCB->initialize ( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to initialize clsCB" ) ;
         goto error ;
      }
      rc = pClsCB->active () ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active clsCB" ) ;
         goto error ;
      }

      //Wait event msg and dispatch msg
      while ( !cb->isDisconnected() )
      {
         if ( cb->waitEvent( eventData, OSS_ONE_SEC ) )
         {
            if ( PMD_EDU_EVENT_TERM == eventData._eventType )
            {
               PD_LOG ( PDEVENT, "EDU[%lld] is terminated", cb->getID() ) ;
            }
            //Dispatch event msg to shardMgr
            else if ( PMD_EDU_EVENT_MSG == eventData._eventType )
            {
               //restore handle
               pClsCB->dispatchMsg( pClsCB->peekMsgHandle ( eventData._Data),
                                    (MsgHeader*)(eventData._Data) ) ;
            }
            else
            {
               pClsCB->dispatchEvent ( &eventData ) ;
            }

            //Relase memory
            if ( eventData._Data && eventData._release )
            {
               SDB_OSS_FREE ( eventData._Data ) ;
               eventData.reset () ;
            }
         }
      }

   done:
      pClsCB->final() ;
      PD_TRACE_EXITRC ( SDB_PMDCLSENTPNT, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 pmdClusterShardEntryPoint( pmdEDUCB * cb, void * pData )
   {
      INT32 rc = SDB_OK;

      clsCB *pClsCB = pmdGetKRCB()->getClsCB() ;
      shardCB *pShdCB = pClsCB->getShardCB() ;
      pmdEDUMgr *pEDUMgr = cb->getEDUMgr() ;
      pmdEDUEvent eventData;

      rc = pEDUMgr->activateEDU( cb->getID() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU" ) ;
         goto error ;
      }

      pEDUMgr->regSystemEDU ( cb->getType(), cb->getID() ) ;

      // attach edu
      pClsCB->getShardMsgHandle()->attachShardCB( cb ) ;

      //Wait event msg and dispatch msg
      while ( !cb->isDisconnected() )
      {
         if ( cb->waitEvent( eventData, OSS_ONE_SEC ) )
         {
            if ( PMD_EDU_EVENT_TERM == eventData._eventType )
            {
               PD_LOG ( PDEVENT, "EDU[%lld] is terminated", cb->getID() ) ;
            }
            //Dispatch event msg to shardMgr
            else if ( PMD_EDU_EVENT_MSG == eventData._eventType )
            {
               //restore handle
               pShdCB->dispatchMsg( pClsCB->peekMsgHandle ( eventData._Data),
                                    (MsgHeader*)(eventData._Data) ) ;
            }
            else
            {
               pShdCB->dispatchEvent ( &eventData ) ;
            }

            //Relase memory
            if ( eventData._Data && eventData._release )
            {
               SDB_OSS_FREE ( eventData._Data ) ;
               eventData.reset () ;
            }
         }
      }

   done:
      pClsCB->getShardMsgHandle()->detachShardCB() ;
      return rc ;
   error:
      goto done ;
   }

}

