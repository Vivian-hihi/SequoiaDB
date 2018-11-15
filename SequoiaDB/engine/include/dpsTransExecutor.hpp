/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = dpsTransExecutor.hpp

   Descriptive Name = Operating System Services Types Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/08/2018  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPS_TRANS_EXECUTOR_HPP__
#define DPS_TRANS_EXECUTOR_HPP__

#include "dpsTransLockDef.hpp"
#include "utilSegment.hpp"
#include "utilMap.hpp"

using namespace bson ;
using namespace std ;

namespace engine
{

   /*
      DPS_TRANS_QUE_TYPE define
   */
   enum DPS_TRANS_QUE_TYPE
   {
      DPS_QUE_NULL         = 0,
      DPS_QUE_UPGRADE,
      DPS_QUE_WAITER
   } ;

   /*
      _dpsTransExecutor define
   */
   class _dpsTransExecutor
   {
      typedef _utilMap<dpsTransLockId,UTIL_OBJIDX,20>    DPS_LOCKID_MAP ;
      typedef DPS_LOCKID_MAP::iterator                   DPS_LOCKID_MAP_IT ;
      typedef DPS_LOCKID_MAP::const_iterator             DPS_LOCKID_MAP_CIT ;

      public:
         _dpsTransExecutor() ;
         virtual ~_dpsTransExecutor() ;

         void     clearAll() ;

      public:

         void                 setWaiterInfo( UTIL_OBJIDX lrbIdx,
                                             DPS_TRANS_QUE_TYPE type ) ;
         void                 clearWaiterInfo() ;

         UTIL_OBJIDX          getWaiterLRBIdx() const ;
         DPS_TRANS_QUE_TYPE   getWaiterQueType() const ;

         void                 setLastLRBIdx( UTIL_OBJIDX lrbIdx ) ;
         void                 clearLastLRBIdx() ;
         UTIL_OBJIDX          getLastLRBIdx() const ;

         BOOLEAN              addLock( const dpsTransLockId &lockID,
                                       UTIL_OBJIDX lrbIdx ) ;
         BOOLEAN              findLock( const dpsTransLockId &lockID,
                                        UTIL_OBJIDX &lrbIdx ) const ;
         BOOLEAN              removeLock( const dpsTransLockId &lockID ) ;
         void                 clearLock() ;

         void                 incLockCount() ;
         void                 decLockCount() ;
         void                 clearLockCount() ;
         UINT32               getLockCount() const ;

         INT64                getTransTimeout() const ;
         void                 setTransTimeout( INT64 timeout ) ;

      public:
         /*
            Interface
         */
         virtual EDUID        getEDUID() const = 0 ;
         virtual void         wakeup() = 0 ;
         virtual INT32        wait( INT64 timeout ) = 0 ;

      protected:
         UTIL_OBJIDX             _waiterIdx ;
         DPS_TRANS_QUE_TYPE      _waiterQueType ;
         UTIL_OBJIDX             _lastLRBIdx ;

         DPS_LOCKID_MAP          _mapLockID ;
         UINT32                  _lockCount ;
         INT64                   _transTimeout ;   /// milli-seconds

   } ;
   typedef _dpsTransExecutor dpsTransExecutor ;

}

#endif // DPS_TRANS_EXECUTOR_HPP__

