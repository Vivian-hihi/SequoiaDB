/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsCleanupJob.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          20/03/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_CLEANUP_JOB_HPP_
#define CLS_CLEANUP_JOB_HPP_

#include "rtnBackgroundJob.hpp"

using namespace bson ;

namespace engine
{
   enum CLS_CLEANUP_TYPE
   {
      CLS_CLEANUP_BY_RANGE          = 0,
      CLS_CLEANUP_BY_CATAINFO,
      CLS_CLEANUP_BY_SHARDINGINDEX
   };

   class _clsCleanupJob : public _rtnBaseJob
   {
      public:
         _clsCleanupJob( const std::string &clFullName,
                         const BSONObj &splitKeyObj,
                         const BSONObj &splitEndKeyObj,
                         BOOLEAN hasShardingIndex,
                         BOOLEAN isHashSharding,
                         SDB_DPSCB *dpsCB ) ;
         virtual ~_clsCleanupJob() ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR* name () const ;
         virtual BOOLEAN muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32 doit () ;

      protected:
         INT32   _cleanByTBSCan ( INT32 w, CLS_CLEANUP_TYPE cleanType ) ;
         INT32   _cleanBySplitKeyObj ( INT32 w ) ;

         INT32   _filterDel ( const CHAR* buff, INT32 buffSize,
                              CLS_CLEANUP_TYPE cleanType,
                              UINT32 groupID ) ;

         CLS_CLEANUP_TYPE _cleanupType () const ;

         void  _makeName () ;

      protected:
         std::string          _clFullName ;
         BSONObj              _splitKeyObj ;
         BSONObj              _splitEndKeyObj ;
         BOOLEAN              _hasShardingIndex ;
         BOOLEAN              _isHashSharding ;

         std::string          _name ;

         SDB_DPSCB            *_dpsCB ;
         SDB_DMSCB            *_dmsCB ;

   };
   typedef class _clsCleanupJob clsCleanupJob ;

   INT32 startCleanupJob ( const std::string &clFullName,
                           const BSONObj &splitKeyObj,
                           const BSONObj &splitEndKeyObj,
                           BOOLEAN hasShardingIndex,
                           BOOLEAN isHashSharding,
                           SDB_DPSCB *dpsCB,
                           EDUID *pEDUID = NULL,
                           BOOLEAN returnResult = FALSE ) ;

}

#endif //CLS_CLEANUP_JOB_HPP_

