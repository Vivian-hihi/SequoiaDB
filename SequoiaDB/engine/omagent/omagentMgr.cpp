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

   Source File Name = omagentMgr.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/


#include "omagentMgr.hpp"

namespace engine
{

   /*
      _omAgentOptions implement
   */
   _omAgentOptions::_omAgentOptions()
   {
   }

   _omAgentOptions::~_omAgentOptions()
   {
   }

   INT32 _omAgentOptions::init ()
   {
      INT32 rc = SDB_OK ;

      // TODO:XUJIANHUI

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omAgentOptions::doDataExchange( pmdCfgExchange * pEX )
   {
      INT32 rc = SDB_OK ;

      // TODO:XUJIANHUI

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _omAgentSessionMgr implement
   */
   _omAgentSessionMgr::_omAgentSessionMgr()
   {
   }

   _omAgentSessionMgr::~_omAgentSessionMgr()
   {
   }

   UINT64 _omAgentSessionMgr::makeSessionID( const NET_HANDLE & handle,
                                             const MsgHeader * header )
   {
      return ossPack32To64( CLS_BASE_HANDLE_ID + handle, header->TID ) ;
   }

   SDB_SESSION_TYPE _omAgentSessionMgr::_prepareCreate( UINT64 sessionID,
                                                        INT32 startType,
                                                        INT32 opCode )
   {
      return SDB_SESSION_OMAGENT ;
   }

   BOOLEAN _omAgentSessionMgr::_canReuse( SDB_SESSION_TYPE sessionType )
   {
      return FALSE ;
   }

   UINT32 _omAgentSessionMgr::_maxCatchSize() const
   {
      return 0 ;
   }

   void _omAgentSessionMgr::_onPushMsgFailed( INT32 rc, const MsgHeader *pReq,
                                              const NET_HANDLE &handle,
                                              clsSession *pSession )
   {
      _reply( handle, rc, pReq ) ;
   }

   clsSession* _omAgentSessionMgr::_createSession( SDB_SESSION_TYPE sessionType,
                                                   INT32 startType,
                                                   UINT64 sessionID,
                                                   void * data )
   {
      // TODO:XUJIANHUI
      return NULL ;
   }

   /*
      omAgentMgr Message MAP
   */
   BEGIN_OBJ_MSG_MAP( _omAgentMgr, _clsObjBase )
      
   END_OBJ_MSG_MAP()

   /*
      omAgentMgr implement
   */
   _omAgentMgr::_omAgentMgr()
   : _msgHandler( &_sessionMgr ),
     _timerHandler( &_sessionMgr ),
     _netAgent( &_msgHandler )
   {
   }

   _omAgentMgr::~_omAgentMgr()
   {
   }

   SDB_CB_TYPE _omAgentMgr::cbType() const
   {
      return SDB_CB_OMAGT ;
   }

   const CHAR* _omAgentMgr::cbName() const
   {
      return "OMAGENT" ;
   }

   INT32 _omAgentMgr::init()
   {
      INT32 rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omAgentMgr::active()
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   INT32 _omAgentMgr::deactive()
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   INT32 _omAgentMgr::fini()
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   omAgentOptions* _omAgentMgr::getOptions()
   {
      return &_options ;
   }

   /*
      get the global om manager object point
   */
   omAgentMgr *sdbGetOMAgentMgr()
   {
      static omAgentMgr s_omagent ;
      return &s_omagent ;
   }

   omAgentOptions* sdbGetOMAgentOptions()
   {
      return sdbGetOMAgentMgr()->getOptions() ;
   }

}


