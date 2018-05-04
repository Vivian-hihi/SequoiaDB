/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = coordRemoteHandle.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/17/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef COORD_REMOTE_HANDLE_HPP__
#define COORD_REMOTE_HANDLE_HPP__

#include "pmdRemoteSession.hpp"
#include "coordDef.hpp"

#include "../bson/bson.h"

using namespace bson ;

namespace engine
{

   /*
      _coordRemoteHandlerBase define
   */
   class _coordRemoteHandlerBase : public _IRemoteSessionHandler
   {
      public:
         _coordRemoteHandlerBase() ;
         virtual ~_coordRemoteHandlerBase() ;

      public:
         virtual INT32  onSendFailed( _pmdRemoteSession *pSession,
                                      _pmdSubSession **ppSub,
                                      INT32 flag ) ;

         /*
            include disconnect: MSG_BS_DISCONNECT or isDisconnect()
         */
         virtual void   onReply( _pmdRemoteSession *pSession,
                                 _pmdSubSession **ppSub,
                                 const MsgHeader *pReply,
                                 BOOLEAN isPending ) ;

         /*
            if return SDB_OK, will continue
            else, send failed
         */
         virtual INT32  onSendConnect( _pmdSubSession *pSub,
                                       const MsgHeader *pReq,
                                       BOOLEAN isFirst ) ;

         virtual INT32  onSend( _pmdRemoteSession *pSession,
                                _pmdSubSession *pSub ) ;

      protected:

         INT32          _sessionInit( _pmdRemoteSession *pSession,
                                      const MsgRouteID &nodeID,
                                      _pmdEDUCB *cb ) ;

         INT32          _buildPacket( _pmdRemoteSession *pSession,
                                      _pmdSubSession *pSub,
                                      MsgHeader *pHeader ) ;

         INT32          _buildPacketWithUpdateSched( _pmdRemoteSession *pSession,
                                                     _pmdSubSession *pSub,
                                                     const BSONObj &objSched ) ;

         INT32          _buildPacketWithSessionInit( _pmdRemoteSession *pSession,
                                                     _pmdSubSession *pSub ) ;

   } ;
   typedef _coordRemoteHandlerBase coordRemoteHandlerBase ;

   /*
      _coordNoSessionInitHandler define
   */
   class _coordNoSessionInitHandler : public _coordRemoteHandlerBase
   {
      public:
         _coordNoSessionInitHandler() ;
         virtual ~_coordNoSessionInitHandler() ;

      public:
         /*
            if return SDB_OK, will continue
            else, send failed
         */
         virtual INT32  onSendConnect( _pmdSubSession *pSub,
                                       const MsgHeader *pReq,
                                       BOOLEAN isFirst ) ;
   } ;
   typedef _coordNoSessionInitHandler coordNoSessionInitHandler ;

   /*
      _coordRemoteHandler define
   */
   class _coordRemoteHandler : public _coordRemoteHandlerBase
   {
      public:
         _coordRemoteHandler() ;
         virtual ~_coordRemoteHandler() ;

         void     enableInterruptWhenFailed( BOOLEAN enable,
                                             const SET_RC *pIgnoreRC = NULL ) ;

      public:

         /*
            include disconnect: MSG_BS_DISCONNECT or isDisconnect()
         */
         virtual void   onReply( _pmdRemoteSession *pSession,
                                 _pmdSubSession **ppSub,
                                 const MsgHeader *pReply,
                                 BOOLEAN isPending ) ;

         virtual INT32  onExpiredReply ( pmdRemoteSessionSite *pSite,
                                         const MsgHeader *pReply ) ;

      protected:
         BOOLEAN        _interruptWhenFailed ;
         SET_RC         _ignoreRC ;

   } ;
   typedef _coordRemoteHandler coordRemoteHandler ;

}

#endif // COORD_REMOTE_HANDLE_HPP__
