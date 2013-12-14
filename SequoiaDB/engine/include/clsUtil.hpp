/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsUitl.hpp

   Descriptive Name = Replication Control Block Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Replication component. This file contains structure for
   replication control block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLSUTIL_HPP_
#define CLSUTIL_HPP_

#include <map>

#include "clsDef.hpp"
#include "netDef.hpp"

using namespace std ;

namespace engine
{
   void clsStrcpy( const CHAR *src, CHAR *dst, const UINT32 &dstLen ) ;

   CLS_SYNC_STATUS clsSyncWindow( const DPS_LSN &remoteLsn,
                                  const DPS_LSN &fileBeginLsn,
                                  const DPS_LSN &memBeginLSn,
                                  const DPS_LSN &endLsn ) ;

   void clsJoin2Full( const CHAR *cs, const CHAR *collection,
                      CHAR *full ) ;


}

#endif

