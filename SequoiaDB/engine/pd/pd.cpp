/*******************************************************************************

   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


   Source File Name = pd.cpp

   Descriptive Name = Problem Determination

   When/how to use: this program may be used on binary and text-formatted
   versions of PD component. This file contains functions dumping diag logs.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#if defined (_LINUX)
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include "pd.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "ossLatch.hpp"
#include "ossPrimitiveFileOp.hpp"
#if defined (SDB_ENGINE) || defined (SDB_TOOL)
#include <boost/thread.hpp>
#endif
#include "pdTrace.hpp"
const static CHAR *PDLEVELSTRING[] =
{
   "SEVERE",
   "ERROR",
   "EVENT",
   "WARNING",
   "INFO",
   "DEBUG"
};

const CHAR* getPDLevelDesp ( PDLEVEL level )
{
   return PDLEVELSTRING[(UINT32)level] ;
}

/*
 * Log Header format
 * Arguments:
 * 1) Year (UINT32)
 * 2) Month (UINT32)
 * 3) Day (UINT32)
 * 4) Hour (UINT32)
 * 5) Minute (UINT32)
 * 6) Second (UINT32)
 * 7) Microsecond (UINT32)
 * 8) Level (string)
 * 9) Process ID (UINT64)
 * 10) Thread ID (UINT64)
 * 11) File Name (string)
 * 12) Function Name (string)
 * 13) Line number (UINT32)
 * 14) Message
 */
const static CHAR *PD_LOG_HEADER_FORMAT="%04d-%02d-%02d-%02d.%02d.%02d.%06d\
               \
Level:%s"OSS_NEWLINE"PID:%-37dTID:%d"OSS_NEWLINE"Function:%-32sLine:%d"\
OSS_NEWLINE"File:%s"OSS_NEWLINE"Message:"OSS_NEWLINE"%s"OSS_NEWLINE OSS_NEWLINE;
/* extern variables */
PDLEVEL _curPDLevel = PD_DFT_DIAGLEVEL ;
CHAR _pdDiagLogPath [OSS_MAX_PATHSIZE+1] = {0} ;

/* private variables */
struct _pdLogFile : public SDBObject
{
   // ossPrimitiveFileOp is native file interface, which returns errno or
   // GetLastError, instead of database error code
   ossPrimitiveFileOp _logFile ;
   ossSpinXLatch _mutex ;
} ;
typedef struct _pdLogFile pdLogFile ;

// currently we only have diag log, we may have different types of logs later
enum _pdLogType
{
   PD_DIAGLOG = 0,
   PD_LOG_MAX
} ;
pdLogFile _pdLogFiles [ PD_LOG_MAX ] ;

PD_TRACE_DECLARE_FUNCTION ( SDB_PDLOGFILEWRITE, "pdLogFileWrite" )
// return code is errno
static INT32 pdLogFileWrite ( _pdLogType type, CHAR *pData )
{
   INT32 rc = SDB_OK ;
   BOOLEAN isOpened = FALSE ;
   PD_TRACE_ENTRY ( SDB_PDLOGFILEWRITE ) ;
   SDB_ASSERT ( type < PD_LOG_MAX, "type is out of range" )
   SINT64 dataSize = ossStrlen ( pData ) ;
   pdLogFile &logFile = _pdLogFiles[type] ;
   // lock file first
   logFile._mutex.get() ;
   // attempt to open the file
   rc = logFile._logFile.Open ( _pdDiagLogPath ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to open log file, errno = %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }
   isOpened = TRUE ;
   logFile._logFile.seekToEnd () ;
   PD_TRACE1 ( SDB_PDLOGFILEWRITE, PD_PACK_RAW ( pData, dataSize ) ) ;
   rc = logFile._logFile.Write ( pData, dataSize ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to reopen log file, errno = %d"OSS_NEWLINE,
                  rc ) ;
      goto error ;
   } // if ( rc )
done :
   if ( isOpened )
      logFile._logFile.Close() ;
   logFile._mutex.release() ;
   PD_TRACE_EXITRC ( SDB_PDLOGFILEWRITE, rc ) ;
   return rc ;
error :
   goto done ;
}

void pdLog(PDLEVEL level, const CHAR* func, const CHAR* file, UINT32 line, std::string message)
{
   pdLog(level, func, file, line, message.c_str());
}

/*
 * Problem Detemination Log
 * Input:
 * Log level (PDSEVERE/PDERROR/PDWARNING/PDINFO/PDEVENT/PDDEBUG)
 * function name (char*)
 * file name (char*)
 * line number (integer)
 * output string (char*)
 * <followed by arguments>
 * Output:
 *    N/A
 */
PD_TRACE_DECLARE_FUNCTION ( SDB_PDLOG, "pdLog" )
void pdLog(PDLEVEL level, const CHAR* func, const CHAR* file, UINT32 line, const CHAR* format, ...)
{
   INT32 rc = SDB_OK ;
   if ( _curPDLevel<level)
      return ;
   va_list ap;
   PD_TRACE_ENTRY ( SDB_PDLOG ) ;
   CHAR userInfo[PD_LOG_STRINGMAX];       // for user defined message
   CHAR sysInfo[PD_LOG_STRINGMAX];        // for log header
   struct tm otm ;
   struct timeval tv;
   struct timezone tz;
   time_t tt ;

#if defined (SDB_ENGINE) || defined (SDB_TOOL)
   // use thread specific pointer to make sure there's no nested pdLog (i.e.
   // calling pdLog in signal handler when the thread is already in pdLog
   // function will not proceed)
   BOOLEAN *pAmIInPD = NULL ;
   static boost::thread_specific_ptr<BOOLEAN> amIInPD ;
   try
   {
      if ( ! ( pAmIInPD = amIInPD.get() ) )
      {
         // first time called by this thread
         amIInPD.reset ( new BOOLEAN ) ;
         pAmIInPD = amIInPD.get() ;
      }
      // make sure we get the pointer and we are not already in pdlog
      if ( !pAmIInPD || TRUE == *pAmIInPD )
         goto done ;
      *pAmIInPD = TRUE ;
   }
   catch ( std::exception )
   {
      // if anything wrong happen, simply return because we can't log anything
      goto error ;
   }
#endif

   gettimeofday(&tv, &tz);
   tt = tv.tv_sec ;

#if defined (_WINDOWS)
   localtime_s( &otm, &tt ) ;
#else
   localtime_r( &tt, &otm ) ;
#endif

   // create the user information
   va_start(ap, format);
   vsnprintf(userInfo, PD_LOG_STRINGMAX, format, ap);
   va_end(ap);

   // create log header
   ossSnprintf(sysInfo, PD_LOG_STRINGMAX, PD_LOG_HEADER_FORMAT,
               otm.tm_year+1900,            // 1) Year (UINT32)
               otm.tm_mon+1,                // 2) Month (UINT32)
               otm.tm_mday,                 // 3) Day (UINT32)
               otm.tm_hour,                 // 4) Hour (UINT32)
               otm.tm_min,                  // 5) Minute (UINT32)
               otm.tm_sec,                  // 6) Second (UINT32)
               tv.tv_usec,                  // 7) Microsecond (UINT32)
               PDLEVELSTRING[level],        // 8) Level (string)
               ossGetCurrentProcessID(),    // 9) Process ID (UINT64)
               ossGetCurrentThreadID(),     // 10) Thread ID (UINT64)
               func,                        // 11) Function Name (string)
               line,                        // 12) Line number (UINT32)
               file,                        // 13) File Name (string)
               userInfo                     // 14) Message
   );

   // if we run outside engine, _pdLogPath may not be set, in this case we
   // simply output to screen
#if defined (_DEBUG) && defined (SDB_ENGINE)
   ossPrintf ( "%s"OSS_NEWLINE, sysInfo ) ;
#else
   /* We write into log file if the string is not empty */
   if ( _pdDiagLogPath[0] != '\0' )
#endif
   {
      rc = pdLogFileWrite ( PD_DIAGLOG, sysInfo ) ;
      if ( rc )
      {
         ossPrintf ( "Failed to write into log file, errno = %d"OSS_NEWLINE, rc ) ;
         ossPrintf ( "%s"OSS_NEWLINE, sysInfo ) ;
      }
   }
#if defined (SDB_ENGINE) || defined (SDB_TOOL)
   // make sure to reset this before leaving
   *pAmIInPD = FALSE ;
done :
#endif
   PD_TRACE_EXITRC ( SDB_PDLOG, rc ) ;
   return ;
#if defined (SDB_ENGINE) || defined (SDB_TOOL)
error :
   goto done ;
#endif
}

#ifdef _DEBUG
void pdassert(const CHAR* string, const CHAR* func, const CHAR* file, UINT32 line)
{
   pdLog(PDSEVERE, func, file, line, string);
   ossPanic() ;
}

void pdcheck(const CHAR* string, const CHAR* func, const CHAR* file, UINT32 line)
{
   pdLog(PDSEVERE, func, file, line, string);
}
#endif

