/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnCoordSql.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnCoordSql.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "msgMessage.hpp"

namespace engine
{
   INT32 _rtnCoordSql::execute( CHAR *pReceiveBuffer, SINT32 packSize,
                                CHAR **ppResultBuffer, pmdEDUCB *cb,
                                MsgOpReply &replyHeader,
                                BSONObj **ppErrorObj )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB();
      SQL_CB *sqlcb = krcb->getSqlCB() ;
      NodeID curNodeID = krcb->getClsCB()->getNodeID() ;
      MsgHeader *header = (MsgHeader *)pReceiveBuffer;
      CHAR *sql = NULL ;
      SINT64 contextID = -1 ;
      rc = msgExtractSql( pReceiveBuffer, &sql ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract sql" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = sqlcb->exec( sql, cb, contextID ) ;
   done:
      msgBuildReplyMsgHeader( replyHeader,
                              sizeof(replyHeader),
                              header->opCode,
                              rc,
                              contextID, 0, 0,
                              curNodeID,
                              header->requestID ) ;
      return rc ;
   error:
      goto done ;
   }
}
