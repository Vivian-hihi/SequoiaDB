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
#include "ossRWMutex.hpp"
#include "ossAtomic.hpp"

#include <string>

namespace engine
{

   /*
      global define
   */
   #define SESSION_USER_NAME_LEN       ( 63 )

   /*
      Internal memory assistor
   */
   typedef struct _sessionMemInfo
   {
      INT32                   _size ;
      _sessionMemInfo         *_next ;
   } sessionMemInfo ;

   /*
      _restSessionInfo define
   */
   struct _restSessionInfo : public SDBObject
   {
      struct _sessionAttr
      {
         UINT64            _sessionID ;      // host ip + seq
         UINT64            _loginTime ;
         CHAR              _userName[SESSION_USER_NAME_LEN+1] ;
      } _attr ;

      // status
      UINT64               _activeTime ;
      BOOLEAN              _authOK ;
      ossAtomic32          _inNum ;
      ossRWMutex           _inRWLock ;
      std::string          _id ;
      sessionMemInfo       *_pSessionMem ;

      _restSessionInfo()
      :_inNum( 0 )
      {
         _attr._sessionID     = 0 ;
         _attr._loginTime     = (UINT64)time(NULL) ;
         ossMemset( _attr._userName, 0, sizeof( _attr._userName ) ) ;
         _activeTime          = _attr._loginTime ;
         _authOK              = FALSE ;
         
         _pSessionMem         = NULL ;
      }

      INT32 getAttrSize()
      {
         return (INT32)sizeof( _attr ) ;
      }

      void releaseMem()
      {
         sessionMemInfo *next = _pSessionMem ;
         while ( _pSessionMem )
         {
            next = _pSessionMem->_next ;
            SDB_OSS_FREE( (CHAR*)_pSessionMem ) ;
            _pSessionMem = next ;
         }
      }

      BOOLEAN isValid() const
      {
         return _id.size() == 0 ? TRUE : FALSE ;
      }

      void invalidate()
      {
         _id = "" ;
      }
   } ;
   typedef _restSessionInfo restSessionInfo ;

   /*
      _pmdRestSession define
   */
   class _pmdRestSession : public _pmdLocalSession
   {
      public:
         _pmdRestSession( SOCKET fd ) ;
         virtual ~_pmdRestSession () ;

         virtual INT32     sessionType() const { return PMD_SESSION_REST ; }
         virtual UINT64    identifyID() ;

      public:
         httpConnection*   getRestConn() { return &_restConn ; }
         CHAR*             getFixBuff() ;
         INT32             getFixBuffSize () const ;

         void              restoreSession( restSessionInfo &sessionInfo ) ;
         void              saveSession( restSessionInfo &sessionInfo ) ;

      protected:
         virtual INT32  _onAuth( MsgHeader *msg ) ;

      protected:

      protected:
         httpConnection                _restConn ;
         CHAR                          *_pFixBuff ;

         UINT64                        _loginTime ;
         std::string                   _userName ;
         

   } ;
   typedef _pmdRestSession pmdRestSession ;

}

#endif //PMD_REST_SESSION_HPP_

