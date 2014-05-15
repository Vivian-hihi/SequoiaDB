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

   Source File Name = clsMgr.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          29/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_MGR_HPP_
#define CLS_MGR_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "clsObjBase.hpp"
#include "ossSocket.hpp"
#include "netRouteAgent.hpp"
#include "clsSession.hpp"
#include "clsMsgHandler.hpp"
#include "clsTimerHandler.hpp"
#include "clsMemPool.hpp"
#include "clsShardMgr.hpp"
#include "clsReplicateSet.hpp"
#include "clsCatalogAgent.hpp"
#include "ossLatch.hpp"
#include "clsTask.hpp"
#include <map>
#include <deque>
#include <vector>
#include <string>

namespace engine
{

   struct _innerSessionInfo : public SDBObject
   {
      INT32       type ;
      INT32       startType ;
      INT32       innerTid ;
      UINT64      sessionID ;
      void*       data ;
   };

   /*
      _clsMgr define
   */
   class _clsMgr : public _clsObjBase, public _IControlBlock,
                   public _IEventHolder
   {
      friend class _clsMsgHandler ;
      friend class _clsTimerHandler ;

      DECLARE_OBJ_MSG_MAP()

      typedef std::map<UINT64, _clsSession*>    MAPSESSION ;
      typedef MAPSESSION::iterator              MAPSESSION_IT ;

      typedef std::map<NET_HANDLE, clsSessionMeta*>  MAPMETA ;
      typedef MAPMETA::iterator                      MAPMETA_IT ;

      typedef std::deque<_clsSession*>          DEQSESSION ;

      typedef std::vector<_innerSessionInfo>    VECINNERPARAM ;
      typedef std::map<UINT64, BSONObj>         MAPTASKQUERY ;

      typedef std::map<void*, NET_HANDLE>       MAP_MSGNET ;
      typedef MAP_MSGNET::iterator              MAP_MSGNET_IT ;

      typedef std::vector< IEventHander* >      VEC_EVENTHANDLER ;

      public:
         _clsMgr() ;
         ~_clsMgr() ;

         virtual SDB_CB_TYPE cbType() const ;
         virtual const CHAR* cbName() const ;

         virtual INT32  init () ;
         virtual INT32  active () ;
         virtual INT32  deactive () ;
         virtual INT32  fini () ;
         virtual void   onConfigChange() ;

         virtual void* queryInterface( SDB_INTERFACE_TYPE type ) ;

         void     attachMainCB ( pmdEDUCB *pMainCB ) ;
         void     detachMainCB ( pmdEDUCB *pMainCB ) ;

         void     ntyPrimaryChange( BOOLEAN primary,
                                    SDB_EVENT_OCCUR_TYPE type ) ;

      public:
         const CHAR* getShardServiceName() const ;
         const CHAR* getReplServiceName () const ;
         NodeID getNodeID () const ;
         UINT16 getShardServiceID () const ;
         UINT16 getReplServiceID () const ;

      public:
         UINT64 setTimer ( CLS_MEMBER_TYPE type, UINT32 milliSec ) ;
         void   killTimer ( UINT64 timerID ) ;
         INT32  sendToCatlog ( MsgHeader * msg ) ;
         INT32  updateCatGroup ( BOOLEAN unsetPrimary = TRUE,
                                 INT64 millisec = 0 ) ;

         INT32  startInnerSession ( INT32 type, INT32 innerTID,
                                    void *data = NULL ) ;
         INT32  startTaskCheck ( const BSONObj& match ) ;
         INT32  stopTask ( UINT64 taskID ) ;
         INT32  removeTask( UINT64 taskID ) ;

         virtual INT32  regEventHandler( IEventHander *pHandler ) ;
         virtual void   unregEventHandler( IEventHander *pHandler ) ;

         _netRouteAgent *getShardRouteAgent () ;
         _netRouteAgent *getReplRouteAgent () ;
         shardCB * getShardCB () ;
         replCB * getReplCB () ;
         catAgent * getCatAgent () ;
         nodeMgrAgent* getNodeMgrAgent () ;
         _clsMsgHandler* getShardMsgHandle() ;
         _clsTaskMgr*  getTaskMgr () ;
         BOOLEAN  isPrimary () ;
         BOOLEAN  isFullSync () ;
         INT32    clearAllData () ;
         INT32    invalidateCata ( const CHAR *name ) ;

         //msg to net handle assist function
         void        pushMsgHandle ( void *msg, NET_HANDLE handle ) ;
         NET_HANDLE  peekMsgHandle ( void *msg ) ;

      protected:
         _clsSession* getSession( INT32 type, UINT64 sessionID,
                                  INT32 startType = CLS_SESSION_PASSIVE,
                                  const NET_HANDLE handle = 0,
                                  BOOLEAN bCreate = FALSE,
                                  void *data = NULL ) ;

         INT32        assignMemory ( _clsSession *pSession, UINT32 msgLength ) ;

         INT32 handleSessionTimeout ( UINT64 timerID , UINT32 interval ) ;
         INT32 handleSessionClose ( INT32 type, const NET_HANDLE handle ) ;

         INT32 _startSessionEDU ( _clsSession *pSession ) ;
         INT32 _releaseSession ( _clsSession *pSession , BOOLEAN delay=FALSE ) ;
         INT32 _releaseSession_i ( _clsSession *pSession, BOOLEAN postQuit,
                                   BOOLEAN delay ) ;

         INT32          _startEDU ( INT32 type, EDU_STATUS waitStatus,
                                    void *agrs, BOOLEAN regSys = TRUE ) ;

         _clsSession    *_createSession ( INT32 type, INT32 startType,
                                          UINT64 sessionID,
                                          void *data = NULL ) ;

         INT32          _attachSessionMeta( INT32 type, _clsSession *pSession,
                                            const NET_HANDLE handle ) ;

         INT32 _sendRegisterMsg () ;
         INT32 _sendQueryTaskReq ( UINT64 requestID, const CHAR *clFullName,
                                   const BSONObj* match ) ;

         void  _callRegisterEventHandler() ;
         void  _callPrimaryChangeHandler( BOOLEAN primary,
                                          SDB_EVENT_OCCUR_TYPE type ) ;

         virtual INT32 _defaultMsgFunc ( NET_HANDLE handle, MsgHeader* msg ) ;
         virtual void  onTimer ( UINT64 timerID, UINT32 interval ) ;

      private:
         MAPSESSION *_getSessionMap ( INT32 type ) ;
         MAPMETA    *_getMetaMap( INT32 type ) ;
         INT32       _startInnerSession ( INT32 type ) ;
         INT32       _prepareTask () ;
         INT32       _addTaskInnerSession ( const CHAR *objdata ) ;

         void        _checkSessionMeta( INT32 type ) ;

      //msg and event function
      protected:
         INT32 _onCatRegisterRes ( NET_HANDLE handle, MsgHeader* msg ) ;
         INT32 _onCatQueryTaskRes ( NET_HANDLE handle, MsgHeader* msg ) ;

      private:
         _shdMsgHandler                _shdMsgHandler ;
         _replMsgHandler               _replMsgHandler ;
         _clsShardTimerHandler         _shdTimerHandler ;
         _clsReplTimerHandler          _replTimerHandler ;

         _netRouteAgent                _replNetRtAgent ;
         _netRouteAgent                _shardNetRtAgent ;

         _clsShardMgr                  _shdObj ;
         _clsReplicateSet              _replObj ;

         ossEvent                      _attachEvent ;

         UINT16                        _shardServiceID ;
         UINT16                        _replServiceID ;
         CHAR                          _replServiceName[OSS_MAX_SERVICENAME+1] ;
         CHAR                          _shdServiceName[OSS_MAX_SERVICENAME+1] ;

         BOOLEAN                       _force ;
         NodeID                        _selfNodeID ;
         _clsMemPool                   _memPool;
         _clsTaskMgr                   _taskMgr ;

         MAPSESSION                    _mapShdSessions;
         MAPMETA                       _mapShdMeta ;
         DEQSESSION                    _deqShdSessions;
         DEQSESSION                    _deqShdDeletingSessions ;
         ossSpinXLatch                 _deqDeletingMutex ;

         MAPSESSION                    _mapReplSessions;
         MAPMETA                       _mapReplMeta ;

         VECINNERPARAM                 _vecInnerSessionParam ;
         MAPTASKQUERY                  _mapTaskQuery ;
         UINT64                        _taskID ;
         map< UINT64, UINT64 >         _mapTaskID ;
         ossSpinSLatch                 _clsLatch ;

         VEC_EVENTHANDLER              _vecEventHandler ;
         ossSpinSLatch                 _handlerLatch ;

         MAP_MSGNET                    _mapMsg2NetHandle ;
         ossSpinXLatch                 _msg2NetLatch ;

         UINT64                        _regTimerID ;
         UINT64                        _oneSecTimerID ;
         UINT64                        _repl1SecTimerID ;
         UINT64                        _shd1MinTimerID ;
         UINT64                        _shd1SecTimerID ;

         UINT64                        _shdHandleCloseTimerID ;
         UINT64                        _replHandleCloseTimerID ;

   };

   typedef _clsMgr  clsCB ;

   /*
      get global cls cb
   */
   clsCB* sdbGetClsCB () ;
   shardCB* sdbGetShardCB () ;
   replCB* sdbGetReplCB () ;

}

#endif //CLS_MGR_HPP_


