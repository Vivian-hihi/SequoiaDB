/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdRestSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef PMD_REST_SESSION_HPP_
#define PMD_REST_SESSION_HPP_

#include "pmdSession.hpp"
#include "restDefine.hpp"

namespace engine
{

   /*
      _pmdRestSession define
   */
   class _pmdRestSession : public _pmdLocalSession
   {
      DECLARE_OBJ_MSG_MAP()

      public:
         _pmdRestSession( SOCKET fd ) ;
         virtual ~_pmdRestSession () ;

         virtual INT32     sessionType() const { return PMD_SESSION_REST ; }
         virtual UINT64    identifyID() ;

      public:
         httpConnection*   getRestConn() { return &_restConn ; }
         CHAR*             getFixBuff() ;
         INT32             getFixBuffSize () const ;

      protected:
         virtual INT32  _defaultMsgFunc ( NET_HANDLE handle, MsgHeader* msg ) ;
         virtual INT32  _onAuth( MsgHeader *msg ) ;

      // message functions
      protected:
         INT32 _onOPMsg ( NET_HANDLE handle, MsgHeader *msg ) ;

      protected:
         httpConnection                _restConn ;
         CHAR                          *_pFixBuff ;


   } ;
   typedef _pmdRestSession pmdRestSession ;

}

#endif //PMD_REST_SESSION_HPP_

