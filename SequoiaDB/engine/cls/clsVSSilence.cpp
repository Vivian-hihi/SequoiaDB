/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsVoteMachine.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/28/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "clsVSSilence.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   _clsVSSilence::_clsVSSilence( _clsGroupInfo *info,
                                   _netRouteAgent *agent ):
                              _clsVoteStatus( info, agent,
                                               CLS_ELECTION_STATUS_SILENCE )
   {

   }

   _clsVSSilence::~_clsVSSilence()
   {

   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSSL_HDINPUT, "_clsVSSilence::handleInput" )
   INT32 _clsVSSilence::handleInput( const MsgHeader *header,
                                     INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSSL_HDINPUT ) ;
      PD_LOG( PDDEBUG, "vote: silence status, ignore msg" ) ;
      next = id() ;
      PD_TRACE_EXIT ( SDB__CLSVSSL_HDINPUT ) ;
      return SDB_OK ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSSL_HDTMOUT, "_clsVSSilence::handleTimeout" )
   void _clsVSSilence::handleTimeout( const UINT32 &millisec,
                                      INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSSL_HDTMOUT ) ;
      _timeout() += millisec ;

      /// silence time must be higher than brk time.
      if ( CLS_SHARING_BRK_TIME + 1000 <= _timeout() )
      {
         next = CLS_ELECTION_STATUS_SEC ;
         goto done ;
      }
      next = id() ;
   done:
      PD_TRACE_EXIT ( SDB__CLSVSSL_HDTMOUT ) ;
      return ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSSL_ACTIVE, "_clsVSSilence::active" )
   void _clsVSSilence::active( INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSSL_ACTIVE ) ;
      _timeout() = 0 ;

      if ( _info()->groupSize() == 1 )
      {
         next = CLS_ELECTION_STATUS_SEC ;
      }
      else
      {
         next = id() ;
      }

      PD_TRACE_EXIT ( SDB__CLSVSSL_ACTIVE ) ;
      return ;
   }
}
