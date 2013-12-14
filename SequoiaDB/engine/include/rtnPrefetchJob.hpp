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
          26/09/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_PREFETCH_JOB_HPP_
#define RTN_PREFETCH_JOB_HPP_

#include "rtnBackgroundJob.hpp"

#include "../bson/bsonobj.h"

using namespace bson ;

namespace engine
{

   /*
      _rtnPrefetchJob define
   */
   class _rtnPrefetchJob : public _rtnBaseJob
   {
      public:
         _rtnPrefetchJob ( INT32 timeout = -1 ) ;
         virtual ~_rtnPrefetchJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR* name () const ;
         virtual BOOLEAN muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32 doit () ;

      private:
         INT32    _timeout ;

   } ;
   typedef _rtnPrefetchJob  rtnPrefetchJob ;

   INT32 startPrefetchJob ( EDUID *pEDUID, INT32 timeout = -1 ) ;

}

#endif //RTN_PREFETCH_JOB_HPP_

