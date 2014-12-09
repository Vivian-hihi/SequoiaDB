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

   Source File Name = pmdSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/04/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef PMD_SESSION_HPP_
#define PMD_SESSION_HPP_

#include "pmdSessionBase.hpp"
#include "msg.h"
#include "pmdDef.hpp"
#include "rtnContext.hpp"
#include "pmdProcessorBase.hpp"

#include <map>
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{

   class _SDB_DMSCB ;
   class _dpsLogWrapper ;
   class _SDB_RTNCB ;
   class _rtnContextBuf ;

   /*
      _pmdLocalSession define
   */
   class _pmdLocalSession : public _pmdSession
   {
      public:
         _pmdLocalSession( SOCKET fd ) ;
         virtual ~_pmdLocalSession () ;

         virtual UINT64    identifyID() ;
         virtual INT32     getServiceType() const ;
         virtual SDB_SESSION_TYPE sessionType() const ;

         INT32             run() ;

      protected:
         INT32          _processMsg( MsgHeader *msg ) ;
         virtual INT32  _onMsgBegin( MsgHeader *msg ) ;
         virtual void   _onMsgEnd( INT32 result, MsgHeader *msg ) ;

         virtual INT32  _processOPMsg( MsgHeader *msg, INT64 &contextID,
                                       const CHAR **ppBody, INT32 &bodyLen,
                                       INT32 &returnNum, INT32 &startPos ) ;

         virtual INT32  _onAuth( MsgHeader *msg ) ;

         INT32          _recvSysInfoMsg( UINT32 msgSize, CHAR **ppBuff,
                                         INT32 &buffLen ) ;
         INT32          _processSysInfoRequest( const CHAR *msg ) ;

         INT32          _reply( MsgOpReply* responseMsg, const CHAR *pBody,
                                INT32 bodyLen ) ;

         INT32          _attachProcessor( _IProcessor *processor ) ;
         void           _detachProcessor() ;

      protected:
         virtual void            _onAttach () ;
         virtual void            _onDetach () ;

      // message process functions
      protected:

         INT32 _onInsertReqMsg( MsgHeader *msg ) ;
         INT32 _onUpdateReqMsg( MsgHeader *msg ) ;
         INT32 _onDelReqMsg( MsgHeader *msg ) ;
         INT32 _onInterruptMsg( MsgHeader *msg ) ;
         INT32 _onMsgReqMsg( MsgHeader *msg ) ;
         INT32 _onQueryReqMsg( MsgHeader *msg, _rtnContextBuf &buffObj,
                               INT32 &startingPos, INT64 &contextID ) ;
         INT32 _onGetMoreReqMsg( MsgHeader *msg, _rtnContextBuf &buffObj,
                                 INT32 &startingPos, INT64 &contextID ) ;
         INT32 _onKillContextsReqMsg( MsgHeader *msg ) ;
         INT32 _onSQLMsg( MsgHeader *msg, INT64 &contextID ) ;
         INT32 _onTransBeginMsg () ;
         INT32 _onTransCommitMsg () ;
         INT32 _onTransRollbackMsg () ;
         INT32 _onAggrReqMsg( MsgHeader *msg, INT64 &contextID ) ;
         INT32 _onOpenLobMsg( MsgHeader *msg, INT64 &contextID,
                              rtnContextBuf &buffObj ) ;
         INT32 _onWriteLobMsg( MsgHeader *msg ) ;
         INT32 _onReadLobMsg( MsgHeader *msg,
                              rtnContextBuf &buffObj ) ;
         INT32 _onCloseLobMsg( MsgHeader *msg ) ;
         INT32 _onRemoveLobMsg( MsgHeader *msg ) ;
         INT32 _onGetLobMeta( MsgHeader *msg,
                              rtnContextBuf &buffObj ) ;

      protected:
         BOOLEAN              _authOK ;
         _SDB_DMSCB           *_pDMSCB ;
         _dpsLogWrapper       *_pDPSCB ;
         _SDB_RTNCB           *_pRTNCB ;

         MsgOpReply           _replyHeader ;
         BOOLEAN              _needReply ;
         BOOLEAN              _needRollback ;
         rtnContextBuf        _contextBuff ;

         BSONObj              _errorInfo ;

         _IProcessor          *_processor ;

   } ;
   typedef _pmdLocalSession pmdLocalSession ;

}

#endif //PMD_SESSION_HPP_

