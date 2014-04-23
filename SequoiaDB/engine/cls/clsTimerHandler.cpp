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
#include "clsMgr.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   _clsTimerHandler::_clsTimerHandler( _clsMgr * pClsMgr )
   {
      _pMgrCB = NULL ;
      _pClsMgr = pClsMgr ;
   }

   _clsTimerHandler::~_clsTimerHandler()
   {
      _pMgrCB = NULL ;
      _pClsMgr = NULL ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSTMHD_HDTMOUT, "_clsTimerHandler::handleTimeout" )
   void _clsTimerHandler::handleTimeout( const UINT32 &millisec,
                                         const UINT32 &id )
   {
      PD_TRACE_ENTRY ( SDB__CLSTMHD_HDTMOUT ) ;
      UINT64 timerID = ossPack32To64 ( type(), id ) ;

      if ( _pClsMgr->handleSessionTimeout( timerID , millisec ) != SDB_OK )
      {
         PMD_EVENT_MESSAGES *eventMsg = (PMD_EVENT_MESSAGES *)
            SDB_OSS_MALLOC( sizeof (PMD_EVENT_MESSAGES ) ) ;

         if ( NULL == eventMsg )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for PDM timeout Event" ) ;
         }
         else
         {
            ossTimestamp ts;
            ossGetCurrentTime(ts);

            eventMsg->timeoutMsg.interval = millisec ;
            eventMsg->timeoutMsg.occurTime = ts.time ;
            eventMsg->timeoutMsg.timerID = ossPack32To64( type(), id ) ;

            _pMgrCB->postEvent( pmdEDUEvent (PMD_EDU_EVENT_TIMEOUT, 
               TRUE, (void*)eventMsg) ) ;
         }
      }
      PD_TRACE_EXIT ( SDB__CLSTMHD_HDTMOUT ) ;
   }

   _clsReplTimerHandler::_clsReplTimerHandler ( _clsMgr * pClsMgr )
      :_clsTimerHandler ( pClsMgr )
   {
   }

   _clsReplTimerHandler::~_clsReplTimerHandler ()
   {
   }

   INT32 _clsReplTimerHandler::type () const
   {
      return CLS_REPL ;
   }

   _clsShardTimerHandler::_clsShardTimerHandler ( _clsMgr * pClsMgr )
      :_clsTimerHandler ( pClsMgr )
   {
   }

   _clsShardTimerHandler::~_clsShardTimerHandler ()
   {
   }

   INT32 _clsShardTimerHandler::type () const
   {
      return CLS_SHARD ;
   }

}
