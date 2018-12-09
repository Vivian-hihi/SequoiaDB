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

   Source File Name = dpsTransLockMgr.hpp

   Descriptive Name = DPS Lock manager header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/28/2018  JT  Initial Draft, locking performance improvement

   Last Changed =

*******************************************************************************/
#ifndef DPSTRANSLOCKMANAGER_HPP_
#define DPSTRANSLOCKMANAGER_HPP_

#include "utilSegment.hpp"
#include "dpsTransLRB.hpp"
#include "dpsTransLockDef.hpp"
#include "monDps.hpp"
#include "ossAtomic.hpp"
#include "ossRWMutex.hpp"

namespace engine
{
   class _dpsTransExecutor ;
   class _IContext ;

   // lock bucket, 512K slots for testing
   #define MAX_LOCKBUCKET_NUM             ( (UTIL_OBJIDX)( 524288  ) )
   // LRB 256K for testing
   #define DPS_INIT_NUM_OF_LRB            ( (UTIL_OBJIDX)( 262144  ) )
   // LRB MAX 2M 
   #define DPS_INIT_NUM_OF_LRB_MAX        ( (UTIL_OBJIDX)( 2097152 ) )
   // LRB Header 256K for testing
   #define DPS_INIT_NUM_OF_LRB_HEADER     ( (UTIL_OBJIDX)( 262144  ) )
   // LRB Header MAX 2M
   #define DPS_INIT_NUM_OF_LRB_HEADER_MAX ( (UTIL_OBJIDX)( 2097152 ) )
   // lock timeout, 5 seconds by default
   #define DPS_LOCK_TIMEOUT_DEFAULT       ( (UINT32) ( 5 * OSS_ONE_SEC ) )

   enum DPS_TRANSLOCK_OP_MODE_TYPE
   {
      DPS_TRANSLOCK_OP_MODE_TRY = 0,
      DPS_TRANSLOCK_OP_MODE_ACQUIRE,
      DPS_TRANSLOCK_OP_MODE_TEST
   } ;

   class dpsTransLockManager : public SDBObject
   {
   public:
      dpsTransLockManager();

      virtual ~dpsTransLockManager();

      // initialization,
      //   . init _LockHdrBkt
      //   . allocate segment for LRB and LRB Header
      INT32 init() ;

      // free allocated LRB and LRB Header segments
      void fini() ;

      // if a lock has any waiters ( upgrade and waiter list )
      BOOLEAN hasWait( const dpsTransLockId &lockId );

      // if lock manager has been initialized
      BOOLEAN isInitialized() { return _initialized ; } ; 

      // acquire a lock with given mode, the higher level lock ( CL, CS )
      // will be also acquired respectively
      // If a lock can't be acquired right away, the request will be added
      // to waiter/upgrade queue and wait for the lock till it be waken up,
      // lock waiting timeout elapsed, or be interrupted. When it is woken up,
      // it will try to acquire the lock again.
      INT32 acquire
      (
         _dpsTransExecutor        * dpsTxExectr,
         const dpsTransLockId     & lockId,
         const DPS_TRANSLOCK_TYPE   requestLockMode,
         _IContext                * pContext      = NULL,
         dpsTransRetInfo          * pdpsTxResInfo = NULL
      ) ;

      // release a lock. The higher level intent lock will be also released
      // respectively. It will also wake up the first one in upgrade or
      // waiter queue if it is necessary.
      void release
      (
         _dpsTransExecutor    * dpsTxExectr,
         const dpsTransLockId & lockId,
         const BOOLEAN          bForceRelease = FALSE
      ) ;

      // release all locks an executor ( EDU ) holding. The executor ( EDU )
      // walks through its EDU LRB chain ( all locks acquired within a tx ),
      // and release all locks its holding and wake up the first one in waiter
      // or upgrade queue if it is necessary.
      void releaseAll( _dpsTransExecutor * dpsTxExectr ) ;

      // try to acquire a lock with given mode and will try to acquire higher
      // level intent lock respectively.
      // If a lock can not be acquired right away, it will NOT put itself in
      // wait/upgrade queue, simply return error SDB_DPS_TRANS_LOCK_INCOMPATIBLE
      INT32 tryAcquire
      (
         _dpsTransExecutor        * dpsTxExectr,
         const dpsTransLockId     & lockId,
         const DPS_TRANSLOCK_TYPE   requestLockMode,
         dpsTransRetInfo          * pdpsTxResInfo = NULL
      ) ;

      // test if a lock with give lock mode can be acquired, higher level intent
      // lock will also be tested.  It will not acquire the lock nor wait
      // the lock.
      // If the test fails, error code SDB_DPS_TRANS_LOCK_INCOMPATIBLE will be
      // returned.
      INT32 testAcquire
      (
         _dpsTransExecutor        * dpsTxExectr,
         const dpsTransLockId     & lockId,
         const DPS_TRANSLOCK_TYPE   requestLockMode,
         dpsTransRetInfo          * pdpsTxResInfo = NULL
      ) ;

      void setLockTimeout( UINT32 timeout ) { _lockTimeout.swap( timeout ) ; }

      UINT32 getLockTimeout() { return _lockTimeout.fetch() ; }

      // Latching for monitoring / dumping locking info of an EDU.
      //   . latch _rwMutext in exclusive mode
      OSS_INLINE void acquireMonLatch() { _rwMutex.lock_w() ; }

      // release monitoring / dumping latch
      OSS_INLINE void releaseMonLatch() { _rwMutex.release_w() ; }

      // dump specific lock info to a file for debugging purpose
      void dumpLockInfo
      (
         const dpsTransLockId & lockId,
         const CHAR           * fileName,
         BOOLEAN                bOutputInPlainMode = FALSE
      ) ;

      // dump all holding locks of an executor( EDU ) to a file for debugging.
      // The caller shall acquire the monitoring( dump ) latch before
      // calling this function and make sure the dpsTxExectr is still valid
      void dumpLockInfo
      (
         _dpsTransExecutor * dpsTxExectr,
         const CHAR        * fileName,
         BOOLEAN             bOutputInPlainMode = FALSE
      ) ;

      // dump all holding locks of an executor ( EDU )
      // the caller shall acquire the monitoring( dump ) latch before
      // calling this function and make sure the dpsTxExectr is still valid
      void dumpLockInfo
      (
         UTIL_OBJIDX       lastLRBIdx,
         VEC_TRANSLOCKCUR  & vecLocks
      ) ;
     
      // dump the waiting lock info of an executor ( EDU ) 
      // the caller shall acquire the monitoring( dump ) latch before
      // calling this function and make sure the dpsTxExectr is still valid
      void dumpLockInfo
      (
         UTIL_OBJIDX       lrbIdx,
         monTransLockCur  &lockCur
      ) ;

      // dump a specific lock info ( waiter, owners, upgrade list etc )
      void dumpLockInfo
      (
         const dpsTransLockId & lockId,
         monTransLockInfo     & monLockInfo
      ) ;

      // get LRB Header ( index and its pointer ) of a given lock from
      // LRB Header bucket. Wrapper of _getLRBHdrByLockId
      BOOLEAN getLRBHdrByLockId
      (
         const dpsTransLockId & lockId,
         UTIL_OBJIDX          & hdrIdx,
         dpsTransLRBHeader *  & pLRBHdr
      ) ;
     
      // get LRB Header handle ( address/pointer ) by LRB Header index 
      dpsTransLRBHeader * getLRBHdrPtrByIdx( const UTIL_OBJIDX hdrIdx ) ;

      // acquire lock bucket latch, wrapper of _acquireOpLatch()
      OSS_INLINE void acquireLockBktLatch( const dpsTransLockId & lockId )
      {
         const UTIL_OBJIDX bktIdx = _getBucketNo( lockId ) ;
         _acquireOpLatch( bktIdx ) ;
      }

      // release lock bucket latch, wrapper of _releaseOpLatch
      OSS_INLINE void releaseLockBktLatch( const dpsTransLockId & lockId )
      {
         const UTIL_OBJIDX bktIdx = _getBucketNo( lockId ) ;
         _releaseOpLatch( bktIdx ) ;
      }

   private:
      // Latch for normal lock operation ( acquire, tryAcquire,
      // testAcquire, release, releaseAll, hasWait etc on ) :
      //     . latch _rwMutext in shared mode
      //     . latch a bucket slot in exclusively
      void _acquireOpLatch ( const UTIL_OBJIDX bucketIndex )
      {
         _rwMutex.lock_r() ;
         _LockHdrBkt[ bucketIndex ].hashHdrLatch.get() ;
      }

      // release latches for normal lock operation
      void _releaseOpLatch ( const UTIL_OBJIDX bucketIndex )
      {
         _LockHdrBkt[ bucketIndex ].hashHdrLatch.release() ;
         _rwMutex.release_r() ;
      }

      // release/return a LRB Header to LRB Header manager
      INT32 _releaseLRBHdr( const UTIL_OBJIDX hdrIdx ) ;

      // get LRB pointer by its index
      dpsTransLRB * _getLRBPtrByIdx( const UTIL_OBJIDX idx ) ;

      // release/return a LRB to LRB manager 
      INT32 _releaseLRB( const UTIL_OBJIDX idx );

      // search LRB list ( owner, waiter or upgrade list ) and find
      // the one with given eduId
      BOOLEAN _getLRBByEDUId
      (
         const EDUID       eduId,
         const UTIL_OBJIDX lrbBegin,
         UTIL_OBJIDX     & idxEduId,
         dpsTransLRB *   & pLRBEduId,
         UTIL_OBJIDX     & indexPrev,
         dpsTransLRB *   & pLRBPrev
      ) ;

      //
      // walk through the owner LRB list, find the one with given
      // eduid and check if the input lockMode is compatible with
      // all owners
      void _getLRBByEDUIdAndCheckWaiterLockMode
      (
         const EDUID               eduId,
         const DPS_TRANSLOCK_TYPE  lockMode,
         const UTIL_OBJIDX         lrbBegin,
         UTIL_OBJIDX             & idxEduId,
         dpsTransLRB *           & pLRBEduId,
         UTIL_OBJIDX             & indexPrev,
         dpsTransLRB *           & pLRBPrev,
         BOOLEAN                 & foundIncomp
      ) ;

      // get LRB Header ( index and its pointer ) of a given lock from
      // LRB Header bucket
      BOOLEAN _getLRBHdrByLockId
      (
         const dpsTransLockId & lockId,
         UTIL_OBJIDX          & hdrIdx,
         dpsTransLRBHeader *  & pLRBHdr
      ) ;


      // add a LRB at the end of the queue ( waiter, upgrade or owner list )
      void _addToLRBListTail
      (
         UTIL_OBJIDX     & lrbBegin,
         const UTIL_OBJIDX idxNew
      ) ;


      // add a LRB at the given position in owner list ( the owner list is
      // sorted on lock mode in ascent order )
      void _addToOwnerLRBList
      (
         const UTIL_OBJIDX insPos,
         const UTIL_OBJIDX idxNew
      ) ; 


      // search owner LRB list, and find
      //  . if the edu is in owner list
      //  . the index which the new LRB shall be inserted after
      //  . the last index of compatible and pointer of first incompatible LRB
      void _searchOwnerLRBList
      (
         const EDUID               eduId,
         const DPS_TRANSLOCK_TYPE  lockMode,
         const UTIL_OBJIDX         lrbBegin,
         UTIL_OBJIDX             & idxToInsert,
         UTIL_OBJIDX             & idxLastComp,
         dpsTransLRB *           & pLRBIncomp,
         UTIL_OBJIDX             & idxEduId,
         UTIL_OBJIDX             & idxPrevEduId,
         dpsTransLRB *           & pLRBPrevEduId
      ) ;


      // add a LRB at the end of the EDU LRB chain,
      // which is a list of all locks acquired within a session/tx
      void _addToEDULRBListTail
      (
         _dpsTransExecutor    * dpsTxExectr,
         const UTIL_OBJIDX      idx,
         const dpsTransLockId & lockId
      ) ;


      // remove a LRB from a LRB list ( owner, waiter, upgrade list )
      void _removeFromLRBList
      (
         UTIL_OBJIDX     & idxBegin,
         const UTIL_OBJIDX idxDel,
         UTIL_OBJIDX     & idxNext
      ) ;


      // remove LRB from waiter or upgrade queue/list,
      // and wake up the next one in the queue if necessary
      void _removeFromUpgradeOrWaitList
      (
         _dpsTransExecutor *    dpsTxExectr,
         const dpsTransLockId & lockId,
         const UTIL_OBJIDX      bktIdx,
         const BOOLEAN          removeLRBHeader
      ) ;

  
      // remove a LRB Header from a LRB Header list
      void _removeFromLRBHeaderList
      (
         UTIL_OBJIDX & idxBegin,
         UTIL_OBJIDX   idxDel
      ) ;


      // remove a LRB from the EDU LRB list
      void _removeFromEDULRBList
      (
         _dpsTransExecutor    * dpsTxExectr,
         const UTIL_OBJIDX      idxDel,
         const dpsTransLockId & lockId
      ) ;


      // core logic of acquire, try or test to get a lock with given mode
      INT32 _tryAcquireOrTest
      (
         _dpsTransExecutor                * dpsTxExectr,
         const dpsTransLockId             & lockId,
         const DPS_TRANSLOCK_TYPE           requestLockMode,
         const DPS_TRANSLOCK_OP_MODE_TYPE   opMode,
         const UTIL_OBJIDX                  bktIdx,
         const BOOLEAN                      bktLatched,
         dpsTransRetInfo                  * pdpsTxResInfo
      ) ;


      // core logic of release a lock
      void _release
      (
         _dpsTransExecutor       * dpsTxExectr,
         const dpsTransLockId    & lockId,
         const BOOLEAN             bForceRelease
      ) ;


      // core logic of release all locks an executor holding
      void _releaseAll
      (
         _dpsTransExecutor    * dpsTxExectr,
         const dpsTransLockId & lockId
      ) ;


      // acquire and setup a new LRB header and a new LRB
      INT32 _prepareNewLRBAndHeader
      (
         _dpsTransExecutor *        dpsTxExectr,
         const dpsTransLockId     & lockId,
         const DPS_TRANSLOCK_TYPE   requestLockMode,
         UTIL_OBJIDX              & hdrIdxNew,
         dpsTransLRBHeader *      & pLRBHdrNew,
         UTIL_OBJIDX              & lrbIdxNew,
         dpsTransLRB       *      & pLRBNew
      ) ;


      // calculate the index to LRB Header bucket
      UTIL_OBJIDX _getBucketNo( const dpsTransLockId &lockId );

      // wakeup a lock waiting exectuor ( EDU )
      void _wakeUp( _dpsTransExecutor *dpsTxExectr ) ;

      // wait a lock till be woken up, lock timeout elapsed, or be interrupted
      INT32 _waitLock( _dpsTransExecutor *dpsTxExectr ) ;

      // format LRB to string, flat one line
      CHAR * _LRBToString ( const UTIL_OBJIDX idx, CHAR * pBuf, UINT32 bufSz ) ;

      // format LRB to string, one field/member per line, with optional prefix
      CHAR * _LRBToString 
      (
         const UTIL_OBJIDX idx,
         CHAR *            pBuf,
         UINT32            bufSz,
         CHAR *            prefix
      ) ;

      // format LRB Header to string, flat one line
      CHAR * _LRBHdrToString 
      (
         const UTIL_OBJIDX idx,
         CHAR            * pBuf,
         UINT32            bufSz
      ) ;

      // format LRB Header, one field/member per line, with optional prefix
      CHAR * _LRBHdrToString
      (
         const UTIL_OBJIDX idx,
         CHAR            * pBuf,
         UINT32            bufSz,
         CHAR            * prefix
      ) ;

   private:
      // LRB manager
      _utilSegmentManager< dpsTransLRB       > * _pLRBMgr ;

      // LRB Header manager
      _utilSegmentManager< dpsTransLRBHeader > * _pLRBHdrMgr ;

      // LRB Header bucket :
      //   class dpsTransLRBHeaderHash : public SDBObject
      //   {
      //   public :
      //      UTIL_OBJIDX    lrbHdrIdx ;     -- LRB Header index
      //      ossSpinXLatch  hashHdrLatch ;  -- bucket slot latch
      //   } ;
      dpsTransLRBHeaderHash  _LockHdrBkt[ MAX_LOCKBUCKET_NUM ] ;

      // flag mark if lock manager has been initialized
      BOOLEAN                _initialized ;

      // lock timeout 
      ossAtomic32            _lockTimeout ;

      //
      // monitor/dump EDU locking info latch
      //
      // Latching protocol :
      // Monitoring/dumping locking info of a specific EDU is mutually
      // exclusive with normal lock operation( acquire, tryAcquire,
      // testAcquire, release, releaseAll, hasWait etc on )
      //   Normal lock operation :
      //     . latch _rwMutext in shared mode
      //     . latch a bucket slot in exclusively
      //   Monitoring/dump EDU locking info :
      //     . latch _rwMutext in exclusive mode
      ossRWMutex             _rwMutex ;
   } ;
}

#endif // DPSTRANSLOCKMANAGER_HPP_

