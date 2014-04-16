/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptUtil.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptUtil.hpp"
#include "sptConvertor.hpp"


namespace engine
{
   INT32 utilGetBsonRawFromCtx( JSContext *cx, JSObject *obj, CHAR **raw )
   {
      INT32 rc = SDB_OK ;
      sptConvertor convertor( cx ) ;
      bson *bs = NULL ;
      rc = convertor.toBson( obj, &bs ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      bs->ownmem = FALSE ;
      *raw = bs->data ;
   done:
      if ( NULL != bs )
      {
         bson_dispose( bs ) ;
      }
      return rc ;
   error:
      goto done ;
   }   
}
