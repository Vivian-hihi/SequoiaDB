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

   Source File Name = monClass.cpp

   Descriptive Name = Monitor Class source

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for OSS operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/24/2019  CW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "monClass.hpp"
#include "pmd.hpp"

namespace engine
{

BOOLEAN (*MonClassArchiveFP[MON_CLASS_MAX])(MonClass*) = {
   noArchive,
   noArchive,
   archiveQuery
};

MonClass::MonClass()
   : _createTS(pmdGetKRCB()->getCurTime()),
     _status(MON_CLASS_STATUS_NORMAL),
     _type(MON_CLASS_MAX)
{}

MonClass::~MonClass() {}

/**
 * add an object to the active list.
 *
 * @param obj object to be added to the active list
 */
void MonClassContainer::add ( MonClass *obj )
{
   ossScopedLock l( &activeListHeadLatch, EXCLUSIVE ) ;
   activeList.push_front( *obj ) ;
   _activeListLen.inc() ;
}

/**
 * Async remove an object from the active list. The
 * object is either marked as pending archive or pending delete
 * depending on the container's archive rule.
 *
 * @param obj object to be removed.
 */
void MonClassContainer::remove ( MonClass *obj )
{
   if ( (*doArchive)( obj ) )
   {
      obj->setPendingArchive() ;
      _numPendingArchive.inc() ;
   }
   else
   {
      obj->setPendingDelete() ;
      _numPendingDelete.inc() ;
   }
}

/**
 * Process pending delete or pending archive object
 */
void MonClassContainer::_processPendingObj()
{
   getListLatch( EXCLUSIVE ) ;
   getArchiveLatch( EXCLUSIVE ) ;
   getHeadLatch( EXCLUSIVE ) ;

   MonClassList::iterator it = activeList.begin() ;

   while ( it != activeList.end() )
   {
      if ( TRUE == it->isPendingArchive() )
      {
         MonClass &obj = *it ;
         activeList.erase(it) ;
         archivedList.push_back(obj) ;
      }
      else if ( TRUE == it->isPendingDelete() )
      {
         activeList.erase(it) ;
      }
      else
      {
         it++ ;
      }
   }
   //TODO: fix this
   releaseHeadLatch( EXCLUSIVE ) ;
   releaseArchiveLatch( EXCLUSIVE ) ;
   releaseListLatch( EXCLUSIVE ) ;
}

/**
 * archive query based on response time
 * TODO: dummy function for now
 */
BOOLEAN archiveQuery ( MonClass *obj )
{
   return TRUE ;
}

BOOLEAN noArchive ( MonClass *obj )
{
   return FALSE ;
}


// MonClassScanner implements

void MonClassReadScanner::initScan()
{
   _container->getListLatch( SHARED ) ;
   _hasListLatch = TRUE ;

   if ( _listType == MON_CLASS_ARCHIVED_LIST )
   {
      _container->getArchiveLatch( SHARED ) ;
      _hasArchiveLatch = TRUE ;
   }

   _container->getHeadLatch( SHARED ) ;
   _hasHeadLatch = TRUE ;

   _currentList = MON_CLASS_ACTIVE_LIST ;

   // If reading active list, or (if reading archive list and
   // there is pending archive in the active list)
   if ( ( _listType == MON_CLASS_ACTIVE_LIST &&
          _container->getActiveListLen() > 0 ) ||
        _container->getNumPendingArchive() > 0 )
   {
      itr = _container->begin( _currentList ) ;
   }
   else
   {
      if ( _listType == MON_CLASS_ARCHIVED_LIST &&
           _container->getArchivedListLen() > 0 )
      {
         _currentList = MON_CLASS_ARCHIVED_LIST ;
         itr = _container->begin( _currentList ) ;
      }
      else
      {
         _endReached = TRUE ;
      }
   }
   _initCalled = TRUE ;
}

MonClass* MonClassReadScanner::getNext()
{
   MonClass *ret = NULL ;
   BOOLEAN found = FALSE ;
   if ( !_initCalled )
   {
      initScan() ;
   }

   // If we have not reached the end of the lists
   while (!_endReached && !found)
   {
      // 1. We skip any pending deletes
      // 2. Skip if this is a pending archive and we are only interested in the active list
      if ( itr->isPendingDelete() ||
           (itr->isPendingArchive() && _listType == MON_CLASS_ACTIVE_LIST ) )
      {
         itr++ ;
      }
      else
      {
         // Found a match, return
         ret = &(*itr) ;
         itr++ ;
         found = TRUE ;
      }

      // Reached the end of the current list, move to the next list
      // if there is one
      if ( itr == _container->end(_currentList) )
      {
         if ( _currentList == MON_CLASS_ACTIVE_LIST &&
              _listType == MON_CLASS_ARCHIVED_LIST &&
              _container->getArchivedListLen() > 0 )
         {
            itr = _container->begin( MON_CLASS_ARCHIVED_LIST ) ;
            _currentList = MON_CLASS_ARCHIVED_LIST ;
         }
         else
         {
            _endReached = TRUE ;
         }
      }
   }

   // The headLatch is required when reading the first node of the active list.
   // By the time we get here, it is guaranteed that we have already read the first node.
   // So we can now release the headLatch.
   if ( _hasHeadLatch )
   {
      _container->releaseHeadLatch(SHARED) ;
      _hasHeadLatch = FALSE ;
   }

   return ret ;
}

void MonClassReadScanner::doneScan()
{
   if (_hasListLatch)
   {
      _container->releaseListLatch(SHARED) ;
   }
   if (_hasHeadLatch )
   {
      _container->releaseHeadLatch(SHARED) ;
   }
   if ( _hasArchiveLatch)
   {
      _container->releaseArchiveLatch(SHARED) ;
   }
}

// MonClassWriteScanner implements
/* TODO: to be completed
void MonClassWriteScanner::initScan()
{
   if ( _listType == MON_CLASS_ARCHIVED_LIST )
   {
      _container->getArchiveLatch() ;
      _hasArchiveLatch = TRUE ;
   }
   else
   {
      _container->getListLatch( EXCLUSIVE ) ;
      _hasListLatch = TRUE ;

      _container->getHeadLatch( EXCLUSIVE ) ;
      _hasHeadLatch = TRUE ;
   }

   if ( _listType == MON_CLASS_ACTIVE_LIST &&
        _container->getActiveListLen() > 0 )
   {
      itr = _container->begin( _listType ) ;
   }
   else if ( _listType == MON_CLASS_ARCHIVED_LIST &&
             _container->getArchivedListLen() > 0 )
   {
      itr = _container->begin( _listType ) ;
   }
   else
   {
      _endReached = TRUE ;
   }

   _initCalled = TRUE ;
}

MonClass* MonClassWriteScanner::getNext()
{
   MonClass *ret = NULL ;
   BOOLEAN found = FALSE ;

   if ( !_initCalled )
   {
      initScan() ;
   }

   // If we have not reached the end of the lists
   while (!_endReached && !found)
   {
      // 1. We skip any pending deletes
      // 2. Skip if this is a pending archive and we are only interested in the active list
      if ( itr->isPendingDelete() || itr->isPendingArchive() ||
           _listType == MON_CLASS_ARCHIVED_LIST )
      {
         // Found a match, return
         ret = &(*itr) ;
         itr++ ;
         found = TRUE ;
      }
      else
      {
         itr++ ;
      }

      if ( itr == _container->end(_listType) )
      {
         _endReached = TRUE ;
      }
   }

   // The headLatch is required when reading the first node of the active list.
   // By the time we get here, it is guaranteed that we have already read the first node.
   // So we can now release the headLatch.
   if ( _hasHeadLatch )
   {
      _container->releaseHeadLatch(SHARED) ;
      _hasHeadLatch = FALSE ;
   }

   return ret ;
}
void MonClassWriteScanner::doneScan()
{
   if (_hasListLatch)
   {
      _container->releaseListLatch(EXCLUSIVE) ;
   }
   if (_hasHeadLatch )
   {
      _container->releaseHeadLatch(EXCLUSIVE) ;
   }
   if ( _hasArchiveLatch)
   {
      _container->releaseArchiveLatch() ;
   }
}
*/
} // namespace engine
