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

   Source File Name = dpsTransLockMgr.cpp

   Descriptive Name = DPS lock manager

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =     JT  10/28/2018, locking performance improvement

*******************************************************************************/
#include "dpsTransLockMgr.hpp"
#include "dpsTransExecutor.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "sdbInterface.hpp"   // IContext
#include <stdio.h>

namespace engine
{
   dpsTransLockManager::dpsTransLockManager() : _pLRBMgr( NULL ),
                                                _pLRBHdrMgr( NULL ),
                                                _initialized( FALSE ),
                                                _lockTimeout( 0 ) { }


   dpsTransLockManager::~dpsTransLockManager() 
   {
      if ( _initialized )
      {
         fini() ;
      }
   }


   // free allocated LRB and LRB Headers
   void dpsTransLockManager::fini()
   {
      if ( _initialized )
      {
         // free LRB Header objects
         if ( _pLRBHdrMgr )
         {
            _pLRBHdrMgr->fini() ;
         }
         // delete LRB Header manager
         SAFE_OSS_DELETE( _pLRBHdrMgr ) ;
         _pLRBHdrMgr = NULL ;

         // free LRB objects
         if ( _pLRBMgr )
         {
            _pLRBMgr->fini() ;
         }
         // delete LRB manager
         SAFE_OSS_DELETE( _pLRBMgr ) ;
         _pLRBMgr = NULL ;

         _initialized = FALSE ;
      }
   }


   // initialization 
   INT32 dpsTransLockManager::init()
   {
      INT32 rc = SDB_OK;

      // init lock header bucket
      for ( UINT32 i = 0; i < MAX_LOCKBUCKET_NUM; i++ )
      {
         _LockHdrBkt[i].lrbHdrIdx = UTIL_INVALID_OBJ_INDEX ;
      }

      // new LRB manager
      _pLRBMgr = SDB_OSS_NEW _utilSegmentManager< dpsTransLRB > ;
      if ( NULL == _pLRBMgr )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR,
                 "Failed to create LRB Manager, rc: %d", rc ) ;
         goto error ;
      }

      // init LRB manager
      rc = _pLRBMgr->init( DPS_INIT_NUM_OF_LRB, DPS_INIT_NUM_OF_LRB_MAX ) ;
      if ( rc )
      {
         PD_LOG( PDERROR,
                 "Failed to allocate memory for LRB, rc: %d", rc ) ;
         goto error ;
      }

      // new LRB Header manager
      _pLRBHdrMgr = SDB_OSS_NEW _utilSegmentManager< dpsTransLRBHeader > ;
      if ( NULL == _pLRBHdrMgr )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR,
                 "Failed to create LRB Header Manager, rc: %d", rc ) ;
         goto error ;
      }

      // init LRB Header manager
      rc = _pLRBHdrMgr->init( DPS_INIT_NUM_OF_LRB_HEADER,
                              DPS_INIT_NUM_OF_LRB_HEADER_MAX );
      if ( rc )
      {
         PD_LOG( PDERROR,
                 "Failed to allocate memory for LRB Header, rc: %d", rc ) ;
         goto error ;
      }

      // init _lockTimeout
      _lockTimeout.swap( DPS_LOCK_TIMEOUT_DEFAULT ) ;

      // set initialized flag
      _initialized = TRUE ;

   done :
      return rc ;
   error :
      goto done ;
   }


   // 
   // Description: find the LRB Header address/pointer by its index
   // Function:    the LRB Header manager uses the index to find the object
   //              pointer from the object arrays   
   // Input:       the object ( LRB Header ) index to the object arrays
   // Return:      the pointer of the object or NULL
   //
   dpsTransLRBHeader * dpsTransLockManager::getLRBHdrPtrByIdx
   (
      const UTIL_OBJIDX hdrIdx
   )
   {
 
      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( hdrIdx ),
                  "Invalid LRB Header index." ) ;

      dpsTransLRBHeader * pLRBHdr = _pLRBHdrMgr->getObjPtrByIndex( hdrIdx ) ;

      SDB_ASSERT( pLRBHdr, "Invalid LRB Header." ) ;
      
      return pLRBHdr ;
   }


   // 
   // Description: find the LRB address/pointer by its index
   // Function:    the LRB manager uses the index to find the object pointer
   //              from the object arrays   
   // Input:       the object ( LRB ) index to the object arrays
   // Return:      the pointer of the object or NULL
   //
   dpsTransLRB * dpsTransLockManager::_getLRBPtrByIdx( const UTIL_OBJIDX idx )
   {
      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( idx ), "Invalid LRB index." ) ;
      
      dpsTransLRB * pLRB = _pLRBMgr->getObjPtrByIndex( idx );
      SDB_ASSERT( pLRB, "Invalid LRB." ) ;
     
      return pLRB ;
   }


   // 
   // Description: release/return a LRB Header to LRB Header manager
   // Function:    return a LRB Header to the LRB Header Manager
   // Input:       the object ( LRB Header ) index to the object arrays
   // Return:      SDB_OK or SDB_INVALIDARG when error      
   //
   INT32 dpsTransLockManager::_releaseLRBHdr( const UTIL_OBJIDX hdrIdx )
   {
      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( hdrIdx ),
                  "Invalid LRB Header index." ) ;
      // reset LRB Header
      dpsTransLRBHeader * pLRBHdr = getLRBHdrPtrByIdx( hdrIdx ) ;   
      pLRBHdr->nextLRBHdrIdx = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdr->ownerLRBIdx   = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdr->waiterLRBIdx  = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdr->upgradeLRBIdx = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdr->lockId.reset() ;
      // release LRB Header
      INT32 rc = _pLRBHdrMgr->release( hdrIdx );
      SDB_ASSERT( SDB_OK == rc, "Failed to release LRB Header" ) ;
      return rc ;
   }


   // 
   // Description: release/return a LRB to LRB manager
   // Function:    return a LRB to the LRB Manager
   // Input:       the object ( LRB ) index to the object arrays
   // Return:      SDB_OK or SDB_INVALIDARG when error      
   //
   INT32 dpsTransLockManager::_releaseLRB( const UTIL_OBJIDX idx )
   {
      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( idx ), "Invalid LRB index." ) ;
      // reset LRB
      dpsTransLRB * pLRB  = _getLRBPtrByIdx( idx ) ;
      pLRB->dpsTxExectr   = NULL ;
      pLRB->eduLrbIdxNext = UTIL_INVALID_OBJ_INDEX ;
      pLRB->eduLrbIdxPrev = UTIL_INVALID_OBJ_INDEX ;
      pLRB->lrbHdrIdx     = UTIL_INVALID_OBJ_INDEX ;
      pLRB->nextLRBIdx    = UTIL_INVALID_OBJ_INDEX ;
      pLRB->lockMode      = DPS_TRANSLOCK_MAX ; 
      // release LRB
      INT32 rc = _pLRBMgr->release( idx );
      SDB_ASSERT( SDB_OK == rc, "Failed to release LRB" ) ;
      return rc ;
   }


   // 
   // Description: search the LRB Header chain and find the one with same lockId
   // Function:    walk through LRB Header list/chain, find the one with same
   //              lockId. 
   // Input: 
   //    lockId   -- lock Id
   //    hdrIdx   -- the first LRB Header object index in the chain
   //    pLRBHdr  -- the first LRB index in the owner queue/list
   // Output:
   //    hdrIdx   -- the index of the first LRB Header object matches
   //                the lockId if it is found.
   //    pLRBHdr  -- the pointer of first LRB Header object matches
   //                the lockId if it is found. If not, it shall be the
   //                pointer of the last LRB Header object in the list  
   //
   // Return:     true  -- found the LRB Header object with same lockId
   //             false -- not found
   //
   BOOLEAN dpsTransLockManager::_getLRBHdrByLockId
   (
      const dpsTransLockId &lockId,
      UTIL_OBJIDX          &hdrIdx,
      dpsTransLRBHeader *  &pLRBHdr
   ) 
   {
      BOOLEAN found = FALSE ;
      while ( IS_VALID_SEG_OBJ_INDEX( hdrIdx ) )
      {
         pLRBHdr = getLRBHdrPtrByIdx( hdrIdx ) ;
         if ( lockId == pLRBHdr->lockId )
         {
            found = TRUE ;
            break ;
         }
         hdrIdx = pLRBHdr->nextLRBHdrIdx ;
      }
      return found ;
   }


   BOOLEAN dpsTransLockManager::getLRBHdrByLockId
   (
      const dpsTransLockId &lockId,
      UTIL_OBJIDX          &hdrIdx,
      dpsTransLRBHeader *  &pLRBHdr
   )
   {
      BOOLEAN found = FALSE ;
      UTIL_OBJIDX bktIdx = _getBucketNo( lockId ) ;

      hdrIdx = _LockHdrBkt[ bktIdx ].lrbHdrIdx ;
      found  = _getLRBHdrByLockId( lockId, hdrIdx, pLRBHdr ) ;
      if ( ! found ) 
      {

         hdrIdx  = UTIL_INVALID_OBJ_INDEX ;
         pLRBHdr = NULL ;
      } 
      return found ;
   }


   //
   // Description: search the LRB chain and find the one with given eduid
   // Function:    walk through LRB list/chain, find the LRB with same eduId
   // Input:
   //    eduId    -- EDU Id.
   //    idx      -- the first LRB index in the chain( owner, waiter or upgrade
   //                queue )
   // Output:
   //    idx      -- the index of the first LRB object matches given eduId
   //                if it is found.
   //    pLRB     -- the pointer of first LRB object matches the eduId
   //                if it is found. If not, it shall be the pointer of
   //                the last LRB object in the list
   //    idxPrev  -- the index of the LRB in the list previous to the owner
   //    pLRBPrev -- the pointer of the LRB
   //
   // Return:     true  -- found the LRB object with the given eduId
   //             false -- not found
   //
   BOOLEAN dpsTransLockManager::_getLRBByEDUId
   (
      const EDUID     eduId,
      UTIL_OBJIDX   & idx,
      dpsTransLRB * & pLRB,
      UTIL_OBJIDX   & idxPrev,
      dpsTransLRB * & pLRBPrev
   )
   {
      BOOLEAN found = FALSE ;
      pLRBPrev = NULL ;
      idxPrev  = UTIL_INVALID_OBJ_INDEX ; 
      while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
      {
         pLRB = _getLRBPtrByIdx( idx ) ;
         if ( eduId == pLRB->dpsTxExectr->getEDUID() )
         {
            found = TRUE ;
            break ;
         }

         idxPrev  = idx ;
         pLRBPrev = pLRB ;
         idx      = pLRB->nextLRBIdx ;
      }
      return found ;
   }


   //
   // Description: add a LRB at the end of the LRB chain/list
   // Function:    walk through LRB list/chain, add the LRB at the end of
   //              list( owner, waiter or upgrade )
   // Input:
   //    lrbBegin -- the first LRB index in the chain( owner, waiter or upgrade
   //                queue )
   //    idxNew   -- the LRB index to be added in
   //
   // Output:     None
   //
   // Return:     None
   //
   void dpsTransLockManager::_addToLRBListTail
   (
      UTIL_OBJIDX       & lrbBegin,
      const UTIL_OBJIDX   idxNew
   )
   {
      if ( IS_VALID_SEG_OBJ_INDEX( lrbBegin ) )
      {
         dpsTransLRB * plrb = NULL ;
         UTIL_OBJIDX   idx  = lrbBegin ;

         // find the last LRB
         while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
         {
            plrb = _getLRBPtrByIdx( idx ) ;
            idx  = plrb->nextLRBIdx ;
         }
         plrb->nextLRBIdx = idxNew ;
      }
      else
      {
         lrbBegin = idxNew ;
      }
   }


   //
   // Description: add a LRB to owner list right after a given LRB index
   // Function:    the onwer list is sorted on lock mode in ascent order,
   //              add a LRB right after the given LRB index 
   // Input:
   //    lrbPos -- the LRB index where the new LRB index is being inserted after
   //               
   //    idxNew -- the LRB index to be added in
   //
   // Output:     None
   //
   // Return:     None
   //
   void dpsTransLockManager::_addToOwnerLRBList
   (
      const UTIL_OBJIDX insPos,
      const UTIL_OBJIDX idxNew
   )
   {
      if (   IS_VALID_SEG_OBJ_INDEX( insPos )
          && IS_VALID_SEG_OBJ_INDEX( idxNew ) )
      {   
         dpsTransLRB *plrb, *pLRB ;
         plrb = _getLRBPtrByIdx( insPos ) ;
         pLRB = _getLRBPtrByIdx( idxNew ) ;
         
         pLRB->nextLRBIdx = plrb->nextLRBIdx ;
         plrb->nextLRBIdx = idxNew ;
      }
   }


   //
   // Description: search owner LRB list and find the expected LRB
   // Function:    walk through owner LRB list ( it is sorted on lockMode in
   //              ascending order ) and find out :
   //              . if the edu is in owner list
   //              . the index which the new LRB shall be inserted after
   //              . the index of last compatible and pointer of first
   //                incompatible LRB
   // Input:
   //    eduId    -- edu Id
   //    lockMode -- lock mode
   //    lrbBegin -- the first LRB index in the owner list
   // Output:
   //    idxToInsert   -- the index which the new LRB shall be inserted after
   //    idxLastComp   -- the index of last compatible LRB
   //    pLRBIncomp    -- pointer of first incompatible LRB
   //    idxEduId      -- the LRB index owned by same eduId
   //    idxPrevEduId  -- the LRB index previous to the idxEduId
   //    pLRBPrevEduId -- the address of the LRB previous to the idxEduId
   // Return:     None
   //
   void dpsTransLockManager::_searchOwnerLRBList
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
   )
   {
      EDUID lrbEduid = 0 ;
      UTIL_OBJIDX idx = lrbBegin, idxPrev = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRB *plrb     = NULL,
                  *plrbPrev = NULL ;
      BOOLEAN foundMyself   = FALSE,
              foundInsert   = FALSE,
              foundLastComp = FALSE ;

      if ( IS_VALID_SEG_OBJ_INDEX( lrbBegin ) )
      {
         while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
         {
            plrb     = _getLRBPtrByIdx( idx ) ;
            lrbEduid = plrb->dpsTxExectr->getEDUID() ;
            if ( ( ! foundMyself ) && ( eduId == lrbEduid ) )
            {
               // save the index if the given eduId is found in the owner list
               idxEduId      = idx ;
               // save the previous LRB index and the pointer/address
               idxPrevEduId  = idxPrev ;
               pLRBPrevEduId = plrbPrev ;
               foundMyself   = TRUE ;
            }

            // the owner list is sorted on lock mode in ascent order
            if ( ( ! foundInsert ) && ( lockMode < plrb->lockMode ) )
            {
               // save the LRB index where it shall be inserted after 
               idxToInsert = idxPrev ;
               foundInsert = TRUE ; 
            }

            if (    ( ! foundLastComp )
                 && ( eduId != lrbEduid )
                 && ( ! dpsIsLockCompatible( plrb->lockMode, lockMode ) ) )
            {
               // save the LRB index previous to first incompatible LRB
               idxLastComp   = idxPrev ; 
               // save the address/pointer of first incompatible LRB
               pLRBIncomp    = plrb ;
               foundLastComp = TRUE ;
            } 

            // break if all jobs are done
            if ( foundMyself && foundInsert && foundLastComp )
            {
               break ;
            }

            // remember previous index and pointer
            idxPrev  = idx ;
            plrbPrev = plrb ;

            // move to next      
            idx      = plrb->nextLRBIdx ;
         }
      }
   }


   //
   // Description: add a LRB at the end of the EDU LRB chain, which is the list
   //              of all locks acquired within a session/tx
   // Function:    walk through EDU LRB chain( doubly linked list ),
   //              add the LRB at the end of list. 
   //              
   //              the dpsTxExectr::_lastLRBIdx is the latest LRB index
   //              acquired within the same tx
   //
   // Input:
   //    dpsTxExectr -- _dpsTransExecutor ptr
   //    idx         -- the LRB index to be added in
   //
   // Output:     None
   //
   // Return:     None
   //
   void dpsTransLockManager::_addToEDULRBListTail
   (
      _dpsTransExecutor    * dpsTxExectr,
      const UTIL_OBJIDX      idx,
      const dpsTransLockId & lockId
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
      if ( dpsTxExectr )
      {
         // get the index of last LRB in the EDU LRB chain
         // and add the new LRB into the chain
         UTIL_OBJIDX eduLRBIdx = dpsTxExectr->getLastLRBIdx() ;
         if ( IS_VALID_SEG_OBJ_INDEX( eduLRBIdx ) )
         {
            dpsTransLRB * plrb  = _getLRBPtrByIdx( eduLRBIdx )  ;
            plrb->eduLrbIdxNext = idx ; 
            if ( IS_VALID_SEG_OBJ_INDEX( idx ) )
            {
               dpsTransLRB *pLRB   = _getLRBPtrByIdx( idx ) ;
               pLRB->eduLrbIdxPrev = eduLRBIdx ;
               pLRB->eduLrbIdxNext = UTIL_INVALID_OBJ_INDEX ;
            }
         } 

         if ( IS_VALID_SEG_OBJ_INDEX( idx ) )
         {
            // add to executor lock map if it is CS or CL lock
            if ( ! lockId.isLeafLevel() )
            {
               dpsTxExectr->addLock( lockId, idx ) ;
            }

            // increase the lock count
            dpsTxExectr->incLockCount() ;

            // clear the wait info in dpsTxExectr
            dpsTxExectr->clearWaiterInfo() ;
         }

         // save the last LRB index
         dpsTxExectr->setLastLRBIdx( idx ) ;
      }
   }


   //
   // Description: remove a LRB from a LRB chain
   // Function:    walk through the LRB chain(linked list, it can be lock owner
   //              list, lock waiter list or upgrade list ) and remove the LRB
   //              from the chain. Please note, it doesn't release/return the
   //              LRB to the LRB manager. 
   // Input:
   //    idxBegin -- the index of the LRB in the list that begin to search
   //    idxDel   -- the index of the LRB object to be removed
   //
   // Output:
   //    idxBegin -- if the idxBegin is same as idxDel,
   //                it will be updated with the index of LRB next to idxDel
   //    idxNext  -- the index next to the idxDel
   // Return:     None
   //
   void dpsTransLockManager::_removeFromLRBList
   (
      UTIL_OBJIDX     & idxBegin,
      const UTIL_OBJIDX idxDel,
      UTIL_OBJIDX     & idxNext
   )
   {
      idxNext = UTIL_INVALID_OBJ_INDEX ; 
      if (    IS_VALID_SEG_OBJ_INDEX( idxBegin )
           && IS_VALID_SEG_OBJ_INDEX( idxDel ) )
      {
         UTIL_OBJIDX idx = idxBegin ;
         dpsTransLRB *plrb, *pLRB ;

         // if the first one is the one to be removed
         if ( idxDel == idxBegin )
         {
            pLRB = _getLRBPtrByIdx( idxDel ) ;
            idxBegin = pLRB->nextLRBIdx ;
            idxNext  = pLRB->nextLRBIdx ;
         }
         else
         {
            while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
            {
               plrb = _getLRBPtrByIdx( idx ) ;
               if ( idxDel == plrb->nextLRBIdx )
               {
                  pLRB = _getLRBPtrByIdx( idxDel ) ;
                  plrb->nextLRBIdx = pLRB->nextLRBIdx ;
                  idxNext = pLRB->nextLRBIdx ;
                  break ;
               }
               idx = plrb->nextLRBIdx ;
            }
         }
      }
   }


   //
   // Description: remove waiter from waiter or upgrade queue/list
   // Function:    remove the waiter LRB ( dpsTxExectr->getWaiterLRBIdx() )
   //              from upgrade or waiter queue/list, and wakeup the next
   //              one in waiting if its lock mode is compatible with
   //              current waiter
   // Input:
   //    dpsTxExectr  -- dpsTxExectr
   //
   // Output:
   //    hdrIdx  -- the LRB Header index in the waiter LRB
   //    pLRBHdr -- the LRB Header address/pointer
   // Return:     None
   //
   void dpsTransLockManager::_removeFromUpgradeOrWaitList
   (
      _dpsTransExecutor *   dpsTxExectr,
      UTIL_OBJIDX         & hdrIdx,
      dpsTransLRBHeader * & pLRBHdr
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;

      UTIL_OBJIDX idx = dpsTxExectr->getWaiterLRBIdx() ;
      UTIL_OBJIDX idxNext = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRB *pLRB = NULL , *plrb = NULL ;
      
      if ( IS_VALID_SEG_OBJ_INDEX( idx ) )
      {
         pLRB = _getLRBPtrByIdx( idx ) ;
         SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( pLRB->lrbHdrIdx ),
                     "Invalid LRB Header." );
         hdrIdx  = pLRB->lrbHdrIdx ;
         pLRBHdr = getLRBHdrPtrByIdx( hdrIdx ) ;
       
         if ( DPS_QUE_UPGRADE == dpsTxExectr->getWaiterQueType() )
         {
            // remove from upgrade list
            _removeFromLRBList( pLRBHdr->upgradeLRBIdx, idx, idxNext ) ; 

            // clear the wait info in dpsTxExectr
            dpsTxExectr->clearWaiterInfo() ;

            // if it is the last one in upgrade list,
            // set idxNext to the first one in waiter list
            if (    ( ! IS_VALID_SEG_OBJ_INDEX( pLRB->nextLRBIdx ) )
                 || ( ! IS_VALID_SEG_OBJ_INDEX( idxNext ) ) )
            {
               idxNext = pLRBHdr->waiterLRBIdx ;
            }
         }
         else if ( DPS_QUE_WAITER == dpsTxExectr->getWaiterQueType() )
         {
            // remove from waiter list
            _removeFromLRBList( pLRBHdr->waiterLRBIdx, idx, idxNext ) ; 

            // clear the wait info in dpsTxExectr
            dpsTxExectr->clearWaiterInfo() ;
         }

         // wake up the next one if necessary, i.e., its lock mode is
         // compatible with current waiter
         if ( IS_VALID_SEG_OBJ_INDEX( idxNext ) )
         {
            plrb = _getLRBPtrByIdx( idxNext ) ;
            SDB_ASSERT( pLRB->lrbHdrIdx == plrb->lrbHdrIdx, "Invalid LRB" ) ;
            if ( dpsIsLockCompatible( pLRB->lockMode, plrb->lockMode ) )
            {
               _wakeUp( plrb->dpsTxExectr ) ;   
            }
         }

         // release the waiter LRB
         _releaseLRB( idx ) ;      
      }
   }


   //
   // Description: remove a LRB Header from a LRB Header chain
   // Function:    walk through the LRB Header chain( linked list ) and remove
   //              the LRB Header from the chain. Please note, it doesn't
   //              release/return the LRB Header to the LRB Header manager.
   // Input:
   //    idxBegin -- the index of the first LRB Header in the list
   //    idxDel   -- the index of the LRB Header object to be removed
   //
   // Output:
   //    idxBegin -- if the idxBegin is same as idxDel, it will be updated with
   //                the index of LRB Header next to idxDel
   // Return:     None
   //
   void dpsTransLockManager::_removeFromLRBHeaderList
   (
      UTIL_OBJIDX & idxBegin,
      UTIL_OBJIDX   idxDel
   )
   {
      if (   IS_VALID_SEG_OBJ_INDEX( idxBegin )
          && IS_VALID_SEG_OBJ_INDEX( idxDel ) )
      {
         UTIL_OBJIDX idx = idxBegin;
         dpsTransLRBHeader *plrbHdr = NULL, *pLRBHeader = NULL;

         // if the first one is the one to be removed
         if ( idxDel == idxBegin )
         {
            pLRBHeader = getLRBHdrPtrByIdx( idxDel ) ;
            idxBegin   = pLRBHeader->nextLRBHdrIdx ;
         }
         else
         {
            while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
            {
               plrbHdr = getLRBHdrPtrByIdx( idx ) ;
               if ( idxDel == plrbHdr->nextLRBHdrIdx )
               {
                  pLRBHeader             = getLRBHdrPtrByIdx( idxDel ) ;
                  plrbHdr->nextLRBHdrIdx = pLRBHeader->nextLRBHdrIdx ;
                  break ;
               }
               idx = plrbHdr->nextLRBHdrIdx ;
            }
         }
      }
   }


   //
   // Description: remove a LRB from the EDU LRB chain
   // Function:    walk through the EDU LRB chain( doubly linked list ),
   //              and remove the LRB Header from the chain.
   //              Please note, it doesn't release/return the LRB to
   //              the LRB Header manager.
   // Input:
   //    dpsTxExectr  -- dpsTxExectr
   //    idxDel       -- the index of the LRB object to be removed
   //
   // Output:
   //    dpsTxExectr->_lastLRBIdx -- the last LRB object index
   //                                in the EDU LRB chain.
   //                                If it is equal to idxDel, it will be
   //                                updated with the index of LRB previous
   //                                to idxDel
   // Return:     None
   //
   void dpsTransLockManager::_removeFromEDULRBList
   (
      _dpsTransExecutor    * dpsTxExectr,
      const UTIL_OBJIDX      idxDel,
      const dpsTransLockId & lockId
   )
   {
      if (   ( dpsTxExectr )
           && IS_VALID_SEG_OBJ_INDEX( idxDel )
           && IS_VALID_SEG_OBJ_INDEX( dpsTxExectr->getLastLRBIdx() ) )
      {
         UTIL_OBJIDX eduLRBIdx = dpsTxExectr->getLastLRBIdx() ;
         UTIL_OBJIDX idx       = eduLRBIdx ;
         // walk though the EDU LRB list find and remove the give LRB
         // from the list
         while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
         {
            dpsTransLRB *plrb = _getLRBPtrByIdx( idx ) ;
            // found the LRB in chain and remove it
            if ( idxDel == idx )
            {
               if ( IS_VALID_SEG_OBJ_INDEX( plrb->eduLrbIdxPrev ) )
               {
                  dpsTransLRB *plrbP = _getLRBPtrByIdx( plrb->eduLrbIdxPrev ) ;
                  plrbP->eduLrbIdxNext = plrb->eduLrbIdxNext ;
               }
               if ( IS_VALID_SEG_OBJ_INDEX( plrb->eduLrbIdxNext ) )
               {
                  dpsTransLRB *plrbN = _getLRBPtrByIdx( plrb->eduLrbIdxNext ) ;
                  plrbN->eduLrbIdxPrev = plrb->eduLrbIdxPrev ;
               }
               if ( eduLRBIdx == idx )
               {
                  dpsTxExectr->setLastLRBIdx( plrb->eduLrbIdxPrev ) ;
               }
 
               // remove the lock from lock ID map if it is CS or CL lock
               if ( ! lockId.isLeafLevel() )
               {
                  dpsTxExectr->removeLock( lockId ) ;
               }

               // decrease the lock count
               dpsTxExectr->decLockCount() ;

               break ;
            }
            idx = plrb->eduLrbIdxPrev ;
         }
      }
   }


   //
   // Description: acquire, try or test to get a lock with given mode
   // Function: core of acquire, try or test operation, behaviour varies
   //   depending operation mode :
   //   1 DPS_TRANSLOCK_OP_MODE_ACQUIRE
   //     return SDB_OK
   //       . lock acquired, new LRB is added in owner list
   //       . if holing higher level lock, no need to add new LRB in owner list
   //     return SDB_PERM
   //       . can't upgrade to requested lock mode
   //     return SDB_DPS_TRANS_APPEND_TO_WAIT
   //       . need to upgrade, new LRB is added to upgrade list
   //       . need to wait, new LRB is added to waiter list   
   //   2 DPS_TRANSLOCK_OP_MODE_TRY
   //     try mode will not add LRB to waiter or upgrade list
   //     return SDB_OK
   //       . lock acquired, new LRB is added in owner list
   //       . holing higher level lock, no need to add new LRB in owner list
   //     return SDB_PERM
   //       . can't upgrade to requested lock mode
   //     return SDB_DPS_TRANS_LOCK_INCOMPATIBLE
   //       . request lock mode can't be acquired
   //   3 DPS_TRANSLOCK_OP_MODE_TEST
   //     return SDB_OK
   //       . request lock can be acquired
   //     return SDB_PERM
   //       . can't upgrade to requested lock mode
   //     return SDB_DPS_TRANS_LOCK_INCOMPATIBLE
   //       . request lock mode can't be acquired
   //
   // Input:
   //    dpsTxExectr     -- dpsTxExectr
   //    lockId          -- lock Id
   //    requestLockMode -- lock mode being requested
   //    opMode          -- try     ( DPS_TRANSLOCK_OP_MODE_TRY )
   //                       acquire ( DPS_TRANSLOCK_OP_MODE_ACQUIRE )
   //                       test    ( DPS_TRANSLOCK_OP_MODE_TEST )
   //    bktIdx          -- bucket index
   //    bktLatched      -- if bucket is already latched
   //
   // Output:
   //    pdpsTxResInfo   -- pointer to dpsTransRetInfo
   // Return:
   //     SDB_OK,
   //     SDB_PERM,
   //     SDB_DPS_TRANS_APPEND_TO_WAIT,
   //     SDB_DPS_TRANS_LOCK_INCOMPATIBLE,
   //     or other errors
   //
   INT32 dpsTransLockManager::_tryAcquireOrTest
   (
      _dpsTransExecutor                * dpsTxExectr,
      const dpsTransLockId             & lockId,
      const DPS_TRANSLOCK_TYPE           requestLockMode,
      const DPS_TRANSLOCK_OP_MODE_TYPE   opMode,
      const UTIL_OBJIDX                  bktIdx,
      const BOOLEAN                      bktLatched,
      dpsTransRetInfo                  * pdpsTxResInfo
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;

      INT32 rc = SDB_OK ;
      UTIL_OBJIDX hdrIdxNew = UTIL_INVALID_OBJ_INDEX ,
                  lrbIdxNew = UTIL_INVALID_OBJ_INDEX ,
                  hdrIdx    = UTIL_INVALID_OBJ_INDEX ;
      UTIL_OBJIDX idxToInsert  = UTIL_INVALID_OBJ_INDEX ,
                  idxLastComp  = UTIL_INVALID_OBJ_INDEX ,
                  idxEduId     = UTIL_INVALID_OBJ_INDEX ,
                  idxPrevEduId = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRB *pLRBNew       = NULL ,
                  *pLRBIncomp    = NULL ,
                  *pLRBPrevEduId = NULL ,
                  *pLRB          = NULL ;
      dpsTransLRBHeader *pLRBHdrNew = NULL ,
                        *pLRBHdr    = NULL ;

      BOOLEAN bFreeLRB       = FALSE ,
              bFreeLRBHeader = FALSE ,
              bLatched       = FALSE ;

      EDUID eduId = dpsTxExectr->getEDUID() ;

      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;

      // acquire and prepare new LRB and LRB Header
      if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
      {
         // no need to allocate LRB Header and LRB for test mode
         rc = _prepareNewLRBAndHeader( dpsTxExectr, lockId, requestLockMode,
                                       hdrIdxNew, pLRBHdrNew,
                                       lrbIdxNew, pLRBNew ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( lrbIdxNew ) &&
                     IS_VALID_SEG_OBJ_INDEX( hdrIdxNew ) &&
                     pLRBNew &&
                     pLRBHdrNew, "Invalid LRB or LRB Header." ) ;
         bFreeLRB       = TRUE ;
         bFreeLRBHeader = TRUE ;
      }

      // latch bucket
      if ( ! bktLatched )
      {
         _acquireOpLatch( bktIdx ) ;
      }
      bLatched = TRUE ;

      // if no LRB Header
      if ( ! IS_VALID_SEG_OBJ_INDEX( _LockHdrBkt[ bktIdx ].lrbHdrIdx ) )
      {
         if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
         {
            // add new LRB header to the link
            _LockHdrBkt[ bktIdx ].lrbHdrIdx = hdrIdxNew ;

            // add new LRB to EDU LRB list
            _addToEDULRBListTail( dpsTxExectr, lrbIdxNew, lockId ) ;

            // mark the new LRB and LRB Header are used
            bFreeLRB       = FALSE ;
            bFreeLRBHeader = FALSE ;
         }

         // job done
         goto done;
      }

      // LRB header exists,
      // lookup the LRB header list and find the one with same lockId
      pLRBHdr = NULL ;
      hdrIdx  = _LockHdrBkt[ bktIdx ].lrbHdrIdx ;
      if ( ! _getLRBHdrByLockId( lockId, hdrIdx, pLRBHdr ) ) 
      {
         // no LRB header with same lockId is found,
         // add the new LRB Header in the lrb header list
         if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
         {
            // at this time, pLRBHdr shall be the tail of LRB header list.
            // add the new LRB header to LRB Header list ; 
            pLRBHdr->nextLRBHdrIdx = hdrIdxNew ; 

            // add the new LRB to EDU LRB list
            _addToEDULRBListTail( dpsTxExectr, lrbIdxNew, lockId ) ;

            // mark the new LRB and new LRB Header are used
            bFreeLRB       = FALSE ;
            bFreeLRBHeader = FALSE ;
         }

         // job done
         goto done;
      }

      // found the LRB header with same lockId
      SDB_ASSERT( ( NULL != pLRBHdr ), "Invalid LRB Header" ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( hdrIdx ), "Invalid LRB Header index");

      // update the lrbHdrIdx of new LRB to current LRB Header index
      if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
      {
         pLRBNew->lrbHdrIdx = hdrIdx ;
      }

      // short cut for non-leaf lock ( CS, CL ),
      // lookup the executor _mapLockID map, if it is found and current
      // lock mode covers the requesting mode then increase refCounter,
      // and job is done. Otherwise, still need to go through the normal
      // routine.
      if ( ! lockId.isLeafLevel() )
      {
         UTIL_OBJIDX lrbIdx = UTIL_INVALID_OBJ_INDEX ;
         pLRB = NULL ;
         if ( dpsTxExectr->findLock( lockId, lrbIdx ) )
         { 
            if ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
            {
               pLRB = _getLRBPtrByIdx( lrbIdx ) ;
            }
            if ( pLRB && dpsLockCoverage( pLRB->lockMode, requestLockMode ) )
            {
               if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
               {
                  pLRB->refCounter++ ;
               }
               goto done ;
            }
         }
      }

      // search owner LRB list, which is sorted on lock mode in ascent order
      //  . to find if the edu is in owner list
      //  . the index which the new LRB shall be inserted after
      //  . the last index of compatible and pointer of first incompatible LRB
      //
      // idxToInsert   -- index to insert after
      // idxLastComp   -- index of last compatible LRB
      // idxEduId      -- LRB index with same EDUId
      // idxPrevEduId  -- idx previous idxEduId
      // pLRBPrevEduId -- addr of idxPrevEduId
      // pLRBIncomp    -- addr of first incompatible
      _searchOwnerLRBList( eduId, requestLockMode, pLRBHdr->ownerLRBIdx,
                           idxToInsert, idxLastComp, pLRBIncomp,
                           idxEduId, idxPrevEduId, pLRBPrevEduId ) ;
      if ( IS_VALID_SEG_OBJ_INDEX( idxEduId ) )
      {
         //
         // in owner list
         //
         pLRB = _getLRBPtrByIdx( idxEduId ) ;
         SDB_ASSERT( pLRB && ( pLRB->lrbHdrIdx == hdrIdx ),
                     "Invalid LRB or the lrbHdrIdx doesn't match "
                     "the LRB Header index" ) ;

         // if current holding lock mode covers the requesting mode,
         // then job is done
         if ( dpsLockCoverage( pLRB->lockMode, requestLockMode ) )
         {
            if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
            {
               pLRB->refCounter++ ;
            }
            goto done ; 
         }

         // if dpsUpgradeCheck is OK
         rc = dpsUpgradeCheck( pLRB->lockMode, requestLockMode ) ;
         if ( SDB_OK != rc )
         {
            // can't do upgrade, job done with error rc set

            // constrct conflict lock info 
            if ( pdpsTxResInfo )
            {
               pdpsTxResInfo->_lockID   = pLRBHdr->lockId ;
               pdpsTxResInfo->_lockType = pLRB->lockMode ;
               pdpsTxResInfo->_eduID    = pLRB->dpsTxExectr->getEDUID();
            }

            goto done ;
         }
       
         // try to do upgrade
         //
         // check if the requested mode is compatible with other owners
         if ( IS_VALID_SEG_OBJ_INDEX( idxLastComp ) || ( NULL != pLRBIncomp ) )
         {
            // valid idxLastComp or valid pLRBIncomp, means
            // an incompatible LRB is found, i.e., not compatible
            // with others 

            if ( DPS_TRANSLOCK_OP_MODE_ACQUIRE == opMode )
            {
               // add the new LRB to upgrade list
               _addToLRBListTail( pLRBHdr->upgradeLRBIdx, lrbIdxNew ) ;

               // set the wait info in dpsTxExectr
               dpsTxExectr->setWaiterInfo( lrbIdxNew, DPS_QUE_UPGRADE ) ;

               // mark the new LRB is used
               bFreeLRB = FALSE ;

               // set return code to SDB_DPS_TRANS_APPEND_TO_WAIT
               rc = SDB_DPS_TRANS_APPEND_TO_WAIT ;
            }
            else
            {
               // for try or test mode
               // . set return code to SDB_DPS_TRANS_LOCK_INCOMPATIBLE
               // . no need to add to upgrade/waiter list
               rc = SDB_DPS_TRANS_LOCK_INCOMPATIBLE ;
            }

            // construct conflcit lock info ( representative )
            if ( pdpsTxResInfo )
            {
               pdpsTxResInfo->_lockID   = pLRBHdr->lockId ;
               pdpsTxResInfo->_lockType = pLRBIncomp->lockMode ;
               pdpsTxResInfo->_eduID    = pLRBIncomp->dpsTxExectr->getEDUID();
            }

            // job done
            goto done ;
         }
         else
         {
            // compatible with all others
            if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
            {
               // upgrade/convert to request mode.
               //   when upgrade, it implies the request mode is greater
               //   than current mode. Since the owner list is sorted on
               //   the lock mode in acsent order, we will need to move 
               //   the owner LRB to the right place by following two steps :
               //     . remove from current place
               //     . move it to the place right after the idxToInsert

               // remove from current place
               if ( IS_VALID_SEG_OBJ_INDEX( idxPrevEduId ) )
               {
                  // when idxPrevEduId is the valid, the pLRBPrevEduId
                  // must be a valid address. 
                  SDB_ASSERT( pLRBPrevEduId, "Invalid LRB index " ) ;
                  pLRBPrevEduId->nextLRBIdx = pLRB->nextLRBIdx ;
               }
               else
               {
                  // if idxPrevEduId is invalid, means it is the first one
                  // in the owner list
                  pLRBHdr->ownerLRBIdx = pLRB->nextLRBIdx ;
               }

               // insert it to the new position
               if ( IS_VALID_SEG_OBJ_INDEX( idxToInsert ) )
               {
                  _addToOwnerLRBList( idxToInsert, idxEduId ) ;
               }
               else
               {
                  // set nextLRBIdx to UTIL_INVALID_OBJ_INDEX
                  // before adding it at the end of owner LRB list.
                  pLRB->nextLRBIdx = UTIL_INVALID_OBJ_INDEX ;

                  // add it at the end of owner list
                  _addToLRBListTail( pLRBHdr->ownerLRBIdx, idxEduId ) ;
               }

               // clear the wait info in dpsTxExectr
               dpsTxExectr->clearWaiterInfo() ;

               // update current lock mode to the request mode
               pLRB->lockMode = requestLockMode ;
             
               pLRB->refCounter++ ;
            }

            // job done
            goto done ; 
         }
      }
      else
      {
         //
         // not in owner list
         //

         // check if lock is compatible with all owners
         if ( IS_VALID_SEG_OBJ_INDEX( idxLastComp ) || ( NULL != pLRBIncomp ) )
         {
            // found an incompatible one, i.e., not compatible with others

            if ( DPS_TRANSLOCK_OP_MODE_ACQUIRE == opMode )
            {
               // add it at the end of waiter list
               _addToLRBListTail( pLRBHdr->waiterLRBIdx, lrbIdxNew ) ;

               // set the wait info in dpsTxExectr
               dpsTxExectr->setWaiterInfo( lrbIdxNew, DPS_QUE_WAITER ) ;

               // mark the new LRB is used
               bFreeLRB = FALSE ; 

               // set return code to SDB_DPS_TRANS_APPEND_TO_WAIT
               rc = SDB_DPS_TRANS_APPEND_TO_WAIT ;
            }
            else
            {
               rc = SDB_DPS_TRANS_LOCK_INCOMPATIBLE ;
            }

            // construct the conflict lock info
            if ( pdpsTxResInfo )
            {
               pdpsTxResInfo->_lockID   = pLRBHdr->lockId ;
               pdpsTxResInfo->_lockType = pLRBIncomp->lockMode ;
               pdpsTxResInfo->_eduID    = pLRBIncomp->dpsTxExectr->getEDUID();
            }

            // job done
            goto done ;

         }
         else
         {
            // 
            // no incompatible found, i.e., compatible with other owners

            // if both upgrade and waiter list are empty add it to owner list
            // else add it to wait list
            if (   ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
                && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx ) ) )
            {
               if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
               {
                  // add the owner list
                  if ( IS_VALID_SEG_OBJ_INDEX( idxToInsert ) )
                  { 
                     _addToOwnerLRBList( idxToInsert, lrbIdxNew ) ;       
                  }
                  else
                  {
                     _addToLRBListTail( pLRBHdr->ownerLRBIdx, lrbIdxNew ) ;   
                  }

                  // add the new LRB to EDU LRB list
                  _addToEDULRBListTail( dpsTxExectr, lrbIdxNew, lockId ) ;

                  // mark the new LRB is used
                  bFreeLRB = FALSE ;
               }

               // job done
               goto done ;
            }
            else
            {
               if ( DPS_TRANSLOCK_OP_MODE_ACQUIRE == opMode )
               {
                  // add to the end of waiter list
                  _addToLRBListTail( pLRBHdr->waiterLRBIdx, lrbIdxNew ) ;

                  // set the wait info in dpsTxExectr
                  dpsTxExectr->setWaiterInfo( lrbIdxNew, DPS_QUE_WAITER ) ;

                  // set return code to SDB_DPS_TRANS_APPEND_TO_WAIT
                  rc = SDB_DPS_TRANS_APPEND_TO_WAIT ;

                  // mark the new LRB is used
                  bFreeLRB = FALSE ;
               }
               else
               {
                  rc = SDB_DPS_TRANS_LOCK_INCOMPATIBLE ;
               }

               // construct the conflict lock info ( representative )
               if ( pdpsTxResInfo )
               {
                  // pick first one from upgrade list or waiter list
                  // as the conflict lock info
                  if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
                  {
                     pLRB = _getLRBPtrByIdx( pLRBHdr->upgradeLRBIdx ) ;
                  }
                  else
                  {
                     pLRB = _getLRBPtrByIdx( pLRBHdr->waiterLRBIdx ) ;
                  }
                  pdpsTxResInfo->_lockID   = pLRBHdr->lockId ;
                  pdpsTxResInfo->_lockType = pLRB->lockMode ;
                  pdpsTxResInfo->_eduID    = pLRB->dpsTxExectr->getEDUID() ;
               }

               // job done
               goto done ;

            }  // if both upgrade and waiter queue/list are empty
         }  // if request lock mode is compatible with other owners
      }  // if in owner list
   done:
      if ( bLatched )
      {
         _releaseOpLatch( bktIdx ) ;
         bLatched = FALSE ;
      }
      if ( bFreeLRB )
      {
         _releaseLRB( lrbIdxNew ) ;
         bFreeLRB = FALSE ;
      }
      if ( bFreeLRBHeader )
      {
         _releaseLRBHdr( hdrIdxNew ) ;
         bFreeLRBHeader = FALSE ;
      }
      return rc;
   error:
      goto done ;
   }


   //
   // Description: acquire and setup a new LRB header and a new LRB
   // Function: acquire a new LRB Header and a new LRB, and initialize these    
   //           two new object with given input parameters.   
   //           the new LRB will be linked to the new LRB Header
   // Input:
   //    eduId           -- edu Id
   //    lockId          -- lock id
   //    requestLockMode -- requested lock mode
   // Output:
   //    hdrIdxNew       -- index of new LRB header
   //    pLRBHdrNew      -- pointer of the new LRB header object
   //    lrbIdxNew       -- index of new LRB
   //    pLRBNew         -- pointer of the new LRB object
   // Return:  SDB_OK or any error returned from _utilSegmentManager::acquire   
   //
   INT32 dpsTransLockManager::_prepareNewLRBAndHeader
   (
      _dpsTransExecutor *        dpsTxExectr,
      const dpsTransLockId     & lockId,
      const DPS_TRANSLOCK_TYPE   requestLockMode,
      UTIL_OBJIDX              & hdrIdxNew,
      dpsTransLRBHeader *      & pLRBHdrNew, 
      UTIL_OBJIDX              & lrbIdxNew,
      dpsTransLRB       *      & pLRBNew
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;

      INT32 rc = SDB_OK ;

      // acquire a free LRB
      rc = _pLRBMgr->acquire( lrbIdxNew, pLRBNew ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to acquire a free LRB (rc=%d)", rc );
         goto error ;
      }
      SDB_ASSERT( pLRBNew,
                  "_prepareNewLRBAndHeader: LRB can't be null" ) ;

      // acuqire a free LRB Header
      rc = _pLRBHdrMgr->acquire( hdrIdxNew, pLRBHdrNew ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR,
                 "Failed to acquire a free LRB Header (rc=%d)", rc );
         goto error ;
      }
      SDB_ASSERT( pLRBHdrNew,
                  "_prepareNewLRBAndHeader: LRB Header can't be null" ) ;

      // initial the new LRB
      // and mark the new LRB Header index in its lrbHdrIdx
      pLRBNew->dpsTxExectr    = dpsTxExectr ;
      pLRBNew->lockMode       = requestLockMode ;
      pLRBNew->refCounter     = 1 ;
      pLRBNew->eduLrbIdxNext  = UTIL_INVALID_OBJ_INDEX ;
      pLRBNew->eduLrbIdxPrev  = UTIL_INVALID_OBJ_INDEX ;
      pLRBNew->lrbHdrIdx      = hdrIdxNew ;
      pLRBNew->nextLRBIdx     = UTIL_INVALID_OBJ_INDEX ;

      // inital the new LRB Header
      // and add the new LRB into the new LRB Header owner list
      pLRBHdrNew->lockId        = lockId ;
      pLRBHdrNew->ownerLRBIdx   = lrbIdxNew ;
      pLRBHdrNew->waiterLRBIdx  = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdrNew->upgradeLRBIdx = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdrNew->nextLRBHdrIdx = UTIL_INVALID_OBJ_INDEX ;

   error :
      return rc ;
   }


   //
   // Description: acquire a lock with given mode
   // Function:    acquire a lock with given mode
   //              . if the request is fulfilled, LRB is added to owner list
   //                and EDU LRB chain ( all locks in same TX ). 
   //              . if the lock is record lock, intent lock on collection
   //                and collection space will be also acquired. 
   //              . if the lock is collection lock, an intention lock on
   //                collection space will be also acquired.
   //              . if lock is not applicable at that time, the LRB will be
   //                added to waiter or upgrade list and wait. 
   //              . while lock waiting it could be either woken up or 
   //                lock waiting time out ( return an error )
   //              . when it is woken up from lock waiting,
   //                it will try to acquire the lock again
   // Input:
   //    dpsTxExectr     -- dpsTxExectr
   //    lockId          -- lock Id
   //    requestLockMode -- lock mode being requested
   // Output:
   //    pdpsTxResInfo   -- pointer to dpsTransRetInfo
   // Return:
   //     SDB_OK,
   //     SDB_PERM,
   //     SDB_DPS_TRANS_APPEND_TO_WAIT,
   //     SDB_INTERRUPT,
   //     SDB_TIMEOUT,
   //     or other errors
   //
   INT32 dpsTransLockManager::acquire
   (
      _dpsTransExecutor        * dpsTxExectr,
      const dpsTransLockId     & lockId,
      const DPS_TRANSLOCK_TYPE   requestLockMode,
      IContext                 * pContext,
      dpsTransRetInfo          * pdpsTxResInfo    
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;

      INT32 rc  = SDB_OK ,
            rc2 = SDB_OK ;
      dpsTransLockId     iLockId ;
      DPS_TRANSLOCK_TYPE iLockMode = DPS_TRANSLOCK_MAX ;
      BOOLEAN isIntentLockAcquired = FALSE ;

      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ;
      BOOLEAN bLatched = FALSE ;

      // get intent lock at first
      // it is not need to get intent lock while lock space
      if ( ! lockId.isRootLevel() )
      {
         iLockId   = lockId.upOneLevel() ;
         iLockMode = dpsIntentLockMode( requestLockMode ) ;
         rc = acquire(dpsTxExectr, iLockId, iLockMode, pContext, pdpsTxResInfo);
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         isIntentLockAcquired = TRUE ;
      }

      // calculate the hash index by lockId 
      bktIdx = _getBucketNo( lockId ) ;

   acquireRetry:
      // acquire the lock
      // _tryAcquireOrTest acquires bucket latch by default unless the input
      // parameter, bLatched, is set to TRUE; and it always releases the latch
      // before returns
      rc = _tryAcquireOrTest( dpsTxExectr, lockId, requestLockMode,
                              DPS_TRANSLOCK_OP_MODE_ACQUIRE,
                              bktIdx,
                              bLatched,
                              pdpsTxResInfo ) ;
      bLatched = FALSE ;
      // if needs to wait
      if ( SDB_DPS_TRANS_APPEND_TO_WAIT == rc )
      {
         rc2 = SDB_OK ;

         // pause the context
         if ( pContext )
         {
            rc2 = pContext->pause() ;
         }

         // wait for the lock
         if ( SDB_OK == rc2 )
         {
            rc = _waitLock( dpsTxExectr ) ;
         }

         // latch before remove it from upgrade or waiter list
         if ( ! bLatched )
         {
            _acquireOpLatch( bktIdx ) ; 
            bLatched = TRUE ;
         }

         // remove from upgrade or waiter list
         UTIL_OBJIDX         hdrIdx  = UTIL_INVALID_OBJ_INDEX ;
         dpsTransLRBHeader * pLRBHdr = NULL ;
         _removeFromUpgradeOrWaitList( dpsTxExectr, hdrIdx, pLRBHdr ) ;

         // when _waitLock fails, either timeout or interrupted 
         // check if we need remove the LRB Header.
         // The reason we don't do this in _removeFromUpgradeOrWaitList
         // is if _waitLock returns success when retry, _tryAcquireOrTest,
         // will add the LRB Header back again. 
         if (    ( SDB_OK != rc ) 
              && bLatched
              && IS_VALID_SEG_OBJ_INDEX( hdrIdx )
              && pLRBHdr
              && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx   ) )
              && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
              && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx  ) ) )
         {
            // remove the LRB Header from the list
            _removeFromLRBHeaderList( _LockHdrBkt[bktIdx].lrbHdrIdx, hdrIdx ) ;
            // release the LRB Header
            _releaseLRBHdr( hdrIdx ) ;
            hdrIdx = UTIL_INVALID_OBJ_INDEX ;
            pLRBHdr = NULL ;
         }

         // resume context if the context has been paused
         if ( ( pContext ) && ( SDB_OK == rc2 ) )
         {
            rc2 = pContext->resume() ;
         }

         // retry acquire the lock if it is woken up
         if ( ( SDB_OK == rc ) && ( SDB_OK == rc2 ) && bLatched )
         {
            // need to hold the latch before retry acquiring the lock
            // to avoid race condition
            goto acquireRetry ;
         }

         // unlatch after remove it from upgrade/waiter list
         if ( bLatched )
         {
            _releaseOpLatch( bktIdx ) ;
            bLatched = FALSE ;
         }

         if ( ( SDB_OK != rc ) || ( SDB_OK != rc2 ) )
         {
            // failed to pause or resume context, or
            // wait lock failed due to timeout or interrupt etc on,
            // shall release the upper level lock
            if ( isIntentLockAcquired )
            {
               release( dpsTxExectr, iLockId, FALSE ) ;
               isIntentLockAcquired = FALSE ;
            }
            // set recode to rc2 when wailLock() fails
            if ( ( SDB_OK != rc2 ) && ( SDB_OK == rc ) )
            {
               rc = rc2 ;
            }
         }
      }
   done:
      if ( bLatched )
      {
         _releaseOpLatch( bktIdx ) ;
         bLatched = FALSE ;
      }
      return rc;

   error:
      if ( isIntentLockAcquired )
      {
         release( dpsTxExectr, iLockId, FALSE ) ;
         isIntentLockAcquired = FALSE ;
      }
      goto done;
   }


   //
   // Description: core logic of release a lock
   // Function: decrease lock reference counter, do following when the counter
   //           comes zero : 
   //           .  remove from the edu ( caller ) LRB chain
   //           .  remove it from owner list
   //           .  wake up a waiter( upgrade or waiter list ) when necessary,
   //           .  remove the LRB Header if it is empty ( owner, waiter,
   //              upgrade list are all empty )
   // Input:
   //    dpsTxExectr     -- dpsTransExecutor
   //    lockId          -- lock id
   //    bForceRelease   -- force release flag
   // Output :
   //    none
   void dpsTransLockManager::_release
   (
      _dpsTransExecutor       * dpsTxExectr,
      const dpsTransLockId    & lockId,
      const BOOLEAN             bForceRelease
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;

      UTIL_OBJIDX hdrIdx         = UTIL_INVALID_OBJ_INDEX ,
                  ownerLrbIdx    = UTIL_INVALID_OBJ_INDEX ,
                  prevOwnerIdx   = UTIL_INVALID_OBJ_INDEX ,
                  lrbHdrToRelase = UTIL_INVALID_OBJ_INDEX ,
                  lrbToRelase    = UTIL_INVALID_OBJ_INDEX ;
      UTIL_OBJIDX bktIdx         = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRB *pOwnerLRB     = NULL ,
                  *pPrevOwnerLRB = NULL ,
                  *pWaiterLRB    = NULL ;
      dpsTransLRBHeader *pLRBHdr = NULL ;
      EDUID eduId = dpsTxExectr->getEDUID() ;
      BOOLEAN bLatched = FALSE ;

      // calculate the hash index by lockId
      bktIdx = _getBucketNo( lockId ) ;

      // latch the bucket 
      _acquireOpLatch( bktIdx ) ;
      bLatched = TRUE ;

      if ( IS_VALID_SEG_OBJ_INDEX( _LockHdrBkt[bktIdx].lrbHdrIdx ) ) 
      {
         hdrIdx = _LockHdrBkt[bktIdx].lrbHdrIdx ;

         // lookup LRB Header list to find the LRB Header with same lockId
         if ( _getLRBHdrByLockId( lockId, hdrIdx, pLRBHdr ) )
         {
            // lookup owner list to find the LRB with same eduid
            ownerLrbIdx = pLRBHdr->ownerLRBIdx ;
            pOwnerLRB   = NULL ;
            if ( _getLRBByEDUId( eduId,
                                 ownerLrbIdx,  pOwnerLRB,
                                 prevOwnerIdx, pPrevOwnerLRB ) ) 
            {
               if ( bForceRelease )
               {
                  pOwnerLRB->refCounter = 0 ; 
               }
               else
               {
                  pOwnerLRB->refCounter -- ; 
               }
               if ( 0 == pOwnerLRB->refCounter )
               {
                  // remove it from dpsTxExectr list
                  _removeFromEDULRBList( dpsTxExectr, ownerLrbIdx, lockId ) ;

                  // remove it from lock owner list
                  if ( pLRBHdr->ownerLRBIdx == ownerLrbIdx )
                  {
                     pLRBHdr->ownerLRBIdx = pOwnerLRB->nextLRBIdx ;
                  }
                  else if (    IS_VALID_SEG_OBJ_INDEX( prevOwnerIdx )
                            && pPrevOwnerLRB )
                  {
                     pPrevOwnerLRB->nextLRBIdx = pOwnerLRB->nextLRBIdx ;
                  }

                  // last the owner in owner list will try to wake up
                  // the first one in upgrade/waiter queue if it is compatible
                  // with the previous owner

                  if (   IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx )
                      && ( ! IS_VALID_SEG_OBJ_INDEX( pOwnerLRB->nextLRBIdx ) ) )
                  {
                     // upgrade list is not empty and the owner is the last one

                     pWaiterLRB = _getLRBPtrByIdx( pLRBHdr->upgradeLRBIdx ) ;
                     if ( IS_VALID_SEG_OBJ_INDEX( prevOwnerIdx ) &&
                          pPrevOwnerLRB )
                     {
                        // exists a LRB previous to current owner LRB
                        if ( dpsIsLockCompatible( pPrevOwnerLRB->lockMode,
                                                  pWaiterLRB->lockMode ) )
                        {
                           // waiter lock mode is compatible with
                           // the one previous to current owner, wake it up

                           // wake up the edu by posting an event
                           _wakeUp( pWaiterLRB->dpsTxExectr ) ;
                        }
                     }
                     else 
                     {
                        // the current owner is the only one in owner list

                        // wake up the edu by posting an event
                        _wakeUp( pWaiterLRB->dpsTxExectr ) ;
                     }
                  }
                  else if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx ) &&
                            ( ! IS_VALID_SEG_OBJ_INDEX( pOwnerLRB->nextLRBIdx)))
                  {
                     // waiter list is not empty and the owner is the last one
                     pWaiterLRB = _getLRBPtrByIdx( pLRBHdr->waiterLRBIdx ) ;
                     if ( IS_VALID_SEG_OBJ_INDEX( prevOwnerIdx ) &&
                          pPrevOwnerLRB )
                     {
                        // exists a LRB previous to current owner LRB
                        if ( dpsIsLockCompatible( pPrevOwnerLRB->lockMode,
                                                  pWaiterLRB->lockMode ) )
                        {
                           // waiter lock mode is compatible with
                           // the one previous to current owner, wake it up

                           // wake up the edu by posting an event
                           _wakeUp( pWaiterLRB->dpsTxExectr ) ;
                        }
                     }
                     else
                     {
                        // the current owner is the only one in owner list,
                        // wake it up

                        // wake up the edu by posting an event
                        _wakeUp( pWaiterLRB->dpsTxExectr ) ;
                     }
                  }

                  // save the index of owner LRB to be released
                  lrbToRelase = ownerLrbIdx ; 

                  // remove the LRB Header
                  if (   ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx   ) )
                      && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
                      && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx  ) ))
                  {
                     _removeFromLRBHeaderList( _LockHdrBkt[bktIdx].lrbHdrIdx, 
                                               hdrIdx ) ;

                     // save index of LRB Header to be released
                     lrbHdrToRelase = hdrIdx ;
                  }
               }
            }
         }
      }

      // release the bucket latch
      if ( bLatched )
      {
         _releaseOpLatch( bktIdx ) ;
         bLatched = FALSE ;
      }
      if ( IS_VALID_SEG_OBJ_INDEX( lrbToRelase ) )
      {
         _releaseLRB( lrbToRelase ) ;
         lrbToRelase = UTIL_INVALID_OBJ_INDEX ;
      }
      if ( IS_VALID_SEG_OBJ_INDEX( lrbHdrToRelase ) )
      {
         _releaseLRBHdr( lrbHdrToRelase ) ;
         lrbHdrToRelase = UTIL_INVALID_OBJ_INDEX ; 
      }
      return ;
   }


   //
   // Description: release a lock
   // Function: release a lock, remove from the edu ( caller ) LRB chain
   //           and remove it from owner list if the refference counter is zero.
   //           Wake up a waiter when necessary, it will also release the upper
   //           level intent lock
   // Input:
   //    eduId           -- edu Id
   //    lockId          -- lock id
   //    bForceRelease   -- requested lock mode
   //
   void dpsTransLockManager::release
   (
      _dpsTransExecutor    * dpsTxExectr,
      const dpsTransLockId & lockId,
      const BOOLEAN          bForceRelease
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;

      dpsTransLockId iLockId ;

      // main logic of release by lockId
      _release( dpsTxExectr, lockId, bForceRelease ) ;

      // release the intent lock
      if ( ! lockId.isRootLevel() )
      {
         iLockId = lockId.upOneLevel() ;
         release( dpsTxExectr, iLockId, bForceRelease ) ;
      }
      return ;
   }


   //
   // Description: an EDU release a lock with force mode. 
   // Function: core function/logic of releaseAll, force release a lock,
   //           . remove from the edu ( caller ) LRB chain
   //           . remove it from owner list
   //           . wake up a waiter when necessary
   //           . release the upper level intent lock
   // Input:
   //    eduId   -- edu Id
   //    lockId  -- lock id
   //    hdrIdx  -- LRB Header index ( optional )
   //    pLRBHdr -- LRB Header pointer ( optional )
   //
   void dpsTransLockManager::_releaseAll
   (
      _dpsTransExecutor    * dpsTxExectr,
      const dpsTransLockId & lockId
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;

      dpsTransLockId iLockId;
 
      if ( lockId.isValid() )
      { 
         // main logic of release by lockId, force release mode
         _release( dpsTxExectr, lockId, TRUE ) ;

         // release the intent lock
         if ( ! lockId.isRootLevel() )
         {
            iLockId = lockId.upOneLevel() ;
            _releaseAll( dpsTxExectr, iLockId ) ;
         }
      }
      return ;
   }


   //
   // Description: Force release all locks an EDU is holding 
   // Function: walk though the EDU LRB chain, force release all locks
   //           an EDU is holding.
   //           . remove them from the edu ( caller ) LRB chain
   //           . remove it from owner list
   //           . wake up a waiter when necessary,
   //           . release the upper level intent lock
   // Input:
   //    eduId           -- edu Id
   //    lockId          -- lock id
   //    bForceRelease   -- requested lock mode
   //
   void dpsTransLockManager::releaseAll( _dpsTransExecutor *dpsTxExectr )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;

      if (   ( dpsTxExectr )
          && IS_VALID_SEG_OBJ_INDEX( dpsTxExectr->getLastLRBIdx() ) )
      {
         dpsTransLRB       *pLRB ;
         dpsTransLRBHeader *pLRBHdr ;
         UTIL_OBJIDX hdrIdx, lrbIdx = dpsTxExectr->getLastLRBIdx() ;

         while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
         {
            pLRB = _getLRBPtrByIdx( lrbIdx ) ;

            // peek LRB Header index
            hdrIdx = pLRB->lrbHdrIdx ;

            // get LRB Header
            if ( IS_VALID_SEG_OBJ_INDEX( hdrIdx ) )
            {
               pLRBHdr = getLRBHdrPtrByIdx( hdrIdx ) ;

               // release the lock 
               _releaseAll( dpsTxExectr, pLRBHdr->lockId ) ;
            }

            // _releaseAll will remove LRB from EDU list
            // by calling  _removeFromEDULRBList(),
            // where the dpsTxExectr->_lastLRBInList will be updated,
            // so we may use dpsTxExectr->getLastLRBIdx() to move to
            // previous LRB in the EDU LRB chain.

            // move to next lock to be released
            // we can't use lrbIdx = lrbIdx->eduLrbIdxPrev to move to next
            // LRB as _releaseAll() will remove upper level intent lock as well,
            // which may advance several LRBs ( CL, CS ).
            lrbIdx = dpsTxExectr->getLastLRBIdx() ;
         }
      }
      return ;
   }


   //
   // Description: calculate the index to LRB Header bucket
   // Function: get the index number to the LRB Header bucket by hashing lockId
   //           
   // Input:
   //    lockId -- lock id
   // Return:
   //        index to the LRB Header bucket
   //
   UTIL_OBJIDX dpsTransLockManager::_getBucketNo( const dpsTransLockId &lockId )
   {
      return (UTIL_OBJIDX)( lockId.lockIdHash() % MAX_LOCKBUCKET_NUM );
   }


   //
   // Description: Wakeup a lock waiting EDU
   // Function: wake up an EDU by post an event
   //
   // Input:
   //    dpsTxExectr -- pointer to _dpsTransExecutor
   //
   void dpsTransLockManager::_wakeUp( _dpsTransExecutor *dpsTxExectr )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTransExecutor can't be NULL" ) ;
      if ( dpsTxExectr )
      {
         dpsTxExectr->wakeup() ;
      }
   }


   //
   // Description: Wait a lock
   // Function: wait a lock by waiting on an event
   //
   // Input:
   //    dpsTxExectr -- pointer to _dpsTransExecutor
   //
   INT32 dpsTransLockManager::_waitLock( _dpsTransExecutor *dpsTxExectr )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( dpsTxExectr, "dpsTransExecutor can't be NULL" ) ;
      if ( dpsTxExectr )
      {     
         rc = dpsTxExectr->wait( _lockTimeout.fetch() ) ;
      }
      else
      {
         rc = SDB_SYS ;
      }
      return rc ;
   }


   //
   // Description: try to acquire a lock with given mode
   // Function:    try to acquire a lock with given mode
   //              . if the request is fulfilled, LRB is added to owner list
   //                and EDU LRB chain ( all locks in same TX ).
   //              . if the lock is record lock, intent lock on collection
   //                and collection space will be also acquired.
   //              . if the lock is collection lock, an intention lock on
   //                collection space will be also acquired.
   //              . if lock is not applicable at that time,
   //                the LRB will NOT be put into waiter / upgrade list,
   //                SDB_DPS_TRANS_LOCK_INCOMPATIBLE will be returned. 
   // Input:
   //    dpsTxExectr     -- dpsTxExectr
   //    lockId          -- lock Id
   //    requestLockMode -- lock mode being requested
   // Output:
   //    pdpsTxResInfo   -- pointer to dpsTransRetInfo
   // Return:
   //     SDB_OK,
   //     SDB_PERM,
   //     SDB_DPS_TRANS_LOCK_INCOMPATIBLE,
   //     or other errors
   //
   INT32 dpsTransLockManager::tryAcquire
   (
      _dpsTransExecutor        * dpsTxExectr,
      const dpsTransLockId     & lockId,
      const DPS_TRANSLOCK_TYPE   requestLockMode,
      dpsTransRetInfo          * pdpsTxResInfo
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;

      INT32 rc = SDB_OK ;
      dpsTransLockId iLockId;
      DPS_TRANSLOCK_TYPE iLockMode = DPS_TRANSLOCK_MAX ;
      BOOLEAN isIntentLockAcquired = FALSE;
      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ;

      // get intent lock at first
      // it is not need to get intent lock while lock space
      if ( ! lockId.isRootLevel() )
      {
         iLockId = lockId.upOneLevel() ;
         iLockMode = dpsIntentLockMode( requestLockMode ) ;
         rc = tryAcquire( dpsTxExectr, iLockId, iLockMode, pdpsTxResInfo );
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         isIntentLockAcquired = TRUE;
      }

      // calculate the hash index by lockId
      bktIdx = _getBucketNo( lockId ) ;

      // try to acquire the lock
      // when tryAcquire will not add LRB to either upgrade or waiter list
      rc = _tryAcquireOrTest( dpsTxExectr, lockId, requestLockMode,
                              DPS_TRANSLOCK_OP_MODE_TRY,
                              bktIdx,
                              FALSE,
                              pdpsTxResInfo ) ;
   done:
      return rc;
   error:
      if ( isIntentLockAcquired )
      {
         release( dpsTxExectr, iLockId, FALSE ) ;
         isIntentLockAcquired = FALSE ;
      }
      goto done;
   }


   //
   // Description: test whether a lock can be acquired with given mode
   // Function:    test whether a lock can be acquired with given mode
   //              . if the request can be fulfilled, returns SDB_OK,
   //                the request will not be added to either owner list
   //                or EDU LRB chain, as it doesn't really acquire the lock.
   //              . if lock is not applicable at that time,
   //                SDB_DPS_TRANS_LOCK_INCOMPATIBLE will be returned.
   // Input:
   //    dpsTxExectr     -- dpsTxExectr
   //    lockId          -- lock Id
   //    requestLockMode -- lock mode being requested
   // Output:
   //    pdpsTxResInfo   -- pointer to dpsTransRetInfo
   // Return:
   //     SDB_OK,
   //     SDB_PERM,
   //     SDB_DPS_TRANS_LOCK_INCOMPATIBLE,
   //     or other errors
   //
   INT32 dpsTransLockManager::testAcquire
   (
      _dpsTransExecutor        * dpsTxExectr,
      const dpsTransLockId     & lockId,
      const DPS_TRANSLOCK_TYPE   requestLockMode,
      dpsTransRetInfo          * pdpsTxResInfo
   )
   {
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;

      INT32 rc = SDB_OK;
      dpsTransLockId iLockId;
      DPS_TRANSLOCK_TYPE iLockMode = DPS_TRANSLOCK_MAX ;
      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ;

      // get intent lock at first
      // it is not need to get intent lock while lock space
      if ( ! lockId.isRootLevel() )
      {
         iLockId = lockId.upOneLevel() ;
         iLockMode = dpsIntentLockMode( requestLockMode ) ;
         rc = testAcquire( dpsTxExectr, iLockId, iLockMode, pdpsTxResInfo);
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }

      // calculate the hash index by lockId 
      bktIdx = _getBucketNo( lockId ) ;

      // test if the request lock mode can be acquired
      // it will not acquire the lock, the LRB will not be added to
      // owner, upgrade or waiter list
      rc = _tryAcquireOrTest( dpsTxExectr, lockId, requestLockMode,
                              DPS_TRANSLOCK_OP_MODE_TEST,
                              bktIdx,
                              FALSE,
                              pdpsTxResInfo ) ;
   done:
      return rc;
   error:
      goto done;
   }


   //
   // Description: whether a lock is being waited
   // Function:    test whether a lock is being waited by checking if
   //              the waiter list and upgrade list are empty.
   // Input:
   //    lockId          -- lock Id
   // Return:
   //    True  -- the lock has waiter(s)
   //    False -- the lock has no waiter(s)
   //
   BOOLEAN dpsTransLockManager::hasWait( const dpsTransLockId &lockId )
   {
      BOOLEAN result = FALSE;
      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ;
      UTIL_OBJIDX hdrIdx = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRBHeader *pLRBHdr = NULL ;

      // calculate the hash index by lockId
      bktIdx = _getBucketNo( lockId ) ;

      // latch the LRB Header list
      _acquireOpLatch( bktIdx ) ;

      hdrIdx  = _LockHdrBkt[bktIdx].lrbHdrIdx ;
      if ( _getLRBHdrByLockId( lockId, hdrIdx, pLRBHdr ) )
      {
         SDB_ASSERT( pLRBHdr && IS_VALID_SEG_OBJ_INDEX( hdrIdx ),
                     "Invalid LRB Header" ) ;
         if (    pLRBHdr 
              && IS_VALID_SEG_OBJ_INDEX( hdrIdx )
              && (   IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx  ) 
                  || IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) ) )
         {
            result = TRUE ;
         } 
      }

      // free LRB Header list latch
      _releaseOpLatch( bktIdx ) ;

      return result ;
   }


   #define DPS_STRING_LEN_MAX ( 256 )
   //
   // format LRB to string, flat one line
   //
   CHAR * dpsTransLockManager::_LRBToString 
   (
      const UTIL_OBJIDX idx,
      CHAR * pBuf,
      UINT32 bufSz 
   )
   {
      if ( IS_VALID_SEG_OBJ_INDEX( idx )  )
      {
         dpsTransLRB *pLRB = _getLRBPtrByIdx( idx ) ;
         ossSnprintf( pBuf, bufSz,
                      "LRB: %u, dpsTxExectr: %p, "
                      "eduLrbIdxNext: %u, eduLrbIdxPrev: %u, "
                      "lrbHdrIdx: %u, nextLRBIdx: %u "
                      "refCounter: %llu, lockMode: %s",
                      idx, pLRB->dpsTxExectr,
                      pLRB->eduLrbIdxNext, pLRB->eduLrbIdxPrev,
                      pLRB->lrbHdrIdx, pLRB->nextLRBIdx,
                      pLRB->refCounter, lockModeToString( pLRB->lockMode ) ) ;
      }
      return pBuf ;
   }



   //
   // format LRB to string, each field/member per line, with optional prefix
   //
   CHAR * dpsTransLockManager::_LRBToString 
   (
      const UTIL_OBJIDX idx,
      CHAR * pBuf,
      UINT32 bufSz, 
      CHAR * prefix
   )
   {
      CHAR * pBuff = pBuf;
      CHAR * pDummy= "" ;
      CHAR * pStr  = ( prefix ? prefix : pDummy ) ;
      if ( IS_VALID_SEG_OBJ_INDEX( idx )  )
      {
         dpsTransLRB *pLRB = _getLRBPtrByIdx( idx ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%sLRB          : %u"OSS_NEWLINE, pStr,
                               idx ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%sdpsTxExectr  : %p"OSS_NEWLINE, pStr,
                               pLRB->dpsTxExectr ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%seduLrbIdxNext: %u"OSS_NEWLINE, pStr,
                               pLRB->eduLrbIdxNext ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%seduLrbIdxPrev: %u"OSS_NEWLINE, pStr,
                               pLRB->eduLrbIdxPrev ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%slrbHdrIdx    : %u"OSS_NEWLINE, pStr,
                               pLRB->lrbHdrIdx ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%snextLRBIdx   : %u"OSS_NEWLINE, pStr,
                               pLRB->nextLRBIdx ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%srefCounter   : %llu"OSS_NEWLINE, pStr,
                               pLRB->refCounter ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%slockMode     : %s"OSS_NEWLINE, pStr,
                               lockModeToString( pLRB->lockMode ) ) ;
      }
      return pBuf ;
   }


   //
   // format LRB Header to flat one line string
   //
   CHAR * dpsTransLockManager::_LRBHdrToString
   (
      const UTIL_OBJIDX idx,
      CHAR * pBuf,
      UINT32 bufSz 
   )
   {
      if ( IS_VALID_SEG_OBJ_INDEX( idx )  )
      {
         dpsTransLRBHeader *pLRBHdr = getLRBHdrPtrByIdx( idx ) ;
         ossSnprintf( pBuf, bufSz,
                      "LRB Header: %u, nextLRBHdrIdx: %u, "
                      "ownerLRBIdx: %u, waiterLRBIdx : %u, upgradeLRBIdx: %u, "
                      "lockId: ( %s )",
                      idx,
                      pLRBHdr->nextLRBHdrIdx,
                      pLRBHdr->ownerLRBIdx,
                      pLRBHdr->waiterLRBIdx,               
                      pLRBHdr->upgradeLRBIdx,
                      pLRBHdr->lockId.toString().c_str() ) ;
      }
      return pBuf;
   }


   //
   // format LRB Header to string, one field/member per line, w/ optional prefix
   //
   CHAR * dpsTransLockManager::_LRBHdrToString 
   (
      const UTIL_OBJIDX idx,
      CHAR * pBuf,
      UINT32 bufSz, 
      CHAR * prefix
   )
   {
      CHAR * pBuff = pBuf;
      CHAR * pDummy= "" ;
      CHAR * pStr  = ( prefix ? prefix : pDummy ) ;
      if ( IS_VALID_SEG_OBJ_INDEX( idx ) )
      {
         dpsTransLRBHeader *pLRBHdr = getLRBHdrPtrByIdx( idx ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%sLRB Header    : %u"OSS_NEWLINE, pStr,
                               idx ) ;
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%snextLRBHdrIdx : %u"OSS_NEWLINE, pStr,
                               pLRBHdr->nextLRBHdrIdx);
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%sownerLRBIdx   : %u"OSS_NEWLINE, pStr,
                               pLRBHdr->ownerLRBIdx );
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%swaiterLRBIdx  : %u"OSS_NEWLINE, pStr,
                               pLRBHdr->waiterLRBIdx );
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%supgradeLRBIdx : %u"OSS_NEWLINE, pStr,
                               pLRBHdr->upgradeLRBIdx );
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%slockId        : ( %s )"OSS_NEWLINE, pStr,
                               pLRBHdr->lockId.toString().c_str() ) ;
      }
      return pBuf;
   }


   //
   // dump all LRB in EDU LRB chain ( to stdout ), for debug purpose 
   // the caller shall acquire the monitoring( dump ) latch, acquireMonLatch(),
   // and make sure the executor is still available
   //
   void dpsTransLockManager::dumpLockInfo
   (
      _dpsTransExecutor * dpsTxExectr,
      const CHAR        * fileName,
      BOOLEAN bOutputInPlainMode
   )
   {
      UTIL_OBJIDX lrbIdx = UTIL_INVALID_OBJ_INDEX;
      dpsTransLRB *pLRB  = NULL ;
      CHAR * pStr = NULL ;
      CHAR * prefixStr = (CHAR*)"   " ; 
      CHAR szBuffer[ DPS_STRING_LEN_MAX ] = { '\0' } ;
      FILE * fp   = NULL;

      if ( dpsTxExectr )
      {
         // open output file
         if ( NULL == ( fp = fopen( fileName, "ab+" ) ) )
         {
            goto error ;
         }
         
         lrbIdx = dpsTxExectr->getLastLRBIdx() ;
         while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
         {
            pLRB    = _getLRBPtrByIdx( lrbIdx ) ;

            SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( pLRB->lrbHdrIdx ),
                        "Invalid LRB Header index." ) ;

            if ( bOutputInPlainMode )
            {
               pStr = (CHAR*) _LRBToString( lrbIdx, szBuffer,
                                            sizeof(szBuffer) ) ;
            }
            else
            {
               pStr = (CHAR*) _LRBToString( lrbIdx, szBuffer,
                                            sizeof(szBuffer), prefixStr ) ;
            }
            fprintf( fp, "%s"OSS_NEWLINE, pStr ) ;
            lrbIdx = pLRB->eduLrbIdxPrev ;
         }

         // close output file
         if ( fp )
         {
            fclose( fp ) ;
         }
      }
   error:
      return ;
   }


   //
   // dump LRB Header, owner/waiter/upgrade list info to stdout,
   // for debug purpose only
   //
   void dpsTransLockManager::dumpLockInfo
   (
      const dpsTransLockId & lockId,
      const CHAR           * fileName,
      BOOLEAN                bOutputInPlainMode
   )
   {
      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX,
                  hdrIdx = UTIL_INVALID_OBJ_INDEX,
                  lrbIdx = UTIL_INVALID_OBJ_INDEX;
      dpsTransLockId iLockId;
      dpsTransLRBHeader *pLRBHdr = NULL ;
      dpsTransLRB       *pLRB    = NULL ;
      CHAR * pStr = NULL ;
      CHAR * prefixStr = (CHAR*)"   " ; 
      CHAR szBuffer[ DPS_STRING_LEN_MAX ] = { '\0' } ;

      FILE * fp = NULL ;


      // dump intent lock at first
      if ( ! lockId.isRootLevel() )
      {
         iLockId = lockId.upOneLevel() ;
         dumpLockInfo( iLockId, fileName, bOutputInPlainMode );
      }

      // open output file
      if ( NULL == ( fp = fopen( fileName, "ab+" ) ) )
      {
         goto error ;
      }

      // calculate the hash index by lockId
      bktIdx = _getBucketNo( lockId ) ;

      // latch the bucket
      _acquireOpLatch( bktIdx ) ; 

      hdrIdx  = _LockHdrBkt[bktIdx].lrbHdrIdx ;
      if ( _getLRBHdrByLockId( lockId, hdrIdx, pLRBHdr ) )
      {
         fprintf( fp, "%s",  "LRB Header " ) ;
         if ( lockId.isRootLevel() ) 
         {
            pStr = "( Container Space Lock )" ;
         }
         else if ( lockId.isLeafLevel() )
         {
            pStr = "( Record Lock )" ;
         }
         else
         {
            pStr = "( Container Lock )" ;
         }

         fprintf( fp, " %s"OSS_NEWLINE, pStr ) ;
         fprintf( fp, "%s", "-------------------------------"OSS_NEWLINE ) ;
         if ( bOutputInPlainMode )
         {
            pStr = (CHAR*) _LRBHdrToString( hdrIdx, szBuffer, sizeof(szBuffer));
         }
         else
         {
            pStr = (CHAR*) _LRBHdrToString( hdrIdx, szBuffer, sizeof(szBuffer),
                                            NULL );
         }
         fprintf( fp, "%s"OSS_NEWLINE OSS_NEWLINE, pStr ) ;
         if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx ) ) 
         {
            if ( bOutputInPlainMode )
            {
               fprintf( fp, "%s", "Owner list:"OSS_NEWLINE ) ;
               fprintf( fp, "%s", "-----------"OSS_NEWLINE ) ;
            }
            else
            {
               fprintf( fp, "%sOwner list:"OSS_NEWLINE, prefixStr ) ;
               fprintf( fp, "%s-----------"OSS_NEWLINE, prefixStr ) ;
            }
            lrbIdx = pLRBHdr->ownerLRBIdx ;
            while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
            {
               pLRB = _getLRBPtrByIdx( lrbIdx ) ;
               if ( bOutputInPlainMode )
               {
                  pStr = (CHAR*) _LRBToString( lrbIdx, szBuffer,
                                               sizeof(szBuffer) ) ;
               }
               else
               {
                  pStr = (CHAR*) _LRBToString( lrbIdx, szBuffer,
                                               sizeof(szBuffer), prefixStr ) ;
               }
               fprintf( fp, "%s"OSS_NEWLINE, pStr ) ;
               lrbIdx = pLRB->nextLRBIdx ;
            } 
            fprintf( fp, "%s", OSS_NEWLINE ) ;
         }
         if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
         {
            fprintf( fp, "%sUpgrade list:"OSS_NEWLINE, prefixStr ) ;
            fprintf( fp, "%s-------------"OSS_NEWLINE, prefixStr ) ;
            lrbIdx = pLRBHdr->upgradeLRBIdx ;
            while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
            {
               pLRB = _getLRBPtrByIdx( lrbIdx ) ;
               if ( bOutputInPlainMode )
               {
                  pStr = (CHAR*) _LRBToString( lrbIdx, szBuffer,
                                               sizeof(szBuffer) ) ;
               }
               else
               {
                  pStr = (CHAR*) _LRBToString( lrbIdx, szBuffer,
                                               sizeof(szBuffer), prefixStr );
               }
               fprintf( fp, "%s"OSS_NEWLINE, pStr ) ;
               lrbIdx = pLRB->nextLRBIdx ;
            }
            fprintf( fp, "%s", OSS_NEWLINE ) ;
         }
         if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx ) )
         {
            fprintf( fp, "%sWaiter list:"OSS_NEWLINE, prefixStr ) ;
            fprintf( fp, "%s------------"OSS_NEWLINE, prefixStr ) ;
            lrbIdx = pLRBHdr->waiterLRBIdx ;
            while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
            {
               pLRB = _getLRBPtrByIdx( lrbIdx ) ;
               if ( bOutputInPlainMode )
               {
                  pStr = (CHAR*) _LRBToString( lrbIdx, szBuffer,
                                               sizeof(szBuffer) ) ;
               }
               else
               {
                  pStr = (CHAR*) _LRBToString( lrbIdx, szBuffer,
                                               sizeof(szBuffer), prefixStr );
               }
               fprintf( fp, "%s"OSS_NEWLINE, pStr ) ;
               lrbIdx = pLRB->nextLRBIdx ;
            }
            fprintf( fp, "%s", OSS_NEWLINE ) ;
         }
         fprintf( fp, "%s", OSS_NEWLINE ) ;
      }

      // free bucket latch
      _releaseOpLatch( bktIdx ) ;

      // close file
      if ( fp ) 
      {
         fclose( fp ) ;
      }
   error:
      return ;
   }


   //
   // dump EDU LRB info
   // the caller shall acquire the monitoring( dump ) latch, acquireMonLatch(),
   // and make sure the executor is still available
   //
   void dpsTransLockManager::dumpLockInfo
   (
      UTIL_OBJIDX         lastLRBIdx,
      VEC_TRANSLOCKCUR  & vecLocks
   )
   {
      monTransLockCur monLock ;
      UTIL_OBJIDX hdrIdx = UTIL_INVALID_OBJ_INDEX ;
      UTIL_OBJIDX lrbIdx = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRBHeader *pLRBHdr = NULL ;
      dpsTransLRB       *pLRB    = NULL ;

      lrbIdx = lastLRBIdx ;
      while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
      {
         pLRB    = _getLRBPtrByIdx( lrbIdx ) ;
         hdrIdx  = pLRB->lrbHdrIdx ;

         SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( hdrIdx ),
                     "Invalid LRB Header index." ) ;

         pLRBHdr = getLRBHdrPtrByIdx( hdrIdx ) ;

         monLock._id    = pLRBHdr->lockId ;
         monLock._mode  = pLRB->lockMode ;
         monLock._count = pLRB->refCounter ;

         vecLocks.push_back( monLock ) ;

         lrbIdx = pLRB->eduLrbIdxPrev ;
      }
   }


   void dpsTransLockManager::dumpLockInfo
   (
      UTIL_OBJIDX       lrbIdx,
      monTransLockCur  &lockCur
   )
   {
      dpsTransLRBHeader *pLRBHdr = NULL ;
      dpsTransLRB       *pLRB    = NULL ;
      UTIL_OBJIDX hdrIdx = UTIL_INVALID_OBJ_INDEX ;

      if ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
      {
         pLRB    = _getLRBPtrByIdx( lrbIdx ) ;
         hdrIdx  = pLRB->lrbHdrIdx ;

         SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( hdrIdx ),
                     "Invalid LRB Header index." ) ;

         pLRBHdr = getLRBHdrPtrByIdx( hdrIdx ) ;

         lockCur._id = pLRBHdr->lockId ;
         lockCur._mode = pLRB->lockMode ;
         lockCur._count = pLRB->refCounter ;
      }
   }


   //
   // dump LRB Header and owner / waiter / upgrade list for a specific lock
   //
   void dpsTransLockManager::dumpLockInfo
   (
      const dpsTransLockId & lockId,
      monTransLockInfo     & monLockInfo
   )
   {
      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ,
                  hdrIdx = UTIL_INVALID_OBJ_INDEX ,
                  lrbIdx = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRBHeader *pLRBHdr = NULL ;
      dpsTransLRB       *pLRB    = NULL ;

      monTransLockInfo::lockItem monLockItem ;

      // calculate the hash index by lockId
      bktIdx = _getBucketNo( lockId ) ;

      // acquire bucket latch
      _acquireOpLatch( bktIdx ) ;

      hdrIdx  = _LockHdrBkt[bktIdx].lrbHdrIdx ;
      if ( _getLRBHdrByLockId( lockId, hdrIdx, pLRBHdr ) )
      {
         monLockInfo._id = pLRBHdr->lockId ;

         // owner list
         if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx ) )
         {
            lrbIdx = pLRBHdr->ownerLRBIdx ;
            while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
            {
               pLRB = _getLRBPtrByIdx( lrbIdx ) ;

               monLockItem._eduID = pLRB->dpsTxExectr->getEDUID() ;
               monLockItem._mode  = pLRB->lockMode ;
               monLockItem._count = pLRB->refCounter ;
               monLockInfo._vecHolder.push_back( monLockItem ) ;

               lrbIdx = pLRB->nextLRBIdx ;
            }
         }
         // upgrade list
         if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
         {
            lrbIdx = pLRBHdr->upgradeLRBIdx ;
            while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
            {
               pLRB = _getLRBPtrByIdx( lrbIdx ) ;

               monLockItem._eduID = pLRB->dpsTxExectr->getEDUID() ;
               monLockItem._mode  = pLRB->lockMode ;
               monLockItem._count = pLRB->refCounter ;
               monLockInfo._vecWaiter.push_back( monLockItem ) ;

               lrbIdx = pLRB->nextLRBIdx ;
            }
         }
         // waiter list
         if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx ) )
         {
            lrbIdx = pLRBHdr->waiterLRBIdx ;
            while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
            {
               pLRB = _getLRBPtrByIdx( lrbIdx ) ;

               monLockItem._eduID = pLRB->dpsTxExectr->getEDUID() ;
               monLockItem._mode  = pLRB->lockMode ;
               monLockItem._count = pLRB->refCounter ;
               monLockInfo._vecWaiter.push_back( monLockItem ) ;

               lrbIdx = pLRB->nextLRBIdx ;
            }
         }
      }

      // release bucket latch
      _releaseOpLatch( bktIdx ) ;
   }


}  // namespace engine
