/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsOp2Record.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains implementation for log record.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/05/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSOP2RECORD_HPP_
#define DPSOP2RECORD_HPP_

#include "dpsLogRecord.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   /// warning: any value can not be value-passed.
   INT32 dpsInsert2Record( const CHAR *fullName,
                           const BSONObj &obj,
                           const DPS_TRANS_ID &transID,
                           const DPS_LSN_OFFSET &preTransLsn,
                           dpsLogRecord &record ) ;

   INT32 dpsRecord2Insert( const CHAR *logRecord,
                           const CHAR **fullName,
                           BSONObj &obj ) ;

   INT32 dpsUpdate2Record( const CHAR *fullName,
                           const BSONObj &oldMatch,
                           const BSONObj &oldObj,
                           const BSONObj &newMatch,
                           const BSONObj &newObj,
                           const DPS_TRANS_ID &transID,
                           const DPS_LSN_OFFSET &preTransLsn,
                           dpsLogRecord &record ) ;

   INT32 dpsRecord2Update( const CHAR *logRecord,
                           const CHAR **fullName,
                           BSONObj &oldMatch,
                           BSONObj &oldObj,
                           BSONObj &newMatch,
                           BSONObj &newObj ) ;

   INT32 dpsDelete2Record( const CHAR *fullName,
                           const BSONObj &oldObj,
                           const DPS_TRANS_ID &transID,
                           const DPS_LSN_OFFSET &preTransLsn,
                           dpsLogRecord &record ) ;

   INT32 dpsRecord2Delete( const CHAR *logRecord,
                           const CHAR **fullName,
                           BSONObj &oldObj ) ;

   INT32 dpsCSCrt2Record( const CHAR *csName,
                          const INT32 &pageSize,
                          dpsLogRecord &record ) ;

   INT32 dpsRecord2CSCrt( const CHAR *logRecord,
                          const CHAR **csName,
                          INT32 &pageSize ) ;

   INT32 dpsCSDel2Record( const CHAR *csName,
                          dpsLogRecord &record ) ;

   INT32 dpsRecord2CSDel( const CHAR *logRecord,
                          const CHAR **csName ) ;

   INT32 dpsCLCrt2Record( const CHAR *fullName,
                          UINT32 attribute,
                          dpsLogRecord &record ) ;

   INT32 dpsRecord2CLCrt( const CHAR *logRecord,
                          const CHAR **fullName,
                          UINT32 &attribute ) ;

   INT32 dpsCLDel2Record( const CHAR *fullName,
                          dpsLogRecord &record ) ;

   INT32 dpsRecord2CLDel( const CHAR *logRecord,
                          const CHAR **fullName ) ;

   INT32 dpsIXCrt2Record( const CHAR *fullName,
                          const BSONObj &index,
                          dpsLogRecord &record ) ;

   INT32 dpsRecord2IXCrt( const CHAR *logRecord,
                          const CHAR **fullName,
                          BSONObj &index ) ;

   INT32 dpsIXDel2Record( const CHAR *fullName,
                          const BSONObj &index,
                          dpsLogRecord &record ) ;

   INT32 dpsRecord2IXDel( const CHAR *logRecord,
                          const CHAR **fullName,
                          BSONObj &index ) ;

   INT32 dpsCLRename2Record( const CHAR *csName,
                             const CHAR *clOldName,
                             const CHAR *clNewName,
                             dpsLogRecord &record ) ;

   INT32 dpsRecord2CLRename( const CHAR *logRecord,
                             const CHAR **csName,
                             const CHAR **clOldName,
                             const CHAR **clNewName ) ;

   INT32 dpsCLTrunc2Record( const CHAR *fullName,
                            dpsLogRecord &record ) ;

   INT32 dpsTransCommit2Record( const DPS_TRANS_ID &transID,
                                dpsLogRecord &record ) ;

   INT32 dpsRecord2TransCommit( const CHAR *logRecord,
                                DPS_TRANS_ID &transID ) ;

//   INT32 dpsTransRollback2Record( const DPS_TRANS_ID &transID,
//                                  const DPS_LSN_OFFSET &preTransLsn,
//                                  dpsLogRecord &record ) ;

   INT32 dpsInvalidCata2Record( const CHAR * clFullName,
                                dpsLogRecord &record ) ;

   INT32 dpsRecord2InvalidCata( const CHAR *logRecord,
                                const CHAR **clFullName ) ;

}

#endif

