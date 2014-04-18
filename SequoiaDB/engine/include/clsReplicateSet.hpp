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

#ifndef CLSREPLICATESET_HPP_
#define CLSREPLICATESET_HPP_

#include "netRouteAgent.hpp"
#include "msgReplicator.hpp"
#include "msgCatalog.hpp"
#include "clsVoteMachine.hpp"
#include "msg.hpp"
#include "clsObjBase.hpp"
#include "clsCatalogCaller.hpp"
#include "clsSyncManager.hpp"
#include "dms.hpp"
#include "clsReplBucket.hpp"
#include <vector>

using namespace std ;

namespace engine
{
   class _netRouteAgent ;
   class _clsMgr ;
   class _clsDataSrcBaseSession ;
   class _pmdEDUCB ;

   #define CLS_SYNCCTRL_THRESHOLD_SIZE          (10)

   class _clsReplicateSet : public _clsObjBase
   {
      DECLARE_OBJ_MSG_MAP()

      public:
         _clsReplicateSet( _netRouteAgent *agent ) ;
         virtual ~_clsReplicateSet() ;

      public:
         OSS_INLINE BOOLEAN primaryIsMe()
         {
            return _vote.primaryIsMe() ;
         }

         OSS_INLINE BOOLEAN isFullSync ()
         {
            return CLS_BS_FULLSYNC == _replStatus ? TRUE : FALSE ;
         }

         OSS_INLINE CLS_BS_STATUS getStatus () const
         {
            return _replStatus ;
         }

         OSS_INLINE clsBucket* getBucket ()
         {
            return &_replBucket ;
         }

         OSS_INLINE void setLocalID( const MsgRouteID &id )
         {
            _info.local = id ;
            /// _agent was set by clsMgr.
         }

         OSS_INLINE void setFullSync( BOOLEAN fullSync )
         {
            if ( fullSync )
            {
               _replStatus = CLS_BS_FULLSYNC ;
            }
            else
            {
               _replStatus = CLS_BS_NORMAL ;
            }
         }

         OSS_INLINE void setStatus( CLS_BS_STATUS status )
         {
            _replStatus = status ;
         }

         OSS_INLINE const UINT32 ailves()
         {
            UINT32 num = 0 ;
            _info.mtx.lock_r() ;
            num = _info.aliveSize() ;
            _info.mtx.release_r() ;
            return num ;
         }

         OSS_INLINE UINT32 groupSize ()
         {
            UINT32 num = 0 ;
            _info.mtx.lock_r () ;
            num = _info.groupSize() ;
            _info.mtx.release_r  () ;
            return num ;
         }

         OSS_INLINE BOOLEAN isAlive ( NodeID node )
         {
            BOOLEAN bAlive = FALSE ;
            _info.mtx.lock_r() ;
            map<UINT64, _clsSharingStatus *>::iterator it =
               _info.alives.find ( node.value ) ;
            if ( it != _info.alives.end() )
            {
               bAlive = TRUE ;
            }
            _info.mtx.release_r() ;

            return bAlive ;
         }

         OSS_INLINE _clsSyncManager *syncMgr()
         {
            return &_sync ;
         }

         OSS_INLINE INT32 sync( const DPS_LSN_OFFSET &offset,
                            _pmdEDUCB *&eduCB,
                            UINT32 w = 1 )
         {
            _clsSyncSession session ;
            session.endLsn = offset ;
            session.eduCB = eduCB ;
            eduCB->getEvent().reset() ;

            if ( w > 1 )
            {
               UINT32 nodes = groupSize () ;
               if ( w > nodes )
               {
                  w = nodes ;
               }
            }

            return _sync.sync( session, w ) ;
         }

         OSS_INLINE UINT32 getNtySessionNum ()
         {
            return _srcSessionNum ;
         }

         void notify( UINT32 suLID, UINT32 clLID, dmsExtentID extLID,
                      const DPS_LSN_OFFSET &offset ) ;

         void notify2Session( UINT32 suLID, UINT32 clLID, dmsExtentID extLID,
                              const DPS_LSN_OFFSET &offset ) ;

         INT32 checkSyncControl( UINT32 reqLen, _pmdEDUCB *cb ) ;

      public:
         void  regSession ( _clsDataSrcBaseSession *pSession ) ;
         void  unregSession ( _clsDataSrcBaseSession *pSession ) ;

      public:
         virtual INT32 active() ;
         virtual INT32 initialize() ;
         virtual INT32 final() ;

         virtual void  onTimer ( UINT64 timerID, UINT32 interval ) ;

         INT32 handleMsg( NET_HANDLE handle, MsgHeader* msg ) ;

         INT32 callCatalog( MsgHeader *header ) ;

         void getGroupInfo( _MsgRouteID &primary,
                            vector<_netRouteNode > &group ) ;

         _MsgRouteID getPrimary () ;

         void tearDown() ;

         INT64 netIn() ;
         INT64 netOut() ;
         void resetMon() ;


      private:
         INT32 _setGroupSet( const CLS_GROUP_VERSION &version,
                             map<UINT64, _netRouteNode> &nodes ) ;

         INT32 _alive( const _MsgRouteID &id ) ;

         INT32 _handleSharingBeat( const _MsgClsBeat *msg ) ;

         INT32 _handleSharingBeatRes( const _MsgClsBeatRes *msg ) ;

         INT32 _handleGroupRes( const _MsgCatGroupRes *msg ) ;

         void _sharingBeat() ;

         void _checkBreak( const UINT32 &millisec ) ;

         void _checkPrimary () ;

         UINT32 _getThresholdTime( UINT64 diffSize ) ;

      private:
         _netRouteAgent          *_agent ;
         _clsGroupInfo           _info ;
         _clsVoteMachine         _vote ;
         _dpsLogWrapper          *_logger ;
         _clsSyncManager         _sync ;
         _clsCatalogCaller       _cata ;
         clsBucket               _replBucket ;
         _clsMgr                 *_clsCB ;
         UINT64                  _timerID ;
         UINT32                  _beatTime ;
         UINT32                  _downloadTime ;
         BOOLEAN                 _active ;
         CLS_BS_STATUS           _replStatus ;
         BOOLEAN                 _prevPrimary ;

         UINT32                  _srcSessionNum ;
         ossRWMutex              _vecLatch ;
         std::vector<_clsDataSrcBaseSession*> _vecSrcSessions ;

         // sync control param
         UINT64                  _totalLogSize ;
         UINT64                  _sizethreshold[ CLS_SYNCCTRL_THRESHOLD_SIZE ] ;
         UINT32                  _timeThreshold[ CLS_SYNCCTRL_THRESHOLD_SIZE ] ;
         BOOLEAN                 _inSyncCtrl ;

   } ;

   typedef class _clsReplicateSet clsReplicateSet ;
   typedef _clsReplicateSet replCB ;
}

#endif

