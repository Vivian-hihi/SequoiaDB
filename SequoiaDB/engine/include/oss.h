/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = oss.hpp

   Descriptive Name = Operating System Services Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for OSS operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OSS_H_
#define OSS_H_

#include "core.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>

#if defined (_LINUX)
#include <errno.h>
#include <unistd.h>
#endif

#define ISEMPTYSTRING(x) (strlen(x)==0)
#define OSS_MAX_GROUPNAME_SIZE      127

#if defined (_LINUX)
#define OSS_FILE_SEP       "/"
#define OSS_FILE_SEP_CHAR  '/'
#define OSS_MAX_PATHSIZE   PATH_MAX
#elif defined (_WINDOWS)
#define OSS_FILE_SEP       "\\"
#define OSS_FILE_SEP_CHAR  '\\'
#define OSS_MAX_PATHSIZE   _MAX_PATH
#endif

#define OSS_DFT_SVCPORT 50000
#define OSS_RENAME_PROCESS_BUFFER_LEN 255
SDB_EXTERN_C_START
#if defined (_WINDOWS)
   #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
      #define DELTA_EPOCH_IN_MICROSECS  11644473600000000
   #else
      #define DELTA_EPOCH_IN_MICROSECS  11644473600000000LL
   #endif //(_MSC_VER) || defined(_MSC_EXTENSIONS)

struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

OSSPID ossGetParentProcessID();
OSSPID ossGetCurrentProcessID();
OSSTID ossGetCurrentThreadID();
UINT32 ossGetLastError();
void ossSleep(UINT32 milliseconds);
void ossPanic () ;
SDB_EXTERN_C_END
#endif // OSS_H_
