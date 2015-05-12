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

   Source File Name = rtnRPCFuncs.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_RPCFUNCS_HPP_
#define RTN_RPCFUNCS_HPP_

#include "rtnRPCDef.hpp"

namespace engine
{
   class _pmdEDUCB ;
   class _dpsLogWrapper ;
   /// see func's name in msgDef.hpp

   /// SDB_RPC_CRT_ID_INDEX
   INT32 rtnCreateIDIndex( const CHAR *name,
                           const bson::BSONObj &pubArgs,
                           const bson::BSONObj &args,
                           _pmdEDUCB *cb,
                           _dpsLogWrapper *dpsCB ) ;

   /// SDB_RPC_DROP_ID_INDEX
   INT32 rtnDropIDIndex( const CHAR *name,
                         const bson::BSONObj &pubArgs,
                         const bson::BSONObj &args,
                         _pmdEDUCB *cb,
                         _dpsLogWrapper *dpsCB ) ;
}

#endif

