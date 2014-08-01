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

   Source File Name = clsMsgHandler.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          22/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_MSG_HANDLER_HPP_
#define CLS_MSG_HANDLER_HPP_

#include "clsBase.hpp"
#include "netMsgHandler.hpp"
#include "pmdEDU.hpp"

namespace engine
{
   class _clsSessionMgr ;

   /*
      _clsMsgHandler define
   */
   class _clsMsgHandler : public _netMsgHandler
   {
      public:
         _clsMsgHandler( _clsSessionMgr *pSessionMgr ) ;
         virtual ~_clsMsgHandler () ;

         OSS_INLINE void attach( pmdEDUCB *cb ) { _pMgrEDUCB = cb; }
         OSS_INLINE void detach() { _pMgrEDUCB = NULL; }

         virtual INT32 handleMsg( const NET_HANDLE &handle,
                                  const _MsgHeader *header,
                                  const CHAR *msg );
         virtual void  handleClose( const NET_HANDLE &handle, _MsgRouteID id ) ;

         virtual void  onStop() ;

      protected:
         void* copyMsg ( const CHAR* msg, UINT32 length ) ;

         INT32 handleSessionMsg( const NET_HANDLE &handle,
                                 const _MsgHeader *header,
                                 const CHAR *msg );

         INT32 handleMainMsg( const NET_HANDLE &handle,
                              const _MsgHeader *header,
                              const CHAR *msg ) ;

      protected:
         virtual void _postMainMsg( const NET_HANDLE &handle,
                                    MsgHeader *pNewMsg ) ;

      protected:
         _clsSessionMgr       *_pSessionMgr ;
         pmdEDUCB             *_pMgrEDUCB ;

   } ;
   typedef _clsMsgHandler clsMsgHandler ;

   /*
      _shdMsgHandler define
   */
   class _shdMsgHandler : public _clsMsgHandler
   {
      public:
         _shdMsgHandler( _clsSessionMgr *pSessionMgr ) ;
         virtual ~_shdMsgHandler();

         OSS_INLINE void attachShardCB( pmdEDUCB *cb ) { _pShardCB = cb ; }
         OSS_INLINE void detachShardCB() { _pShardCB = NULL ; }

      protected:
         virtual void _postMainMsg( const NET_HANDLE &handle,
                                    MsgHeader *pNewMsg ) ;

      protected:
         pmdEDUCB             *_pShardCB ;
   } ;
   typedef _shdMsgHandler shdMsgHandler ;

   /*
      _replMsgHandler define
   */
   class _replMsgHandler : public _clsMsgHandler
   {
      public:
         _replMsgHandler ( _clsSessionMgr *pSessionMgr ) ;
         virtual ~_replMsgHandler () ;

         INT32 type () const ;

      protected:
         virtual void _postMainMsg( const NET_HANDLE &handle,
                                    MsgHeader *pNewMsg ) ;

   } ;
   typedef _replMsgHandler replMsgHandler ;

}

#endif //CLS_MSG_HANDLER_HPP_

