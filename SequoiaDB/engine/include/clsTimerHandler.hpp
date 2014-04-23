/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

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
