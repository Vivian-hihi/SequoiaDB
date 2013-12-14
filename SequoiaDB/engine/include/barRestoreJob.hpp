/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnPrefetchJob.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/11/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef BAR_RESTORE_JOB_HPP__
#define BAR_RESTORE_JOB_HPP__

#include "rtnBackgroundJob.hpp"
#include "barBkupLogger.hpp"

namespace engine
{

   /*
      _barRestoreJob define
   */
   class _barRestoreJob : public _rtnBaseJob
   {
      public:
         _barRestoreJob ( barRSBaseLogger *pRSLogger ) ;
         virtual ~_barRestoreJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR* name () const ;
         virtual BOOLEAN muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32 doit () ;

      private:
         barRSBaseLogger         *_rsLogger ;

   } ;
   typedef _barRestoreJob  barRestoreJob ;

   INT32 startRestoreJob ( EDUID *pEDUID, barRSBaseLogger *pRSLogger ) ;

}

#endif //BAR_RESTORE_JOB_HPP__

