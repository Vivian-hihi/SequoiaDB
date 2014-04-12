/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnPageCleanerJob.hpp

   Descriptive Name = Page cleaner header. Page cleaner is a type of background
                      job that flush storage unit in period of time.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/04/2014  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_PAGECLEANER_JOB_HPP_
#define RTN_PAGECLEANER_JOB_HPP_

#include "rtnBackgroundJob.hpp"
#include "../bson/bsonobj.h"

// wake up every 1000 milliseconds
#define RTNPAGECLEANER_WAKEUPTIME 1000

using namespace bson ;

namespace engine
{
   /*
    * _rtnPageCleanerJob define
    */
   class _rtnPageCleanerJob : public _rtnBaseJob
   {
   public :
      _rtnPageCleanerJob ( INT32 periodTime = RTNPAGECLEANER_WAKEUPTIME ) ;
      virtual ~_rtnPageCleanerJob () ;
   public :
      virtual RTN_JOB_TYPE type () const ;
      virtual const CHAR* name() const ;
      virtual BOOLEAN muteXOn ( const _rtnBaseJob *pOther ) ;
      virtual INT32 doit () ;
   private :
      INT32   _periodTime ;
   } ;
   typedef _rtnPageCleanerJob rtnPageCleanerJob ;

   INT32 startPageCleanerJob ( EDUID *pEDUID, INT32 periodTime ) ;
}

#endif
