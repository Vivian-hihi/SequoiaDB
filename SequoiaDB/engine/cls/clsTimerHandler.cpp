/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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
