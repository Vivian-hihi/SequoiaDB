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

#include "clsVSVote.hpp"
#include "pmd.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   _clsVSVote::_clsVSVote( _clsGroupInfo *info,
                             _netRouteAgent *agent ):
                        _clsVoteStatus( info, agent,
                                        CLS_ELECTION_STATUS_VOTE )
   {

   }

   _clsVSVote::~_clsVSVote()
   {

   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSVT_HDINPUT, "_clsVSVote::handleInput" )
   INT32 _clsVSVote::handleInput( const MsgHeader *header,
                                  INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSVT_HDINPUT ) ;
      SDB_ASSERT( NULL != header, "header should not be NULL" )

      if ( MSG_CLS_BALLOT == header->opCode )
      {
         const _MsgClsElectionBallot *msg = ( const _MsgClsElectionBallot * )
                                            header ;
         if ( CLS_ELECTION_ROUND_STAGE_ONE == msg->round )
         {
            if ( SDB_OK == _promise( msg ) )
            {
               next = CLS_ELECTION_STATUS_SEC;
            }
            else
            {
               next = id() ;
            }
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
      else if ( MSG_CLS_BALLOT_RES == (UINT32)header->opCode )
      {
         const _MsgClsElectionRes *msg = ( const _MsgClsElectionRes * )
                                         header ;
         if ( CLS_ELECTION_ROUND_STAGE_ONE == msg->round )
         {
            if ( SDB_OK == msg->header.res )
            {
               if ( _info()->groupSize() <= ( ++_accepted() + 1 ) )
               {
                  next = CLS_ELECTION_STATUS_ANNOUNCE ;
               }
               else
               {
                  next = id() ;
               }
            }
            else
            {
               next = CLS_ELECTION_STATUS_SEC ;
            }
         }
         else
         {
            next = id() ;
         }
      }
      else
      {
         next = id() ;
      }

      PD_TRACE_EXIT ( SDB__CLSVSVT_HDINPUT ) ;
      return SDB_OK ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSVT_HDTMOUT, "_clsVSVote::handleTimeout" )
   void _clsVSVote::handleTimeout( const UINT32 &millisec,
                                   INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSVT_HDTMOUT ) ;
      _timeout() += millisec ;
      if ( CLS_VOTE_CS_TIME <= _timeout() )
      {
         if ( _isAccepted() )
         {
            next = CLS_ELECTION_STATUS_ANNOUNCE ;
         }
         else
         {
            next = CLS_ELECTION_STATUS_SEC ;
         }
      }
      else
      {
         next = id() ;
      }
      PD_TRACE_EXIT ( SDB__CLSVSVT_HDTMOUT ) ;
      return ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSVT_ACTIVE, "_clsVSVote::active" )
   void _clsVSVote::active( INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSVT_ACTIVE ) ;
      _timeout() = 0 ;
      _accepted() = 0 ;

      if ( _info()->groupSize() == 1 &&
           SDB_START_NORMAL == pmdGetKRCB()->getStartType())
      {
         next = CLS_ELECTION_STATUS_ANNOUNCE ;
      }
      else
      {
         next = id() ;
         _vote() ;
      }
      PD_TRACE_EXIT ( SDB__CLSVSVT_ACTIVE ) ;
      return ;
   }
}
