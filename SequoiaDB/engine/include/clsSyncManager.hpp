/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsSyncManager.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLSSYNCMANAGER_HPP_
#define CLSSYNCMANAGER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "clsDef.hpp"
#include "ossLatch.hpp"
#include "clsSyncMinHeap.hpp"
#include "msgReplicator.hpp"
#include "ossAtomic.hpp"
#include <set>

using namespace std ;

namespace engine
{
   class _netRouteAgent ;
   class _dpsLogWrapper ;
   class _pmdEDUCB;

   typedef multiset<DPS_LSN_OFFSET> CLS_WAKE_PLAN ;

   class _clsSyncManager : public SDBObject
   {
   public:
      _clsSyncManager( _netRouteAgent *agent,
                       _clsGroupInfo *info ) ;

      ~_clsSyncManager() ;

   public:
      INT32 sync( _clsSyncSession &session,
                  const UINT32 &w ) ;

      INT32 updateNotifyList() ;

      void complete( const MsgRouteID &id,
                     const DPS_LSN &lsn,
                     UINT32 TID ) ;


      void handleTimeout( const UINT32 &interval ) ;

      void notify( const DPS_LSN_OFFSET &offset ) ;

      MsgRouteID getSyncSrc( const set<UINT64> &blacklist ) ;

      MsgRouteID getFullSrc( const set<UINT64> &blacklist ) ;

      inline BOOLEAN isReadyToReplay()
      {
         _info->mtx.lock_r() ;
         BOOLEAN rc = _info->primary.value ==
                      _info->local.value &&
                      _info->primary.value !=
                      MSG_INVALID_ROUTEID ?
                      FALSE : TRUE ;
         _info->mtx.release_r() ;
         return rc ;
      }

      void cut( UINT32 alives ) ;

   private:
      INT32 _wait( _pmdEDUCB *&cb,
                  UINT32 sub ) ;

      void _createWakePlan( const DPS_LSN_OFFSET &oOffset,
                            CLS_WAKE_PLAN &plan ) ;

      void _wake( CLS_WAKE_PLAN &plan ) ;

      void _complete( const MsgRouteID &id,
                      const DPS_LSN_OFFSET &offset ) ;


//      void _clearSyncList( const UINT32 &num,
//                           _clsSyncStatus *left ) ;

      INT32 _findStartPoint( const DPS_LSN &remote, DPS_LSN &lsn ) ;

   private:
      /// sub between <0, CLS_REPLSET_MAX_NODE_SIZE - 2>.
      /// means ( w = 2 ) to ( w = CLS_REPLSET_MAX_NODE_SIZE ).
      _clsSyncMinHeap _syncList[CLS_REPLSET_MAX_NODE_SIZE - 1] ;
      DPS_LSN_OFFSET  _checkList[CLS_REPLSET_MAX_NODE_SIZE -1] ;
      _ossSpinXLatch _mtxs[CLS_REPLSET_MAX_NODE_SIZE - 1] ;
      _clsSyncStatus _notifyList[CLS_REPLSET_MAX_NODE_SIZE - 1] ;

      _netRouteAgent *_agent ;
      _clsGroupInfo *_info ;
      MsgRouteID _syncSrc ;

      /// valid _notifyList size
      UINT32 _validSync ;
      UINT32 _timeout ;
   } ;
}

#endif

