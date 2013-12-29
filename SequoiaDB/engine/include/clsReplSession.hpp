/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsReplSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_REPL_SESSION_HPP_
#define CLS_REPL_SESSION_HPP_

#include "clsSession.hpp"
#include "dpsMessageBlock.hpp"
#include "clsReplayer.hpp"
#include "dpsLogRecord.hpp"
#include "msgReplicator.hpp"
#include "clsSrcSelector.hpp"

namespace engine
{
   class _dpsLogWrapper ;
   class _clsSyncManager ;
   class _netRouteAgent ;
   class _clsReplicateSet ;
   class _clsBucket ;

   enum CLS_SESSION_STATUS
   {
      CLS_SESSION_STATUS_SYNC = 1,
      CLS_SESSION_STATUS_CONSULT = 2,
      CLS_SESSION_STATUS_FULL_SYNC = 3,
   } ;

   class _clsReplSession : public _clsSession
   {
   DECLARE_OBJ_MSG_MAP()
   public:
      _clsReplSession ( UINT64 sessionID ) ;
      virtual ~_clsReplSession () ;

      virtual INT32 type () const ;
      virtual EDU_TYPES eduType () const ;
      virtual void    onRecieve ( const NET_HANDLE netHandle,
                                  MsgHeader * msg ) ;
      // called by net io thread
      virtual BOOLEAN timeout ( UINT32 interval ) ;
      // called by self thread
      virtual void    onTimer ( UINT64 timerID, UINT32 interval ) ;
      virtual void   _onDetach () ;

   public:
      INT32 handleSyncReq( NET_HANDLE handle, MsgHeader* header ) ;

      INT32 handleSyncRes( NET_HANDLE handle, MsgHeader* header ) ;

      INT32 handleVirSyncReq( NET_HANDLE handle, MsgHeader* header ) ;

      INT32 handleNotify( NET_HANDLE handle, MsgHeader* header ) ;

      INT32 handleConsultReq( NET_HANDLE handle, MsgHeader *header ) ;

      INT32 handleConsultRes( NET_HANDLE handle, MsgHeader *header ) ;


   private:
      INT32 _syncLog( const NET_HANDLE &handle,
                      const MsgReplSyncReq *req ) ;

      INT32 _replayLog( const CHAR *logs, const UINT32 &len, UINT32 &num ) ;

      INT32 _replay( dpsLogRecordHeader *header ) ;

      void _sendSyncReq( DPS_LSN *pCompleteLSN = NULL ) ;

      void _sendVirSyncReq() ;

      void _sendConsultReq() ;

      INT32 _rollback( const CHAR *log ) ;

      void _fullSync() ;

   private:
      _dpsMessageBlock              _mb ;
      clsSrcSelector                _selector ;
      _dpsLogWrapper                *_logger ;
      _clsSyncManager               *_sync ;
      _clsReplicateSet              *_repl ;
      _netRouteAgent                *_agent ;
      _clsBucket                    *_pReplBucket ;
      _clsReplayer                  _replayer ;
      MsgRouteID                    _syncSrc ;
      CLS_SESSION_STATUS            _status ;
      BOOLEAN                       _quit ;
      BOOLEAN                       _isFirstToSync ;
      UINT32                        _timeout ;
      UINT64                        _requestID ;
      UINT32                        _syncFailedNum ;

      DPS_LSN                       _completeLSN ;
      DPS_LSN                       _consultLsn ;
   };


}

#endif //CLS_REPL_SESSION_HPP_

