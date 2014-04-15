/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdMain.cpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdCommon.hpp"
#include "ossUtil.hpp"
#include "ossIO.hpp"
#include "msg.h"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   //PD_TRACE_DECLARE_FUNCTION ( SDB_PMDBLDFULLPATH, "pmdBuildFullPath" )
   INT32 pmdBuildFullPath( const CHAR *path, const CHAR *name,
                           UINT32 fullSize, CHAR *full )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDBLDFULLPATH ) ;
      if ( ossStrlen( path ) + ossStrlen( name )
           + 2 > fullSize )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      ossMemset( full, 0, fullSize );
      ossStrcpy( full, path ) ;
      if ( '\0' != path[0] &&
           0 != ossStrcmp(&path[ossStrlen(path)-1],OSS_FILE_SEP) )
      {
         ossStrncat( full, OSS_FILE_SEP, 1 ) ;
      }
      ossStrncat( full, name, ossStrlen( name ) ) ;

   done:
      PD_TRACE_EXITRC ( SDB_PMDBLDFULLPATH, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   SDB_ROLE pmdGetRoleEnum( const CHAR *role )
   {
      if ( NULL == role )
         return SDB_ROLE_STANDALONE;
      else if ( SDB_OK == ossStrcmp( role, SDB_ROLE_DATA_STR ) )
         return SDB_ROLE_DATA;
      else if ( SDB_OK == ossStrcmp( role, SDB_ROLE_CATALOG_STR ) )
         return SDB_ROLE_CATALOG;
      else if ( SDB_OK == ossStrcmp( role, SDB_ROLE_AUTH_STR ) )
         return SDB_ROLE_AUTH;
      else if ( SDB_OK == ossStrcmp( role, SDB_ROLE_COORD_STR ) )
         return SDB_ROLE_COORD;
      else if ( SDB_OK == ossStrcmp( role, SDB_ROLE_OM_STR ) )
         return SDB_ROLE_OM ;
      else
         return SDB_ROLE_STANDALONE;
   }

   INT32 pmdPrefReplStr2Enum( const CHAR * prefReplStr )
   {
      INT32 enumPrefRepl = PREFER_REPL_ANYONE ;

      if ( prefReplStr && *prefReplStr && !*(prefReplStr+1) )
      {
         CHAR ch = *prefReplStr ;
         if ( ch >= '1' && ch <= '7' )
         {
            enumPrefRepl = (INT32)( ch - '0' ) ;
         }
         else if ( 'M' == ch || 'm' == ch )
         {
            enumPrefRepl = PREFER_REPL_MASTER ;
         }
         else if ( 'S' == ch || 's' == ch )
         {
            enumPrefRepl = PREFER_REPL_SLAVE ;
         }
      }
      return enumPrefRepl ;
   }

   INT32 pmdPrefReplEnum2Str( INT32 enumPrefRepl, CHAR * prefReplStr,
                              UINT32 len )
   {
      ossMemset( prefReplStr, 0, len ) ;

      if ( enumPrefRepl >= PREFER_REPL_NODE_1 &&
           enumPrefRepl <= PREFER_REPL_NODE_7 )
      {
         ossSnprintf( prefReplStr, len-1, "%d", enumPrefRepl ) ;
      }
      else if ( enumPrefRepl == PREFER_REPL_MASTER )
      {
         ossSnprintf( prefReplStr, len-1, "%s", "M" ) ;
      }
      else if ( enumPrefRepl == PREFER_REPL_SLAVE )
      {
         ossSnprintf( prefReplStr, len-1, "%s", "S" ) ;
      }
      else if ( enumPrefRepl == PREFER_REPL_ANYONE )
      {
         ossSnprintf( prefReplStr, len-1, "%s", "A" ) ;
      }
      return SDB_OK ;
   }

   static SDB_ROLE g_dbRole = SDB_ROLE_STANDALONE ;
   SDB_ROLE pmdGetDBRole()
   {
      return g_dbRole ;
   }
   void  pmdSetDBRole( SDB_ROLE role )
   {
      g_dbRole = role ;
   }

}

