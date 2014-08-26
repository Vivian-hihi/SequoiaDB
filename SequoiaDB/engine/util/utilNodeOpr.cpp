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

   Source File Name = utilNodeOpr.cpp

   Descriptive Name =

   When/how to use: node operation

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/08/2014  XJH Initial Draft

   Last Changed =

******************************************************************************/

#include "utilNodeOpr.hpp"
#include "pmdDef.hpp"
#include "pmdCommon.hpp"
#include "ossProc.hpp"
#include "pd.hpp"

#if defined( _LINUX )
#include <dirent.h>
#endif //_LINUX

namespace engine
{

#if defined (_LINUX)
   INT32 utilListNodes( UTIL_VEC_NODES & nodes, INT32 typeFilter,
                        const CHAR * svcnameFilter,
                        OSSPID pidFilter )
   {
      INT32 rc                   = SDB_OK ;
      DIR *pDir                  = NULL ;
      struct dirent *pDirent     = NULL ;
      BOOLEAN isOpen = FALSE ;
      CHAR *pStr                 = NULL ;
      SDB_TYPE beginType         = SDB_TYPE_DB ;
      CHAR *pSvcBegin            = NULL ;
      CHAR *pSvcEnd              = NULL ;
      UINT32 matchNum            = 0 ;
      utilNodeInfo findNode ;

      pDir = opendir( PROC_PATH ) ;
      PD_CHECK( pDir != NULL, SDB_IO, error, PDERROR,
                "Failed to open the directory:%s, errno=%d",
                PROC_PATH, ossGetLastError() ) ;
      isOpen = TRUE ;

      while( (pDirent = readdir( pDir )) != NULL )
      {
         CHAR pathName[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
         ossSnprintf( pathName, OSS_MAX_PATHSIZE, PROC_CMDLINE_PATH_FORMAT,
                      pDirent->d_name ) ;
         FILE *fp = NULL ;
         fp = fopen( pathName, "r" ) ;
         if ( !fp )
         {
            continue ;
         }
         CHAR commandLine[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
         CHAR *pTmp = fgets ( commandLine, OSS_MAX_PATHSIZE, fp ) ;
         fclose(fp) ;
         if ( NULL == pTmp )
         {
            continue ;
         }

         // analysize node info
         pStr = NULL ;
         matchNum = 0 ;
         beginType = SDB_TYPE_DB ;
         pSvcBegin = NULL ;
         pSvcEnd = NULL ;

         // 1. svcname
         pSvcBegin = ossStrchr( commandLine, '(' ) ;
         if ( !pSvcBegin )
         {
            continue ;
         }
         pSvcEnd = ossStrchr( pSvcBegin + 1, ')' ) ;
         if ( !pSvcEnd || pSvcEnd - pSvcBegin <= 1 ||
              ossStrlen( pSvcEnd ) != 1 )
         {
            continue ;
         }
         *pSvcEnd = 0 ;
         if ( svcnameFilter && 0 != svcnameFilter &&
              0 != ossStrcmp( pSvcBegin + 1, svcnameFilter ) )
         {
            continue ;
         }

         // 2. type
         while ( beginType < SDB_TYPE_MAX )
         {
            pStr = ossStrstr( commandLine, pmdDBTypeStr( beginType ) ) ;
            if ( pStr == commandLine &&
                 pSvcBegin - pStr = ossStrlen( pmdDBTypeStr( beginType ) ) )
            {
               ++matchNum ;
               findNode._type = beginType ;
               if ( -1 == typeFilter )
               {
                  break ;
               }
            }

            if ( typeFilter == (INT32)beginType )
            {
               break ;
            }
            ++beginType ;
         }
         if ( 0 == matchNum )
         {
            continue ;
         }

         // 3. pid
         findNode._pid = ossAtoi( pDirent->d_name ) ;
         if ( pidFilter != OSS_INVALID_PID &&
              pidFilter != findNode._pid )
         {
            continue ;
         }

         findNode._svcname = string( pSvcBegin + 1 ) ;
         *pSvcEnd = ')' ;
         findNode._orgname = commandLine ;

         // find it
         nodes.push_back( findNode ) ;

         if ( pidFilter != OSS_INVALID_PID ||
              ( svcnameFilter && 0 != *svcnameFilter ) )
         {
            break ;
         }
      }

   done:
      if ( isOpen )
      {
         closedir( pDir ) ;
      }
      return rc ;
   error:
      goto done ;
   }


#else



#endif // _LINUX

}


