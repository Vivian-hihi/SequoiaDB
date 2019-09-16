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

   Source File Name = monMgr.cpp

   Descriptive Name = Monitor Service Source

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

#include "monMgr.hpp"

namespace engine
{

archiveFunc monClassArchiveFP[MON_CLASS_MAX] = {
   noArchive,
   noArchive,
   archiveQuery,
   noArchive
};

MonitorManager::MonitorManager()
   : _monClass(MON_CLASS_MAX)
{
   for (int i = 0; i < MON_CLASS_MAX; i ++ )
   {
      //TODO: create array of function pointers
      MonClassContainer *list = new MonClassContainer(monClassArchiveFP[i]) ;
      _monClass[i] = list ;
   }
}

MonitorManager::~MonitorManager()
{
   for (int i = 0; i < MON_CLASS_MAX; i ++ )
   {
      delete _monClass[i] ;
   }
}

void MonitorManager::cleanup()
{
   for (int i = 0; i < MON_CLASS_MAX; i++ )
   {
      MonClassContainer *curContainer = _monClass[i] ;

      if ( curContainer->_numPendingArchive.fetch() > 0 ||
           curContainer->_numPendingDelete.fetch() > 0 )
      {
         curContainer->_processPendingObj() ;
      }
   }
}
} // namespace engine
