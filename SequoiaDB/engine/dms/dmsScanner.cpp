/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = dmsTBScanner.hpp

   Descriptive Name = Data Management Service Storage Unit Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          22/08/2013  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "dmsScanner.hpp"
#include "dmsStorageIndex.hpp"
#include "dmsStorageDataCommon.hpp"
#include "rtnIXScanner.hpp"
#include "rtnDiskIXScanner.hpp"
#include "rtnMergeIXScanner.hpp"
#include "rtnIXScannerFactory.hpp"
#include "bpsPrefetch.hpp"
#include "dmsCompress.hpp"
#include "dmsTransContext.hpp"
#include "dpsTransLockMgr.hpp"
#include "dpsTransExecutor.hpp"
#include "dpsTransLockCallback.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "dmsTrace.hpp"


using namespace bson ;

namespace engine
{

   #define DMS_IS_WRITE_OPR(accessType)   \
      ( DMS_ACCESS_TYPE_UPDATE == accessType || \
        DMS_ACCESS_TYPE_DELETE == accessType ||\
        DMS_ACCESS_TYPE_INSERT == accessType )

   #define DMS_IS_READ_OPR(accessType) \
      ( DMS_ACCESS_TYPE_QUERY == accessType || \
        DMS_ACCESS_TYPE_FETCH == accessType )

   /*
      _dmsScanner implement
   */
   _dmsScanner::_dmsScanner( dmsStorageDataCommon *su, dmsMBContext *context,
                             mthMatchRuntime *matchRuntime,
                             DMS_ACCESS_TYPE accessType,
                             _dpsITransLockCallback * callback )
   {
      SDB_ASSERT( su, "storage data can't be NULL" ) ;
      SDB_ASSERT( context, "context can't be NULL" ) ;
      _pSu = su ;
      _context = context ;
      _matchRuntime = matchRuntime ;
      _accessType = accessType ;
      _mbLockType = SHARED ;
      _callback = callback;
      _transIsolation = TRANS_ISOLATION_RU ;
      _waitLock = FALSE ;
      _useRollbackSegment = TRUE ;


      if ( DMS_IS_WRITE_OPR( _accessType ) )
      {
         _mbLockType = EXCLUSIVE ;
      }
   }

   _dmsScanner::~_dmsScanner()
   {
      _context    = NULL ;
      _pSu        = NULL ;
   }

   /*
      _dmsExtScannerBase implement
   */
   _dmsExtScannerBase::_dmsExtScannerBase( dmsStorageDataCommon *su,
                                           dmsMBContext *context,
                                           mthMatchRuntime *matchRuntime,
                                           dmsExtentID curExtentID,
                                           DMS_ACCESS_TYPE accessType,
                                           INT64 maxRecords,
                                           INT64 skipNum,
                                           INT32 flag,
                                           _dpsITransLockCallback * callback )
   :_dmsScanner( su, context, matchRuntime, accessType, callback ),
    _curRecordPtr( NULL )
   {
      _maxRecords          = maxRecords ;
      _skipNum             = skipNum ;
      _next                = DMS_INVALID_OFFSET ;
      _firstRun            = TRUE ;
      _extent              = NULL ;
      _pTransCB            = NULL ;
      _curRID._extent      = curExtentID ;
      _recordLock          = DPS_TRANSLOCK_MAX ;
      _needUnLock          = FALSE ;
      _selectForUpdate     = FALSE ;
      _cb                  = NULL ;

      if ( OSS_BIT_TEST( flag, FLG_QUERY_FOR_UPDATE ) )
      {
         _selectForUpdate = TRUE ;
      }
   }

   _dmsExtScannerBase::~_dmsExtScannerBase ()
   {
      _extent     = NULL ;

      if ( FALSE == _firstRun && _recordLock != DPS_TRANSLOCK_MAX &&
           DMS_INVALID_OFFSET != _curRID._offset )
      {
         _pTransCB->transLockRelease( _cb, _pSu->logicalID(), _context->mbID(),
                                      &_curRID, _callback ) ;
      }
   }

   dmsExtentID _dmsExtScannerBase::nextExtentID() const
   {
      if ( _extent )
      {
         return _extent->_nextExtent ;
      }
      return DMS_INVALID_EXTENT ;
   }

   INT32 _dmsExtScannerBase::stepToNextExtent()
   {
      if ( 0 != _maxRecords &&
           DMS_INVALID_EXTENT != nextExtentID() )
      {
         _curRID._extent = nextExtentID() ;
         _firstRun = TRUE ;
         return SDB_OK ;
      }
      return SDB_DMS_EOC ;
   }

   void _dmsExtScannerBase::_checkMaxRecordsNum( _mthRecordGenerator &generator )
   {
      if ( _maxRecords > 0 )
      {
         if ( _maxRecords >= generator.getRecordNum() )
         {
            _maxRecords -= generator.getRecordNum() ;
         }
         else
         {
            INT32 num = generator.getRecordNum() - _maxRecords ;
            generator.popTail( num ) ;
            _maxRecords = 0 ;
         }
      }
   }

   INT32 _dmsExtScannerBase::advance( dmsRecordID &recordID,
                                      _mthRecordGenerator &generator,
                                      pmdEDUCB *cb,
                                      _mthMatchTreeContext *mthContext )
   {
      INT32 rc = SDB_OK ;

      if ( _firstRun )
      {
         rc = _firstInit( cb ) ;
         PD_RC_CHECK( rc, PDWARNING, "first init failed, rc: %d", rc ) ;
      }
      // have locked, but not trans, need to release record lock held 
      // from last round of scan
      else if ( _needUnLock && DMS_INVALID_OFFSET != _curRID._offset 
                && !_curRID.isIDXRid() )
      {
         _pTransCB->transLockRelease( cb, _pSu->logicalID(), _context->mbID(),
                                      &_curRID, _callback ) ;
      }

      rc = _fetchNext( recordID, generator, cb, mthContext ) ;
      if ( rc )
      {
         // Do not write error log when EOC.
         if ( SDB_DMS_EOC != rc )
         {
            PD_LOG( PDERROR, "Get next record failed, rc: %d", rc ) ;
         }
         goto error ;
      }

   done:
      return rc ;
   error:
      recordID.reset() ;
      _curRID._offset = DMS_INVALID_OFFSET ;
      goto done ;
   }

   void _dmsExtScannerBase::stop()
   {
      if ( FALSE == _firstRun && _recordLock != DPS_TRANSLOCK_MAX
           && DMS_INVALID_OFFSET != _curRID._offset )
      {
         _pTransCB->transLockRelease( _cb, _pSu->logicalID(), _context->mbID(),
                                      &_curRID, _callback ) ;
      }
      _next = DMS_INVALID_OFFSET ;
      _curRID._offset = DMS_INVALID_OFFSET ;
   }

   _dmsExtScanner::_dmsExtScanner( dmsStorageDataCommon *su, _dmsMBContext *context,
                                   mthMatchRuntime *matchRuntime,
                                   dmsExtentID curExtentID,
                                   DMS_ACCESS_TYPE accessType,
                                   INT64 maxRecords,
                                   INT64 skipNum,
                                   INT32 flag,
                                   _dpsITransLockCallback * callback )
   : _dmsExtScannerBase( su, context, matchRuntime, curExtentID, accessType,
                         maxRecords, skipNum, flag, callback )
   {
   }

   _dmsExtScanner::~_dmsExtScanner()
   {
   }

   // Description:
   //    DMS setup lock call back function. It create the call back if not
   // already exist. This function can be called by initial setup, or within
   // each round of dms scan to update the recor.
   // 
   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSCAN_SETUPTRANSLOCKCALLBACK, "_dmsScanner::setupTransLockCallback" )
   void _dmsScanner::setupTransLockCallback( dmsRecordRW* recordRW )
   {
      PD_TRACE_ENTRY ( SDB__DMSSCAN_SETUPTRANSLOCKCALLBACK );

      if ( _callback )
      {
         // callback already created, do cleanup and setup for this round
         _callback->setRecordRW(recordRW);
         _callback->resetWorkingArea();
      }
      PD_TRACE_EXIT ( SDB__DMSSCAN_SETUPTRANSLOCKCALLBACK );
   }

   // Description:
   //    DMS verify if certain index (LID) was changed by any updates based on
   // information kept in transCB callback oldVer
   // Dependency:
   //    Caller should guarantee the memory used here are safe
   BOOLEAN _dmsScanner::isIdxChanged( dmsExtentID    lid,
                                      const BSONObj* value )
   {
      BOOLEAN   changed = FALSE;
      oldVersionContainer *oldVer = NULL;
      if ( _callback )
      {
         oldVer = (oldVersionContainer*) _callback->getWorkingArea();

         if( oldVer->idxLidExist( lid ) )
         {
            BSONObj* oldValue = oldVer->getOldIdxValue( lid );
            if ( NULL != oldValue )
            {
               changed = !(oldValue->equal(*value));
            }
         }
      }
      return changed;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSEXTSCAN__FIRSTINIT, "_dmsExtScanner::_firstInit" )
   INT32 _dmsExtScanner::_firstInit( pmdEDUCB *cb )
   {
      INT32 rc          = SDB_OK ;
      _pTransCB         = pmdGetKRCB()->getTransCB() ;
      SDB_BPSCB *pBPSCB = pmdGetKRCB()->getBPSCB () ;
      BOOLEAN   bPreLoadEnabled = pBPSCB->isPreLoadEnabled() ;
      dpsTransExecutor *pExe = cb->getTransExecutor() ;

      PD_TRACE_ENTRY ( SDB__DMSEXTSCAN__FIRSTINIT );
      _transIsolation = pExe->getTransIsolation() ;
      _waitLock = pExe->isTransWaitLock() ;
      _useRollbackSegment = pExe->useRollbackSegment() ;

      /// When not use trans lock
      if ( !pExe->useTransLock() )
      {
         _recordLock = DPS_TRANSLOCK_MAX ;
         _selectForUpdate = FALSE ;
      }
      /// When not in transaction
      else if ( DPS_INVALID_TRANS_ID == cb->getTransID() )
      {
         /// Write operation should release lock right now
         if ( DMS_IS_WRITE_OPR( _accessType ) )
         {
            _recordLock = DPS_TRANSLOCK_X ;
            _needUnLock = TRUE ;
            _useRollbackSegment = FALSE ;
         }
         /// Read is always no lock
         else
         {
            _recordLock = DPS_TRANSLOCK_MAX ;
            _selectForUpdate = FALSE ;
         }

      }
      /// In transaction
      else
      {
         if ( DMS_IS_WRITE_OPR( _accessType ) )
         {
            _recordLock = DPS_TRANSLOCK_X ;
            _needUnLock = FALSE ;
         }
         else if ( TRANS_ISOLATION_RU == _transIsolation &&
                   !_selectForUpdate )
         {
            _recordLock = DPS_TRANSLOCK_MAX ;
            _selectForUpdate = FALSE ;
         }
         else
         {
            _recordLock = _selectForUpdate ? DPS_TRANSLOCK_U :
                                             DPS_TRANSLOCK_S ;
            if ( TRANS_ISOLATION_RS == _transIsolation ||
                 _selectForUpdate )
            {
               _needUnLock = FALSE ;
            }
            else
            {
               _needUnLock = TRUE ;
            }

            if ( _selectForUpdate )
            {
               _waitLock = TRUE ;
            }
         }
      }

      _extRW = _pSu->extent2RW( _curRID._extent, _context->mbID() ) ;
      _extRW.setNothrow( TRUE ) ;
      _extent = _extRW.readPtr<dmsExtent>() ;
      if ( NULL == _extent )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( cb->isInterrupted() )
      {
         rc = SDB_APP_INTERRUPT ;
         goto error ;
      }
      if ( !_context->isMBLock( _mbLockType ) )
      {
         rc = _context->mbLock( _mbLockType ) ;
         PD_RC_CHECK( rc, PDERROR, "dms mb lock failed, rc: %d", rc ) ;
      }
      if ( !dmsAccessAndFlagCompatiblity ( _context->mb()->_flag,
                                           _accessType ) )
      {
         PD_LOG ( PDERROR, "Incompatible collection mode: %d",
                  _context->mb()->_flag ) ;
         rc = SDB_DMS_INCOMPATIBLE_MODE ;
         goto error ;
      }
      if ( !_extent->validate( _context->mbID() ) )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      // send pre-load request
      if ( bPreLoadEnabled && DMS_INVALID_EXTENT != _extent->_nextExtent )
      {
         pBPSCB->sendPreLoadRequest ( bpsPreLoadReq( _pSu->CSID(),
                                                     _pSu->logicalID(),
                                                     _extent->_nextExtent )) ;
      }

      _cb   = cb ;
      _next = _extent->_firstRecordOffset ;

      // unset first run
      _firstRun = FALSE ;

   done:
      PD_TRACE_EXITRC ( SDB__DMSEXTSCAN__FIRSTINIT, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSEXTSCAN__FETCHNEXT, "_dmsExtScanner::_fetchNext" )
   INT32 _dmsExtScanner::_fetchNext( dmsRecordID &recordID,
                                     _mthRecordGenerator &generator,
                                     pmdEDUCB *cb,
                                     _mthMatchTreeContext *mthContext )
   {
      INT32 rc                = SDB_OK ;
      BOOLEAN result          = TRUE ;
      ossValuePtr recordDataPtr ;
      dmsRecordData recordData ;
      BOOLEAN lockedRecord    = FALSE ;
      BOOLEAN newXAcquire     = FALSE ;

      PD_TRACE_ENTRY ( SDB__DMSEXTSCAN__FETCHNEXT );
      PD_TRACE5 ( SDB__DMSEXTSCAN__FETCHNEXT, 
                 PD_PACK_UINT(_needUnLock),
                 PD_PACK_UINT(_mbLockType),
                 PD_PACK_UINT(_accessType),
                 PD_PACK_UINT(_waitLock),
                 PD_PACK_UINT(_recordLock) );

      // skip extent all
      if ( !_matchRuntime && _skipNum > 0 && _skipNum >= _extent->_recCount )
      {
         _skipNum -= _extent->_recCount ;
         _next = DMS_INVALID_OFFSET ;
      }

      while ( DMS_INVALID_OFFSET != _next && 0 != _maxRecords )
      {
         _curRID._offset = _next ;
         _recordRW = _pSu->record2RW( _curRID, _context->mbID() ) ;
         _curRecordPtr = _recordRW.readPtr( 0 ) ;
         _next = _curRecordPtr->getNextOffset() ;
         newXAcquire = FALSE ;

         if ( _recordLock != DPS_TRANSLOCK_MAX )
         {

            dmsTBTransContext tbTxContext( _context, _accessType ) ;
            dpsTransRetInfo   lockConflict ;

            // We should have already allocated the callback, this is to 
            // setup proper recordRW in callback 
            // FIXME: next step we will have caller create local variable
            // and passed an instance of callback

            setupTransLockCallback( &_recordRW );

            if ( DPS_TRANSLOCK_X == _recordLock )
            {
               rc = _pTransCB->transLockGetX( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              & tbTxContext, &lockConflict, _callback ) ;
               newXAcquire = lockConflict._newAcquire;
            }
            else if ( DPS_TRANSLOCK_U == _recordLock )
            {
               rc = _pTransCB->transLockGetU( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              &tbTxContext,
                                              &lockConflict ) ;
            }
            // We need to check TRANSWAITLOCK config to decide if
            // we do try or acquire S lock. 
            else if ( needWaitForLock() )
            {
               rc = _pTransCB->transLockGetS( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              & tbTxContext,
                                              &lockConflict ) ;
            }

            else
            {
               // for new RC logic, we should first try on S lock instead 
               // of directly wait on the record lock. Under the cover, 
               // the lock call back function would try to use the old copy
               // (previous committed version) if exist
               rc = _pTransCB->transLockTryS( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                               & lockConflict, _callback ) ;

               // If failed with SDB_DPS_TRANS_LOCK_INCOMPATIBLE,
               // that means the Xlock holder was from a Non-transaction update
               // OR in a small timing window of update guy in the middle of   
               // releasing lock. Either way, lock should be released soon.
               // Let's wait on the record lock.
               // If there was already another X waiting on the lock before 
               // us, unconditionally wait on S might wait for long time. 
               // Alternative solution is to use a while loop here, when failed
               // to get lock, we will just do tryS again. We either get the 
               // lock, or got old copy before come out. However, there could
               // be CPU overhead, and the lock try request could be starved
               // because we comeout instead of waiting in the queue.
               // For now, we take the simple approach, just wait on the lock
               if ( rc == SDB_DPS_TRANS_LOCK_INCOMPATIBLE && 
                    FALSE == lockConflict._useOldVersion )
               {
                  rc = _pTransCB->transLockGetS( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              & tbTxContext, & lockConflict ) ;
               }
               else if ( rc == SDB_CLT_OBJ_NOT_EXIST )
               {
                  // continue to next record if this is a newly created one
                  rc = SDB_OK;
                  continue;
               }

            }

            // In case we use old version, we might not have the lock held
            if ( ( rc != SDB_DPS_TRANS_LOCK_INCOMPATIBLE) || 
                 ( TRUE != lockConflict._useOldVersion )  )
            {
               PD_RC_CHECK( rc, PDERROR,
                         "Failed to get record lock, rc: %d"OSS_NEWLINE
                         "Request Mode:   %s"OSS_NEWLINE 
                         "Conflict ( representative ):"OSS_NEWLINE
                         "   EDUID:  %llu"OSS_NEWLINE
                         "   LockId: %s"OSS_NEWLINE
                         "   Mode:   %s"OSS_NEWLINE, 
                         rc,
                         lockModeToString( _recordLock ),
                         lockConflict._eduID,
                         lockConflict._lockID.toString().c_str(), 
                         lockModeToString( lockConflict._lockType ) ) ;

               lockedRecord = TRUE ;
            }

            // while we wait on the record lock, we could give up the mblatch,
            // another guy can come in and delete the next record (note that 
            // the current record we are working on should be fine). So we
            // need to refresh the next record(_next) based on the record
            // on disk. We might or might not have record lock, but we have
            // mbLatch, so it's a valid version.. 
            _next = _curRecordPtr->getNextOffset() ;

            // Since we might got an old version of the record from a buffer,
            // let's the _curRecordPtr. Otherwise we might mistakenly skip
            // the record if it's being deleted.
            _curRecordPtr = _recordRW.readPtr( 0 ) ;

         }

         // FIXME: if this delete is from the same transaction as marking it 
         // deleting, we would end up do the delete, but how to tell?
         // If the X lock was newly acquired/granted, we consider it as a 
         // new transaction, because the original one has committed and 
         // original lock was released.
         if ( _curRecordPtr->isDeleting() )
         {
            if ( _recordLock == DPS_TRANSLOCK_X )
               {
                  INT32 rc1 = _pSu->deleteRecord( _context, _curRID,
                                                  0, cb, NULL, 
                                                  newXAcquire, _callback ) ;
                  if ( rc1 )
                  {
                     PD_LOG( PDWARNING, "Failed to delete the deleting record, "
                             "rc: %d", rc ) ;
                  }
               }

            if ( lockedRecord )
            {
               _pTransCB->transLockRelease( cb, _pSu->logicalID(),
                                            _context->mbID(), &_curRID, _callback ) ;
               lockedRecord = FALSE ;
            }
            continue ;
         }
         SDB_ASSERT( !_curRecordPtr->isDeleted(), "record can't be deleted" ) ;

         if ( !_matchRuntime && _skipNum > 0 )
         {
            --_skipNum ;
         }
         else
         {
            
            recordID = _curRID ;
            rc = _pSu->extractData( _context, _recordRW, cb, recordData ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Extract record data failed, rc: %d", rc ) ;
               goto error ;
            }
            recordDataPtr = ( ossValuePtr )recordData.data() ;
            generator.setDataPtr( recordDataPtr ) ;

            // math
            if ( _matchRuntime && _matchRuntime->getMatchTree() )
            {
               result = TRUE ;
               try
               {
                  _mthMatchTree *matcher = _matchRuntime->getMatchTree() ;
                  rtnParamList *parameters = _matchRuntime->getParametersPointer() ;
                  BSONObj obj( recordData.data() ) ;
                  //do not clear dollarlist flag
                  mthContextClearRecordInfoSafe( mthContext ) ;
                  rc = matcher->matches( obj, result, mthContext, parameters ) ;
                  if ( rc )
                  {
                     PD_LOG( PDERROR, "Failed to match record, rc: %d", rc ) ;
                     goto error ;
                  }
                  if ( result )
                  {
                     rc = generator.resetValue( obj, mthContext ) ;
                     PD_RC_CHECK( rc, PDERROR, "resetValue failed:rc=%d", rc ) ;

                     if ( _skipNum > 0 )
                     {
                        if ( _skipNum >= generator.getRecordNum() )
                        {
                           _skipNum -= generator.getRecordNum() ;
                        }
                        else
                        {
                           generator.popFront( _skipNum ) ;
                           _skipNum = 0 ;
                           _checkMaxRecordsNum( generator ) ;

                           goto done ;
                        }
                     }
                     else
                     {
                        _checkMaxRecordsNum( generator ) ;
                        goto done ; // find ok
                     }
                  }
               }
               catch( std::exception &e )
               {
                  PD_LOG ( PDERROR, "Failed to create BSON object: %s",
                           e.what() ) ;
                  rc = SDB_SYS ;
                  goto error ;
               }
            } // if ( _match )
            else
            {
               try
               {
                  BSONObj obj( recordData.data() ) ;
                  rc = generator.resetValue( obj, mthContext ) ;
                  PD_RC_CHECK( rc, PDERROR, "resetValue failed:rc=%d", rc ) ;
               }
               catch( std::exception &e )
               {
                  rc = SDB_SYS ;
                  PD_RC_CHECK( rc, PDERROR, "Failed to create BSON object: %s",
                               e.what() ) ;
                  goto error ;
               }

               if ( _skipNum > 0 )
               {
                  --_skipNum ;
               }
               else
               {
                  if ( _maxRecords > 0 )
                  {
                     --_maxRecords ;
                  }
                  goto done ; // find ok
               }
            }
         }

         if ( lockedRecord == TRUE )
         {
            _pTransCB->transLockRelease( cb, _pSu->logicalID(),
                                         _context->mbID(), &_curRID, _callback ) ;
            lockedRecord = FALSE ;
         }
      } // while

      rc = SDB_DMS_EOC ;
      goto error ;

   done:
      PD_TRACE_EXITRC ( SDB__DMSEXTSCAN__FETCHNEXT, rc );
      return rc ;
   error:
      if ( lockedRecord && _recordLock != DPS_TRANSLOCK_MAX )
      {
         _pTransCB->transLockRelease( cb, _pSu->logicalID(), _context->mbID(),
                                      &_curRID, _callback ) ;
      }
      recordID.reset() ;
      recordDataPtr = 0 ;
      generator.setDataPtr( recordDataPtr ) ;
      _curRID._offset = DMS_INVALID_OFFSET ;
      goto done ;
   }

   _dmsCappedExtScanner::_dmsCappedExtScanner( dmsStorageDataCommon *su,
                                               dmsMBContext *context,
                                               mthMatchRuntime *matchRuntime,
                                               dmsExtentID curExtentID,
                                               DMS_ACCESS_TYPE accessType,
                                               INT64 maxRecords,
                                               INT64 skipNum,
                                               INT32 flag )
   : _dmsExtScannerBase( su, context, matchRuntime, curExtentID, accessType,
                         maxRecords, skipNum, flag )
   {
      _maxRecords = maxRecords ;
      _skipNum = skipNum ;
      _extent = NULL ;
      _curRID._extent = curExtentID ;
      _next = DMS_INVALID_OFFSET ;
      _lastOffset = DMS_INVALID_OFFSET ;
      _firstRun = TRUE ;
      _cb = NULL ;
      _workExtInfo = NULL ;
      _rangeInit = FALSE ;
      _fastScanByID = FALSE ;

      /// not support transaction
      _recordLock = DPS_TRANSLOCK_MAX ;
   }

   _dmsCappedExtScanner::~_dmsCappedExtScanner()
   {
   }

   INT32 _dmsCappedExtScanner::_firstInit( pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN inRange = FALSE ;
      _pTransCB = pmdGetKRCB()->getTransCB() ;

      _extRW = _pSu->extent2RW( _curRID._extent, _context->mbID() ) ;
      _extRW.setNothrow( TRUE ) ;
      _extent = _extRW.readPtr<dmsExtent>() ;
      if ( NULL == _extent )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = _validateRange( inRange ) ;
      PD_RC_CHECK( rc , PDERROR, "Failed to validate extant range, rc: %d",
                   rc ) ;

      if ( !inRange )
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }

      if ( !_context->isMBLock( _mbLockType ) )
      {
         rc = _context->mbLock( _mbLockType ) ;
         PD_RC_CHECK( rc, PDERROR, "dms mb lock failed, rc: %d", rc ) ;
      }

      _workExtInfo =
         (( _dmsStorageDataCapped * )_pSu)->getWorkExtInfo( _context->mbID() ) ;

      // Check if we are about to scan the working extent. As the extent is very
      // big, in order to improve performance, try to avoid getting the last
      // offset again and again from the extent head when in advance.
      if ( _curRID._extent == _workExtInfo->getID() )
      {
         _next = _workExtInfo->_firstRecordOffset ;
         _lastOffset = _workExtInfo->_lastRecordOffset ;
      }
      else
      {
         _next = _extent->_firstRecordOffset ;
         _lastOffset = _extent->_lastRecordOffset ;
      }

      if ( !_extent->validate( _context->mbID() ) )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      _cb = cb ;
      _firstRun = FALSE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsCappedExtScanner::_fetchNext( dmsRecordID &recordID,
                                           _mthRecordGenerator &generator,
                                           pmdEDUCB *cb,
                                           _mthMatchTreeContext *mthContext )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN result = TRUE ;
      ossValuePtr recordDataPtr ;
      dmsRecordData recordData ;
      const dmsCappedRecord *record = NULL ;
      UINT32 currExtRecNum = 0 ;

      currExtRecNum = ( _curRID._extent == _workExtInfo->getID() ) ?
                       _workExtInfo->_recCount : _extent->_recCount ;

      if ( !_matchRuntime && _skipNum > 0 && _skipNum >= currExtRecNum &&
           ( _curRID._extent != _context->mb()->_lastExtentID ) )
      {
         _skipNum -= currExtRecNum ;
         _next = DMS_INVALID_OFFSET ;
      }

      while ( _next <= _lastOffset && DMS_INVALID_OFFSET != _next &&
              0 != _maxRecords )
      {
         _curRID._offset = _next ;
         _recordRW = _pSu->record2RW( _curRID, _context->mbID() ) ;
         _recordRW.setNothrow( TRUE ) ;
         record= _recordRW.readPtr<dmsCappedRecord>( 0 ) ;
         if ( !record )
         {
            PD_LOG( PDERROR, "Get record failed" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         _next += record->getSize() ;

         if ( !_matchRuntime && _skipNum > 0 )
         {
            --_skipNum ;
         }
         else
         {
            recordID = _curRID ;
            rc = _pSu->extractData( _context, _recordRW, cb, recordData ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Extract record data failed, rc: %d", rc ) ;
               goto error ;
            }
            recordDataPtr = ( ossValuePtr )recordData.data() ;
            generator.setDataPtr( recordDataPtr ) ;

            if ( _matchRuntime && _matchRuntime->getMatchTree() )
            {
               result = TRUE ;
               try
               {
                  _mthMatchTree *matcher = _matchRuntime->getMatchTree() ;
                  rtnParamList *parameters = _matchRuntime->getParametersPointer() ;
                  BSONObj obj( recordData.data() ) ;
                  //do not clear dollarlist flag
                  mthContextClearRecordInfoSafe( mthContext ) ;
                  rc = matcher->matches( obj, result, mthContext, parameters ) ;
                  if ( rc )
                  {
                     PD_LOG( PDERROR, "Failed to match record, rc: %d", rc ) ;
                     goto error ;
                  }
                  if ( result )
                  {
                     rc = generator.resetValue( obj, mthContext ) ;
                     PD_RC_CHECK( rc, PDERROR, "resetValue failed:rc=%d", rc ) ;

                     if ( _skipNum > 0 )
                     {
                        if ( _skipNum >= generator.getRecordNum() )
                        {
                           _skipNum -= generator.getRecordNum() ;
                        }
                        else
                        {
                           generator.popFront( _skipNum ) ;
                           _skipNum = 0 ;
                           _checkMaxRecordsNum( generator ) ;

                           goto done ;
                        }
                     }
                     else
                     {
                        _checkMaxRecordsNum( generator ) ;
                        goto done ; // find ok
                     }
                  }
               }
               catch( std::exception &e )
               {
                  PD_LOG ( PDERROR, "Failed to create BSON object: %s",
                           e.what() ) ;
                  rc = SDB_SYS ;
                  goto error ;
               }
            } // if ( _match )
            else
            {
               try
               {
                  BSONObj obj( recordData.data() ) ;
                  rc = generator.resetValue( obj, mthContext ) ;
                  PD_RC_CHECK( rc, PDERROR, "resetValue failed, rc: %d", rc ) ;
               }
               catch( std::exception &e )
               {
                  rc = SDB_SYS ;
                  PD_RC_CHECK( rc, PDERROR, "Failed to create BSON object: %s",
                               e.what() ) ;
                  goto error ;
               }

               if ( _maxRecords > 0 )
               {
                  --_maxRecords ;
               }

               goto done ;
            }
         }
      }

      rc =  SDB_DMS_EOC ;
      goto error ;

   done:
      return rc ;
   error:
      goto done ;
   }

   OSS_INLINE dmsExtentID _dmsCappedExtScanner::_idToExtLID( INT64 id )
   {
      SDB_ASSERT( id >= 0, "id can't be negative" ) ;
      return ( id / DMS_CAP_EXTENT_BODY_SZ ) ;
   }

   // Filter out the extents that we really need to scan(in the range of the
   // condition).
   INT32 _dmsCappedExtScanner::_initFastScanRange()
   {
      INT32 rc = SDB_OK ;
      rtnPredicateSet predicateSet ;
      _mthMatchTree *matcher = _matchRuntime->getMatchTree() ;
      rtnParamList *parameters = _matchRuntime->getParametersPointer() ;

      rc = matcher->calcPredicate( predicateSet, parameters ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to calculate predicate, rc: %d",
                   rc ) ;

      {
         rtnPredicate predicate = predicateSet.predicate( DMS_ID_KEY_NAME ) ;
         if ( predicate.isGeneric() )
         {
            _fastScanByID = FALSE ;
         }
         else
         {
            _fastScanByID = TRUE ;
            // Add valid logical id range.
            BSONObj boStartKey = BSON( DMS_ID_KEY_NAME << 0 ) ;
            BSONObj boStopKey = BSON( DMS_ID_KEY_NAME << OSS_SINT64_MAX ) ;

            rc = predicateSet.addPredicate( DMS_ID_KEY_NAME,
                                            boStartKey.firstElement(),
                                            BSONObj::GTE, FALSE, FALSE, FALSE,
                                            -1, -1 ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to add predicate, rc: %d", rc ) ;
            rc = predicateSet.addPredicate( DMS_ID_KEY_NAME,
                                            boStopKey.firstElement(),
                                            BSONObj::LTE, FALSE, FALSE, FALSE,
                                            -1, -1 ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to add predicate, rc: %d", rc ) ;

            predicate = predicateSet.predicate( DMS_ID_KEY_NAME ) ;

            for ( RTN_SSKEY_LIST::iterator itr = predicate._startStopKeys.begin();
                  itr != predicate._startStopKeys.end(); ++itr )
            {
               dmsExtentID startExtLID = DMS_INVALID_EXTENT ;
               dmsExtentID endExtLID = DMS_INVALID_EXTENT ;
               const rtnKeyBoundary &startKey = itr->_startKey ;
               const rtnKeyBoundary &stopKey = itr->_stopKey ;

               startExtLID = _idToExtLID( startKey._bound.numberLong() ) ;
               endExtLID = _idToExtLID( stopKey._bound.numberLong() ) ;

               _rangeSet.insert( std::pair<dmsExtentID, dmsExtentID>( startExtLID,
                                                                      endExtLID ) ) ;
            }
         }
      }
      _rangeInit = TRUE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // As there is no index on capped table, query with condition will be slow as
   // we always need to do full table scan. But if _id is specified in the
   // condition, we can first filter out the extents which are in the range of
   // the condition. This will lead to much better performance.
   INT32 _dmsCappedExtScanner::_validateRange( BOOLEAN &inRange )
   {
      INT32 rc = SDB_OK ;

      // If there is no condition, or no _id in the condition, the validation
      // result should always be TRUE.
      inRange = TRUE ;
      if ( _matchRuntime )
      {
         if ( !_rangeInit )
         {
            rc = _initFastScanRange() ;
            PD_RC_CHECK( rc, PDERROR, "Failed to initial fast scan range, "
                         "rc: %d", rc ) ;
         }

         if ( _fastScanByID )
         {
            // Traverse the range set to check if the current extent is in any
            // valid range.
            for ( EXT_RANGE_SET_ITR itr = _rangeSet.begin();
                  itr != _rangeSet.end(); ++itr )
            {
               if ( _extent->_logicID >= itr->first &&
                    _extent->_logicID <= itr->second )
               {
                  inRange = TRUE ;
                  break ;
               }
            }
         }
         else
         {
            inRange = TRUE ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   dmsExtentID _dmsCappedExtScanner::nextExtentID () const
   {
      if ( _extent )
      {
         return _extent->_nextExtent ;
      }
      return DMS_INVALID_EXTENT ;
   }

   /*
      _dmsTBScanner implement
   */
   _dmsTBScanner::_dmsTBScanner( dmsStorageDataCommon *su,
                                 dmsMBContext *context,
                                 mthMatchRuntime *matchRuntime,
                                 DMS_ACCESS_TYPE accessType,
                                 INT64 maxRecords,
                                 INT64 skipNum,
                                 INT32 flag, 
                                 _dpsITransLockCallback * callback )
   :_dmsScanner( su, context, matchRuntime, accessType, callback )
   {
      _extScanner    = NULL ;
      _curExtentID   = DMS_INVALID_EXTENT ;
      _firstRun      = TRUE ;
      _maxRecords    = maxRecords ;
      _skipNum       = skipNum ;
      _flag          = flag ;
   }

   _dmsTBScanner::~_dmsTBScanner()
   {
      _curExtentID   = DMS_INVALID_EXTENT ;
      if ( _extScanner )
      {
         SDB_OSS_DEL _extScanner ;
      }
   }

   INT32 _dmsTBScanner::_firstInit()
   {
      INT32 rc = SDB_OK ;

      rc = _getExtScanner() ;
      PD_RC_CHECK( rc, PDERROR, "Get extent scanner failed, rc: %d", rc ) ;

      if ( !_context->isMBLock( _extScanner->_mbLockType ) )
      {
         rc = _context->mbLock( _extScanner->_mbLockType ) ;
         PD_RC_CHECK( rc, PDERROR, "dms mb lock failed, rc: %d", rc ) ;
      }
      _curExtentID = _context->mb()->_firstExtentID ;
      _resetExtScanner() ;
      _firstRun = FALSE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _dmsTBScanner::_resetExtScanner()
   {
      _extScanner->_firstRun = TRUE ;
      _extScanner->_curRID._extent = _curExtentID ;
   }

   INT32 _dmsTBScanner::_getExtScanner()
   {
      INT32 rc = SDB_OK ;
      _extScanner = dmsGetScannerFactory()->create( _pSu, _context,
                                                    _matchRuntime,
                                                    _curExtentID,
                                                    _accessType,
                                                    _maxRecords,
                                                    _skipNum,
                                                    _flag, 
                                                    _callback ) ;
      if ( !_extScanner )
      {
         PD_LOG( PDERROR, "Create extent scanner failed" ) ;
         rc = SDB_OOM ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsTBScanner::advance( dmsRecordID &recordID,
                                 _mthRecordGenerator &generator,
                                 pmdEDUCB *cb,
                                 _mthMatchTreeContext *mthContext )
   {
      INT32 rc = SDB_OK ;
      if ( _firstRun )
      {
         rc = _firstInit() ;
         PD_RC_CHECK( rc, PDERROR, "First init failed, rc: %d", rc ) ;
      }

      while ( DMS_INVALID_EXTENT != _curExtentID )
      {
         rc = _extScanner->advance( recordID, generator, cb, mthContext ) ;
         if ( SDB_DMS_EOC == rc )
         {
            if ( 0 != _extScanner->getMaxRecords() )
            {
               _curExtentID = _extScanner->nextExtentID() ;
               _resetExtScanner() ;
               _context->pause() ;
               continue ;
            }
            else
            {
               _curExtentID = DMS_INVALID_EXTENT ;
               goto error ;
            }
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Extent scanner failed, rc: %d", rc ) ;
            goto error ;
         }
         else
         {
            goto done ;
         }
      }
      rc = SDB_DMS_EOC ;
      goto error ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _dmsTBScanner::stop()
   {
      _extScanner->stop() ;
      _curExtentID = DMS_INVALID_EXTENT ;
   }

   /*
      _dmsIXSecScanner implement
   */
   _dmsIXSecScanner::_dmsIXSecScanner( dmsStorageDataCommon *su,
                                       dmsMBContext *context,
                                       mthMatchRuntime *matchRuntime,
                                       rtnIXScanner *scanner,
                                       DMS_ACCESS_TYPE accessType,
                                       INT64 maxRecords,
                                       INT64 skipNum,
                                       INT32 flag, 
                                       _dpsITransLockCallback * callback )
   :_dmsScanner( su, context, matchRuntime, accessType, callback ),
    _curRecordPtr( NULL )
   {
      _maxRecords          = maxRecords ;
      _skipNum             = skipNum ;
      _firstRun            = TRUE ;
      _pTransCB            = NULL ;
      _recordLock          = DPS_TRANSLOCK_MAX ;
      _needUnLock          = FALSE ;
      _selectForUpdate     = FALSE ;
      _cb                  = NULL ;
      _scanner             = scanner ;
      _onceRestNum         = 0 ;
      _eof                 = FALSE ;
      _indexBlockScan      = FALSE ;
      _judgeStartKey       = FALSE ;
      _includeStartKey     = FALSE ;
      _includeEndKey       = FALSE ;
      _blockScanDir        = 1 ;
      _countOnly           = FALSE ;
      //_transLockwait       = TRUE ;  // default block on record lock

      if ( OSS_BIT_TEST( flag, FLG_QUERY_FOR_UPDATE ) )
      {
         _selectForUpdate = TRUE ;
      }
   }

   _dmsIXSecScanner::~_dmsIXSecScanner ()
   {
      if ( FALSE == _firstRun && _recordLock != DPS_TRANSLOCK_MAX
           && DMS_INVALID_OFFSET != _curRID._offset )
      {
         _pTransCB->transLockRelease( _cb, _pSu->logicalID(), _context->mbID(),
                                      &_curRID, _callback ) ;
      }

      _scanner    = NULL ;
   }

   void  _dmsIXSecScanner::enableIndexBlockScan( const BSONObj &startKey,
                                                 const BSONObj &endKey,
                                                 const dmsRecordID &startRID,
                                                 const dmsRecordID &endRID,
                                                 INT32 direction )
   {
      SDB_ASSERT( _scanner, "Scanner can't be NULL" ) ;

      _blockScanDir     = direction ;
      _indexBlockScan   = TRUE ;
      _judgeStartKey    = FALSE ;
      _includeStartKey  = FALSE ;
      _includeEndKey    = FALSE ;

      _startKey = startKey.getOwned() ;
      _endKey = endKey.getOwned() ;
      _startRID = startRID ;
      _endRID = endRID ;

      BSONObj startObj = _scanner->getPredicateList()->startKey() ;
      BSONObj endObj = _scanner->getPredicateList()->endKey() ;

      if ( 0 == startObj.woCompare( *_getStartKey(), BSONObj(), false ) &&
           _getStartRID()->isNull() )
      {
         _includeStartKey = TRUE ;
      }
      if ( 0 == endObj.woCompare( *_getEndKey(), BSONObj(), false ) &&
           _getEndRID()->isNull() )
      {
         _includeEndKey = TRUE ;
      }

      // reset start/end RID
      if ( _getStartRID()->isNull() )
      {
         if ( 1 == _scanner->getDirection() )
         {
            _getStartRID()->resetMin () ;
         }
         else
         {
            _getStartRID()->resetMax () ;
         }
      }
      if ( _getEndRID()->isNull() )
      {
         if ( !_includeEndKey )
         {
            if ( 1 == _scanner->getDirection() )
            {
               _getEndRID()->resetMin () ;
            }
            else
            {
               _getEndRID()->resetMax () ;
            }
         }
         else
         {
            if ( 1 == _scanner->getDirection() )
            {
               _getEndRID()->resetMax () ;
            }
            else
            {
               _getEndRID()->resetMin () ;
            }
         }
      }
   }

   INT32 _dmsIXSecScanner::_firstInit( pmdEDUCB * cb )
   {
      INT32 rc          = SDB_OK ;
      _pTransCB         = pmdGetKRCB()->getTransCB() ;
      dpsTransExecutor *pExe = cb->getTransExecutor() ;

      _transIsolation = pExe->getTransIsolation() ;
      _waitLock = pExe->isTransWaitLock() ;
      _useRollbackSegment = pExe->useRollbackSegment() ;

      /// When not use trans lock
      if ( !pExe->useTransLock() )
      {
         _recordLock = DPS_TRANSLOCK_MAX ;
         _selectForUpdate = FALSE ;
      }
      /// When not in transaction
      else if ( DPS_INVALID_TRANS_ID == cb->getTransID() )
      {
         /// Write operation should release lock right way
         if ( DMS_IS_WRITE_OPR( _accessType ) )
         {
            _recordLock = DPS_TRANSLOCK_X ;
            _needUnLock = TRUE ;
            _useRollbackSegment = FALSE ;  // don't use old copy
         }
         /// Read is always no lock
         else
         {
            _recordLock = DPS_TRANSLOCK_MAX ;
            _selectForUpdate = FALSE ;
         }
      }
      /// In transaction
      else
      {
         if ( DMS_IS_WRITE_OPR( _accessType ) )
         {
            _recordLock = DPS_TRANSLOCK_X ;
            _needUnLock = FALSE ;
         }
         else if ( TRANS_ISOLATION_RU == _transIsolation &&
                   !_selectForUpdate )
         {
            _recordLock = DPS_TRANSLOCK_MAX ;
            _selectForUpdate = FALSE ;
         }
         else
         {
            _recordLock = _selectForUpdate ? DPS_TRANSLOCK_U :
                                             DPS_TRANSLOCK_S ;
            if ( TRANS_ISOLATION_RS == _transIsolation ||
                 _selectForUpdate )
            {
               _needUnLock = FALSE ;
            }
            else
            {
               _needUnLock = TRUE ;
            }

            if ( _selectForUpdate )
            {
               _waitLock = TRUE ;
            }
         }
      }

      if ( NULL == _scanner )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( cb && cb->isInterrupted() )
      {
         rc = SDB_APP_INTERRUPT ;
         goto error ;
      }
      if ( !_context->isMBLock( _mbLockType ) )
      {
         rc = _context->mbLock( _mbLockType ) ;
         PD_RC_CHECK( rc, PDERROR, "dms mb lock failed, rc: %d", rc ) ;
      }
      if ( !dmsAccessAndFlagCompatiblity ( _context->mb()->_flag,
                                           _accessType ) )
      {
         PD_LOG ( PDERROR, "Incompatible collection mode: %d",
                  _context->mb()->_flag ) ;
         rc = SDB_DMS_INCOMPATIBLE_MODE ;
         goto error ;
      }

      rc = _scanner->resumeScan( ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to resum ixscan, rc: %d", rc ) ;

      _cb   = cb ;

      // unset first run
      _firstRun = FALSE ;
      _onceRestNum = (INT64)pmdGetKRCB()->getOptionCB()->indexScanStep() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   BSONObj* _dmsIXSecScanner::_getStartKey ()
   {
      return _scanner->getDirection() == _blockScanDir ? &_startKey : &_endKey ;
   }

   BSONObj* _dmsIXSecScanner::_getEndKey ()
   {
      return _scanner->getDirection() == _blockScanDir ? &_endKey : &_startKey ;
   }

   dmsRecordID* _dmsIXSecScanner::_getStartRID ()
   {
      return _scanner->getDirection() == _blockScanDir ? &_startRID : &_endRID ;
   }

   dmsRecordID* _dmsIXSecScanner::_getEndRID ()
   {
      return _scanner->getDirection() == _blockScanDir ? &_endRID : &_startRID ;
   }

   void _dmsIXSecScanner::_updateMaxRecordsNum( _mthRecordGenerator &generator )
   {
      if ( _maxRecords > 0 )
      {
         if ( _maxRecords >= generator.getRecordNum() )
         {
            _maxRecords -= generator.getRecordNum() ;
         }
         else
         {
            INT32 num = generator.getRecordNum() - _maxRecords ;
            generator.popTail( num ) ;
            _maxRecords = 0 ;
         }
      }
   }

   // Description
   //    Index section scan. Advance to next index entry based on the on-disk
   // index tree and searching criteria, return the found recordID pointed
   // by the index
   // Input
   //
   // Ouput
   //    recordID: 
   //    On normal return, record lock is held on the matching record.
   // Return
   // 
   //
   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSIXSECSCAN_ADVANCE, "_dmsIXSecScanner::advance" )
   INT32 _dmsIXSecScanner::advance( dmsRecordID &recordID,
                                    _mthRecordGenerator &generator,
                                    pmdEDUCB * cb,
                                    _mthMatchTreeContext *mthContext )
   {
      INT32 rc                = SDB_OK ;
      BOOLEAN result          = TRUE ;
      ossValuePtr recordDataPtr ;
      dmsRecordData recordData ;
      BOOLEAN lockedRecord    = FALSE ;
      BOOLEAN verifyRecord    = FALSE ;
      BOOLEAN newXAcquire     = FALSE ;

      PD_TRACE_ENTRY ( SDB__DMSIXSECSCAN_ADVANCE );

      PD_TRACE5( SDB__DMSIXSECSCAN_ADVANCE, 
                 PD_PACK_UINT(_needUnLock),
                 PD_PACK_UINT(_mbLockType),
                 PD_PACK_UINT(_accessType),
                 PD_PACK_UINT(_waitLock),
                 PD_PACK_UINT(_recordLock) );    

      if ( _firstRun )
      {
         // first initialize the merge scanner if needed
         if ( SCANNER_TYPE_MERGE == _scanner->type() )
         {
            // Let's try to set up the in-memory tree for the mem scan
            // it returns FALSE when the in memory tree does not exist 
            // which means the index was never changed. 
            // However the tree can show up in the middel of the scan
            // after the pauseScan. So resumeScan should try to do the 
            // same initialization(FIXME, we should likely do it in 
            // rtnMergeIXScan() )
            rtnMergeIXScanner *mergeScanner = (rtnMergeIXScanner*) _scanner;
            if( !( (rtnMemIXTreeScanner *)mergeScanner->left())->initMemIXScan(
                                        pmdGetKRCB()->getTransCB(), 
                                        _pSu->logicalID(),
                                        _context->mbID(), 
                                        mergeScanner->getSharedInfo() ) )
            {
               PD_LOG ( PDDEBUG, "rtnMemIXTreeScanner not initialized" ) ;
            }                                           
         }

         rc = _firstInit( cb ) ;
         PD_RC_CHECK( rc, PDWARNING, "first init failed, rc: %d", rc ) ;


         // set up isReadOnly flag in scanner
         _scanner->setIsReadOnly ( isReadOnly() );
      }
      // last run have record lock held, but not trans, need to release
      // record lock
      else if ( _needUnLock && DMS_INVALID_OFFSET != _curRID._offset )
         //       && !_curRID.isIDXRid() )    // FIXME ??need this
      {
         _pTransCB->transLockRelease( cb, _pSu->logicalID(), _context->mbID(),
                                      &_curRID, _callback ) ;
      }

      while ( _onceRestNum-- > 0 && 0 != _maxRecords )
      {
         // advance index tree 
         rc = _scanner->advance( _curRID ) ;
         if ( SDB_IXM_EOC == rc )
         {
            _eof = TRUE ;
            rc = SDB_DMS_EOC ;
            break ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "IXScanner advance failed, rc: %d", rc ) ;
            goto error ;
         }
         SDB_ASSERT( _curRID.isValid(), "rid msut valid" ) ;

         // index block scan
         if ( _indexBlockScan )
         {
            INT32 result = 0 ;
            if ( !_judgeStartKey )
            {
               _judgeStartKey = TRUE ;

               result = _scanner->compareWithCurKeyObj( *_getStartKey() ) ;
               if ( 0 == result )
               {
                  result = _curRID.compare( *_getStartRID() ) *
                           _scanner->getDirection() ;
               }
               if ( result < 0 )
               {
                  // need to relocate
                  rc = _scanner->relocateRID( *_getStartKey(),
                                              *_getStartRID() ) ;
                  PD_RC_CHECK( rc, PDERROR, "Failed to relocateRID, rc: %d",
                               rc ) ;
                  continue ;
               }
            }

            result = _scanner->compareWithCurKeyObj( *_getEndKey() ) ;
            if ( 0 == result )
            {
               result = _curRID.compare( *_getEndRID() ) *
                        _scanner->getDirection() ;
            }
            if ( result > 0 || ( !_includeEndKey && result == 0 ) )
            {
               _eof = TRUE ;
               rc = SDB_DMS_EOC ;
               break ;
            }
         }

         // don't read data
         if ( !_matchRuntime )
         {
            if ( _skipNum > 0 )
            {
               --_skipNum ;
               continue ;
            }
            else if ( _countOnly )
            {
               if ( _maxRecords > 0 )
               {
                  --_maxRecords ;
               }
               recordID = _curRID ;
               recordDataPtr = 0 ;
               generator.setDataPtr( recordDataPtr ) ;
               goto done ;
            }
         }

         // record2RW already take care of in memory version vs on disk 
         // version under the cover
         _recordRW = _pSu->record2RW( _curRID, _context->mbID() ) ;

         newXAcquire = FALSE ;

         // If need lock and the RID was not setup from the in memory tree.
         // As an optimization, if the RID was originally found through in 
         // memory tree scan, don't try lock at all.
         if ( _recordLock != DPS_TRANSLOCK_MAX )
         {
            dpsTransRetInfo   lockConflict ;
            dmsIXTransContext ixTxContext( _context, _accessType,
                                           _scanner, isReadOnly() ) ;
            dpsTransLRBHeader * lrbHdr;

            // It's possible that the index scan found old verion from 
            // in memory tree, but we should not blindly use that version
            // because a scan can come from the same transaction as an earlier
            // update, so we should allow the scan to use the latest version
            // on disk if we can get the S lock throuh tryS
            if ( _curRID.isIDXRid() )
            {
               // In index scan, we hold the in memory tree latch
               // which will prevent commit to clean up. so we can safely
               // update record2RW ptr to in memory old version. 
               // BTW, index in the mem-tree should be sync with the old 
               // version record, so no need to re-verify record as above.
               lrbHdr = _pTransCB->getLockMgrHandle()
                                 ->getLRBHdrPtrByIdx(_curRID._offset);

               _curRID._extent = lrbHdr->lockId.extentID() ;
               _curRID._offset = lrbHdr->lockId.offset() ; 
               _recordRW = _pSu->record2RW( _curRID, _context->mbID() ) ;

            }

            // We should have already allocated the callback, this is to
            // setup proper recordRW in callback
            setupTransLockCallback( &_recordRW );

            if ( DPS_TRANSLOCK_X == _recordLock )
            {
               // exclusive lock has to always wait on the lock
               rc = _pTransCB->transLockGetX( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              &ixTxContext, 
                                              &lockConflict, _callback ) ;
               newXAcquire = lockConflict._newAcquire;
            }
            else if ( DPS_TRANSLOCK_U == _recordLock )
            {
               rc = _pTransCB->transLockGetU( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              &ixTxContext,
                                              & lockConflict ) ;
            }
            else if ( needWaitForLock() )
            {
               // no need to do callback if we decide to wait on the S lock
               rc = _pTransCB->transLockGetS( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              &ixTxContext, &lockConflict ) ;
            }
            else
            {
               rc = _pTransCB->transLockTryS( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              & lockConflict, _callback );

               if ( rc == SDB_DPS_TRANS_LOCK_INCOMPATIBLE )
               {
                  if(  FALSE == lockConflict._useOldVersion )
                  {
                     // can't get lock and didn't use old copy. wait back on 
                     // the lock. Hopefully we will get it quickly
                     rc = _pTransCB->transLockGetS( cb, _pSu->logicalID(),
                                              _context->mbID(), &_curRID,
                                              & ixTxContext, & lockConflict ) ;
                  }
                  else
                  {
                     // When we use old version, we wouldn't have lock. 
                     // There is a chance that the index on disk might not
                     // match the old record if the change affected the index.
                     // In that case, we need to re-verify the record with the
                     // index again. Here is how this could happen:
                     // Session 1 did update, changed index from 1 to 2, paused;
                     // session 2 does index scan, searching for record with 
                     // index 2. It found the index on disk. Did get record lock
                     // and ended up using the old version from memory. But 
                     // the old version record contain the index 1. We must verify
                     // and skip this record. 
                     verifyRecord = TRUE;

                     // Note that in this case, recordRW is already setup by the 
                     // callback function. However, there is a catch here: we don't
                     // have lock, and once we return the record to caller, caller
                     // did not know. It might blindly try to release lock once 
                     // finished scan. But the worst case is to print warning msg
                     // in diaglog. We may want to figure out ways to improve this, 
                     // for example, set the rid to isIDXRid again, and skip lock
                     // release in the case. For now, we don't bother.
                     rc = SDB_OK;
                  }
               }
               // For newly created records by another transaction,
               // we could still find it through diskIXScan, we will 
               // skip those records without waiting for lock. 
               else if ( rc == SDB_CLT_OBJ_NOT_EXIST )
               {
                  rc = SDB_OK;
                  continue;
               }
            }

            PD_RC_CHECK( rc, PDERROR,
                      "Failed to get record lock, rc: %d"OSS_NEWLINE
                      " Request mode: %s"OSS_NEWLINE
                      "Conflict ( representative ):"OSS_NEWLINE
                      "   EDUID:  %llu"OSS_NEWLINE
                      "   LockId: %s"OSS_NEWLINE
                      "   Mode:   %s"OSS_NEWLINE,
                      rc,
                      lockModeToString( _recordLock ),
                      lockConflict._eduID,
                      lockConflict._lockID.toString().c_str(),
                      lockModeToString( lockConflict._lockType ) ) ;

            // index has changed under us between wait on lock and got lock
            if ( !ixTxContext.isCursorValid() )
            {
               _pTransCB->transLockRelease( cb, _pSu->logicalID(),
                                            _context->mbID(), &_curRID, 
                                            _callback ) ;

               /// remove the duplicate key
               _scanner->removeDuplicatRID( _curRID ) ;
#ifdef _DEBUG
               PD_LOG( PDDEBUG, "Cursor changed while waiting for lock,"
                                " rid(%d, %d)", 
                                _curRID._extent, _curRID._offset ) ;
#endif
               continue;
            }
             
            // It's not accurate as we might not hold lock if we use old 
            // copy. However, there is no way the caller know at all. The
            // worst case is to have several ERROR message in diag when we
            // try to release none exist lock.
            lockedRecord = TRUE ;

         } // end of (_recordLock != -1)

         // Move _curRecordPtr to here so that _recordRW is fully setup for 
         // all cases.
         _curRecordPtr = _recordRW.readPtr( 0 ) ;

         // Handle the record being deleted 
         if ( _curRecordPtr->isDeleting() )
         {
            if ( _recordLock == DPS_TRANSLOCK_X )

            {
                  rc = _scanner->pauseScan( ) ;
                  PD_RC_CHECK( rc, PDERROR, "Failed to pause ixscan, rc: %d", rc ) ;
                  rc = _pSu->deleteRecord( _context, _curRID, 0, 
                                           cb, NULL, newXAcquire, _callback ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDWARNING, "Failed to delete the deleting record, "
                             "rc: %d", rc ) ;
                  }
           
                  rc = _scanner->resumeScan( ) ;
                  PD_RC_CHECK( rc, PDERROR, 
                               "Failed to resume ixscan, rc: %d", rc ) ;

            }

            if ( lockedRecord )
            {
               _pTransCB->transLockRelease( cb, _pSu->logicalID(),
                                            _context->mbID(), &_curRID, 
                                            _callback ) ;
               lockedRecord = FALSE ;
            }
            continue ;
         }
         SDB_ASSERT( !_curRecordPtr->isDeleted(), 
                     "record can't be deleted" ) ;

         if (verifyRecord)
         {
            // 1. verify if the index LID is among the set of index changed
            // we can only get here when old version was used, and we do
            // hold the mem tree latch. So the memory(oldVer) won't be 
            // released under us.
            if ( isIdxChanged( _scanner->getIdxLID(), 
                               _scanner->getCurKeyObj() ) )
            {
               // 2. retrieve the affected index from the old record
               // 3. skip the record if the retrieved index does NOT match
               // the current index value (in scanner). Note that simply use
               // the current predicate wouldn't help because the original and
               // new key might land in the same range. 
               // current index key is stored in _scanner.getCurKeyObj()
               continue;
            }
         }

         recordID = _curRID ;
         rc = _pSu->extractData( _context, _recordRW, cb, recordData ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Extract record data failed, rc: %d", rc ) ;
            goto error ;
         }
         recordDataPtr = ( ossValuePtr )recordData.data() ;
         generator.setDataPtr( recordDataPtr ) ;

         // match
         if ( _matchRuntime && _matchRuntime->getMatchTree() )
         {
            result = TRUE ;
            try
            {
               _mthMatchTree *matcher = _matchRuntime->getMatchTree() ;
               rtnParamList *parameters = _matchRuntime->getParametersPointer() ;
               BSONObj obj ( recordData.data() ) ;
               //do not clear dollarlist flag
               mthContextClearRecordInfoSafe( mthContext ) ;
               rc = matcher->matches( obj, result, mthContext, parameters ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Failed to match record, rc: %d", rc ) ;
                  goto error ;
               }
               if ( result )
               {
                  rc = generator.resetValue( obj, mthContext ) ;
                  PD_RC_CHECK( rc, PDERROR, "resetValue failed:rc=%d", rc ) ;
                  if ( _skipNum > 0 )
                  {
                     if ( _skipNum >= generator.getRecordNum() )
                     {
                        _skipNum -= generator.getRecordNum() ;
                     }
                     else
                     {
                        generator.popFront( _skipNum ) ;
                        _skipNum = 0 ;
                        _updateMaxRecordsNum( generator ) ;
                        goto done ;
                     }
                  }
                  else
                  {
                     _updateMaxRecordsNum( generator ) ;
                     goto done ; // find ok
                  }
               }
            }
            catch( std::exception &e )
            {
               PD_LOG ( PDERROR, "Failed to create BSON object: %s",
                        e.what() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
         } // if ( _match )
         else
         {
            try
            {
               BSONObj obj( recordData.data() ) ;
               rc = generator.resetValue( obj, mthContext ) ;
               PD_RC_CHECK( rc, PDERROR, "resetValue failed:rc=%d", rc ) ;
            }
            catch( std::exception &e )
            {
               rc = SDB_SYS ;
               PD_RC_CHECK( rc, PDERROR, "Failed to create BSON object: %s",
                            e.what() ) ;
               goto error ;
            }

            if ( _skipNum > 0 )
            {
               --_skipNum ;
            }
            else
            {
               if ( _maxRecords > 0 )
               {
                  --_maxRecords ;
               }
               goto done ; // find ok
            }
         }


         // Proper found case will jump to done, if we got here, there was
         // either unmatch, or we need to skip. Either way, we should 
         // release the record lock before advance to next index
         if ( lockedRecord )
         {
            _pTransCB->transLockRelease( cb, _pSu->logicalID(),
                                         _context->mbID(), &_curRID, 
                                         _callback ) ;
            lockedRecord = FALSE ;
         }
      } // while

      rc = SDB_DMS_EOC ;
      {
         INT32 rcTmp = _scanner->pauseScan(  ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Pause scan failed, rc: %d", rcTmp ) ;
            rc = rcTmp ;
         }
      }
      goto error ;

   done:
      if ( SDB_OK == rc && ( _onceRestNum <= 0 || 0 == _maxRecords ) )
      {
         rc = _scanner->pauseScan( ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Pause scan failed, rc: %d", rc ) ;
         }
      }
      //FIXME: to remove
      PD_LOG( PDDEBUG, "IXSecScan.advance return,rc=%d, rid(%d, %d)", 
                       rc, _curRID._extent, _curRID._offset ) ;

      PD_TRACE_EXITRC ( SDB__DMSIXSECSCAN_ADVANCE, rc );
      return rc ;
   error:
      if ( lockedRecord && _recordLock != DPS_TRANSLOCK_MAX )
      {
         _pTransCB->transLockRelease( cb, _pSu->logicalID(), _context->mbID(),
                                      &_curRID, _callback ) ;
      }
      recordID.reset() ;
      recordDataPtr = 0 ;
      generator.setDataPtr( recordDataPtr ) ;
      _curRID._offset = DMS_INVALID_OFFSET ;
      goto done ;
   }

   void _dmsIXSecScanner::stop ()
   {
      if ( FALSE == _firstRun && _recordLock != DPS_TRANSLOCK_MAX
           && DMS_INVALID_OFFSET != _curRID._offset )
      {
         _pTransCB->transLockRelease( _cb, _pSu->logicalID(), _context->mbID(),
                                      &_curRID, _callback ) ;
      }
      if ( DMS_INVALID_OFFSET != _curRID._offset )
      {
         INT32 rc = _scanner->pauseScan(  ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Pause scan failed, rc: %d", rc ) ;
         }
      }
      _curRID._offset = DMS_INVALID_OFFSET ;
   }

   /*
      _dmsIXScanner implement
   */
   _dmsIXScanner::_dmsIXScanner( dmsStorageDataCommon *su,
                                 dmsMBContext *context,
                                 mthMatchRuntime *matchRuntime,
                                 rtnIXScanner *scanner,
                                 BOOLEAN ownedScanner,
                                 DMS_ACCESS_TYPE accessType,
                                 INT64 maxRecords,
                                 INT64 skipNum,
                                 INT32 flag,
                                 _dpsITransLockCallback * callback )
   :_dmsScanner( su, context, matchRuntime, accessType, callback ),
    _secScanner( su, context, matchRuntime, scanner, accessType, maxRecords,
                 skipNum, flag, callback )
   {
      _scanner       = scanner ;
      _eof           = FALSE ;
      _ownedScanner  = ownedScanner ;
   }

   _dmsIXScanner::~_dmsIXScanner()
   {
      if ( _scanner && _ownedScanner )
      {
         SDB_OSS_DEL _scanner ;
      }
      _scanner       = NULL ;
   }

   void _dmsIXScanner::_resetIXSecScanner ()
   {
      _secScanner._firstRun = TRUE ;
   }

   INT32 _dmsIXScanner::advance( dmsRecordID &recordID,
                                 _mthRecordGenerator &generator,
                                 pmdEDUCB * cb,
                                 _mthMatchTreeContext *mthContext )
   {
      INT32 rc = SDB_OK ;

      while ( !_eof )
      {
         rc = _secScanner.advance( recordID, generator, cb, mthContext ) ;
         if ( SDB_DMS_EOC == rc )
         {
            if ( 0 != _secScanner.getMaxRecords() &&
                 !_secScanner.eof() )
            {
               _resetIXSecScanner() ;
               _context->pause() ;
               continue ;
            }
            else
            {
               _eof = TRUE ;
               goto error ;
            }
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "IX scanner failed, rc: %d", rc ) ;
            goto error ;
         }
         else
         {
            goto done ;
         }
      }
      rc = SDB_DMS_EOC ;
      goto error ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _dmsIXScanner::stop ()
   {
      _secScanner.stop() ;
      _eof = TRUE ;
   }

   /*
      _dmsExtentItr implement
   */
   _dmsExtentItr::_dmsExtentItr( dmsStorageData *su, dmsMBContext * context,
                                 DMS_ACCESS_TYPE accessType,
                                 INT32 direction )
   :_curExtent( NULL )
   {
      SDB_ASSERT( su, "storage data unit can't be NULL" ) ;
      SDB_ASSERT( context, "context can't be NULL" ) ;

      _pSu = su ;
      _context = context ;
      _extentCount = 0 ;
      _accessType = accessType ;
      _direction = direction ;
   }

   void _dmsExtentItr::reset( INT32 direction )
   {
      _extentCount = 0 ;
      _direction = direction ;
      _curExtent = NULL ;
   }

   _dmsExtentItr::~_dmsExtentItr ()
   {
      _pSu = NULL ;
      _context = NULL ;
      _curExtent = NULL ;
   }

   #define DMS_EXTENT_ITR_EXT_PERCOUNT    20

   INT32 _dmsExtentItr::next( dmsExtentID &extentID, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      if ( _extentCount >= DMS_EXTENT_ITR_EXT_PERCOUNT )
      {
         _context->pause() ;
         _extentCount = 0 ;

         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }
      }

      if ( !_context->isMBLock() )
      {
         if ( _context->canResume() )
         {
            rc = _context->resume() ;
         }
         else
         {
            rc = _context->mbLock( DMS_IS_WRITE_OPR( _accessType ) ?
                                   EXCLUSIVE : SHARED ) ;
         }
         PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;

         if ( !dmsAccessAndFlagCompatiblity ( _context->mb()->_flag,
                                              _accessType ) )
         {
            PD_LOG ( PDERROR, "Incompatible collection mode: %d",
                     _context->mb()->_flag ) ;
            rc = SDB_DMS_INCOMPATIBLE_MODE ;
            goto error ;
         }
      }

      if ( NULL == _curExtent )
      {
         if ( _direction > 0 )
         {
            if ( DMS_INVALID_EXTENT == _context->mb()->_firstExtentID )
            {
               rc = SDB_DMS_EOC ;
               goto error ;
            }
            else
            {
               _extRW = _pSu->extent2RW( _context->mb()->_firstExtentID,
                                         _context->mbID() ) ;
               _extRW.setNothrow( FALSE ) ;
               _curExtent = _extRW.readPtr<dmsExtent>() ;
            }
         }
         else
         {
            if ( DMS_INVALID_EXTENT == _context->mb()->_lastExtentID )
            {
               rc = SDB_DMS_EOC ;
               goto error ;
            }
            else
            {
               _extRW = _pSu->extent2RW( _context->mb()->_lastExtentID,
                                         _context->mbID() ) ;
               _extRW.setNothrow( FALSE ) ;
               _curExtent = _extRW.readPtr<dmsExtent>() ;
            }
         }
      }
      else if ( ( _direction >= 0 &&
                  DMS_INVALID_EXTENT == _curExtent->_nextExtent ) ||
                ( _direction < 0 &&
                  DMS_INVALID_EXTENT == _curExtent->_prevExtent ) )
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }
      else
      {
         dmsExtentID next = _direction >= 0 ?
                            _curExtent->_nextExtent :
                            _curExtent->_prevExtent ;
         _extRW = _pSu->extent2RW( next, _context->mbID() ) ;
         _extRW.setNothrow( FALSE ) ;
         _curExtent = _extRW.readPtr<dmsExtent>() ;
      }

      if ( 0 == _curExtent )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !_curExtent->validate( _context->mbID() ) )
      {
         PD_LOG( PDERROR, "Invalid extent[%d]", _extRW.getExtentID() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      extentID = _extRW.getExtentID() ;
      ++_extentCount ;

   done:
      return rc ;
   error:
      goto done ;
   }

   _dmsExtScannerFactory::_dmsExtScannerFactory()
   {
   }

   _dmsExtScannerFactory::~_dmsExtScannerFactory()
   {
   }

   dmsExtScannerBase* _dmsExtScannerFactory::create( dmsStorageDataCommon *su,
                                                     dmsMBContext *context,
                                                     mthMatchRuntime *matchRuntime,
                                                     dmsExtentID curExtentID,
                                                     DMS_ACCESS_TYPE accessType,
                                                     INT64 maxRecords,
                                                     INT64 skipNum,
                                                     INT32 flag,
                                                     _dpsITransLockCallback * callback )
   {
      dmsExtScannerBase* scanner = NULL ;

      if ( OSS_BIT_TEST( DMS_MB_ATTR_CAPPED, context->mb()->_attributes ) )
      {
         scanner = SDB_OSS_NEW dmsCappedExtScanner( su, context,
                                                    matchRuntime,
                                                    curExtentID,
                                                    accessType,
                                                    maxRecords,
                                                    skipNum,
                                                    flag ) ;
      }
      else
      {
         scanner = SDB_OSS_NEW dmsExtScanner( su, context,
                                              matchRuntime,
                                              curExtentID,
                                              accessType,
                                              maxRecords,
                                              skipNum,
                                              flag,
                                              callback ) ;
      }

      if ( !scanner )
      {
         PD_LOG( PDERROR, "Allocate memory for extent scanner failed" ) ;
         goto error ;
      }

   done:
      return scanner ;
   error:
      goto done ;
   }

   dmsExtScannerFactory* dmsGetScannerFactory()
   {
      static dmsExtScannerFactory s_factory ;
      return &s_factory ;
   }

   // Callback function constructor
   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSTRANSLOCKCALLBACK_DMSTRANSLOCKCALLBACK, "dmsTransLockCallback::dmsTransLockCallback" )
   dmsTransLockCallback::dmsTransLockCallback
   (
      dpsTransCB        * transCB,
      _pmdEDUCB         * eduCB,
      _dmsRecordRW      * recordRW
   )
   {
      PD_TRACE_ENTRY( SDB_DMSTRANSLOCKCALLBACK_DMSTRANSLOCKCALLBACK );
      // Caller should only create the transaction callback is transaction
      // is enabled
      SDB_ASSERT( transCB->isTransOn(), "Transaction must be enabled!");
      _transCB = transCB;
      _oldVer = NULL;
      _eduCB = eduCB;
      _isolationLevel = pmdGetOptionCB()->transIsolation();
      _recordRW = recordRW;
      _transExecutor = eduCB->getTransExecutor();
      PD_TRACE_EXIT( SDB_DMSTRANSLOCKCALLBACK_DMSTRANSLOCKCALLBACK );
   }

   // Description:
   //    Function called after lock acquirement. There are two cases to handle:
   //
   // CASE 1: Under RC, TB scanner failed to get record lock in S mode due to
   // incompatibility, we'll try to use the saved old copy if it exist.
   // Note that normally tb scanner or idx scanner will wait on record lock
   // unless transaction isolation level is RC and translockwait is set to NO
   //
   // CASE 2: Update successfully acquire X record lock within a transaction,
   // we'll try to copy the data to lrbHrd if it's not already there
   // Note that we have decided to defer the copy to the actual update time,
   // because there are cases where X lock was acquired, but no update will 
   // be made due to other critieras. However, we will do some preparation 
   // work at this time.
   //
   // Input:
   //    lockId: lock id to operate on
   //    rc:  rc from the lock acquire
   //    requestLockMode: lock mode requested (IS/IX/S/U/X)
   //    opMode: lock operation mode (TRY/ACQUIRE/TEST)
   // Output:
   //    pdpsTxResInfo: return information about the lock
   // Dependency: 
   //    caller must hold lrb bucket latch
   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE, "dmsTransLockCallback::afterLockAcquire" )
   void dmsTransLockCallback::afterLockAcquire
   ( 
      const dpsTransLockId             &lockId,
            INT32                      &rc,
      const DPS_TRANSLOCK_TYPE          requestLockMode,
      const DPS_TRANSLOCK_OP_MODE_TYPE  opMode,
            dpsTransRetInfo           * pdpsTxResInfo = NULL
   )
   {
      UINT32 recSize  = 0;
      PD_TRACE_ENTRY( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE );

      // Handle case 1 mentioned above
      if( (SDB_DPS_TRANS_LOCK_INCOMPATIBLE == rc) &&
          (DPS_TRANSLOCK_S == requestLockMode)     &&
          (lockId.isLeafLevel())                   &&
          (DPS_TRANSLOCK_OP_MODE_TRY == opMode) )
      {
         const dmsRecord * r        = NULL;
         dmsRecord * pbuf     = NULL;
         OBJIDX      hdrIdx;
         dpsTransLRBHeader *lrbHdr = NULL;

         // 0. get the old version record from lrbHdr
         _transCB->getLockMgrHandle()-> getLRBHdrByLockId( lockId,
                                        hdrIdx,
                                        lrbHdr );
         
         // When the update was done by non transaction session, it's 
         // possible that the oldRecord does not exist. We do nothing
         // in that case
         if ( lrbHdr != NULL )
         {
            // skip newly created record
            if ( lrbHdr->isNewRecord() )
            {
               // FIXME: what better RC to use internally
               rc = SDB_CLT_OBJ_NOT_EXIST;
               goto done;
            }

            if ( (r = lrbHdr->getOldRecord()) != NULL )
            {
#ifdef _DEBUG
               PD_LOG( PDDEBUG, 
                       "Use old copy for rid (%d, %d, %d, %d) from memory",
                       lockId.csID(), lockId.clID(),
                       lockId.extentID(), lockId.offset() );
#endif
               // 1. get the record size
               recSize = r->getSize();

               // 2. allocate a seperate buffer off transaction executor to 
               // store the old version. We must do this because the old version
               // record hanging off lrbHeader could be freed up when the
               // update transaction commits. As soon as we get out of the 
               // locking function, there is nothing prevent the transaction
               // commit. 
               //pbuf = _transExecutor->getBuf();
               pbuf = (dmsRecord *)_transExecutor->getExecutor()->getBuffer( recSize );

               if ( pbuf != NULL )
               {
                  // 3. to copy record to buffer 
                  ossMemcpy ( pbuf, r, recSize );

                  // 4. setup the buffer pointer in dmsRecordRW
                  // _ptr is private, but we are its friend now.  
                  _recordRW->_ptr = (const dmsRecord*)pbuf;

                  // 5. set up the mask so that extractData can directly read the 
                  //    ptr. see special handling in readPtr(0)
                  _recordRW->_rid._extent |= DMS_IDX_RID_MASK; 
                  // 6. set the return info if we successfully used old version
                  if ( pdpsTxResInfo )
                  {
                     pdpsTxResInfo->_useOldVersion = TRUE;
                  }

                  // set up the current oldVer in callback for this round, 
                  // note that it is cleared during setup for each round
                  _oldVer = lrbHdr->oldVer;
                  PD_TRACE1 ( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE, 
                              PD_PACK_STRING("Will use old version") );

               } // else we can't get memory to hold old version, 
                 // we simplely keep the RC and caller will wait on the lock
            }
            // else keep the original rc to return
         }
         goto done;
      } // end of case 1

      // Handle case 2 mentioned above
      // X record lock request from a transaction need to prepare to set
      // up old copy if the copy is not already there
      if ( ( SDB_OK == rc )                                 &&
           ( DPS_TRANSLOCK_X == requestLockMode)            &&
           ( lockId.isLeafLevel())                          &&
           ( (DPS_TRANSLOCK_OP_MODE_ACQUIRE == opMode || 
              DPS_TRANSLOCK_OP_MODE_TRY == opMode) ) )
      {
         OBJIDX      hdrIdx;
         dpsTransLRBHeader *lrbHdr = NULL;

         // 1. get the old version record from lrbHdr
         _transCB->getLockMgrHandle()->
                       getLRBHdrByLockId( lockId, hdrIdx, lrbHdr );

         // this is after lock acquire and we got irc== OK, must have 
         // the lock and must be able to find lrbHdr
         SDB_ASSERT ( ((lrbHdr != NULL) && (lrbHdr->oldVer != NULL)),
                      "Can't find the lock" );

         // If this is the first update to the record, the caller will do the
         // setup. Note  the old version might have already been setup, then
         // we don't need to do anything again.
         if ( lrbHdr->getOldRecord() == NULL )
         {
            _oldVer = lrbHdr->oldVer;
         }
#ifdef _DEBUG
         PD_LOG( PDDEBUG, 
                 "Set oldVer(%d) for rid (%d, %d, %d, %d) in memory",
                 _oldVer,
                 lockId.csID(), lockId.clID(),
                 lockId.extentID(), lockId.offset());

         PD_TRACE1 ( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE, 
                     PD_PACK_STRING("Will setup old version") );
#endif
         goto done;
      }

      done:
      PD_TRACE_EXIT( SDB_DMSTRANSLOCKCALLBACK_AFTERLOCKACQUIRE );
      return;
   }

   // Description:
   //    Function called before lock release(in dpsTransLockManager::_release)
   // Under RC, before X lock is released, the update is responsible
   // to free up the kept old version record and clean up all related indexes
   // from the in-memory index tree. All information were kept in lrbHdr->oldVer
   // The latching protocal has to be: 
   // 1. LRB hash bkt latch must be held(X) to tranverse/update lrbHdrs/LRBs
   // 2. preIdxTree latch must be held in X to insert/delete node in the tree
   //    oldVersionCB(_oldVersionCBLatch) need to be held in S before 
   //    accessing individual index tree.
   // 3. Request preIdxTree latch while holding LRB hash bkt latch is OK, 
   //    But reverse order is forbidden. 
   // 
   // Input:
   //    lockId: lock id to operate on
   //    lockMode: lock mode requested (IS/IX/S/U/X)
   //    refCounter: current reference counter of the lock
   //    oldVer: pointer to oldVersionContainer
   // Output:
   //    oldVer: pointer to oldVersionContainer
   // Dependency: 
   //    caller MUST hold the record lock and LRB hash bkt latch in X
   // which protects all the update on oldVer
   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE, "dmsTransLockCallback::beforeLockRelease" )
   void dmsTransLockCallback::beforeLockRelease
   (
      const dpsTransLockId     &lockId ,  
      const DPS_TRANSLOCK_TYPE  lockMode,
      const UINT64              refCounter,  
      oldVersionContainer      *oldVer   
   ) 
   {
      PD_TRACE_ENTRY( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE );

      oldVersionCB      *oldVCB    = NULL;
      preIdxTree        *memTree   = NULL;
      globIdxID          gID;
      dmsRecordID        rid(lockId.extentID(), lockId.offset()) ;
      BSONObj           *idxobj    = NULL;
      INT32              numDeleted = 0;

      gID._csID = lockId.csID();
      gID._clID = lockId.clID();

      PD_TRACE4( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE, 
                 PD_PACK_INT(gID._csID),
                 PD_PACK_INT(gID._clID),
                 PD_PACK_INT(rid._extent),
                 PD_PACK_INT(rid._offset) );

      oldVCB = _transCB->getOldVCB();

      // early exit if this is not record lock OR not in X mode OR 
      // there is no old record setup. 
      if ( ( !lockId.isLeafLevel() )               ||
           ( DPS_TRANSLOCK_X != lockMode )         ||
           ( 0 != refCounter )                     ||
           ( NULL == oldVer )                      ||
           ( NULL == oldVer->getOldRecord() ) )
      {  
         // During insert, we will set record new without putting old record
         // Now when commit/rollback, we must clear the flag
         if( (0 == refCounter) && (DPS_TRANSLOCK_X == lockMode) && oldVer )
         {
            oldVer->unsetRecordNew();
         }
#ifdef _DEBUG
         PD_LOG( PDDEBUG, "skipping beforeLockRelease callback, lockmode=%d,"
                          "refCounter=%d, oldVer=%d, rid=(%d, %d)",
                          lockMode, refCounter, oldVer, rid._extent, rid._offset );
#endif
         goto done; 
      }
      
      SDB_ASSERT ( oldVCB != NULL, "oldVCB can't be NULL" );
      // few common setups

      PD_LOG( PDDEBUG, 
              "Trying to delete old copy for rid (%d, %d, %d, %d) from memory",
              gID._csID, gID._clID, rid._extent, rid._offset );

      // Now remove all indexes from the in memory index tree and free the 
      // indexes. We can't simply do set clear because of the memory allocated
      for( idxObjSet::iterator it = oldVer->getIdxSet().begin();
           it != oldVer->getIdxSet().end(); ++it )
      {
         gID._idxLID = it->_idxLID;

         idxobj = const_cast<BSONObj *>(&(it->_idxObj));
#ifdef _DEBUG
         PD_LOG( PDDEBUG, 
                 "Trying to delete index (key=%s), id=(%d, %d, %d) "
                 "from in memory tree",
                 idxobj->toString().c_str(),
                 gID._csID, gID._clID, gID._idxLID );
#endif
         oldVCB->latchS();
         memTree = oldVCB->getIdxTree( gID );

         // delete the entry from the tree, tree lock is taken in the func
         numDeleted = memTree->remove( idxobj, rid );
         
         oldVCB->releaseS();

         PD_TRACE1( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE, 
                    PD_PACK_INT(numDeleted) );

         SDB_ASSERT( ( 1 == numDeleted ), "deleted other than one keys" );

         // now free the index object from segment
         //oldVCB->getMemBlockPool()-> release( (CHAR **) &idxobj,
         //                                     it->_recordMemType );

         // free the entry from index set protected by lrbhashbkt latch
         oldVer->deleteIdx(it);
      } // end of for

      // Now free index LID set 
      oldVer->clearIdxLid();

      // delete order
      oldVer->freeOrder();

      // Now free the old record
      oldVCB->getMemBlockPool()->release( (CHAR * &) (oldVer->getOldRecord()), 
                                          oldVer->getRecordMemType() );
      oldVer->setOldRecord( NULL );
      oldVer->unsetRecordNew();
      // Note that LRBHdr->oldVer is never freed up until freeing lrbHdr 

      done:
      PD_TRACE_EXIT( SDB_DMSTRANSLOCKCALLBACK_BEFORELOCKRELEASE );
      return;
   }

   // Description
   INT32 dmsTransLockCallback::saveOldVersionRecord( _dmsRecordRW * recordRW )
   {
      INT32  rc      = SDB_OK;
      UINT32 recSize = 0;
      // if the oldRecord does not exist, we will create one
      if ( _oldVer && _oldVer->getOldRecord()== NULL )
      {
            const dmsRecord *pRecord= recordRW->readPtr(0) ;
            char * buf = NULL;

            // 1. get to overflow record if needed
            if ( pRecord->isOvf() )
            {
               dmsRecordID ovfRID = pRecord->getOvfRID() ;
               //dmsRecordRW ovfRW = _recordRW->derive( ovfRID );
               dmsRecordRW ovfRW = recordRW->derive( ovfRID );
               //ovfRW.setNothrow( _recordRW->isNothrow() ) ;
               ovfRW.setNothrow( recordRW->isNothrow() ) ;
               pRecord = ovfRW.readPtr( 0 ) ;
            }

            // 2. allocate from segment
            recSize = pRecord->getSize();
            rc =  _transCB->getOldVCB()->getMemBlockPool()->
                      acquire(recSize, buf, &(_oldVer->getRecordMemType()));

            if ( SDB_OK == rc )
            {
               SDB_ASSERT( ( buf != NULL ), "allocation returned NULL" );
               // 3. copy from _recordRW->_ptr to the old record
               //ossMemcpy ( buf, _recordRW->readPtr(), recSize );
               ossMemcpy ( buf, pRecord, recSize );
               _oldVer->setOldRecord( (dmsRecord *)buf ); 

#ifdef _DEBUG
               PD_LOG ( PDDEBUG, "Saved old copy for rid(%d, %d) to oldVer(%d)", 
                        recordRW->getRecordID()._extent, 
                        recordRW->getRecordID()._offset, 
                        _oldVer );
#endif              
            }
            else
            {
               PD_LOG ( PDERROR, "Allocating buffer for old copy failed, rc=%d", rc );
            }
      }
      return rc;
   }


}


