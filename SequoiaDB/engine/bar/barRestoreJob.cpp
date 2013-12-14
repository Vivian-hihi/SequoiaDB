/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = barRestoreJob.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/11/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "barRestoreJob.hpp"

namespace engine
{
   
   /*
      _barRestoreJob implement
   */
   _barRestoreJob::_barRestoreJob( barRSBaseLogger * pRSLogger )
   {
      _rsLogger = pRSLogger ;
   }

   _barRestoreJob::~_barRestoreJob ()
   {
      _rsLogger = NULL ;
   }

   RTN_JOB_TYPE _barRestoreJob::type() const
   {
      return RTN_JOB_RESTORE ;
   }

   const CHAR *_barRestoreJob::name() const
   {
      return "Restore" ;
   }

   BOOLEAN _barRestoreJob::muteXOn( const _rtnBaseJob * pOther )
   {
      return FALSE ;
   }

   INT32 _barRestoreJob::doit ()
   {
      return _rsLogger->restore( eduCB() ) ;
   }

   INT32 startRestoreJob( EDUID * pEDUID, barRSBaseLogger * pRSLogger )
   {
      INT32 rc                = SDB_OK ;
      barRestoreJob * pJob    = NULL ;

      pJob = SDB_OSS_NEW barRestoreJob ( pRSLogger ) ;
      if ( !pJob )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Allocate failed" ) ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

}


