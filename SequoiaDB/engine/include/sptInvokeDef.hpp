/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptInvokeDef.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_INVOKEDEF_HPP_
#define SPT_INVOKEDEF_HPP_

#include "sptParamContainer.hpp"
#include "sptReturnVal.hpp"
#include "jsapi.h"

namespace engine
{
   namespace JS_INVOKER
   {
      typedef JSBool (*MEMBER_FUNC)(JSContext *cx , uintN argc , jsval *vp) ;

      typedef void (*DESTRUCT_FUNC)(JSContext *cx , JSObject *obj) ;

      typedef JSBool (*RESLOVE_FUNC)( JSContext *cx , JSObject *obj , jsid id ,
                            uintN flags , JSObject ** objp) ;
   }
}

#endif

