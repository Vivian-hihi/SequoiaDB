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

   Source File Name = pmdController.hpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          02/05/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMD_CONTROLLER_HPP__
#define PMD_CONTROLLER_HPP__

#include "pmdEnv.hpp"
#include "ossSocket.hpp"
#include "sdbInterface.hpp"

namespace engine
{

   /*
      _pmdController define
   */
   class _pmdController : public _IControlBlock
   {
      public:
         _pmdController () ;
         virtual ~_pmdController () ;

         virtual SDB_CB_TYPE cbType() const ;
         virtual const CHAR* cbName() const ;

         virtual INT32  init () ;
         virtual INT32  active () ;
         virtual INT32  deactive () ;
         virtual INT32  fini () ;

      public:

         virtual void  registerCB( SDB_ROLE dbrole ) ;

      private:
         ossSocket               *_pTcpListener ;
         ossSocket               *_pHttpListener ;

   } ;
   typedef _pmdController pmdController ;

   /*
      get global pointer
   */
   pmdController* sdbGetPMDController() ;

}

#endif //PMD_CONTROLLER_HPP__

