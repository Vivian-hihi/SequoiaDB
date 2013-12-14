/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossEvent.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OSS_EVENT_HPP_
#define OSS_EVENT_HPP_

#include "core.hpp"
#include "oss.hpp"
#include <boost/thread.hpp>
#include <boost/thread/thread_time.hpp>

namespace engine
{

   class _ossEvent : public SDBObject
   {
      public:
         _ossEvent () ;
         virtual ~_ossEvent () ;

      public:
         INT32 wait ( INT64 millisec = -1, INT32 *pData = NULL ) ;
         INT32 signal ( INT32 data = 0 ) ;
         INT32 signalAll ( INT32 data = 0 ) ;
         INT32 reset () ;
         UINT32 waitNum () ;

      protected:
         virtual void _onWait () ;

      protected:
         mutable boost::mutex       _mutex ;
         boost::condition_variable  _cond ;
         UINT32                     _signal ;
         UINT32                     _waitNum ;
         INT32                      _useData ;

   };

   typedef _ossEvent ossEvent ;

   class _ossAutoEvent : public _ossEvent
   {
      public:
         _ossAutoEvent () ;
         virtual ~_ossAutoEvent () ;

      protected:
         virtual void _onWait () ;

   };

   typedef _ossAutoEvent ossAutoEvent ;

}

#endif //OSS_EVENT_HPP_
