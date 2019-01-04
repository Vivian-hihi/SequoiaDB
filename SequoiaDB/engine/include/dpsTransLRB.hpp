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
      UINT32 refCounter ;           // lock reference counter
      DPS_TRANSLOCK_TYPE lockMode ; // lock mode, UINT8, 1 byte
      UINT8 pad[3] ;                // 3 byte for padding
   } ;


   // Lock Request Block Header ( LRB Header )
   class dpsTransLRBHeader : public SDBObject
   {
   public :
      UTIL_OBJIDX nextLRBHdrIdx ;// index of next LRB Header in the chain
      UTIL_OBJIDX ownerLRBIdx ;  // index of the first owner LRB in its chain
      UTIL_OBJIDX waiterLRBIdx ; // index of the first waiter LRB in its chain
      UTIL_OBJIDX upgradeLRBIdx; // index of the first upgrader LRB in its chain
      dpsTransLockId lockId ;    // lockId, 16 bytes
   } ;


   class dpsTransLRBHeaderHash : public SDBObject
   {
   public :
      UTIL_OBJIDX    lrbHdrIdx    ; // index of 1st LRB Header in the chain
#if defined ( _LINUX )
      ossRWLatchNS   hashHdrLatch ;
#else
      ossSpinSLatch  hashHdrLatch ;
#endif
   } ;
#pragma pack()
}

#endif // DPSTRANSLRB_HPP_

