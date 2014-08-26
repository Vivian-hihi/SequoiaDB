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

   Source File Name = pmdCommon.hpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          24/04/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDCOMMON_HPP_
#define PMDCOMMON_HPP_

#include "core.hpp"
#include "msgDef.h"
#include "pmdDef.hpp"
#include "utilStr.hpp"
#include "msg.h"

#include "../bson/bson.h"

using namespace bson ;

namespace engine
{

   /*
      PMD ROLE ENUM AND STRING TRANSFER
   */
   SDB_ROLE pmdGetRoleEnum( const CHAR *role ) ;
   const CHAR* pmdDBRoleStr( SDB_ROLE dbrole ) ;

   /*
      PMD ROLE_TYPE ENUM AND STRING TRANSFER
   */
   SDB_TYPE pmdGetTypeEnum( const CHAR *type ) ;
   const CHAR* pmdDBTypeStr( SDB_TYPE type ) ;

   SDB_TYPE pmdRoleToType( SDB_ROLE role ) ;

   /*
      PMD Pref instance enum and string transfer
   */
   INT32 pmdPrefReplStr2Enum( const CHAR *prefReplStr ) ;

   INT32 pmdPrefReplEnum2Str( INT32 enumPrefRepl,
                              CHAR *prefReplStr,
                              UINT32 len ) ;

   /*
      PMD get error bson
   */
   BSONObj        pmdGetErrorBson( INT32 flags, const CHAR *detail ) ;

}



#endif //PMDCOMMON_HPP_

