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

   Source File Name = pmdRemoteSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/05/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef PMD_REMOTE_SESSION_HPP_
#define PMD_REMOTE_SESSION_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "msg.h"
#include "pmdDef.hpp"
#include "netRouteAgent.hpp"

#include <map>
#include <set>
#include <vector>
#include "../bson/bson.h"

using namespace bson ;
using namespace std ;

namespace engine
{

   class _pmdRemoteSession ;
   class _pmdRemoteSessionMgr ;
   class _pmdRemoteSessionSite ;
   class _pmdSubSession ;
   class _pmdEDUCB ;

   /*
      _IRemoteSessionHandler define
   */
   class _IRemoteSessionHandler : public SDBObject
   {
      public:
         _IRemoteSessionHandler() {}
         virtual ~_IRemoteSessionHandler() {}

      public:
         virtual INT32  onSendFailed( _pmdRemoteSession *pSession,
                                      _pmdSubSession **ppSub,
                                      INT32 flag ) = 0 ;

   } ;
   typedef _IRemoteSessionHandler IRemoteSessionHandler ;

   /*
      _pmdSubSession define
   */
   class _pmdSubSession : public SDBObject
   {
      friend class _pmdRemoteSession ;
      friend class _pmdRemoteSessionSite ;

      public:
         _pmdSubSession() ;
         ~_pmdSubSession() ;

         _pmdRemoteSession* parent() { return _parent ; }

         void        clearReplyInfo() ;
         void        clearRequestInfo() ;

         netIOVec*   getIODatas() { return &_ioDatas ; }
         void        clearIODatas() { _ioDatas.clear() ; }
         void        addIODatas( const netIOVec &ioVec ) ;
         void        addIOData( const netIOV &io ) ;
         UINT32      getIODataLen() ;

         void        setReqMsg( MsgHeader *pReqMsg ) { _pReqMsg = pReqMsg ; }
         MsgHeader*  getReqMsg() { return _pReqMsg ; }
         MsgHeader*  getRspMsg() { return ( MsgHeader* )_event._Data ; }

         UINT64      getNodeIDUInt() const { return _nodeID.value ; }
         MsgRouteID  getNodeID() const { return _nodeID ; }
         UINT64      getReqID() const { return _reqID ; }

         BOOLEAN     isDisconnect() const { return _isDisconnect ; }
         BOOLEAN     isSend() const { return _isSend ; }
         BOOLEAN     hasReply() const { /* TODO:XUJIANHUI */ return FALSE ; }
         BOOLEAN     isProcessed() const { /* TODO:XUJIANHUI */ return FALSE ; }
         INT32       getProcessRet() const { return _processResult ; }

         void        setProcessInfo( INT32 processResult ) ;

      protected:
         void        setParent( _pmdRemoteSession *parent ) { _parent = parent ; }
         void        setNodeID( UINT64 nodeID ) { _nodeID.value = nodeID ; }
         void        setReqID( UINT64 reqID ) { _reqID = reqID ; }
         void        setSendResult( BOOLEAN isSend ) { _isSend = isSend ; }
         void        processEvent( pmdEDUEvent &event ) ;

      protected:
         _pmdRemoteSession          *_parent ;
         MsgRouteID                 _nodeID ;
         UINT64                     _reqID ;
         BOOLEAN                    _isSend ;
         BOOLEAN                    _isDisconnect ;

         MsgHeader                  *_pReqMsg ;
         netIOVec                   _ioDatas ;
         pmdEDUEvent                _event ;

         BOOLEAN                    _isProcessed ;
         INT32                      _processResult ;
   } ;
   typedef _pmdSubSession pmdSubSession ;

   typedef map< UINT64, pmdSubSession >            MAP_SUB_SESSION ;
   typedef MAP_SUB_SESSION::iterator               MAP_SUB_SESSION_IT ;

   typedef map< UINT64, pmdSubSession* >           MAP_SUB_SESSIONPTR ;
   typedef MAP_SUB_SESSIONPTR::iterator            MAP_SUB_SESSIONPTR_IT ;

   typedef vector< pmdSubSession* >                VEC_SUB_SESSIONPTR ;

   typedef set< UINT64 >                           SET_SUB_SESSIONID ;

   /*
      PMD_SUB_SESSION_FILTER define
   */
   enum PMD_SSITR_FILTER
   {
      PMD_SSITR_ALL           = 0,     // all sub sessions
      PMD_SSITR_UNSENT,                // not send
      PMD_SSITR_SENT,                  // send req succeed
      PMD_SSITR_UNREPLY,               // send req, but not reply
      PMD_SSITR_REPLY,                 // send req, and recv reply succeed
      PMD_SSITR_UNPROCESSED,           // recv reply, but not processed
      PMD_SSITR_PROCESSED,             // recv reply, and processed
      PMD_SSITR_PROCESS_SUC,           // has process, and result = SDB_OK
      PMD_SSITR_PROCESS_FAIL,          // has process, but result != SDB_OK
      PMD_SSITR_DISCONNECT             // send, but disconnect
   } ;

   /*
      _pmdSubSessionItr define
   */
   class _pmdSubSessionItr : public SDBObject
   {
      public:
         _pmdSubSessionItr( MAP_SUB_SESSION *pSessions,
                            PMD_SSITR_FILTER filter = PMD_SSITR_ALL ) ;
         ~_pmdSubSessionItr() ;

         BOOLEAN more() ;
         pmdSubSession* next() ;

      protected:
         void _findPos() ;

      private:
         MAP_SUB_SESSION            *_pSessions ;
         PMD_SSITR_FILTER           _filter ;
         MAP_SUB_SESSION_IT         _curPos ;

   } ;
   typedef _pmdSubSessionItr pmdSubSessionItr ;

   /*
      _pmdRemoteSession define
   */
   class _pmdRemoteSession : public SDBObject
   {
      friend class _pmdRemoteSessionMgr ;
      friend class _pmdRemoteSessionSite ;

      public:
         virtual ~_pmdRemoteSession() ;

         _pmdEDUCB* getEDUCB() { return _pEDUCB ; }
         UINT64     sessionID() const { return _sessionID ; }

         void setTimeout( INT64 timeout ) ;

         pmdSubSessionItr getSubSessionItr( PMD_SSITR_FILTER filter =
                                            PMD_SSITR_ALL ) ;
         pmdSubSession* addSubSession( UINT64 nodeID ) ;
         pmdSubSession* getSubSession( UINT64 nodeID ) ;
         void           delSubSession( UINT64 nodeID ) ;
         void           clearSubSession() ;

         UINT32         getSubSessionCount( PMD_SSITR_FILTER filter =
                                            PMD_SSITR_ALL ) ;

         BOOLEAN        isTimeout() const ;
         BOOLEAN        isAllReply() ;

      public:
         /*
            Send by sub session map and use the pSrcMsg.
            If the sub has sent, will not send.
            if send failed, will to call the handle callback
         */
         INT32    sendMsg( MsgHeader *pSrcMsg, INT32 *pSucNum = NULL,
                           INT32 *pTotalNum = NULL ) ;

         /*
            Send only to the subs and use the pSrcMsg.
            If this subs not exist, will added.
            If the subs has sent, will not send again.
            If failed, will to call the handle callback functions
         */
         INT32    sendMsg( MsgHeader *pSrcMsg, SET_SUB_SESSIONID &subs,
                           INT32 *pSucNum = NULL, INT32 *pTotalNum = NULL ) ;

         /*
            Send by sub session map, if the sub has sent, will not send.
            if send failed, will to call the handle callback
         */
         INT32    sendMsg( INT32 *pSucNum = NULL, INT32 *pTotalNum = NULL ) ;

         /*
            Send to specail sub session, if the sub has sent, will not send.
            if send failed, doesn't to call the handle callback
         */
         INT32    sendMsg( UINT64 nodeID ) ;
         INT32    sendMsg( pmdSubSession *pSub ) ;

         INT32    waitReply1( BOOLEAN waitAll = FALSE,
                              MAP_SUB_SESSIONPTR *pSubs = NULL ) ;
         INT32    waitReply( BOOLEAN waitAll = FALSE,
                             VEC_SUB_SESSIONPTR *pSubs = NULL ) ;

      protected:
         void     addPending( pmdSubSession *pSubSession ) ;

      private:
         _pmdRemoteSession( netRouteAgent *pAgent,
                            UINT64 sessionID,
                            _pmdRemoteSessionSite *pSite,
                            INT64 timeout = -1,
                            IRemoteSessionHandler *pHandle = NULL ) ;
         void attachCB( _pmdEDUCB *cb ) { _pEDUCB = cb ; }
         void detachCB() { _pEDUCB = NULL ; }
         void reset( UINT64 sessionID,
                     _pmdRemoteSessionSite *pSite,
                     INT64 timeout = -1,
                     IRemoteSessionHandler *pHandle = NULL ) ;
         void clear() ;

      protected:
         MAP_SUB_SESSION               _mapSubSession ;
         MAP_SUB_SESSIONPTR            _mapPendingSubSession ;
         IRemoteSessionHandler         *_pHandle ;
         _pmdRemoteSessionSite         *_pSite ;
         netRouteAgent                 *_pAgent ;
         _pmdEDUCB                     *_pEDUCB ;
         UINT64                        _sessionID ;
         INT64                         _milliTimeout ; // ms

   } ;
   typedef _pmdRemoteSession pmdRemoteSession ;

   /*
      _pmdRemoteSessionSite define
   */
   class _pmdRemoteSessionSite : public SDBObject
   {
      friend class _pmdRemoteSession ;
      public:
         _pmdRemoteSessionSite() ;
         ~_pmdRemoteSessionSite() ;

         void setEduCB( _pmdEDUCB *cb ) { _pEDUCB = cb ; }
         _pmdEDUCB* eduCB() { return _pEDUCB ; }

      protected:
         INT32    processEvent( pmdEDUEvent &event,
                                MAP_SUB_SESSION &mapSessions,
                                pmdSubSession **ppSub ) ;
         void     addSubSession( pmdSubSession *pSub ) ;

      private:
         MAP_SUB_SESSIONPTR               _mapReq2SubSession ;
         _pmdEDUCB                        *_pEDUCB ;

   } ;
   typedef _pmdRemoteSessionSite pmdRemoteSessionSite ;

   /*
      _pmdRemoteSessionMgr define
   */
   class _pmdRemoteSessionMgr : public SDBObject
   {
      typedef map< UINT64, pmdRemoteSession* >        MAP_REMOTE_SESSION ;
      typedef MAP_REMOTE_SESSION::iterator            MAP_REMOTE_SESSION_IT ;

      typedef vector< pmdRemoteSession* >             VEC_REMOTE_SESSION ;

      typedef map< UINT32, pmdRemoteSessionSite >     MAP_TID_2_EDU ;
      typedef MAP_TID_2_EDU::iterator                 MAP_TID_2_EDU_IT ;

      public:
         _pmdRemoteSessionMgr() ;
         ~_pmdRemoteSessionMgr() ;

         INT32       init( netRouteAgent *pAgent ) ;
         INT32       fini() ;

         void        registerEDU( _pmdEDUCB *cb ) ;
         void        unregEUD( _pmdEDUCB *cb ) ;
         pmdRemoteSessionSite* getSite( _pmdEDUCB *cb ) ;

         INT32       pushMessage( const NET_HANDLE &handle,
                                  const MsgHeader *pMsg ) ;
         void        handleClose( const NET_HANDLE &handle,
                                  const _MsgRouteID &id ) ;

      public:

         pmdRemoteSession* addSession( _pmdEDUCB *cb,
                                       INT64 timeout = -1, // ms
                                       IRemoteSessionHandler *pHandle = NULL ) ;
         pmdRemoteSession* getSession( UINT64 sessionID ) ;
         void              removeSession( UINT64 sessionID ) ;
         void              removeSession( pmdRemoteSession *pSession ) ;
         UINT32            sessionCount() ;

      private:
         netRouteAgent              *_pAgent ;
         UINT64                     _sessionHWNum ;
         MAP_REMOTE_SESSION         _mapSessions ;
         ossSpinSLatch              _mapLatch ;
         VEC_REMOTE_SESSION         _idleSessions ;

         MAP_TID_2_EDU              _mapTID2EDU ;
         ossSpinSLatch              _edusLatch ;

   } ;
   typedef _pmdRemoteSessionMgr pmdRemoteSessionMgr ;

}

#endif //PMD_REMOTE_SESSION_HPP_

