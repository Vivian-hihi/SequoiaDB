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

   Source File Name = rtnMemIXScanner.hpp

   Descriptive Name = RunTime Index Scanner Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for index
   scanner, which is used to traverse index tree.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/08/2012  YXC Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNMEMIXTREESCANNER_HPP__
#define RTNMEMIXTREESCANNER_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ixm.hpp"
#include "rtnPredicate.hpp"
#include "pmdEDU.hpp"
#include "../bson/ordering.h"
#include "../bson/oid.h"
#include "rtnIXScanner.hpp"
#include "dpsTransLockMgr.hpp"
#include "dmsStorageUnit.hpp"
#include "monCB.hpp"
#include "dmsTransLockCallback.hpp"

namespace engine
{
   class _rtnIXScanner;
   class _dmsStorageUnit;

   /*
      _rtnMemIXTreeScanner define
      This scanner is used to scan the in memory index tree which holds
      old copy (last committed) of index values. During runtime, index scan
      will merge the result from this scan with the rtnDiskIXScanner.
      See rtnMergeIXScanner for details
   */
   class _rtnMemIXTreeScanner : public rtnIXScanner
   {
   private :
      // a local indexCB for index validation purpose
      ixmIndexCB               *_indexCB ;
      rtnPredicateList         *_predList ;
      rtnPredicateListIterator  _listIterator ;
      Ordering                  _order ;

      _dmsStorageUnit          *_su ;
      monContextCB             *_pMonCtxCB ;
      _pmdEDUCB                *_cb ;
      INT32                     _direction ;
      dpsTransCB               *_pTransCB;
      // set to true by relocateRID or first run of advance.
      // indicate if have done first run which will call keyLocate to point
      // _curIndexIter to a best starting point
      BOOLEAN                   _init ; 


      // Track the current index iterator location under proper protection.
      // This could become invalid if the index is erased from the tree
      // even worse case, if someone reused the same memory, they could 
      // set some other information. So this iterator only gives a quick
      // access to previous locationr. As soon as we released the tree latch
      // you can no longer trust the information. We have to re-validate the
      // content (key information) in second against _curKeyObj. If they don't
      // match, we have to do a relocateRID based on savedObj.
      // If we hold tree latch, after calling relocateRID, we can simply 
      // manipulate the iterator towards the direction. We can always store
      // current key value in _curKeyObj for computation/comparison purpose.
      INDEX_BINARY_TREE::iterator _curIndexIter;
      BSONObj                   _curKeyObj ;

      // need to store a duplicate buffer for dmsRecordID for each ixscan,
      // because each record may be refered by more than one index key (say
      // {c1:[1,2,3,4,5]}, there will be 5 index keys pointing to the same
      // record), so index scan may return the same record more than one time,
      // which should be avoided. Thus we keep a set ( maybe we can further
      // improve performance by customize data structure ) to record id that
      // already scanned.
      // note that a record id will not be changed during update because it will
      // use overflowed record with existing record head
      // we used a pointer here because the memIXTreeScanner are likely used
      // together with another scanner (like disk scanner), we should share 
      // the same dupBuffer. 
      scannerSharedInfo         _sharedInfo;
/*
      UINT64                    _dedupBufferSize ;
      dmsRecIDSet              *_dupBuffer ;
*/
      // After releasing mbLatch and treeLatch, another session may change
      // the index structure. So everytime before pause, we must store
      // the current index key value into _savedInMemObj, and the rid into 
      // dmsRecordID. After resume, verify if it's still remaining the same. 
      // If the object doens't match the obj/rid in LRBHdr, something must
      // goes changed and we should relocate
      // Because malloc is involved during the saving, we should only setup 
      // the BSONObj during pause(), otherwise it would affect runtime perf.
      BSONObj                   _savedInMemObj ;
      dmsRecordID               _savedRID ;

      UINT32                    _csID;      // collection space id
      UINT32                    _clID;      // collection id
      OID                       _indexOID ; // index oid, 
      dmsExtentID               _indexLID;  // index logic id

      BOOLEAN                   _isReadOnly;
      // only consider initialized after the in memory index tree is setup
      BOOLEAN                   _initialized;
      // pointer to the in memory index tree. It's hanging off dpsTransCB
      preIdxTree               *_memIdxTree;
      // Protocal to take and hold tree latch:
      // During tree scan (advance), we will hold the treeLatch so that commit
      // or rollback won't remove the nodes from the tree while scanner is 
      // walking over the tree. The latch is aquired by resume and released
      // by pause.
      BOOLEAN                   _treeLatchHeld;
      // if we need to take and hold memIdxTree latch in X or not. Read only
      // scanner take the latch in S, intended update must acquire tree latch
      // in X. Otherwise the update can cause self dead latch
      BOOLEAN                   _latchX;

   public :
      _rtnMemIXTreeScanner ( ixmIndexCB *indexCB, rtnPredicateList *predList,
                             _dmsStorageUnit *su, _pmdEDUCB *cb,
                             scannerSharedInfo * sharedInfo = NULL ) ;
      ~_rtnMemIXTreeScanner () ;

      void setMonCtxCB ( monContextCB *monCtxCB )
      {
         _pMonCtxCB = monCtxCB ;
      }

      void setIsReadOnly( BOOLEAN readOnly )
      {
         _isReadOnly = readOnly;
      }

      const BSONObj* getSavedObj () const
      {
         return &_savedInMemObj;
      }

      dmsRecordID getSavedRID () const
      {
         return _savedRID ;
      }

      dmsExtentID getIdxLID() const
      {
         return _indexLID ;
      }

      INDEX_BINARY_TREE::iterator getCurIndexIter () 
      {
         return _curIndexIter;
      }

      const BSONObj* getCurKeyObj () const
      {
         return &_curKeyObj ;
      }

      INT32 getDirection () const
      {
         return _direction ;
      }

      rtnPredicateList* getPredicateList ()
      {
         return _predList ;
      }

      dmsRecIDSet * getDupBuff()
      {
         return _sharedInfo.getDupBuf();
      }

      const dmsExtentID getIdxLid() const
      {
         return _indexLID;
      }

      const BOOLEAN initialized () const
      {
         return _initialized;
      }

      INT32 compareWithCurKeyObj ( const BSONObj &keyObj ) const
      {
         return _curKeyObj.woCompare( keyObj, _order, false ) * _direction ;
      }

      void reset()
      {
         _listIterator.reset() ;
         //_sharedInfo.getDupBuf()->clear() ;
         _init    = FALSE ;
      }

      const BSONObj  getCurIdxKeyObjFromIter();

      const MEMTREE_LATCH_MODE getMemtreeLatchMode()
      {
         MEMTREE_LATCH_MODE m = MEMTREE_LATCH_NONE;
         if( _treeLatchHeld ) 
         {
            m = _latchX ? MEMTREE_LATCH_X : MEMTREE_LATCH_S;
         }
         return m;
      }

      const BOOLEAN initMemIXScan( dpsTransCB * pTransCB,
                                   UINT32       csID,
                                   UINT32       clID,
                                   scannerSharedInfo *sharedInfo,
                                   BOOLEAN      latchX = FALSE );

      // save the bson key for the current index rid, before releasing X latch
      // on the collection
      // we have to call resumeScan after getting X latch again just in case
      // other sessions changed tree structure
      INT32 pauseScan( );
      // restoring the bson key and rid for the current index rid. This is done
      // by comparing the current key/rid pointed by _curIndexIter still same as
      // previous saved one. If so it means there's no change in this key, and
      // we can move on the from the current index rid. Otherwise we have to
      // locate the new position for the saved key+rid
      INT32 resumeScan( );

      // return SDB_IXM_EOC for end of index
      // otherwise rid is set to dmsRecordID
      INT32 advance ( dmsRecordID &rid ) ;
      INT32 relocateRID () ;
      INT32 relocateRID ( const BSONObj &keyObj, const dmsRecordID &rid ) ;


      virtual INT32 isCursorSame( const BSONObj &saveObj,
                                  const dmsRecordID &saveRID,
                                  BOOLEAN &isSame ) ;

      virtual void  removeDuplicatRID( const dmsRecordID &rid ) 
      {
         _sharedInfo.getDupBuf()->erase( rid ) ;
      }


   } ;
   typedef class _rtnMemIXTreeScanner rtnMemIXTreeScanner ;

}

#endif //RTNMEMIXTREESCANNER_HPP__

