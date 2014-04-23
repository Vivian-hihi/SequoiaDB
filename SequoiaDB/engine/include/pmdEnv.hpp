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

   Source File Name = pmdEnv.hpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          22/04/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDENV_HPP_
#define PMDENV_HPP_

#include "pmdCommon.hpp"

using namespace bson ;

namespace engine
{

   /*
      pmd system info define
   */
   typedef struct _pmdSysInfo
   {
      SDB_ROLE                      _dbrole ;
      MsgRouteID                    _nodeID ;

      _pmdSysInfo()
      {
         _dbrole        = SDB_ROLE_STANDALONE ;
         _nodeID.value  = MSG_INVALID_ROUTEID ;
      }
   } pmdSysInfo ;

   SDB_ROLE       pmdGetDBRole() ;
   void           pmdSetDBRole( SDB_ROLE role ) ;
   MsgRouteID     pmdGetNodeID() ;
   void           pmdSetNodeID( MsgRouteID id ) ;

   pmdSysInfo*    pmdGetSysInfo () ;

   /*
      pmd trap functions
   */
   INT32    pmdEnableSignalEvent( const CHAR *filepath ) ;

   INT32&   pmdGetSigNum() ;

   /*
      Env define
   */
   #define  PMD_SIGNUM                 pmdGetSigNum()

}

#endif //PMDENV_HPP_

