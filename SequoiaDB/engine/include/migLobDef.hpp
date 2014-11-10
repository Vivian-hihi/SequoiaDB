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

   Source File Name = migLobDef.hpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/31/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef MIG_LOBDEF_HPP_
#define MIG_LOBDEF_HPP_

#include "ossUtil.hpp"
#include "pd.hpp"

namespace lobtool
{
#define MIG_FILE_EYE "SDBLOBF"
#define MIG_HOSTNAME "hostname"
#define MIG_SERVICE "service"
#define MIG_USRNAME "usrname"
#define MIG_PASSWD "passwd"
#define MIG_CL "collection"
#define MIG_OP "operation"
#define MIG_FILE "file"

#define MIG_OP_IMPRT "import"
#define MIG_OP_EXPRT "export"

#define MIG_LOB_TOOL_VERSION 1

enum MIG_OP_TYPE
{
   MIG_OP_TYPE_IMPRT = 1,
   MIG_OP_TYPE_EXPRT
} ;

   struct migOptions
   {
      const CHAR *hostname ;
      const CHAR *service ;
      const CHAR *usrname ;
      const CHAR *passwd ;
      const CHAR *collection ;
      const CHAR *file ;
      MIG_OP_TYPE type ;

      migOptions()
      :hostname( NULL ),
       service( NULL ),
       usrname( NULL ),
       passwd( NULL ),
       collection( NULL ),
       file( NULL ),
       type( MIG_OP_TYPE_IMPRT )
      {

      }
   } ;

   /// 64KB
   struct migFileHeader 
   {
      CHAR eyeCatcher[8] ;
      UINT32 version ;
      UINT32 pad1 ; 
      UINT64 totalNum ;
      CHAR pad[65512] ;

      migFileHeader()
      {
         SDB_ASSERT( 65536 == sizeof( migFileHeader ), "must be 64KB" ) ;
         ossMemset( this, 0, sizeof( migFileHeader ) ) ;
         ossMemcpy( this, MIG_FILE_EYE, ossStrlen( MIG_FILE_EYE ) + 1 ) ;
         version = MIG_LOB_TOOL_VERSION ;
      }
   } ;
}

#endif

