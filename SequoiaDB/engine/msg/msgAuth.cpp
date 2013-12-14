/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = msgAuth.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "msgAuth.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "msgTrace.hpp"


namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_EXTRACTAUTHMSG, "extractAuthMsg" )
   INT32 extractAuthMsg( MsgHeader *header, BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_EXTRACTAUTHMSG );
       CHAR *offset = NULL ;
      if ( NULL == header )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      offset = ( CHAR *)header + sizeof(MsgHeader) ;
      try
      {
         BSONObj tmp( offset ) ;
         obj = tmp ;
      }
      catch (std::exception &e)
      {
         PD_LOG( PDERROR, "unexpected err:%s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC ( SDB_EXTRACTAUTHMSG, rc );
      return rc ;
   error:
      goto done ;
   }
}
