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

   Source File Name = rtnDiskIXScanner.hpp

   Descriptive Name = RunTime Disk Index Scanner Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for index
   scanner, which is used to traverse index tree.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNDISKIXSCANNER_HPP__
#define RTNDISKIXSCANNER_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ixm.hpp"
#include "ixmExtent.hpp"
#include "rtnPredicate.hpp"
#include "../bson/ordering.h"
#include "../bson/oid.h"
#include "dms.hpp"
#include "monCB.hpp"
#include "rtnIXScanner.hpp"
#include "dmsStorageUnit.hpp"
#include <set>

namespace engine
{
   class _rtnIXScanner ;
   class _dmsStorageUnit ;
   class _pmdEDUCB;

   /*
      _rtnDiskIXScanner define
      A scanner to traverse through on disk index pages/slots
   */
   class _rtnDiskIXScanner : public rtnIXScanner
   {
   private :
      // predicate list and iterator points to the verbs from access plan
      rtnPredicateList        *_predList ;
      rtnPredicateListIterator _listIterator ;
      Ordering                 _order ;

      //FIXME: to remove
      // indicate if the scanner need to wait for lock
      //BOOLEAN                  _transLockwait;

      // keep index and disk storage related information so that we can
      // access extent on disk to validate if the index is still valid.
      // We can not simply rely on cached pointers to the in memory tree
      // to tell if the index have changed or not. The easiest way is to
      // look up the extent header of the root page. The alternative is to
      // go through the lock manager and follow through the lrbHdr.
      // However, another benefit of having following two poiter is to build
      // ixmExtent so that we can reuse its keyCmp function.(can we make the
      // keyCmp() static function instead)
      ixmIndexCB              *_indexCB ;
      _dmsStorageUnit         *_su ;

      monContextCB            *_pMonCtxCB ;
      _pmdEDUCB               *_cb ;
      INT32                    _direction ;
      // track the extent/slot of current scan
      ixmRecordID              _curIndexRID ;
      // Flag to indicate if we need to locate/relocate the key
      BOOLEAN                  _init ;

      // need to store a duplicate buffer for dmsRecordID for each ixscan,
      // because each record may be refered by more than one index key (say
      // {c1:[1,2,3,4,5]}, there will be 5 index keys pointing to the same
      // record), so index scan may return the same record more than one time,
      // which should be avoided. Thus we keep a set ( maybe we can further
      // improve performance by customize data structure ) to record id that
      // already scanned.
      // note that a record id will not be changed during update because it
      // will use overflowed record with existing record head
      scannerSharedInfo        _sharedInfo;
/*
      UINT64                   _dedupBufferSize ;
      dmsRecIDSet             *_dupBuffer ;
*/
      // after the caller release X latch, another session may change the index
      // structure. So everytime before releasing X latch, the caller must store
      // the current obj, and then verify if it's still remaining the same after
      // next check. If the object doens't match the on-disk obj, something must
      // goes changed and we should relocate
      BSONObj                  _savedObj ;
      dmsRecordID              _savedRID ;

      BSONObj                  _curKeyObj ;

      OID                      _indexOID ;
      dmsExtentID              _indexCBExtent ;
      dmsExtentID              _indexLID ;

      BOOLEAN                  _isReadOnly ;

      BOOLEAN                  _initialized ;

      BufBuilder               _builder;
   public :
      _rtnDiskIXScanner ( ixmIndexCB *indexCB, rtnPredicateList *predList,
                          _dmsStorageUnit *su, _pmdEDUCB *cb,
                          scannerSharedInfo *sharedInfo=NULL ) ;

      ~_rtnDiskIXScanner () ;

   protected :
      virtual INT32 _isCursorSame( ixmExtent *pExtent,
                              const BSONObj &saveObj,
                              const dmsRecordID &saveRID,
                              BOOLEAN &isSame,
                              BOOLEAN *hasRead = NULL )  ;
   public :
      void setup( ixmIndexCB *indexCB, rtnPredicateList *predList,
                   _dmsStorageUnit *su, _pmdEDUCB *cb ) ;

      void setMonCtxCB ( monContextCB *monCtxCB )
      {
         _pMonCtxCB = monCtxCB ;
      }

      void setIsReadOnly( BOOLEAN readOnly )
      {
         _isReadOnly = readOnly;
      }
/*  FIXME:
      void setNeedWaitForLock( BOOLEAN w )
      {
         _transLockwait = w;
      }
*/
      const BSONObj * getSavedObj () const
      {
         return &_savedObj ;
      }

      dmsRecordID getSavedRID () const
      {
         return _savedRID ;
      }
/*
      const BOOLEAN needWaitForLock() const
      {
         return _transLockwait ;
      }
*/
      ixmRecordID getCurIndexRID () const
      {
         return _curIndexRID ;
      }

      const BSONObj* getCurKeyObj () const
      {
         return &_curKeyObj ;
      }

      INT32 getDirection () const
      {
         return _direction ;
      }

      dmsExtentID getIdxLID() const
      {
         return _indexLID ;
      }

      dmsStorageUnitID getCSID() const
      {
         return _su->CSID();
      }

      rtnPredicateList* getPredicateList ()
      {
         return _predList ;
      }

      INT32 compareWithCurKeyObj ( const BSONObj &keyObj ) const
      {
         return _curKeyObj.woCompare( keyObj, _order, false ) * _direction ;
      }

      dmsRecIDSet * getDupBuff()
      {
         return _sharedInfo.getDupBuf();
      }

      const BOOLEAN initialized () const
      {
         return _initialized;
      }

      const BOOLEAN isValid () const
      {
         // when we haven't done first run, consider it as valid
         return ( _isValid || !_init );
      }

      // clear buffer and set the scanner to initial state
      void reset()
      {
         _curIndexRID.reset() ;
         _listIterator.reset() ;
         if ( _sharedInfo.isLocal() )
         {
            _sharedInfo.getDupBuf()->clear() ;
         }
         _init    = FALSE ;
      }
      // save the bson key for the current index rid, before releasing X latch
      // on the collection
      // we have to call resumeScan after getting X latch again just in case
      // other sessions changed tree structure
      INT32 pauseScan( ) ;
      // restoring the bson key and rid for the current index rid. This is done
      // by comparing the current key/rid pointed by _curIndexRID still same as
      // previous saved one. If so it means there's no change in this key, and
      // we can move on the from the current index rid. Otherwise we have to
      // locate the new position for the saved key+rid
      INT32 resumeScan( );

      // return SDB_IXM_EOC for end of index
      // otherwise rid is set to dmsRecordID
      INT32 advance ( dmsRecordID &rid ) ;
      INT32 relocateRID () ;
      INT32 relocateRID ( const BSONObj &keyObj,
                          const dmsRecordID &rid,
                          const BOOLEAN resetWithIndexPos ) ;

      virtual INT32 isCursorSame( const BSONObj &saveObj,
                                  const dmsRecordID &saveRID,
                                  BOOLEAN &isSame ) ;

      virtual void  removeDuplicatRID( const dmsRecordID &rid ) ;

      virtual const BOOLEAN wasFromMemTreeScan () const { return FALSE ; }

   protected:
      rtnPredicateListIterator* getPredicateListInterator ()
      {
         return &_listIterator ;
      }

   } ;
   typedef class _rtnDiskIXScanner rtnDiskIXScanner ;

}

#endif //RTNDISKIXSCANNER_HPP__

