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
#include "ossLatch.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

using namespace bson ;

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
         return SDB_ROLE_MAX;
      else if ( *role == 0 || 0 == ossStrcmp( role, SDB_ROLE_STANDALONE_STR ) )
         return SDB_ROLE_STANDALONE ;
      else if ( 0 == ossStrcmp( role, SDB_ROLE_DATA_STR ) )
         return SDB_ROLE_DATA;
      else if ( 0 == ossStrcmp( role, SDB_ROLE_CATALOG_STR ) )
         return SDB_ROLE_CATALOG;
      else if ( 0 == ossStrcmp( role, SDB_ROLE_AUTH_STR ) )
         return SDB_ROLE_AUTH;
      else if ( 0 == ossStrcmp( role, SDB_ROLE_COORD_STR ) )
         return SDB_ROLE_COORD;
      else if ( 0 == ossStrcmp( role, SDB_ROLE_OM_STR ) )
         return SDB_ROLE_OM ;
      else
         return SDB_ROLE_MAX;
   }

   const CHAR* pmdDBRoleStr( SDB_ROLE dbrole )
   {
      switch ( dbrole )
      {
         case SDB_ROLE_DATA :
            return SDB_ROLE_DATA_STR ;
         case SDB_ROLE_COORD :
            return SDB_ROLE_COORD_STR ;
         case SDB_ROLE_CATALOG :
            return SDB_ROLE_CATALOG_STR ;
         case SDB_ROLE_AUTH :
            return SDB_ROLE_AUTH_STR ;
         case SDB_ROLE_STANDALONE :
            return SDB_ROLE_STANDALONE_STR ;
         case SDB_ROLE_OM :
            return SDB_ROLE_OM_STR ;
         default :
            break ;
      }
      return "" ;
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

   pmdSysInfo* pmdGetSysInfo()
   {
      static pmdSysInfo s_sysInfo ;
      return &s_sysInfo ;
   }
   SDB_ROLE pmdGetDBRole()
   {
      return pmdGetSysInfo()->_dbrole ;
   }
   void  pmdSetDBRole( SDB_ROLE role )
   {
      pmdGetSysInfo()->_dbrole = role ;
   }
   MsgRouteID pmdGetNodeID()
   {
      return pmdGetSysInfo()->_nodeID ;
   }
   void pmdSetNodeID( MsgRouteID id )
   {
      pmdGetSysInfo()->_nodeID = id ;
   }

   BSONObj pmdGetErrorBson( INT32 flags, const CHAR *detail )
   {
      static BSONObj _retObj [SDB_MAX_ERROR + SDB_MAX_WARNING + 1] ;
      static BOOLEAN _init = FALSE ;
      static ossSpinXLatch _lock ;

      // init retobj
      if ( FALSE == _init )
      {
         _lock.get() ;
         if ( FALSE == _init )
         {
            for ( SINT32 i = -SDB_MAX_ERROR; i <= SDB_MAX_WARNING ; i ++ )
            {
               BSONObjBuilder berror ;
               berror.append ( OP_ERRNOFIELD, i ) ;
               berror.append ( OP_ERRDESP_FIELD, getErrDesp ( i ) ) ;
               _retObj[ i + SDB_MAX_ERROR ] = berror.obj() ;
            }
            _init = TRUE ;
         }
         _lock.release() ;
      }

      // check flags
      if ( flags < -SDB_MAX_ERROR || flags > SDB_MAX_WARNING )
      {
         PD_LOG ( PDERROR, "Error code error[rc:%d]", flags ) ;
         flags = SDB_SYS ;
      }

      // return new obj
      if ( detail && *detail != 0 )
      {
         BSONObjBuilder bb ;
         bb.append ( OP_ERRNOFIELD, flags ) ;
         bb.append ( OP_ERRDESP_FIELD, getErrDesp ( flags ) ) ;
         bb.append ( OP_ERR_DETAIL, detail ) ;
         return bb.obj() ;
      }
      // return fix obj
      return _retObj[ SDB_MAX_ERROR + flags ] ;
   }

}

