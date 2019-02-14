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

   Source File Name = dmsTransLockCallback.hpp

   Descriptive Name = DMS Transaction Lock Callback Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/02/2019  CYX Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DMS_TRANS_LOCK_CALLBACK_HPP__
#define DMS_TRANS_LOCK_CALLBACK_HPP__

#include "dpsTransLockCallback.hpp"
#include "dpsTransCB.hpp"
#include "pmdEDU.hpp"

namespace engine
{

   class dpsTransCB;
   class _dmsRecordRW;
   class oldVersionContainer;
   class oldVersionCB;

   // current latch mode held on in memory index tree latch
   enum MEMTREE_LATCH_MODE
   {
      MEMTREE_LATCH_NONE   = 0,   // No latch held
      MEMTREE_LATCH_S,          
      MEMTREE_LATCH_X          
   };

   // Class to implment lock call back funtions for DMS scanner
   class dmsTransLockCallback : public _dpsITransLockCallback
   {
      public:
      //dmsTransLockCallback() {};
      dmsTransLockCallback( dpsTransCB  * transCB,
                            _pmdEDUCB   * eduCB,
                            _dmsRecordRW * recordRW ) ;

      virtual ~dmsTransLockCallback() {};

      virtual void beforeLockAcquire( const dpsTransLockId    &lockId,
                                      const INT32              irc,
                                      const DPS_TRANSLOCK_TYPE requestLockMode,
                                 const DPS_TRANSLOCK_OP_MODE_TYPE opMode
                                ) {};
      virtual void afterLockAcquire( const dpsTransLockId     &lockId,
                                           INT32              &irc,
                                     const DPS_TRANSLOCK_TYPE  requestLockMode,
                                     const DPS_TRANSLOCK_OP_MODE_TYPE opMode,
                                           dpsTransRetInfo *pdpsTxResInfo) ;

      virtual void beforeLockRelease( const dpsTransLockId     &lockId,
                                      const DPS_TRANSLOCK_TYPE  lockMode,
                                      const UINT64              refCounter,
                                      oldVersionContainer      *oldVer ) ;

      virtual void afterLockRelease( const dpsTransLockId &lockId ) {};

      void setRecordRW ( _dmsRecordRW * recordRW ) { _recordRW = recordRW; }

      void setMemTreeLatchMode ( const MEMTREE_LATCH_MODE m ) 
      {
         _memTreeLatchMode = m ; 
      }

      BOOLEAN memTreeLatchHeld() const
      {
         return ( MEMTREE_LATCH_NONE != _memTreeLatchMode) ;
      }

      CHAR * getWorkingArea () { return (CHAR *) _oldVer ; }
      void resetWorkingArea () { _oldVer = NULL ; }
      UINT32 isolationLevel() const { return _isolationLevel ; }
      BOOLEAN lockwaitLevel() const { return _lockwaitLevel ; }

      INT32 saveOldVersionRecord( _dmsRecordRW *recordRW );
      oldVersionCB * getOldVCB() { return _transCB->getOldVCB(); }

      // INT32 saveOldVersionIndexes( );

      private:
      dpsTransCB        * _transCB;   // use it to access global old copy tree
      pmdEDUCB          * _eduCB;
      UINT32              _isolationLevel;
      BOOLEAN             _lockwaitLevel;
      MEMTREE_LATCH_MODE  _memTreeLatchMode;
      _dpsTransExecutor * _transExecutor;  // use it to access thread local buf
      // DMS related information
      _dmsRecordRW       * _recordRW;
      // working area to be setup by callback function so the update can
      // put proper old copy into the area right before the update
      oldVersionContainer *_oldVer;
   };

}

#endif // DMS_TRANS_LOCK_CALLBACK_HPP__

