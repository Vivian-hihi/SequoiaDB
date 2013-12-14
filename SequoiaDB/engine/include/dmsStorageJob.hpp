/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dmsStorageJob.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/10/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DMS_STORAGE_JOB_HPP__
#define DMS_STORAGE_JOB_HPP__

#include "rtnBackgroundJob.hpp"

namespace engine
{

   class _dmsStorageBase ;

   /*
      _dmsExtendSegmentJob define
   */
   class _dmsExtendSegmentJob : public _rtnBaseJob
   {
      public:
         _dmsExtendSegmentJob ( _dmsStorageBase *pSUBase ) ;
         virtual ~_dmsExtendSegmentJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR* name () const ;
         virtual BOOLEAN muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32 doit () ;

      private:
         _dmsStorageBase            *_pSUBase ;

   } ;
   typedef _dmsExtendSegmentJob  dmsExtendSegmentJob ;

   INT32 startExtendSegmentJob ( EDUID *pEDUID, _dmsStorageBase *pSUBase ) ;

}

#endif //DMS_STORAGE_JOB_HPP__

