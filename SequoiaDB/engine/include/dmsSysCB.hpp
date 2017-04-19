/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = dmsSysCB.hpp

   Descriptive Name = Data Management Service Sys Control Block Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS System Table Control Block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/
#ifndef DMSSYSCB_HPP__
#define DMSSYSCB_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossLatch.hpp"
#include "dms.hpp"
#include <queue>
#include <map>

using namespace std ;

namespace engine
{

   class _SDB_DMSCB ;
   class _dmsStorageUnit ;
   class _dmsMBContext ;

   /*
      _dmsSysCB define
   */
   class _dmsSysCB : public SDBObject
   {
      public :
         _dmsSysCB ( _SDB_DMSCB *dmsCB )
         : _dmsCB( dmsCB )
         {
            _su = NULL ;
         }

         virtual ~_dmsSysCB () {}

         _dmsStorageUnit *getSU ()
         {
            return _su ;
         }

      protected :
      #ifdef DMSSYSCB_XLOCK
      #undef DMSSYSCB_XLOCK
      #endif
      #define DMSSYSCB_XLOCK ossScopedLock _lock(&_mutex, EXCLUSIVE);

      #ifdef DMSSYSCB_SLOCK
      #undef DMSSYSCB_SLOCK
      #endif
      #define DMSSYSCB_SLOCK ossScopedLock _lock(&_mutex, SHARED) ;

         ossSpinSLatch        _mutex ;
         _dmsStorageUnit      *_su ;
         _SDB_DMSCB           *_dmsCB ;
   } ;


}

#endif //DMSSYSCB_HPP__

