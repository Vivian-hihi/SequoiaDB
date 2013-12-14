/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossMem.h

   Descriptive Name = Operating System Services Memory Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declares for all memory
   allocation/free operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OSSMEM_H_
#define OSSMEM_H_
#include "core.h"

/*
 * [x bytes start][8 bytes data size][4 bytes guard size][4 bytes file hash][4
 * bytes line num][data][1 byte end][x-1 bytes stop]*/
#define SDB_MEMDEBUG_MINGUARDSIZE 256
#define SDB_MEMDEBUG_MAXGUARDSIZE 4194304
#define SDB_MEMDEBUG_GUARDSTART ((CHAR)0xBE)
#define SDB_MEMDEBUG_GUARDSTOP  ((CHAR)0xBF)
#define SDB_MEMDEBUG_GUARDEND   ((CHAR)0xBD)
#define SDB_MEMHEAD_EYECATCHER1 0xFABD0538
#define SDB_MEMHEAD_EYECATCHER2 0xFACE7352

#define SDB_OSS_MALLOC(x)       ossMemAlloc(x,__FILE__,__LINE__)
#define SDB_OSS_FREE(x)         ossMemFree(x)
#define SDB_OSS_REALLOC(x,y)    ossMemRealloc(x,y,__FILE__,__LINE__)

#define SDB_OSS_MALLOC3(x,y,z)  ossMemAlloc(x,y,z)

#define SAFE_OSS_FREE(p)      \
   do {                       \
      if (p) {                \
         SDB_OSS_FREE(p) ;    \
         p = NULL ;           \
      }                       \
   } while (0)

SDB_EXTERN_C_START
extern BOOLEAN ossMemDebugEnabled ;
extern UINT32 ossMemDebugSize ;

void* ossMemAlloc ( size_t size, const CHAR* file, UINT32 line ) ;

void* ossMemRealloc ( void* pOld, size_t size,
                      const CHAR* file, UINT32 line ) ;

void ossMemFree ( void *p ) ;

void ossMemTrack ( void *p ) ;

void ossMemUnTrack ( void *p ) ;

void ossMemTrace ( const CHAR *pPath ) ;

SDB_EXTERN_C_END
#endif
