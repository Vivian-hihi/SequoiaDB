/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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
   class _clsMgr;

   class _clsMsgHandler : public _netMsgHandler
   {
      public:
         _clsMsgHandler ( _clsMgr * pClsMgr ) ;
         virtual ~_clsMsgHandler () ;

         virtual INT32 type () const = 0 ;

         OSS_INLINE void attach( pmdEDUCB *cb ) { _pMgrEDUCB = cb; }
         OSS_INLINE void detach() { _pMgrEDUCB = NULL; }

         OSS_INLINE void attachShardCB( pmdEDUCB *cb ) { _pShardCB = cb ; }
         OSS_INLINE void detachShardCB() { _pShardCB = NULL ; }

         virtual INT32 handleMsg( const NET_HANDLE &handle,
                                  const _MsgHeader *header,
                                  const CHAR *msg );
         virtual void  handleClose( const NET_HANDLE &handle, _MsgRouteID id ) ;

      protected:
         void* copyMsg ( const CHAR* msg, UINT32 length ) ;

         INT32 handleSessionMsg( const NET_HANDLE &handle,
                                 const _MsgHeader *header,
                                 const CHAR *msg );

         INT32 handleClsMsg( const NET_HANDLE &handle,
                             const _MsgHeader *header,
                             const CHAR *msg );

      protected:
         _clsMgr              *_pClsMgr ;
         pmdEDUCB             *_pMgrEDUCB ;
         pmdEDUCB             *_pShardCB ;

   };

   class _shdMsgHandler : public _clsMsgHandler
   {
      public:
         _shdMsgHandler( _clsMgr * pClsMgr );
         virtual ~_shdMsgHandler();

         virtual INT32 type () const ;

      protected:

   };

   class _replMsgHandler : public _clsMsgHandler
   {
      public:
         _replMsgHandler ( _clsMgr * pClsMgr ) ;
         virtual ~_replMsgHandler () ;

         virtual INT32 type () const ;

      protected:

   };

}

#endif //CLS_MSG_HANDLER_HPP_

