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

   Source File Name = dpsTransLockCallback.hpp

   Descriptive Name = DPS Transaction Lock Callback Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          19/01/2019  CYX Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPS_TRANS_LOCK_CALLBACK_HPP__
#define DPS_TRANS_LOCK_CALLBACK_HPP__

#include "oss.hpp"
#include "dpsTransLockDef.hpp"
#include "dpsTransLockMgr.hpp"

namespace engine
{

   class dpsTransCB;
   class _pmdEDUCB;
   class _dmsRecordRW;
   class oldVersionContainer;

   enum transLockCallbackType
   {
      LOCK_CALLBACK_TYPE_DMS = 0,
      LOCK_CALLBACK_TYPE_MAX = 1
   };

   // pure virtual function as interface for callbacks
   class _dpsITransLockCallback : public SDBObject
   {
      public:
      _dpsITransLockCallback() {};
      _dpsITransLockCallback( dpsTransCB  * transCB,
                              _pmdEDUCB   * eduCB,
                              _dmsRecordRW * recordRW ) {};
      virtual ~_dpsITransLockCallback() {};

      virtual void beforeLockAcquire( const dpsTransLockId    &lockId,
                                      const INT32              irc,
                                      const DPS_TRANSLOCK_TYPE requestLockMode,
                                      const DPS_TRANSLOCK_OP_MODE_TYPE opMode
                                    ) = 0;
      virtual void afterLockAcquire( const dpsTransLockId     &lockId,
                                           INT32              &irc,
                                     const DPS_TRANSLOCK_TYPE  requestLockMode,
                                     const DPS_TRANSLOCK_OP_MODE_TYPE opMode,
                                           dpsTransRetInfo    *pdpsTxResInfo
                              ) = 0;
      // FIXME: might want to remove pLRBHdr
      virtual void beforeLockRelease( const dpsTransLockId &lockId,
                                      const DPS_TRANSLOCK_TYPE  lockMode,
                                      const UINT64              refCounter,
                                      oldVersionContainer      *oldVer ) = 0;

      virtual void afterLockRelease( const dpsTransLockId &lockId ) = 0;


      virtual void setRecordRW ( _dmsRecordRW * recordRW ) = 0;
      virtual CHAR * getWorkingArea () = 0;
      virtual void resetWorkingArea () = 0;
      virtual INT32 saveOldVersionRecord(_dmsRecordRW * recordRW) = 0;
      virtual oldVersionCB* getOldVCB() = 0;
   };

   // factory to create different type of instances for _dpsITransLockCallback
   class dpsTransLockCallbackFactory : public SDBObject
   {
      public:
      dpsTransLockCallbackFactory() {};
      ~dpsTransLockCallbackFactory() {};

      _dpsITransLockCallback *create( const INT32          type,
                                            _pmdEDUCB    * eduCB,
                                            _dmsRecordRW * recordRW );

   };
}

#endif // DPS_TRANS_LOCK_CALLBACK_HPP__

