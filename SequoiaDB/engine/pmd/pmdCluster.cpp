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

#include "pmdEDUMgr.hpp"
#include "clsMgr.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCLSENTPNT, "pmdClusterEntryPoint" )
   INT32 pmdClusterEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDCLSENTPNT );

      clsCB *pClsCB = ( clsCB* )pData ;
      pmdEDUMgr *pEDUMgr = cb->getEDUMgr() ;
      pmdEDUEvent eventData;

      pClsCB->attachMainCB( cb ) ;

      rc = pEDUMgr->activateEDU( cb->getID() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU" ) ;
         goto error ;
      }

      //Wait event msg and dispatch msg
      while ( !cb->isDisconnected() )
      {
         if ( cb->waitEvent( eventData, OSS_ONE_SEC ) )
         {
            if ( PMD_EDU_EVENT_TERM == eventData._eventType )
            {
               PD_LOG ( PDDEBUG, "EDU[%lld, %s] is terminated", cb->getID(),
                        getEDUName( cb->getType() ) ) ;
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
      pClsCB->detachMainCB( cb ) ;
      PD_TRACE_EXITRC ( SDB_PMDCLSENTPNT, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 pmdClusterShardEntryPoint( pmdEDUCB * cb, void * pData )
   {
      INT32 rc = SDB_OK;

      clsCB *pClsCB = ( clsCB* )pData ;
      shardCB *pShdCB = pClsCB->getShardCB() ;
      pmdEDUMgr *pEDUMgr = cb->getEDUMgr() ;
      pmdEDUEvent eventData;

      pClsCB->attachMainCB( cb ) ;

      rc = pEDUMgr->activateEDU( cb->getID() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU" ) ;
         goto error ;
      }

      //Wait event msg and dispatch msg
      while ( !cb->isDisconnected() )
      {
         if ( cb->waitEvent( eventData, OSS_ONE_SEC ) )
         {
            if ( PMD_EDU_EVENT_TERM == eventData._eventType )
            {
               PD_LOG ( PDDEBUG, "EDU[%lld, %s] is terminated", cb->getID(),
                        getEDUName( cb->getType() ) ) ;
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
      pClsCB->detachMainCB( cb ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 pmdClsNtyEntryPoint( pmdEDUCB * cb, void * arg )
   {
      INT32 rc = SDB_OK ;
      clsLSNNtyInfo lsnInfo ;
      EDUID myEDUID = cb->getID () ;
      pmdEDUMgr * eduMgr = cb->getEDUMgr() ;
      replCB *pReplCb = ( replCB* )arg ;
      ossQueue< clsLSNNtyInfo > *pNtyQue = pReplCb->getNtyQue() ;

      rc = eduMgr->activateEDU ( myEDUID ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to activate EDU" ) ;
         goto error ;
      }

      // just sit here do nothing at the moment
      while ( !cb->isDisconnected() )
      {
         if ( !pNtyQue->timed_wait_and_pop( lsnInfo, OSS_ONE_SEC ) )
         {
            continue ;
         }
         cb->incEventCount() ;
         pReplCb->notify2Session( lsnInfo._csLID, lsnInfo._clLID,
                                  lsnInfo._extLID, lsnInfo._offset ) ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

}

