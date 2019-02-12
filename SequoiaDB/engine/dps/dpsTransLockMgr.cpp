/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   GNU Affero General Public License for more details.

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
#include "dpsTransLockCallback.hpp"
#include "dpsTransDef.hpp"
#include "dpsTrace.hpp"
#include "pd.hpp"
#include "dpsTrace.hpp"
#include "pdTrace.hpp"
#include "sdbInterface.hpp"   // IContext
#include "pmd.hpp"            // pmdGetOptionCB
#include <stdio.h>

#if 0
//#ifdef _DEBUG    // once we upgrade the boost, we might want to have this
#include <boost/stacktrace.hpp>
#endif

namespace engine
{
   #define DPS_LOCKID_STRING_MAX_SIZE ( 128 ) 
   dpsTransLockManager::dpsTransLockManager() : _pLRBMgr( NULL ),
                                                _pLRBHdrMgr( NULL ),
                                                _initialized( FALSE )
   {
   }

   dpsTransLockManager::~dpsTransLockManager() 
   {
      if ( _initialized )
      {
         fini() ;
      }
   }


   //
   // Description: free allocated LRBs, LRB Headers and reset buckets
   // Input:       none
   // Output:      none
   // Return:      none
   // Dependency:  this function is called during system shutdown,  
   //              the caller shall make sure no threads are accessing locking
   //              resource.
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


   //
   // Description: Initialize lock manager
   //              . initialize bucket
   //              . allocate LRB Headers
   //              . allocate LRBs
   // Input:       none
   // Output:      none
   // Return:      none
   // Dependency:  this function is called during system starting up,
   //              the caller shall make sure no thread is trying to access
   //              lock resource before lock manager is fully initialized
   //
   INT32 dpsTransLockManager::init()
   {
      INT32 rc = SDB_OK;
      UINT32 initLRB  = pmdGetOptionCB()->transLRBPerSegment(),
             totalLRB = pmdGetOptionCB()->transLRBTotal(),
             roundUp  = ossNextPowerOf2( initLRB ) ;
       

      // init lock header bucket
      for ( UINT32 i = 0; i < DPS_TRANS_LOCKBUCKET_SLOTS_MAX; i++ )
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

      if ( totalLRB < roundUp )
      {
         totalLRB = roundUp ;
      }
      PD_LOG( PDEVENT, 
              "Lock manager initializing"OSS_NEWLINE
              "  LRBs per segment        : %u"OSS_NEWLINE
              "  LRBs Max                : %u"OSS_NEWLINE
              "  LRB Headers per segment : %u"OSS_NEWLINE
              "  LRB Headers Max         : %u",
              roundUp, totalLRB, roundUp / 4, totalLRB ) ;

      // init LRB manager
      rc = _pLRBMgr->init( roundUp, totalLRB ) ;
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
      rc = _pLRBHdrMgr->init( roundUp / 4, totalLRB );
      if ( rc )
      {
         PD_LOG( PDERROR,
                 "Failed to allocate memory for LRB Header, rc: %d", rc ) ;
         goto error ;
      }

      // set initialized flag
      _initialized = TRUE ;

   done :
      return rc ;
   error :
      // clean up LRB in error code path
      if ( _pLRBMgr )
      {
         // free LRB objects if allocated
         _pLRBMgr->fini() ;
         // delete LRB manager
         SAFE_OSS_DELETE( _pLRBMgr ) ;
         _pLRBMgr = NULL ;
      }

      // clean LRB Header in error code path
      if ( _pLRBHdrMgr )
      {
         // free LRB Header objects if allocated
         _pLRBHdrMgr->fini() ;
         // delete LRB Header manager
         SAFE_OSS_DELETE( _pLRBHdrMgr ) ;
         _pLRBHdrMgr = NULL ;
      }
      goto done ;
   }


   // 
   // Description: find the LRB Header address/pointer by its index
   // Function:    the LRB Header manager uses the index to find the object
   //              pointer from the object arrays   
   // Input:       the object ( LRB Header ) index to the object arrays
   // Output:      none
   // Return:      the pointer of the object or NULL
   // Dependency:  the lock manager must be initialized
   dpsTransLRBHeader * dpsTransLockManager::_getLRBHdrPtrByIdx
   (
      const UTIL_OBJIDX hdrIdx
   )
   {
#ifdef _DEBUG 
      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( hdrIdx ),
                  "Invalid LRB Header index." ) ;
#endif
      dpsTransLRBHeader * pLRBHdr = _pLRBHdrMgr->getObjPtrByIndex( hdrIdx ) ;
#ifdef _DEBUG 
      SDB_ASSERT( pLRBHdr, "Invalid LRB Header." ) ;
#endif      
      return pLRBHdr ;
   }


   // 
   // Description: find the LRB address/pointer by its index
   // Function:    the LRB manager uses the index to find the object pointer
   //              from the object arrays   
   // Input:       the object ( LRB ) index to the object arrays
   // Return:      the pointer of the object or NULL
   // Dependency:  the lock manager must be initialized
   //
   OSS_INLINE dpsTransLRB * dpsTransLockManager::_getLRBPtrByIdx
   (
      const UTIL_OBJIDX idx
   )
   {
#ifdef _DEBUG 
      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( idx ), "Invalid LRB index." ) ;
#endif      
      dpsTransLRB * pLRB = _pLRBMgr->getObjPtrByIndex( idx );
#ifdef _DEBUG 
      SDB_ASSERT( pLRB, "Invalid LRB." ) ;
#endif     
      return pLRB ;
   }


   // 
   // Description: release/return a LRB Header to LRB Header manager
   // Function:    return a LRB Header to the LRB Header Manager
   // Input:       the object ( LRB Header ) index to the object arrays
   // Output:      none
   // Return:      SDB_OK or SDB_INVALIDARG when error      
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER__RELEASELRBHDR, "dpsTransLockManager::_releaseLRBHdr" )
   OSS_INLINE INT32 dpsTransLockManager::_releaseLRBHdr
   (
      const UTIL_OBJIDX hdrIdx
   )
   {
#ifdef _DEBUG
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER__RELEASELRBHDR ) ;

      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( hdrIdx ),
                  "Invalid LRB Header index." ) ;
#endif
      // reset LRB Header
      dpsTransLRBHeader * pLRBHdr = _getLRBHdrPtrByIdx( hdrIdx ) ;   
      pLRBHdr->nextLRBHdrIdx = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdr->ownerLRBIdx   = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdr->waiterLRBIdx  = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdr->upgradeLRBIdx = UTIL_INVALID_OBJ_INDEX ;
      if ( pLRBHdr->oldVer )
      {
         // debug code to track the guy freeing oldVer
         if ( pLRBHdr->oldVer->getOldRecord() != NULL )
         {
#if 0
            PD_LOG ( PDDEBUG, "Freeing oldver while freeing LRBHdr for (%s)."
                     " Stack is:\n %s",
                     lockId.toString(), boost::stacktrace::stacktrace() );
#endif
            PD_LOG ( PDDEBUG, "Freeing oldver while freeing LRBHdr for (%s).",
                     pLRBHdr->lockId.toString().c_str() );
         }
         SDB_OSS_DEL pLRBHdr->oldVer;
         pLRBHdr->oldVer = NULL;
      }
      pLRBHdr->lockId.reset() ;

      // release LRB Header
      INT32 rc = _pLRBHdrMgr->release( hdrIdx );
#ifdef _DEBUG
      SDB_ASSERT( SDB_OK == rc, "Failed to release LRB Header" ) ;

      PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__RELEASELRBHDR,
                 PD_PACK_STRING( "Release LRB Header:" ),
                 PD_PACK_UINT( hdrIdx ) ) ; 

      PD_TRACE_EXITRC( SDB_DPSTRANSLOCKMANAGER__RELEASELRBHDR, rc ) ;
#endif
      return rc ;
   }


   // 
   // Description: release/return a LRB to LRB manager
   // Function:    return a LRB to the LRB Manager
   // Input:       the object ( LRB ) index to the object arrays
   // Output:      none
   // Return:      SDB_OK or SDB_INVALIDARG when error      
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER__RELEASELRB, "dpsTransLockManager::_releaseLRB" )
   OSS_INLINE INT32 dpsTransLockManager::_releaseLRB( const UTIL_OBJIDX idx )
   {
#ifdef _DEBUG
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER__RELEASELRB ) ;

      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( idx ), "Invalid LRB index." ) ;
#endif
      // reset LRB
      dpsTransLRB * pLRB  = _getLRBPtrByIdx( idx ) ;
      pLRB->dpsTxExectr   = NULL ;
      pLRB->eduLrbIdxNext = UTIL_INVALID_OBJ_INDEX ;
      pLRB->eduLrbIdxPrev = UTIL_INVALID_OBJ_INDEX ;
      pLRB->lrbHdrIdx     = UTIL_INVALID_OBJ_INDEX ;
      pLRB->nextLRBIdx    = UTIL_INVALID_OBJ_INDEX ;
      pLRB->lockMode      = DPS_TRANSLOCK_MAX ; 
      pLRB->refCounter    = 0 ; 
      pLRB->beginTick.clear() ; 
      // release LRB
      INT32 rc = _pLRBMgr->release( idx );
#ifdef _DEBUG
      SDB_ASSERT( SDB_OK == rc, "Failed to release LRB" ) ;

      PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__RELEASELRB,
                 PD_PACK_STRING( "Release LRB:" ),
                 PD_PACK_UINT( idx ) ) ; 

      PD_TRACE_EXITRC( SDB_DPSTRANSLOCKMANAGER__RELEASELRB, rc ) ;
#endif
      return rc ;
   }


   // 
   // Description: search the LRB Header chain and find the one with same lockId
   // Function:    walk through LRB Header list/chain, find the one with same
   //              lockId. 
   // Input: 
   //    lockId   -- lock Id
   //    hdrIdx   -- the first LRB Header object index in the chain
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
   // Dependency:  the lock bucket latch shall be acquired
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
         pLRBHdr = _getLRBHdrPtrByIdx( hdrIdx ) ;
         if ( lockId == pLRBHdr->lockId )
         {
            found = TRUE ;
            break ;
         }
         hdrIdx = pLRBHdr->nextLRBHdrIdx ;
      }
      return found ;
   }


   //
   // Description: search the LRB Header chain and find the one with same lockId
   // Function:    walk through LRB Header list/chain, find the one with same
   //              lockId.  A wrapper function of _getLRBHdrByLockId
   // Input:
   //    lockId   -- lock Id
   // Output:
   //    hdrIdx   -- the index of the first LRB Header object matches
   //                the lockId if it is found.
   //    pLRBHdr  -- the pointer of first LRB Header object matches
   //                the lockId if it is found. If not, it shall be the
   //                pointer of the last LRB Header object in the list
   //
   // Return:     true  -- found the LRB Header object with same lockId
   //             false -- not found
   // Dependency:  the lock bucket latch shall be acquired
   //
   BOOLEAN dpsTransLockManager::getLRBHdrByLockId
   (
      const dpsTransLockId &lockId,
      UTIL_OBJIDX          &hdrIdx,
      dpsTransLRBHeader *  &pLRBHdr
   )
   {
      BOOLEAN found = FALSE ;
      if ( lockId.isValid() )
      {
         UTIL_OBJIDX bktIdx = _getBucketNo( lockId ) ;

         hdrIdx = _LockHdrBkt[ bktIdx ].lrbHdrIdx ;
         found  = _getLRBHdrByLockId( lockId, hdrIdx, pLRBHdr ) ;
         if ( ! found ) 
         {

            hdrIdx  = UTIL_INVALID_OBJ_INDEX ;
            pLRBHdr = NULL ;
         } 
      }
      else
      {
         PD_LOG( PDERROR,
                 "Invalid lockId:%s", lockId.toString().c_str() ) ;
      }
      return found ;
   }


   //
   // Description: search the LRB chain and find the one with given eduid
   // Function:    walk through LRB list/chain, find the LRB with same eduId
   // Input:
   //    eduId    -- EDU Id.
   //    lrbBegin -- the first LRB index in the chain( owner, waiter or upgrade
   //                queue )
   // Output:
   //    idxEduId -- the index of the first LRB object matches given eduId
   //                if it is found.
   //    pLRBEduId-- the pointer of first LRB object matches the eduId
   //                if it is found. If not, it shall be the pointer of
   //                the last LRB object in the list
   //    idxPrev  -- the index of the LRB in the list previous to the owner
   //    pLRBPrev -- the pointer of the LRB in the list previous to the owner
   // Return: 
   //    true  -- found the LRB object with the given eduId
   //    false -- not found
   // Dependency:  the lock bucket latch shall be acquired
   //
   BOOLEAN dpsTransLockManager::_getLRBByEDUId
   (
      const EDUID       eduId,
      const UTIL_OBJIDX lrbBegin,
      UTIL_OBJIDX     & idxEduId,
      dpsTransLRB *   & pLRBEduId,
      UTIL_OBJIDX     & indexPrev,
      dpsTransLRB *   & pLRBPrev
   )
   {
      BOOLEAN found = FALSE ;
      UTIL_OBJIDX idx = lrbBegin,
                  idxPrev = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRB *plrb     = NULL ,
                  *plrbPrev = NULL ;

      idxEduId  = UTIL_INVALID_OBJ_INDEX ; 
      pLRBEduId = NULL ;
      indexPrev = UTIL_INVALID_OBJ_INDEX ;
      pLRBPrev  = NULL ;
      while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
      {
         plrb = _getLRBPtrByIdx( idx ) ;
         if ( eduId == plrb->dpsTxExectr->getEDUID() )
         {
            idxEduId  = idx ;
            pLRBEduId = plrb ;

            indexPrev = idxPrev ;
            pLRBPrev  = plrbPrev ;

            found = TRUE ;
            break ;
         }

         // remember previous index and pointer
         idxPrev  = idx ;
         plrbPrev = plrb ;

         // move to next
         idx = plrb->nextLRBIdx ;
      }
      return found ;
   }


   //
   // Description: walk through the owner LRB list, find the one with given
   //              eduid and check if the input lockMode is compatible with
   //              all owners
   // Input:
   //    eduId    -- EDU Id to search in owner LRB list
   //    lockMode -- lock mode to check ( waiter lock mode )
   //    lrbBegin -- the first LRB index in the owne list
   // Output:
   //    idxEduId -- the index of the first LRB object matches given eduId
   //                   if it is found.
   //    pLRBEduId-- the pointer of first LRB object matches the eduId
   //                if it is found. If not, it shall be the pointer of
   //                the last LRB object in the list
   //    idxPrev  -- the index of the LRB in the list previous to the owner
   //    pLRBPrev -- the pointer of the LRB in the list previous to the owner
   //    foundIncomp -- if an incompatible one is found in owner list
   // Dependency:  the lock bucket latch shall be acquired
   //
   void dpsTransLockManager::_getLRBByEDUIdAndCheckWaiterLockMode
   (
      const EDUID               eduId,
      const DPS_TRANSLOCK_TYPE  lockMode, 
      const UTIL_OBJIDX         lrbBegin,
      UTIL_OBJIDX             & idxEduId,
      dpsTransLRB *           & pLRBEduId, 
      UTIL_OBJIDX             & indexPrev,
      dpsTransLRB *           & pLRBPrev,
      BOOLEAN                 & foundIncomp
   )
   {
      EDUID lrbEduid  = 0 ;
      UTIL_OBJIDX idx = lrbBegin,
                  idxPrev = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRB *plrb     = NULL ,
                  *plrbPrev = NULL ;
      BOOLEAN foundMyself = FALSE ;

      idxEduId    = UTIL_INVALID_OBJ_INDEX ;
      pLRBEduId   = NULL ;
      indexPrev   = UTIL_INVALID_OBJ_INDEX ;
      pLRBPrev    = NULL ;
      foundIncomp = FALSE ;
      while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
      {
         plrb     = _getLRBPtrByIdx( idx ) ;
         lrbEduid = plrb->dpsTxExectr->getEDUID() ;
         if ( ( ! foundMyself ) && ( eduId == lrbEduid ) )
         {
            // save the index if the given eduId is found in the owner list
            idxEduId    = idx ;
            pLRBEduId   = plrb ;

            indexPrev   = idxPrev ;
            pLRBPrev    = plrbPrev ;

            foundMyself = TRUE ;
         }

         if (    ( ! foundIncomp )
              && ( eduId != lrbEduid )
              && ( ! dpsIsLockCompatible( plrb->lockMode, lockMode ) ) )
         {
            foundIncomp = TRUE ;
         } 

         // break if all jobs are done
         if ( foundMyself && foundIncomp )
         {
            break ;
         }

         // remember previous index and pointer
         idxPrev  = idx ;
         plrbPrev = plrb ;

         // move to next      
         idx = plrb->nextLRBIdx ;
      }
   }


   //
   // Description: add a LRB at the end of the LRB chain/list
   // Function:    walk through LRB list/chain, add the LRB at the end of
   //              list( owner, waiter or upgrade )
   // Input:
   //    lrbBegin -- the first LRB index in the chain( owner, waiter or upgrade
   //                queue )
   //    idxNew   -- the LRB index to be added in
   // Output:     None
   // Return:     None
   // Dependency:  the lock bucket latch shall be acquired
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
   // Output:     None
   // Return:     None
   // Dependency:  the lock bucket latch shall be acquired
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
   // Dependency:  the lock bucket latch shall be acquired
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


   //
   // Description: add a LRB at the end of the EDU LRB chain, which is the list
   //              of all locks acquired within a session/tx
   // Function:    walk through EDU LRB chain( doubly linked list ),
   //              add the LRB at the end of list. 
   //              
   //              the dpsTxExectr::_lastLRBIdx is the latest LRB index
   //              acquired within the same tx
   // Input:
   //    dpsTxExectr -- _dpsTransExecutor ptr
   //    idx         -- the LRB index to be added in
   // Output:     None
   // Return:     None
   // Dependency:  the lock bucket latch shall be acquired
   //
   void dpsTransLockManager::_addToEDULRBListTail
   (
      _dpsTransExecutor    * dpsTxExectr,
      const UTIL_OBJIDX      idx,
      const dpsTransLockId & lockId
   )
   {
#ifdef _DEBUG
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif
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
         else
         {
            // when dpsTxExectr->getLastLRBIdx() is invalid, means,
            // this LRB ( idx ) is the first LRB to be added in EDU LRB list
            if ( IS_VALID_SEG_OBJ_INDEX( idx ) )
            {
               dpsTransLRB *pLRB   = _getLRBPtrByIdx( idx ) ;
               pLRB->eduLrbIdxPrev = UTIL_INVALID_OBJ_INDEX ;
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
   // Dependency:  the lock bucket latch shall be acquired
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
   //              one if necessary
   // REVISIT:
   // Here are two cases :
   // 1. an EDU was waken up ( _waitLock returned SDB_OK )
   //    it checks next waiters lock mode whether compatible with itself
   //    as itself will retry acquire the lock. Only wake up next waiter
   //    if the lock mode is compatbile with current waiter.
   //
   // 2. an EDU was timeout / interrupted from _waitLock
   //    if the owner list is empty, we may wake up the next waiter, as
   //    the current one will not retry acquire the lock.
   //
   //    alternatives :
   //      a. treat this EDU same as been waken up case and retry acquire.
   //      b. do nothing, next waiter(s) can timeout same as this EDU.
   //
   // Input:
   //    dpsTxExectr     -- pointer to _dpsTransExecutor
   //    removeLRBHeader -- whether remove LRB Header when owner, waiter,
   //                       upgrade queue are all empty
   //                       when it is true, it means _waitLock returned
   //                       non SDB_OK value, due to either lock waiting
   //                       timeout elapsed or be interrupted.
   // Output:  none
   // Return:  none
   // Dependency:  the lock bucket latch shall be acquired
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST, "dpsTransLockManager::_removeFromUpgradeOrWaitList" )
   void dpsTransLockManager::_removeFromUpgradeOrWaitList
   (
      _dpsTransExecutor *    dpsTxExectr,
      const dpsTransLockId & lockId,
      const UTIL_OBJIDX      bktIdx,
      const BOOLEAN          removeLRBHeader
   )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST ) ;
#ifdef _DEBUG
      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      CHAR hdrLockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;

      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif
      UTIL_OBJIDX idx = dpsTxExectr->getWaiterLRBIdx() ;
      UTIL_OBJIDX idxNext = UTIL_INVALID_OBJ_INDEX ;
      UTIL_OBJIDX hdrIdx  = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRB *pLRB = NULL , *plrb = NULL ;
      dpsTransLRBHeader * pLRBHdr = NULL ;
#ifdef _DEBUG
      EDUID eduIDTrc = 0 ;

      ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                   "%s", lockId.toString().c_str() ) ;
      PD_TRACE6( SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST,
                 PD_PACK_ULONG( dpsTxExectr ),
                 PD_PACK_STRING( lockIdStr ), 
                 PD_PACK_UINT( bktIdx ),
                 PD_PACK_UINT( removeLRBHeader ),
                 PD_PACK_STRING( "LRB to be removed:" ),
                 PD_PACK_UINT( idx ) ) ;
#endif
      if ( IS_VALID_SEG_OBJ_INDEX( idx ) )
      {
         pLRB = _getLRBPtrByIdx( idx ) ;
#ifdef _DEBUG
         SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( pLRB->lrbHdrIdx ),
                     "Invalid LRB Header." );
#endif
         hdrIdx  = pLRB->lrbHdrIdx ;
         pLRBHdr = _getLRBHdrPtrByIdx( hdrIdx ) ;

         // sanity check, panic if fails.
         if ( ! ( pLRBHdr->lockId == lockId ) )
         {
            PD_LOG( PDERROR,
                    "Fatal error, requested lockId doesn't match LRB Header."
                    "Requested lockId:%s, lockId in LRB Header %s",
                    lockId.toString().c_str(),
                    pLRBHdr->lockId.toString().c_str() ) ;
#ifdef _DEBUG
            ossSnprintf( hdrLockIdStr, sizeof( hdrLockIdStr ),
                         "%s", pLRBHdr->lockId.toString().c_str() ) ;
            PD_TRACE3( SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST,
                       PD_PACK_STRING( "Invalid LRB Header, lockId not match"),
                       PD_PACK_STRING( lockIdStr ),
                       PD_PACK_STRING( hdrLockIdStr ) ) ;
#endif
            ossPanic() ;
         }
       
         if ( DPS_QUE_UPGRADE == dpsTxExectr->getWaiterQueType() )
         {
            // remove from upgrade list
            _removeFromLRBList( pLRBHdr->upgradeLRBIdx, idx, idxNext ) ; 

            // clear the wait info in dpsTxExectr
            dpsTxExectr->clearWaiterInfo() ;

            // if it is the last one in upgrade list,
            // set idxNext to the first one in waiter list
            if ( ! IS_VALID_SEG_OBJ_INDEX( idxNext ) )
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

            // in case upgrade is not empty,
            // set the idxNext to the first one in upgrade list,
            // as the upgrade list is actually high priority waiter list
            if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
            {
               idxNext = pLRBHdr->upgradeLRBIdx ;
            }
         }
#ifdef _DEBUG
         PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST,
                    PD_PACK_STRING( "Next LRB in wait or upgrade list:" ),
                    PD_PACK_UINT( idxNext ) ) ;
#endif
         // wake up the next waiting one if necessary.
         // Here are two cases :
         // 1. an EDU was waken up ( _waitLock returned SDB_OK )
         //    it checks next waiters lock mode whether compatible with itself
         //    as itself will retry acquire the lock. Only wake up next waiter
         //    if the lock mode is compatbile with current waiter.
         //
         // REVISIT 
         //
         // 2. an EDU was timeout / interrupted from _waitLock  
         //    if the owner list is empty, we may wake up the next waiter, as
         //    the current one will not retry acquire the lock. 
         //
         //    another approach is treat this EDU same as been waken up case
         //    and retry acquire. Or, do nothing let other waiters timeout.
         //    
         if ( IS_VALID_SEG_OBJ_INDEX( idxNext ) )
         {
            plrb = _getLRBPtrByIdx( idxNext ) ;
#ifdef _DEBUG
            SDB_ASSERT( pLRB->lrbHdrIdx == plrb->lrbHdrIdx, "Invalid LRB" ) ;
#endif
            // the EDU was waken up ( _waitLock returned SDB_OK )
            if ( FALSE == removeLRBHeader ) 
            {
               if ( dpsIsLockCompatible( pLRB->lockMode, plrb->lockMode ) )
               {
                  // wake up next waiter if the lock mode is compatible
                  _wakeUp( plrb->dpsTxExectr ) ;   
#ifdef _DEBUG
                  eduIDTrc = plrb->dpsTxExectr->getEDUID() ;

                  PD_TRACE4(
                     SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST,
                     PD_PACK_STRING( "Wake up EDU:" ),
                     PD_PACK_ULONG( eduIDTrc ),
                     PD_PACK_STRING( "Waiter LRB idx:" ),
                     PD_PACK_UINT( idxNext ) ) ;
#endif
               }
            }
            else
            {
               // the _waitLock() returned timeout or interrupted

               if ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx ) )
               {
                  // wake up next waiter if owner list is empty
                  _wakeUp( plrb->dpsTxExectr ) ;   
#ifdef _DEBUG
                  eduIDTrc = plrb->dpsTxExectr->getEDUID() ;

                  PD_TRACE4(
                     SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST,
                     PD_PACK_STRING( "Wake up EDU:" ),
                     PD_PACK_ULONG( eduIDTrc ),
                     PD_PACK_STRING( "Waiter LRB idx:" ),
                     PD_PACK_UINT( idxNext ) ) ;
#endif
               }
            }
         }

         // release the waiter LRB
         _releaseLRB( idx ) ;      

         // when LRB Header is empty, release it if necessary
#ifdef _DEBUG
         SDB_ASSERT( ( IS_VALID_SEG_OBJ_INDEX( hdrIdx ) && pLRBHdr ),
                     "Invalid LRB Header." ) ;
#endif
         if (    removeLRBHeader
              && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx   ) )
              && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
              && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx  ) ) )
         {
#ifdef _DEBUG
            PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST,
                       PD_PACK_STRING( "LRB Header to be removed:" ),
                       PD_PACK_UINT( hdrIdx ) ) ;
#endif
            // remove the LRB Header from the list
            _removeFromLRBHeaderList( _LockHdrBkt[bktIdx].lrbHdrIdx, hdrIdx ) ;
            // release the LRB Header
            _releaseLRBHdr( hdrIdx ) ;
            hdrIdx = UTIL_INVALID_OBJ_INDEX ;
            pLRBHdr = NULL ;
         }
      }
      PD_TRACE_EXIT( SDB_DPSTRANSLOCKMANAGER__REMOVEFROMUPGRADEORWAITLIST ) ;
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
   // Dependency:  the lock bucket latch shall be acquired
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
            pLRBHeader = _getLRBHdrPtrByIdx( idxDel ) ;
            idxBegin   = pLRBHeader->nextLRBHdrIdx ;
         }
         else
         {
            while ( IS_VALID_SEG_OBJ_INDEX( idx ) )
            {
               plrbHdr = _getLRBHdrPtrByIdx( idx ) ;
               if ( idxDel == plrbHdr->nextLRBHdrIdx )
               {
                  pLRBHeader             = _getLRBHdrPtrByIdx( idxDel ) ;
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
   // Dependency:  the lock bucket latch shall be acquired
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
   //     return SDB_DPS_INVALID_LOCK_UPGRADE_REQUEST
   //       . can't upgrade to requested lock mode
   //     return SDB_DPS_TRANS_APPEND_TO_WAIT
   //       . need to upgrade, new LRB is added to upgrade list
   //       . need to wait, new LRB is added to waiter list   
   //   2 DPS_TRANSLOCK_OP_MODE_TRY
   //     try mode will not add LRB to waiter or upgrade list
   //     return SDB_OK
   //       . lock acquired, new LRB is added in owner list
   //       . holing higher level lock, no need to add new LRB in owner list
   //     return SDB_DPS_INVALID_LOCK_UPGRADE_REQUEST
   //       . can't upgrade to requested lock mode
   //     return SDB_DPS_TRANS_LOCK_INCOMPATIBLE
   //       . request lock mode can't be acquired
   //   3 DPS_TRANSLOCK_OP_MODE_TEST
   //     return SDB_OK
   //       . request lock can be acquired
   //     return SDB_DPS_INVALID_LOCK_UPGRADE_REQUEST
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
   //     SDB_DPS_INVALID_LOCK_UPGRADE_REQUEST,
   //     SDB_DPS_TRANS_APPEND_TO_WAIT,
   //     SDB_DPS_TRANS_LOCK_INCOMPATIBLE,
   //     or other errors
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST, "dpsTransLockManager::_tryAcquireOrTest" )
   INT32 dpsTransLockManager::_tryAcquireOrTest
   (
      _dpsTransExecutor                * dpsTxExectr,
      const dpsTransLockId             & lockId,
      const DPS_TRANSLOCK_TYPE           requestLockMode,
      const DPS_TRANSLOCK_OP_MODE_TYPE   opMode,
      const UTIL_OBJIDX                  bktIdx,
      const BOOLEAN                      bktLatched,
      dpsTransRetInfo                  * pdpsTxResInfo,
      _dpsITransLockCallback           * callback 
   )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST ) ;
#ifdef _DEBUG
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif
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

      EDUID eduId    = dpsTxExectr->getEDUID() ;

#ifdef _DEBUG
      EDUID eduIDTrc = 0 ;
      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;

      ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                   "%s", lockId.toString().c_str() ) ;
      PD_TRACE8( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                 PD_PACK_ULONG( dpsTxExectr ),
                 PD_PACK_STRING( lockIdStr ),
                 PD_PACK_BYTE( requestLockMode ),
                 PD_PACK_BYTE( opMode ),
                 PD_PACK_UINT( bktIdx ),
                 PD_PACK_UINT( bktLatched ),
                 PD_PACK_ULONG( pdpsTxResInfo ),
                 PD_PACK_ULONG( eduId ) ) ;

      SDB_ASSERT( _initialized, "dpsTransLockManager is not initialized." ) ;
#endif

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
#ifdef _DEBUG
         SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( lrbIdxNew ) &&
                     IS_VALID_SEG_OBJ_INDEX( hdrIdxNew ) &&
                     pLRBNew &&
                     pLRBHdrNew, "Invalid LRB or LRB Header." ) ;
#endif
         bFreeLRB       = TRUE ;
         bFreeLRBHeader = TRUE ;
         if ( pdpsTxResInfo )
         {
            // default mark this edu has newly aquired/granted the lock
            // we will mark it false if we found the lock been covered
            pdpsTxResInfo->_newAcquire = TRUE;
         }
#ifdef _DEBUG
         PD_TRACE4( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                    PD_PACK_STRING("Acquired LRB Header:"),
                    PD_PACK_UINT( hdrIdxNew ),
                    PD_PACK_STRING("Acquired LRB:"),
                    PD_PACK_UINT( lrbIdxNew ) ) ;
#endif
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
#ifdef _DEBUG
         PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                    PD_PACK_STRING( "No LRB Header in bucket, lock acquired" ));
#endif
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
#ifdef _DEBUG
         PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                    PD_PACK_STRING( "No LRB Header found, lock acquired" ) ) ;
#endif
         // job done
         goto done;
      }
#ifdef _DEBUG
      PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                 PD_PACK_STRING( "Found LRB Header:" ),
                 PD_PACK_UINT( hdrIdx ) ) ;

      SDB_ASSERT( ( NULL != pLRBHdr ), "Invalid LRB Header" ) ;
      SDB_ASSERT( IS_VALID_SEG_OBJ_INDEX( hdrIdx ), "Invalid LRB Header index");
#endif
      // found the LRB header with same lockId

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
#ifdef _DEBUG
              PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                         PD_PACK_STRING( "Found CS,CL lock LRB:" ),
                         PD_PACK_UINT( lrbIdx ) ) ;
#endif
               pLRB = _getLRBPtrByIdx( lrbIdx ) ;
               if ( dpsLockCoverage( pLRB->lockMode, requestLockMode ) )
               {
                  if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
                  {
                     pLRB->refCounter++ ;

                     // clear the wait info in dpsTxExectr
                     dpsTxExectr->clearWaiterInfo() ;
                  }
#ifdef _DEBUG
                  PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                             PD_PACK_STRING( "CS,CL lock coverage chk passed"));
#endif
                  if( pdpsTxResInfo )
                  {
                     pdpsTxResInfo->_newAcquire = FALSE;
                  }
                  goto done ;
               }
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
#ifdef _DEBUG
      PD_TRACE7( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                 PD_PACK_STRING( "Result of searching owner list:" ),
                 PD_PACK_UINT( idxToInsert ),
                 PD_PACK_UINT( idxLastComp ),
                 PD_PACK_ULONG( pLRBIncomp ),
                 PD_PACK_UINT( idxEduId ),
                 PD_PACK_UINT( idxPrevEduId ),
                 PD_PACK_ULONG( pLRBPrevEduId ) ) ;
#endif

      if ( IS_VALID_SEG_OBJ_INDEX( idxEduId ) )
      {
#ifdef _DEBUG
         PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                    PD_PACK_STRING( "Found in owner list, LRB:" ) ,
                    PD_PACK_UINT( idxEduId ) ) ;
#endif
         //
         // in owner list
         //
         pLRB = _getLRBPtrByIdx( idxEduId ) ;
#ifdef _DEBUG
         SDB_ASSERT( pLRB && ( pLRB->lrbHdrIdx == hdrIdx ),
                     "Invalid LRB or the lrbHdrIdx doesn't match "
                     "the LRB Header index" ) ;
#endif
         // if current holding lock mode covers the requesting mode,
         // then job is done
         if ( dpsLockCoverage( pLRB->lockMode, requestLockMode ) )
         {
            if ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
            {
               pLRB->refCounter++ ;

               // clear the wait info in dpsTxExectr
               dpsTxExectr->clearWaiterInfo() ;
            }
#ifdef _DEBUG
            PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                       PD_PACK_STRING( "Current mode covers requesting" ),
                       PD_PACK_BYTE( pLRB->lockMode ) ) ; 
#endif
            if ( pdpsTxResInfo )
            {
               pdpsTxResInfo->_newAcquire = FALSE;
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
#ifdef _DEBUG
            PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                       PD_PACK_STRING( "Current mode can't be upgraded:" ),
                       PD_PACK_BYTE( pLRB->lockMode ) ) ; 
#endif
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
#ifdef _DEBUG
            eduIDTrc = pLRBIncomp->dpsTxExectr->getEDUID() ;

            PD_TRACE3( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                       PD_PACK_STRING( "May put in upgrade Q, conflict with:" ),
                       PD_PACK_ULONG( eduIDTrc ),
                       PD_PACK_BYTE( pLRBIncomp->lockMode ) ) ; 
#endif

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
#ifdef _DEBUG
                  SDB_ASSERT( pLRBPrevEduId, "Invalid LRB index " ) ;
#endif
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
#ifdef _DEBUG
            PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                       PD_PACK_STRING( "Upgrade sucessfully." ) ) ;
#endif
            // job done
            goto done ; 
         }
      }
      else
      {
#ifdef _DEBUG
         PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                    PD_PACK_STRING( "Not found in owner list" ) ) ;
#endif
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
#ifdef _DEBUG
            eduIDTrc = pLRBIncomp->dpsTxExectr->getEDUID() ;

            PD_TRACE3( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                       PD_PACK_STRING( "May put in waiter Q, conflict with:" ),
                       PD_PACK_ULONG( eduIDTrc ),
                       PD_PACK_BYTE( pLRBIncomp->lockMode ) ) ; 
#endif
            // job done
            goto done ;

         }
         else
         {
            // 
            // no incompatible found, i.e., compatible with other owners
#ifdef _DEBUG
            PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                       PD_PACK_STRING( "Compatible with all owners" ) );
#endif
            // add to owner list when satisfy following conditions :
            // a. if both upgrade and waiter list are empty
            // b. if it is doing retry-acquire after been woken up.( when input
            //    parameter, bktLatched, is true, means retry acquiring )
            if (   (    ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
                     && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx ) ) )
                || ( bktLatched ) )
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
#ifdef _DEBUG
               PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                          PD_PACK_STRING( "Lock acquired" ) );
#endif
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
#ifdef _DEBUG

                  eduIDTrc = pLRB->dpsTxExectr->getEDUID() ;

                  PD_TRACE3( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                             PD_PACK_STRING("Conflict with:"),
                             PD_PACK_ULONG( eduIDTrc ),
                             PD_PACK_BYTE( pLRB->lockMode ) ) ; 

#endif

               }

#ifdef _DEBUG
               PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST,
                          PD_PACK_STRING("Has waiter, may put in waiter Q") ) ;
#endif
               // job done
               goto done ;

            }  // if both upgrade and waiter queue/list are empty
         }  // if request lock mode is compatible with other owners
      }  // if in owner list
   done:
      if( callback )
      {
         callback->afterLockAcquire( lockId, rc, 
                                     requestLockMode,
                                     opMode,
                                     pdpsTxResInfo );
      }

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
      else
      {
         // sample lock owning( first time ) or waiting timestamp ( ossTick )
         if (    ( DPS_TRANSLOCK_OP_MODE_TEST != opMode )
              && IS_VALID_SEG_OBJ_INDEX( lrbIdxNew ) )
         {
            pLRB = _getLRBPtrByIdx( lrbIdxNew ) ;            
            pLRB->beginTick.sample() ;
         }
      }
      if ( bFreeLRBHeader )
      {
         _releaseLRBHdr( hdrIdxNew ) ;
         bFreeLRBHeader = FALSE ;
      }

      PD_TRACE_EXITRC( SDB_DPSTRANSLOCKMANAGER__TRYACQUIREORTEST, rc ) ;
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
#ifdef _DEBUG
      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif
      INT32   rc             = SDB_OK ;
      BOOLEAN lrbAcquired    = FALSE;
      BOOLEAN lrbHdrAcquired = FALSE;

      // acquire a free LRB
      rc = _pLRBMgr->acquire( lrbIdxNew, pLRBNew ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to acquire a free LRB (rc=%d)", rc );
         goto error ;
      }
#ifdef _DEBUG
      SDB_ASSERT( pLRBNew,
                  "_prepareNewLRBAndHeader: LRB can't be null" ) ;
#endif
      lrbAcquired = TRUE;

      // acuqire a free LRB Header
      rc = _pLRBHdrMgr->acquire( hdrIdxNew, pLRBHdrNew ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
#ifdef _DEBUG
      SDB_ASSERT( pLRBHdrNew,
                  "_prepareNewLRBAndHeader: LRB Header can't be null" ) ;
#endif
      // initial the new LRB
      // and mark the new LRB Header index in its lrbHdrIdx
      pLRBNew->dpsTxExectr    = dpsTxExectr ;
      pLRBNew->lockMode       = requestLockMode ;
      pLRBNew->refCounter     = 1 ;
      pLRBNew->eduLrbIdxNext  = UTIL_INVALID_OBJ_INDEX ;
      pLRBNew->eduLrbIdxPrev  = UTIL_INVALID_OBJ_INDEX ;
      pLRBNew->lrbHdrIdx      = hdrIdxNew ;
      pLRBNew->nextLRBIdx     = UTIL_INVALID_OBJ_INDEX ;
      pLRBNew->beginTick.clear() ;

      // inital the new LRB Header
      // and add the new LRB into the new LRB Header owner list
      pLRBHdrNew->lockId        = lockId ;
      pLRBHdrNew->ownerLRBIdx   = lrbIdxNew ;
      pLRBHdrNew->waiterLRBIdx  = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdrNew->upgradeLRBIdx = UTIL_INVALID_OBJ_INDEX ;
      pLRBHdrNew->nextLRBHdrIdx = UTIL_INVALID_OBJ_INDEX ;

      pLRBHdrNew->oldVer = SDB_OSS_NEW oldVersionContainer(hdrIdxNew);
      if ( pLRBHdrNew->oldVer == NULL )
      {
         rc = SDB_OOM;
         PD_LOG( PDERROR,
                 "Failed to acquire oldVer (rc=%d)", rc );
         goto error;
      }

   done:
      return rc ;
   error :
      if( lrbAcquired )
      {
         // release LRB in case failed to acquire LRB Header
         _releaseLRB( lrbIdxNew ) ;
      }

      if( lrbHdrAcquired )
      {
         // release LRB in case failed to acquire LRB Header
         _releaseLRBHdr( hdrIdxNew ) ;
      }

      PD_LOG( PDERROR,
                 "Failed to acquire a free LRB & Header (rc=%d)", rc );
      goto done;
   }


   //
   // Description: acquire a lock with given mode
   // Function:    acquire a lock with requested mode
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
   //    dpsTxExectr     -- dpsTxExectr ( per EDU, similar to eduCB,
   //                                     isolate pmd from dps )
   //    lockId          -- lock Id
   //    requestLockMode -- lock mode being requested
   //    pContext        -- pointer to context :
   //                         dmsTBTransContext
   //                         dmsIXTransContext
   //    callback        -- pointer to trans lock callback
   // Output:
   //    pdpsTxResInfo   -- pointer to dpsTransRetInfo, a structure used to
   //                       save the conflict lock info
   // Return:
   //     SDB_OK,                                 -- lock is acquired
   //     SDB_DPS_TRANS_APPEND_TO_WAIT,           -- put on wait/upgrade list
   //     SDB_DPS_INVALID_LOCK_UPGRADE_REQUEST,   -- invalid upgrade request
   //     SDB_INTERRUPT,                          -- lock wait interrrupted
   //     SDB_TIMEOUT,                            -- lock wait timeout elapsed
   //     or other errors
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER_ACQUIRE, "dpsTransLockManager::acquire" )
   INT32 dpsTransLockManager::acquire
   (
      _dpsTransExecutor        * dpsTxExectr,
      const dpsTransLockId     & lockId,
      const DPS_TRANSLOCK_TYPE   requestLockMode,
      IContext                 * pContext,
      dpsTransRetInfo          * pdpsTxResInfo,
      _dpsITransLockCallback   * callback
   )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER_ACQUIRE ) ;
#ifdef _DEBUG
      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      CHAR iLockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                   "%s", lockId.toString().c_str() ) ;
      PD_TRACE5( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                 PD_PACK_ULONG( dpsTxExectr ),
                 PD_PACK_STRING( lockIdStr ),
                 PD_PACK_BYTE( requestLockMode ),
                 PD_PACK_ULONG( pContext ),
                 PD_PACK_ULONG( pdpsTxResInfo ) ) ;

      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif
      INT32 rc  = SDB_OK ,
            rc2 = SDB_OK ; // context pause or resume return code
      dpsTransLockId     iLockId ;
      DPS_TRANSLOCK_TYPE iLockMode = DPS_TRANSLOCK_MAX ;
      BOOLEAN isIntentLockAcquired = FALSE ;

      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ;
      BOOLEAN bLatched   = FALSE ;
#ifdef _DEBUG
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;
#endif
      if ( ! lockId.isValid() )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR,
                 "Invalid lockId:%s", lockId.toString().c_str() ) ;
         goto error ;
      }

      // get intent lock at first
      if ( ! lockId.isRootLevel() )
      {
         iLockId   = lockId.upOneLevel() ;
         iLockMode = dpsIntentLockMode( requestLockMode ) ;
#ifdef _DEBUG
         ossSnprintf( iLockIdStr, sizeof( iLockIdStr ),
                      "%s", iLockId.toString().c_str() ) ;
         PD_TRACE3( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                    PD_PACK_STRING( "Acquiring intent lock:" ),
                    PD_PACK_STRING( iLockIdStr ),
                    PD_PACK_BYTE( iLockMode )  ) ;
#endif
         rc = acquire( dpsTxExectr, iLockId, iLockMode, 
                       pContext, pdpsTxResInfo );
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         isIntentLockAcquired = TRUE ;
      }

      // calculate the hash index by lockId 
      bktIdx = _getBucketNo( lockId ) ;

   acquireRetry:
      //
      // acquire the lock
      //
      rc = _tryAcquireOrTest( dpsTxExectr, lockId, requestLockMode,
                              DPS_TRANSLOCK_OP_MODE_ACQUIRE,
                              bktIdx,
                              bLatched,
                              pdpsTxResInfo,
                              callback ) ;
      // _tryAcquireOrTest acquires bucket latch by default unless the input
      // parameter, bLatched, is set to TRUE; and it always releases the latch
      // before returns
      bLatched = FALSE ;

      if ( SDB_OK == rc )
      {
         // lock acquired sucessfully, job done
         goto done ;
      }
      else if ( SDB_DPS_TRANS_APPEND_TO_WAIT == rc )
      {
         // checks if need to process lock waiting logic
         goto LockWaiting ;
      }
      else
      {
         // lock request is neither fulfilled nor put on wait
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB_DPSTRANSLOCKMANAGER_ACQUIRE, rc ) ;
      return rc ;
   
   LockWaiting:
      //
      // Processing lock waiting. 
      //   at this time the lock request is put on upgrade or wait queue.
      //
 
      // init rc2. it is used to mark whether the context has
      // been successfully paused / resumed
      rc2 = SDB_OK ;
#ifdef _DEBUG
      PD_TRACE1( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                 PD_PACK_STRING("Lock waiting process") ) ;
#endif
      // pause the context before waiting the lock
      if ( pContext )
      {
         rc2 = pContext->pause() ;
#ifdef _DEBUG
         PD_TRACE2( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                    PD_PACK_STRING("context pause rc:"),
                    PD_PACK_INT( rc2 ) )  ;
#endif
      }
      if ( SDB_OK != rc2 )
      {
         PD_LOG( PDERROR, "Failed to pause context, rc:%d", rc2 ) ;
         goto postLockWaiting ;
      }
     
      // wait for the lock
      rc = _waitLock( dpsTxExectr ) ;
#ifdef _DEBUG
      PD_TRACE2( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                 PD_PACK_STRING("waitLock rc:"),
                 PD_PACK_INT( rc ) )  ;
#endif
      if ( SDB_OK != rc )
      {
         goto postLockWaiting ;
      }

      // resume context
      if ( pContext )
      {
         rc2 = pContext->resume() ;
#ifdef _DEBUG
         PD_TRACE2( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                    PD_PACK_STRING("context resume rc:"),
                    PD_PACK_INT( rc2 ) )  ;
#endif
      }
      if ( SDB_OK != rc2 )
      {
         PD_LOG( PDERROR, "Failed to resume context, rc:%d", rc2 ) ;
      }

   postLockWaiting:
#ifdef _DEBUG
      PD_TRACE1( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                 PD_PACK_STRING("Removing from upgrade or wait list") ) ;
#endif
      // remove the LRB from upgrade or waiter list and remove the empty
      // LRB Header if it is needed
      if ( ! bLatched )
      {
         // need latch bucket before remove it from upgrade or waiter list
         _acquireOpLatch( bktIdx ) ; 
         bLatched = TRUE ;
      }

      // remove the LRB from upgrade or waiter list and wakeup next waiter
      // if necessary. The empty LRB Header will be removed only when it is
      // needed, i.e., when _waitLock() fails( either timeout duration elapsed
      // or be interrupted )
      // The reason removing the empty LRB Header only when _waitLock() fails
      // is if _waitLock returns success, when retry acquiring the lock,
      // _tryAcquireOrTest(), the LRB Header will be added back again.
      _removeFromUpgradeOrWaitList( dpsTxExectr,
                                    lockId, bktIdx, ( SDB_OK != rc ) ) ;

      // retry acquire the lock when following conditions are satisfied:
      //   . SDB_OK == rc = _waitLock(), it has been woken up
      //   . SDB_OK == rc2, context resumed successfully
      //   . bLatched, holding the bucket latch, to avoid race condition
      //     between itself and the one is woken up
      if ( ( SDB_OK == rc ) && ( SDB_OK == rc2 ) && bLatched )
      {
#ifdef _DEBUG
         PD_TRACE1( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                    PD_PACK_STRING("Retry acquire") ) ;
#endif
         goto acquireRetry ;
      }

      if ( bLatched )
      {
         _releaseOpLatch( bktIdx ) ;
         bLatched = FALSE ;
      }

      // when _waitLock() fails ( timeout or be interrupted ), or pause/resume
      // context fails, we will need to release upper level intent lock
#ifdef _DEBUG
      PD_TRACE1( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                 PD_PACK_STRING("lock waiting error code path") ) ;
#endif
      // set recode to rc2 when _waitLock() succeeded but resume()
      // or pause() failed
      if ( ( SDB_OK != rc2 ) && ( SDB_OK == rc ) )
      {
         rc = rc2 ;
      }

   error:
      if ( bLatched )
      {
         _releaseOpLatch( bktIdx ) ;
         bLatched = FALSE ;
      }
#ifdef _DEBUG
      PD_TRACE1( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                 PD_PACK_STRING("Error code path") ) ;
#endif
      if ( isIntentLockAcquired )
      {
         release( dpsTxExectr, iLockId, FALSE ) ;
         isIntentLockAcquired = FALSE ;
#ifdef _DEBUG
         PD_TRACE2( SDB_DPSTRANSLOCKMANAGER_ACQUIRE,
                    PD_PACK_STRING( "Release intent lock:" ),
                    PD_PACK_STRING( iLockId.toString().c_str() ) ) ;
#endif
      }
      goto done ;
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
   //    callback        -- pointer to translock callback
   // Output :
   //    none
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER__RELEASE, "dpsTransLockManager::_release" )
   void dpsTransLockManager::_release
   (
      _dpsTransExecutor       * dpsTxExectr,
      const dpsTransLockId    & lockId,
      const BOOLEAN             bForceRelease,
      _dpsITransLockCallback  * callback
   )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER__RELEASE ) ;
#ifdef _DEBUG
      EDUID eduIDTrc = 0 ;

      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                   "%s", lockId.toString().c_str() ) ;
      PD_TRACE4( SDB_DPSTRANSLOCKMANAGER__RELEASE,
                 PD_PACK_ULONG( dpsTxExectr ),
                 PD_PACK_ULONG( callback ),
                 PD_PACK_STRING( lockIdStr ),
                 PD_PACK_UINT( bForceRelease ) ) ;

      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;
#endif
      UTIL_OBJIDX hdrIdx         = UTIL_INVALID_OBJ_INDEX ,
                  ownerLrbIdx    = UTIL_INVALID_OBJ_INDEX ,
                  prevOwnerIdx   = UTIL_INVALID_OBJ_INDEX ,
                  lrbHdrToRelase = UTIL_INVALID_OBJ_INDEX ,
                  lrbToRelase    = UTIL_INVALID_OBJ_INDEX ;
      UTIL_OBJIDX bktIdx         = UTIL_INVALID_OBJ_INDEX ;
      UTIL_OBJIDX waiterIdx      = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRB *pOwnerLRB     = NULL ,
                  *pPrevOwnerLRB = NULL ,
                  *pWaiterLRB    = NULL ;
      dpsTransLRBHeader *pLRBHdr = NULL ;
      EDUID eduId = dpsTxExectr->getEDUID() ;
      BOOLEAN bLatched = FALSE ;
      BOOLEAN foundIncomp = FALSE ;

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
            // remove the LRB Header if onwer, waiter,
            // and upgrade list are all empty
            if (    ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx   ) )
                 && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
                 && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx  ) ) )
            {
               _removeFromLRBHeaderList( _LockHdrBkt[bktIdx].lrbHdrIdx,
                                         hdrIdx ) ;
               // save index of LRB Header to be released
               lrbHdrToRelase = hdrIdx ;
#ifdef _DEBUG
               PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__RELEASE,
                          PD_PACK_STRING( "Remove empty LRB Header" ),
                          PD_PACK_UINT( hdrIdx ) ) ;
               PD_LOG( PDERROR, "Remove empty LRB Header %u", hdrIdx ) ;
#endif
               goto done ;
            }

            // get the waiter LRB pointer
            if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
            {
               pWaiterLRB = _getLRBPtrByIdx( pLRBHdr->upgradeLRBIdx ) ;
               waiterIdx  = pLRBHdr->upgradeLRBIdx ;     

#ifdef _DEBUG
               eduIDTrc   = pWaiterLRB->dpsTxExectr->getEDUID() ;
#endif

            }
            else if ( IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx ) )
            {
               pWaiterLRB = _getLRBPtrByIdx( pLRBHdr->waiterLRBIdx ) ;
               waiterIdx  = pLRBHdr->waiterLRBIdx ;

#ifdef _DEBUG
               eduIDTrc   = pWaiterLRB->dpsTxExectr->getEDUID() ;
#endif

            }

            // lookup owner list to find the LRB with same eduid
            ownerLrbIdx = UTIL_INVALID_OBJ_INDEX ;
            pOwnerLRB   = NULL ;

            if ( pWaiterLRB )
            { 
               // lookup owner list to find the LRB with same eduid,
               // and check if the waiter lockMode is compabile with other
               // owners
               _getLRBByEDUIdAndCheckWaiterLockMode( eduId,
                                                     pWaiterLRB->lockMode,
                                                     pLRBHdr->ownerLRBIdx,
                                                     ownerLrbIdx, pOwnerLRB,
                                                     prevOwnerIdx,pPrevOwnerLRB,
                                                     foundIncomp ) ;
            }
            else
            { 
               // lookup owner list to find the LRB with same eduid
               _getLRBByEDUId( eduId,
                               pLRBHdr->ownerLRBIdx,
                               ownerLrbIdx, pOwnerLRB,
                               prevOwnerIdx,pPrevOwnerLRB ) ;
               
            }
#ifdef _DEBUG
            PD_TRACE8( SDB_DPSTRANSLOCKMANAGER__RELEASE,
                       PD_PACK_STRING( "LRB Header:" ),
                       PD_PACK_UINT( hdrIdx ),
                       PD_PACK_STRING( "Owner LRB:" ),
                       PD_PACK_UINT( ownerLrbIdx ),
                       PD_PACK_STRING( "Waiter LRB idx:" ),
                       PD_PACK_UINT( waiterIdx ) ,
                       PD_PACK_STRING( "Found an incompatible in owner Q:" ),
                       PD_PACK_UINT( foundIncomp ) ) ;
#endif
            if ( ! ( pOwnerLRB && IS_VALID_SEG_OBJ_INDEX( ownerLrbIdx ) ) )
            {
#ifdef _DEBUG
               PD_LOG( PDERROR,
                       "Owner LRB is not found."OSS_NEWLINE
                       "EDU:%llu lockId:%s"OSS_NEWLINE
                       "LRB Header:%u Owner LRB:%u Waiter LRB:%u"OSS_NEWLINE
                       "Found an incompatible LRB:%d",
                       eduId,lockId.toString().c_str(),
                       hdrIdx, ownerLrbIdx, waiterIdx,
                       foundIncomp ) ;
#endif
               goto done ;
            }
            
            if ( bForceRelease )
            {
               pOwnerLRB->refCounter = 0 ; 
            }
            else
            {
#ifdef _DEBUG
               SDB_ASSERT( pOwnerLRB->refCounter > 0,
                           "refCounter is negative" ) ;
#endif
               pOwnerLRB->refCounter -- ; 
            }

#ifdef _DEBUG
               PD_LOG( PDDEBUG, "releasing lock before callback,"
                       " lockid=%s,bForceRelease=%d, callback=%x,refcounter=%d",
                       lockIdStr, bForceRelease, callback, pOwnerLRB->refCounter);

#endif

            // invoke call back function before release
            if( callback )
            {
               callback->
                  beforeLockRelease( lockId,
                                     pOwnerLRB->lockMode,
                                     pOwnerLRB->refCounter,
                                     pLRBHdr->oldVer );
            }

            if ( 0 == pOwnerLRB->refCounter )
            {
#ifdef _DEBUG
               PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__RELEASE,
                          PD_PACK_STRING( "Remove from EDU LRB and owner Q" ) );
#endif
               // remove it from EDU LRB list
               _removeFromEDULRBList( dpsTxExectr, ownerLrbIdx, lockId ) ;

               // remove it from lock owner list
               if ( pLRBHdr->ownerLRBIdx == ownerLrbIdx )
               {
                  // it is the first one in owner list
                  pLRBHdr->ownerLRBIdx = pOwnerLRB->nextLRBIdx ;
               }
               else if ( IS_VALID_SEG_OBJ_INDEX( prevOwnerIdx ) )
               {
                  pPrevOwnerLRB->nextLRBIdx = pOwnerLRB->nextLRBIdx ;
               }

               // if the owner queue is empty ( after remove current owner ),
               // or if the waiter lockMode is compabile with other owners
               // wake it up
               if ( pWaiterLRB && 
                    ( ( FALSE == foundIncomp ) || 
                      ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx ) ) ) )
               {
                  // wake up the edu by posting an event
                  _wakeUp( pWaiterLRB->dpsTxExectr ) ;

#ifdef _DEBUG
                  PD_TRACE4( SDB_DPSTRANSLOCKMANAGER__RELEASE,
                             PD_PACK_STRING( "Wakeup the EDU:" ),
                             PD_PACK_ULONG( eduIDTrc ),
                             PD_PACK_STRING( "Waiter LRB idx:" ),
                             PD_PACK_UINT( waiterIdx ) ) ;
#endif

               }

               // save the index of owner LRB to be released
               lrbToRelase = ownerLrbIdx ; 

               // remove the LRB Header if onwer, waiter,
               // and upgrade list are all empty
               if (    ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->ownerLRBIdx   ) )
                    && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->upgradeLRBIdx ) )
                    && ( ! IS_VALID_SEG_OBJ_INDEX( pLRBHdr->waiterLRBIdx  ) ) )
               {
                  _removeFromLRBHeaderList( _LockHdrBkt[bktIdx].lrbHdrIdx, 
                                            hdrIdx ) ;

                  // save index of LRB Header to be released
                  lrbHdrToRelase = hdrIdx ;
#ifdef _DEBUG
                  PD_TRACE1( SDB_DPSTRANSLOCKMANAGER__RELEASE,
                             PD_PACK_STRING( "Remove LRB Header" ) ) ;
#endif
               }
            }
         }
         else
         {
#ifdef _DEBUG
            // FIXME: should this LRBHdr existing?
               PD_LOG( PDDEBUG, "LRBHdr not found/valid, releasing lock skipped,"
                       " lockid=%s,bForceRelease=%d, callback=%x,hdrIdx=%d",
                       lockIdStr, bForceRelease, callback, hdrIdx);

#endif
         }
      }
   done: 
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

      PD_TRACE_EXIT( SDB_DPSTRANSLOCKMANAGER__RELEASE ) ;
      return ;
   }


   //
   // Description: release a lock
   // Function: release a lock, remove from the edu ( caller ) LRB chain
   //           and remove it from owner list if the refference counter is zero.
   //           Wake up a waiter when necessary, it will also release the upper
   //           level intent lock
   // Input:
   //    dpsTxExectr     -- pointer to dpsTransExecutor 
   //    lockId          -- lock id
   //    bForceRelease   -- requested lock mode
   // Output:
   //    none
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER_RELEASE, "dpsTransLockManager::release" )
   void dpsTransLockManager::release
   (
      _dpsTransExecutor      * dpsTxExectr,
      const dpsTransLockId   & lockId,
      const BOOLEAN            bForceRelease,
      _dpsITransLockCallback * callback 
   )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER_RELEASE ) ;
#ifdef _DEBUG
      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                   "%s", lockId.toString().c_str() ) ;
      PD_TRACE4( SDB_DPSTRANSLOCKMANAGER_RELEASE,
                 PD_PACK_ULONG( dpsTxExectr ),
                 PD_PACK_STRING( lockIdStr ),
                 PD_PACK_ULONG( callback ),
                 PD_PACK_UINT( bForceRelease ) ) ;

      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;
#endif

      if ( lockId.isValid() )
      {
         dpsTransLockId iLockId ;

         // main logic of release by lockId
         _release( dpsTxExectr, lockId, bForceRelease, callback ) ;

         // release the intent lock
         if ( ! lockId.isRootLevel() )
         {
            iLockId = lockId.upOneLevel() ;
#ifdef _DEBUG
            PD_TRACE3( SDB_DPSTRANSLOCKMANAGER_RELEASE,
                       PD_PACK_STRING( "Releasing intent lock:" ),
                       PD_PACK_STRING( iLockId.toString().c_str() ),
                       PD_PACK_UINT( bForceRelease ) ) ;
#endif 
            release( dpsTxExectr, iLockId, bForceRelease ) ;
         }
      }
      else
      {
         PD_LOG( PDERROR,
                 "Invalid lockId:%s", lockId.toString().c_str() ) ;
      }

      PD_TRACE_EXIT( SDB_DPSTRANSLOCKMANAGER_RELEASE ) ;
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
   //    dpsTxExectr -- pointer to _dpsTransExecutor 
   //    lockId      -- lock id
   // Output:
   //    none
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER__RELEASEALL, "dpsTransLockManager::_releaseAll" )
   void dpsTransLockManager::_releaseAll
   (
      _dpsTransExecutor    * dpsTxExectr,
      const dpsTransLockId & lockId,
      _dpsITransLockCallback * callback
   )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER__RELEASEALL ) ;
#ifdef _DEBUG
      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                   "%s", lockId.toString().c_str() ) ;
      PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__RELEASEALL,
                 PD_PACK_ULONG( dpsTxExectr ),
                 PD_PACK_STRING( lockIdStr ) ) ;

      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif

      dpsTransLockId iLockId;
 
      if ( lockId.isValid() )
      { 
         // main logic of release by lockId, force release mode
         _release( dpsTxExectr, lockId, TRUE, callback ) ;

         // release the intent lock
         if ( ! lockId.isRootLevel() )
         {
            iLockId = lockId.upOneLevel() ;
#ifdef _DEBUG
            PD_TRACE2( SDB_DPSTRANSLOCKMANAGER__RELEASEALL,
                       PD_PACK_STRING( "ReleasingAll intent lock:"),
                       PD_PACK_STRING( iLockId.toString().c_str() ) ) ;
#endif
            _releaseAll( dpsTxExectr, iLockId ) ;
         }
      }
      else
      {
         PD_LOG( PDERROR,
                 "Invalid lockId:%s", lockId.toString().c_str() ) ;
      }

      PD_TRACE_EXIT( SDB_DPSTRANSLOCKMANAGER__RELEASEALL ) ;
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
   //    dpsTxExectr     -- pointer to _dpsTransExecutor 
   //    callback        -- pointer to call back function
   // Output:
   //    none
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER_RELEASEALL, "dpsTransLockManager::releaseAll" )
   void dpsTransLockManager::releaseAll( _dpsTransExecutor *dpsTxExectr,
                                         _dpsITransLockCallback * callback )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER_RELEASEALL ) ;
#ifdef _DEBUG
      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      PD_TRACE1( SDB_DPSTRANSLOCKMANAGER_RELEASEALL,
                 PD_PACK_ULONG( dpsTxExectr ) ) ;

      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif

      if ( IS_VALID_SEG_OBJ_INDEX( dpsTxExectr->getLastLRBIdx() ) )
      {
         dpsTransLRB       *pLRB ;
         dpsTransLRBHeader *pLRBHdr ;
         UTIL_OBJIDX hdrIdx, lrbIdx = dpsTxExectr->getLastLRBIdx() ;
         dpsTransLockId lockId;

         while ( IS_VALID_SEG_OBJ_INDEX( lrbIdx ) )
         {
            pLRB = _getLRBPtrByIdx( lrbIdx ) ;

            // peek LRB Header index
            hdrIdx = pLRB->lrbHdrIdx ;
#ifdef _DEBUG
            PD_TRACE3( SDB_DPSTRANSLOCKMANAGER_RELEASEALL,
                       PD_PACK_STRING( "LRB Header and LRB:" ),
                       PD_PACK_UINT( hdrIdx ),
                       PD_PACK_UINT( lrbIdx ) ) ;
#endif
            // get LRB Header
            if ( IS_VALID_SEG_OBJ_INDEX( hdrIdx ) )
            {
               pLRBHdr = _getLRBHdrPtrByIdx( hdrIdx ) ;
               lockId  = pLRBHdr->lockId ;
#ifdef _DEBUG
               ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                            "%s", lockId.toString().c_str() ) ;
               PD_TRACE2( SDB_DPSTRANSLOCKMANAGER_RELEASEALL,
                          PD_PACK_STRING( "Releasing lock:" ),
                          PD_PACK_STRING( lockIdStr ) ) ;

               SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;
#endif

               // release the lock 
               _releaseAll( dpsTxExectr, lockId, callback ) ;
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
      PD_TRACE_EXIT( SDB_DPSTRANSLOCKMANAGER_RELEASEALL ) ;
      return ;
   }


   //
   // Description: calculate the index to LRB Header bucket
   // Function: get the index number to the LRB Header bucket by hashing lockId
   //           
   // Input:
   //    lockId -- lock id
   // Return:
   //    index to the LRB Header bucket
   //
   UTIL_OBJIDX dpsTransLockManager::_getBucketNo
   (
      const dpsTransLockId &lockId
   )
   {
      return (UTIL_OBJIDX)( lockId.lockIdHash() % DPS_TRANS_LOCKBUCKET_SLOTS_MAX );
   }


   //
   // Description: Wakeup a lock waiting EDU
   // Function: wake up an EDU by post an event
   //
   // Input:
   //    dpsTxExectr -- pointer to _dpsTransExecutor
   // Output:
   //    none
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER__WAKEUP, "dpsTransLockManager::_wakeUp" )
   void dpsTransLockManager::_wakeUp( _dpsTransExecutor *dpsTxExectr )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER__WAKEUP ) ;
#ifdef _DEBUG
      SDB_ASSERT( dpsTxExectr, "dpsTransExecutor can't be NULL" ) ;
#endif
      dpsTxExectr->wakeup() ;
      PD_TRACE_EXIT( SDB_DPSTRANSLOCKMANAGER__WAKEUP ) ;
   }


   //
   // Description: Wait a lock
   // Function: wait a lock by waiting on an event
   //
   // Input:
   //    dpsTxExectr -- pointer to _dpsTransExecutor
   // Output:
   //    none
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER__WAITLOCK, "dpsTransLockManager::_waitLock" )
   INT32 dpsTransLockManager::_waitLock( _dpsTransExecutor *dpsTxExectr )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER__WAITLOCK ) ;
#ifdef _DEBUG
      SDB_ASSERT( dpsTxExectr, "dpsTransExecutor can't be NULL" ) ;
#endif
      rc = dpsTxExectr->wait( dpsTxExectr->getTransTimeout() ) ;

      PD_TRACE_EXITRC( SDB_DPSTRANSLOCKMANAGER__WAITLOCK, rc ) ;
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
   //     SDB_DPS_INVALID_LOCK_UPGRADE_REQUEST,
   //     SDB_DPS_TRANS_LOCK_INCOMPATIBLE,
   //     or other errors
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER_TRYACQUIRE, "dpsTransLockManager::tryAcquire" )
   INT32 dpsTransLockManager::tryAcquire
   (
      _dpsTransExecutor        * dpsTxExectr,
      const dpsTransLockId     & lockId,
      const DPS_TRANSLOCK_TYPE   requestLockMode,
      dpsTransRetInfo          * pdpsTxResInfo,
      _dpsITransLockCallback   * callback
   )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER_TRYACQUIRE ) ;
#ifdef _DEBUG
      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      CHAR iLockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                   "%s", lockId.toString().c_str() ) ;
      PD_TRACE4( SDB_DPSTRANSLOCKMANAGER_TRYACQUIRE,
                 PD_PACK_ULONG( dpsTxExectr ),
                 PD_PACK_STRING( lockIdStr ),
                 PD_PACK_BYTE( requestLockMode ),
                 PD_PACK_ULONG( pdpsTxResInfo ) ) ;

      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif

      INT32 rc = SDB_OK ;
      dpsTransLockId iLockId;
      DPS_TRANSLOCK_TYPE iLockMode = DPS_TRANSLOCK_MAX ;
      BOOLEAN isIntentLockAcquired = FALSE;
      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ;
#ifdef _DEBUG
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;
#endif
      if ( ! lockId.isValid() )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR,
                 "Invalid lockId:%s", lockId.toString().c_str() ) ;
         goto error ;
      }

      // get intent lock at first
      // it is not need to get intent lock while lock space
      if ( ! lockId.isRootLevel() )
      {
         iLockId = lockId.upOneLevel() ;
         iLockMode = dpsIntentLockMode( requestLockMode ) ;
#ifdef _DEBUG
         ossSnprintf( iLockIdStr, sizeof( iLockIdStr ),
                      "%s", iLockId.toString().c_str() ) ;
         PD_TRACE3( SDB_DPSTRANSLOCKMANAGER_TRYACQUIRE,
                    PD_PACK_STRING( "Trying intent lock:" ),
                    PD_PACK_STRING( iLockIdStr ),
                    PD_PACK_BYTE( iLockMode )  ) ;
#endif
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
                              pdpsTxResInfo, 
                              callback ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB_DPSTRANSLOCKMANAGER_TRYACQUIRE, rc ) ;
      return rc;

   error:
      if ( isIntentLockAcquired )
      {
         release( dpsTxExectr, iLockId, FALSE ) ;
         isIntentLockAcquired = FALSE ;
#ifdef _DEBUG
         PD_TRACE2( SDB_DPSTRANSLOCKMANAGER_TRYACQUIRE,
                    PD_PACK_STRING( "Release intent lock:" ),
                    PD_PACK_STRING( iLockId.toString().c_str() ) ) ;
#endif
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
   //     SDB_DPS_INVALID_LOCK_UPGRADE_REQUEST,
   //     SDB_DPS_TRANS_LOCK_INCOMPATIBLE,
   //     or other errors
   // Dependency:  the lock manager must be initialized
   //

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSLOCKMANAGER_TESTACQUIRE, "dpsTransLockManager::testAcquire" )
   INT32 dpsTransLockManager::testAcquire
   (
      _dpsTransExecutor        * dpsTxExectr,
      const dpsTransLockId     & lockId,
      const DPS_TRANSLOCK_TYPE   requestLockMode,
      dpsTransRetInfo          * pdpsTxResInfo
   )
   {
      PD_TRACE_ENTRY( SDB_DPSTRANSLOCKMANAGER_TESTACQUIRE ) ;
#ifdef _DEBUG
      CHAR lockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      CHAR iLockIdStr[ DPS_LOCKID_STRING_MAX_SIZE ] = { '\0' } ;
      ossSnprintf( lockIdStr, sizeof( lockIdStr ),
                   "%s", lockId.toString().c_str() ) ;
      PD_TRACE4( SDB_DPSTRANSLOCKMANAGER_TESTACQUIRE,
                 PD_PACK_ULONG( dpsTxExectr ),
                 PD_PACK_STRING( lockIdStr ),
                 PD_PACK_BYTE( requestLockMode ),
                 PD_PACK_ULONG( pdpsTxResInfo ) ) ;

      SDB_ASSERT( dpsTxExectr, "dpsTxExectr can't be null" ) ;
#endif
      INT32 rc = SDB_OK;
      dpsTransLockId iLockId;
      DPS_TRANSLOCK_TYPE iLockMode = DPS_TRANSLOCK_MAX ;
      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ;
#ifdef _DEBUG
      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;
#endif
      if ( ! lockId.isValid() )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR,
                 "Invalid lockId:%s", lockId.toString().c_str() ) ;
         goto error ;
      }

      // get intent lock at first
      // it is not need to get intent lock while lock space
      if ( ! lockId.isRootLevel() )
      {
         iLockId = lockId.upOneLevel() ;
         iLockMode = dpsIntentLockMode( requestLockMode ) ;
#ifdef _DEBUG
         ossSnprintf( iLockIdStr, sizeof( iLockIdStr ),
                      "%s", iLockId.toString().c_str() ) ;
         PD_TRACE3( SDB_DPSTRANSLOCKMANAGER_TESTACQUIRE,
                    PD_PACK_STRING( "Testing intent lock:" ),
                    PD_PACK_STRING( iLockIdStr ),
                    PD_PACK_BYTE( iLockMode )  ) ;
#endif
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
      if ( SDB_OK != rc )
      {
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB_DPSTRANSLOCKMANAGER_TESTACQUIRE, rc ) ;
      return rc;
   error:
      goto done;
   }


   //
   // Description: whether a lock is being waited
   // Function:    test whether a lock is being waited by checking if
   //              the waiter list and upgrade list are empty.
   // Input:
   //    lockId -- lock Id
   // Output:
   //    none
   // Return:
   //    True   -- the lock has waiter(s)
   //    False  -- the lock has no waiter(s)
   // Dependency:  the lock manager must be initialized
   //
   BOOLEAN dpsTransLockManager::hasWait( const dpsTransLockId &lockId )
   {
      BOOLEAN result = FALSE;
      UTIL_OBJIDX bktIdx = UTIL_INVALID_OBJ_INDEX ;
      UTIL_OBJIDX hdrIdx = UTIL_INVALID_OBJ_INDEX ;
      dpsTransLRBHeader *pLRBHdr = NULL ;

      SDB_ASSERT( lockId.isValid(), "Invalid lockId" ) ;
      if ( ! lockId.isValid() )
      {
         PD_LOG( PDERROR,
                 "Invalid lockId:%s", lockId.toString().c_str() ) ;
         goto error ;
      }

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

   done:
      return result ;
   error:
      goto done ;
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

         UINT32 seconds = 0, microseconds = 0 ;
         ossTickConversionFactor factor ;
         ossTick endTick ;
         endTick.sample() ;
         ossTickDelta delta = endTick - pLRB->beginTick ;
         delta.convertToTime( factor, seconds, microseconds ) ;
 
         ossSnprintf( pBuf, bufSz,
                      "LRB: %u, dpsTxExectr: %p, "
                      "eduLrbIdxNext: %u, eduLrbIdxPrev: %u, "
                      "lrbHdrIdx: %u, nextLRBIdx: %u "
                      "refCounter: %llu, lockMode: %s, duration:%llu",
                      idx, pLRB->dpsTxExectr,
                      pLRB->eduLrbIdxNext, pLRB->eduLrbIdxPrev,
                      pLRB->lrbHdrIdx, pLRB->nextLRBIdx,
                      pLRB->refCounter,
                      lockModeToString( pLRB->lockMode ),
                      (UINT64)(seconds*1000 + microseconds / 1000 ) ) ;
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

         UINT32 seconds = 0, microseconds = 0 ;
         ossTickConversionFactor factor ;
         ossTick endTick ; 
         endTick.sample() ;
         ossTickDelta delta = endTick - pLRB->beginTick ;
         delta.convertToTime( factor, seconds, microseconds ) ;

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
         pBuff += ossSnprintf( pBuff, bufSz - strlen( pBuf ),
                               "%sduration     : %llu"OSS_NEWLINE, pStr,
                               (UINT64)(seconds*1000 + microseconds / 1000 ) ) ;
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
         dpsTransLRBHeader *pLRBHdr = _getLRBHdrPtrByIdx( idx ) ;
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
         dpsTransLRBHeader *pLRBHdr = _getLRBHdrPtrByIdx( idx ) ;
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
   // dump all LRB in EDU LRB chain to specified the file ( full path name )
   // for debugging purpose .
   // It walks through EDU LRB chain, the caller shall acquire the monitoring(
   // dump ) latch, acquireMonLatch(), and make sure the executor is still
   // available
   //
   void dpsTransLockManager::dumpLockInfo
   (
      _dpsTransExecutor * dpsTxExectr,
      const CHAR        * fileName,
      BOOLEAN             bOutputInPlainMode
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
   // dump LRB Header, owner/waiter/upgrade list info
   // to specified file( full path name ), for debugging purpose only
   // This function doesn't need to acquire monitoring/dump
   // latch ( acquireMonLatch() ), it will get the bucket latch
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
   // dump EDU LRB info into VEC_TRANSLOCKCUR
   // It walks through the EDU LRB chain, the caller shall acquire
   // the monitoring( dump ) latch, acquireMonLatch(),
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

         pLRBHdr = _getLRBHdrPtrByIdx( hdrIdx ) ;

         monLock._id    = pLRBHdr->lockId ;
         monLock._mode  = pLRB->lockMode ;
         monLock._count = pLRB->refCounter ;
         monLock._beginTick = pLRB->beginTick ;

         vecLocks.push_back( monLock ) ;

         lrbIdx = pLRB->eduLrbIdxPrev ;
      }
   }


   // dump the lock info ( lockId, lockMode, refCounter ) into monTransLockCur
   // via LRB index
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

         pLRBHdr = _getLRBHdrPtrByIdx( hdrIdx ) ;

         lockCur._id = pLRBHdr->lockId ;
         lockCur._mode = pLRB->lockMode ;
         lockCur._count = pLRB->refCounter ;
         lockCur._beginTick = pLRB->beginTick ;
      }
   }


   //
   // dump LRB Header and owner / waiter / upgrade list for a specific lock
   // into monTransLockInfo. This function doesn't need to acquire
   // monitoring/dump latch ( acquireMonLatch() ), it will get the bucket latch
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
               monLockItem._beginTick = pLRB->beginTick ;

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
               monLockItem._beginTick = pLRB->beginTick ;

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
               monLockItem._beginTick = pLRB->beginTick ;

               monLockInfo._vecWaiter.push_back( monLockItem ) ;

               lrbIdx = pLRB->nextLRBIdx ;
            }
         }
      }

      // release bucket latch
      _releaseOpLatch( bktIdx ) ;
   }


}  // namespace engine
