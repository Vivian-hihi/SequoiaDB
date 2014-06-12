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

         virtual INT32     run() ;

      public:
         httpConnection*   getRestConn() { return &_restConn ; }
         CHAR*             getFixBuff() ;
         INT32             getFixBuffSize () const ;

         void              restoreSession( restSessionInfo *pSessionInfo ) ;
         void              saveSession( restSessionInfo &sessionInfo ) ;

      protected:
         virtual void            _onAttach () ;
         virtual void            _onDetach () ;

      protected:
         virtual INT32  _onAuth( MsgHeader *msg ) ;

         INT32          _processRestMsg( HTTP_PARSE_COMMON command, const CHAR *pFilePath ) ;

         INT32          _getFileContent( string filePath, CHAR **pFileContent, 
                                         INT32 &fileContentLen );

      protected:

      protected:
         httpConnection                _restConn ;
         CHAR*                         _pFixBuff ;

         restSessionInfo*              _pSessionInfo ;

         string                        _wwwRootPath ;

   } ;
   typedef _pmdRestSession pmdRestSession ;

}

#endif //PMD_REST_SESSION_HPP_

