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

   Source File Name = monMgr.hpp

   Descriptive Name = Monitor Services Header

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

#ifndef MONMGR_HPP_
#define MONMGR_HPP_

#include <vector>
#include "monClass.hpp"

namespace engine
{

#define MON_GROUP_MASK_DEFAULT 0
#define MON_GROUP_QUERY_BASIC 0x0000001
#define MON_GROUP_QUERY_DETAIL 0x00000002
#define MON_GROUP_LATCH_BASIC 0x00000004
#define MON_GROUP_LATCH_DETAIL 0x00000008
#define MON_GROUP_LOCK_BASIC  0x00000010
#define MON_GROUP_LOCK_DETAIL 0x00000020

/**
 * A monitor manager responsible for memory management and metric management
 * of the monitor classes
 */
class _MonitorManager
{
   // disable assignment/copy
   _MonitorManager& operator= (const _MonitorManager&) ;
   _MonitorManager(_MonitorManager&) ;

public:
   _MonitorManager() ;

   ~_MonitorManager() ;

   /**
    * Register a new monitor object
    *
    * @param classType the type of monitor object getting registered
    * @return T a pointer to the new monitor object
    */
   template<class T>
   T* registerMonitorObject()
   {
      //TODO need to verify T is a subclass of MonClass
      MonitorClassType classType = T::getType() ;
      T* ptr = _monClass[classType]->add<T>();
      return ptr ;
   }

   /**
    * Register a new monitor object
    *
    * @param classType the type of monitor object getting registered
    * @return T a pointer to the new monitor object
    */
   template<class T>
   T* registerMonitorObject(_MonClassBaseData *data)
   {
      //TODO need to verify T is a subclass of MonClass
      MonitorClassType classType = T::getType() ;
      T* ptr = _monClass[classType]->add<T>(data);

      return ptr ;
   }
   /**
    * Remove a monitor object.
    *
    * This will lookup the object type and remove the object from the
    * appropriate container
    *
    * @param obj the object to be removed
    */
   void removeMonitorObject(MonClass* obj)
   {
      SDB_ASSERT ( NULL != obj, "removing a NULL monitor object" ) ;
      MonitorClassType classType = obj->getType();
      _monClass[classType]->remove(obj);
   }

   /**
    * Update the monitor status using a mask
    *
    * @param mask the monitor mask
    */
   void setMonitorStatus( UINT64 mask )
   {
      if ( mask & MON_GROUP_QUERY_DETAIL )
      {
         setCollectionLvl(MON_CLASS_QUERY, MON_DATA_LVL_DETAIL) ;
      }
      else if ( mask & MON_GROUP_QUERY_BASIC )
      {
         setCollectionLvl(MON_CLASS_QUERY, MON_DATA_LVL_BASIC) ;
      }
      else
      {
         setCollectionLvl(MON_CLASS_QUERY, MON_DATA_LVL_NONE) ;
      }

      if ( mask & MON_GROUP_LATCH_DETAIL )
      {
         setCollectionLvl(MON_CLASS_LATCH, MON_DATA_LVL_DETAIL) ;
      }
      else if ( mask & MON_GROUP_LATCH_BASIC )
      {
         setCollectionLvl(MON_CLASS_LATCH, MON_DATA_LVL_BASIC) ;
      }
      else
      {
         setCollectionLvl(MON_CLASS_LATCH, MON_DATA_LVL_NONE) ;
      }

      if ( mask & MON_GROUP_LOCK_DETAIL )
      {
         setCollectionLvl(MON_CLASS_LOCK, MON_DATA_LVL_DETAIL) ;
      }
      else if ( mask & MON_GROUP_LOCK_BASIC )
      {
         setCollectionLvl(MON_CLASS_LOCK, MON_DATA_LVL_BASIC) ;
      }
      else
      {
         setCollectionLvl(MON_CLASS_LOCK, MON_DATA_LVL_NONE) ;
      }
   }

   /**
    * Update the monitor data collection lvl of a class container
    *
    * @param classType the class type to update
    * @param mode      the new collection level
    */
   void setCollectionLvl( MonitorClassType classType, MonDataLvl mode )
   {
      _monClass[classType]->setCollectionLvl( mode ) ;
   }

   MonDataLvl getCollectionLvl( MonitorClassType classType )
   {
      return _monClass[classType]->getCollectionLvl() ;
   }

   BOOLEAN isOperational( MonitorClassType classType )
   {
      return _monClass[classType]->isOperational() ;
   }

   /**
    * Update the history event size of all class container
    *
    * @param size      the new monitor history event size
    */
   void setHistEventSize( UINT32 size )
   {
      for (int i = 0; i < MON_CLASS_MAX; i ++ )
      {
         _monClass[i]->setMaxArchivedListLen( size ) ;
      }
   }
   /**
    * Get a scanner to read a class' content
    *
    * @param classType the class type to scan
    * @param listType the list to scan (active/archived)
    */
   MonClassReadScanner* getReadScanner ( MonitorClassType classType, MonClassListType listType )
   {
      MonClassReadScanner *scanner = new MonClassReadScanner( _monClass[classType], listType ) ;

      return scanner ;
   }

   /**
    * Cleanup each container's pending archive and pending delete objects
    */
   void cleanup() ;

private:
   std::vector<MonClassContainer*> _monClass;
} ;

typedef _MonitorManager MonitorManager ;
}
#endif
