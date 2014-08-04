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

   Source File Name = omagentMgr.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_MGR_HPP__
#define OMAGENT_MGR_HPP__

#include "omagentDef.hpp"
#include "clsSession.hpp"
#include "netRouteAgent.hpp"
#include "clsMsgHandler.hpp"
#include "clsTimerHandler.hpp"
#include "pmdOptionsMgr.hpp"
#include "sdbInterface.hpp"

#include <string>

using namespace std ;

namespace engine
{
   class _pmdEDUCB ;

   /*
      _omAgentOptions define
   */
   class _omAgentOptions : public _pmdCfgRecord
   {
      public:
         _omAgentOptions() ;
         virtual ~_omAgentOptions() ;

         INT32    init ( const CHAR *pRootPath ) ;

         const CHAR* getCfgFileName() const { return _cfgFileName ; }
         const CHAR* getLocalCfgPath() const { return _localCfgPath ; }
         const CHAR* getStartProcFile() const { return _startProcFile ; }
         const CHAR* getStopProcFile() const { return _stopProcFile ; }

         const CHAR* getCMServiceName() const { return _cmServiceName ; }
         INT32       getRestartCount() const { return _restartCount ; }
         INT32       getRestartInterval() const { return _restartInterval ; }
         BOOLEAN     isAutoStart() const { return _autoStart ; }

      protected:
         virtual INT32 doDataExchange( pmdCfgExchange *pEX ) ;

      private:
         string                     _hostKey ;

         CHAR                       _cmServiceName[ OSS_MAX_SERVICENAME + 1 ] ;
         // -1: always restart, 0: nerver restart
         INT32                      _restartCount ;
         INT32                      _restartInterval ;
         BOOLEAN                    _autoStart ;

         CHAR                       _cfgFileName[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR                       _localCfgPath[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR                       _startProcFile[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR                       _stopProcFile[ OSS_MAX_PATHSIZE + 1 ] ;

   } ;
   typedef _omAgentOptions omAgentOptions ;

   /*
      _omAgentSessionMgr define
   */
   class _omAgentSessionMgr : public _clsSessionMgr
   {
      public:
         _omAgentSessionMgr() ;
         virtual ~_omAgentSessionMgr() ;

      public:
         virtual UINT64       makeSessionID( const NET_HANDLE &handle,
                                             const MsgHeader *header ) ;

      protected:
         /*
            Parse the session type
         */
         virtual SDB_SESSION_TYPE   _prepareCreate( UINT64 sessionID,
                                                    INT32 startType,
                                                    INT32 opCode ) ;

         virtual BOOLEAN      _canReuse( SDB_SESSION_TYPE sessionType ) ;
         virtual UINT32       _maxCatchSize() const ;
         virtual void         _onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                                const NET_HANDLE &handle,
                                                clsSession *pSession ) ;
         /*
            Create session
         */
         virtual clsSession*  _createSession(  SDB_SESSION_TYPE sessionType,
                                               INT32 startType,
                                               UINT64 sessionID,
                                               void *data = NULL ) ;

   } ;
   typedef _omAgentSessionMgr omAgentSessionMgr ;

   /*
      _omAgentMgr define
   */
   class _omAgentMgr : public _clsObjBase, public _IControlBlock
   {
      DECLARE_OBJ_MSG_MAP()

      public:
         _omAgentMgr() ;
         virtual ~_omAgentMgr() ;

         virtual SDB_CB_TYPE cbType() const ;
         virtual const CHAR* cbName() const ;

         virtual INT32  init () ;
         virtual INT32  active () ;
         virtual INT32  deactive () ;
         virtual INT32  fini () ;

      public:

         omAgentOptions* getOptions() ;

      protected:

      private:
         omAgentOptions             _options ;
         omAgentSessionMgr          _sessionMgr ;
         clsMsgHandler              _msgHandler ;
         clsTimerHandler            _timerHandler ;
         netRouteAgent              _netAgent ;

   } ;

   typedef _omAgentMgr omAgentMgr ;

   /*
      get the global om manager object point
   */
   omAgentMgr *sdbGetOMAgentMgr() ;
   omAgentOptions *sdbGetOMAgentOptions() ;

}

#endif // OMAGENT_MGR_HPP__

