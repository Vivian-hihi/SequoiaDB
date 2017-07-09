/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

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
#include "pmdOptionsMgr.hpp"
#include "pmdAsyncSession.hpp"
#include "pmdAsyncHandler.hpp"
#include "utilESCltMgr.hpp"

#define SEADPT_FIELD_NAME              "name"
#define SEADPT_TID(sessionID)          ((UINT32)(sessionID & 0xFFFFFFFF))
namespace engine
{
   class _seAdptCB ;

   enum SEADPT_SESSION_TYPE
   {
      SEADPT_SESSION_INDEX = 1
   } ;

   enum SEADPT_INNER_SESSION_TID
   {
      SEADPT_TID_INDEXER = 1
   } ;

   enum SEADPT_IDX_UPDATE_STEP
   {
      SEADPT_IDX_UPDATE_BEGIN = 1,
      SEADPT_IDX_UPDATE_QUERY,
      SEADPT_IDX_UPDATE_START_TASK
   } ;

   struct _seAdptSessionInfo : public SDBObject
   {
      INT32       type ;
      INT32       startType ;
      INT32       innerTid ;
      UINT64      sessionID ;
      void*       data ;
   } ;
   typedef _seAdptSessionInfo seAdptSessionInfo ;

   // Manage the configurations of the search engine adapter
   class _seAdptOptions : public _pmdCfgRecord
   {
   public:
      _seAdptOptions() ;
      virtual ~_seAdptOptions() {}

      INT32 init( const CHAR *rootPath ) ;

      const CHAR* getCfgFileName() const { return _cfgFileName ; }
      const CHAR* getSvcName() const { return _serviceName ; }
      const CHAR* getDbHost() const { return _dbHost ; }
      const CHAR* getDbService() const { return _dbService ; }
      const CHAR* getSeHost() const { return _seHost ; }
      const CHAR* getSeService() const { return _seService ; }
      UINT32      getDbGroup() const { return _dbGroup; }
      UINT32      getDbNodeID() const { return _dbNodeID ; }
      PDLEVEL     getDiagLevel() const ;

   protected:
      virtual INT32 doDataExchange( pmdCfgExchange *pEX ) ;

   private:
      CHAR     _cfgFileName[ OSS_MAX_PATHSIZE + 1 ] ;
      CHAR     _serviceName[ OSS_MAX_SERVICENAME + 1 ] ;
      CHAR     _dbHost[ OSS_MAX_HOSTNAME + 1 ] ;
      CHAR     _dbService[ OSS_MAX_SERVICENAME + 1 ] ;
      CHAR     _seHost[ OSS_MAX_PATHSIZE + 1 ] ;
      CHAR     _seService[ OSS_MAX_SERVICENAME + 1 ] ;
      INT32    _dbGroup ;
      INT32    _dbNodeID ;
      INT32    _diagLevel ;
   } ;
   typedef _seAdptOptions seAdptOptions ;

   // Service sessions manager. It manages the sessions which started by client
   // connections.
   class _seSvcSessionMgr : public _pmdAsycSessionMgr
   {
   public:
      _seSvcSessionMgr( _seAdptCB *pAdptCB ) ;
      virtual ~_seSvcSessionMgr() {}

      virtual INT32 handleSessionTimeout( UINT32 timerID, UINT32 interval ) ;
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
                                     pmdAsyncSession *pSession )
      {
      }
      virtual pmdAsyncSession* _createSession( SDB_SESSION_TYPE sessionType,
                                               INT32 startType,
                                               UINT64 sessionID,
                                               void *data = NULL ) ;
   private:
      _seAdptCB *_pAdptCB ;
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

      _seIndexTask( string origCSName, string origCLName, string origIdxName,
                    string cappedCSName, string cappedCLName, string esIdxName,
                    string esTypeName, BSONObj &indexKey )
      {
         _origCSName = origCSName ;
         _origCLName = origCLName ;
         _cappedCSName = cappedCSName ;
         _cappedCLName = cappedCLName ;
         _origIdxName = origIdxName ;
         _esIdxName = esIdxName ;
         _esTypeName = esTypeName ;
         _indexDef = indexKey ;
      }

      std::string toString() const
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
   } ;
   typedef _seIndexTask seIndexTask ;

   class _seIndexSessionMgr : public _pmdAsycSessionMgr
   {
      typedef map<UINT64, seIndexTask> OBJ_PROCESS_MAP ;

   public:
      _seIndexSessionMgr( _seAdptCB *pAdptCB ) ;
      virtual ~_seIndexSessionMgr() ;

      virtual INT32 handleSessionTimeout( UINT32 timerID, UINT32 interval ) ;
      virtual INT32 handleSessionClose( const NET_HANDLE handle ) ;

      virtual UINT64 makeSessionID( const NET_HANDLE &handle,
                                    const MsgHeader *header ) ;
      virtual void onSessionDestoryed( pmdAsyncSession *pSession ) ;

      BOOLEAN isIndexTimerStarted() const ;
      void startIndexTimer( UINT32 interval ) ;
      void stopIndexTimer() ;

      INT64 getIndexVersion() { return _indexVersion; }
      INT32 updateIndexInfo( BSONObj &obj ) ;

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
      OBJ_PROCESS_MAP      _objProcessMap ;
      INT64                _indexVersion ;
      UINT32               _indexSessionTimer ;
      UINT32               _innerSessionID ;
      SEADPT_IDX_UPDATE_STEP _updateStep ;
   } ;
   typedef _seIndexSessionMgr seIndexSessionMgr ;

   /*
    * Key control block of search engine adapter.
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

      seAdptOptions*       getOptions() ;
      utilESCltMgr*        getSeCltMgr() ;
      seSvcSessionMgr*     getSeAgentMgr() ;
      seIndexSessionMgr*   getIdxSessionMgr() ;
      netRouteAgent*       getIdxRouteAgent() ;

      INT32 startInnerSession( INT32 type, INT32 innerTID, void *data = NULL ) ;
      INT32 sendToDataNode( MsgHeader *msg ) ;
      BOOLEAN isDataNodePrimary() { return _peerPrimary ; }
      UINT32 getDataNodeGID() { return _peerGroupID ; }

   private:
      INT32 _initSdbAddr() ;
      INT32 _initSearchEngineAddr() ;
      INT32 _sendRegisterMsg() ;
      INT32 _startEDU( INT32 type, EDU_STATUS waitStatus,
                       void *args, BOOLEAN regSys ) ;

      INT32 _onRegisterRes( NET_HANDLE handle, MsgHeader *msg ) ;

   private:
      INT32 _startInnerSession( INT32 type, pmdAsycSessionMgr *pSessionMgr ) ;
      INT32 _sendIdxUpdateReq() ;
      INT32 _onIdxUpdateRes( NET_HANDLE handle, MsgHeader *msg ) ;

   private:
      pmdAsyncMsgHandler      _indexMsgHandler ;
      pmdAsyncMsgHandler      _svcMsgHandler ;
      pmdAsyncTimerHandler    _indexTimerHandler ;
      pmdAsyncTimerHandler    _svcTimerHandler ;
      netRouteAgent           _indexNetRtAgent ;  // net route agent for indexer
      netRouteAgent           _svcRtAgent ;
      seIndexSessionMgr       _idxSessionMgr ;
      seSvcSessionMgr         _sessionMgr ;
      seAdptOptions           _options ;
      CHAR                    _serviceName[ OSS_MAX_SERVICENAME + 1 ] ;

      ossEvent                _attachEvent ;
      UINT32                  _oneSecTimer ;
      MsgRouteID              _dataNodeID ;
      BOOLEAN                 _peerPrimary ;    // If the connected data node is
                                                // primary. If not, no document
                                                // index should be done. The
                                                // role of the node may change,
                                                // so need to update this member
                                                // accorrdingly.
      UINT32                  _peerGroupID ;

      utilESCltMgr            _seCltMgr ;
      MsgRouteID              _selfRouteID ;
      ossSpinSLatch           _seLatch ;
      VECINNERPARAM           _vecInnerSessionParam ;

      INT64                   _textIdxVersion ;
   } ;
   typedef _seAdptCB seAdptCB ;

   seAdptCB* sdbGetSeAdapterCB() ;
   seAdptOptions* sdbGetSeAdptOptions() ;
   seSvcSessionMgr* sdbGetSeAgentCB() ;
   utilESCltMgr* sdbGetSeCltMgr() ;
}

#endif /* SE_ADPTMGR_HPP_ */

