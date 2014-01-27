/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsReplayer.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains Runtime code for insert
   request.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLSREPLAYER_HPP_
#define CLSREPLAYER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "dpsLogRecord.hpp"
#include "rtnBackgroundJob.hpp"
#include "../bson/bsonobj.h"

using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;
   class _SDB_DMSCB ;
   class _dpsLogWrapper ;
   class _clsBucket ;

   /*
      _clsReplayer define
   */
   class _clsReplayer : public SDBObject
   {
   public:
      _clsReplayer( BOOLEAN useDps = FALSE ) ;
      ~_clsReplayer() ;

      void enableDPS () ;
      void disableDPS () ;
      BOOLEAN isDPSEnabled () const { return _dpsCB ? TRUE : FALSE ; }

   public:
      INT32 replay( dpsLogRecordHeader *recordHeader, _pmdEDUCB *eduCB,
                    BOOLEAN incCount = TRUE ) ;
      INT32 replayByBucket( dpsLogRecordHeader *recordHeader,
                            _pmdEDUCB *eduCB, _clsBucket *pBucket ) ;

      INT32 replayCrtCS( const CHAR *cs, INT32 pageSize,
                         _pmdEDUCB *eduCB, BOOLEAN delWhenExist = FALSE ) ;

      INT32 replayCrtCollection( const CHAR *collection,
                                 UINT32 attributes,
                                 _pmdEDUCB *eduCB ) ;

      INT32 replayIXCrt( const CHAR *collection,
                         BSONObj &index,
                         _pmdEDUCB *eduCB ) ;

      INT32 replayInsert( const CHAR *collection,
                          BSONObj &obj,
                          _pmdEDUCB *eduCB ) ;

      INT32 rollback( const dpsLogRecordHeader *recordHeader,
                      _pmdEDUCB *eduCB ) ;

   private:
      _SDB_DMSCB              *_dmsCB ;
      _dpsLogWrapper          *_dpsCB ;
      monDBCB                 *_monDBCB ;

   } ;
   typedef class _clsReplayer clsReplayer ;
}

#endif

