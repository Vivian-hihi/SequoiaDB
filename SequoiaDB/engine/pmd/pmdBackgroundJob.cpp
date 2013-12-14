/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdReplay.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          30/11/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pd.hpp"
#include "rtnBackgroundJob.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_PMDBGJOBENTPNT, "pmdBackgroundJobEntryPoint" )
   INT32 pmdBackgroundJobEntryPoint( pmdEDUCB *cb, void *pData )
   {
      SDB_ASSERT( NULL != pData, "impossible" )
      PD_TRACE_ENTRY ( SDB_PMDBGJOBENTPNT );
      rtnJobMgr *jobMgr = rtnGetJobMgr () ;
      pmdEDUMgr *pEDUMgr = pmdGetKRCB()->getEDUMgr() ;
      rtnBaseJob *job = (rtnBaseJob*)pData ;
      INT32 rc = SDB_OK ;

      PD_LOG( PDINFO, "Start a background job[%s]", job->name() ) ;

      cb->setName( job->name() ) ;
      job->attachIn( cb ) ;

      pEDUMgr->activateEDU( cb->getID() ) ;
      rc = job->doit () ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDWARNING, "Background job[%s] do failed, rc = %d",
                  job->name(), rc ) ;
      }
      else
      {
         PD_LOG ( PDINFO, "Background job[%s] finished", job->name() ) ;
      }

      job->attachOut () ;

      // remove from job mgr
      jobMgr->_removeJob ( cb->getID() ) ;

      PD_TRACE_EXITRC ( SDB_PMDBGJOBENTPNT, rc );
      return SDB_OK ;
   }
}

