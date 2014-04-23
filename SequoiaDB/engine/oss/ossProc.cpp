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

   Source File Name = ossProc.hpp

   Descriptive Name = Operating System Services Process

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains implementation for process
   operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/24/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "core.hpp"
#if defined (_LINUX)
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/prctl.h>
#elif defined (_WINDOWS)
#include <ShlObj.h>
#endif
#include "ossProc.hpp"
#include "ossEDU.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"
#include "ossMem.hpp"
#include "pdTrace.hpp"
#include "ossTrace.hpp"
#if defined (_LINUX)
// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSISPROCRUNNING, "ossIsProcessRunning" )
BOOLEAN ossIsProcessRunning ( OSSPID pid )
{
   PD_TRACE_ENTRY ( SDB_OSSISPROCRUNNING );
   CHAR buffer [ OSS_MAX_PATHSIZE + 1 ] ;
   ossMemset ( buffer, 0, sizeof(buffer) ) ;
   ossSnprintf ( buffer, OSS_MAX_PATHSIZE, "/proc/%d", pid ) ;
   BOOLEAN ret = access ( buffer, F_OK ) != -1 ;
   PD_TRACE_EXIT ( SDB_OSSISPROCRUNNING );
   return ret ;
}

#define OSS_INVALID_MSG_QUEUE_ID -1
// Linux wait child process
// It calls waitpid until the given pid stop
// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSWAITCHLD, "ossWaitChild" )
INT32 ossWaitChild ( OSSPID pid, ossResultCode &result )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSWAITCHLD );
   INT32 err = 0 ;
   INT32 statuslocation ;
   // loop until the program finish
   do
   {
      rc = waitpid ( pid, &statuslocation, WUNTRACED ) ;
      err = errno ;
   } while ( -1 == rc && EINTR == err ) ;

   // if we can get return code or the child doesn't exist
   if ( -1 == rc || 0 == rc )
   {
      // child already terminated
      if ( -1 == rc && ECHILD == err )
      {
         result.termcode = OSS_EXIT_NORMAL ;
         result.exitcode = 0 ;
         rc = SDB_OK ;
      }
      else
      {
         PD_LOG ( PDERROR, "Failed to wait child, err = %d", err ) ;
         rc = SDB_SYS ;
      }
   }
   // otherwise let's check the status of the output process id
   else
   {
      if ( WIFEXITED ( statuslocation ) )
      {
         // if exited
         result.termcode = OSS_EXIT_NORMAL ;
         result.exitcode = WEXITSTATUS ( statuslocation ) ;
      }
      else if ( WIFSTOPPED ( statuslocation ) )
      {
         // if stopped
         switch ( WSTOPSIG ( statuslocation ) )
         {
         case 0 :
            result.termcode = OSS_EXIT_NORMAL ;
            break ;
         case SIGBUS :
            result.termcode = OSS_EXIT_ERROR ;
            break ;
         case SIGKILL :
         case SIGSTOP :
         case SIGCONT :
            result.termcode = OSS_EXIT_KILL ;
            break ;
         default :
            result.termcode = OSS_EXIT_TRAP ;
            break ;
         }
         result.exitcode = (UINT32)-1 ;
      }
      else
      {
         // if WIFSIGNALED() true
         INT32 sig = WTERMSIG ( statuslocation ) ;
         switch ( sig )
                  {
         case 0 :
            result.termcode = OSS_EXIT_NORMAL ;
            break ;
         case SIGBUS :
            result.termcode = OSS_EXIT_ERROR ;
            break ;
         case SIGKILL :
         case SIGSTOP :
         case SIGCONT :
            result.termcode = OSS_EXIT_KILL ;
            break ;
         default :
            result.termcode = OSS_EXIT_TRAP ;
            break ;
         }
         result.exitcode = sig ;
      }
      rc = SDB_OK ;
   }
   PD_TRACE_EXITRC ( SDB_OSSWAITCHLD, rc );
   return rc ;
}

#define OSS_FIRST_ARGUMENT_LEN 255
// create pointer list from character array
// the array may contain 0 or more arguments, each arguments are separated by
// '\0'. Two adjcent '\0\0' represent end of the string
// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSCRTLST, "ossCreateList" )
static INT32 ossCreateList ( const CHAR *pArguments,
                             const CHAR ***pppList,
                             INT32 iMinSize,
                             INT32 iStartingPos,
                             BOOLEAN isArgument )
{
   INT32 rc        = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSCRTLST );
   INT32 iNumArgs  = iMinSize ;
   const CHAR *p   = pArguments ;
   INT32 c         = 0 ;
   INT32 bufferLen = 0 ;
   // first let's count how many arguments we have
   if ( NULL != pArguments )
   {
      UINT32 count = 0 ;
      UINT32 i = 0 ;
      while ( pArguments[i] != '\0' )
      {
         while ( pArguments[i] != '\0' )
         {
            i++ ;
         }
         i++ ;
         count++ ;
      }
      iNumArgs += count ;
      bufferLen = i ;
      // if the original buffer size is not large enough to hold the rename
      // buffer, let's allocate a new one
      if ( isArgument && bufferLen < OSS_RENAME_PROCESS_BUFFER_LEN )
      {
         ++iNumArgs ;
         // allocate memory
         CHAR *pTempMem = (CHAR*)SDB_OSS_MALLOC
               ( OSS_RENAME_PROCESS_BUFFER_LEN ) ;
         // make sure allocation success
         PD_CHECK ( pTempMem, SDB_OOM, error, PDERROR,
                    "Failed to allocate memory for %d bytes",
                    OSS_RENAME_PROCESS_BUFFER_LEN ) ;
         // copy original arguments to new memory
         ossMemcpy ( pTempMem, pArguments, bufferLen ) ;
         // set the last argument to all space
         ossMemset ( &pTempMem[bufferLen], ' ',
                     OSS_RENAME_PROCESS_BUFFER_LEN-bufferLen ) ;
         // set last byte to two '\0'
         pTempMem[OSS_RENAME_PROCESS_BUFFER_LEN-1] = '\0' ;
         pTempMem[OSS_RENAME_PROCESS_BUFFER_LEN-2] = '\0' ;
         p = pTempMem ;
      }
   }

   // allocate memory, caller is responsible to free memory
   *pppList = (const CHAR **)SDB_OSS_MALLOC ( iNumArgs * sizeof(*pppList) ) ;
   if ( !*pppList )
   {
      PD_LOG ( PDERROR, "Failed to create argument list" ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( *pppList, 0, sizeof(*pppList)*iNumArgs ) ;
   // assign pppList element to argument
   if ( p )
   {
      while ( '\0' != *p )
      {
         (*pppList)[c] = p ;
         ++c ;
         while ( '\0' != *p )
            ++p ;
         ++p ;
      }
   }
   (*pppList)[c] = NULL ;
done :
   PD_TRACE_EXITRC ( SDB_OSSCRTLST, rc );
   return rc ;
error :
   goto done ;
}

// called by ossExec
// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSEXEC2, "ossExec2" )
static INT32 ossExec2 ( const CHAR *program,
                        const CHAR *arguments,
                        const CHAR *environment,
                        INT32 msgQueue,
                        pid_t &pid,
                        INT32 flag,
                        OSSNPIPE * const npHandleStdin,
                        OSSNPIPE * const npHandleStdout )
{
   INT32 rc                = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSEXEC2 );
   BOOLEAN bInheritHandles = FALSE ;
   INT32 err               = 0 ;
   INT32 pipeDescStdIn[2] = { SDB_INVALID_FH, SDB_INVALID_FH } ;
   INT32 pipeDescStdOut[2] = { SDB_INVALID_FH, SDB_INVALID_FH } ;
   if ( OSS_EXEC_INHERIT_HANDLES & flag )
   {
      bInheritHandles = TRUE ;
   }

   // if the caller want stdout
   if ( NULL != npHandleStdout )
   {
      if ( -1 == pipe ( pipeDescStdOut ) )
      {
         err = ossGetLastError () ;
         PD_LOG ( PDERROR, "Failed to create pipe, err = %d", err ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   }
   // if the caller want stdin
   if ( NULL != npHandleStdin )
   {
       if ( -1 == pipe ( pipeDescStdIn ) )
       {
         err = ossGetLastError () ;
         PD_LOG ( PDERROR, "Failed to create pipe, err = %d", err ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   }
   // let's fork the process
   pid = fork() ;
   if ( -1 == pid )
   {
      // can't fork
      err = ossGetLastError () ;
      PD_LOG ( PDERROR, "Failed to fork process, err = %d", err ) ;
      if ( EAGAIN == err )
      {
         rc = SDB_OSS_NORES ;
      }
      else if ( ENOMEM == err )
      {
         rc = SDB_OOM ;
      }
      else
      {
         rc = SDB_SYS ;
      }
      goto error ;
   }
   // check if i'm the new process or old one
   if ( 0 == pid )
   {
      const CHAR ** ppArgv   = NULL ;
      const CHAR ** ppEnvv   = NULL ;
      if ( npHandleStdin )
      {
         // if we want to handle pipe in, we have to redirect pipe to stdin fd
         close ( pipeDescStdIn[1] ) ;
         dup2 ( pipeDescStdIn[0], STDIN_FILENO ) ;
         // since dup2 duplicate fd, but fd still share real file descriptor, we
         // shouldn't close the original one
      }
      if ( npHandleStdout )
      {
         // if we want to handle pipe out, we have to redirect pipe to stdout fd
         close ( pipeDescStdOut[0] ) ;
         dup2 ( pipeDescStdOut[1], STDOUT_FILENO ) ;
         dup2 ( STDOUT_FILENO, STDERR_FILENO ) ;
         // since dup2 duplicate fd, but fd still share real file descriptor, we
         // shouldn't close the original one
      }
      // if this is child process
      if ( !bInheritHandles )
      {
         // if we don't want to inherit any handles
         // first let's close all opened file descriptors
         if ( ( NULL != npHandleStdout ) ||
              ( NULL != npHandleStdin ) )
            ossCloseAllOpenFileHandles ( FALSE ) ;
         else
            ossCloseAllOpenFileHandles ( TRUE ) ;
      }
      // we have to reset all signal handlers to default
      ossSigSet sigSet ;
      // fill out all signals ( not SIGKILL and SIGSTOP are ignored in the
      // function ossRegisterSignalHandle
      sigSet.fillSet () ;
      rc = ossRegisterSignalHandle( sigSet, SIG_DFL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to register signal handlers, rc = %d", rc ) ;
         _exit ( rc ) ;
      }
      // create argument list, memory will be freed by end of the function
      rc = ossCreateList ( arguments, &ppArgv, 1, 0, TRUE ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to create list, rc = %d", rc ) ;
         _exit ( rc ) ;
      }
      // create environment list, memory will be freed by end of the function
      rc = ossCreateList ( environment, &ppEnvv, 1, 0, FALSE ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to create list, rc = %d", rc ) ;
         SDB_OSS_FREE ( ppArgv ) ;
         ppArgv = NULL ;
         _exit ( rc ) ;
      }

      // execute the program
      if ( environment != NULL )
      {
         rc = execve ( program, (OSS_EXECV_CAST)ppArgv,
                       (OSS_EXECV_CAST)ppEnvv ) ;
      }
      else
      {
         rc = execvp ( program, (OSS_EXECV_CAST)ppArgv ) ;
      }
      // if this code is reached, something goes wrong with exec
      if ( ppArgv )
      {
         if ( ppArgv[0] )
            SDB_OSS_FREE ( (CHAR*)ppArgv[0] ) ;
         SDB_OSS_FREE ( ppArgv ) ;
      }
      if ( ppEnvv )
         SDB_OSS_FREE ( ppEnvv ) ;
      if ( rc == -1 )
      {
         err = ossGetLastError () ;
         if ( msgQueue != OSS_INVALID_MSG_QUEUE_ID )
         {
            // write error code into queue
            msgsnd ( msgQueue, &err, sizeof(err), 0 ) ;
         }
         _exit ( err ) ;
      }
   } // if ( 0 == pid )
   else
   {
      // parent should create a valid OSSNPIPE
      if ( npHandleStdout )
      {
         // create a pipe for child's stdout stream, so from parent we should
         // create a pipe with inbound
         close ( pipeDescStdOut[1] ) ;
         pipeDescStdOut[1] = SDB_INVALID_FH ;
         ossMemset ( npHandleStdout, 0, sizeof(OSSNPIPE) ) ;
         npHandleStdout->_handle = pipeDescStdOut[0] ;
         npHandleStdout->_state = OSS_NPIPE_INBOUND |
                                  OSS_NPIPE_BLOCK_WITH_TIMEOUT ;
         npHandleStdout->_bufSize = fpathconf ( pipeDescStdOut[0],
                                                _PC_PIPE_BUF ) ;
         if ( -1 == npHandleStdout->_bufSize )
         {
            err = ossGetLastError () ;
            PD_LOG ( PDERROR, "Failed to get buf size, error = %d", err ) ;
            rc = SDB_SYS ;
            npHandleStdout->_handle = SDB_INVALID_FH ;
            goto error ;
         }
      }
      if ( npHandleStdin )
      {
         // create a pipe for child's stdin stream, so from parent we should
         // create a pipe with outbound
         close ( pipeDescStdIn[0] ) ;
         pipeDescStdIn[0] = SDB_INVALID_FH ;
         ossMemset ( npHandleStdin, 0, sizeof(OSSNPIPE) ) ;
         npHandleStdin->_handle = pipeDescStdIn[1] ;
         npHandleStdin->_state = OSS_NPIPE_OUTBOUND |
                                 OSS_NPIPE_BLOCK_WITH_TIMEOUT ;
         npHandleStdin->_bufSize = fpathconf ( pipeDescStdIn[1],
                                               _PC_PIPE_BUF ) ;
         if ( -1 == npHandleStdin->_bufSize )
         {
            err = ossGetLastError () ;
            PD_LOG ( PDERROR, "Failed to get buf size, error = %d", err ) ;
            rc = SDB_SYS ;
            npHandleStdin->_handle = SDB_INVALID_FH ;
            goto error ;
         }
      }
   }
done :
   PD_TRACE1 ( SDB_OSSEXEC2, PD_PACK_INT(pid) );
   PD_TRACE_EXITRC ( SDB_OSSEXEC2, rc );
   return rc ;
error :
   if ( SDB_INVALID_FH != pipeDescStdIn[1] )
      close ( pipeDescStdIn[1] ) ;
   if ( SDB_INVALID_FH != pipeDescStdIn[0] )
      close ( pipeDescStdIn[0] ) ;
   if ( SDB_INVALID_FH != pipeDescStdOut[1] )
      close ( pipeDescStdOut[1] ) ;
   if ( SDB_INVALID_FH != pipeDescStdOut[0] )
      close ( pipeDescStdOut[0] ) ;
   goto done ;
}

// function to execute program.
// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSEXEC, "ossExec" )
INT32 ossExec ( const CHAR * program,
                const CHAR * arguments,
                const CHAR * environment,
                INT32 flag,
                OSSPID &pid,
                ossResultCode &result,
                OSSNPIPE * const npHandleStdin,
                OSSNPIPE * const npHandleStdout )
{
   INT32 rc                                   = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSEXEC );
   struct sigaction    ignore ;
   struct sigaction    savechild ;
   sigset_t            childmask ;
   sigset_t            savemask ;
   BOOLEAN             restoreSigMask         = FALSE ;
   BOOLEAN             restoreSIGCHLDHandling = FALSE ;
   BOOLEAN             queueCreated           = FALSE ;
   INT32               err                    = 0 ;
   INT32               sysRC                  = 0 ;
   INT32               msgQueue               = 0 ;
   INT32               retcode                = 0 ;
   INT32               failedMessage          = 0 ;
   INT32               msgRecvBytes           = 0 ;
   if ( OSS_EXEC_SSAVE & flag )
   {
      // we should block SIGCHLD and rembmer caller's mask
      sigemptyset ( &childmask ) ;
      sigaddset ( &childmask, SIGCHLD ) ;
      // set new mask, save old mask
      err = pthread_sigmask( SIG_BLOCK, &childmask, &savemask ) ;
      if ( err )
      {
         PD_LOG ( PDERROR, "Failed to block sigchld, err=%d", err ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      // once we changed signal mask, we have to restore it later
      restoreSigMask = TRUE ;
      // change sigchld action to default
      ignore.sa_handler = SIG_DFL ;
      sigemptyset ( &ignore.sa_mask ) ;
      ignore.sa_flags = 0 ;
      sysRC = sigaction ( SIGCHLD, &ignore, &savechild ) ;
      if ( sysRC < 0 )
      {
         PD_LOG ( PDERROR, "Failed to run sigaction, sysRC = %d", sysRC ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      // once we change signal handler, we have to restore it later
      restoreSIGCHLDHandling = TRUE ;

      // get a message queue to communicate with child
      msgQueue = msgget ( IPC_PRIVATE, IPC_CREAT | IPC_EXCL |
                                       S_IXOTH | S_IXUSR |
                                       S_IRUSR | S_IWUSR ) ;
      if ( -1 == msgQueue )
      {
         rc = SDB_OSS_NORES ;
         goto error ;
      }
      queueCreated = TRUE ;
      // execute the program
      retcode = ossExec2 ( program, arguments, environment, msgQueue, pid,
                           flag, npHandleStdin, npHandleStdout ) ;
      if ( SDB_OK == retcode )
      {
         // wait for program to terminate
         retcode = ossWaitChild ( pid, result ) ;
         if ( retcode )
         {
            PD_LOG ( PDERROR, "Failed to wait for child, rc = %d", retcode ) ;
            goto error ;
         }
         PD_LOG ( ( (0==result.termcode) && (0==result.exitcode) ) ?
                  PDINFO:PDERROR,
                  "Process %d is terminated with termcode %d, exitcode %d", pid,
                  result.termcode, result.exitcode ) ;
         // if a message exists on the queue, then exec() failed, and let's get
         // failed message
         msgRecvBytes = msgrcv ( msgQueue, &failedMessage,
                                 sizeof(failedMessage),
                                 0, IPC_NOWAIT ) ;
         if ( msgRecvBytes > 0 )
         {
            // usually this means something wrong with execv
            PD_LOG ( PDERROR, "Child process %d is failed with code %d",
                     pid, failedMessage ) ;
            rc = failedMessage ;
            goto done ;
         }
         else if ( -1 == msgRecvBytes  )
         {
            // if failed to receive ( ex, someone removed ipc queue )
            // or everything goes fine :)
            err = errno ;
            if ( ( ENOMSG == err ) || (EINVAL == err ) )
            {
               // if we have ENOMSG and retcode = 0, that means everything is
               // good
               if ( retcode != 0 )
               {
                  PD_LOG ( PDERROR, "Cannot find msg in queue, retcode=%d",
                           retcode ) ;
                  rc = retcode ;
                  goto error ;
               }
            }
            else
            {
               PD_LOG ( PDERROR, "Error receive from queue, err=%d", err ) ;
               goto error ;
            }
         }
         else
         {
            PD_LOG ( PDERROR, "Error receive from queue, retcode=%d", retcode );
            goto error ;
         }
      } // if ( SDB_OK == retcode )
   } // if ( OSS_EXEC_SSAVE & exec_flag )
   else
   {
      // if we don't need to wait for child, let's simply call ossExec2
      msgQueue = OSS_INVALID_MSG_QUEUE_ID ;
      retcode = ossExec2 ( program, arguments, environment, msgQueue, pid,
                           flag, npHandleStdin, npHandleStdout ) ;
   }
done :
   if ( restoreSIGCHLDHandling )
   {
      sigaction ( SIGCHLD, &savechild, NULL ) ;
   }
   if ( restoreSigMask )
   {
      pthread_sigmask ( SIG_SETMASK, &savemask, NULL ) ;
   }
   if ( queueCreated )
   {
      sysRC = msgctl ( msgQueue, IPC_RMID, NULL ) ;
      if ( (sysRC) && ((retcode!=0)||(EINVAL!=errno)))
      {
         PD_LOG ( PDERROR, "Failed to remove message queue, errno=%d", errno ) ;
      }
   }
   PD_TRACE1 ( SDB_OSSEXEC, PD_PACK_INT(pid) );
   PD_TRACE_EXITRC ( SDB_OSSEXEC, rc );
   return rc ;
error :
   goto done ;
}

static CHAR **g_specProgramName = NULL ;
static CHAR   g_WorkBuffer [ OSS_RENAME_PROCESS_BUFFER_LEN ] ;
static size_t g_workBufferLen   = OSS_RENAME_PROCESS_BUFFER_LEN ;
static size_t g_origBufferLen   = 0 ;
BOOLEAN g_bNameChangeEnabled    = FALSE ;

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSENBNMCHGS, "ossEnableNameChanges" )
void ossEnableNameChanges ( const INT32 argc, CHAR **pArgv0 )
{
   PD_TRACE_ENTRY ( SDB_OSSENBNMCHGS );
   g_specProgramName = pArgv0 ;
   g_origBufferLen   = 0 ;
   for ( INT32 i = 0; i < argc; ++i )
   {
      g_origBufferLen += ossStrlen ( pArgv0[i] ) + 1 ;
   }
   g_bNameChangeEnabled = TRUE ;
   PD_TRACE_EXIT ( SDB_OSSENBNMCHGS );
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSRENMPROC, "ossRenameProcess" )
void ossRenameProcess ( const CHAR *pNewName )
{
   PD_TRACE_ENTRY ( SDB_OSSRENMPROC );
   SDB_ASSERT ( g_bNameChangeEnabled,
                "program must be enabled with name change" )
   // first copy to temp buffer
   ossStrncpy ( g_WorkBuffer, pNewName, g_workBufferLen ) ;
   UINT32 inputLen = ossStrlen ( g_WorkBuffer ) ;
   ossStrncpy ( g_specProgramName[0],
                g_WorkBuffer,
                g_origBufferLen ) ;
   if ( inputLen < g_origBufferLen )
   {
      ossMemset ( &g_specProgramName[0][inputLen], 0,
                  g_origBufferLen - inputLen ) ;
   }
   else
   {
      PD_LOG ( PDERROR, "new name is too long: %s, buffer size = %d",
               pNewName, g_origBufferLen ) ;
   }
   g_specProgramName[0][g_origBufferLen - 1] = '\0' ;
   PD_TRACE_EXIT ( SDB_OSSRENMPROC );
}

#elif defined (_WINDOWS)
// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSRSVPATH, "ossResolvePath" )
static INT32 ossResolvePath ( const CHAR *pPathToResolve,
                              CHAR       *pResolvedPath )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSRSVPATH );
   WCHAR *tSep = NULL ;
   WCHAR sep = '\\' ;
   LPWSTR lpszwProgName = NULL ;
   WCHAR  szwProgName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   WCHAR  szwCurrentPath [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   WCHAR  szwExecutablePath [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   LPCWSTR *pathList = NULL ;
   LPSTR  lpszProgName = NULL ;
   // lpszwProgName is free at end of the function
   rc = ossANSI2WC ( pPathToResolve, &lpszwProgName, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to resolve path: %s, rc = %d",
               pPathToResolve, rc ) ;
      goto error ;
   }
   wcscpy ( szwProgName, lpszwProgName ) ;
   // get executable path
   if ( !GetModuleFileName ( NULL, szwExecutablePath, OSS_MAX_PATHSIZE ) )
   {
      PD_LOG ( PDERROR, "Failed to GetModuleFileName, rc = %d",
               ossGetLastError() ) ;
      rc = SDB_SYS ;
      goto error ;
   }
   tSep = wcsrchr ( szwExecutablePath, sep ) ;
   if ( tSep )
   {
      *(++tSep) = '\0' ;
   }
   // get current path
   if ( !GetCurrentDirectory ( OSS_MAX_PATHSIZE, szwCurrentPath  ) )
   {
      PD_LOG ( PDERROR, "Failed to GetCurrentDir, rc = %d",
               ossGetLastError () ) ;
      rc = SDB_SYS ;
      goto error ;
   }
   // free at end of the function
   pathList = (LPCWSTR*)SDB_OSS_MALLOC ( sizeof(LPCWSTR) * 3 ) ;
   if ( !pathList )
   {
      PD_LOG ( PDERROR, "Failed to allocate pathList" ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   pathList[0] = szwCurrentPath ;
   pathList[1] = szwExecutablePath ;
   pathList[2] = NULL ;
   if ( !PathResolve ( szwProgName, pathList,
                       PRF_VERIFYEXISTS | PRF_DONTFINDLNK |
                       PRF_TRYPROGRAMEXTENSIONS | PRF_FIRSTDIRDEF ) )
   {
      PD_LOG ( PDERROR, "Failed to find program name: %s, rc = %d",
               pPathToResolve, ossGetLastError() ) ;
      rc = SDB_FNE ;
      goto error ;
   }
   // lpszProgName is free at end of the function
   rc = ossWC2ANSI ( szwProgName, &lpszProgName, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to resolve path: %s, rc = %d",
               pPathToResolve ) ;
      goto error ;
   }
   ossStrncpy ( pResolvedPath, lpszProgName, ossStrlen ( lpszProgName ) + 1 ) ;
done :
   if ( lpszwProgName )
      SDB_OSS_FREE ( lpszwProgName ) ;
   if ( lpszProgName )
      SDB_OSS_FREE ( lpszProgName ) ;
   if ( pathList )
      SDB_OSS_FREE ( pathList ) ;
   PD_TRACE_EXITRC ( SDB_OSSRSVPATH, rc );
   return rc ;
error :
   goto done ;
}

BOOLEAN ossIsProcessRunning ( OSSPID pid )
{
   HANDLE process = OpenProcess ( SYNCHRONIZE, FALSE, pid ) ;
   DWORD ret = WaitForSingleObject ( process, 0 ) ;
   CloseHandle ( process ) ;
   return ret == WAIT_TIMEOUT ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSWTINT, "ossWaitInterrupt" )
INT32 ossWaitInterrupt ( HANDLE handle, DWORD timeout )
{
   PD_TRACE_ENTRY ( SDB_OSSWTINT );
   DWORD rc ;
   // wait until the process stop, since we don't have IPC in windows, we can't
   // detect whether the process terminated properly or not
   rc = WaitForSingleObject ( handle, timeout ) ;
   switch ( rc )
   {
   case WAIT_OBJECT_0:
      rc = SDB_OK ;
      break ;
   case WAIT_TIMEOUT :
      rc = SDB_TIMEOUT ;
      break ;
   default :
      PD_LOG ( PDERROR, "Wait interrupt failed with code %d",
               ossGetLastError() ) ;
      rc = SDB_SYS ;
   }
   PD_TRACE_EXITRC ( SDB_OSSWTINT, rc );
   return rc ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSCRTPADUPHND, "ossCreatePipeAndDupHandle" )
static INT32 ossCreatePipeAndDupHandle ( PHANDLE const pReadHandle,
                                         PHANDLE const pWriteHandle,
                                         PSECURITY_ATTRIBUTES const pSecAttr,
                                         PHANDLE const pHandleToDuplicate,
                                         PHANDLE const pDuplicateHandle )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSCRTPADUPHND );
   SDB_ASSERT ( pReadHandle, "read handle can't be NULL" )
   SDB_ASSERT ( pWriteHandle, "write handle can't be NULL" )
   SDB_ASSERT ( pSecAttr, "attributes can't be NULL" )
   SDB_ASSERT ( pHandleToDuplicate, "handle to dup can't be NULL" )
   SDB_ASSERT ( pDuplicateHandle, "dup handle can't be NULL" )
   SDB_ASSERT ( pHandleToDuplicate == pReadHandle ||
                pHandleToDuplicate == pWriteHandle,
                "dup handle must be read or write" )
   HANDLE pid = GetCurrentProcess () ;
   if ( !CreatePipe ( pReadHandle, pWriteHandle, pSecAttr, 0 ) )
   {
      PD_LOG ( PDERROR, "Failed to create pipe, error = %d",
               ossGetLastError () ) ;
      rc = SDB_SYS ;
      goto error ;
   }
   // make sure the duplicated handle can't be inherite
   if ( !DuplicateHandle ( pid, *pHandleToDuplicate,
                           pid, pDuplicateHandle, 0, FALSE,
                           DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) )
   {
      PD_LOG ( PDERROR, "Failed to duplicate pipe, error = %d",
               ossGetLastError () ) ;
      rc = SDB_SYS ;
      goto error ;
   }
done :
   PD_TRACE_EXITRC ( SDB_OSSCRTPADUPHND, rc );
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_WIN_OSSEXEC, "ossExec" )
INT32 ossExec ( const CHAR * program,
                const CHAR * arguments,
                const CHAR * environment,
                INT32 flag,
                OSSPID &pid,
                ossResultCode &result,
                OSSNPIPE * const npHandleStdin,
                OSSNPIPE * const npHandleStdout )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_WIN_OSSEXEC );
   PROCESS_INFORMATION procInfo  = {0} ;
   STARTUPINFO        startInfo  = {0} ;
   DWORD              ntFlag     = NORMAL_PRIORITY_CLASS | DETACHED_PROCESS ;
   BOOLEAN            inheritH   = FALSE ;
   CHAR               progName [ OSS_MAX_PATHSIZE + 1 + 2 ] = {0} ; // +2 for ""
   CHAR               workProgName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   INT32              bufferLen  = 0 ;
   CHAR *             argBuffer  = NULL ;
   CHAR *             pArgs      = NULL ;
   LPWSTR             lpszwArgs  = NULL ;

   // security attribute is used to setup inheritable handles
   SECURITY_ATTRIBUTES secAttr = {'\0'} ;
   HANDLE stdinReadPipe = INVALID_HANDLE_VALUE ;
   HANDLE stdinWritePipeChild = INVALID_HANDLE_VALUE ;
   HANDLE stdoutReadPipeChild = INVALID_HANDLE_VALUE ;
   HANDLE stdoutWritePipe = INVALID_HANDLE_VALUE ;
   HANDLE pipeTemp = INVALID_HANDLE_VALUE ;
   // setup attribute so that child processes are able to inherite our handles
   if ( ( npHandleStdin != NULL ) || ( npHandleStdout != NULL ) )
   {
      secAttr.bInheritHandle = TRUE ;
      secAttr.nLength = sizeof(secAttr) ;
      secAttr.lpSecurityDescriptor = NULL ;
      startInfo.hStdInput = GetStdHandle ( STD_INPUT_HANDLE ) ;
      startInfo.hStdOutput = GetStdHandle ( STD_OUTPUT_HANDLE ) ;
      startInfo.hStdError = GetStdHandle ( STD_ERROR_HANDLE ) ;
   }
   if ( npHandleStdin )
   {
      ossMemset ( npHandleStdin, 0, sizeof(OSSNPIPE) ) ;

      startInfo.dwFlags |= STARTF_USESTDHANDLES ;
      rc = ossCreatePipeAndDupHandle ( &stdinReadPipe, &stdinWritePipeChild,
                                       &secAttr, &stdinWritePipeChild,
                                       &pipeTemp ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to create and dup pipe, rc = %d", rc ) ;
         goto error ;
      }
      startInfo.hStdInput    = stdinReadPipe ;
      npHandleStdin->_handle = pipeTemp ;
      npHandleStdin->_state  = OSS_NPIPE_OUTBOUND |
                               OSS_NPIPE_BLOCK_WITH_TIMEOUT ;
   }
   if ( npHandleStdout )
   {
      ossMemset ( npHandleStdout, 0, sizeof(OSSNPIPE) ) ;

      startInfo.dwFlags |= STARTF_USESTDHANDLES ;
      rc = ossCreatePipeAndDupHandle ( &stdoutReadPipeChild, &stdoutWritePipe,
                                       &secAttr, &stdoutReadPipeChild,
                                       &pipeTemp ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to create and dup pipe, rc = %d", rc ) ;
         goto error ;
      }
      startInfo.hStdOutput    = stdoutWritePipe ;
      startInfo.hStdError     = stdoutWritePipe ;
      npHandleStdout->_handle = pipeTemp ;
      npHandleStdout->_state  = OSS_NPIPE_INBOUND |
                                OSS_NPIPE_BLOCK_WITH_TIMEOUT ;
   }
   // check input arguments
   if ( ( flag & OSS_EXEC_INHERIT_HANDLES ) ||
        ( npHandleStdout ) || ( npHandleStdin) )
   {
      inheritH = TRUE ;
   }
   // parse arguments
   if ( arguments )
   {
      pArgs = (CHAR*)arguments ;
      INT32 arg0Len = 0 ;
      INT32 progLen = 0 ;
      rc = ossResolvePath ( arguments, workProgName ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to resolve argument: %s, rc = %d",
                  arguments, rc ) ;
         goto error ;
      }

      // if the path contains space, it's then ambiguous, so we have to add ""
      // around the command
      ossSnprintf ( progName, sizeof(progName), "\"%s\"",
                    workProgName ) ;
      arg0Len = ossStrlen ( arguments ) ;
      progLen = ossStrlen ( progName ) ;
      // iterate all argument until we find "\0\0"
      // this part we check if we have more than one argument in the list
      while ( TRUE )
      {
         INT32 argXLen = ossStrlen ( pArgs ) ;
         if ( ( '\0' == pArgs[argXLen] ) &&
              ( '\0' == pArgs[argXLen + 1] ) )
         {
            // iterate to end of the argument
            pArgs = &pArgs[argXLen] ;
            break ;
         }
         // iterate to the next argument
         pArgs = &pArgs[argXLen + 1] ;
      }

      // if the argument list include more than 1 argument, then we need to
      // calculate the size of buffer required to hold them
      if ( pArgs != arguments )
      {
         bufferLen = progLen + 1 + ( pArgs - &arguments[arg0Len] ) ;
      }
      else
      {
         bufferLen = progLen + 1;
      }
      // allocate memory, free by end of the function
      argBuffer = (CHAR*) SDB_OSS_MALLOC ( bufferLen+1 ) ;
      if ( !argBuffer )
      {
         PD_LOG ( PDERROR, "Failed to allocate argument buffer" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossMemset ( argBuffer, 0, bufferLen + 1) ;
      ossMemcpy ( argBuffer, progName, progLen ) ;
      ossMemcpy ( &argBuffer[progLen], &arguments[arg0Len],
                  bufferLen - progLen ) ;
      // iterate through and change all '\0' to ' '
      pArgs = argBuffer ;
      while ( TRUE )
      {
         INT32 argXLen = ossStrlen ( pArgs ) ;
         // break when we hit '\0''\0'
         if ( ( '\0' == pArgs[argXLen] ) &&
              ( '\0' == pArgs[argXLen + 1] ) )
         {
            break ;
         }
         pArgs[argXLen] = ' ' ;
         pArgs = &pArgs[argXLen + 1] ;
      }
      argBuffer [ bufferLen ] = '\0' ;
      pArgs = argBuffer ;
      PD_LOG ( PDINFO, "Execute command: %s", argBuffer ) ;
   } // if ( arguments )
   else
   {
      // if we don't have arguments
      if ( program )
      {
         CHAR *lpszProgName                         = NULL ;
         CHAR *pContext                             = NULL ;
         CHAR tempProgName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
         const CHAR *lpszRest                       = NULL ;
         INT32 restLen                              = 0 ;
         INT32 progLen                              = 0 ;
         // since we are going to use strtok to separate arguments, let's make a
         // dup of const char
         ossStrncpy ( tempProgName, program, OSS_MAX_PATHSIZE ) ;
         lpszProgName = ossStrtok ( tempProgName, " ", &pContext ) ;
         rc = ossResolvePath ( lpszProgName, workProgName ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to resolve argument: %s, rc = %d",
                     lpszProgName, rc ) ;
            goto error ;
         }
         ossSnprintf ( progName, sizeof(progName), "\"%s\"",
                       workProgName ) ;
         // check for and add input parameters to the buffer
         if ( NULL != (lpszRest = ossStrchr ( program, ' ' ) ) )
         {
            progLen = ossStrlen ( progName ) ;
            restLen = ossStrlen ( lpszRest ) ;
            argBuffer = (CHAR*)SDB_OSS_MALLOC ( progLen + restLen + 1 ) ;
            if ( !argBuffer )
            {
               PD_LOG ( PDERROR, "Failed to allocate buffer" ) ;
               rc = SDB_OOM ;
               goto error ;
            }
            ossStrncpy ( argBuffer, progName, progLen + 1 ) ;
            ossStrncat ( argBuffer, lpszRest, progLen + restLen + 1 ) ;
            pArgs = argBuffer ;
         }
         else
         {
            pArgs = progName ;
         }
      } // if ( program )
   }

   startInfo.cb                   = sizeof(STARTUPINFO) ;
   startInfo.lpReserved           = NULL ;
   startInfo.lpTitle              = NULL ;
   startInfo.lpDesktop            = NULL ;
   startInfo.dwX                  = 0 ;
   startInfo.dwY                  = 0 ;
   startInfo.dwXSize              = 0 ;
   startInfo.dwYSize              = 0 ;
   startInfo.wShowWindow          = SW_HIDE ;
   startInfo.lpReserved2          = NULL ;
   startInfo.cbReserved2          = 0 ;
   rc = ossANSI2WC ( pArgs, &lpszwArgs, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to convert args" ) ;
      goto error ;
   }
   // create a new process with lpszArgs for arguments ( and commands )
   // Note the first word in argument must be the command we are trying to
   // execute. And since the command may include space ( like C:\Program
   // Files\xxx ), we have to use double quotes to quote the command ( like
   // "C:\Program Files\xxx" ), and then followed by zero or more arguments
   // separated by space
   if ( !CreateProcess ( NULL,
                         lpszwArgs,         // arguments
                         NULL,              // process security
                         NULL,              // thread security
                         inheritH,          // inherit handles
                         ntFlag,
                         NULL,              // inherit my env
                         NULL,              // inherit current dir
                         &startInfo,        // start info structure
                         &procInfo ) )      // process infomration
   {
      rc = ossGetLastError () ;
      if ( ERROR_FILE_NOT_FOUND )
      {
         PD_LOG ( PDERROR, "Cannot find program to execute: %s", pArgs ) ;
         rc = SDB_FNE ;
      }
      else
      {
         PD_LOG ( PDERROR, "Failed to create process, GetLastError=%d", rc ) ;
         rc = SDB_SYS ;
      }
   }
   else
   {
      rc = SDB_OK ;
      if ( flag & OSS_EXEC_SSAVE )
      {
         // if we need to wait for result
         rc = ossWaitInterrupt ( procInfo.hProcess, INFINITE ) ;
         if ( rc == SDB_OK )
         {
            // get termination code
            DWORD pgm_rc ;
            if ( !GetExitCodeProcess ( procInfo.hProcess, &pgm_rc ) )
            {
               PD_LOG ( PDERROR, "Failed to get exit code for process, \
GetLastError=%d", ossGetLastError() ) ;
               rc = SDB_SYS ;
            }
            else
            {
               rc = SDB_OK ;
               result.termcode = OSS_EXIT_NORMAL ;
               result.exitcode = pgm_rc ;
            }
         } // if ( rc == SDB_OK )
      } // if ( results && ( flag & OSS_EXEC_SSAVE ) )
      CloseHandle ( procInfo.hThread ) ; // close thread handle obj
      CloseHandle ( procInfo.hProcess ) ; // close process handle obj
      pid = procInfo.dwProcessId ;
   }
done :
   if ( argBuffer )
      SDB_OSS_FREE ( argBuffer ) ;
   if ( lpszwArgs )
      SDB_OSS_FREE ( lpszwArgs ) ;
   if ( npHandleStdout != NULL &&
        INVALID_HANDLE_VALUE != stdoutWritePipe )
   {
      CloseHandle ( stdoutWritePipe ) ;
   }

   if ( npHandleStdin != NULL &&
        INVALID_HANDLE_VALUE != stdinReadPipe )
   {
      CloseHandle ( stdinReadPipe ) ;
   }
   PD_TRACE_EXITRC ( SDB_WIN_OSSEXEC, rc );
   return rc ;
error :
   if ( npHandleStdout != NULL )
   {
      if ( INVALID_HANDLE_VALUE != npHandleStdout->_handle )
      {
         CloseHandle ( npHandleStdout->_handle ) ;
      }
   }
   if ( npHandleStdin != NULL )
   {
      if ( INVALID_HANDLE_VALUE != npHandleStdin->_handle )
      {
         CloseHandle ( npHandleStdin->_handle ) ;
      }
   } 
   goto done ;
}
#endif

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSGETEWD, "ossGetEWD" )
INT32 ossGetEWD ( CHAR *pBuffer, INT32 maxlen )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSGETEWD );
#if defined (_LINUX)
   CHAR *tSep = NULL ;
   CHAR sep = '/' ;
   INT32 len = readlink(PROC_SELF_EXE, pBuffer, maxlen ) ;
   if ( len <= 0 || len >= maxlen )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   pBuffer[len] = '\0' ;
   tSep = ossStrrchr ( pBuffer, sep ) ;
   if ( tSep )
      *tSep = '\0' ;
#elif defined (_WINDOWS)
   LPSTR lpszPath = NULL ;
   WCHAR *tSep = NULL ;
   WCHAR sep = '\\' ;
   WCHAR lpszwPath[OSS_MAX_PATHSIZE + 1] = {0} ;
   if ( !GetModuleFileName ( NULL, lpszwPath, OSS_MAX_PATHSIZE ) )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   tSep = wcsrchr ( lpszwPath, sep ) ;
   // path with prefix "?\"
   if ( tSep )
      *tSep = '\0' ;
   // path not with prefix
   else
   {
      lpszwPath[0] = '.' ;
      lpszwPath[1] = '\0' ;
   }
   // lpszPath is free at the end of this function 
   rc = ossWC2ANSI ( lpszwPath, &lpszPath, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get ewd, rc = %d", rc ) ;
      goto error ;
   }
   INT32 len = ossStrlen ( lpszPath ) ;
   if ( len > maxlen )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossStrncpy ( pBuffer, lpszPath, len +1 ) ;
#endif
done:
#if defined (_WINDOWS)
   if ( lpszPath )
      SDB_OSS_FREE ( lpszPath ) ;
#endif
   PD_TRACE_EXITRC ( SDB_OSSGETEWD, rc );
   return rc ;
error:
   goto done ;
}

INT32 ossTerminateProcess( const OSSPID &pid, BOOLEAN force )
{
   INT32 rc = SDB_OK ;
#if defined (_LINUX)
   if ( -1 == ::kill(pid, force ? SIGKILL : SIGTERM ))
   {
      rc = SDB_SYS ;
      goto error ;
   }
#elif defined (_WINDOWS)
   HANDLE h = ::OpenProcess( PROCESS_TERMINATE, FALSE, pid ) ;
   if ( NULL == h )
   {
      PD_LOG( PDERROR, "failed to open process[%d]", pid ) ;
      rc = SDB_SYS ;
      goto error ;
   }
   if ( !::TerminateProcess( h, EXIT_FAILURE ) )
   {
      rc = SDB_SYS ;
      ::CloseHandle(h) ;
      goto error ;
   }

   if ( !::CloseHandle(h) )
   {
      PD_LOG( PDERROR, "failed to close handle" ) ;
      rc = SDB_SYS ;
      goto error;
   }
#endif
done:
   return rc ;
error:
   PD_LOG( PDERROR, "failed to terminate process[%d]:[%d]",
           pid, ossGetLastError() ) ;
   goto done ;
}
