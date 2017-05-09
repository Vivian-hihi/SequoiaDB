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

   Source File Name = coordCB.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/28/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef COORDCB_HPP__
#define COORDCB_HPP__

#include "netRouteAgent.hpp"
#include "ossUtil.h"
#include "coordRemoteSession.hpp"
#include "coordMsgEventHandler.hpp"
#include "sdbInterface.hpp"

using namespace std ;

namespace engine
{
   /*
      _CoordCB define
   */
   class _CoordCB : public _IControlBlock
   {
      public:
         _CoordCB() ;
         virtual ~_CoordCB() ;

         virtual SDB_CB_TYPE cbType() const { return SDB_CB_COORD ; }
         virtual const CHAR* cbName() const { return "COORDCB" ; }

         virtual INT32  init () ;
         virtual INT32  active () ;
         virtual INT32  deactive () ;
         virtual INT32  fini () ;
         virtual void   onConfigChange() ;

         virtual void   attachCB( _pmdEDUCB *cb ) ;
         virtual void   detachCB( _pmdEDUCB *cb ) ;

         UINT32      setTimer( UINT32 milliSec ) ;
         void        killTimer( UINT32 timerID ) ;

         coordResource*    getResource() ;

      protected:
         virtual void      onTimer ( UINT64 timerID, UINT32 interval ) ;

      private:

         coordResource                       _resource ;
         pmdRemoteSessionMgr                 _remoteSessionMgr ;
         coordSessionPropMgr                 _sitePropMgr ;

         coordMsgHandler                     *_pMsgHandler ;
         coordTimerHandler                   *_pTimerHandler ;
         netRouteAgent                       *_pAgent ;

   } ;
   typedef _CoordCB CoordCB ;

   /*
      get global coord cb
   */
   CoordCB* sdbGetCoordCB() ;

}

#endif // COORDCB_HPP__

