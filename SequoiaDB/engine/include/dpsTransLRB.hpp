/******************************************************************************


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

   Source File Name = dpsTransLRB.hpp

   Descriptive Name = DPS LRB ( lock request block )

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/10/2018  JT  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSTRANSLRB_HPP_
#define DPSTRANSLRB_HPP_

#include "ossLatch.hpp"         // ossSpinXLatch
#include "utilSegment.hpp"      // UTIL_UTIL_OBJIDX
#include "dpsTransLockDef.hpp"  // DPS_TRANSLOCK_TYPE, dpsTransLockId
#include "dpsTransVersionCtrl.hpp" // oldVersionContainer

namespace engine
{
   class _dpsTransExecutor ;
#pragma pack(4)

   // Lock Request Block ( LRB )
   class dpsTransLRB : public SDBObject
   {
   public :
      _dpsTransExecutor * dpsTxExectr ;                
      UTIL_OBJIDX eduLrbIdxNext ;   // idx of next LRB in chain EDU owning in tx
      UTIL_OBJIDX eduLrbIdxPrev ;   // idx of prev LRB in chain EDU owning in tx
      UTIL_OBJIDX lrbHdrIdx ;       // idx of the LRB Header
      UTIL_OBJIDX nextLRBIdx ;      // idx of next LRB in the owner/waiter chain
      ossTick beginTick ;           // timestamp( ossTick ) of owning / waiting
      UINT32 refCounter ;           // lock reference counter
      DPS_TRANSLOCK_TYPE lockMode ; // lock mode, UINT8, 1 byte
      UINT8 pad[3] ;                // 3 byte for padding
   } ;  // 40 bytes in total


   // Lock Request Block Header ( LRB Header )
   class dpsTransLRBHeader : public SDBObject
   {
   public :
      UTIL_OBJIDX nextLRBHdrIdx ;   // index of next LRB Header in the chain
      UTIL_OBJIDX ownerLRBIdx ;     // index of the first owner LRB in its chain
      UTIL_OBJIDX waiterLRBIdx ;    // index of the first waiter LRB in its chain
      UTIL_OBJIDX upgradeLRBIdx;    // index of the first upgrader LRB in its chain
      dpsTransLockId lockId ;  // lockId, 16 bytes
      oldVersionContainer *oldVer ;   // a pointer to the structure containing
                                      // version page/index information
   public :
      // is this lock for a newly created record or not
      BOOLEAN isNewRecord()
      { return oldVer ? oldVer->isRecordNew() : FALSE ;  }

      void setNewRecord()
      {
         if( oldVer )
         { 
            oldVer->setRecordNew();
         }
      }
      void unsetNewRecord()
      {
         if( oldVer )
         { 
            oldVer->unsetRecordNew();
         }
      }

      // access to oldVersionContainer element
      const dmsRecord * getOldRecord() const 
      {
         return oldVer ? oldVer->getOldRecord() : NULL;
      }

      void setOldRecord(dmsRecord * r) { oldVer->setOldRecord(r); }

      MEMBLOCKPOOL_TYPE &oldRecordMemType() { return oldVer->getRecordMemType(); }

      // try to insert to lid set. The caller should hold record lock in X
      // as the protection
      // Return TRUE if succeeded. return FALSE if already exist.
      BOOLEAN  insertOldIdxLid( const  SINT32 lid, ixmIndexCB* indexCB )
      { 
         return oldVer->insertOldIdxLid(lid, indexCB); 
      }

      BOOLEAN idxLidExist(SINT32 id)
      {
         BOOLEAN found  = FALSE;
         if ( oldVer != NULL )
         {
            found = oldVer->idxLidExist(id);
         }
         return found;
      }

      // given logical idx id, return the index value 
      BSONObj* getOldIdxValue(const  SINT32 lid ) 
      { 
         BSONObj * obj = NULL;
         if ( oldVer != NULL )
         {
            obj = oldVer->getOldIdxValue(lid);
         }
         return obj;
      }

      //MEMBLOCKPOOL_TYPE &oldIdxMemType() { return oldVer->_idxMemType; }
   } ;  // 48 bytes in total


   class dpsTransLRBHeaderHash : public SDBObject
   {
   public :
      UTIL_OBJIDX    lrbHdrIdx    ; // index of 1st LRB Header in the chain
      ossSpinXLatch  hashHdrLatch ; // ossSpinXLatch, 48 bytes
   } ; // 56 bytes in total
#pragma pack()
}

#endif // DPSTRANSLRB_HPP_

