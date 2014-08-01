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

   Source File Name = clsSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_SESSION_HPP_
#define CLS_SESSION_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "clsBase.hpp"
#include "clsObjBase.hpp"
#include "pmdEDU.hpp"
#include "netRouteAgent.hpp"
#include "ossLatch.hpp"
#include "ossAtomic.hpp"
#include "clsMemPool.hpp"
#include "sdbInterface.hpp"

#include <map>
#include <deque>

namespace engine
{

   #define INVLIAD_SESSION_ID       (0)
   #define SESSION_NAME_LEN         (50)

   #define MAX_BUFFER_ARRAY_SIZE    (20)

   #define CLS_BUFF_INVALID         (0)
   #define CLS_BUFF_ALLOC           (1)
   #define CLS_BUFF_USING           (2)
   #define CLS_BUFF_FREE            (3)

   class _clsSessionMgr ;

   /*
      _clsBuffInfo define
   */
   class _clsBuffInfo : public SDBObject
   {
   public :
      CHAR     *pBuffer ;
      UINT32   size ;
      INT32    useFlag ;
      time_t   addTime ;

      BOOLEAN isAlloc () { return useFlag == CLS_BUFF_ALLOC ? TRUE : FALSE ; }
      BOOLEAN isUsing () { return useFlag == CLS_BUFF_USING ? TRUE : FALSE ; }
      BOOLEAN isFree () { return useFlag == CLS_BUFF_FREE ? TRUE : FALSE ; }
      BOOLEAN isInvalid () { return useFlag == CLS_BUFF_INVALID ? TRUE : FALSE ; }

      void setFree () { useFlag = CLS_BUFF_FREE ; }
   } ;
   typedef class _clsBuffInfo clsBuffInfo ;

   /*
      _clsSessionMeta define
   */
   class _clsSessionMeta : public SDBObject
   {
      public:
         _clsSessionMeta( const NET_HANDLE handle ) ;
         virtual ~_clsSessionMeta() ;

         ossSpinXLatch* getLatch() { return &_Latch ; }
         UINT32         getBasedHandleNum()
         {
            return _basedHandleNum.peek() ;
         }
         NET_HANDLE     getHandle() const { return _netHandle ; }

         void          incBaseHandleNum()
         {
            _basedHandleNum.inc() ;
         }
         void          decBaseHandleNum()
         {
            _basedHandleNum.dec() ;
         }

      private:
         ossSpinXLatch        _Latch ;
         ossAtomic32          _basedHandleNum ;
         NET_HANDLE           _netHandle ;

   } ;
   typedef _clsSessionMeta clsSessionMeta ;

   /*
      _clsSession define
   */
   class _clsSession : public _clsObjBase, public _ISession
   {
      friend class _clsSessionMgr ;
      DECLARE_OBJ_MSG_MAP()

      public:
         _clsSession( UINT64 sessionID );
         virtual ~_clsSession();

         virtual UINT64          identifyID() ;
         virtual const CHAR*     sessionName() const ;
         virtual INT32           getServiceType() const ;

         virtual EDU_TYPES eduType () const = 0 ;

         virtual void    onRecieve ( const NET_HANDLE netHandle,
                                     MsgHeader * msg ) ;
         virtual BOOLEAN timeout ( UINT32 interval ) ;

         virtual void clear() ;
         virtual BOOLEAN canAttachMeta() const { return TRUE ; }

         void* copyMsg ( const CHAR *msg, UINT32 length ) ;
         INT32 waitAttach () ;
         INT32 waitDetach () ;
         INT32 attachIn ( pmdEDUCB *cb ) ;
         INT32 attachOut () ;

         BOOLEAN isAttached () const ;
         BOOLEAN isDetached () const ;

      public:
         UINT64      sessionID () const ;
         EDUID       eduID () const ;
         pmdEDUCB*   eduCB () const ;
         NET_HANDLE  netHandle () const ;
         BOOLEAN     isStartActive() ;
         clsSessionMeta* getMeta() { return _pMeta ; }

         BOOLEAN        isBufferFull() const ;
         BOOLEAN        isBufferEmpty() const ;

      private:
         void        startType ( INT32 startType ) ;
         void        meta ( clsSessionMeta * pMeta ) ;
         void        sessionID ( UINT64 sessionID ) ;

         clsBuffInfo*   frontBuffer () ;
         void           popBuffer () ;
         INT32          pushBuffer ( CHAR *pBuffer, UINT32 size ) ;

         UINT32         _incBuffPos ( UINT32 pos ) ;
         UINT32         _decBuffPos ( UINT32 pos ) ;

      protected:
         void  _makeName () ;
         INT32 _lock () ;
         INT32 _unlock () ;

         virtual void   _onAttach () ;
         virtual void   _onDetach () ;

      protected:

         UINT64               _sessionID ;
         pmdEDUCB             *_pEDUCB ;
         EDUID                _eduID ;
         NET_HANDLE           _netHandle ;

         CHAR                 _name[SESSION_NAME_LEN+1] ;
         clsSessionMeta        *_pMeta ;

      private:
         ossSpinXLatch        _latchIn ;
         ossSpinXLatch        _latchOut ;
         BOOLEAN              _lockFlag ;
         INT32                _startType ;

         clsBuffInfo          _buffArray[MAX_BUFFER_ARRAY_SIZE] ;
         UINT32               _buffBegin ;
         UINT32               _buffEnd ;
         UINT32               _buffCount ;

   };
   typedef _clsSession clsSession ;

   /*
      _clsSessionMgr define
   */
   class _clsSessionMgr : public SDBObject
   {
      public:
      typedef std::map<UINT64, _clsSession*>          MAPSESSION ;
      typedef MAPSESSION::iterator                    MAPSESSION_IT ;

      typedef std::map<NET_HANDLE, clsSessionMeta*>   MAPMETA ;
      typedef MAPMETA::iterator                       MAPMETA_IT ;

      typedef std::deque<_clsSession*>                DEQSESSION ;

      public:
         _clsSessionMgr() ;
         virtual ~_clsSessionMgr() ;

         virtual INT32        init( netRouteAgent *pRTAgent,
                                    _netTimeoutHandler *pTimerHandle,
                                    UINT32 timerInterval ) ;
         virtual INT32        fini() ;
         void                 setForced() { _force = TRUE ; }

         virtual void         onTimer( UINT32 interval ) ;

         /*
            The following function:
            Note: The caller thread must be the net thread
         */
         INT32          pushMessage ( clsSession *pSession,
                                      const MsgHeader *header,
                                      const NET_HANDLE &handle ) ;

         clsSession     *getSession( UINT64 sessionID, INT32 startType,
                                     const NET_HANDLE handle,
                                     BOOLEAN bCreate, INT32 opCode,
                                     void *data ) ;

         INT32          releaseSession( clsSession *pSession,
                                        BOOLEAN delay = FALSE ) ;

         INT32          handleSessionClose( const NET_HANDLE handle ) ;

         virtual INT32  handleSessionTimeout( UINT32 timerID,
                                              UINT32 interval ) ;

      public:
         virtual UINT64       makeSessionID( const NET_HANDLE &handle,
                                             const MsgHeader *header ) = 0 ;

      protected:
         /*
            Parse the session type
         */
         virtual SDB_SESSION_TYPE   _prepareCreate( UINT64 sessionID,
                                                    INT32 startType,
                                                    INT32 opCode ) = 0 ;

         virtual BOOLEAN      _canReuse( SDB_SESSION_TYPE sessionType ) = 0 ;
         virtual UINT32       _maxCatchSize() const = 0 ;
         virtual void         _onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                                const NET_HANDLE &handle,
                                                clsSession *pSession ) = 0 ;
         /*
            Create session
         */
         virtual clsSession*  _createSession(  SDB_SESSION_TYPE sessionType,
                                               INT32 startType,
                                               UINT64 sessionID,
                                               void *data = NULL ) = 0 ;

      protected:
         INT32          _attachSessionMeta( clsSession *pSession,
                                            const NET_HANDLE handle ) ;

         INT32          _startSessionEDU( clsSession * pSession ) ;

         INT32          _releaseSession_i ( clsSession *pSession,
                                            BOOLEAN postQuit,
                                            BOOLEAN delay ) ;

         INT32          _reply( const NET_HANDLE &handle, INT32 rc,
                                const MsgHeader *pReqMsg ) ;

      protected:
         void           _checkSession( UINT32 interval ) ;
         void           _checkSessionMeta( UINT32 interval ) ;

      protected:
         MAPSESSION                 _mapSession ;
         MAPMETA                    _mapMeta ;
         DEQSESSION                 _deqCatchSessions ;

         // Delay delete sessions
         DEQSESSION                 _deqDeletingSessions ;
         ossSpinXLatch              _deqDeletingMutex ;

         clsMemPool                 _memPool ;
         netRouteAgent              *_pRTAgent ;
         _netTimeoutHandler         *_pTimerHandle ;

         UINT32                     _handleCloseTimerID ;
         UINT32                     _sessionTimerID ;
         UINT32                     _timerInterval ;

         BOOLEAN                    _force ;

   } ;
   typedef _clsSessionMgr clsSessionMgr ;

}

#endif //CLS_SESSION_HPP_

