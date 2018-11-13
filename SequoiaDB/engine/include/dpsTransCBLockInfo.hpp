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

   Source File Name = dpsTransCBLockInfo.hpp

   Descriptive Name = Operating System Services Types Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPS_TRANSCBLOCKINFO_HPP__
#define DPS_TRANSCBLOCKINFO_HPP__

#include "dpsTransLockDef.hpp"
#include <map>
#include "ossAtomic.hpp"
#include "msg.h"

namespace engine
{
   class _pmdEDUCB ;

   typedef std::map<UINT32, MsgRouteID>      DpsTransNodeMap ;

   /*
      _dpsTransCBLockInfo define
   */
   class _dpsTransCBLockInfo : public SDBObject
   {
   public:
      _dpsTransCBLockInfo( DPS_TRANSLOCK_TYPE lockType ) ;
      ~_dpsTransCBLockInfo() ;

      INT64    incRef() ;
      INT64    decRef() ;
      BOOLEAN  isLockMatch( DPS_TRANSLOCK_TYPE type ) const ;
      DPS_TRANSLOCK_TYPE   getType() const ;
      void                 setType( DPS_TRANSLOCK_TYPE lockType ) ;
      _pmdEDUCB*           getNextWaitCB();
      void                 setNextWaitCB( _pmdEDUCB *pWaitCB ) ;

   private:
      _pmdEDUCB                  *_pNextWaitCB ;
      DPS_TRANSLOCK_TYPE         _lockType ;
      ossAtomicSigned64          *_pRef ;
   } ;
   typedef _dpsTransCBLockInfo dpsTransCBLockInfo ;
   typedef std::map< dpsTransLockId, dpsTransCBLockInfo * > DpsTransCBLockList ;

}

#endif // DPS_TRANSCBLOCKINFO_HPP__

