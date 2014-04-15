/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsReplicateSet.hpp

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

#include "clsReplicateSet.hpp"
#include "netRouteAgent.hpp"
#include "clsUtil.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "clsMgr.hpp"
#include "clsFSSrcSession.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{

   BEGIN_OBJ_MSG_MAP( _clsReplicateSet, _clsObjBase )
      //ON_MSG ( )
      ON_MSG( MSG_CAT_GRP_RES, handleMsg )
      ON_MSG( MSG_CLS_BEAT, handleMsg )
      ON_MSG( MSG_CLS_BEAT_RES, handleMsg )
      ON_MSG( MSG_CLS_BALLOT, handleMsg )
      ON_MSG( MSG_CLS_BALLOT_RES, handleMsg )
      ON_MSG( MSG_CAT_PAIMARY_CHANGE_RES, handleMsg )
      ON_MSG( MSG_CLS_GINFO_UPDATED, handleMsg )
   END_OBJ_MSG_MAP ()

   const UINT32 CLS_REPL_SEC_TIME = 1000 ;
   UINT32 CLS_SHARING_BRK_TIME = 0 ;


   #define CLS_REPL_ACTIVE_CHECK( rc ) \
           if ( !_active ) \
           { \
              rc = SDB_REPL_GROUP_NOT_ACTIVE ;\
              goto error ;\
           }

#if defined (_WINDOWS)
   #define CLS_CONNREFUSED    WSAECONNREFUSED
#else
   #define CLS_CONNREFUSED    ECONNREFUSED
#endif //_WINDOWS

   #define CLS_SYNCCTRL_BASE_TIME         (10)

   _clsReplicateSet::_clsReplicateSet( _netRouteAgent *agent ):
                                    _agent( agent ),
                                    _vote( &_info, _agent),
                                    _logger( NULL ),
                                    _sync( _agent, &_info ),
                                    _clsCB( NULL ),
                                    _timerID( CLS_INVALID_TIMERID ),
                                    _beatTime( 0 ),
                                    _downloadTime( 0 ),
                                    _active( FALSE ),
                                    _replStatus( CLS_BS_NORMAL )
   {
      _srcSessionNum = 0 ;
      _prevPrimary = FALSE ;

      _totalLogSize = 0 ;
      _inSyncCtrl   = FALSE ;
      memset( _sizethreshold, 0, sizeof( _sizethreshold ) ) ;
      memset( _timeThreshold, 0, sizeof( _timeThreshold ) ) ;
   }

   _clsReplicateSet::~_clsReplicateSet()
   {

   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPPSET_NOTIFY, "_clsReplicateSet::notify" )
   void _clsReplicateSet::notify ( UINT32 suLID, UINT32 clLID,
                                   dmsExtentID extLID,
                                   const DPS_LSN_OFFSET &offset )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPPSET_NOTIFY );

      _sync.notify( offset ) ;

      PD_TRACE_EXIT ( SDB__CLSREPPSET_NOTIFY );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPPSET_NOTIFY2SESSION, "_clsReplicateSet::notify2Session" )
   void _clsReplicateSet::notify2Session( UINT32 suLID, UINT32 clLID,
                                          dmsExtentID extLID,
                                          const DPS_LSN_OFFSET & offset )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPPSET_NOTIFY2SESSION );
      // the src session is not empty, should notify every one
      if ( _srcSessionNum > 0 )
      {
         UINT32 index = 0 ;
         _vecLatch.lock_r () ;
         while ( index < _srcSessionNum )
         {
            _vecSrcSessions[index]->notifyLSN ( suLID, clLID, extLID, offset ) ;
            ++index ;
         }
         _vecLatch.release_r () ;
      }
      PD_TRACE_EXIT ( SDB__CLSREPPSET_NOTIFY2SESSION );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET_REGSN, "_clsReplicateSet::regSession" )
   void _clsReplicateSet::regSession ( _clsDataSrcBaseSession * pSession )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSET_REGSN );
      SDB_ASSERT ( pSession, "Session can't be null" ) ;

      _vecLatch.lock_w () ;
      _srcSessionNum++ ;
      _vecSrcSessions.push_back ( pSession ) ;
      _vecLatch.release_w () ;
      PD_TRACE_EXIT ( SDB__CLSREPSET_REGSN );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET_UNREGSN, "_clsReplicateSet::unregSession" )
   void _clsReplicateSet::unregSession ( _clsDataSrcBaseSession * pSession )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSET_UNREGSN );
      SDB_ASSERT ( pSession, "Session can't be null" ) ;

      _vecLatch.lock_w () ;
      std::vector<_clsDataSrcBaseSession*>::iterator it =
         _vecSrcSessions.begin() ;
      while ( it != _vecSrcSessions.end() )
      {
         if ( *it == pSession )
         {
            _vecSrcSessions.erase ( it ) ;
            _srcSessionNum-- ;
            break ;
         }
         ++it ;
      }
      _vecLatch.release_w () ;
      PD_TRACE_EXIT ( SDB__CLSREPSET_UNREGSN );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET_INIT, "_clsReplicateSet::initialize" )
   INT32 _clsReplicateSet::initialize ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSET_INIT );

      _logger = pmdGetKRCB()->getDPSCB() ;
      _clsCB = pmdGetKRCB()->getClsCB() ;
      SDB_ASSERT( NULL != _logger, "logger should not be NULL" ) ;

      rc = _replBucket.init() ;
      PD_RC_CHECK( rc, PDERROR, "Init repl bucket failed, rc: %d", rc ) ;

      _totalLogSize = (UINT64)_logger->getLogFileSz() *
                      (UINT64)_logger->getLogFileNum() ;
      // init sync control param
      {
         UINT32 rate = 2 ;
         UINT32 timeBase = CLS_SYNCCTRL_BASE_TIME ;

         for ( UINT32 idx = 0 ; idx < CLS_SYNCCTRL_THRESHOLD_SIZE ; ++idx )
         {
            rate = 2 << idx ;
            _sizethreshold[ idx ] = _totalLogSize * ( rate - 1 ) / rate ;
            _timeThreshold[ idx ] = timeBase << idx ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSET_INIT, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsReplicateSet::final ()
   {
      _replBucket.fini() ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET_ACTIVE, "_clsReplicateSet::active" )
   INT32 _clsReplicateSet::active()
   {
      INT32 rc      = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSET_ACTIVE );
      if ( _active )
      {
         goto done ;
      }

      {
         _MsgRouteID id = _clsCB->getNodeID() ;
         id.columns.serviceID = _clsCB->getReplServiceID() ;
         setLocalID( id ) ;
         _MsgCatGroupReq msg ;
         msg.id = _info.local ;
         _cata.call( (MsgHeader *)(&msg) ) ;
         _timerID = _clsCB->setTimer( CLS_REPL, CLS_REPL_SEC_TIME ) ;
      }
   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSET_ACTIVE, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET__SETGPSET, "_clsReplicateSet::_setGroupSet" )
   INT32 _clsReplicateSet::_setGroupSet( const CLS_GROUP_VERSION &version,
                                         map<UINT64, _netRouteNode> &nodes )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__CLSREPSET__SETGPSET );
      if ( version <= _info.version )
      {
         rc = SDB_REPL_REMOTE_G_V_EXPIRED ;
         goto error ;
      }

      if ( CLS_REPLSET_MAX_NODE_SIZE  < nodes.size() )
      {
         rc = SDB_CLS_INVALID_GROUP_NUM ;
         PD_LOG( PDWARNING, "invalid group size : %d",
                 nodes.size() ) ;
         goto error ;
      }

      _info.version = version ;

      {
      /// update new nodes, include the node with
      /// same id but different address
      BOOLEAN hasLocal = FALSE ;
      map<UINT64, _netRouteNode>::iterator itr =
                                          nodes.begin() ;
      for ( ; itr != nodes.end(); itr++ )
      {
        if ( itr->first == _info.local.value )
        {
           hasLocal = TRUE ;
           continue ;
        }
        if ( SDB_OK == _agent->updateRoute( itr->second._id,
                                            itr->second ) )
        {
           _clsGroupBeat &beat = (_info.info[itr->first]).beat ;
           beat.identity = itr->second._id ;
           beat.beatID = 0 ;
           /// we alive the changed node here. if it is unnormal,
           /// break it out later.
           _alive( itr->second._id ) ;
           PD_LOG( PDEVENT, "add node [%s:%s]",
                   itr->second._host, itr->second._service[0].c_str() ) ;
        }
      } // for ( ; itr != nodes.end(); itr++ )

      if ( !hasLocal )
      {
         PD_LOG( PDERROR, "local node is not in the cluster!" ) ;
         PMD_SHUTDOWN_DB( SDB_SYS ) ;
         goto done ;
      }
      }

      {
      /// remove deleted nodes
      map<UINT64, _clsSharingStatus>::iterator itr2 =
                                          _info.info.begin() ;
      for ( ; itr2 != _info.info.end(); )
      {
         if ( nodes.end() == nodes.find( itr2->first ) )
         {
            /// if primary is deleted, set primary invalid
            if ( itr2->first == _info.primary.value )
            {
               _info.primary.value = 0 ;
            }
            MsgRouteID tmp ;
            tmp.value = itr2->first ;
            PD_LOG( PDEVENT, "erase node[%d,%d]",
                    tmp.columns.groupID, tmp.columns.nodeID ) ;
            _info.mtx.lock_w() ;
            _info.alives.erase( itr2->first ) ;
            _info.mtx.release_w() ;
            _info.info.erase( itr2++ ) ;
         }
         else
         {
            ++itr2 ;
         }
      } // for ( ; itr2 != _info.info.end(); itr2++ )
      }

      _sync.updateNotifyList( TRUE ) ;
   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSET__SETGPSET, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsReplicateSet::callCatalog( MsgHeader *header )
   {
      return _cata.call( header ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET_GETPRMY, "_clsReplicateSet::getPrimary" )
   _MsgRouteID _clsReplicateSet::getPrimary ()
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSET_GETPRMY );
      _MsgRouteID primary ;
      _info.mtx.lock_r () ;
      primary = _info.primary ;
      _info.mtx.release_r () ;

      PD_TRACE_EXIT ( SDB__CLSREPSET_GETPRMY );
      return primary ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET_GETGPINFO, "_clsReplicateSet::getGroupInfo" )
   void _clsReplicateSet::getGroupInfo( _MsgRouteID &primary,
                                        vector<_netRouteNode> &group )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSET_GETGPINFO );
      map<UINT64, _clsSharingStatus>::const_iterator itr =
                                          _info.info.begin() ;
      INT32 rc = SDB_OK ;
      _netRouteNode node ;
      _MsgRouteID id ;
      _info.mtx.lock_r() ;
      primary = _info.primary ;
      _info.mtx.release_r() ;
      for ( ; itr != _info.info.end(); itr++ )
      {
         id.value = itr->first ;
         rc = _agent->route( id, node ) ;
         SDB_ASSERT( SDB_OK == rc, "impossible" )
         if ( SDB_OK == rc )
         {
            group.push_back( node ) ;
         }
         else
         {
            PD_LOG( PDERROR, "group info is not match route table." ) ;
         }
      }
      id = _info.local ;
      rc = _agent->route( id, node ) ;
      if ( SDB_OK == rc )
      {
         group.push_back( node ) ;
      }
      else
      {
         PD_LOG( PDERROR, "group info is not match route table." ) ;
         SDB_ASSERT( false, "impossible" )
      }
      PD_TRACE_EXIT ( SDB__CLSREPSET_GETGPINFO );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET_ONTMR, "_clsReplicateSet::onTimer" )
   void _clsReplicateSet::onTimer( UINT64 timerID, UINT32 interval )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSET_ONTMR );
      if ( _timerID == timerID )
      {
         _checkPrimary () ;

         _cata.handleTimeout( interval ) ;
         if ( !_active )
         {
            goto done ;
         }
         _beatTime += interval ;
         if ( CLS_SHARING_BETA_INTERVAL <= _beatTime )
         {
            _sharingBeat() ;
            _beatTime = 0 ;
         }

         _checkBreak( interval ) ;

         _vote.handleTimeout( interval ) ;
      }

   done:
      PD_TRACE_EXIT ( SDB__CLSREPSET_ONTMR );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET_HNDMSG, "_clsReplicateSet::handleMsg" )
   INT32 _clsReplicateSet::handleMsg( NET_HANDLE handle, MsgHeader* msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSET_HNDMSG );
      switch ( msg->opCode )
      {
         case MSG_CAT_GRP_RES:
         {
            rc = _handleGroupRes( (const _MsgCatGroupRes *)msg ) ;
            break ;
         }
         case MSG_CLS_BEAT :
         {
            CLS_REPL_ACTIVE_CHECK( rc )
            rc = _handleSharingBeat( ( const _MsgClsBeat *)msg ) ;
            break ;
         }
         case MSG_CAT_PAIMARY_CHANGE_RES:
         {
            MsgInternalReplyHeader *changeRes = (MsgInternalReplyHeader *)msg ;
            if ( SDB_CLS_NOT_PRIMARY == changeRes->res )
            {
               _clsCB->updateCatGroup ( TRUE ) ;
            }
            else if ( SDB_OK == changeRes->res )
            {
               _cata.remove( ( _MsgInternalReplyHeader *)msg ) ;
            }
            break ;
         }
         case MSG_CLS_BEAT_RES :
         {
            CLS_REPL_ACTIVE_CHECK( rc )
            rc = _handleSharingBeatRes( ( const _MsgClsBeatRes *)msg ) ;
            break ;
         }
         case MSG_CLS_BALLOT :
         {
            CLS_REPL_ACTIVE_CHECK( rc )
            rc = _vote.handleInput( msg ) ;
            break ;
         }
         case MSG_CLS_BALLOT_RES :
         {
            CLS_REPL_ACTIVE_CHECK( rc )
            rc = _vote.handleInput( msg ) ;
            break ;
         }
         case MSG_CLS_GINFO_UPDATED :
         {
            PD_LOG( PDEVENT,
                    "group info has been updated, download again." ) ;
            MsgCatGroupReq msg ;
            msg.id = _info.local ;
            _cata.call( (MsgHeader *)(&msg) ) ;
            break ;
         }
         default :
         {
            PD_LOG( PDWARNING, "unknown msg %d", msg->opCode ) ;
         }
      }
   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSET_HNDMSG, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET__HNDGPRES, "_clsReplicateSet::_handleGroupRes" )
   INT32 _clsReplicateSet::_handleGroupRes( const _MsgCatGroupRes *msg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSET__HNDGPRES );

      if ( SDB_OK != msg->header.res )
      {
         if ( SDB_CLS_NOT_PRIMARY == msg->header.res )
         {
            _clsCB->updateCatGroup ( TRUE ) ;
         }

         PD_LOG( PDERROR, "download group info request was refused" ) ;
         goto error ;
      }
      {
      CLS_GROUP_VERSION version ;
      map<UINT64, _netRouteNode> group ;
      string groupName ;
      rc = msgParseCatGroupRes( msg, version, groupName, group ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDWARNING, "parse _MsgCatGroupRes err, rc = %d", rc ) ;
         goto error ;
      }

      rc = _setGroupSet( version, group ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDWARNING, "set group info failed, rc = %d", rc ) ;
         goto error ;
      }

      _cata.remove( (MsgInternalReplyHeader *)msg ) ;

      pmdGetKRCB()->setGroupName ( groupName.c_str() ) ;
      if ( !_active )
      {
         PD_LOG( PDEVENT, "download group info successfully" ) ;

         //start repl sync session
         _clsCB->startInnerSession ( CLS_REPL, CLS_TID_REPL_SYC ) ;

         _active = TRUE ;
         _vote.init() ;
      }
      }
   done :
      PD_TRACE_EXITRC ( SDB__CLSREPSET__HNDGPRES, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET__SHRBEAT, "_clsReplicateSet::_sharingBeat" )
   void _clsReplicateSet::_sharingBeat()
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSET__SHRBEAT );
      if ( _info.info.empty() )
      {
         goto done ;
      }
      {
      DPS_LSN fBegin ;
      DPS_LSN mBegin ;
      DPS_LSN end ;
      _logger->getLsnWindow( fBegin, mBegin, end ) ;
      _MsgClsBeat msg ;
      msg.beat.identity = _info.local ;
      msg.beat.endLsn = end ;
      msg.beat.version = _info.version ;
      msg.beat.role = _vote.primaryIsMe() ?
                      CLS_GROUP_ROLE_PRIMARY : CLS_GROUP_ROLE_SECONDARY ;
      msg.beat.beatID = ++_info.localBeatID ;
      map<UINT64, _clsSharingStatus>::iterator itr =
                                        _info.info.begin() ;
      for ( ; itr != _info.info.end(); itr++ )
      {
         if ( itr->second.timeout >= CLS_SHARING_BRK_TIME &&
              itr->second.timeout >= _beatTime )
         {
            itr->second.timeout -= _beatTime ;
            continue ;
         }
         msg.beat.syncStatus = clsSyncWindow( itr->second.beat.endLsn,
                                              fBegin, mBegin, end ) ;
         if ( SDB_OK != _agent->syncSend( itr->second.beat.identity, &msg ) &&
              _info.alives.find( itr->first ) == _info.alives.end() )
         {
            UINT32 resetTimeout = 0 ;
            itr->second.timeout = CLS_SHARING_BRK_TIME - 1 ;
            if ( SOCKET_GETLASTERROR == CLS_CONNREFUSED )
            {
               resetTimeout = 1800 * OSS_ONE_SEC ;
            }
            else
            {
               resetTimeout = 120 * OSS_ONE_SEC ;
            }
            itr->second.timeout += resetTimeout ;

            PD_LOG( PDEVENT, "Reset node[%d] sharing-beat time to %u(sec)",
                    itr->second.beat.identity.columns.nodeID,
                    resetTimeout / OSS_ONE_SEC ) ;
         }
      }
      }
   done:
      PD_TRACE_EXIT ( SDB__CLSREPSET__SHRBEAT );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET__CHKPRM, "_clsReplicateSet::_checkPrimary" )
   void _clsReplicateSet::_checkPrimary ()
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSET__CHKPRM );
      // change to primary
      if ( !_prevPrimary && primaryIsMe () )
      {
         _prevPrimary = TRUE ;
         // start task query
         _clsCB->_onPrimaryChange( _prevPrimary ) ;
      }
      // change to secondary
      else if ( _prevPrimary && !primaryIsMe () )
      {
         _prevPrimary = FALSE ;
         _clsCB->_onPrimaryChange ( _prevPrimary ) ;
      }
      PD_TRACE_EXIT ( SDB__CLSREPSET__CHKPRM );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET__CHKBRK, "_clsReplicateSet::_checkBreak" )
   void _clsReplicateSet::_checkBreak( const UINT32 &millisec )
   {
      /// avoid the use of w lock. only find item need to be
      /// erase, we lock w. here we think that no need to lock
      /// w when change value
      PD_TRACE_ENTRY ( SDB__CLSREPSET__CHKBRK );
      BOOLEAN needErase = FALSE ;
      map<UINT64, _clsSharingStatus *>::iterator itr =
                                        _info.alives.begin() ;
      for ( ; itr != _info.alives.end(); itr++ )
      {
         itr->second->timeout += millisec ;
         if ( CLS_SHARING_BRK_TIME <= itr->second->timeout )
         {
            needErase = TRUE ;
         }
      }

      if ( !needErase )
      {
         goto done ;
      }

      _info.mtx.lock_w() ;
      itr = _info.alives.begin() ;
      for ( ; itr != _info.alives.end(); )
      {
         if ( CLS_SHARING_BRK_TIME <= itr->second->timeout )
         {
            if ( itr->first == _info.primary.value )
            {
               PD_LOG( PDERROR, "vote: primary [node:%d] break",
                       _info.primary.columns.nodeID ) ;
               _info.primary.value = MSG_INVALID_ROUTEID ;
            }
            PD_LOG( PDERROR, "vote: [node:%d] alive break",
                        itr->second->beat.identity.columns.nodeID ) ;
            itr->second->beat.beatID = 0 ;

            _sync.updateNodeStatus( itr->second->beat.identity, FALSE ) ;

            _info.alives.erase( itr++ ) ;
         }
         else
         {
            ++itr ;
         }
      }
      _info.mtx.release_w() ;
      /// cutting when down to secandary is in _clsVSPrimary.
      if ( _vote.primaryIsMe() )
      {
         _sync.cut( _info.alives.size() ) ;
      }

   done:
      PD_TRACE_EXIT ( SDB__CLSREPSET__CHKBRK );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSREPSET__HNDSHRBEAT, "_clsReplicateSet::_handleSharingBeat" )
   INT32 _clsReplicateSet::_handleSharingBeat( const _MsgClsBeat *msg )
   {
      SDB_ASSERT( NULL != msg, "msg should not be NULL" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSET__HNDSHRBEAT );
      const _clsGroupBeat &beat = msg->beat ;
      map<UINT64, _clsSharingStatus>::iterator itr=
                     _info.info.find( beat.identity.value ) ;
      if ( _info.info.end() == itr && beat.version <= _info.version )
      {
         rc = SDB_REPL_INVALID_GROUP_MEMBER ;
         goto error ;
      }

      if ( beat.version > _info.version )
      {
         rc = SDB_REPL_LOCAL_G_V_EXPIRED ;
         //download ;
         _MsgCatGroupReq msg ;
         msg.id = _info.local ;
         _cata.call( (MsgHeader *)(&msg) ) ;
      }
      else if ( itr != _info.info.end() )
      {
         itr->second.beat = beat ;
         if ( CLS_GROUP_ROLE_PRIMARY == beat.role )
         {
            g_startShiftTime = -1 ; // have primary node

            if ( _vote.primaryIsMe() )
            {
               DPS_LSN lsn  = _logger->getCurrentLsn() ;
               if ( 0 >= lsn.compare( beat.endLsn ) )
               {
                //  notifyLosePrimary() ;
                  _info.mtx.lock_w() ;
                  _info.primary = beat.identity ;
                  _info.mtx.release_w() ;
                  _vote.force( CLS_ELECTION_STATUS_SILENCE ) ;
                  PD_LOG( PDEVENT, "vote:remote lsn[%d:%lld]"
                          " higher(or equal) than local lsn[%d:%lld],"
                          " we change to secondary.",
                          beat.endLsn.version, beat.endLsn.offset,
                          lsn.version, lsn.offset ) ;
               }
            }
            else if ( _info.primary.value != beat.identity.value )
            {
               PD_LOG( PDEVENT, "vote: the discovery of new primary[%d]",
                                 beat.identity.columns.nodeID ) ;
               _cata.remove( MSG_CAT_PAIMARY_CHANGE_RES ) ;
               _vote.force( CLS_ELECTION_STATUS_SILENCE ) ;
               _info.mtx.lock_w() ;
               _info.primary = beat.identity ;
               _info.mtx.release_w() ;
            }
         }
         else
         {
            if ( _info.primary.value == beat.identity.value )
            {
               _cata.remove( MSG_CAT_PAIMARY_CHANGE_RES ) ;
               _info.mtx.lock_w() ;
               _info.primary.value = MSG_INVALID_ROUTEID ;
               _info.mtx.release_w() ;
            }
         }
      }
      {
         _alive( beat.identity ) ;
         _MsgClsBeatRes res ;
         res.identity = _info.local ;
         _agent->syncSend( msg->header.routeID, &res ) ;
      }
   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSET__HNDSHRBEAT, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsReplicateSet::_handleSharingBeatRes( const _MsgClsBeatRes *msg )
   {
      SDB_ASSERT( NULL != msg, "msg should not be NULL" )
      return _alive( msg->identity ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION (SDB__CLSREPSET__ALIVE, "_clsReplicateSet::_alive" )
   INT32 _clsReplicateSet::_alive( const _MsgRouteID &id )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSREPSET__ALIVE );
      map<UINT64, _clsSharingStatus>::iterator itr=
                     _info.info.find( id.value ) ;
      if ( _info.info.end() == itr )
      {
         rc = SDB_REPL_INVALID_GROUP_MEMBER ;
         goto error ;
      }
      if ( _info.alives.end() == _info.alives.find( itr->first ) )
      {
         _clsSharingStatus &status = itr->second ;
         _info.mtx.lock_w() ;
         _info.alives.insert( make_pair( itr->first, &status ) ) ;
         _info.mtx.release_w() ;

         _sync.updateNodeStatus( status.beat.identity, TRUE ) ;

         PD_LOG( PDEVENT, "vote: [node:%d] aliving from break",
                 status.beat.identity.columns.nodeID ) ;
      }
      itr->second.timeout = 0 ;
   done:
      PD_TRACE_EXITRC ( SDB__CLSREPSET__ALIVE, rc );
      return rc ;
   error:
      goto done ;
   }

   UINT32 _clsReplicateSet::_getThresholdTime( UINT64 diffSize )
   {
      UINT32 i = 0 ;
      UINT32 threshTime = 0 ;

      for ( ; i < CLS_SYNCCTRL_THRESHOLD_SIZE ; ++i )
      {
         if ( diffSize < _sizethreshold[ i ] )
         {
            break ;
         }
      }
      if ( i > 1 || ( 1 == i && _inSyncCtrl ) )
      {
         threshTime = _timeThreshold[ i - 1 ] ;
      }
      return threshTime ;
   }

   // PD_TRACE_DECLARE_FUNCTION (SDB__CLSREPSET__CHECKSYNCCTRL, "_clsReplicateSet::checkSyncControl" )
   INT32 _clsReplicateSet::checkSyncControl( UINT32 reqLen, pmdEDUCB *cb )
   {
      PD_TRACE_ENTRY ( SDB__CLSREPSET__CHECKSYNCCTRL );
      INT32 rc = SDB_OK ;

      UINT32 threshTime = 0 ;
      UINT32 waitTime = 0 ;
      DPS_LSN_OFFSET offset = _sync.getSyncCtrlArbitLSN() ;

      if ( DPS_INVALID_LSN_OFFSET == offset )
      {
         goto done ;
      }
      else
      {
         DPS_LSN expectLSN ;

         while ( SDB_OK == rc )
         {
            offset = _sync.getSyncCtrlArbitLSN() ;
            expectLSN = _logger->expectLsn() ;

            // when log file number == 1
            if ( offset >= expectLSN.offset )
            {
               goto done ;
            }

            threshTime = _getThresholdTime( expectLSN.offset - offset ) ;
            if ( 0 == threshTime )
            {
               goto done ;
            }

            expectLSN.offset += reqLen ;
            if ( ( expectLSN.offset > offset + _logger->getLogFileSz() &&
                   _logger->calcFileID( expectLSN.offset ) ==
                   _logger->calcFileID( offset ) ) ||
                 ( waitTime < threshTime ) )
            {
               if ( !_inSyncCtrl )
               {
                  _inSyncCtrl = TRUE ;
                  PD_LOG( PDWARNING, "Begin sync control..." ) ;
               }
               ossSleep( CLS_SYNCCTRL_BASE_TIME ) ;
               waitTime += CLS_SYNCCTRL_BASE_TIME ;
            }
            else
            {
               break ;
            }

            if ( cb->isInterrupted() )
            {
               rc = SDB_APP_INTERRUPT ;
            }
            else if ( !_vote.primaryIsMe() )
            {
               rc = SDB_CLS_NOT_PRIMARY ;
            }
         }
      }

   done:
      if ( 0 == waitTime && _inSyncCtrl )
      {
         _inSyncCtrl = FALSE ;
         PD_LOG( PDWARNING, "End sync control" ) ;
      }
      PD_TRACE_EXITRC ( SDB__CLSREPSET__CHECKSYNCCTRL, rc );
      return rc ;
   }

   INT64 _clsReplicateSet::netIn()
   {
      return _agent->netIn() ;
   }

   INT64 _clsReplicateSet::netOut()
   {
      return _agent->netOut() ;
   }

   void _clsReplicateSet::resetMon()
   {
      return _agent->resetMon() ;
   }

}
