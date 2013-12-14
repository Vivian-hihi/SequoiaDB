/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossMem.cpp

   Descriptive Name = Operating System Services Memory Implementation

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains impl for all memory
   allocation/free operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "ossMem.hpp"
#include "ossMem.c"
#include "ossLatch.hpp"
#include "ossPrimitiveFileOp.hpp"
#include <set>

// in C++, we keep track of memory allocation
#if defined (__cplusplus) && defined (SDB_ENGINE)
static BOOLEAN ossMemTrackCBInit = FALSE ;
struct _ossMemTrackCB
{
   ossSpinXLatch  _memTrackMutex ;
   std::set<void*> _memTrackMap ;
   _ossMemTrackCB () { ossMemTrackCBInit = TRUE ; }
   ~_ossMemTrackCB () { ossMemTrackCBInit = FALSE ; }
} ;
typedef struct _ossMemTrackCB ossMemTrackCB ;
static ossMemTrackCB gMemTrackCB ;

void ossMemTrack ( void *p )
{
   if ( ossMemTrackCBInit )
   {
      gMemTrackCB._memTrackMutex.get() ;
      gMemTrackCB._memTrackMap.insert(p) ;
      gMemTrackCB._memTrackMutex.release() ;
   }
}
void ossMemUnTrack ( void *p )
{
   if ( ossMemTrackCBInit )
   {
      gMemTrackCB._memTrackMutex.get() ;
      gMemTrackCB._memTrackMap.erase(p) ;
      gMemTrackCB._memTrackMutex.release() ;
   }
}

#define OSSMEMTRACEDUMPBUFSZ 1024
// dump memory trace info for specific address into memtrace file
static UINT64 ossMemTraceDump ( void *p, ossPrimitiveFileOp &trapFile )
{
   CHAR lineBuffer [ OSSMEMTRACEDUMPBUFSZ + 1 ] = {0} ;
   CHAR *pAddr = (CHAR*)p ;
   ossMemset ( lineBuffer, 0, sizeof(lineBuffer) ) ;
   ossSnprintf ( lineBuffer, sizeof(lineBuffer),
                 " Address: 0x%p\n", pAddr ) ;
   trapFile.Write ( lineBuffer ) ;
   ossMemset ( lineBuffer, 0, sizeof(lineBuffer) ) ;
   ossSnprintf ( lineBuffer, sizeof(lineBuffer),
                 " Freed: %s, Size: %lld, DebugSize: %d\n",
                 (*(UINT32*)(pAddr+OSS_MEM_HEAD_FREEDOFFSET))==0?"false":"true",
                 (*(UINT64*)(pAddr+OSS_MEM_HEAD_SIZEOFFSET)),
                 (*(UINT32*)(pAddr+OSS_MEM_HEAD_DEBUGOFFSET)) ) ;
   trapFile.Write ( lineBuffer ) ;
   ossMemset ( lineBuffer, 0, sizeof(lineBuffer) ) ;
   ossSnprintf ( lineBuffer, sizeof(lineBuffer),
                 " File: 0x%x, Line: %d\n",
                 (*(UINT32*)(pAddr+OSS_MEM_HEAD_FILEOFFSET)),
                 (*(UINT32*)(pAddr+OSS_MEM_HEAD_LINEOFFSET)) ) ;
   trapFile.Write ( lineBuffer ) ;
   trapFile.Write ( "\n" ) ;
   return (*(UINT64*)(pAddr+OSS_MEM_HEAD_SIZEOFFSET)) ;
}

// dump memory trace info into memtrace file
void ossMemTrace ( const CHAR *pPath )
{
   ossPrimitiveFileOp trapFile ;
   CHAR fileName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   UINT32 StrLen = 0 ;
   UINT64 totalSize = 0 ;
   // skip if not initialized
   if ( !ossMemTrackCBInit )
   {
      // do NOT goto error since we didn't go into critical section yet
      return ;
   }
   // enter critical section
   gMemTrackCB._memTrackMutex.get() ;

   // sanity check
   if ( OSS_MAX_PATHSIZE <
        ossStrlen ( pPath ) + ossStrlen ( OSS_PRIMITIVE_FILE_SEP ) +
        ossStrlen ( SDB_OSS_MEMDUMPNAME ) )
   {
      // do NOT dump PD info since we should not reference pd component from
      // here
      goto error ;
   }
   // build mem trace file name
   ossMemset ( fileName, 0, sizeof ( fileName ) ) ;
   StrLen = ossSnprintf ( fileName, sizeof(fileName), "%s%s%s",
                          pPath, OSS_PRIMITIVE_FILE_SEP, SDB_OSS_MEMDUMPNAME ) ;
   // open memtrace file

   trapFile.Open ( fileName ) ;

   // dump into trap file
   if ( trapFile.isValid () )
   {
      trapFile.seekToEnd () ;
      trapFile.Write ( " -------- Memory Allocation Information --------\n" ) ;
      std::set<void*>::iterator it ;
      for ( it = gMemTrackCB._memTrackMap.begin() ;
            it != gMemTrackCB._memTrackMap.end() ;
            ++it )
      {
         void *p = *it ;
         totalSize += ossMemTraceDump ( p, trapFile ) ;
      }
      // record total allocated memory size
      ossMemset ( fileName, 0, sizeof ( fileName ) ) ;
      ossSnprintf ( fileName, sizeof(fileName),
                    " -------- Totally Allocated %lld Bytes --------\n",
                    totalSize ) ;
      trapFile.Write ( fileName ) ;
   }
done :
   trapFile.Close () ;
   gMemTrackCB._memTrackMutex.release() ;
   return ;
error :
   goto done ;
}
#endif
