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
#include "utilCommon.hpp"
#include "ossProc.hpp"
#include "ossUtil.hpp"
#include "ossPath.hpp"
#include "utilParam.hpp"
#include "pd.hpp"

/*
#if defined( _LINUX )
#include <dirent.h>
#endif //_LINUX
*/

namespace engine
{

   static INT32 _utilCheckOrCleanNamedPipe( const CHAR *fullPipeName,
                                            OSSPID &pid )
   {
#if defined( _LINUX )
      INT32 rc = SDB_OK ;
      const CHAR *pPidPtr = NULL ;

      pPidPtr = ossStrrchr( fullPipeName, '_' ) ;
      if ( !pPidPtr || 0 == *( pPidPtr + 1 ) ||
           0 == ( pid = ossAtoi( pPidPtr + 1 ) ) )
      {
         goto done ;
      }
      if ( ossIsProcessRunning( pid ) )
      {
         rc = SDB_FE ;
         goto error ;
      }
      else
      {
         rc = ossCleanNamedPipeByName( fullPipeName ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
#else
      return SDB_FE ;
#endif // _LINUX
   }

   INT32 utilPrepareForNamedPipe( const CHAR * pPattern )
   {
      INT32 rc = SDB_OK ;
      OSSPID pid = OSS_INVALID_PID ;
      vector< string > names ;
      ossEnumNamedPipes( names, pPattern, OSS_MATCH_LEFT ) ;

      for ( UINT32 i = 0 ; i < names.size() ; ++i )
      {
         rc = _utilCheckOrCleanNamedPipe( names[ i ].c_str(), pid ) ;
         if ( SDB_FE == rc )
         {
            PD_LOG( PDERROR, "Named pipe[%s] process[%s] is running, "
                    "conflict", names[ i ].c_str(), pid ) ;
            goto error ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Clean named pipe[%s] failed, rc: %d",
                    names[ i ].c_str(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

/*
#if defined (_LINUX)
   INT32 utilListNodes( UTIL_VEC_NODES & nodes,
                        INT32 typeFilter,
                        const CHAR * svcnameFilter,
                        OSSPID pidFilter,
                        INT32 roleFilter )
   {
      INT32 rc                   = SDB_OK ;
      DIR *pDir                  = NULL ;
      struct dirent *pDirent     = NULL ;
      BOOLEAN isOpen = FALSE ;
      CHAR *pStr                 = NULL ;
      INT32 beginType            = SDB_TYPE_DB ;
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
         if ( 0 == ossStrcmp( pDirent->d_name, "self" ) )
         {
            continue ;
         }
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
              ossStrlen( pSvcEnd ) > 3  )
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
            pStr = ossStrstr( commandLine,
                              utilDBTypeStr( (SDB_TYPE)beginType ) ) ;
            if ( pStr == commandLine &&
                 (UINT32)( pSvcBegin - pStr) ==
                 (UINT32)ossStrlen( utilDBTypeStr( (SDB_TYPE)beginType ) ) )
            {
               if ( -1 == typeFilter || typeFilter == beginType )
               {
                  ++matchNum ;
                  findNode._type = beginType ;
                  break ;
               }
            }

            if ( typeFilter == beginType )
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

         // 4. role
         findNode._role = SDB_ROLE_MAX ;
         if ( ' ' == *( pSvcEnd + 1 ) )
         {
            findNode._role = utilShortStr2DBRole( pSvcEnd + 2 ) ;
         }

         if ( SDB_ROLE_MAX == findNode._role )
         {
            switch( findNode._type )
            {
               case SDB_TYPE_OM :
                  findNode._role = SDB_ROLE_OM ;
                  break ;
               case SDB_TYPE_OMA :
                  findNode._role = SDB_ROLE_OMA ;
                  break ;
               case SDB_TYPE_DB :
                  findNode._role = SDB_ROLE_STANDALONE ;
                  break ;
               default :
                  break ;
            }
         }

         if ( roleFilter != -1 && roleFilter != findNode._role )
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
*/
   static INT32 utilWriteReadPipe( const CHAR *pPipeName,
                                   const CHAR *pWriteBuf, INT64 writeLen,
                                   CHAR *pReadBuf, INT64 readLen,
                                   INT64 *bufRead )
   {
      INT32 rc = SDB_OK ;
      OSSNPIPE handle ;
      BOOLEAN isOpen = FALSE ;

      rc = ossOpenNamedPipe( pPipeName,
                             OSS_NPIPE_DUPLEX | OSS_NPIPE_BLOCK |
                             OSS_NPIPE_BLOCK_WITH_TIMEOUT, 1, handle ) ;
      if ( rc && SDB_FE != rc )
      {
         PD_LOG ( PDERROR, "Failed to create named pipe: %s, rc: %d",
                  pPipeName, rc ) ;
         goto error ;
      }
      isOpen = TRUE ;

      rc = ossWriteNamedPipe( handle, pWriteBuf, writeLen, NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to send %s to %s, rc: %d",
                  pWriteBuf, pPipeName, rc ) ;
         goto error ;
      }

      if ( !pReadBuf || 0 == readLen )
      {
         goto done ;
      }

      rc = ossReadNamedPipe( handle, pReadBuf, readLen, bufRead,
                             LIST_TIMEOUT ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read %s return from pip %s, rc: %d",
                  pWriteBuf, pPipeName, rc ) ;
         goto error ;
      }

      if ( readLen != *bufRead )
      {
         PD_LOG ( PDERROR, "Failed to read %s return from pip %s, rc: %d",
                  pWriteBuf, pPipeName, rc ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      if ( isOpen )
      {
         ossCloseNamedPipe( handle ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 utilListNodes( UTIL_VEC_NODES & nodes, INT32 typeFilter,
                        const CHAR * svcnameFilter, OSSPID pidFilter,
                        INT32 roleFilter )
   {
      INT32 rc = SDB_OK ;
      vector< string > names ;
      utilNodeInfo findNode ;
      INT64 readSize = 0 ;
      UINT32 prefixLen = ossStrlen( ENGINE_NPIPE_PREFIX ) ;
      OSSPID pid = OSS_INVALID_PID ;

      rc = ossEnumNamedPipes ( names, ENGINE_NPIPE_PREFIX, OSS_MATCH_LEFT ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to enum pipes, rc: %d", rc ) ;

      for ( UINT32 i = 0 ; i < names.size() ; ++i )
      {
         rc = _utilCheckOrCleanNamedPipe( names[ i ].c_str() ) ;
         if ( SDB_FE != rc )
         {
            continue ;
         }

         // linux:   sequoiadb_engine_11790(svcname)_2500(pid)
         // windows: sequoiadb_engine_11790(svcname)
         // get svcname
         findNode._svcname = names[ i ].substr( prefixLen ) ;
#if defined( _LINUX )
         findNode._svcname = findNode._svcname.substr( 0,
                             findNode._svcname.find( "_" ) ) ;
#endif // _LINUX

         // 1. svcname
         if ( svcnameFilter && 0 != *svcnameFilter &&
              0 != ossStrcmp( findNode._svcname.c_str(), svcnameFilter ) )
         {
            continue ;
         }

         // 2. type
         rc = utilWriteReadPipe( names[ i ].c_str(),
                                 ENGINE_NPIPE_MSG_TYPE,
                                 sizeof( ENGINE_NPIPE_MSG_TYPE ),
                                 (CHAR*)&findNode._type,
                                 sizeof( findNode._type ),
                                 &readSize ) ;
         if ( rc )
         {
            continue ;
         }
         if ( -1 != typeFilter && typeFilter != findNode._type )
         {
            continue ;
         }

         // 3. pid
         rc = utilWriteReadPipe( names[ i ].c_str(),
                                 ENGINE_NPIPE_MSG_PID,
                                 sizeof( ENGINE_NPIPE_MSG_PID ),
                                 (CHAR *)&findNode._pid,
                                 sizeof( findNode._pid ),
                                 &readSize ) ;
         if ( rc )
         {
            continue ;
         }
         if ( pidFilter != OSS_INVALID_PID && pidFilter != findNode._pid )
         {
            continue ;
         }

         // 4. role
         rc = utilWriteReadPipe( names[ i ].c_str(), ENGINE_NPIPE_MSG_ROLE,
                                 sizeof( ENGINE_NPIPE_MSG_ROLE ),
                                 (CHAR *)&findNode._role,
                                 sizeof( findNode._role ),
                                 &readSize ) ;
         if ( rc )
         {
            continue ;
         }
         if ( roleFilter != -1 && roleFilter != findNode._role )
         {
            continue ;
         }

         // find it
         findNode._orgname = names[ i ] ;
         nodes.push_back( findNode ) ;

         if ( pidFilter != OSS_INVALID_PID ||
              ( svcnameFilter && 0 != *svcnameFilter ) )
         {
            break ;
         }
      }
      rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

//#endif // _LINUX

   INT32 utilEnumNodes( const string &localPath,
                        UTIL_VEC_NODES &nodes,
                        INT32 typeFilter,
                        const CHAR *svcnameFilter,
                        INT32 roleFilter )
   {
      INT32 rc = SDB_OK ;
      vector< string > allsvcnames ;
      utilNodeInfo node ;
      CHAR confPath[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

      node._orgname = "" ;
      node._pid = OSS_INVALID_PID ;

      if ( SDB_TYPE_OMA != typeFilter )
      {
         rc = ossEnumSubDirs( localPath, allsvcnames, 1 ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Enum %s failed, rc: %d", localPath.c_str(),
                    rc ) ;
            goto error ;
        }
      }

      for ( UINT32 i = 0 ; i < allsvcnames.size() ; ++i )
      {
         if ( svcnameFilter && *svcnameFilter &&
              0 != ossStrcmp( svcnameFilter, allsvcnames[ i ].c_str() ) )
         {
            continue ;
         }
         node._svcname = allsvcnames[ i ] ;

         utilBuildFullPath( localPath.c_str(), node._svcname.c_str(),
                            OSS_MAX_PATHSIZE, confPath ) ;

         rc = utilGetRoleByConfigPath( confPath, node._role ) ;
         if ( -1 != roleFilter && ( SDB_OK != rc ||
              roleFilter != node._role ) )
         {
            continue ;
         }

         node._type = utilRoleToType( (SDB_ROLE)node._role ) ;
         if ( -1 != typeFilter && typeFilter != node._type )
         {
            continue ;
         }

         // find it
         nodes.push_back( node ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilWaitNodeOK( utilNodeInfo & node, const CHAR * svcname,
                         OSSPID pid, INT32 typeFilter,
                         INT32 timeout )
   {
      INT32 rc = SDB_OK ;
      UTIL_VEC_NODES nodes ;

      if ( timeout < 0 )
      {
         timeout = 0x7FFFFFFF ;
      }
      else if ( timeout == 0 )
      {
         timeout = 1 ;
      }

      while ( timeout > 0 )
      {
         --timeout ;

         nodes.clear() ;
         rc = utilListNodes( nodes, typeFilter, svcname, pid ) ;
         if ( SDB_OK == rc && nodes.size() > 0 )
         {
            node = ( *nodes.begin() ) ;
            rc = SDB_OK ;
            goto done ;
         }

         if ( pid != OSS_INVALID_PID && !ossIsProcessRunning( pid ) )
         {
            PD_LOG( PDERROR, "Process[%d] has exist", pid ) ;
            // process exist
            rc = SDB_SYS ;
            goto error ;
         }

         // sleep one seconds
         ossSleep( OSS_ONE_SEC ) ;
      }
      rc = SDB_TIMEOUT ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilStopNode( utilNodeInfo & node, INT32 timeout )
   {
      INT32 rc = SDB_OK ;

      if ( timeout < 0 )
      {
         timeout = 0x7FFFFFFF ;
      }
      else if ( timeout == 0 )
      {
         timeout = 1 ;
      }

#if defined( _LINUX )
      rc = ossTerminateProcess( node._pid, FALSE ) ;
#else
      rc = utilWriteReadPipe( node._orgname.c_str(),
                              ENGINE_NPIPE_MSG_SHUTDOWN,
                              sizeof( ENGINE_NPIPE_MSG_SHUTDOWN ),
                              NULL, 0, NULL ) ;
#endif // _LINUX
      if ( rc && ossIsProcessRunning( node._pid ) )
      {
         PD_LOG( PDERROR, "kill or $shutdown node[%d] failed, rc: %d",
                 node._pid, rc ) ;
         goto error ;
      }
      else if ( rc )
      {
         rc = SDB_OK ;
         goto done ;
      }

      while ( timeout > 0 )
      {
         --timeout ;
         if ( !ossIsProcessRunning( node._pid ) )
         {
            rc = SDB_OK ;
            goto done ;
         }
         ossSleep( OSS_ONE_SEC ) ;
      }
      rc = SDB_TIMEOUT ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilGetNodeVerInfo( const CHAR *pCommand,
                             utilNodeVerInfo & verInfo )
   {
      INT32 rc = SDB_OK ;
      FILE *fp = NULL ;
      CHAR buff[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

      if ( !pCommand )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      fp = ossPopen( pCommand, "r" ) ;
      if ( !fp )
      {
         PD_LOG( PDERROR, "File to popen[%s], rc: %d", pCommand,
                 ossGetLastError() ) ;
         rc = SDB_SYS ;
      }

      fread( buff, OSS_MAX_PATHSIZE, 1, fp ) ;

      rc = utilParseVersion( buff, verInfo._version, verInfo._subVersion,
                             verInfo._release, verInfo._buildStr ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to parse version info[%s], rc: %d",
                 buff, rc ) ;
         goto error ;
      }

   done:
      if ( fp )
      {
         ossPclose( fp ) ;
      }
      return rc ;
   error:
      goto done ;
   }

}


