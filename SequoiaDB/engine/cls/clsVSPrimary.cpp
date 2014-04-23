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

   Source File Name = clsVSPrimary.cpp

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

#include "clsVSPrimary.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   _clsVSPrimary::_clsVSPrimary( _clsGroupInfo *info,
                                  _netRouteAgent *agent ):
                                 _clsVoteStatus( info, agent,
                                               CLS_ELECTION_STATUS_PRIMARY)
   {

   }

   _clsVSPrimary::~_clsVSPrimary()
   {

   }

   INT32 _clsVSPrimary::handleInput( const MsgHeader *header,
                                     INT32 &next )
   {
      /// primary do not accept any request
      next = id() ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSPMY_HDTMOUT, "_clsVSPrimary::handleTimeout" )
   void _clsVSPrimary::handleTimeout( const UINT32 &millisec,
                                      INT32 &next )
   {
      PD_TRACE_ENTRY ( SDB__CLSVSPMY_HDTMOUT ) ;
      _timeout() += millisec ;
      if ( CLS_VOTE_CS_TIME <= _timeout() )
      {
         if ( CLS_IS_MAJORITY( _info()->alives.size() + 1,
                               _info()->groupSize() ) )
         {
            _timeout() = 0 ;
            next = id() ;
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
      PD_TRACE_EXIT ( SDB__CLSVSPMY_HDTMOUT ) ;
      return ;
   }

   void _clsVSPrimary::deactive ()
   {
      _MsgCatPrimaryChange msg ;

      _info()->mtx.lock_w() ;
      if ( _info()->local.value == _info()->primary.value )
      {
         _info()->primary.value = MSG_INVALID_ROUTEID ;
      }
      msg.newPrimary = _info()->primary ;
      msg.oldPrimary = _info()->local ;
      _info()->mtx.release_w() ;

      // interrupt writing edus
      pmdGetKRCB()->getEDUMgr()->interruptWritingEDUS() ;

      // stop rollback
      pmdGetKRCB()->getTransCB()->stopRollbackTask() ;
      pmdGetKRCB()->getTransCB()->termAllTrans();

      /// when we are not primary any more, we should clear
      /// waiting list.
      pmdGetKRCB()->getClsCB()->getReplCB()->syncMgr()->cut( 0 ) ;

      pmdGetKRCB()->getClsCB()->getReplCB()->callCatalog( (MsgHeader *)&msg ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSVSPMY_ACTIVE, "_clsVSPrimary::active" )
   void _clsVSPrimary::active( INT32 &next )
   {
      pmdKRCB *pKRCB = pmdGetKRCB() ;
      PD_TRACE_ENTRY ( SDB__CLSVSPMY_ACTIVE ) ;
      _timeout() = 0 ;
      next = id() ;
      _MsgCatPrimaryChange msg ;

      pmdGetKRCB()->getReplCB()->getBucket()->reset() ;

      // start trans rollback
      pKRCB->getTransCB()->startRollbackTask();

      // clear catalog info
      pKRCB->getClsCB()->getCatAgent()->lock_w() ;
      pKRCB->getClsCB()->getCatAgent()->clearAll() ;
      pKRCB->getClsCB()->getCatAgent()->release_w() ;

      _info()->mtx.lock_w() ;
      msg.newPrimary = _info()->local ;
      msg.oldPrimary = _info()->primary ;
      _info()->primary = _info()->local ;
      _info()->mtx.release_w() ;
      pmdGetKRCB()->getDPSCB()->incVersion() ;
      pmdGetKRCB()->getClsCB()->getReplCB()->callCatalog( (MsgHeader *)&msg ) ;

      PD_LOG ( PDEVENT, "Change to Primary" ) ;

      PD_TRACE_EXIT ( SDB__CLSVSPMY_ACTIVE ) ;
      return ;
   }

}
