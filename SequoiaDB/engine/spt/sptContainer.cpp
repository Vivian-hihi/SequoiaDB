/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptContainer.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptContainer.hpp"
#include "sptSPScope.hpp"
#include "pd.hpp"

namespace engine
{
   _sptContainer::_sptContainer()
   {
   }

   _sptContainer::~_sptContainer()
   {
   }

   _sptScope *_sptContainer::newScope( SPT_SCOPE_TYPE type )
   {
      _sptScope *scope = NULL ;
      INT32 rc = SDB_OK ;
      if ( SPT_SCOPE_TYPE_SP == type )
      {
         scope = SDB_OSS_NEW _sptSPScope() ;
      }

      if ( NULL == scope )
      {
         PD_LOG( PDERROR, "it is a unknown type of scope:%d", type ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = scope->start() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init scope:%d", rc ) ;
         goto error ;
      }
   done:
      return scope ;
   error:
      SAFE_OSS_DELETE( scope ) ;
      scope = NULL ;
      goto done ;
   }
}

