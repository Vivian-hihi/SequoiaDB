/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsLogWrapper.cpp

   Descriptive Name = Data Protection Service Log Wrapper

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains code logic for log wrapper,
   which is also called DPS Control Block

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/01/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "dpsLogWrapper.hpp"
#include "dpsLogDef.hpp"
#include "dpsReplicaLogMgr.hpp"
#include "pd.hpp"
#include "dpsMergeBlock.hpp"
#include "dms.hpp"
#include "dpsOp2Record.hpp"
#include "pdTrace.hpp"
#include "dpsTrace.hpp"
#include "dpsLogRecordDef.hpp"
namespace engine
{
   const UINT64 PLACE_HOLDER = 0;
   const UINT64 LSN_HOLDER = 0;

   _dpsLogWrapper::_dpsLogWrapper()
   {
      _initialized = FALSE ;
      _dpslocal = FALSE ;
   }
   _dpsLogWrapper::~_dpsLogWrapper()
   {
   }

   void _dpsLogWrapper::writeData ( dpsMergeInfo & info )
   {
      _buf.writeData( info ) ;
   }

/*
   // insert record operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDINSERT, "_dpsLogWrapper::recordInsert" )
   INT32 _dpsLogWrapper::recordInsert( const CHAR *csName,
                                       const CHAR *clName,
                                       const BSONObj &obj,
                                       const DPS_TRANS_ID &transID,
                                       const DPS_LSN_OFFSET &preTransLsn,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDINSERT );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clName, "clName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock() ;
         block.clear() ;
         dpsLogRecord &record =  block.record() ;
         rc = dpsInsert2Record( csName, clName, obj,
                                transID, preTransLsn,
                                record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build insert record:%d", rc ) ;
            goto error ;
         }
      }

      rc = _buf.preparePages ( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDINSERT, rc );
      return rc ;
   error:
      goto done ;
   }
   // update record operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDUPDATE, "_dpsLogWrapper::recordUpdate" )
   INT32 _dpsLogWrapper::recordUpdate( const CHAR *csName,
                                       const CHAR *clName,
                                       const BSONObj &oldMatch,
                                       const BSONObj &oldObj,
                                       const BSONObj &newMatch,
                                       const BSONObj &newObj,
                                       const DPS_TRANS_ID &transID,
                                       const DPS_LSN_OFFSET &preTransLsn,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDUPDATE );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clName, "clName can't be NULL" )
         dpsMergeBlock &block = info.getMergeBlock () ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsUpdate2Record( csName, clName,
                                oldMatch, oldObj,
                                newMatch, newObj,
                                transID, preTransLsn,
                                record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build update record:%d",rc ) ;
            goto error ;
         }
      }
      rc =  _buf.preparePages ( info );
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDUPDATE, rc );
      return rc ;
   error:
      goto done ;
   }

   // record delete operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDDEL, "_dpsLogWrapper::recordDelete" )
   INT32 _dpsLogWrapper::recordDelete( const CHAR *csName,
                                       const CHAR *clName,
                                       const BSONObj &oldObj,
                                       const DPS_TRANS_ID &transID,
                                       const DPS_LSN_OFFSET &preTransLsn,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDDEL );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clName, "clName can't be NULL" )
         dpsMergeBlock &block = info.getMergeBlock () ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsDelete2Record( csName, clName,
                                oldObj,
                                transID, preTransLsn,
                                record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build delete record:%d",rc ) ;
            goto error ;
         }
      }
      rc =  _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDDEL, rc );
      return rc ;
   error:
      goto done ;
   }

   // create collection space operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDCSCRT, "_dpsLogWrapper::recordCScrt" )
   INT32 _dpsLogWrapper::recordCScrt ( const CHAR *csName, const INT32 &pageSize,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDCSCRT );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )

         _dpsMergeBlock &block = info.getMergeBlock () ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsCSCrt2Record( csName, pageSize, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build cscrt record:%d",rc ) ;
            goto error ;
         }

      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDCSCRT, rc );
      return rc ;
   error:
      goto done ;
   }

   // delete collection space operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDCSDEL, "_dpsLogWrapper::recordCSdel" )
   INT32 _dpsLogWrapper::recordCSdel ( const CHAR *csName,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDCSDEL );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock() ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsCSDel2Record( csName, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build csdel record:%d",rc ) ;
            goto error ;
         }
      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDCSDEL, rc );
      return rc ;
   error:
      goto done ;
   }

   // create collection operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDCLCRT, "_dpsLogWrapper::recordCLcrt" )
   INT32 _dpsLogWrapper::recordCLcrt ( const CHAR *csName,
                                       const CHAR *clName,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDCLCRT );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clName, "clName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock() ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsCLCrt2Record( csName, clName, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build clcrt record:%d",rc ) ;
            goto error ;
         }
      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDCLCRT, rc );
      return rc ;
   error:
      goto done ;
   }

   // drop collection operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDCLDEL, "_dpsLogWrapper::recordCLdel" )
   INT32 _dpsLogWrapper::recordCLdel ( const CHAR *csName,
                                       const CHAR *clName,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDCLDEL );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clName, "clName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock() ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsCLDel2Record( csName, clName, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build cldel record:%d",rc ) ;
            goto error ;
         }

      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDCLDEL, rc );
      return rc ;
   error:
      goto done ;
   }

   // create index operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDIXCRT, "_dpsLogWrapper::recordIXcrt" )
   INT32 _dpsLogWrapper::recordIXcrt ( const CHAR *csName,
                                       const CHAR *clName,
                                       const BSONObj &index,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDIXCRT );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clName, "clName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock() ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsIXCrt2Record( csName, clName, index, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build ixcrt record:%d",rc ) ;
            goto error ;
         }
      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDIXCRT, rc );
      return rc ;
   error:
      goto done ;
   }

   // drop index operation
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDIXDEL, "_dpsLogWrapper::recordIXdel" )
   INT32 _dpsLogWrapper::recordIXdel ( const CHAR *csName,
                                       const CHAR *clName,
                                       const BSONObj &index,
                                       dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDIXDEL );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clName, "csName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock () ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsIXDel2Record( csName, clName, index, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build ixdel record:%d",rc ) ;
            goto error ;
         }
      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDIXDEL, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDCLRN, "_dpsLogWrapper::recordCLrename" )
   INT32 _dpsLogWrapper::recordCLrename ( const CHAR *csName,
                                          const CHAR *clOldName,
                                          const CHAR *clNewName,
                                          dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDCLRN );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clOldName, "clOldName can't be NULL" )
         SDB_ASSERT ( clNewName, "clNewName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock() ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsCLRename2Record( csName, clOldName, clNewName, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build clrename record:%d",rc ) ;
            goto error ;
         }
      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDCLRN, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDCLTR, "_dpsLogWrapper::recordCLtrunc" )
   INT32 _dpsLogWrapper::recordCLtrunc ( const CHAR *csName,
                                         const CHAR *clName,
                                         dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDCLTR );
      if ( !_initialized )
         goto done ;
      {
         SDB_ASSERT ( csName, "csName can't be NULL" )
         SDB_ASSERT ( clName, "clName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock() ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsCLTrunc2Record( csName, clName, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build cltrunc record:%d",rc ) ;
            goto error ;
         }

      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDCLTR, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDTRANSCMM, "_dpsLogWrapper::recordTransCommit" )
   INT32 _dpsLogWrapper::recordTransCommit( const DPS_TRANS_ID &transID,
                                          dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDTRANSCMM );
      if ( !_initialized )
      {
         goto done;
      }
      {
         _dpsMergeBlock &block = info.getMergeBlock();
         block.clear();
         dpsLogRecord &record = block.record();
         rc = dpsTransCommit2Record( transID, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build transcommit record:%d",rc ) ;
            goto error ;
         }
      }
      rc =_buf.preparePages( info );
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDTRANSCMM, rc );
      return rc ;
   error:
      goto done ;
   }


   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDTRANSRB, "_dpsLogWrapper::recordTransRollback" )
   INT32 _dpsLogWrapper::recordTransRollback( const DPS_TRANS_ID &transID,
                                            const DPS_LSN_OFFSET &preTransLsn,
                                             dpsMergeInfo &info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDTRANSRB );
      if ( !_initialized )
      {
         goto done;
      }
      {
         _dpsMergeBlock &block = info.getMergeBlock();
         block.clear();
         dpsLogRecord &record = block.record();
         rc = dpsTransRollback2Record( transID, preTransLsn, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build transrollback record:%d",rc ) ;
            goto error ;
         }
      }
      rc =_buf.preparePages( info );
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDTRANSRB, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDIVCATA, "_dpsLogWrapper::recordInvalidateCata" )
   INT32 _dpsLogWrapper::recordInvalidateCata( const CHAR * clFullName,
                                               dpsMergeInfo & info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDIVCATA );
      if ( !_initialized )
      {
         goto done;
      }
      {
         SDB_ASSERT ( clFullName, "clFullName can't be NULL" )
         _dpsMergeBlock &block = info.getMergeBlock() ;
         block.clear() ;
         dpsLogRecord &record = block.record();
         rc = dpsInvalidCata2Record( clFullName, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build invaliddatecata record:%d",rc ) ;
            goto error ;
         }
      }
      rc = _buf.preparePages( info ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDIVCATA, rc );
      return rc ;
   error:
      goto done ;
   }
*/
   // record a row
   // PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_RECDROW, "_dpsLogWrapper::recordRow" )
   INT32 _dpsLogWrapper::recordRow( const CHAR *row, UINT32 len )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGWRAPP_RECDROW );
      if ( !_initialized )
      {
         goto done;
      }
      {
         SDB_ASSERT( NULL != row, "row should not be NULL!")
         _dpsMergeBlock block ;
//         block.clear() ;
         dpsLogRecord &record = block.record();
         dpsLogRecordHeader &header = record.head() ;
         ossMemcpy( &header, row, sizeof(dpsLogRecordHeader) );
         block.setRow( TRUE ) ;
         rc = record.push( DPS_LOG_ROW_ROWDATA,
                           header._length -  sizeof(dpsLogRecordHeader),
                           row + sizeof(dpsLogRecordHeader)) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to push row to record:%d",rc) ;
            goto error;
         }

         rc = _buf.merge( block );
      }
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGWRAPP_RECDROW, rc );
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _dpsLogWrapper::isInRestore()
   {
      return _buf.isInRestore();
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGWRAPP_PREPARE, "prepare" )
   INT32 _dpsLogWrapper::prepare( dpsMergeInfo &info )
   {
      PD_TRACE_ENTRY( SDB__DPSLGWRAPP_PREPARE ) ;
      INT32 rc = SDB_OK ;
      if ( !_initialized )
      {
         goto done;
      }
      rc = _buf.preparePages( info ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to prepare pages, rc = %d", rc ) ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB__DPSLGWRAPP_PREPARE, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}
