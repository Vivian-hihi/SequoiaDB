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

   Source File Name = sdbInterface.hpp

   Descriptive Name = Process MoDel Engine Dispatchable Unit Event Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains structure for events that
   used as inter-EDU communications.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SDB_INTERFACE_HPP__
#define SDB_INTERFACE_HPP__

#include "core.hpp"
#include "oss.hpp"

namespace engine
{

   /*
      _pmdBaseSession define
   */
   class _ISession : public SDBObject
   {
      public:
         _ISession() {}
         virtual ~_ISession() {}

         virtual INT32           run() = 0 ;

      public:
         virtual INT32           sessionType() const = 0 ;
         virtual UINT64          identifyID() = 0 ;
         virtual const CHAR*     sessionName() const = 0 ;

      protected:
         virtual void            _onAttach () {}
         virtual void            _onDetach () {}

   } ;
   typedef _ISession ISession ;

}

#endif // SDB_INTERFACE_HPP__

