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

   Source File Name = sptCmdRunner.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptCmdRunner.hpp"
#include "ossProc.hpp"
#include "pd.hpp"
#include "ossMem.hpp"
#include "ossUtil.hpp"

#if defined (_LINUX)
#define SPT_SHELL_CALL "/bin/sh"
#define SPT_SHELL_CALL_LEN 7
#define SPT_SHELL_ARG "-c"
#define SPT_SHELL_ARG_LEN 2
#elif defined (_WINDOWS)
#define SPT_SHELL_CALL "cmd"
#define SPT_SHELL_CALL_LEN 3
#define SPT_SHELL_ARG "/c"
#define SPT_SHELL_ARG_LEN 2
#endif


namespace engine
{
   _sptCmdRunner::_sptCmdRunner()
   {

   }

   _sptCmdRunner::~_sptCmdRunner()
   {
      done() ; 
   }

   INT32 _sptCmdRunner::exec( const CHAR *cmd, UINT32 &exit )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != cmd, "can not be null" )

      UINT32 cmdLen = ossStrlen( cmd ) ;
      CHAR *arguments = NULL ;
      CHAR *cp = NULL ;
      //// arguments:   SPT_SHELL_CALL\0SPT_SHELL_ARG\0cmd\0\0
      UINT32 size = SPT_SHELL_CALL_LEN + SPT_SHELL_ARG_LEN + cmdLen + 4 ;
      ossResultCode res ;

      arguments = ( CHAR * )SDB_OSS_MALLOC( size ) ;
      if ( NULL == arguments )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      cp = arguments ;
      ossMemcpy( cp, SPT_SHELL_CALL, SPT_SHELL_CALL_LEN + 1 ) ;
      cp += ( SPT_SHELL_CALL_LEN + 1 ) ;
      ossMemcpy( cp, SPT_SHELL_ARG, SPT_SHELL_ARG_LEN + 1 ) ;
      cp += ( SPT_SHELL_ARG_LEN + 1 ) ;
      ossMemcpy( cp, cmd, cmdLen + 1 ) ;
      arguments[size - 1] = '\0' ;

      /// TODO: how about to achieve ossPopen() ?
      rc = ossExec( arguments, arguments, NULL,
                    OSS_EXEC_SSAVE | OSS_EXEC_NORESIZEARGV,
                    _id, res, NULL, &_out ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to exec cmd:%s, rc:%d",
                 cmd, rc ) ;
         goto error ;
      }

      exit = res.exitcode ;
   done:
      if ( NULL != arguments )
      {
         SDB_OSS_FREE( arguments ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptCmdRunner::read( CHAR *buf,
                              SINT64 len,
                              SINT64 &got )
   {
      INT32 rc = SDB_OK ;
      rc = ossReadNamedPipe( _out, buf, len, &got ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_EOF != rc )
         {
            PD_LOG( PDERROR, "failed to read data from pipe:%d", rc ) ;
         }
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptCmdRunner::done()
   {
      if ( -1 != _id )
      {
         ossCloseNamedPipe( _out ) ;
      }
      return SDB_OK ;
   }
}

