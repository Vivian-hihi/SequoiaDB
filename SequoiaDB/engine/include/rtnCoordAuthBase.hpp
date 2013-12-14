/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnCoordAuthBase.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTNCOORDAUTHBASE_HPP_
#define RTNCOORDAUTHBASE_HPP_

#include "rtnCoordOperator.hpp"

namespace engine
{
   class rtnCoordAuthBase : public rtnCoordOperator
   {
   protected:
      INT32 forward(CHAR *pReceiveBuffer, SINT32 packSize,
                    CHAR **ppResultBuffer, pmdEDUCB *cb,
                    MsgOpReply &replyHeader, INT32 msgType,
                    BOOLEAN sWhenNoPrimary = TRUE ) ;

   } ;
}

#endif

