/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsVoteStatus.hpp

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

#ifndef CLSVOTESTATUS_HPP_
#define CLSVOTESTATUS_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "clsDef.hpp"
#include "msgReplicator.hpp"

namespace engine
{
   #define CLS_IS_MAJORITY( now, total ) \
           ( ( total / 2) < now )

   const INT32 CLS_INVALID_VOTE_ID = -1 ;

   class _netRouteAgent ;
   class _dpsLogWrapper ;

   class _clsVoteStatus : public SDBObject
   {
   public:
      _clsVoteStatus( _clsGroupInfo *info,
                       _netRouteAgent *agent,
                       INT32 id ) ;

      virtual ~_clsVoteStatus() ;

   public:
      virtual INT32 handleInput( const MsgHeader *header,
                                 INT32 &next ) = 0 ;

      virtual void handleTimeout( const UINT32 &millisec,
                                  INT32 &next ) = 0;

      virtual void active( INT32 &next ) = 0 ;
      virtual void deactive () {}

      virtual const CHAR *name() const { return "" ;}

   public:
      OSS_INLINE const INT32 &id()
      {
         return _id ;
      }

   protected:
      OSS_INLINE INT32 _vote()
      {
         return _launch( CLS_ELECTION_ROUND_STAGE_ONE ) ;
      }

      OSS_INLINE INT32 _announce()
      {
         return _launch( CLS_ELECTION_ROUND_STAGE_TWO ) ;
      }

      OSS_INLINE INT32 _promise( const _MsgClsElectionBallot *msg )
      {
         return _launch( msg->weights,
                         msg->identity,
                         CLS_ELECTION_ROUND_STAGE_ONE ) ;
      }

      OSS_INLINE INT32 _accept( const _MsgClsElectionBallot *msg )
      {
         return _launch( msg->weights,
                        msg->identity,
                        CLS_ELECTION_ROUND_STAGE_TWO ) ;
      }

      OSS_INLINE BOOLEAN _isAccepted()
      {
         /// must be approved by myself
         return CLS_IS_MAJORITY( _acceptedNum + 1,
                                 _groupInfo->groupSize() ) ;
      }

      OSS_INLINE UINT32 &_timeout()
      {
         return _time ;
      }

      OSS_INLINE UINT32 &_accepted()
      {
         return _acceptedNum ;
      }

      OSS_INLINE _clsGroupInfo *_info()
      {
         return _groupInfo ;
      }

   private:
      INT32 _launch( const CLS_ELECTION_ROUND &round ) ;

      INT32 _launch( const DPS_LSN &lsn,
                     const _MsgRouteID &id,
                     const CLS_ELECTION_ROUND &round ) ;
      void _broadcastAlives( void *msg ) ;
   private:
      _clsGroupInfo *_groupInfo ;
      _netRouteAgent *_agent ;
      _dpsLogWrapper *_logger ;
      INT32 _id ;
      UINT32 _time ;
      UINT32 _acceptedNum ;
   } ;
}

#endif

