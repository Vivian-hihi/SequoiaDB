/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = catSplit.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          19/07/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CAT_SPLIT_HPP__
#define CAT_SPLIT_HPP__

#include "core.hpp"
#include "pd.hpp"
#include "oss.hpp"
#include "ossErr.h"
#include "../bson/bson.h"
#include "catDef.hpp"
#include "pmd.hpp"
#include "clsTask.hpp"

using namespace bson ;

namespace engine
{

   INT32 catSplitPrepare( const BSONObj &splitInfo, const CHAR *clFullName,
                          clsCatalogSet *cataSet, INT32 &groupID,
                          pmdEDUCB *cb ) ;

   INT32 catSplitReady( const BSONObj &splitInfo, const CHAR *clFullName,
                        clsCatalogSet *cataSet, INT32 &groupID,
                        clsTaskMgr &taskMgr, pmdEDUCB *cb, INT16 w,
                        UINT64 *pTaskID = NULL ) ;

   INT32 catSplitStart( const BSONObj &splitInfo, pmdEDUCB *cb, INT16 w ) ;

   INT32 catSplitChgMeta( const BSONObj &splitInfo, const CHAR *clFullName,
                          clsCatalogSet *cataSet, pmdEDUCB *cb, INT16 w ) ;

   INT32 catSplitCleanup( const BSONObj &splitInfo, pmdEDUCB *cb, INT16 w ) ;

   INT32 catSplitFinish( const BSONObj &splitInfo, pmdEDUCB *cb, INT16 w ) ;

   INT32 catSplitCancel( const BSONObj &splitInfo, pmdEDUCB *cb,
                         INT32 &groupID, INT16 w ) ;

   INT32 catSplitCheckConflict( BSONObj &match, clsSplitTask &splitTask,
                                BOOLEAN &conflict, pmdEDUCB *cb ) ;

}


#endif //CAT_SPLIT_HPP__

