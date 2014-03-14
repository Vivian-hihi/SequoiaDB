/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = monDump.hpp

   Descriptive Name = Monitor Dump Header

   When/how to use: this program may be used on binary and text-formatted
   versions of monitoring component. This file contains declare for
   functions that generate result dataset for given resources.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MONDUMP_HPP_
#define MONDUMP_HPP_

#include "pmdEDU.hpp"
#include "rtnCB.hpp"
#include "rtnContext.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   #define MON_MASK_NODE_NAME          0x00000001
   #define MON_MASK_HOSTNAME           0x00000002
   #define MON_MASK_SERVICE_NAME       0x00000004
   #define MON_MASK_GROUP_NAME         0x00000008
   #define MON_MASK_IS_PRIMARY         0x00000010
   #define MON_MASK_SERVICE_STATUS     0x00000020
   #define MON_MASK_LSN_INFO           0x00000040
   #define MON_MASK_NODEID             0x00000080
   #define MON_MASK_ALL                0xFFFFFFFF

   INT32 monAppendSystemInfo ( BSONObjBuilder &ob,
                               UINT32 mask = MON_MASK_ALL ) ;

   void  monAppendVersion ( BSONObjBuilder &ob ) ;

   INT32 monDumpContextsFromCB ( pmdEDUCB *cb, rtnContextDump *context,
                                 SDB_RTNCB *rtncb, BOOLEAN simple = TRUE ) ;
   INT32 monDumpAllContexts ( SDB_RTNCB *rtncb, rtnContextDump *context,
                              BOOLEAN simple = TRUE ) ;
   INT32 monDumpSessionFromCB ( pmdEDUCB *cb, rtnContextDump *context,
                                BOOLEAN simple = TRUE ) ;
   INT32 monDumpAllSessions ( pmdEDUCB *cb, rtnContextDump *context,
                              BOOLEAN simple = TRUE ) ;

   INT32 monDumpMonSystem ( rtnContextDump *context ) ;

   INT32 monDumpMonDBCB ( rtnContextDump *context ) ;

   INT32 monDumpAllCollections ( SDB_DMSCB *dmsCB, rtnContextDump *context,
                                 BOOLEAN details = FALSE ) ;

   INT32 monDumpAllCollectionSpaces ( SDB_DMSCB *dmsCB, rtnContextDump *context,
                                      BOOLEAN details = FALSE ) ;

   INT32 monDumpAllStorageUnits ( SDB_DMSCB *dmsCB, rtnContextDump *context ) ;

   INT32 monDumpIndexes( vector<monIndex> &indexes, rtnContextDump *context ) ;

   INT32 monDumpTraceStatus ( rtnContextDump *context ) ;

   INT32 monDumpDatablocks( std::vector<dmsExtentID> &datablocks,
                            rtnContextDump *context ) ;

   INT32 monDumpIndexblocks( std::vector< BSONObj > &idxBlocks,
                             std::vector< dmsRecordID > &idxRIDs,
                             const CHAR *indexName,
                             dmsExtentID indexLID,
                             INT32 direction,
                             rtnContextDump *context ) ;

   void  monResetMon () ;

   INT32 monDBDumpStorageInfo( BSONObjBuilder &ob );

   INT32 monDBDumpProcMemInfo( BSONObjBuilder &ob );

   INT32 monDBDumpNetInfo( BSONObjBuilder &ob );

   INT32 monDBDumpLogInfo( BSONObjBuilder &ob );

   void monDumpLastOpInfo( BSONObjBuilder &ob, const monAppCB &moncb ) ;

}

#endif //MONDUMP_HPP_

