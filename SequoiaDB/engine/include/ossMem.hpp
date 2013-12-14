/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossMem.hpp

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
#ifndef OSSMEM_HPP_
#define OSSMEM_HPP_
#include "ossMem.h"
#include <new>

#define SDB_OSS_MEMDUMPNAME      "memdump.info"
#define SDB_OSS_NEW              new(__FILE__,__LINE__,std::nothrow)
#define SDB_OSS_DEL              delete

#define SAFE_OSS_DELETE(p) \
   do {                    \
      if (p) {             \
         SDB_OSS_DEL p ;   \
         p = NULL ;        \
      }                    \
   } while (0)

/*
void* operator new (size_t size, const CHAR *file, UINT32 line ) ;

void operator delete ( void *p, const CHAR *file, UINT32 line ) ;*/
#endif

