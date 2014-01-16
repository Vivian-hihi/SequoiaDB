
#include "rtn.hpp"
#include "pmdCB.hpp"
#include "dpsMessageBlock.hpp"
#include "clsReplayer.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "dpsTransLockDef.hpp"
#include "dpsLogRecordDef.hpp"
#include "dpsOp2Record.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNTRANSBEGIN, "rtnTransBegin" )
   INT32 rtnTransBegin( _pmdEDUCB * cb )
   {
      PD_TRACE_ENTRY ( SDB_RTNTRANSBEGIN ) ;
      SDB_ASSERT( cb, "cb can't be null" )
      INT32 rc = SDB_OK;
      if ( !pmdGetKRCB()->getTransCB()->isTransOn() )
      {
         goto done;
      }
      if ( cb->getTransID() == DPS_INVALID_TRANS_ID )
      {
         cb->setTransID( pmdGetKRCB()->getTransCB()->allocTransID() );
         cb->setCurTransLsn( DPS_INVALID_LSN_OFFSET );
         pmdGetKRCB()->getTransCB()->addTransCB( cb->getTransID(), cb );
      }
      PD_LOG( PDEVENT,
            "begin transaction operations(transID=%llu)",
            cb->getTransID() );
      PD_TRACE_EXIT ( SDB_RTNTRANSBEGIN ) ;
   done:
      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNTRANSCOMMIT, "rtnTransCommit" )
   INT32 rtnTransCommit( _pmdEDUCB * cb, SDB_DPSCB *dpsCB )
   {
      PD_TRACE_ENTRY ( SDB_RTNTRANSCOMMIT ) ;
      SDB_ASSERT( cb, "cb can't be null" )
      INT32 rc = SDB_OK;
      DPS_LSN_OFFSET offset;
      DPS_TRANS_ID curTransID;
      DPS_LSN_OFFSET preTransLsn;
      dpsMergeInfo info;
      dpsLogRecord &record = info.getMergeBlock().record() ;

      if ( !pmdGetKRCB()->getTransCB()->isTransOn() )
      {
         goto done;
      }
      curTransID = cb->getTransID();
      preTransLsn = cb->getCurTransLsn();

      if ( curTransID == DPS_INVALID_TRANS_ID
         || preTransLsn == DPS_INVALID_LSN_OFFSET )
      {
         pmdGetKRCB()->getTransCB()->delTransCB( curTransID );
         cb->setTransID( DPS_INVALID_TRANS_ID );
         goto done;
      }
      if ( !dpsCB )
      {
         goto done;
      }

      PD_LOG( PDEVENT,
            "execute commit(transID=%llu, lastLsn=%llu)",
            curTransID, preTransLsn );

      rc = dpsTransCommit2Record( curTransID, record ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to build commit log:%d",rc ) ;
         goto error ;
      }

      info.setInfoEx( ~0, DMS_INVALID_CLID, DMS_INVALID_EXTENT ) ;
      rc = dpsCB->prepare( info );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to insert record into log(rc=%d)", rc );
      dpsCB->writeData( info );
      if ( pmdGetKRCB()->getDBRole() != SDB_ROLE_STANDALONE )
      {
         if ( info.hasDummy() )
         {
            offset = info.getDummyBlock().record().head()._lsn;
            cb->insertLsn( offset ) ;
            pmdGetKRCB()->getReplCB()->notify( ~0,
                                               DMS_INVALID_CLID,
                                               DMS_INVALID_EXTENT,
                                               offset ) ;
         }
         offset = info.getMergeBlock().record().head()._lsn;
         cb->insertLsn( offset );
         pmdGetKRCB()->getReplCB()->notify( ~0,
                                            DMS_INVALID_CLID,
                                            DMS_INVALID_EXTENT,
                                            offset );
      }

      pmdGetKRCB()->getTransCB()->delTransCB( curTransID );
      cb->setTransID( DPS_INVALID_TRANS_ID );
      cb->setCurTransLsn( DPS_INVALID_LSN_OFFSET );
      pmdGetKRCB()->getTransCB()->transLockReleaseAll( cb );
   done:
      PD_TRACE_EXITRC ( SDB_RTNTRANSCOMMIT, rc ) ;
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNTRANSROLLBACK, "rtnTransRollback" )
   INT32 rtnTransRollback( _pmdEDUCB * cb, SDB_DPSCB *dpsCB )
   {
      PD_TRACE_ENTRY ( SDB_RTNTRANSROLLBACK ) ;
      SDB_ASSERT( cb, "cb can't be null" )
      INT32 rc = SDB_OK;
      _dpsMessageBlock mb(DPS_MSG_BLOCK_DEF_LEN);
      DPS_LSN dpsLsn;
      DPS_LSN_OFFSET curLsnOffset;
      DPS_TRANS_ID transID;
      DPS_TRANS_ID curTransID;
      DPS_TRANS_ID rollbackID;
      _clsReplayer replayer(TRUE);

      /*if ( !pmdGetKRCB()->getTransCB()->isTransOn() )
      {
         goto done;
      }*/

      cb->startRollback();
      curLsnOffset = cb->getCurTransLsn();
      transID = cb->getTransID();
      rollbackID = pmdGetKRCB()->getTransCB()->getRollbackID( transID );
      if ( DPS_INVALID_TRANS_ID == transID
         || DPS_INVALID_LSN_OFFSET == curLsnOffset )
      {
         goto done;
      }
      if ( !dpsCB )
      {
         goto done;
      }

      PD_LOG ( PDEVENT,
            "execute rollback(transID=%llu, lastLsn=%llu)",
            transID, curLsnOffset );

      cb->setTransID( rollbackID );
      // read the log and rollback one by one
      while ( curLsnOffset != DPS_INVALID_LSN_OFFSET )
      {
         dpsLogRecord record;
         mb.clear();
         dpsLsn.offset = curLsnOffset;
         rc = dpsCB->search( dpsLsn, &mb );
         PD_RC_CHECK( rc, PDERROR,
                     "rollback failed, "
                     "failed to get the log(offset=%llu, version=%d, rc=%d)",
                     curLsnOffset, dpsLsn.version, rc );
         rc = record.load( mb.offset( 0 ));
         PD_RC_CHECK( rc, PDERROR,
                     "rollback failed, failed to parse log",
                     "(lsn=%llu, rc=%d)", curLsnOffset, rc );
         dpsLogRecord::iterator itr = record.find( DPS_LOG_PUBLIC_TRANSID ) ;
         if ( !itr.valid() )
         {
            PD_LOG( PDERROR, "can not find DPS_LOG_PUBLIC_TRANSID"
                             " in record." ) ;
            rc = SDB_SYS ;
            break ;
         }
         curTransID = pmdGetKRCB()->getTransCB()->getTransID(
                                        *((DPS_TRANS_ID *)itr.value()) );
         PD_CHECK( curTransID == transID, SDB_DPS_CORRUPTED_LOG, error,
                  PDERROR, "failed to rollback(lsn=%llu), the log is damaged",
                  curLsnOffset );
         if ( pmdGetKRCB()->getDBRole() == SDB_ROLE_DATA
            && !(pmdGetKRCB()->getReplCB()->primaryIsMe()) )
         {
            if ( !(pmdGetKRCB()->getClsCB()->isFullSync()) )
            {
               pmdGetKRCB()->getTransCB()->addTransInfo( transID, curLsnOffset );
            }
            rc = SDB_CLS_NOT_PRIMARY;
            goto error;
         }

         {
         dpsLogRecord::iterator itr = record.find(DPS_LOG_PUBLIC_PRETRANS ) ;
         if ( !itr.valid() )
         {
            /// it is the first log.
            curLsnOffset = DPS_INVALID_LSN_OFFSET ;
         }
         else
         {
            curLsnOffset = *(( DPS_LSN_OFFSET *)itr.value());
         }
         
         cb->setCurTransLsn( curLsnOffset );
         /// TODO:already get record, no need to parse agagin in rollback.
         rc = replayer.rollback( ( dpsLogRecordHeader *)mb.offset(0), cb );
         PD_RC_CHECK( rc, PDERROR,
                     "rollback failed(lsn=%llu,rc=%d)",
                     dpsLsn.offset, rc );
         }
      }
   done:
      // complete the transaction whether success or not,
      // this avoid infinite recursion when rollback failed
      pmdGetKRCB()->getTransCB()->delTransCB( transID );
      cb->setTransID( DPS_INVALID_TRANS_ID );
      cb->setCurTransLsn( DPS_INVALID_LSN_OFFSET );
      pmdGetKRCB()->getTransCB()->transLockReleaseAll( cb );
      cb->stopRollback();
      PD_TRACE_EXITRC ( SDB_RTNTRANSROLLBACK, rc ) ;
      return rc;
   error:
      goto done;
   }

   INT32 rtnTransRollbackAll( _pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB();
      SDB_DPSCB *pDpsCB = pmdGetKRCB()->getDPSCB();
      TRANS_MAP *pTransMap = pTransCB->getTransMap();
      DPS_LSN dpsLsn;
      DPS_TRANS_ID transID;
      DPS_TRANS_ID rollbackID;
      DPS_LSN_OFFSET curLsnOffset;
      _clsReplayer replayer( TRUE );
      _dpsMessageBlock mb(DPS_MSG_BLOCK_DEF_LEN);

      cb->startRollback();

      PD_LOG ( PDEVENT, "rollback all unfinished-transaction" ) ;

      while ( pTransMap->size() != 0 )
      {
         TRANS_MAP::iterator iterMap = pTransMap->begin();
         transID = iterMap->first;
         rollbackID = pTransCB->getRollbackID( transID );
         curLsnOffset = iterMap->second;
         cb->setTransID( rollbackID );
         while ( curLsnOffset != DPS_INVALID_LSN_OFFSET )
         {
            if ( !pTransCB->isDoRollback() )
            {
               PD_LOG ( PDEVENT,
                        "rollback is interrupted" );
               rc = SDB_INTERRUPT;
               goto error;
            }
            dpsLogRecord record;
            mb.clear();
            dpsLsn.offset = curLsnOffset;
            rc = pDpsCB->search( dpsLsn, &mb );
            if ( rc )
            {
               // don't return,
               // stop rollback current transaction,
               // go on to rollback other transaction
               PD_LOG ( PDERROR,
                        "rollback failed, "
                        "failed to get the log( offset =%llu, version=%d, rc=%d)",
                        curLsnOffset, dpsLsn.version, rc );
               break;
            }
            rc = record.load( mb.offset( 0 ));
            if ( rc )
            {
               // don't return,
               // stop rollback current transaction,
               // go on to rollback other transaction
               PD_LOG ( PDERROR,
                        "rollback failed, "
                        "failed to parse log(lsn=%llu, rc=%d)",
                        curLsnOffset, rc );
               break;
            }
            dpsLogRecord::iterator itr =
                            record.find( DPS_LOG_PUBLIC_TRANSID ) ;
            if ( !itr.valid() )
            {
               PD_LOG( PDERROR, "failed to find transid in record." ) ;
               rc = SDB_SYS ;
               break ;
            }

            if ( transID != pTransCB->getTransID(
                                     *(DPS_TRANS_ID *)(itr.value()) ))
            {
               // don't return,
               // stop rollback current transaction,
               // go on to rollback other transaction
               PD_LOG ( PDERROR,
                        "failed to rollback(lsn=%llu), "
                        "the log is damaged",
                        curLsnOffset );
               break;
            }

            {
            dpsLogRecord::iterator itr =
                            record.find( DPS_LOG_PUBLIC_PRETRANS ) ;
            if ( !itr.valid() )
            {
               curLsnOffset = DPS_INVALID_LSN_OFFSET;
            }
            else
            {
               curLsnOffset = *((DPS_LSN_OFFSET *)itr.value() );
            }
            cb->setCurTransLsn( curLsnOffset );
            rc = replayer.rollback( (dpsLogRecordHeader *)mb.offset(0), cb );
            if ( rc )
            {
               PD_LOG( PDERROR,
                     "rollback failed(lsn=%llu, rc=%d)",
                     dpsLsn.offset, rc );
               break;
            }
            iterMap->second = curLsnOffset;
            }
         }
         pTransMap->erase( iterMap );
      }
   done:
      pmdGetKRCB()->getTransCB()->transLockReleaseAll( cb );
      pTransCB->stopRollbackTask();
      cb->stopRollback();
      return rc;
   error:
      goto done;
   }

   INT32 rtnTransTryLockCL( const CHAR *pCollection, INT32 lockType,
                           _pmdEDUCB *cb,SDB_DMSCB *dmsCB,
                           SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK;
      dmsStorageUnitID suID = DMS_INVALID_CS;
      UINT32 logicCSID = ~0;
      dmsStorageUnit *su = NULL;
      const CHAR *pCollectionShortName = NULL;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB();
      UINT16 collectionID = DMS_INVALID_MBID;
      SDB_ASSERT ( pCollection, "collection can't be NULL" )
      SDB_ASSERT ( dmsCB, "dmsCB  can't be NULL" )
      SDB_ASSERT ( dpsCB, "dpsCB  can't be NULL" )
      SDB_ASSERT ( cb, "cb  can't be NULL" )
      rc = rtnResolveCollectionNameAndLock ( pCollection, dmsCB, &su,
                                          &pCollectionShortName, suID );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to resolve collection name(collection:%s, rc=%d)",
                  pCollection, rc );
      rc = su->data()->findCollection ( pCollectionShortName, collectionID ) ;
      logicCSID = su->LogicalCSID();
      dmsCB->suUnlock ( suID );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to find the collection(collection:%s, rc=%d)",
                  pCollection, rc );
      switch( lockType )
      {
      case DPS_TRANSLOCK_S:
            rc = pTransCB->transLockTryS( cb, logicCSID, collectionID );
            break;
      case DPS_TRANSLOCK_X:
            rc = pTransCB->transLockTryX( cb, logicCSID, collectionID );
            break;
      default:
            rc = SDB_INVALIDARG;
            PD_RC_CHECK( rc, PDERROR, "invalid lock-type" );
      }
   done:
      return rc;
   error:
      goto done;
   }

   INT32 rtnTransTryLockCS( const CHAR *pSpace, INT32 lockType,
                           _pmdEDUCB *cb,SDB_DMSCB *dmsCB,
                           SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK;
      dmsStorageUnitID suID = DMS_INVALID_CS;
      UINT32 logicCSID = ~0;
      dmsStorageUnit *su = NULL;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB();
      SDB_ASSERT ( pSpace, "space can't be NULL" )
      SDB_ASSERT ( dmsCB, "dmsCB  can't be NULL" )
      SDB_ASSERT ( dpsCB, "dpsCB  can't be NULL" )
      SDB_ASSERT ( cb, "cb  can't be NULL" )
      UINT32 length = ossStrlen ( pSpace );
      PD_CHECK( (length > 0 && length <= DMS_SU_NAME_SZ), SDB_INVALIDARG,
               error, PDERROR, "invalid length of collectionspace name:%s",
               pSpace );

      rc = dmsCB->nameToSUAndLock( pSpace, suID, &su );
      PD_CHECK(( su != NULL && suID != DMS_INVALID_SUID), SDB_DMS_CS_NOTEXIST,
               error, PDERROR, "lock collection space(%s) failed(rc=%d)",
               pSpace, rc );
      logicCSID = su->LogicalCSID();
      dmsCB->suUnlock ( suID ) ;
      switch( lockType )
      {
      case DPS_TRANSLOCK_S:
            rc = pTransCB->transLockTryS( cb, logicCSID );
            break;
      case DPS_TRANSLOCK_X:
            rc = pTransCB->transLockTryX( cb, logicCSID );
            break;
      default:
            rc = SDB_INVALIDARG;
            PD_RC_CHECK( rc, PDERROR, "invalid lock-type" );
      }
   done:
      return rc;
   error:
      goto done;
   }

   INT32 rtnTransReleaseLock( const CHAR *pCollection,
                           _pmdEDUCB *cb,SDB_DMSCB *dmsCB,
                           SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK;
      dmsStorageUnitID suID = DMS_INVALID_CS;
      UINT32 logicCSID = ~0;
      dmsStorageUnit *su = NULL;
      const CHAR *pCollectionShortName = NULL;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB();
      UINT16 collectionID = DMS_INVALID_MBID;
      CHAR *pDot = NULL;
      CHAR *pDot1 = NULL;
      pDot = (CHAR *)ossStrchr( pCollection, '.' );
      pDot1 = (CHAR *)ossStrrchr( pCollection, '.' );
      PD_CHECK( (pDot == pDot1 && pCollection != pDot), SDB_INVALIDARG,
               error, PDERROR, "invalid format for collection name:%s, "
               "expected format:<collectionspace>[.<collectionname>]",
               pCollection );
      if ( pDot )
      {
         rc = rtnResolveCollectionNameAndLock( pCollection, dmsCB, &su,
                                             &pCollectionShortName, suID );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to resolve collection name(collection:%s, rc=%d)",
                     pCollection, rc );
         rc = su->data()->findCollection( pCollectionShortName, collectionID ) ;
         logicCSID = su->LogicalCSID();
         dmsCB->suUnlock( suID );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to find collection(collection:%s, rc=%d)",
                     pCollection, rc );
      }
      else
      {
         rc = dmsCB->nameToSUAndLock( pCollection, suID, &su );
         PD_CHECK(( su != NULL && suID != DMS_INVALID_SUID), SDB_DMS_CS_NOTEXIST,
               error, PDERROR, "lock collection space(%s) failed(rc=%d)",
               pCollection, rc );
         logicCSID = su->LogicalCSID();
         dmsCB->suUnlock( suID );
      }
      pTransCB->transLockRelease( cb, logicCSID, collectionID );
   done:
      return rc;
   error:
      goto done;
   }

}
