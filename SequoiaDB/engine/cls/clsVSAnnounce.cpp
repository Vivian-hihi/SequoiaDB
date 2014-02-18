/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsVSAnnounce.cpp

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

#include "clsVSAnnounce.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   _clsVSAnnounce::_clsVSAnnounce( _clsGroupInfo *info,
                                   _netRouteAgent *agent ):
                                _clsVoteStatus( info, agent,
                                               CLS_ELECTION_STATUS_ANNOUNCE )
   {

   }

   _clsVSAnnounce::~_clsVSAnnounce()
   {

   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSANN_HDINPUT, "_clsVSAnnounce::handleInput" )
   INT32 _clsVSAnnounce::handleInput( const MsgHeader *header,
                                      INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSANN_HDINPUT ) ;
      SDB_ASSERT( NULL != header, "header should not be NULL" )

      /// do not accept any ballot
      if ( MSG_CLS_BALLOT == header->opCode )
      {
         next = id() ;
      }
      else if ( MSG_CLS_BALLOT_RES == (UINT32)header->opCode )
      {
         const _MsgClsElectionRes *msg = ( const _MsgClsElectionRes * )
                                          header ;
         if ( CLS_ELECTION_ROUND_STAGE_ONE == msg->round )
         {
            next = id() ;
         }
         else
         {
            if ( SDB_OK == msg->header.res )
            {
               if ( _info()->groupSize() <= ( ++_accepted() + 1 ) )
               {
                  next =  CLS_ELECTION_STATUS_PRIMARY;
               }
               else
               {
                  next = id() ;
               }
            }
            else
            {
               next = CLS_ELECTION_STATUS_SILENCE ;
            }
         }
      }
      else
      {
         /// error msg
         next = id() ;
      }
      PD_TRACE_EXIT ( SDB__CLSVSANN_HDINPUT ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSANN_HDTMOUT, "_clsVSAnnounce::handleTimeout" )
   void _clsVSAnnounce::handleTimeout( const UINT32 &millisec,
                                       INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSANN_HDTMOUT ) ;
      _timeout() += millisec ;
      if ( CLS_VOTE_CS_TIME <= _timeout() )
      {
         if ( _isAccepted() )
         {
            next = CLS_ELECTION_STATUS_PRIMARY ;
         }
         else
         {
            next = CLS_ELECTION_STATUS_SILENCE ;
         }
      }
      else
      {
         next = id() ;
      }
      PD_TRACE_EXIT ( SDB__CLSVSANN_HDTMOUT ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSANN_ACTIVE, "_clsVSAnnounce::active" )
   void _clsVSAnnounce::active( INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSANN_ACTIVE ) ;
      _timeout() = 0 ;
      _accepted() = 0 ;

      if ( _info()->groupSize() == 1 )
      {
         next =  CLS_ELECTION_STATUS_PRIMARY ;
      }
      else
      {
         next = id() ;
         _announce() ;
      }
      PD_TRACE_EXIT ( SDB__CLSVSANN_ACTIVE ) ;
      return ;
   }
}
