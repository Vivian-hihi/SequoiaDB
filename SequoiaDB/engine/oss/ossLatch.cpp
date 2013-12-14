/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossLatch.cpp

   Descriptive Name = Operating System Services Latch

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for latch operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "ossLatch.hpp"

void ossLatch ( ossSLatch *latch, OSS_LATCH_MODE mode )
{
   if ( SHARED == mode )
      latch->get_shared () ;
   else
      latch->get () ;
}
void ossLatch ( ossXLatch *latch )
{
   latch->get () ;
}
void ossUnlatch ( ossSLatch *latch, OSS_LATCH_MODE mode )
{
   if ( SHARED == mode )
      latch->release_shared () ;
   else
      latch->release();
}
void ossUnlatch ( ossXLatch *latch )
{
   latch->release () ;
}
BOOLEAN ossTestAndLatch ( ossSLatch *latch, OSS_LATCH_MODE mode )
{
   if ( SHARED == mode )
      return latch->try_get_shared () ;
   else
      return latch->try_get();
}
BOOLEAN ossTestAndLatch ( ossXLatch *latch )
{
   return latch->try_get () ;
}

