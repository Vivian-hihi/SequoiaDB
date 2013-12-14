/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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

   class _clsMgr : public _clsObjBase
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

      public:
         _clsMgr() ;
         ~_clsMgr() ;

         INT32    initialize ( pmdEDUCB *pMainCB ) ;
         INT32    active () ;
         INT32    final () ;
         INT32    startNet () ;

         // for clsReplicateSet
         void     _onPrimaryChange ( BOOLEAN primary ) ;

      public:
         const CHAR* getHostName() const ;
         const CHAR* getShardServiceName() const ;
         const CHAR* getReplServiceName () const ;
         NodeID getNodeID () const ;
         UINT16 getShardServiceID () const ;
         UINT16 getReplServiceID () const ;

         void setHostName ( const CHAR *hostName ) ;
         void setShardServiceName ( const CHAR *serviceName ) ;
         void setReplServiceName ( const CHAR *serviceName ) ;
         void setNodeID ( const NodeID &nodeID ) ;
         void setShardServiceID ( UINT16 serviceID ) ;
         void setReplServiceID ( UINT16 serviceID ) ;
         void setCatlogInfo ( const NodeID &id, const std::string& host,
                              const std::string& service ) ;

      public:
         UINT64 setTimer ( CLS_MEMBER_TYPE type, UINT32 milliSec ) ;
         void   killTimer ( UINT64 timerID ) ;
         INT32  sendToCatlog ( MsgHeader * msg ) ;
         INT32  updateCatGroup ( BOOLEAN unsetPrimary = TRUE ) ;

         INT32  startInnerSession ( INT32 type, INT32 innerTID,
                                    void *data = NULL ) ;
         INT32  startTaskCheck ( const BSONObj& match ) ;
         INT32  stopTask ( UINT64 taskID ) ;
         INT32  removeTask( UINT64 taskID ) ;

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

         //sub edu attach in/out
         void        attachIn() ;
         void        attachOut() ;

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

         INT32          _startRouteAgentEDU ( INT32 type ) ;

         _clsSession    *_createSession ( INT32 type, INT32 startType,
                                          UINT64 sessionID,
                                          void *data = NULL ) ;

         INT32          _attachSessionMeta( INT32 type, _clsSession *pSession,
                                            const NET_HANDLE handle ) ;

         INT32 _sendRegisterMsg () ;
         INT32 _sendQueryTaskReq ( UINT64 requestID, const CHAR *clFullName,
                                   const BSONObj* match ) ;

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
         _netRouteAgent                *_pReplNetRtAgent ;
         _netRouteAgent                *_pShardNetRtAgent ;

         _clsMsgHandler                *_pShdMsgHandler ;
         _clsMsgHandler                *_pReplMsgHandler ;
         _clsTimerHandler              *_pShdTimerHandler ;
         _clsTimerHandler              *_pReplTimerHandler ;

         _clsShardMgr                  *_pShdObj ;
         _clsReplicateSet              *_pReplObj ;

         ossRWMutex                    _subEduAttachLock ;

         UINT16                        _shardServiceID ;
         UINT16                        _replServiceID ;
         CHAR                          _hostName[OSS_MAX_HOSTNAME+1] ;
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

         MAP_MSGNET                    _mapMsg2NetHandle ;
         ossSpinXLatch                 _msg2NetLatch ;

         VECCATLOG                     _vecCatlog ;

         UINT64                        _regTimerID ;
         UINT64                        _oneSecTimerID ;
         UINT64                        _repl1SecTimerID ;
         UINT64                        _shd1MinTimerID ;
         UINT64                        _shd1SecTimerID ;

         UINT64                        _shdHandleCloseTimerID ;
         UINT64                        _replHandleCloseTimerID ;

   };

   typedef _clsMgr  clsCB ;

}

#endif //CLS_MGR_HPP_


