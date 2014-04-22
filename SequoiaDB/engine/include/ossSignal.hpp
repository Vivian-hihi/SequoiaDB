/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossSignal.hpp

   Descriptive Name = Operating System Services Signal Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains structure and functions for
   signal processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OSS_SIGNAL_HPP
#define OSS_SIGNAL_HPP

#include <setjmp.h>

#if defined (_LINUX)
   #include <signal.h>
   #include <errno.h>
   #include <sys/ucontext.h>
   #include <sys/resource.h>
   #include <pthread.h>
   // stack dump signal for linux: 23
   #define OSS_STACK_DUMP_SIGNAL SIGURG
   #define OSS_STACK_DUMP_SIGNAL_INTERNAL SIGUSR1

   typedef ucontext_t * ossSignalContext ;

   /* Set Handler for Signal */
   typedef siginfo_t *           oss_siginfo_t ;

   #define OSS_HANDPARMS         int signum, oss_siginfo_t sigcode, void * scp
   #define OSS_HANDARGS_DUMMY    0, 0, 0
   #define OSS_HANDARGS          signum, sigcode, scp

#elif defined (_WINDOWS)
   #include <windows.h>

   #define OSS_HANDPARMS         unsigned long signum, \
                                 PEXCEPTION_RECORD pExceptionRecord, \
                                 PCONTEXT          pContextRecord
   #define OSS_HANDARGS_DUMMY    0, 0, 0
   #define OSS_HANDARGS          signum, pExceptionRecord, pContextRecord
#endif  // ifdef _LINUX



/*
 * Wrapper macros for sigsetjmp and siglongjmp
 *     ossSetJump
 *     ossLongJump
 */
#if defined (_LINUX)
   #define ossLongJump(jmpBuf, arg) siglongjmp( jmpBuf, arg )
   #define ossSetJump( jb, arg) sigsetjmp ( jb, arg )
#elif defined (_WINDOWS)
   #define ossLongJump(jmpBuf, arg) longjmp( jmpNuf, arg )
   #define sqloSetJump( jb, arg) setjmp ( jb )
#else
   #error Undefined operating system
#endif

typedef void (* OSS_SIGFUNCPTR)(OSS_HANDPARMS) ;

#endif // OSS_SIGNAL_HPP
