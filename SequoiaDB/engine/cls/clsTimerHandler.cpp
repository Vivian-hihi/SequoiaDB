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

   Source File Name = clsTimerHandler.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          1/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "core.hpp"
#include "clsTimerHandler.hpp"
#include "clsSession.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   /*
      _clsTimerHandler implement
   */
   _clsTimerHandler::_clsTimerHandler( _clsSessionMgr * pSessionMgr )
   {
      _pMgrCB = NULL ;
      _pSessionMgr = pSessionMgr ;
   }

   _clsTimerHandler::~_clsTimerHandler()
   {
      _pMgrCB = NULL ;
      _pSessionMgr = NULL ;
   }

   UINT64 _clsTimerHandler::_makeTimerID( UINT32 timerID )
   {
      return ( UINT64 )timerID ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSTMHD_HDTMOUT, "_clsTimerHandler::handleTimeout" )
   void _clsTimerHandler::handleTimeout( const UINT32 &millisec,
                                         const UINT32 &id )
   {
      PD_TRACE_ENTRY ( SDB__CLSTMHD_HDTMOUT ) ;
      UINT64 timerID = _makeTimerID ( id ) ;

      if ( _pSessionMgr->handleSessionTimeout( timerID , millisec ) != SDB_OK )
      {
         PMD_EVENT_MESSAGES *eventMsg = (PMD_EVENT_MESSAGES *)
            SDB_OSS_MALLOC( sizeof (PMD_EVENT_MESSAGES ) ) ;

         if ( NULL == eventMsg )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for PDM "
                     "timeout Event" ) ;
         }
         else
         {
            ossTimestamp ts;
            ossGetCurrentTime(ts);

            eventMsg->timeoutMsg.interval = millisec ;
            eventMsg->timeoutMsg.occurTime = ts.time ;
            eventMsg->timeoutMsg.timerID = timerID ;

            _pMgrCB->postEvent( pmdEDUEvent ( PMD_EDU_EVENT_TIMEOUT, 
                                              PMD_EDU_MEM_ALLOC,
                                              (void*)eventMsg ) ) ;
         }
      }
      PD_TRACE_EXIT ( SDB__CLSTMHD_HDTMOUT ) ;
   }

   /*
      _clsReplTimerHandler implement
   */
   _clsReplTimerHandler::_clsReplTimerHandler ( _clsSessionMgr * pSessionMgr )
      :_clsTimerHandler ( pSessionMgr )
   {
   }

   _clsReplTimerHandler::~_clsReplTimerHandler ()
   {
   }

   UINT64 _clsReplTimerHandler::_makeTimerID( UINT32 timerID )
   {
      return ossPack32To64( CLS_REPL, timerID ) ;
   }

   /*
      _clsShardTimerHandler implement
   */
   _clsShardTimerHandler::_clsShardTimerHandler ( _clsSessionMgr *pSessionMgr )
      :_clsTimerHandler ( pSessionMgr )
   {
   }

   _clsShardTimerHandler::~_clsShardTimerHandler ()
   {
   }

   UINT64 _clsShardTimerHandler::_makeTimerID( UINT32 timerID )
   {
      return ossPack32To64( CLS_SHARD, timerID ) ;
   }

}

