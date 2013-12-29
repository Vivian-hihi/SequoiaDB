/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsReplSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

*******************************************************************************/

#include "clsReplSession.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "rtn.hpp"
#include "pmdStartup.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   const UINT32 CLS_SYNC_DEF_LEN = 1024 ;
   const UINT16 CLS_SYNC_INTERVAL = 2000 ;

   #define CLS_REPL_MAX_TIME           (2)

   BEGIN_OBJ_MSG_MAP( _clsReplSession , _clsSession )
      //ON_MSG
      ON_MSG( MSG_CLS_SYNC_REQ, handleSyncReq )
      ON_MSG( MSG_CLS_SYNC_RES, handleSyncRes )
      ON_MSG( MSG_CLS_SYNC_NOTIFY, handleNotify )
      ON_MSG( MSG_CLS_SYNC_VIR_REQ, handleVirSyncReq )
      ON_MSG( MSG_CLS_CONSULTATION_REQ, handleConsultReq )
      ON_MSG( MSG_CLS_CONSULTATION_RES, handleConsultRes )
   END_OBJ_MSG_MAP()

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN__CLSREPSN, "_clsReplSession::_clsReplSession" )
   _clsReplSession::_clsReplSession ( UINT64 sessionID )
      :_clsSession ( sessionID ),
       _mb( CLS_SYNC_DEF_LEN ),
       _status( CLS_SESSION_STATUS_SYNC ),
       _quit( FALSE ),
       _timeout( 0 ),
       _consultLsn()
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSN__CLSREPSN );
      _logger = pmdGetKRCB()->getDPSCB() ;
      _sync = pmdGetKRCB()->getClsCB()->getReplCB()->syncMgr() ;
      _agent = pmdGetKRCB()->getClsCB()->getReplRouteAgent() ;
      _repl = pmdGetKRCB()->getClsCB()->getReplCB() ;
      _pReplBucket = _repl->getBucket() ;
      _pReplBucket->reset() ;

      _requestID = 0 ;
      _syncFailedNum = 0 ;
      _isFirstToSync = TRUE ;

      _syncSrc.value = MSG_INVALID_ROUTEID ;

      //if start form crash, should full sync
      if ( SDB_START_CRASH == pmdGetKRCB()->getStartType() )
      {
         _status = CLS_SESSION_STATUS_FULL_SYNC ;
      }
      PD_TRACE_EXIT ( SDB__CLSREPSN__CLSREPSN );
   }

   _clsReplSession::~_clsReplSession ()
   {
   }

   INT32 _clsReplSession::type () const
   {
      return CLS_REPL ;
   }

   EDU_TYPES _clsReplSession::eduType () const
   {
      return EDU_TYPE_REPLAGENT ;
   }

   void _clsReplSession::onRecieve ( const NET_HANDLE netHandle,
                                     MsgHeader * msg )
   {
      if ( !isStartActive() )
      {
         _timeout = 0 ;
      }
   }

   BOOLEAN _clsReplSession::timeout( UINT32 interval )
   {
      return _quit ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN_ONTIMER, "_clsReplSession::onTimer" )
   void _clsReplSession::onTimer( UINT64 timerID, UINT32 interval )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSN_ONTIMER ) ;
      _timeout += interval ;

      if ( ! isStartActive() )
      {
         //if the peer node no msg a long time,need to quit
         if ( CLS_DST_SESSION_NO_MSG_TIME < _timeout )
         {
            PD_LOG ( PDEVENT, "Session[%s] peer node a long time no msg, quit",
                     sessionName() ) ;
            _quit = TRUE ;
         }

         goto done ;
      }

      _selector.timeout( interval ) ;

      if ( _timeout < CLS_SYNC_INTERVAL )
      {
         goto done ;
      }
      _timeout = 0 ;

      if ( !_sync->isReadyToReplay() )
      {
         _isFirstToSync = TRUE ;
         goto done ;
      }
      else if ( _isFirstToSync &&
                pmdGetKRCB()->getEDUMgr()->getWritingEDUCount() > 0 )
      {
         PD_LOG( PDWARNING, "Has some writing edus don't exist, can't to "
                 "sync" ) ;
         goto done ;
      }
      _isFirstToSync = FALSE ;

      //if the peer node is sharing-break, shoud change node
      if ( MSG_INVALID_ROUTEID != _syncSrc.value
         && !_repl->isAlive ( _syncSrc ) )
      {
         PD_LOG ( PDWARNING, "Session[%s] peer node sharing-break",
                  sessionName() ) ;
         _selector.addToBlakList ( _syncSrc ) ;
         _selector.clearSrc () ;
         _syncSrc = _selector.src() ;
      }

      // has error, need to rollback
      if ( CLS_BUCKET_WAIT_ROLLBACK == _pReplBucket->getStatus() )
      {
         _pReplBucket->waitEmptyAndRollback() ;
         DPS_LSN expectLSN = _pReplBucket->completeLSN() ;
         INT32 rcTmp = _logger->move( expectLSN.offset, expectLSN.version ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Failed to move lsn to[%u, %llu], rc: %d",
                    expectLSN.version, expectLSN.offset, rcTmp ) ;
            _status = CLS_SESSION_STATUS_FULL_SYNC ;
         }
         else
         {
            PD_LOG( PDEVENT, "Move lsn to[%u, %llu]", expectLSN.version,
                    expectLSN.offset ) ;
         }
      }

      if ( CLS_SESSION_STATUS_SYNC == _status )
      {
         _sendSyncReq() ;
      }
      else if ( CLS_SESSION_STATUS_CONSULT == _status )
      {
         _sendConsultReq() ;
      }
      else if ( CLS_SESSION_STATUS_FULL_SYNC == _status )
      {
         _fullSync () ;
      }
      else
      {
         //do nothing
      }

   done:
      PD_TRACE1 ( SDB__CLSREPSN_ONTIMER, PD_PACK_INT(_quit) ) ;
      PD_TRACE_EXIT ( SDB__CLSREPSN_ONTIMER );
      return  ;
   }

   void _clsReplSession::_onDetach()
   {
      if ( isStartActive() )
      {
         if ( _pReplBucket->waitEmptyAndRollback() )
         {
            DPS_LSN expectLSN = _pReplBucket->completeLSN() ;
            INT32 rcTmp = _logger->move( expectLSN.offset, expectLSN.version ) ;
            if ( rcTmp )
            {
               PD_LOG( PDERROR, "Failed to move lsn to[%u, %llu], rc: %d",
                       expectLSN.version, expectLSN.offset, rcTmp ) ;
            }
            else
            {
               PD_LOG( PDEVENT, "Move lsn to[%u, %llu]", expectLSN.version,
                       expectLSN.offset ) ;
            }
         }
         _pReplBucket->close() ;
      }

      if ( isStartActive() && CLS_SESSION_STATUS_FULL_SYNC != _status
           && CLS_BS_CLOSED != _repl->getStatus() )
      {
         pmdGetKRCB()->getClsCB()->startInnerSession( CLS_REPL,
                                                      CLS_TID_REPL_SYC ) ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN_HNDSYNCREQ, "_clsReplSession::handleSyncReq" )
   INT32 _clsReplSession::handleSyncReq( NET_HANDLE handle, MsgHeader* header )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSN_HNDSYNCREQ );
      SDB_ASSERT( NULL != header, "header should not be NULL" )
      MsgReplSyncReq *msg = ( MsgReplSyncReq * )header ;
      if ( DPS_INVALID_LSN_OFFSET != msg->completeNext.offset )
      {
         _sync->complete( msg->identity, msg->completeNext,
                          CLS_TID( _sessionID ) ) ;
      }
      else
      {
         _sync->complete( msg->identity, msg->next,
                          CLS_TID( _sessionID ) ) ;
      }
      rc = _syncLog( handle, msg ) ;
      PD_TRACE_EXITRC ( SDB__CLSREPSN_HNDSYNCREQ, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN_HNDSYNCRES, "_clsReplSession::handleSyncRes" )
   INT32 _clsReplSession::handleSyncRes( NET_HANDLE handle, MsgHeader* header )
   {
      SDB_ASSERT( NULL != header, "header should not be NULL" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSN_HNDSYNCRES );

      /// 1. some validate.
      MsgReplSyncRes *msg = ( MsgReplSyncRes * )header ;
      if ( CLS_SESSION_STATUS_SYNC != _status )
      {
         PD_LOG ( PDDEBUG, "replSync: status[%d] not expect[%d], ignore",
                  _status , CLS_SESSION_STATUS_SYNC ) ;
         goto done ;
      }
      else if ( !_sync->isReadyToReplay() )
      {
         PD_LOG ( PDDEBUG, "Not ready to replay, ignore" ) ;
         goto done ;
      }
      else if ( msg->identity.value != _syncSrc.value )
      {
         PD_LOG ( PDDEBUG, "Node id[%d] is not expect[%d], ignore",
                  msg->identity.columns.nodeID,
                  _syncSrc.columns.nodeID ) ;
         goto done ;
      }
      else if ( msg->header.header.requestID != _requestID )
      {
         PD_LOG ( PDDEBUG, "RequestID[%lld] not expected[%lld], ignore",
                  msg->header.header.requestID, _requestID ) ;
         goto done ;
      }
      else
      {
         _selector.clearTime() ;

         if ( CLS_BS_NORMAL != _repl->getStatus() )
         {
            PD_LOG ( PDDEBUG, "Repl status[%d] is not normal, ignore",
                     _repl->getStatus() ) ;
            goto done ;
         }
      }

      if ( SDB_OK == msg->header.res )
      {
         UINT32 num = 0 ;
         rc = _replayLog( (const CHAR *)
               ( ( ossValuePtr )(&(msg->header)) + sizeof( MsgReplSyncRes ) ),
               msg->header.header.messageLength - sizeof( MsgReplSyncRes ),
               num ) ;
         PD_LOG ( PDDEBUG, "repl: ReplyLog num:%d, rc: %d", num, rc ) ;

         if ( 0 != num )
         {
            _syncFailedNum = 0 ;
            rc = SDB_OK ;

            _sendSyncReq() ;
            _selector.clearBlacklist () ;
         }
         else if ( SDB_OK == rc )
         {
            if ( _repl->getPrimary().value != _syncSrc.value )
            {
               _syncFailedNum = 0 ;

               /// sync res is ok but remote has no more new data.
               /// we choose a new node to sync data.
               _selector.addToBlakList( _syncSrc ) ;
               _selector.clearSrc() ;

               // can't call _sendSyncReq, because the primary maybe
               // sharing-break, so this will run loop for other nodes fastly
               _timeout = CLS_SYNC_INTERVAL ;
            }

            if ( _pReplBucket->maxReplSync() > 0 )
            {
               // if has complete some log replay,need to notify primary
               DPS_LSN completeLSN = _pReplBucket->completeLSN() ;
               if ( !completeLSN.invalid() &&
                    _completeLSN.offset != completeLSN.offset )
               {
                  _sendSyncReq( &completeLSN ) ;
               }
               else if ( !completeLSN.invalid() &&
                         completeLSN.offset != _logger->expectLsn().offset )
               {
                  if ( SDB_OK == _pReplBucket->waitSubmit( OSS_ONE_SEC ) )
                  {
                     _sendSyncReq () ;
                  }
                  else
                  {
                     _timeout += OSS_ONE_SEC ;
                  }
               }
            }
         }
      }

      if ( SDB_OK != msg->header.res || SDB_OK != rc )
      {
         _syncFailedNum++ ;
         if ( _syncFailedNum < _repl->groupSize() - 1 )
         {
            _selector.addToBlakList( _syncSrc ) ;
            _selector.clearSrc() ;

            _sendSyncReq() ;
         }
         else
         {
            _sendConsultReq() ;
         }
      }
      else
      {
         if ( pmdGetKRCB()->getTransCB()->isNeedSyncTrans() )
         {
            rc = pmdGetKRCB()->getTransCB()->syncTransInfoFromLocal(
                                                   msg->oldestTransLsn ) ;
            if ( SDB_OK == rc )
            {
               pmdGetKRCB()->getTransCB()->setIsNeedSyncTrans( FALSE );
            }
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSN_HNDSYNCRES, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN_HNDVIRSYNCREQ, "_clsReplSession::handleVirSyncReq" )
   INT32 _clsReplSession::handleVirSyncReq( NET_HANDLE handle,
                                            MsgHeader* header )
   {
      SDB_ASSERT( NULL != header, "header should not be NULL" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSN_HNDVIRSYNCREQ );
      MsgReplVirSyncReq *msg = ( MsgReplVirSyncReq * )header ;
      _sync->complete( msg->from, msg->next, CLS_TID( _sessionID ) ) ;
      PD_TRACE_EXITRC ( SDB__CLSREPSN_HNDVIRSYNCREQ, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN_HNDNTF, "_clsReplSession::handleNotify" )
   INT32 _clsReplSession::handleNotify( NET_HANDLE handle, MsgHeader* header )
   {
      SDB_ASSERT( NULL != header, "header should not be NULL" )
      PD_TRACE_ENTRY ( SDB__CLSREPSN_HNDNTF );
      if ( CLS_SESSION_STATUS_SYNC != _status )
      {
         PD_LOG ( PDDEBUG, "Status[%d] not expected, ignore", _status,
                  CLS_SESSION_STATUS_SYNC ) ;
         goto done ;
      }

      _sendSyncReq() ;
   done:
      PD_TRACE_EXIT ( SDB__CLSREPSN_HNDNTF );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN_HNDCSTREQ, "_clsReplSession::handleConsultReq" )
   INT32 _clsReplSession::handleConsultReq( NET_HANDLE handle,
                                            MsgHeader* header )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSN_HNDCSTREQ );
      SDB_ASSERT( NULL != header, "header should not be NULL" )
      _MsgReplConsultation *msg = ( _MsgReplConsultation * )header ;
      _MsgReplConsultationRes res ;
      res.header.header.TID = msg->header.TID ;
      res.header.header.routeID = msg->header.routeID ;
      res.header.header.requestID = msg->header.requestID ;

      if ( msg->current.invalid() )
      {
         res.header.res = SDB_CLS_CONSULT_FAILED ;
         goto done ;
      }

      {
      DPS_LSN fLsn ;
      DPS_LSN mLsn ;
      DPS_LSN eLsn ;
      _logger->getLsnWindow( fLsn, mLsn, eLsn ) ;
      PD_LOG( PDEVENT, "sync: recv a consult req. [remote offset:%lld]"
              "[remote ver:%d], [local foffset:%lld][local fver:%d], "
              "[local eoffset:%lld], [local ever:%d]",
              msg->current.offset, msg->current.version,
              fLsn.offset, fLsn.version, eLsn.offset, eLsn.version ) ;

      if ( eLsn.invalid() )
      {
         res.header.res = SDB_CLS_CONSULT_FAILED ;
         goto done ;
      }
      /// remote version 
      else if ( 0 < fLsn.compare( msg->current )/* ||
                0 > eLsn.compareVersion(  msg->current.version - 1 )*/ )
      {
         res.header.res = SDB_CLS_CONSULT_FAILED ;
         goto done ;
      }
      else if ( eLsn.compareVersion ( msg->current.version ) <= 0 &&
                eLsn.compareOffset ( msg->current.offset ) < 0 )
      {
         res.header.res = SDB_OK ;
         res.returnTo = eLsn ;
         goto done ;
      }
      else
      {
         _mb.clear() ;
         DPS_LSN search = msg->current ;
         /// finally, try again.
         if ( SDB_OK == _logger->search( search, &_mb ) )
         {
            if ( ((dpsLogRecordHeader *)(_mb.offset(0)))->_version ==
                  msg->current.version )
            {
               res.header.res = SDB_OK ;
               res.returnTo = search ;
               goto done ;
            }

            /// find the same offset, we search pre from this offset.
            search.offset = ((dpsLogRecordHeader *)(_mb.offset(0)))->_preLsn ;
         }
         else
         {
            search.offset = eLsn.offset ;
         }

         DPS_LSN returnTo ;
         /// we try to find the lsn whose version is equal to
         /// remote minus one.
         do
         {
            _mb.clear() ;
            if ( SDB_OK != _logger->search( search, &_mb ) )
            {
               break ;
            }
            else
            {
               returnTo.offset = ((dpsLogRecordHeader *)(_mb.offset(0)))->_lsn ;
               returnTo.version = ((dpsLogRecordHeader *)(_mb.offset(0)))->
                                  _version ;
               search.offset = ((dpsLogRecordHeader *)(_mb.offset(0)))->_preLsn;
            }
         }while ( 0 < returnTo.compareVersion( search.version - 1 ) ) ;

         /// we do not know whether remote can rollback.
         /// but we'd better to send back.
         if ( returnTo.invalid() )
         {
            res.returnTo = fLsn ;
         }
         else
         {
            res.returnTo = returnTo ;
         }
         res.header.res = SDB_OK ;
      }
      }
   done:
      PD_LOG( PDEVENT, "sync: consult [res: %d], [return offset:%lld], "
              "[return version:%d]", res.header.res, res.returnTo.offset,
              res.returnTo.version ) ;
      _agent->syncSend( handle, &res ) ;
      PD_TRACE_EXIT ( SDB__CLSREPSN_HNDCSTREQ );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN_HNDSSTRES, "_clsReplSession::handleConsultRes" )
   INT32 _clsReplSession::handleConsultRes( NET_HANDLE handle,
                                            MsgHeader *header )
   {
      SDB_ASSERT( NULL != header, "header should not be NULL" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSN_HNDSSTRES );

      if ( CLS_SESSION_STATUS_CONSULT != _status )
      {
         PD_LOG ( PDDEBUG, "Status[%d] not expected[%d], ignore", _status,
                  CLS_SESSION_STATUS_CONSULT ) ;
         goto done ;
      }
      else if ( !_sync->isReadyToReplay() )
      {
         PD_LOG ( PDDEBUG, "Not ready to replay, ignore" ) ;
         goto done ;
      }
      else if ( header->requestID != _requestID )
      {
         PD_LOG ( PDDEBUG, "RequestID[%lld] not exptected[%lld], ignore",
                  header->requestID, _requestID ) ;
         goto done ;
      }

      {
      _MsgReplConsultationRes *msg = ( _MsgReplConsultationRes * )header ;
      if ( SDB_OK != msg->header.res )
      {
         _fullSync() ;
         goto done ;
      }
      else
      {
         PD_LOG ( PDEVENT, "repl: Consult returnTo LSN[ver:%d, offset:%lld]",
                  msg->returnTo.version, msg->returnTo.offset ) ;

         _selector.clearTime() ;
         _mb.clear() ;
         DPS_LSN curLsn = _logger->getCurrentLsn() ;

         if ( msg->returnTo.invalid() )
         {
            PD_LOG ( PDWARNING, "repl: Consult returnTo lsn is invalid" ) ;
            _fullSync() ;
            goto done ;
         }
         /// can not find rollback point, will find the consult lsn again
         if ( SDB_OK != _logger->search( msg->returnTo, &_mb ) ||
              curLsn.compare( msg->returnTo ) == 0 )
         {
            PD_LOG ( PDINFO, "repl: consultLsn[%d,%lld], curLsn[%d,%lld]",
                     _consultLsn.version, _consultLsn.offset,
                     curLsn.version, curLsn.offset ) ;

            //find the first lsn which is less than returnTo lsn
            DPS_LSN search = _consultLsn ;
            do
            {
               _mb.clear() ;
               if ( SDB_OK != _logger->search( search, &_mb ) )
               {
                  PD_LOG ( PDWARNING,
                           "No find the lsn less than(offset:%lld, version:%d)",
                           msg->returnTo.offset,
                           msg->returnTo.version ) ;
                  _fullSync () ;
                  goto done ;
               }
               _consultLsn.offset = ((dpsLogRecordHeader *)
                                      ( _mb.offset(0) ))->_lsn ;
               _consultLsn.version = ((dpsLogRecordHeader *)
                                      ( _mb.offset(0) ))->_version ;
               search.offset = ((dpsLogRecordHeader *)
                                (_mb.offset(0)))->_preLsn ;
            } while ( _consultLsn.compare( msg->returnTo ) >= 0 ) ;

            _sendConsultReq () ;
            goto done ;
         }
         else
         {
            DPS_LSN rollback = _logger->getCurrentLsn() ;
            DPS_LSN point ;
            point.offset = ((dpsLogRecordHeader *)( _mb.offset(0) ))->_lsn ;
            point.version = ((dpsLogRecordHeader *)( _mb.offset(0) ))->
                            _version ;

            if ( 0 != point.compareVersion(msg->returnTo.version ) )
            {
               _fullSync() ;
               goto done ;
            }

            /// now we are sure the point of rollback exists.
            /// begin to rollback.
            while ( TRUE )
            {
               _mb.clear() ;
               rc = _logger->search( rollback, &_mb ) ;
               SDB_ASSERT( SDB_OK == rc, "search should always be successful" )
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "sync: search  lsn failed.[%lld, %d]",
                          rollback.offset, rollback.version  ) ;
                  _fullSync() ;
                  goto done ;
               }

               if ( 0 == point.compareOffset( rollback.offset ) )
               {
                  break ;
               }
               else if ( SDB_OK != _rollback( _mb.offset( 0 ) ) )
               {
                  PD_LOG( PDERROR, "sync: rollback lsn failed.[%lld, %d]",
                          rollback.offset, rollback.version  ) ;
                  _fullSync() ;
                  goto done ;
               }
               else
               {
                  rollback.offset = ((dpsLogRecordHeader *)( _mb.offset(0) ))->
                                    _preLsn ;
               }
            }

            /// move to correct expect point.
            if ( SDB_OK != _logger->move( point.offset +
                                          ((dpsLogRecordHeader *)
                                           ( _mb.offset(0) ))->_length,
                                          point.version ) )
            {
               PD_LOG( PDERROR, "sync: rollback log failed." ) ;
               _fullSync() ;
               goto done ;
            }
            _status = CLS_SESSION_STATUS_SYNC ;
            _syncFailedNum = 0 ;
            _consultLsn.offset = DPS_INVALID_LSN_OFFSET ;
            _consultLsn.version = DPS_INVALID_LSN_VERSION ;
         }
      }
      }
   done:
      PD_TRACE_EXIT ( SDB__CLSREPSN_HNDSSTRES );
     return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN__FULLSYNC, "_clsReplSession::_fullSync" )
   void _clsReplSession::_fullSync()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSN__FULLSYNC );
      pmdEDUCB *cb = NULL ;
      clsCB *pClsCB = pmdGetKRCB()->getClsCB() ;

      if ( !isStartActive() )
      {
         goto done ;
      }

      _status = CLS_SESSION_STATUS_FULL_SYNC ;

      if ( CLS_BS_BACKUPOFFLINE == _repl->getStatus() ||
           CLS_BS_CLOSED == _repl->getStatus() )
      {
         goto done ;
      }

      SDB_ASSERT( 0 < pmdGetKRCB()->getClsCB()->getReplCB()->groupSize(),
                  "impossible" )

      // if the group size is 1, then rebuild, otherwise full sync
      if ( 1 >=  pClsCB->getReplCB()->groupSize() )
      {
         PD_LOG( PDWARNING, "group size is one, begin to rebuild database" ) ;
         pClsCB->getReplCB()->setFullSync( TRUE ) ;
         cb = SDB_OSS_NEW pmdEDUCB ( pmdGetKRCB()->getEDUMgr(),
                                     EDU_TYPE_AGENT ) ;
         if ( NULL == cb )
         {
            rc = SDB_OOM ;
            goto error ;
         }
         rc = rtnRebuildDB ( cb ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         pClsCB->getReplCB()->setFullSync( FALSE ) ;
         pmdGetStartup().ok ( TRUE ) ;
         _status = CLS_SESSION_STATUS_SYNC ;
      }
      else
      {
         _quit = TRUE ;

         pClsCB->startInnerSession( CLS_REPL, CLS_TID_REPL_FS_SYC ) ;
         pClsCB->getReplCB()->setFullSync( TRUE ) ;
         PD_LOG( PDEVENT, "sync: start the synchronization of full." ) ;
      }

   done:
      PD_TRACE_EXIT ( SDB__CLSREPSN__FULLSYNC );
      return ;
   error:
      if ( cb )
      {
         SDB_OSS_DEL cb ;
      }
      PD_LOG ( PDSEVERE, "local database rebuild failed with %d", rc ) ;
      PMD_SHUTDOWN_DB( rc ) ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN__RLBCK, "_clsReplSession::_rollback" )
   INT32 _clsReplSession::_rollback( const CHAR *log )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSN__RLBCK );
      const dpsLogRecordHeader *header = (const dpsLogRecordHeader *)log;
      PD_LOG( PDDEBUG, "begin to rollback lsn:[%lld, %d]",
              header->_lsn, header->_version ) ;
      rc = _replayer.rollback( header, eduCB() ) ;
   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSN__RLBCK, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN__SNDVSYNCREQ, "_clsReplSession::_sendVirSyncReq" )
   void _clsReplSession::_sendVirSyncReq()
   {
      PD_TRACE_ENTRY( SDB__CLSREPSN__SNDSYNCREQ ) ;

      _syncSrc = _selector.selected() ;
      if ( MSG_INVALID_ROUTEID != _syncSrc.value )
      {
         _MsgReplVirSyncReq msg ;
         msg.header.TID = CLS_TID( _sessionID ) ;
         msg.next = _logger->expectLsn() ;
         msg.from = _agent->localID() ;
         _agent->syncSend( _syncSrc, &msg ) ;
         PD_LOG( PDDEBUG, "sync: send vir sync req to [node: %d] [group:%d]"
                 "lsn: [%lld][%d]",
                 _syncSrc.columns.nodeID, _syncSrc.columns.groupID,
                 msg.next.offset, msg.next.version ) ;
      }
      PD_TRACE_EXIT( SDB__CLSREPSN__SNDSYNCREQ ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN__SNDSYNCREQ, "_clsReplSession::_sendSyncReq" )
   void _clsReplSession::_sendSyncReq( DPS_LSN *pCompleteLSN )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSN__SNDSYNCREQ );

      _syncSrc = _selector.selected() ;

      if ( MSG_INVALID_ROUTEID != _syncSrc.value )
      {
         _timeout = 0 ;

         _MsgReplSyncReq msg ;
         msg.header.TID = CLS_TID( _sessionID ) ;
         msg.header.requestID = ++_requestID ;
         msg.next = _logger->expectLsn() ;
         msg.needData = ( CLS_BS_NORMAL == _repl->getStatus() ) ? 1 : 0 ;

         /// when lsn is not specified we set complete with expected.
         if ( pCompleteLSN )
         {
            _completeLSN = *pCompleteLSN ;
            msg.completeNext = _completeLSN ;
         }
         else if ( _pReplBucket->maxReplSync() > 0 )
         {
            _completeLSN = _pReplBucket->completeLSN() ;
            if ( !_completeLSN.invalid() )
            {
               msg.completeNext = _completeLSN ;
            }
         }

         msg.identity = _agent->localID() ;
         _agent->syncSend( _syncSrc, &msg ) ;
         PD_LOG( PDDEBUG, "sync: send sync req to [node: %d] [group:%d]"
                 "lsn: [%llu][%u], complete lsn: [%lld][%u]",
                 _syncSrc.columns.nodeID, _syncSrc.columns.groupID,
                 msg.next.offset, msg.next.version, msg.completeNext.offset,
                 msg.completeNext.version ) ;
      }

      PD_TRACE_EXIT ( SDB__CLSREPSN__SNDSYNCREQ ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN__SNDCSTREQ, "_clsReplSession::_sendConsultReq" )
   void _clsReplSession::_sendConsultReq()
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSN__SNDCSTREQ );

      if ( CLS_SESSION_STATUS_CONSULT != _status )
      {
         _status = CLS_SESSION_STATUS_CONSULT ;
      }

      if ( !_pReplBucket->isEmpty() )
      {
         PD_LOG( PDDEBUG, "Repl bucket is not empty, size: %d, can't send "
                 "consult req", _pReplBucket->size() ) ;
         goto done ;
      }

      if ( _consultLsn.invalid () )
      {
         _consultLsn = _logger->getCurrentLsn() ;
      }

      _syncSrc = _selector.selected() ;
      if ( MSG_INVALID_ROUTEID != _syncSrc.value )
      {
         _timeout = 0 ;

         _MsgReplConsultation msg ;
         msg.header.TID = CLS_TID( _sessionID ) ;
         msg.header.requestID = ++_requestID ;
         msg.current =  _consultLsn ;
         msg.identity = _agent->localID() ;
         _agent->syncSend( _syncSrc, &msg ) ;
         PD_LOG( PDEVENT, "sync: send consult req to [node: %d] [group:%d]"
                 "[ver:%d] [offset:%lld]",
                 _syncSrc.columns.nodeID, _syncSrc.columns.groupID,
                 msg.current.version,  msg.current.offset ) ;
      }

   done:
      PD_TRACE_EXIT ( SDB__CLSREPSN__SNDCSTREQ );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN__REPLG, "_clsReplSession::_replayLog" )
   INT32 _clsReplSession::_replayLog( const CHAR *logs, const UINT32 &len,
                                      UINT32 &num )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSN__REPLG ) ;

      dpsLogRecordHeader *recordHeader = NULL ;
      const CHAR *log = logs ;
      num = 0 ;
      BOOLEAN needRollback = FALSE ;

      while ( log < logs + len )
      {
         if ( eduCB()->isInterrupted() )
         {
            PD_LOG ( PDEVENT, "Session[%s] replayLog is interrupted",
                     sessionName() ) ;
            break ;
         }

         recordHeader = (dpsLogRecordHeader *)log ;
         needRollback = FALSE ;

         PD_LOG( PDDEBUG, "sync: replay record [lsn:%lld] [version:%d][len:%d]"
                 "[preLsn:%lld]", recordHeader->_lsn, recordHeader->_version,
                 recordHeader->_length, recordHeader->_preLsn ) ;

#ifdef _DEBUG
         DPS_LSN lsn = _logger->expectLsn() ;
         if ( 0 != lsn.compareOffset( recordHeader->_lsn ) ||
              0 < lsn.compareVersion( recordHeader->_version ) )
         {
            PD_LOG ( PDWARNING, "repl: replayLog, cur lsn[%d,%lld] can't fit "
                     "expect lsn[%d,%lld]", recordHeader->_version,
                     recordHeader->_lsn, lsn.version, lsn.offset ) ;
            rc = SDB_CLS_REPLAY_LOG_FAILED ;
            goto error ;
         }
#endif
         rc = _replay( recordHeader ) ;
         SDB_ASSERT( SDB_OK == rc, "must be ok" )
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "sync: failed to replay log, rc: %d", rc ) ;
            goto error ;
         }

         rc = _logger->recordRow( log, recordHeader->_length ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "sync: row record failed[rc:%d]", rc ) ;
            needRollback = TRUE ;
            goto error ;
         }

         log += recordHeader->_length ;
         ++num ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSN__REPLG, rc );
      return rc ;
   error:
      if ( _pReplBucket->waitEmptyAndRollback() )
      {
         DPS_LSN expectLSN = _pReplBucket->completeLSN() ;
         rc = _logger->move( expectLSN.offset, expectLSN.version ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to move lsn to [%u, %llu], rc: %d",
                    expectLSN.version, expectLSN.offset ) ;
            _fullSync() ;
         }
         else
         {
            PD_LOG( PDEVENT, "Move lsn to[%u, %llu]", expectLSN.version,
                    expectLSN.offset ) ;
         }
      }
      else if ( needRollback )
      {
         rc = _rollback( log ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to rollback[%lld, type: %d], rc: %d",
                    recordHeader->_lsn, recordHeader->_type, rc ) ;
            _fullSync() ;
         }
      }
      goto done ;
   }

   INT32 _clsReplSession::_replay( dpsLogRecordHeader *header )
   {
      if ( _pReplBucket->maxReplSync() > 0 )
      {
         return _replayer.replayByBucket( header, eduCB(), _pReplBucket ) ;
      }
      else
      {
         return _replayer.replay( header, eduCB() ) ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSN__SYNCLOG, "_clsReplSession::_syncLog" )
   INT32 _clsReplSession::_syncLog( const NET_HANDLE &handle,
                                    const MsgReplSyncReq *req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSN__SYNCLOG );
      time_t bTime = time( NULL ) ;
      MsgReplSyncRes msg ;
      if ( DPS_INVALID_LSN_OFFSET == req->next.offset )
      {
         rc = SDB_CLS_SYNC_FAILED ;
         goto done ;
      }

      //if don't know who is primary node, don't reply
      if ( MSG_INVALID_ROUTEID == _repl->getPrimary().value )
      {
         PD_LOG ( PDINFO, "Don't know who is primary node, not reply" ) ;
         goto done ;
      }

      /// set remote routeID especially, to find remote session.
      msg.oldestTransLsn = pmdGetKRCB()->getTransCB()->getOldestBeginLsn();
      msg.header.header.routeID = req->header.routeID ;
      msg.header.header.TID = req->header.TID ;
      msg.header.header.requestID = req->header.requestID ;
      msg.identity = _agent->localID() ;

      {
      DPS_LSN fLsn ;
      DPS_LSN mLsn ;
      DPS_LSN eLsn ;
      DPS_LSN expect ;
      DPS_LSN search = req->next ;

      _logger->getLsnWindow( fLsn, mLsn, eLsn, &expect ) ;

      if ( 0 < fLsn.compareOffset( req->next.offset ) )
      {
         PD_LOG( PDWARNING, "sync: remote lsn is too old. remote [offset:%lld]"
                 "[version:%d], local [fLsn offset:%lld]"
                 "[fLsn version:%d] [eLsn offset:%lld][eLsn version:%d]",
                 req->next.offset,
                 req->next.version, fLsn.offset, fLsn.version,
                 eLsn.offset, eLsn.version ) ;
         msg.header.res = SDB_CLS_SYNC_FAILED ;
         _agent->syncSend( handle, &msg ) ;
         rc = SDB_CLS_SYNC_FAILED ;
         goto done ;
      }
      else if ( 0 > eLsn.compare( req->next ) )
      {
         rc = SDB_OK ;
         if ( pmdGetKRCB()->getClsCB()->isPrimary() )
         {
            if (  0 != expect.compareOffset( req->next.offset ) ||
                  0 > expect.compareVersion( req->next.version ) )
            {
               msg.header.res = SDB_CLS_SYNC_FAILED ;
               rc = SDB_CLS_SYNC_FAILED ;
               PD_LOG( PDWARNING, "sync: remote lsn is not match local."
                       "[remote offset:%lld][remote ver:%d][end offset:%lld]"
                       "[end version:%d][expect offset:%lld]"
                       "[expect version:%d]", req->next.offset,
                       req->next.version, eLsn.offset,
                       eLsn.version, expect.offset, expect.version ) ;
            }
            else
            {
               PD_LOG( PDDEBUG, "sync: local has no more new data." ) ;
               msg.header.res = SDB_OK ;
               rc = SDB_OK ;
            }
         }
         else
         {
            PD_LOG( PDDEBUG, "sync: local has no more new data." ) ;
            msg.header.res = SDB_OK ;
            rc = SDB_OK ;
         }
         _agent->syncSend( handle, &msg ) ;
         goto done ;
      }
      else if ( 0 == req->needData )
      {
         msg.header.res = SDB_OK ;
         _agent->syncSend( handle, &msg ) ;
         goto done ;
      }
      else
      {
         PD_LOG( PDDEBUG, "sync: begin to find log. remote [offset:%lld]"
                 "[version:%d], local [fLsn offset:%lld]"
                 "[fLsn version:%d] [eLsn offset:%lld][eLsn version:%d]",
                 req->next.offset, req->next.version, fLsn.offset,
                 fLsn.version, eLsn.offset, eLsn.version ) ;
         _mb.clear() ;
         while ( TRUE )
         {
            rc = _logger->search( search, &_mb ) ;
            if ( SDB_OK != rc )
            {
               break ;
            }
            else
            {
               dpsLogRecordHeader *header = ( dpsLogRecordHeader * )
                                            _mb.readPtr() ;
               _mb.readPtr( _mb.length() ) ;
               if ( CLS_SYNC_MAX_LEN <= _mb.length() ||
                    ( time( NULL ) - bTime >= CLS_REPL_MAX_TIME &&
                      _mb.length() > 0 ) )
               {
                  break ;
               }
               else
               {
                  search.offset += header->_length ;
               }
             }
         }
      }

      msg.header.header.messageLength = sizeof( _MsgReplSyncRes) +
                                           _mb.length() ;

      if ( 0 != _mb.length() )
      {
         rc = SDB_OK ;
         msg.header.res = SDB_OK ;
         _agent->syncSend( handle, ( MsgHeader *)(&msg),
                           _mb.offset( 0 ), _mb.length() ) ;
      }
      else
      {
         rc = SDB_CLS_SYNC_FAILED ;
         msg.header.res = SDB_CLS_SYNC_FAILED ;
         PD_LOG( PDWARNING, "sync: can not find [ver:%d][offset:%lld]",
                 search.version, search.offset ) ;
         _agent->syncSend( handle, &msg ) ;
      }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSN__SYNCLOG, rc );
      return rc ;
   }

}
