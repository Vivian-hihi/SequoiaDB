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

   Source File Name = rtnIXScanner.hpp

   Descriptive Name = RunTime Index Scanner Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for index
   scanner, which is used to traverse index tree.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/09/2018  YXC Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNIXSCANNER_HPP__
#define RTNIXSCANNER_HPP__

#include "oss.hpp"
#include "dms.hpp"
#include "ixm.hpp"
#include "monCB.hpp"
#include "rtnPredicate.hpp"
#include "../bson/ordering.h"
#include "../bson/oid.h"

using namespace bson ;

namespace engine
{
   class _dmsStorageUnit;
   class _pmdEDUCB;

   #define RTN_IXSCANNER_DEDUPBUFSZ_DFT      (1024*1024)
 
   // define type of index scanners
   enum IXScannerType
   {
      SCANNER_TYPE_DISK      = 0,
      SCANNER_TYPE_MEM_TREE,
      SCANNER_TYPE_MERGE,
      SCANNER_TYPE_MAX
   };

   class _scannerSharedInfo
   {
      public:
      ~_scannerSharedInfo()
      {
         this->free();
      }

      INT32 init()
      {
         INT32 rc = SDB_OK;
         _dedupBufferSize = RTN_IXSCANNER_DEDUPBUFSZ_DFT;
         _savedRID.reset();
         _dupBuffer = new dmsRecIDSet();
         SDB_ASSERT( _dupBuffer, "_dupBuffer creation failed" );
         if( _dupBuffer == NULL )
         {
            rc = SDB_OOM;
         }
         _bufIsLocal = TRUE;
         return rc;
      }

      void init( _scannerSharedInfo* sharedInfo )
      {
         this->_dedupBufferSize = sharedInfo->getBufSize();
         this->_dupBuffer = sharedInfo->getDupBuf();
         this->_bufIsLocal = FALSE;
         _savedRID.reset();
      }

      void free()
      {
         if ( _bufIsLocal && (_dupBuffer != NULL) )
         {
            delete _dupBuffer;
            _dupBuffer = NULL;
         }
      }


      void update();


      dmsRecIDSet * getDupBuf() { return _dupBuffer; }

      UINT64 getBufSize()  { return _dedupBufferSize; }

      private:
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
      UINT64                    _dedupBufferSize ;
      dmsRecIDSet              *_dupBuffer ;
      BOOLEAN                   _bufIsLocal;

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

   };
   typedef class _scannerSharedInfo scannerSharedInfo;

   /*
      _rtnIXScanner define
      This is a super class for all index scanners to inherit from
   */
   class _rtnIXScanner : public SDBObject
   {
      
      public:
      _rtnIXScanner() {};
      _rtnIXScanner( ixmIndexCB       *indexCB, 
                     rtnPredicateList *predList,
                     _dmsStorageUnit  *su, 
                     _pmdEDUCB        *cb,
                     scannerSharedInfo * sharedInfo = NULL ) {};

      virtual ~_rtnIXScanner() {};

      public:
      virtual INT32 advance ( dmsRecordID &rid ) = 0;
      virtual INT32 resumeScan( ) = 0 ;
      virtual INT32 pauseScan( ) = 0;
      virtual void setIsReadOnly(BOOLEAN v) = 0;
      virtual void setMonCtxCB(monContextCB *monCtxCB) = 0;

      virtual INT32 relocateRID () = 0;
      virtual INT32 relocateRID ( const BSONObj &keyObj, const dmsRecordID &rid ) = 0 ;

      virtual const BOOLEAN initialized() const = 0;
      virtual INT32 getDirection () const = 0 ;
      virtual const BSONObj* getCurKeyObj() const = 0;
      virtual dmsExtentID getIdxLID() const = 0;

      virtual dmsRecordID getSavedRID () const = 0;
      virtual const BSONObj * getSavedObj () const = 0;

      // Dummy virtual functions. Defined here so that not all derived 
      // class need to implement them. But we are not suppose to invoke them
      virtual INT32 compareWithCurKeyObj ( const BSONObj &keyObj ) const 
                    {return 0;}
      virtual rtnPredicateList* getPredicateList () { return NULL; }
      virtual dmsRecIDSet * getDupBuff() { return NULL; }
 
      const UINT32 type() const
      {
         return _type;
      }

      const BOOLEAN eof() const
      {
         return _eof;
      }

      virtual INT32 isCursorSame( const BSONObj &saveObj,
                                  const dmsRecordID &saveRID,
                                  BOOLEAN &isSame ) = 0 ;

      virtual void  removeDuplicatRID( const dmsRecordID &rid ) = 0;


      protected:
      IXScannerType  _type;
      BOOLEAN        _eof;

   };
   typedef class _rtnIXScanner rtnIXScanner ;


}

#endif //RTNIXSCANNER_HPP__

