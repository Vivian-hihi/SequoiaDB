/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsVSSecondary.cpp

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

#include "clsVSSecondary.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   _clsVSSecondary::_clsVSSecondary( _clsGroupInfo *info,
                                     _netRouteAgent *agent ):
                                _clsVoteStatus( info, agent,
                                                 CLS_ELECTION_STATUS_SEC )
   {

   }

   _clsVSSecondary::~_clsVSSecondary()
   {

   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSSD_HDINPUT, "_clsVSSecondary::handleInput" ) 
   INT32 _clsVSSecondary::handleInput( const MsgHeader *header,
                                       INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSSD_HDINPUT ) ;
      SDB_ASSERT( NULL != header, "header should not be NULL" )
      if ( MSG_CLS_BALLOT_RES == (UINT32)header->opCode )
      {
         next = id() ;
         goto done ;
      }
      else if ( MSG_CLS_BALLOT == header->opCode )
      {
         const _MsgClsElectionBallot *msg = ( const _MsgClsElectionBallot * )
                                              header ;
         if ( CLS_ELECTION_ROUND_STAGE_ONE == msg->round )
         {
            _promise( msg ) ;
            next = id() ;
         }
         else
         {
            if ( SDB_OK == _accept( msg ) )
            {
               next = CLS_ELECTION_STATUS_SILENCE ;
            }
            else
            {
               next = id() ;
            }
         }
      }
      else
      {
         /// error msg
         next = id() ;
      }
   done:
      PD_TRACE_EXIT ( SDB__CLSVSSD_HDINPUT ) ;
      return SDB_OK ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSSD_HDTMOUT, "_clsVSSecondary::handleTimeout" )
   void _clsVSSecondary::handleTimeout( const UINT32 &millisec,
                                        INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSSD_HDTMOUT ) ;
      _timeout() += millisec ;
      if ( CLS_VOTE_CS_TIME <= _timeout() )
      {
         next = CLS_ELECTION_STATUS_VOTE ;
      }
      else
      {
         next = id() ;
      }
      PD_TRACE_EXIT ( SDB__CLSVSSD_HDTMOUT ) ;
      return ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSSD_ACTIVE, "_clsVSSecondary::active" )
   void _clsVSSecondary::active( INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSSD_ACTIVE ) ;
      _timeout() = 0 ;

      if ( _info()->groupSize() == 1 )
      {
         next = CLS_ELECTION_STATUS_VOTE ;
      }
      else
      {
         next = id() ;
      }
      PD_TRACE_EXIT ( SDB__CLSVSSD_ACTIVE ) ;
      return ;
   }
}
