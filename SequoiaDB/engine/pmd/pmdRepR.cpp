/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdRepR.cpp

   Descriptive Name = Process MoDel Replicate Receiver

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains entry point for replicate
   receiver thread.

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
#include "pmdCB.hpp"
#include "ossASIO.hpp"
#include "ossSocket.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDREQRENTPNT, "pmdRepREntryPoint" )
   INT32 pmdRepREntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_PMDREQRENTPNT );
      pmdEDUMgr *pEDUMgr = cb->getEDUMgr () ;
      _clsMgr *pClsMgr = (_clsMgr *)pData;

      pClsMgr->attachIn() ;

      rc = cb->getEDUMgr()->activateEDU( cb->getID() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to active EDU[type:%d,ID:%lld]", cb->getType() ,
                  cb->getID() ) ;
         goto error ;
      }

      //start run
      PD_LOG ( PDEVENT, "Run replicate group listen..." ) ;
      try
      {
         pEDUMgr->addIOService( pClsMgr->getReplRouteAgent()->ioservice() ) ;
         pClsMgr->getReplRouteAgent()->run() ;
         pEDUMgr->deleteIOService( pClsMgr->getReplRouteAgent()->ioservice() ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception during start replR session: %s",
                  e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "Stop replicate group listen" ) ;

   done:
      pClsMgr->attachOut() ;
      PD_TRACE_EXITRC ( SDB_PMDREQRENTPNT, rc );
      return rc;
   error:
      goto done;
   }
}

