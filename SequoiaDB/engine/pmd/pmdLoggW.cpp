/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdLoggW.cpp

   Descriptive Name = Process MoDel Log Global Writer

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains entry point for log global
   writer thread.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include <stdio.h>
#include "pd.hpp"
#include "pmd.hpp"
#include "pmdEDUMgr.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOGGWENTPNT, "pmdLoggWEntryPoint" )
   INT32 pmdLoggWEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDLOGGWENTPNT );
      EDUID myEDUID = cb->getID () ;
      pmdEDUMgr * eduMgr = cb->getEDUMgr() ;
      SDB_DPSCB *dpsCb = pmdGetKRCB()->getDPSCB();
      rc = eduMgr->activateEDU ( myEDUID ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to activate EDU" ) ;
         goto error ;
      }

      // just sit here do nothing at the moment
      while ( !cb->isDisconnected() )
      {
         rc = dpsCb->run( cb );
         if ( rc )
         {
            PD_LOG ( PDSEVERE, "Failed to run dpsCB, rc = %d", rc ) ;
            ossPanic () ;
         }
      }

      rc = dpsCb->tearDown();
      if ( rc )
      {
         PD_LOG ( PDSEVERE, "Failed to run tearDown(), rc = %d", rc ) ;
         ossPanic () ;
      }

   done :
      PD_TRACE_EXITRC ( SDB_PMDLOGGWENTPNT, rc );
      return rc;
   error :
      switch ( rc )
      {
      case SDB_SYS :
         PD_LOG ( PDSEVERE, "System error occured" ) ;
         break ;
      default :
         PD_LOG ( PDSEVERE, "Internal error, rc = %d", rc ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDLOGGWNTY, "pmdLoggNtyEntryPoint" )
   INT32 pmdLoggNtyEntryPoint( pmdEDUCB * cb, void * arg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDLOGGWNTY );
      dpsLSNInfoEx lsnInfo ;
      EDUID myEDUID = cb->getID () ;
      pmdEDUMgr * eduMgr = cb->getEDUMgr() ;
      SDB_DPSCB *dpsCb = pmdGetKRCB()->getDPSCB() ;
      ossQueue< dpsLSNInfoEx > *pNtyQue = dpsCb->getLogMgr()->getNtyQue() ;
      replCB *replCB = pmdGetKRCB()->getReplCB() ;
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
         replCB->notify2Session( lsnInfo._clLID, lsnInfo._clLID,
                                 lsnInfo._extLID, lsnInfo._offset ) ;
      }

   done :
      PD_TRACE_EXITRC ( SDB_PMDLOGGWNTY, rc );
      return rc;
   error :
      goto done ;
   }

}

