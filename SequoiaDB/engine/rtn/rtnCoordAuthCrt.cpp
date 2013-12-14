/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnCoordAuthCrt.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnCoordAuthCrt.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOAUTHCRT_EXECUTE, "rtnCoordAuthCrt::execute" )
   INT32 rtnCoordAuthCrt::execute( CHAR *pReceiveBuffer, SINT32 packSize,
                                   CHAR **ppResultBuffer, pmdEDUCB *cb,
                                   MsgOpReply &replyHeader,
                                   BSONObj **ppErrorObj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNCOAUTHCRT_EXECUTE ) ;
      rc = forward( pReceiveBuffer, packSize,
                      ppResultBuffer, cb, replyHeader,
                      MSG_AUTH_CRTUSR_RES, FALSE ) ;
      PD_TRACE_EXITRC ( SDB_RTNCOAUTHCRT_EXECUTE, rc ) ;
      return rc ;
   }
}
