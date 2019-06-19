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

/**
 * A monitor manager responsible for memory management and metric management
 * of the monitor classes
 */
class MonitorManager
{
   // disable assignment/copy
   MonitorManager& operator= (const MonitorManager&) ;
   MonitorManager(MonitorManager&) ;

public:
   MonitorManager() ;

   ~MonitorManager() ;

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
      T* ptr = NULL ;

      if ( _monClass[classType]->isMonitorOn() )
      {
         //TODO improve memory allocation method
         ptr = new T();
         _monClass[classType]->add(ptr);
      }

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
      MonitorClassType classType = obj->getType();
      _monClass[classType]->remove(obj);
   }

   /**
    * Update the monitor status of a class container
    *
    * @param classType the class type to update
    * @param mode      the new monitor status
    */
   void setMonitorStatus( MonitorClassType classType, BOOLEAN mode )
   {
      _monClass[classType]->setMonitorStatus( mode ) ;
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

}
#endif
