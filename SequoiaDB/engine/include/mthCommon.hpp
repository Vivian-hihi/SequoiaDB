/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = mthCommon.hpp

   Descriptive Name = Method Common Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Method component.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/12/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MTHCOMMON_HPP__
#define MTHCOMMON_HPP__

#include "core.hpp"

namespace engine
{
   #define MTH_TEMPSTRBUFLEN     (128)

   INT32 mthAppendString ( CHAR **ppStr, INT32 &bufLen,
                           INT32 strLen, const CHAR *newStr,
                           INT32 newStrLen, INT32 *pMergedLen = NULL ) ;

   INT32 mthDoubleBufferSize ( CHAR **ppStr, INT32 &bufLen ) ;

}

#endif //MTHCOMMON_HPP__
