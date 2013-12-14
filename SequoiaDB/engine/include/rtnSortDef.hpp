/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSortDef.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains declare for runtime
   functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTNSORTDEF_HPP_
#define RTNSORTDEF_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.hpp"

using namespace bson ;

namespace engine
{
   enum RTN_SORT_STEP
   {
      RTN_SORT_STEP_BEGIN = 0,
      RTN_SORT_STEP_FETCH_FROM_INTER,
      RTN_SORT_STEP_FETCH_FROM_MERGE,
   } ;

  const UINT32 RTN_SORT_MIN_BUFSIZE = 128 ;
  const UINT32 RTN_SORT_MIN_MERGESIZE = 3 ;
  const UINT32 RTN_SORT_MAX_MERGESIZE = 10 ;

}

#endif

