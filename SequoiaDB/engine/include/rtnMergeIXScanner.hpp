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

   Source File Name = rtnMergeIXScanner.hpp

   Descriptive Name = RunTime Index Scanner Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for index
   scanner, which is used to traverse index tree.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/09/2018  YXC Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNMERGEIXSCANNER_HPP__
#define RTNMERGEIXSCANNER_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ixm.hpp"
#include "rtnPredicate.hpp"
#include "pmdEDU.hpp"
#include "../bson/ordering.h"
#include "../bson/oid.h"
#include "dms.hpp"
#include "monCB.hpp"
#include "rtnIXScanner.hpp"
#include "dmsStorageUnit.hpp"
#include "dmsTransLockCallback.hpp"


namespace engine
{
   class _rtnIXScanner;


   // Index merge scanner is used to merge results from different scanner.
   // For instance, we have a scanner to traverse on disk index trees; we
   // also have in memory index tree to track previously committed indexes.
   // As part of index scan, if a transaction require RC level isolation, 
   // it will use this merge scanner to retrieve the last committed version
   // record including the data on disk and in memory. In this case, we can
   // instantiate the two scanner member to rtnDiskScanner and 
   // rtnMemIXTreeScanner
   class _rtnMergeIXScanner : public rtnIXScanner
   {
      private:
      // indicate if the transaction need to wait for lock
//      BOOLEAN           _transLockwait;

      // flag indicating if the last index used was from left scanner
      BOOLEAN           _wasFromLeft;
      BOOLEAN           _firstRun;

      INT32             _direction ;
      Ordering          _order ;

      dmsRecordID       _lrid;   // for memIXScan return
      dmsRecordID       _rrid;   // for DiskScan return

      scannerSharedInfo _sharedInfo;  // shared information for both scanners

      // After releas mbLatch, another session may change the index structure
      // So everytime before pause, we must store the current index key value
      // and the rid. We must keep this information in shared structure in
      // merge scan because the memory tree scan might not exist in previous
      // interval and now the mem tree was created due to new update. We need
      // the saved information to locate the key.
      // After resume, verify if it's still remaining the same. If the
      // object/rid doens't match, something must have changed. we should
      // relocate key.
      // Because malloc is involved during the saving, we should only setup
      // the BSONObj during pause(), otherwise it would affect runtime perf.
      BSONObj                   _savedObj;
      dmsRecordID               _savedRID ;

      // pointer to left and right scanners to merge
      rtnIXScanner *    _leftIXScanner;
      rtnIXScanner *    _rightIXScanner;

      public:
      _rtnMergeIXScanner( ixmIndexCB *indexCB,
                          rtnPredicateList *predList,
                          _dmsStorageUnit *su, _pmdEDUCB *cb,
                          scannerSharedInfo * sharedInfo = NULL );

      ~_rtnMergeIXScanner();

      void setIsReadOnly( BOOLEAN readOnly )
      {
         if ( _leftIXScanner )
         {
            _leftIXScanner->setIsReadOnly( readOnly );
         }
         if ( _rightIXScanner )
         {
            _rightIXScanner->setIsReadOnly( readOnly );
         }
      }

      void setMergeScanner( IXScannerType ltype, IXScannerType rtype,
                            ixmIndexCB *indexCB,
                            rtnPredicateList *predList,
                            dmsStorageUnit *su, pmdEDUCB *cb );

      rtnIXScanner * left()
      {
         return _leftIXScanner;
      }

      rtnIXScanner * right()
      {
         return _rightIXScanner;
      }

      void setMonCtxCB ( monContextCB *monCtxCB )
      {
         if ( _leftIXScanner )
         {
            _leftIXScanner->setMonCtxCB( monCtxCB );
         }
         if ( _rightIXScanner )
         {
            _rightIXScanner->setMonCtxCB( monCtxCB );
         }
      }

      const BSONObj * getSavedObj () const { return &_savedObj ; }

      dmsRecordID getSavedRID () const { return _savedRID; }

      const BSONObj * getSavedObjFromChild () const
      {
         return _wasFromLeft 
              ?  _leftIXScanner->getSavedObj()
              :  _rightIXScanner->getSavedObj();
      }

      dmsRecordID getSavedRIDFromChild () const
      {
         return _wasFromLeft 
              ?  _leftIXScanner->getSavedRID()
              :  _rightIXScanner->getSavedRID() ;
      }

      scannerSharedInfo * getSharedInfo() 
      {
         return &_sharedInfo;
      }

      INT32 getDirection () const
      {
         return _direction;
      }

      const BSONObj* getCurKeyObj() const
      {
         SDB_ASSERT( _leftIXScanner && _rightIXScanner, 
                    " merge scanner is not setup properly " );
         return  _wasFromLeft ? _leftIXScanner->getCurKeyObj() :
                                _rightIXScanner->getCurKeyObj();
      }

      dmsExtentID getIdxLID() const
      {
         SDB_ASSERT( _leftIXScanner && _rightIXScanner, 
                    " merge scanner is not setup properly " );
         // left and right should be built with the same indexCB, thus same LID
         return _leftIXScanner->getIdxLID();
      }

      // This merges result from diskScanner and _memIXScanner
      INT32 advance ( dmsRecordID &rid );
      INT32 pauseScan(  );
      INT32 resumeScan( );
      // do not call these, it's defined and called in individual scanner
      INT32 relocateRID ()
      {
         return SDB_SYS;
      };

      INT32 relocateRID ( const BSONObj &keyObj, const dmsRecordID &rid,
                          const BOOLEAN resetWithIndexPos = FALSE )
      {
         return SDB_SYS;
      };

      const BOOLEAN initialized() const 
      {
         BOOLEAN rc = FALSE;
         if ( (_rightIXScanner && _rightIXScanner->initialized())  ||
              (_leftIXScanner && _leftIXScanner->initialized()) )
         {
            rc = TRUE;
         }
         return rc;
      }

      dmsRecIDSet * getDupBuff()
      {
         return _sharedInfo.getDupBuf();
      }

      const MEMTREE_LATCH_MODE getMemtreeLatchMode()
      {
         return _leftIXScanner->getMemtreeLatchMode(); 
      }

      virtual INT32 isCursorSame( const BSONObj &saveObj,
                                  const dmsRecordID &saveRID,
                                  BOOLEAN &isSame )
      {
         return _wasFromLeft  
             ?  _leftIXScanner->isCursorSame( saveObj, saveRID, isSame ) 
             :  _rightIXScanner->isCursorSame( saveObj, saveRID, isSame );
      }

      virtual void  removeDuplicatRID( const dmsRecordID &rid ) 
      {
         // do local remove inside merge Scanner
         _sharedInfo.getDupBuf()->erase( rid ) ;
      }

   };
   typedef class _rtnMergeIXScanner rtnMergeIXScanner ;
}

#endif //RTNMERGEIXSCANNER_HPP__
