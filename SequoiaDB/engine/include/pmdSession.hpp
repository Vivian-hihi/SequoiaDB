/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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

#include "core.hpp"
#include "oss.hpp"
#include "clsObjBase.hpp"
#include "ossSocket.hpp"
#include "pmdDef.hpp"

namespace engine
{

   class _pmdEDUCB ;

   /*
      _pmdSession define
   */
   class _pmdSession : public _clsObjBase, public _ISession
   {
      DECLARE_OBJ_MSG_MAP()

      public:
         _pmdSession( SOCKET fd ) ;
         virtual ~_pmdSession() ;

         virtual const CHAR*     sessionName() const ;

      public:
         UINT64      sessionID () const { return _eduID ; }
         EDUID       eduID () const { return _eduID ; }
         _pmdEDUCB*  eduCB () const { return _pEDUCB ; }
         ossSocket*  socket () { return &_socket ; }

         void        attach( _pmdEDUCB * cb ) ;
         void        dettach() ;

         INT32       getBuff( INT32 len, CHAR **ppBuff, INT32 &buffLen ) ;

         void        disconnect() ;
         INT32       sendData( const CHAR *pData, INT32 size ) ;
         INT32       recvData( CHAR *pData, INT32 size ) ;

      protected:

      protected:

         _pmdEDUCB            *_pEDUCB ;
         EDUID                _eduID ;
         ossSocket            _socket ;

      private:
         CHAR                 *_pBuff ;
         INT32                _buffLen ;

   } ;
   typedef _pmdSession pmdSession ;

   /*
      _pmdLocalSession define
   */
   class _pmdLocalSession : public _pmdSession
   {
      DECLARE_OBJ_MSG_MAP()

      public:
         _pmdLocalSession( _pmdEDUCB *cb, SOCKET fd ) ;
         virtual ~_pmdLocalSession () ;

         virtual INT32     sessionType() const { return PMD_SESSION_LOCAL ; }
         virtual UINT64    identifyID() ;

      protected:
         virtual INT32  _defaultMsgFunc ( NET_HANDLE handle, MsgHeader* msg ) ;
         virtual INT32  _onAuth( MsgHeader *msg ) ;

         INT32          _onSysInfoRequest( const CHAR *msg ) ;

      // message functions
      protected:
         INT32 _onOPMsg ( NET_HANDLE handle, MsgHeader *msg ) ;

      protected:
         BOOLEAN              _authOK ;

   } ;
   typedef _pmdLocalSession pmdLocalSession ;

}

#endif //PMD_SESSION_HPP_

