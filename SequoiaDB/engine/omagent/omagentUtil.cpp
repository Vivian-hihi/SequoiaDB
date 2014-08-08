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

   Source File Name = omagentUtil.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "ossPrimitiveFileOp.hpp"
#include "pd.hpp"
#include "omagentUtil.hpp"
#include "ossProc.hpp"
#include "pmdOptions.hpp"
#include "ossPath.hpp"
#include "ossIO.hpp"
#include "pmdDef.hpp"
/*
#include "spt.hpp"
#include "sptSPScope.hpp"
#include "sptUsrSsh.hpp"
#include "sptUsrCmd.hpp"
#include "sptUsrFile.hpp"
#include "sptUsrSystem.hpp"
#include "../spt/js_in_cpp.hpp"
*/
#if defined( _LINUX )
#include <dirent.h>
#endif //_LINUX

//JSBool InitDbClasses( JSContext *cx, JSObject *obj ) ;

namespace engine
{
   /*
      Local Define
   */
   #define CM_NPIPE_SIZE                  64


   INT32 checkBuffer ( CHAR **ppBuffer, INT32 *bufferSize,
                       INT32 packetLength )
   {
      INT32 rc = SDB_OK ;
      if ( packetLength > *bufferSize )
      {
         CHAR *pOrigMem = *ppBuffer ;
         INT32 newSize = ossRoundUpToMultipleX ( packetLength, SDB_PAGE_SIZE ) ;
         if ( newSize < 0 )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "new buffer overflow" ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         *ppBuffer = (CHAR*)SDB_OSS_REALLOC ( *ppBuffer, sizeof(CHAR)*(newSize)) ;
         if ( !*ppBuffer )
         {
            PD_LOG ( PDERROR, "Failed to allocate %d bytes send buffer",
                     newSize ) ;
            rc = SDB_OOM ;
            // realloc does NOT free original memory if it fails, so we have to
            // assign pointer to original
            *ppBuffer = pOrigMem ;
            goto error ;
         }
         *bufferSize = newSize ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }


   INT32 readFile ( const CHAR * name , CHAR ** buf , UINT32 * bufSize,
                    UINT32 * readSize )
   {
      ossPrimitiveFileOp op ;
      ossPrimitiveFileOp::offsetType offset ;
      INT32 rc = SDB_OK ;

      SDB_ASSERT ( name && buf && bufSize, "Invalid arguments" ) ;

      rc = op.Open ( name , OSS_PRIMITIVE_FILE_OP_READ_WRITE ) ;
      if ( rc != SDB_OK )
      {
         ossPrintf ( "Can't open file: %s"OSS_NEWLINE, name ) ;
         goto error ;
      }

      rc = op.getSize ( &offset ) ;
      if ( rc != SDB_OK )
      {
         goto error ;
      }

      if ( *bufSize < offset.offset + 1 )
      {
         if ( *buf )
         {
            SDB_OSS_FREE( *buf ) ;
            *buf = NULL ;
            *bufSize = 0 ;
         }
         *buf = (CHAR *) SDB_OSS_MALLOC ( offset.offset + 1 ) ;
         if ( ! *buf )
         {
            rc = SDB_OOM ;
            PD_LOG ( PDERROR , "fail to alloc memory" ) ;
            goto error ;
         }
         *bufSize = offset.offset + 1 ;
      }

      rc = op.Read ( offset.offset , *buf , NULL ) ;
      if ( rc != SDB_OK )
      {
         goto error ;
      }
      (*buf)[ offset.offset ] = 0 ;
      if ( readSize ) *readSize = offset.offset ;

   done :
      op.Close() ;
      return rc ;
   error :
      goto done ;
   }
/*
   // get spider monkey engine
   INT32 getSptScope ( _sptScope **scope )
   {
      INT32 rc = SDB_OK ;
      _sptContainer container ;
      _sptScope *_scope = container.newScope( SPT_SCOPE_TYPE_SP ) ;
      SDB_ASSERT( _scope, "Failed to get spt scope" ) ;
      // init db classes for omagent to use db driver API
      if ( !InitDbClasses( ((sptSPScope *)_scope)->getContext(),
                           ((sptSPScope *)_scope)->getGlobalObj() ) )
      {
         PD_LOG( PDERROR, "failed to init dbclass" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      // init another classes for omagent to get hosts info and so on
      rc = _scope->loadUsrDefObj<_sptUsrSsh>() ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
      rc = _scope->loadUsrDefObj<_sptUsrCmd>() ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
      rc = _scope->loadUsrDefObj<_sptUsrFile>() ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
      rc = _scope->loadUsrDefObj<_sptUsrSystem>() ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to load class _sptUsrSsh, rc = %d", rc ) ;
      rc = evalInitScripts2( _scope ) ;
      PD_CHECK ( SDB_OK == rc, SDB_SYS, error, PDERROR,
                 "Failed to init spt scope, rc = %d", rc ) ;
      // return result
      *scope = _scope ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // get bson field
   INT32 omaGetIntElement ( const BSONObj &obj, const CHAR *fieldName,
                                INT32 &value )
   {
      SINT32 rc = SDB_OK ;
      SDB_ASSERT ( fieldName, "field name can't be NULL" ) ;
      BSONElement ele = obj.getField ( fieldName ) ;
      PD_CHECK ( !ele.eoo(), SDB_FIELD_NOT_EXIST, error, PDDEBUG,
                 "Can't locate field '%s': %s",
                 fieldName,
                 obj.toString().c_str() ) ;
      PD_CHECK ( ele.isNumber(), SDB_INVALIDARG, error, PDDEBUG,
                 "Unexpected field type : %s, supposed to be Integer",
                 obj.toString().c_str()) ;
      value = ele.numberInt() ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 omaGetStringElement ( const BSONObj &obj, const CHAR *fieldName,
                                   const CHAR **value )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( fieldName && value, "field name and value can't be NULL" ) ;
      BSONElement ele = obj.getField ( fieldName ) ;
      PD_CHECK ( !ele.eoo(), SDB_FIELD_NOT_EXIST, error, PDDEBUG,
                 "Can't locate field '%s': %s",
                 fieldName,
                 obj.toString().c_str() ) ;
      PD_CHECK ( String == ele.type(), SDB_INVALIDARG, error, PDDEBUG,
                 "Unexpected field type : %s, supposed to be String",
                 obj.toString().c_str()) ;
      *value = ele.valuestr() ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 omaGetObjElement ( const BSONObj &obj, const CHAR *fieldName,
                                BSONObj &value )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( fieldName , "field name can't be NULL" ) ;
      BSONElement ele = obj.getField ( fieldName ) ;
      PD_CHECK ( !ele.eoo(), SDB_FIELD_NOT_EXIST, error, PDDEBUG,
                 "Can't locate field '%s': %s",
                 fieldName,
                 obj.toString().c_str() ) ;
      PD_CHECK ( Object == ele.type(), SDB_INVALIDARG, error, PDDEBUG,
                 "Unexpected field type : %s, supposed to be Object",
                 obj.toString().c_str()) ;
      value = ele.embeddedObject() ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 omaGetBooleanElement ( const BSONObj &obj, const CHAR *fieldName,
                                    BOOLEAN &value )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( fieldName , "field name can't be NULL" ) ;
      BSONElement ele = obj.getField ( fieldName ) ;
      PD_CHECK ( !ele.eoo(), SDB_FIELD_NOT_EXIST, error, PDDEBUG,
                 "Can't locate field '%s': %s",
                 fieldName,
                 obj.toString().c_str() ) ;
      PD_CHECK ( Bool == ele.type(), SDB_INVALIDARG, error, PDDEBUG,
                 "Unexpected field type : %s, supposed to be Bool",
                 obj.toString().c_str()) ;
      value = ele.boolean() ;
   done :
      return rc ;
   error :
      goto done ;
   }
*/
   /*
      Node Manager Tool Functions Implement
   */
   INT32 omStartDBNode( const CHAR *pExecName,
                        const CHAR *pCfgPath,
                        OSSPID &pid )
   {
      INT32 rc                = SDB_OK ;
      CHAR *pArgumentBuffer   = NULL ;
      INT32 argBuffLen        = 0 ;
      CHAR pNPipeBuf[ CM_NPIPE_SIZE + 1 ] = { 0 } ;

      list< const CHAR* > argv ;
      OSSNPIPE outNPipe ;
      ossResultCode result ;
      OSSPID tmppid ;

      // verify the configuration file
      rc = ossAccess ( pCfgPath ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Can not access the configure file: %s", pCfgPath ) ;
         goto error ;
      }

      argv.push_back ( pExecName ) ;
      argv.push_back ( SDBCM_OPTION_PREFIX PMD_OPTION_CONFPATH ) ;
      argv.push_back ( pCfgPath ) ;
      rc = ossBuildArguments( &pArgumentBuffer, argBuffLen, argv ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build sdbstart arguments, rc: %d",
                  rc ) ;
         goto error ;
      }

      // call exec to run the command with arguments, 
      // do NOT wait until program finish
      rc = ossExec ( pArgumentBuffer, pArgumentBuffer, NULL,
                     OSS_EXEC_SSAVE, tmppid, result, NULL,
                     &outNPipe ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to execute %s, rc: %d",
                  pArgumentBuffer, rc ) ;
         rc = SDBCM_FAIL ;
         goto error ;
      }
      // verify the executing result
      if ( result.termcode == OSS_EXIT_NORMAL &&
           result.exitcode == SDB_OK  )
      {
         // if execute sdbStart succeed
         INT64 bufRead ;
         rc = ossReadNamedPipe ( outNPipe, pNPipeBuf, CM_NPIPE_SIZE,
                                 &bufRead, 0 ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to trace the PID, rc: %d", rc ) ;
            goto error ;
         }
         CHAR *p = ossStrrchr ( pNPipeBuf, '(' ) ;
         if ( p )
         {
            // start successfully
            pid = (OSSPID) ossAtoi ( p + 1 ) ;
         }
         else
         {
            rc = SDBCM_FAIL ;
            goto error ;
         }
      }
      else
      {
         rc = SDBCM_FAIL ;
      }

   done:
      if ( pArgumentBuffer )
      {
         SDB_OSS_FREE( pArgumentBuffer ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omStopDBNode( const CHAR * pExecName, const CHAR * pServiceName )
   {
      INT32 rc                = SDB_OK ;
      CHAR *pArgumentBuffer   = NULL ;
      INT32 argBuffLen        = 0 ;

      list<const CHAR*> argv ;
      ossResultCode result ;
      OSSPID pid ;

      argv.push_back( pExecName ) ;
      if ( pServiceName && pServiceName[0] )
      {
         argv.push_back( SDBCM_OPTION_PREFIX PMD_OPTION_SVCNAME ) ;
         argv.push_back( pServiceName ) ;
      }

      rc = ossBuildArguments( &pArgumentBuffer, argBuffLen, argv ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build sdbstop arguments, rc: %d",
                  rc ) ;
         goto error ;
      }
      // call exec to run the command with arguments,
      // do NOT wait until program finish
      rc = ossExec ( pArgumentBuffer, pArgumentBuffer, NULL,
                     OSS_EXEC_SSAVE, pid, result, NULL, NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to execute %s, rc: %d",
                  pArgumentBuffer, rc ) ;
         goto error ;
      }

      // verify the executing result
      if ( result.termcode != OSS_EXIT_NORMAL )
      {
         rc = SDBCM_FAIL ;
      }
      else
      {
         switch ( result.exitcode )
         {
            case SDB_OK:
               rc = SDB_OK ;
               break ;
            case STOPPART:
               rc = SDBCM_STOP_PART ;
               break ;
            default:
               rc = SDBCM_FAIL ;
         }
      }

   done:
      if ( pArgumentBuffer )
      {
         SDB_OSS_FREE( pArgumentBuffer ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omGetSvcListFromConfig( const CHAR * pCfgRootDir,
                                 vector < string > &svcList )
   {
      INT32 rc = SDB_OK ;

      rc = ossEnumSubDirs( pCfgRootDir, svcList, 1 ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to enum service in path[%s], rc: %d",
                 pCfgRootDir, rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omCheckDBProcessBySvc( const CHAR *svcname,
                                BOOLEAN &isRuning,
                                OSSPID &pid )
   {
      INT32 rc = SDB_OK ;
      isRuning = FALSE ;

#if defined( _WINDOWS )
      CHAR enginePipeName [ PROC_PIPE_NAME_LEN + 1 ] = { 0 } ;
      vector< string > names ;
      OSSNPIPE handle ;
      INT64 readSize = 0 ;
      BOOLEAN isOpen = FALSE ;

      ossSnprintf ( enginePipeName, PROC_PIPE_NAME_LEN,
                    ENGINE_NPIPE_PATTERN, svcname ) ;

      rc = ossEnumNamedPipes ( names, enginePipeName ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to enum pipes, rc: %d", rc ) ;

      if ( names.size() == 0 )
      {
         // process is not running
         goto done ;
      }
      else if ( names.size() != 1 )
      {
         PD_LOG( PDWARNING, "Name pipe[%s] size[%d] more than 1",
                 enginePipeName, names.size() ) ;
      }

      rc = ossOpenNamedPipe( enginePipeName, OSS_NPIPE_DUPLEX | OSS_NPIPE_BLOCK,
                             OSS_NPIPE_BLOCK_WITH_TIMEOUT, handle ) ;
      if ( rc && SDB_FE != rc )
      {
         PD_LOG ( PDERROR, "Failed to create named pipe: %s, rc: %d",
                  enginePipeName, rc ) ;
         goto error ;
      }
      isOpen = TRUE ;
      rc = ossWriteNamedPipe( handle, ENGINE_NPIPE_MSG_PID,
                              sizeof( ENGINE_NPIPE_MSG_PID ),
                              NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to send "ENGINE_NPIPE_MSG_PID" to %s, "
                  "rc: %d", enginePipeName, rc ) ;
         goto error ;
      }

      rc = ossReadNamedPipe( handle, (CHAR *)&pid, sizeof(pid), &readSize,
                             LIST_TIMEOUT ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to read pid from pipe %s",
                   enginePipeName ) ;
      PD_CHECK( sizeof( pid ) == readSize, SDB_SYS, error, PDERROR,
                "Failed to read pid from pipe %s", enginePipeName ) ;

      isRuning = TRUE ;

   done:
      if ( isOpen )
      {
         ossCloseNamedPipe( handle );
      }
#else
      DIR *pDir                  = NULL ;
      struct dirent *pDirent     = NULL ;
      CHAR engineName [ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
      BOOLEAN isOpen = FALSE ;

      pDir = opendir( PROC_PATH ) ;
      PD_CHECK( pDir != NULL, SDB_IO, error, PDERROR,
                "Failed to open the directory:%s, errno=%d",
                PROC_PATH, ossGetLastError() ) ;
      isOpen = TRUE ;
      ossSnprintf ( engineName, OSS_MAX_PATHSIZE, ENGINE_NAME_PATTERN,
                    svcname ) ;

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
         if ( 0 != ossStrcmp( commandLine, engineName ) )
         {
            continue ;
         }
         pid = ossAtoi( pDirent->d_name ) ;
         isRuning = TRUE ;
         break ;
      }
      if ( NULL == pDirent )
      {
         rc = SDBCM_NODE_NOTEXISTED ;
      }
   done:
      if ( isOpen )
      {
         closedir( pDir ) ;
      }
#endif // _WINDOWS
      return rc ;
   error:
      goto done ;
   }

}
