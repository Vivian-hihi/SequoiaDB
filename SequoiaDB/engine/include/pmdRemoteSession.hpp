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
   class _pmdEDUCB ;

   /*
      PMD_RS_PROCESS_CODE define
   */
   enum PMD_RS_PROCESS_CODE
   {
   } ;

   /*
      _IRemoteSessionHandler define
   */
   class _IRemoteSessionHandler : public SDBObject
   {
      public:
         _IRemoteSessionHandler() {}
         virtual ~_IRemoteSessionHandler() {}

      public:
         virtual PMD_RS_PROCESS_CODE   onRecvReply() = 0 ;

   } ;
   typedef _IRemoteSessionHandler IRemoteSessionHandler ;


   /*
      define
   */
   class _pmdRemoteSessionMgr ;

   /*
      _pmdSubSession define
   */
   class _pmdSubSession : public SDBObject
   {
      friend class _pmdRemoteSession ;

      public:
         _pmdSubSession() ;
         ~_pmdSubSession() ;

         void        clearReplyInfo() ;

         netIOVec*   getIODatas() { return &_ioDatas ; }
         void        clearIODatas() { _ioDatas.clear() ; }
         void        addIODatas( const netIOVec &ioVec )
         {
            for ( UINT32 i = 0 ; i < ioVec.size() ; ++i )
            {
               _ioDatas.push_back( ioVec[ i ] ) ;
            }
         }
         void        addIOData( const netIOV &io )
         {
            _ioDatas.push_back( io ) ;
         }
         UINT32      getIODataLen()
         {
            UINT32 len = 0 ;
            for ( UINT32 i = 0 ; i < _ioDatas.size() ; ++i )
            {
               len += _ioDatas[ i ].iovLen ;
            }
            return len ;
         }

         void        setReqMsg( MsgHeader *pReqMsg ) { _pReqMsg = pReqMsg ; }
         MsgHeader*  getReqMsg() { return _pReqMsg ; }
         MsgHeader*  getRspMsg() { return _pRspMsg ; }

         UINT64      getNodeID() const { return _nodeID ; }
         UINT64      getReqID() const { return _reqID ; }

         BOOLEAN     isDisconnect() const { return _isDisconnect ; }
         BOOLEAN     isSend() const { return _isSend ; }

         void        setProcessInfo( INT32 processResult )
         {
            _processResult = processResult ;
            _isProcessed   = TRUE ;
         }

      protected:
         void        setNodeID( UINT64 nodeID ) { _nodeID = nodeID ; }
         void        setReqID( UINT64 reqID ) { _reqID = reqID ; }
         void        disconnect() { _isDisconnect = TRUE ; }
         void        setSendResult( BOOLEAN isSend ) { _isSend = isSend ; }
         void        setRspMsg( MsgHeader *pRspMsg ) { _pRspMsg = pRspMsg ; }

      protected:
         UINT64                     _nodeID ;
         UINT64                     _reqID ;
         BOOLEAN                    _isSend ;
         BOOLEAN                    _isDisconnect ;

         MsgHeader                  *_pReqMsg ;
         netIOVec                   _ioDatas ;
         MsgHeader                  *_pRspMsg ;

         BOOLEAN                    _isProcessed ;
         INT32                      _processResult ;
   } ;
   typedef _pmdSubSession pmdSubSession ;

   typedef map< UINT64, pmdSubSession >            MAP_SUB_SESSION ;
   typedef MAP_SUB_SESSION::iterator               MAP_SUB_SESSION_IT ;

   typedef set< UINT64 >                           SET_SUB_SESSIONID ;
   typedef vector< pmdSubSession* >                VEC_SUB_SESSIONPTR ;

   /*
      _pmdRemoteSession define
   */
   class _pmdRemoteSession : public SDBObject
   {
      friend class _pmdRemoteSessionMgr ;

      public:
         virtual ~_pmdRemoteSession() ;

         _pmdEDUCB* getEDUCB() { return _pEDUCB ; }
         UINT64     sessionID() const { return _sessionID ; }

         MAP_SUB_SESSION* getSubSessions() ;
         pmdSubSession* addSubSession( UINT64 nodeID ) ;
         pmdSubSession* getSubSession( UINT64 nodeID ) ;
         void           delSubSession( UINT64 nodeID ) ;
         INT32          disconnSubSession( UINT64 nodeID ) ;
         void           clearSubSession() ;

         UINT32         getSubSessionCount() ;
         UINT32         getReplyCount( BOOLEAN exceptProcessed = FALSE ) ;
         UINT32         getSucReplyCount() ;

         BOOLEAN        isTimeout() ;
         BOOLEAN        isAllReply() ;

      public:
         INT32    sendMsg( MsgHeader *pSrcMsg, INT32 *pSucNum = NULL,
                           INT32 *pTotalNum = NULL ) ;
         INT32    sendMsg( MsgHeader *pSrcMsg, SET_SUB_SESSIONID &subs,
                           INT32 *pSucNum = NULL, INT32 *pTotalNUm = NULL ) ;
         INT32    sendMsg( INT32 *pSucNum = NULL, INT32 *pTotalNum = NULL ) ;
         INT32    sendMsg( UINT64 nodeID ) ;

         INT32    waitReply( BOOLEAN waitAll = FALSE,
                             VEC_SUB_SESSIONPTR *pVecSubs = NULL,
                             INT64 millisec = -1 ) ;
      private:
         _pmdRemoteSession( netRouteAgent *pAgent,
                            UINT64 sessionID,
                            INT64 timeout = -1,
                            IRemoteSessionHandler *pHandle = NULL ) ;
         void attachCB( _pmdEDUCB *cb ) { _pEDUCB = cb ; }
         void detachCB() { _pEDUCB = NULL ; }
         void reset( UINT64 sessionID,
                     INT64 timeout = -1,
                     IRemoteSessionHandler *pHandle = NULL ) ;
         void clear() ;

      protected:
         MAP_SUB_SESSION               _mapSubSession ;
         IRemoteSessionHandler         *_pHandle ;
         netRouteAgent                 *_pAgent ;
         _pmdEDUCB                     *_pEDUCB ;
         UINT64                        _sessionID ;

   } ;
   typedef _pmdRemoteSession pmdRemoteSession ;

   /*
      _pmdRemoteSessionMgr define
   */
   class _pmdRemoteSessionMgr : public SDBObject
   {
      typedef map< UINT64, pmdRemoteSession* >        MAP_REMOTE_SESSION ;
      typedef MAP_REMOTE_SESSION::iterator            MAP_REMOTE_SESSION_IT ;

      typedef vector< pmdRemoteSession* >             VEC_REMOTE_SESSION ;

      typedef map< UINT32, _pmdEDUCB* >               MAP_TID_2_EDU ;
      typedef MAP_TID_2_EDU::iterator                 MAP_TID_2_EDU_IT ;

      public:
         _pmdRemoteSessionMgr() ;
         ~_pmdRemoteSessionMgr() ;

         INT32       init( netRouteAgent *pAgent ) ;
         INT32       fini() ;

         void        registerEDU( _pmdEDUCB *cb ) ;
         void        unregEUD( _pmdEDUCB *cb ) ;

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

