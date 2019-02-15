/*******************************************************************************


   Copyright (C) 2011-2019 SequoiaDB Ltd.

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

   Source File Name = dmsTransLockCallback.cpp

   Descriptive Name = Data Management Service Lock Callback Functions

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/02/2019  CYX Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdEDU.hpp"
#include "pdTrace.hpp"
#include "dmsTrace.hpp"
#include "dmsStorageDataCommon.hpp"
#include "dpsTransLockDef.hpp"
#include "dpsTransExecutor.hpp"
#include "dmsTransLockCallback.hpp"
#include "dpsTransVersionCtrl.hpp"

using namespace bson ;


namespace engine
{


   // Callback function constructor
   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSTRANSLOCKCALLBACK_DMSTRANSLOCKCALLBACK, "dmsTransLockCallback::dmsTransLockCallback" )
   dmsTransLockCallback::dmsTransLockCallback
   (
      dpsTransCB        * transCB,
      _pmdEDUCB         * eduCB,
      _dmsRecordRW      * recordRW
   )
   {
      PD_TRACE_ENTRY( SDB_DMSTRANSLOCKCALLBACK_DMSTRANSLOCKCALLBACK );
      // Caller should only create the transaction callback is transaction
      // is enabled
      SDB_ASSERT( transCB->isTransOn(), "Transaction must be enabled!");
      _transCB = transCB;
      _oldVer = NULL;
      _eduCB = eduCB;
      _isolationLevel = pmdGetOptionCB()->transIsolation();
      _recordRW = recordRW;
      _transExecutor = eduCB->getTransExecutor();
      _memTreeLatchMode = MEMTREE_LATCH_NONE;
      PD_TRACE_EXIT( SDB_DMSTRANSLOCKCALLBACK_DMSTRANSLOCKCALLBACK );
   }

   // Description:
   //    Function called after lock acquirement. There are two cases to handle:
   //
   // CASE 1: Under RC, TB scanner failed to get record lock in S mode due to
   // incompatibility, we'll try to use the saved old copy if it exist.
   // Note that normally tb scanner or idx scanner will wait on record lock
   // unless transaction isolation level is RC and translockwait is set to NO
   //
   // CASE 2: Update successfully acquire X record lock within a transaction,
   // we'll try to copy the data to lrbHrd if it's not already there
   // Note that we have decided to defer the copy to the actual update time,
   // because there are cases where X lock was acquired, but no update will
   // be made due to other critieras. However, we will do some preparation
   // work at this time.
   //
   // Input:
   //    lockId: lock id to operate on
   //    rc:  rc from the lock acquire
   //    requestLockMode: lock mode requested (IS/IX/S/U/X)
   //    opMode: lock operation mode (TRY/ACQUIRE/TEST)
   // Output:
   //    pdpsTxResInfo: return information about the lock
   // Dependency:
   //    caller must hold lrb bucket latch
   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE, "dmsTransLockCallback::afterLockAcquire" )
   void dmsTransLockCallback::afterLockAcquire
   (
      const dpsTransLockId             &lockId,
            INT32                      &rc,
      const DPS_TRANSLOCK_TYPE          requestLockMode,
      const DPS_TRANSLOCK_OP_MODE_TYPE  opMode,
            dpsTransRetInfo           * pdpsTxResInfo = NULL
   )
   {
      UINT32 recSize  = 0;
      PD_TRACE_ENTRY( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE );

      // Handle case 1 mentioned above
      if( (SDB_DPS_TRANS_LOCK_INCOMPATIBLE == rc) &&
          (DPS_TRANSLOCK_S == requestLockMode)     &&
          (lockId.isLeafLevel())                   &&
          (DPS_TRANSLOCK_OP_MODE_TRY == opMode) )
      {
         const dmsRecord   * r       = NULL;
         dmsRecord         * pbuf    = NULL;
         UTIL_OBJIDX         hdrIdx;
         dpsTransLRBHeader * lrbHdr = NULL;

         // 0. get the old version record from lrbHdr
         _transCB->getLockMgrHandle()-> getLRBHdrByLockId( lockId,
                                        hdrIdx,
                                        lrbHdr );

         // When the update was done by non transaction session, it's
         // possible that the oldRecord does not exist. We do nothing
         // in that case
         if ( lrbHdr != NULL )
         {
            // skip newly created record
            if ( lrbHdr->isNewRecord() )
            {
               if ( pdpsTxResInfo )
               {
                  pdpsTxResInfo->_skipNewRecord = TRUE;
               }
               goto done;
            }

            if ( (r = lrbHdr->getOldRecord()) != NULL )
            {
#ifdef _DEBUG
               PD_LOG( PDDEBUG,
                       "Use old copy for rid (%d, %d, %d, %d) from memory",
                       lockId.csID(), lockId.clID(),
                       lockId.extentID(), lockId.offset() );
#endif
               // 1. get the record size
               recSize = r->getSize();

               // 2. allocate a seperate buffer off transaction executor to
               // store the old version. We must do this because the old version
               // record hanging off lrbHeader could be freed up when the
               // update transaction commits. As soon as we get out of the
               // locking function, there is nothing prevent the transaction
               // commit.
               //pbuf = _transExecutor->getBuf();
               pbuf = (dmsRecord *)_transExecutor->getExecutor()->getBuffer( recSize );

               if ( pbuf != NULL )
               {
                  // 3. to copy record to buffer
                  ossMemcpy ( pbuf, r, recSize );

                  // 4. setup the buffer pointer in dmsRecordRW
                  // _ptr is private, but we are its friend now.
                  _recordRW->_ptr = (const dmsRecord*)pbuf;

                  // 5. set up the mask so that extractData can directly read the
                  //    ptr. see special handling in readPtr(0)
                  _recordRW->_rid._extent |= DMS_IDX_RID_MASK;
                  // 6. set the return info if we successfully used old version
                  if ( pdpsTxResInfo )
                  {
                     pdpsTxResInfo->_useOldVersion = TRUE;
                  }

                  // set up the current oldVer in callback for this round,
                  // note that it is cleared during setup for each round
                  _oldVer = lrbHdr->oldVer;
                  SDB_ASSERT( _oldVer, "oldVer does not exist!");
                  PD_TRACE1 ( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE,
                              PD_PACK_STRING("Will use old version") );

               } // else we can't get memory to hold old version,
                 // we simplely keep the RC and caller will wait on the lock
            }
            // else keep the original rc to return
         }
         goto done;
      } // end of case 1

      // Handle case 2 mentioned above
      // X record lock request from a transaction need to prepare to set
      // up old copy if the copy is not already there
      if ( ( SDB_OK == rc )                                 &&
           ( DPS_TRANSLOCK_X == requestLockMode)            &&
           ( lockId.isLeafLevel())                          &&
           ( (DPS_TRANSLOCK_OP_MODE_ACQUIRE == opMode ||
              DPS_TRANSLOCK_OP_MODE_TRY == opMode) ) )
      {
         UTIL_OBJIDX        hdrIdx;
         dpsTransLRBHeader *lrbHdr = NULL;

         // 1. get the old version record from lrbHdr
         _transCB->getLockMgrHandle()->
                       getLRBHdrByLockId( lockId, hdrIdx, lrbHdr );

         // this is after lock acquire and we got irc== OK, must have
         // the lock and must be able to find lrbHdr
         SDB_ASSERT ( ((lrbHdr != NULL) && (lrbHdr->oldVer != NULL)),
                      "Can't find the lock" );

         // If this is the first update to the record, the caller will do the
         // setup. Note  the old version might have already been setup, then
         // we don't need to do anything again.
         if ( lrbHdr->getOldRecord() == NULL )
         {
            _oldVer = lrbHdr->oldVer;
         }
#ifdef _DEBUG
         PD_LOG( PDDEBUG,
                 "Set oldVer(%d) for rid (%d, %d, %d, %d) in memory",
                 _oldVer,
                 lockId.csID(), lockId.clID(),
                 lockId.extentID(), lockId.offset());

         PD_TRACE1 ( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE,
                     PD_PACK_STRING("Will setup old version") );
#endif
         goto done;
      }

      done:
      PD_TRACE_EXIT( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE );
      return;
   }


   // Description:
   //    Function called before lock release(in dpsTransLockManager::_release)
   // Under RC, before X lock is released, the update is responsible
   // to free up the kept old version record and clean up all related indexes
   // from the in-memory index tree. All information were kept in lrbHdr->oldVer
   // The latching protocal has to be:
   // 1. LRB hash bkt latch must be held(X) to tranverse/update lrbHdrs/LRBs
   // 2. preIdxTree latch must be held in X to insert/delete node in the tree
   //    oldVersionCB(_oldVersionCBLatch) need to be held in S before
   //    accessing individual index tree.
   // 3. Request preIdxTree latch while holding LRB hash bkt latch is OK,
   //    But reverse order is forbidden.
   //
   // Input:
   //    lockId: lock id to operate on
   //    lockMode: lock mode requested (IS/IX/S/U/X)
   //    refCounter: current reference counter of the lock
   //    oldVer: pointer to oldVersionContainer
   // Output:
   //    oldVer: pointer to oldVersionContainer
   // Dependency:
   //    caller MUST hold the record lock and LRB hash bkt latch in X
   // which protects all the update on oldVer
   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE, "dmsTransLockCallback::beforeLockRelease" )
   void dmsTransLockCallback::beforeLockRelease
   (
      const dpsTransLockId     &lockId ,
      const DPS_TRANSLOCK_TYPE  lockMode,
      const UINT64              refCounter,
      oldVersionContainer      *oldVer
   )
   {
      PD_TRACE_ENTRY( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE );

      oldVersionCB      *oldVCB    = NULL;
      preIdxTree        *memTree   = NULL;
      globIdxID          gID;
      dmsRecordID        rid(lockId.extentID(), lockId.offset()) ;
      BSONObj           *idxobj    = NULL;
      INT32              numDeleted = 0;

      gID._csID = lockId.csID();
      gID._clID = lockId.clID();

      PD_TRACE4( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE,
                 PD_PACK_INT(gID._csID),
                 PD_PACK_INT(gID._clID),
                 PD_PACK_INT(rid._extent),
                 PD_PACK_INT(rid._offset) );

      oldVCB = _transCB->getOldVCB();

      // early exit if this is not record lock OR not in X mode OR
      // there is no old record setup.
      if ( ( !lockId.isLeafLevel() )               ||
           ( DPS_TRANSLOCK_X != lockMode )         ||
           ( 0 != refCounter )                     ||
           ( NULL == oldVer )                      ||
           ( NULL == oldVer->getOldRecord() ) )
      {
         // During insert, we will set record new without putting old record
         // Now when commit/rollback, we must clear the flag
         if( (0 == refCounter) && (DPS_TRANSLOCK_X == lockMode) && oldVer )
         {
            oldVer->unsetRecordNew();
         }
#ifdef _DEBUG
         PD_LOG( PDDEBUG, "skipping beforeLockRelease callback, lockmode=%d,"
                          "refCounter=%d, oldVer=%d, rid=(%d, %d)",
                          lockMode, refCounter, oldVer, rid._extent, rid._offset );
#endif
         goto done;
      }

      SDB_ASSERT ( oldVCB != NULL, "oldVCB can't be NULL" );
      // few common setups

      PD_LOG( PDDEBUG,
              "Trying to delete old copy for rid (%d, %d, %d, %d) from memory",
              gID._csID, gID._clID, rid._extent, rid._offset );

      // Now remove all indexes from the in memory index tree and free the
      // indexes. We can't simply do set clear because of the memory allocated
      for( idxObjSet::iterator it = oldVer->getIdxSet().begin();
           it != oldVer->getIdxSet().end(); )
      {
         gID._idxLID = it->_idxLID;

         idxobj = const_cast<BSONObj *>(&(it->_idxObj));
#ifdef _DEBUG
         PD_LOG( PDDEBUG,
                 "Trying to delete index (key=%s), id=(%d, %d, %d) "
                 "from in memory tree",
                 idxobj->toString().c_str(),
                 gID._csID, gID._clID, gID._idxLID );
#endif
         oldVCB->latchS();
         memTree = oldVCB->getIdxTree( gID );
         oldVCB->releaseS();

         // delete the entry from the tree, tree lock is taken in the func
         numDeleted = memTree->remove( idxobj, rid );


         PD_TRACE1( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE,
                    PD_PACK_INT(numDeleted) );

         SDB_ASSERT( ( 1 == numDeleted ), "deleted other than one keys" );

         // now free the index object from segment
         //oldVCB->getMemBlockPool()-> release( (CHAR **) &idxobj,
         //                                     it->_recordMemType );

         // free the entry from index set protected by lrbhashbkt latch
         oldVer->deleteIdx(it++);
      } // end of for

      // Now free index LID set
      oldVer->clearIdxLid();

      // delete order
      oldVer->freeOrder();

      // Now free the old record
      oldVCB->getMemBlockPool()->release( (CHAR * &) (oldVer->getOldRecord()),
                                          oldVer->getRecordMemType() );
      oldVer->setOldRecord( NULL );
      oldVer->unsetRecordNew();
      // Note that LRBHdr->oldVer is never freed up until freeing lrbHdr

      done:
      PD_TRACE_EXIT( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE );
      return;
   }

   // Description
   INT32 dmsTransLockCallback::saveOldVersionRecord( _dmsRecordRW * recordRW )
   {
      INT32  rc      = SDB_OK;
      UINT32 recSize = 0;
      // if the oldRecord does not exist, we will create one
      if ( _oldVer && _oldVer->getOldRecord()== NULL )
      {
            const dmsRecord *pRecord= recordRW->readPtr(0) ;
            char * buf = NULL;

            // 1. get to overflow record if needed
            if ( pRecord->isOvf() )
            {
               dmsRecordID ovfRID = pRecord->getOvfRID() ;
               //dmsRecordRW ovfRW = _recordRW->derive( ovfRID );
               dmsRecordRW ovfRW = recordRW->derive( ovfRID );
               //ovfRW.setNothrow( _recordRW->isNothrow() ) ;
               ovfRW.setNothrow( recordRW->isNothrow() ) ;
               pRecord = ovfRW.readPtr( 0 ) ;
            }

            // 2. allocate from segment
            recSize = pRecord->getSize();
            rc =  _transCB->getOldVCB()->getMemBlockPool()->
                      acquire(recSize, buf, &(_oldVer->getRecordMemType()));

            if ( SDB_OK == rc )
            {
               SDB_ASSERT( ( buf != NULL ), "allocation returned NULL" );
               // 3. copy from _recordRW->_ptr to the old record
               //ossMemcpy ( buf, _recordRW->readPtr(), recSize );
               ossMemcpy ( buf, pRecord, recSize );
               _oldVer->setOldRecord( (dmsRecord *)buf );

#ifdef _DEBUG
               PD_LOG ( PDDEBUG, "Saved old copy for rid(%d, %d) to oldVer(%d)",
                        recordRW->getRecordID()._extent,
                        recordRW->getRecordID()._offset,
                        _oldVer );
#endif
            }
            else
            {
               PD_LOG ( PDERROR, "Allocating buffer for old copy failed, rc=%d", rc );
            }
      }
      return rc;
   }



}
