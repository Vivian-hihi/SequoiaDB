/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsTimerHandler.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          1/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_TIMER_HANDLER_HPP_
#define CLS_TIMER_HANDLER_HPP_

#include "netTimer.hpp"
#include "pmdEDU.hpp"

namespace engine
{
   class _clsMgr;

   class _clsTimerHandler : public _netTimeoutHandler
   {
      public:
         _clsTimerHandler ( _clsMgr * pClsMgr ) ;
         virtual ~_clsTimerHandler () ;

         virtual INT32 type () const = 0 ;

         virtual void handleTimeout( const UINT32 &millisec,
                                     const UINT32 &id ) ;

      public:
         OSS_INLINE void attach ( pmdEDUCB *cb ) { _pMgrCB = cb ; }
         OSS_INLINE void detach () { _pMgrCB = NULL ; }

      protected:
         pmdEDUCB             *_pMgrCB ;
         _clsMgr              *_pClsMgr ;

   };

   class _clsReplTimerHandler : public _clsTimerHandler
   {
      public:
         _clsReplTimerHandler ( _clsMgr * pClsMgr ) ;
         virtual ~_clsReplTimerHandler () ;

         virtual INT32 type () const ;

      protected:

   };

   class _clsShardTimerHandler : public _clsTimerHandler
   {
      public:
         _clsShardTimerHandler ( _clsMgr * pClsMgr ) ;
         virtual ~_clsShardTimerHandler () ;

         virtual INT32 type () const ;

      protected:
   };

}

#endif //CLS_TIMER_HANDLER_HPP_
