/*******************************************************************************

   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

*******************************************************************************/

#include "dpsTransCB.hpp"
#include "dpsTransLock.hpp"
#include "pdTrace.hpp"
#include "dpsTrace.hpp"
#include "pmdEDU.hpp"
#include "pmdEDUMgr.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pmdDef.hpp"
#include "dpsLogRecord.hpp"
#include "dpsMessageBlock.hpp"
#include "dpsLogRecordDef.hpp"

namespace engine
{

   dpsTransCB::dpsTransCB():_TransIDL48Cur(1)
   {
      _TransIDH16          = 0;
      _isOn                = FALSE;
      _doRollback          = FALSE;
      _isNeedSyncTrans     = TRUE;
      _maxUsedSize         = 0;
      _logFileTotalSize    = 0;
      _accquiredSpace      = 0;
      _isInit              = FALSE;
   }

   dpsTransCB::~ dpsTransCB()
   {
   }

   DPS_TRANS_ID dpsTransCB::allocTransID()
   {
      DPS_TRANS_ID temp = 0;
      while ( 0 == temp )
      {
         temp = _TransIDL48Cur.inc();
         temp = ( temp & DPS_TRANSID_SN_BIT ) | _TransIDH16 |
                DPS_TRANSID_FIRSTOP_BIT;
      }
      return temp ;
   }

   void dpsTransCB::setNodeID( UINT16 nodeID )
   {
      _TransIDH16 = (DPS_TRANS_ID)nodeID << 48 ;
   }

   DPS_TRANS_ID dpsTransCB::getRollbackID( DPS_TRANS_ID transID )
   {
      return transID | DPS_TRANSID_ROLLBACKTAG_BIT ;
   }

   DPS_TRANS_ID dpsTransCB::getTransID( DPS_TRANS_ID rollbackID )
   {
      return rollbackID & DPS_TRANSID_VALID_BIT ;
   }

   BOOLEAN dpsTransCB::isRollback( DPS_TRANS_ID transID )
   {
      if ( transID & DPS_TRANSID_ROLLBACKTAG_BIT )
      {
         return TRUE;
      }
      return FALSE;
   }

   BOOLEAN dpsTransCB::isFirstOp( DPS_TRANS_ID transID )
   {
      if ( transID & DPS_TRANSID_FIRSTOP_BIT )
      {
         return TRUE;
      }
      return FALSE;
   }

   void dpsTransCB::clearFirstOpTag( DPS_TRANS_ID &transID )
   {
      transID = transID & ( ~DPS_TRANSID_FIRSTOP_BIT );
   }

   INT32 dpsTransCB::startRollbackTask()
   {
      INT32 rc = SDB_OK;
      EDUID eduID = PMD_INVALID_EDUID;
      pmdEDUMgr *pEduMgr = NULL;
      if ( !_isOn )
      {
         goto done;
      }
      _isNeedSyncTrans = FALSE;
      _doRollback = TRUE;
      pEduMgr = pmdGetKRCB()->getEDUMgr();
      eduID = pEduMgr->getSystemEDU( EDU_TYPE_DPSROLLBACK_TASK );
      pEduMgr->postEDUPost( eduID, PMD_EDU_EVENT_ACTIVE, FALSE, NULL );
   done:
      return rc;
   }

   INT32 dpsTransCB::stopRollbackTask()
   {
      INT32 rc = SDB_OK;
      _doRollback = FALSE;
      return rc ;
   }

   BOOLEAN dpsTransCB::isDoRollback()
   {
      return _doRollback;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSCB_SVTRANSINFO, "dpsTransCB::saveTransInfo" )
   void dpsTransCB::updateTransInfo( DPS_TRANS_ID transID,
                                     DPS_LSN_OFFSET lsnOffset )
   {
      PD_TRACE_ENTRY ( SDB_DPSTRANSCB_SVTRANSINFO );
      if ( !_isOn || DPS_INVALID_TRANS_ID == transID ||
           pmdGetKRCB()->getDPSCB()->isInRestore() )
      {
         goto done;
      }

      if ( pmdGetKRCB()->getDBRole() != SDB_ROLE_DATA ||
           pmdGetKRCB()->getReplCB()->primaryIsMe() )
      {
         // in primary, transaction-info save in EDUCB
         goto done;
      }
      {
         transID = getTransID( transID );
         ossScopedLock _lock( &_MapMutex );

         if ( DPS_INVALID_LSN_OFFSET == lsnOffset )
         {
            // invalid-lsn means the transaction is complete
            _TransMap.erase( transID );
         }
         else
         {
            _TransMap[ transID ] = lsnOffset;
         }
      }
   done:
      PD_TRACE_EXIT ( SDB_DPSTRANSCB_SVTRANSINFO );
      return ;
   }

   void dpsTransCB::addTransInfo( DPS_TRANS_ID transID,
                                  DPS_LSN_OFFSET lsnOffset )
   {
      transID = getTransID( transID );
      ossScopedLock _lock( &_MapMutex );
      if ( _TransMap.find( transID ) == _TransMap.end() )
      {
         // it is means transaction is synchronous by log if transID is exist
         _TransMap[ transID ] = lsnOffset;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSCB_ADDTRANSCB, "dpsTransCB::addTransCB" )
   void dpsTransCB::addTransCB( DPS_TRANS_ID transID, _pmdEDUCB *eduCB )
   {
      {
         transID = getTransID( transID );
         ossScopedLock _lock( &_CBMapMutex );
         _cbMap[ transID ] = eduCB;
      }
      PD_TRACE_EXIT ( SDB_DPSTRANSCB_ADDTRANSCB );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSCB_DELTRANSCB, "dpsTransCB::delTransCB" )
   void dpsTransCB::delTransCB( DPS_TRANS_ID transID )
   {
      {
         transID = getTransID( transID );
         ossScopedLock _lock( &_CBMapMutex );
         _cbMap.erase( transID );
      }
      PD_TRACE_EXIT ( SDB_DPSTRANSCB_DELTRANSCB );
   }

   TRANS_MAP *dpsTransCB::getTransMap()
   {
      return &_TransMap;
   }

   void dpsTransCB::clearTransInfo()
   {
      _TransMap.clear();
      _cbMap.clear();
      _beginLsnIdMap.clear();
      _idBeginLsnMap.clear();
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSCB_SAVETRANSINFOFROMLOG, "dpsTransCB::saveTransInfoFromLog" )
   void dpsTransCB::saveTransInfoFromLog( const dpsLogRecord &record )
   {
      dpsLogRecord::iterator itr = record.find( DPS_LOG_PUBLIC_TRANSID ) ;
      DPS_TRANS_ID transID ;
      DPS_LSN_OFFSET lsn ;
      if ( !itr.valid() )
      {
         goto done;
      }
      transID = *((DPS_TRANS_ID *)itr.value()) ;
      lsn = record.head()._lsn ;
      updateTransInfo( transID, lsn ) ;
      if ( isFirstOp( transID ))
      {
         addBeginLsn( lsn, transID );
      }
      else if( DPS_INVALID_LSN_OFFSET == lsn )
      {
         delBeginLsn( transID );
      }
   done:
      PD_TRACE_EXIT ( SDB_DPSTRANSCB_SAVETRANSINFOFROMLOG );
      return ;
   }

   void dpsTransCB::addBeginLsn( DPS_LSN_OFFSET beginLsn, DPS_TRANS_ID transID )
   {
      SDB_ASSERT( beginLsn != DPS_INVALID_LSN_OFFSET, "invalid begin-lsn" )
      SDB_ASSERT( transID != DPS_INVALID_TRANS_ID, "invalid transaction-ID" )
      transID = getTransID( transID );
      ossScopedLock _lock( &_lsnMapMutex );
      _beginLsnIdMap[ beginLsn ] = transID;
      _idBeginLsnMap[ transID ] = beginLsn;
   }

   void dpsTransCB::delBeginLsn( DPS_TRANS_ID transID )
   {
      transID = getTransID( transID );
      ossScopedLock _lock( &_lsnMapMutex );
      DPS_LSN_OFFSET beginLsn;
      TRANS_LSN_ID_MAP::iterator iter = _idBeginLsnMap.find( transID );
      if ( iter != _idBeginLsnMap.end() )
      {
         beginLsn = iter->second;
         _beginLsnIdMap.erase( beginLsn );
         _idBeginLsnMap.erase( transID );
      }
   }

   DPS_LSN_OFFSET dpsTransCB::getOldestBeginLsn()
   {
      ossScopedLock _lock( &_lsnMapMutex );
      if ( _beginLsnIdMap.size() > 0 )
      {
         return _beginLsnIdMap.begin()->first;
      }
      return DPS_INVALID_LSN_OFFSET;
   }

   BOOLEAN dpsTransCB::isNeedSyncTrans()
   {
      return _isNeedSyncTrans;
   }

   void dpsTransCB::setIsNeedSyncTrans( BOOLEAN isNeed )
   {
      _isNeedSyncTrans = isNeed;
   }

   INT32 dpsTransCB::syncTransInfoFromLocal( DPS_LSN_OFFSET beginLsn )
   {
      INT32 rc = SDB_OK;
      DPS_LSN curLsn;
      curLsn.offset = beginLsn;
      _dpsMessageBlock mb(DPS_MSG_BLOCK_DEF_LEN);
      SDB_DPSCB *dpsCB = pmdGetKRCB()->getDPSCB();
      if ( !_isNeedSyncTrans || DPS_INVALID_LSN_OFFSET == beginLsn )
      {
         goto done;
      }
      clearTransInfo();
      while ( curLsn.offset!= DPS_INVALID_LSN_OFFSET &&
              curLsn.compareOffset( dpsCB->expectLsn().offset ) < 0 )
      {
         mb.clear();
         rc = dpsCB->search( curLsn, &mb );
         PD_RC_CHECK( rc, PDERROR, "Failed to search %lld in dpsCB, rc=%d",
                      curLsn.offset, rc );
         _dpsLogRecord record ;
         rc = record.load( mb.readPtr() ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to load log record, rc=%d", rc );
         saveTransInfoFromLog( record );
         curLsn.offset += record.head()._length;
      }
   done:
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DPSTRANSCB_TERMALLTRANS, "dpsTransCB::termAllTrans" )
   void dpsTransCB::termAllTrans()
   {
      {
         ossScopedLock _lock( &_CBMapMutex );
         TRANS_CB_MAP::iterator iterMap = _cbMap.begin();
         while( iterMap != _cbMap.end() )
         {
            iterMap->second->postEvent( pmdEDUEvent(
                                        PMD_EDU_EVENT_TRANS_STOP ) ) ;
            _cbMap.erase( iterMap++ );
         }
      }
      PD_TRACE_EXIT ( SDB_DPSTRANSCB_TERMALLTRANS );
   }

   INT32 dpsTransCB::transLockGetX( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                    UINT16 collectionID,
                                    const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.acquireX( eduCB, lockId );
   }

   INT32 dpsTransCB::transLockGetS( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                    UINT16 collectionID,
                                    const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.acquireS( eduCB, lockId );
   }

   INT32 dpsTransCB::transLockGetIX( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                     UINT16 collectionID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      dpsTransLockId lockId( logicCSID, collectionID, NULL );
      return _TransLock.acquireIX( eduCB, lockId );
   }

   INT32 dpsTransCB::transLockGetIS( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                     UINT16 collectionID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      dpsTransLockId lockId( logicCSID, collectionID, NULL );
      return _TransLock.acquireIS( eduCB, lockId );
   }

   void dpsTransCB::transLockRelease( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                      UINT16 collectionID,
                                      const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return ;
      }
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.release( eduCB, lockId );
   }

   void dpsTransCB::transLockReleaseAll( _pmdEDUCB *eduCB )
   {
      if ( !_isOn )
      {
         return ;
      }
      return _TransLock.releaseAll( eduCB );
   }

   BOOLEAN dpsTransCB::isTransOn()
   {
      return _isOn;
   }

   void dpsTransCB::setTransSwitch( BOOLEAN isOn )
   {
      _isOn = isOn;
   }

   INT32 dpsTransCB::transLockTestS( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                     UINT16 collectionID,
                                     const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.testS( eduCB, lockId );
   }

   INT32 dpsTransCB::transLockTestX( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                     UINT16 collectionID,
                                     const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.testX( eduCB, lockId );
   }

   INT32 dpsTransCB::transLockTryX( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                    UINT16 collectionID,
                                    const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.tryX( eduCB, lockId );
   }

   INT32 dpsTransCB::transLockTryS( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                    UINT16 collectionID,
                                    const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.tryS( eduCB, lockId );
   }

   INT32 dpsTransCB::tryOrAppendX( _pmdEDUCB *eduCB, UINT32 logicCSID,
                                   UINT16 collectionID,
                                   const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      SDB_ASSERT( collectionID!=DMS_INVALID_MBID, "invalid collectionID" )
      SDB_ASSERT( recordID, "recordID can't be NULL" )
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.tryOrAppendX( eduCB, lockId );
   }

   INT32 dpsTransCB::waitLock( _pmdEDUCB * eduCB, UINT32 logicCSID,
                              UINT16 collectionID,
                              const dmsRecordID *recordID )
   {
      if ( !_isOn )
      {
         return SDB_OK;
      }
      SDB_ASSERT( collectionID!=DMS_INVALID_MBID, "invalid collectionID" )
      SDB_ASSERT( recordID, "recordID can't be NULL" )
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.wait( eduCB, lockId );
   }

   BOOLEAN dpsTransCB::hasWait( UINT32 logicCSID, UINT16 collectionID,
                                const dmsRecordID *recordID)
   {
      if ( !_isOn )
      {
         return FALSE;
      }
      SDB_ASSERT( collectionID!=DMS_INVALID_MBID, "invalid collectionID" )
      SDB_ASSERT( recordID, "recordID can't be NULL" )
      dpsTransLockId lockId( logicCSID, collectionID, recordID );
      return _TransLock.hasWait( lockId );
   }

   INT32 dpsTransCB::reservedLogSpace( UINT32 length )
   {
      INT32 rc = SDB_OK;
      UINT64 usedSize = 0;
      if ( !_isOn )
      {
         goto done;
      }

      if ( !_isInit )
      {
         init();
      }

      {
         ossScopedLock _lock( &_maxFileSizeMutex );
         _accquiredSpace += length;
      }
      usedSize = usedLogSpace();
      if ( usedSize + _accquiredSpace >= _maxUsedSize )
      {
         rc = SDB_DPS_LOG_FILE_OUT_OF_SIZE ;
         ossScopedLock _lock( &_maxFileSizeMutex );
         if ( _accquiredSpace >= length )
         {
            _accquiredSpace -= length ;
         }
         else
         {
            _accquiredSpace = 0 ;
         }
         goto error ;
      }
   done:
      return rc;
   error:
      goto done;
   }

   void dpsTransCB::releaseLogSpace( UINT32 length )
   {
      if ( !_isOn )
      {
         return ;
      }
      ossScopedLock _lock( &_maxFileSizeMutex );
      if ( _accquiredSpace >= length )
      {
         _accquiredSpace -= length ;
      }
      else
      {
         _accquiredSpace = 0 ;
      }
   }

   INT32 dpsTransCB::init()
   {
      if ( !_isInit )
      {
         ossScopedLock _lock( &_maxFileSizeMutex ) ;
         if ( !_isInit )
         {
            UINT64 logFileSize = pmdGetKRCB()->getDPSCB()->getLogFileSz() ;
            UINT32 logFileNum = pmdGetKRCB()->getDPSCB()->getLogFileNum() ;
            _logFileTotalSize = logFileSize * logFileNum ;

            // (1).the max-size of operation-log(update) is 2*DMS_RECORD_MAX_SZ,
            // (2).the max-size of unavailable space in cross-file is 
            // 2*DMS_RECORD_MAX_SZ*logFileNum,
            // the available size is: availableSize =
            // _logFileTotalSize - (1) - (2) ;
            // the availableSize can used for operation-log and rollback-log,
            // so the size of operation-log is:
            // _maxUsedSize = availableSize / 2;
            _maxUsedSize = ( _logFileTotalSize - 2 * DMS_RECORD_MAX_SZ *
                             logFileNum ) / 2 ;

            // if the logFileSize is 32M, the caculation method is:
            // if the transaction-operation-log  caused X(MB) of log-file,
            // the rollback-log will caused up to 2X( 1X for normal rollback-log
            // and 1X for cross-file-space )
            UINT64 temp = _logFileTotalSize / 3 ;
            if ( _maxUsedSize < temp )
            {
               _maxUsedSize = temp;
            }

            _isInit = TRUE ;
         }
      }
      return SDB_OK ;
   }

   UINT64 dpsTransCB::usedLogSpace()
   {
      DPS_LSN_OFFSET beginLsnOffset;
      DPS_LSN_OFFSET curLsnOffset;
      DPS_LSN curLsn;
      SDB_DPSCB *dpsCB = pmdGetKRCB()->getDPSCB();
      UINT64 usedSize = 0;

      beginLsnOffset = getOldestBeginLsn();
      if ( DPS_INVALID_LSN_OFFSET == beginLsnOffset )
      {
         goto done;
      }
      curLsn = dpsCB->expectLsn();
      curLsnOffset = curLsn.offset;
      if ( DPS_INVALID_LSN_OFFSET == curLsnOffset )
      {
         goto done;
      }

      beginLsnOffset = beginLsnOffset % _logFileTotalSize ;
      curLsnOffset = curLsnOffset % _logFileTotalSize ;
      usedSize = ( curLsnOffset + _logFileTotalSize - beginLsnOffset ) %
                 _logFileTotalSize ;
   done:
      return usedSize ;
   }

   UINT64 dpsTransCB::remainLogSpace()
   {
      UINT64 remainSize = _logFileTotalSize ;
      UINT64 allocatedSize = 0 ;
      if ( !_isInit )
      {
         init();
      }

      if ( !_isOn )
      {
         goto done ;
      }

      {
      ossScopedLock _lock( &_maxFileSizeMutex ) ;
      allocatedSize = _accquiredSpace + usedLogSpace() ;
      }
      if ( _maxUsedSize > allocatedSize )
      {
         remainSize = _maxUsedSize - allocatedSize ;
      }
      else
      {
         remainSize = 0 ;
      }
   done:
      return remainSize ;
   }
}
