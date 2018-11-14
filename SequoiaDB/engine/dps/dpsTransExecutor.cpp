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

   Source File Name = dpsTransExecutor.cpp

   Descriptive Name = Operating System Services Types Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/08/2018  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "dpsTransExecutor.hpp"


namespace engine
{

   /*
      _dpsTransExecutor implement
   */
   _dpsTransExecutor::_dpsTransExecutor()
   {
      _waiterIdx        = UTIL_INVALID_OBJ_INDEX ;
      _waiterQueType    = DPS_QUE_NULL ;
      _lastLRBIdx       = UTIL_INVALID_OBJ_INDEX ;
   }

   _dpsTransExecutor::~_dpsTransExecutor()
   {
   }

   void _dpsTransExecutor::clearAll()
   {
      clearWaiterInfo() ;
      clearLastLRBIdx() ;
      clearLock() ;
   }

   void _dpsTransExecutor::setWaiterInfo( UTIL_OBJIDX lrbIdx,
                                          DPS_TRANS_QUE_TYPE type )
   {
      _waiterIdx     = lrbIdx ;
      _waiterQueType = type ;
   }

   void _dpsTransExecutor::clearWaiterInfo()
   {
      _waiterIdx     = UTIL_INVALID_OBJ_INDEX ;
      _waiterQueType = DPS_QUE_NULL ;
   }

   UTIL_OBJIDX _dpsTransExecutor::getWaiterLRBIdx() const
   {
      return _waiterIdx ;
   }

   DPS_TRANS_QUE_TYPE _dpsTransExecutor::getWaiterQueType() const
   {
      return _waiterQueType ;
   }

   void _dpsTransExecutor::setLastLRBIdx( UTIL_OBJIDX lrbIdx )
   {
      _lastLRBIdx = lrbIdx ;
   }

   void _dpsTransExecutor::clearLastLRBIdx()
   {
      _lastLRBIdx = UTIL_INVALID_OBJ_INDEX ;
   }

   UTIL_OBJIDX _dpsTransExecutor::getLastLRBIdx() const
   {
      return _lastLRBIdx ;
   }

   BOOLEAN _dpsTransExecutor::addLock( const dpsTransLockId &lockID,
                                       UTIL_OBJIDX lrbIdx )
   {
      BOOLEAN hasAdd = FALSE ;

      /// Not leaf level
      if ( !lockID.isLeafLevel() )
      {
         if ( _mapLockID.insert( std::make_pair( lockID, lrbIdx ) ).second )
         {
            hasAdd = TRUE ;
         }
      }

      return hasAdd ;
   }

   BOOLEAN _dpsTransExecutor::findLock( const dpsTransLockId &lockID,
                                        UTIL_OBJIDX &lrbIdx ) const
   {
      if ( lockID.isLeafLevel() )
      {
         return FALSE ;
      }

      DPS_LOCKID_MAP_CIT cit = _mapLockID.find( lockID ) ;
      if ( cit != _mapLockID.end() )
      {
         lrbIdx = cit->second ;
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _dpsTransExecutor::removeLock( const dpsTransLockId &lockID )
   {
      return _mapLockID.erase( lockID ) ? TRUE : FALSE ;
   }

   void _dpsTransExecutor::clearLock()
   {
      _mapLockID.clear() ;
   }

}

