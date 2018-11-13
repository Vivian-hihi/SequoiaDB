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

   Source File Name = dpsTransCBLockInfo.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "dpsTransCBLockInfo.hpp"
#include "dms.hpp"
#include "dmsStorageUnit.hpp"

using namespace bson ;

namespace engine
{
   /*
      _dpsTransCBLockInfo implement
   */
   _dpsTransCBLockInfo::_dpsTransCBLockInfo( DPS_TRANSLOCK_TYPE lockType )
   : _lockType( lockType )
   {
      _pNextWaitCB = NULL ;
      _pRef = SDB_OSS_NEW ossAtomicSigned64(0) ;
   }

   _dpsTransCBLockInfo::~_dpsTransCBLockInfo()
   {
      if ( _pRef )
      {
         SDB_OSS_DEL _pRef ;
      }
   }

   INT64 _dpsTransCBLockInfo::incRef()
   {
      return _pRef->inc();
   }

   INT64 _dpsTransCBLockInfo::decRef()
   {
      return _pRef->dec();
   }

   BOOLEAN _dpsTransCBLockInfo::isLockMatch( DPS_TRANSLOCK_TYPE type ) const
   {
      return dpsLockCoverage( _lockType, type ) ;
   }

   DPS_TRANSLOCK_TYPE _dpsTransCBLockInfo::getType() const
   {
      return _lockType ;
   }

   void _dpsTransCBLockInfo::setType( DPS_TRANSLOCK_TYPE lockType )
   {
      _lockType = lockType ;
   }

   _pmdEDUCB *_dpsTransCBLockInfo::getNextWaitCB()
   {
      return _pNextWaitCB ;
   }

   void _dpsTransCBLockInfo::setNextWaitCB( _pmdEDUCB *pWaitCB )
   {
      _pNextWaitCB = pWaitCB ;
   }

}
