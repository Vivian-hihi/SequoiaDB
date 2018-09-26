/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = clsUniqueIDCheckJob.cpp

   Descriptive Name = CS/CL UniqueID Checking Job

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who     Description
   ====== =========== ======= ==============================================
          06/08/2018  Ting YU Initial Draft

   Last Changed =

*******************************************************************************/
#include "clsUniqueIDCheckJob.hpp"
#include "pmd.hpp"
#include "clsMgr.hpp"
#include "monDMS.hpp"
#include "dmsStorageUnit.hpp"
#include "rtn.hpp"
#include "clsTrace.hpp"

namespace engine
{
   #define CLS_UNIQUEID_CHECK_INTERVAL ( OSS_ONE_SEC * 3 )

   /*
    *  _clsUniqueIDCheckJob implement
    */
   _clsUniqueIDCheckJob::_clsUniqueIDCheckJob()
   {
   }

   _clsUniqueIDCheckJob::~_clsUniqueIDCheckJob()
   {
   }

   BOOLEAN _clsUniqueIDCheckJob::muteXOn( const _rtnBaseJob *pOther )
   {
      if ( type() == pOther->type() )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSUIDCHKJOB_DOIT, "_clsUniqueIDCheckJob::doit" )
   INT32 _clsUniqueIDCheckJob::doit()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__CLSUIDCHKJOB_DOIT ) ;

      pmdEDUCB *cb = eduCB() ;
      pmdKRCB* pKrcb = pmdGetKRCB() ;
      SDB_DMSCB *pDmsCB = pKrcb->getDMSCB() ;
      SDB_DPSCB *pDpsCB = pKrcb->getDPSCB() ;
      shardCB* pShdMgr = sdbGetShardCB() ;
      clsDCMgr* pDcMgr = pShdMgr->getDCMgr() ;
      pmdEDUMgr* pEduMgr = pKrcb->getEDUMgr() ;
      UINT64 loopCnt = 0 ;
      BOOLEAN isCataReady = FALSE ;

      PD_LOG( PDDEBUG, "clsUniqueIDCheckJob: start job" ) ;

      while ( !PMD_IS_DB_DOWN() &&
              pmdIsPrimary() &&
              pDmsCB->nullCSUniqueIDCnt() > pDmsCB->localCSCnt() )
      {
         /*
          * Before any one is found in the queue, the status of this thread is
          * wait. Once found, it will be changed to running.
          */
         if ( loopCnt++ != 0 )
         {
            pEduMgr->waitEDU( cb ) ;
            pmdEDUEvent event ;
            cb->waitEvent( event, CLS_UNIQUEID_CHECK_INTERVAL ) ;
            pEduMgr->activateEDU( cb ) ;
         }

         // 1. check if the cs/cl unique id on catalog have been generated.
         if ( !isCataReady )
         {
            rc = pShdMgr->updateDCBaseInfo() ;
            if ( rc )
            {
               PD_LOG( PDERROR,
                       "Update data center base info failed, rc: %d", rc ) ;
               continue ;
            }

            clsDCBaseInfo* pDcInfo = pDcMgr->getDCBaseInfo() ;
            if ( UTIL_UNIQUEID_NULL == pDcInfo->getCSUniqueHWM() )
            {
               continue ;
            }
         }
         isCataReady = TRUE ;

         // 2. loop each cs
         MON_CS_LIST csList ;
         std::set<_monCollectionSpace>::const_iterator iterCS ;
         UINT32 localCSCnt = 0 ;

         pDmsCB->dumpInfo( csList, FALSE ) ;

         for ( iterCS = csList.begin() ; iterCS != csList.end(); ++iterCS )
         {
            const _monCollectionSpace &cs = *iterCS ;
            utilCSUniqueID csUniqueID = UTIL_UNIQUEID_NULL ;
            BSONObj clInfoObj ;

            PD_LOG( PDDEBUG,
                    "clsUniqueIDCheckJob: checking collection space [%s]",
                    cs._name ) ;

            if ( PMD_IS_DB_DOWN() || !pmdIsPrimary() )
            {
               break ;
            }

            // we only need to operate cs which unique id = 0
            if ( cs._csUniqueID != UTIL_UNIQUEID_NULL )
            {
               continue ;
            }

            // update catalog info
            rc = pShdMgr->rGetCSInfo( cs._name, csUniqueID,
                                      NULL, NULL, NULL, &clInfoObj ) ;

            if ( SDB_DMS_CS_NOTEXIST == rc )
            {
               localCSCnt++ ;
            }
            if ( rc )
            {
               PD_LOG( PDWARNING,
                       "Fail to update cs [%s] catalog info, rc: %d",
                       cs._name, rc ) ;
               continue ;
            }

            rc = rtnChangeUniqueID( cs._name, csUniqueID, clInfoObj,
                                    cb, pDmsCB, pDpsCB ) ;
            if ( rc )
            {
               PD_LOG( PDWARNING,
                       "Fail to add unique id, cs: %s, rc: %d",
                       cs._name, rc ) ;
               continue ;
            }

         }// end for

         pDmsCB->setLocalCSCnt( localCSCnt ) ;

      }// end while

      PD_TRACE_EXITRC( SDB__CLSUIDCHKJOB_DOIT, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_STARTUIDCHKJOB, "startUniqueIDCheckJob" )
   INT32 startUniqueIDCheckJob ( EDUID* pEDUID )
   {
      PD_TRACE_ENTRY( SDB_STARTUIDCHKJOB ) ;

      INT32 rc = SDB_OK ;
      clsUniqueIDCheckJob *pJob = NULL ;

      pJob = SDB_OSS_NEW clsUniqueIDCheckJob() ;
      if ( !pJob )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Allocate failed" ) ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID ) ;

   done:
      PD_TRACE_EXITRC( SDB_STARTUIDCHKJOB, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_STOPUIDCHKJOB, "stopUniqueIDCheckJob" )
   INT32 stopUniqueIDCheckJob ( EDUID& EDUID )
   {
      PD_TRACE_ENTRY( SDB_STOPUIDCHKJOB ) ;

      if ( PMD_INVALID_EDUID != EDUID )
      {
         pmdEDUMgr *eduMgr = pmdGetKRCB()->getEDUMgr() ;

         if ( !eduMgr->isDestroyed() )
         {
            eduMgr->forceUserEDU( EDUID ) ;
         }

         EDUID = PMD_INVALID_EDUID ;
      }

      PD_TRACE_EXIT( SDB_STOPUIDCHKJOB ) ;
      return SDB_OK ;
   }
}
