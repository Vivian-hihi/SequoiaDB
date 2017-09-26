/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = seAdptMgr.hpp

   Descriptive Name = Search engine adapter manager.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SE_ADPTMGR_HPP_
#define SE_ADPTMGR_HPP_

#include "sdbInterface.hpp"
#include "seAdptOptionsMgr.hpp"
#include "pmdAsyncSession.hpp"
#include "pmdAsyncHandler.hpp"
#include "utilESCltMgr.hpp"
#include "seAdptMsgHandler.hpp"

namespace engine
{
   class _seAdptCB ;

   enum SEADPT_SESSION_TYPE
   {
      SEADPT_SESSION_INDEX = 1      // Indexer session for one text index.
   } ;

   enum SEADPT_IDX_UPDATE_STEP
   {
      SEADPT_IDX_UPDATE_BEGIN = 1,
      SEADPT_IDX_UPDATE_START_TASK
   } ;

   struct _seAdptSessionInfo
   {
      SEADPT_SESSION_TYPE  type ;
      INT32                startType ;
      INT32                innerTid ;
      UINT64               sessionID ;
      void*                data ;
   } ;
   typedef _seAdptSessionInfo seAdptSessionInfo ;

   /*
    * Service sessions manager. It manages the sessions which started by client
    * connections, or connections for text search from SDB data node.
    * One listener is created during startup, and one seperate session will be
    * created for each client connection.
    */
   class _seSvcSessionMgr : public _pmdAsycSessionMgr
   {
   public:
      _seSvcSessionMgr() {}
      virtual ~_seSvcSessionMgr() {}

      virtual UINT64 makeSessionID( const NET_HANDLE &handle,
                                    const MsgHeader *header ) ;

   protected:
      virtual SDB_SESSION_TYPE _prepareCreate( UINT64 sessionID,
                                               INT32 startType,
                                               INT32 opCode ) ;

      virtual BOOLEAN _canReuse( SDB_SESSION_TYPE sessionType )
      {
         return FALSE ;
      }
      virtual UINT32 _maxCacheSize() const
      {
         return 0 ;
      }
      virtual void _onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                     const NET_HANDLE &handle,
                                     pmdAsyncSession *pSession ) ;

      // Called by the thread of EDU_TYPE_SE_SERVICE, when a new connection
      // comes.
      virtual pmdAsyncSession* _createSession( SDB_SESSION_TYPE sessionType,
                                               INT32 startType,
                                               UINT64 sessionID,
                                               void *data = NULL ) ;
   } ;
   typedef _seSvcSessionMgr seSvcSessionMgr ;

   // Each text index is bind with a task. It's held by
   struct _seIndexTask
   {
      string _origCSName ;
      string _origCLName ;
      string _cappedCSName ;
      string _cappedCLName ;
      string _origIdxName ;
      string _esIdxName ;
      string _esTypeName ;
      BSONObj _indexDef ;  // Used for fetching data from original collection.

      _seIndexTask( string origCSName, string origCLName,
                    string origIdxName, string cappedCSName,
                    string cappedCLName, string esIdxName,
                    string esTypeName, BSONObj &indexKey )
      {
         _origCSName = origCSName ;
         _origCLName = origCLName ;
         _cappedCSName = cappedCSName ;
         _cappedCLName = cappedCLName ;
         _origIdxName = origIdxName ;
         _esIdxName = esIdxName ;
         _esTypeName = esTypeName ;
         _indexDef = indexKey.copy() ;
      }

      BOOLEAN valid() const
      {
         return ( !_origCSName.empty() && !_origCLName.empty() &&
                  !_cappedCSName.empty() && !_cappedCLName.empty() &&
                  !_origIdxName.empty() && !_esIdxName.empty() &&
                  !_esTypeName.empty() && !_indexDef.isEmpty() &&
                  _indexDef.valid() ) ;
      }

      std::string toString() const
      {
         try
         {
            std::stringstream ss ;
            ss << "Original cs[" << _origCSName << "], "
               << "original cl[" << _origCLName << "], "
               << "capped cs[" << _cappedCSName << "], "
               << "capped cl[" << _cappedCLName << "], "
               << "es index[" << _esIdxName << "], "
               << "es type[" << _esTypeName << "], "
               << "original index[" << _origIdxName << ", "
               << _indexDef.toString() << "]" ;
            return ss.str() ;
         }
         catch ( std::exception &e )
         {
            return "" ;
         }
      }
   } ;
   typedef _seIndexTask seIndexTask ;

   // Manager of seAdptIndexSession.
   class _seIndexSessionMgr : public _pmdAsycSessionMgr
   {
   public:
      _seIndexSessionMgr( _seAdptCB *pAdptCB ) ;
      virtual ~_seIndexSessionMgr() ;

      virtual UINT64 makeSessionID( const NET_HANDLE &handle,
                                    const MsgHeader *header ) ;

      INT64 getIndexVersion() { return _indexVersion; }
      INT32 updateIndexInfo( BSONObj &obj ) ;
      void  stopAllIndexer() ;

   protected:
      virtual SDB_SESSION_TYPE _prepareCreate( UINT64 sessionID,
                                               INT32 startType,
                                               INT32 opCode ) ;
      virtual BOOLEAN _canReuse( SDB_SESSION_TYPE sessionType ) ;
      virtual UINT32 _maxCacheSize() const ;
      virtual void _onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                     const NET_HANDLE &handle,
                                     pmdAsyncSession *pSession ) ;
      virtual pmdAsyncSession* _createSession( SDB_SESSION_TYPE sessionType,
                                               INT32 startType,
                                               UINT64 sessionID,
                                               void *data = NULL ) ;
      UINT32 _getInnerSessionID() { return ++_innerSessionID ; }

   private:
      _seAdptCB            *_pAdptCB ;
      list<seIndexTask>    _taskList ;       // All the tasks according to the last update.
      list<seIndexTask>    _newTaskList ;    // Tasks that must start this round.
      INT64                _indexVersion ;
      UINT32               _indexSessionTimer ;
      UINT32               _innerSessionID ;
      SEADPT_IDX_UPDATE_STEP _updateStep ;
   } ;
   typedef _seIndexSessionMgr seIndexSessionMgr ;

   /*
    * Key control block of search engine adapter.
    * It takes care of the following things:
    * (1) Register the adapter on SDB data node, both at startup and when a
    *     network break happens.
    * (2) Check for text index udpates.
    * (2) Use the seIndexSessionMgr to manage all the indexer sessions. The
    *     index session manager does not have a seperated thread.
    */
   class _seAdptCB : public _pmdObjBase, public _IControlBlock
   {
      friend class _seIndexSessionMgr ;
      DECLARE_OBJ_MSG_MAP()
      typedef std::vector<_seAdptSessionInfo>    VECINNERPARAM ;
   public:
      _seAdptCB() ;
      virtual ~_seAdptCB() ;

      virtual SDB_CB_TYPE cbType() const ;
      virtual const CHAR* cbName() const ;

      virtual INT32 init() ;
      virtual INT32 active() ;
      virtual INT32 deactive() ;
      virtual INT32 fini() ;

      virtual void attachCB( _pmdEDUCB *cb ) ;
      virtual void detachCB( _pmdEDUCB *cb ) ;

      virtual void onTimer( UINT64 timerID, UINT32 interval ) ;

      seAdptOptionsMgr*    getOptions() ;
      utilESCltMgr*        getSeCltMgr() ;
      seSvcSessionMgr*     getSeAgentMgr() ;
      seIndexSessionMgr*   getIdxSessionMgr() ;
      netRouteAgent*       getIdxRouteAgent() ;

      INT32 startInnerSession( SEADPT_SESSION_TYPE type,
                               INT32 innerTID, void *data = NULL ) ;
      void  cleanInnerSession( INT32 type ) ;
      INT32 sendToDataNode( MsgHeader *msg ) ;
      BOOLEAN isDataNodePrimary() { return _peerPrimary ; }
      void setDataNodePrimary( BOOLEAN isPrimary )
      {
         _peerPrimary = isPrimary ;
      }

      const CHAR *getDataNodeGrpName() { return _peerGroupName ; }

      INT32 syncUpdateCLVersion( const CHAR *collectionName, INT64 millsec,
                                 pmdEDUCB *cb, INT32 &version ) ;

   private:
      INT32 _startSvcListener() ;
      INT32 _initSdbAddr() ;
      INT32 _initSearchEngineAddr() ;
      INT32 _sendRegisterMsg() ;
      INT32 _resumeRegister() ;
      INT32 _startEDU( INT32 type, EDU_STATUS waitStatus,
                       void *args, BOOLEAN regSys ) ;

      INT32 _onRegisterRes( NET_HANDLE handle, MsgHeader *msg ) ;

   private:
      INT32 _startInnerSession( INT32 type, pmdAsycSessionMgr *pSessionMgr ) ;
      INT32 _sendIdxUpdateReq() ;
      INT32 _onIdxUpdateRes( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _onRemoteDisconnect( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _setTimers() ;
      void  _killTimer( UINT32 timerID ) ;
      INT32 _onCatalogResMsg( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _sendCataQueryReq( const BSONObj &query, const BSONObj &selector,
                               UINT64 requestID, _pmdEDUCB *cb ) ;

   private:
      indexMsgHandler         _indexMsgHandler ;
      pmdAsyncMsgHandler      _svcMsgHandler ;
      pmdAsyncTimerHandler    _indexTimerHandler ;
      pmdAsyncTimerHandler    _svcTimerHandler ;
      netRouteAgent           _indexNetRtAgent ;  // net route agent for indexer
      netRouteAgent           _svcRtAgent ;
      seIndexSessionMgr       _idxSessionMgr ;
      seSvcSessionMgr         _svcSessionMgr ;
      seAdptOptionsMgr        _options ;
      CHAR                    _serviceName[ OSS_MAX_SERVICENAME + 1 ] ;

      ossEvent                _attachEvent ;
      MsgRouteID              _dataNodeID ;
      MsgRouteID              _cataNodeID ;
      BOOLEAN                 _peerPrimary ;    // If the connected data node is
                                                // primary. If not, no document
                                                // index should be done. The
                                                // role of the node may change,
                                                // so need to update this member
                                                // accorrdingly.
      CHAR                    _peerGroupName[ OSS_MAX_GROUPNAME_SIZE + 1 ] ;

      utilESCltMgr            _seCltMgr ;
      MsgRouteID              _selfRouteID ;
      ossSpinSLatch           _seLatch ;
      VECINNERPARAM           _vecInnerSessionParam ;
      UINT32                  _regTimerID ;        // For register adapter on data node.
      UINT32                  _idxUpdateTimerID ;  // For text index information update.
      UINT32                  _oneSecTimerID ;     // For session check by session managers.
      INT32                   _clVersion ;
      ossSpinSLatch           _verUpdateLock ;
      ossEvent                _cataEvent ;
   } ;
   typedef _seAdptCB seAdptCB ;

   seAdptCB* sdbGetSeAdapterCB() ;
   seAdptOptionsMgr* sdbGetSeAdptOptions() ;
   seSvcSessionMgr* sdbGetSeAgentCB() ;
   utilESCltMgr* sdbGetSeCltMgr() ;
}

#endif /* SE_ADPTMGR_HPP_ */

