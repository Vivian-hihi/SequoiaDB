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

   Source File Name = clsSession.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "clsSession.hpp"
#include "ossMem.hpp"
#include "pmd.hpp"
#include "rtnCommand.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   // _clsSessionMeta implement
   _clsSessionMeta::_clsSessionMeta( const NET_HANDLE handle )
   :_basedHandleNum( 0 )
   {
      _netHandle = handle ;
   }

   _clsSessionMeta::~_clsSessionMeta()
   {
   }

   // _clsSession implement
   BEGIN_OBJ_MSG_MAP( _clsSession, _clsObjBase )
      //ON_MSG
   END_OBJ_MSG_MAP()

   _clsSession::_clsSession( UINT64 sessionID )
   {
      _lockFlag = FALSE ;
      _startType = CLS_SESSION_PASSIVE ;

      clear() ;

      _sessionID = sessionID ;
      _makeName () ;
      _latchIn.try_get () ;
   }

   _clsSession::~_clsSession()
   {
      clear() ;
      _latchIn.release () ;
   }

   UINT64 _clsSession::identifyID()
   {
      // TODO:XUJIANHUI
      return 0 ;
   }

   INT32 _clsSession::getServiceType() const
   {
      return CMD_SPACE_SERVICE_SHARD ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_ATHIN, "_clsSession::attachIn" )
   INT32 _clsSession::attachIn ( pmdEDUCB *cb )
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_ATHIN );
      _latchOut.try_get () ;
      _latchIn.release () ;

      SDB_ASSERT( cb, "cb can't be NULL" ) ;

      PD_LOG( PDINFO, "Session[%s] attach edu[%d]", sessionName(),
              cb->getID() ) ;

      _pEDUCB = cb ;
      _eduID  = cb->getID() ;
      _pEDUCB->setName( sessionName() ) ;
      _pEDUCB->attachSession( this ) ;

      _onAttach () ;

      PD_TRACE_EXIT ( SDB__CLSSN_ATHIN );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_ATHOUT, "_clsSession::attachOut" )
   INT32 _clsSession::attachOut ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_ATHOUT );

      PD_LOG( PDINFO, "Session[%s] detach edu[%d]", sessionName(),
              eduID() ) ;

      _onDetach () ;

      _pEDUCB->detachSession() ;
      _latchOut.release () ;
      _pEDUCB = NULL ;
      PD_TRACE_EXIT ( SDB__CLSSN_ATHOUT );
      return SDB_OK ;
   }

   BOOLEAN _clsSession::isDetached () const
   {
      return _pEDUCB ? FALSE : TRUE ;
   }

   BOOLEAN _clsSession::isAttached () const
   {
      return _pEDUCB ? TRUE : FALSE ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_CLEAR, "_clsSession::clear" )
   void _clsSession::clear()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_CLEAR );
      if ( _lockFlag )
      {
         _unlock () ;
      }

      _sessionID = INVLIAD_SESSION_ID;
      _pEDUCB = NULL ;
      _eduID = PMD_INVALID_EDUID ;
      _netHandle = NET_INVALID_HANDLE ;
      _name [0] = 0 ;
      _pMeta = NULL ;

      for ( UINT32 index = 0 ; index < MAX_BUFFER_ARRAY_SIZE; ++index )
      {
         _buffArray[index].pBuffer = NULL ;
         _buffArray[index].size = 0 ;
         _buffArray[index].useFlag = CLS_BUFF_INVALID ;
         _buffArray[index].addTime = 0 ;
      }
      _buffBegin = 0 ;
      _buffEnd = 0 ;
      _buffCount = 0 ;
      PD_TRACE_EXIT ( SDB__CLSSN_CLEAR );
   }

   void _clsSession::onRecieve ( const NET_HANDLE netHandle, MsgHeader * msg )
   {
   }

   BOOLEAN _clsSession::timeout ( UINT32 interval )
   {
      return FALSE ;
   }

   void _clsSession::_onAttach ()
   {
   }

   void _clsSession::_onDetach ()
   {
   }

   UINT64 _clsSession::sessionID () const
   {
      return _sessionID ;
   }

   void _clsSession::sessionID ( UINT64 sessionID )
   {
      _sessionID = sessionID ;
      _makeName () ;
   }

   EDUID _clsSession::eduID () const
   {
      return _eduID ;
   }

   pmdEDUCB *_clsSession::eduCB () const
   {
      return _pEDUCB ;
   }

   NET_HANDLE _clsSession::netHandle () const
   {
      return _netHandle ;
   }

   void _clsSession::meta ( clsSessionMeta * pMeta )
   {
      _pMeta = pMeta ;
      if ( _pMeta )
      {
         _netHandle = _pMeta->getHandle() ;
      }
      else
      {
         _netHandle = NET_INVALID_HANDLE ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN__MKNAME, "_clsSession::_makeName" )
   void _clsSession::_makeName ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN__MKNAME );
      UINT32 nodeID, TID ;
      ossUnpack32From64 ( _sessionID, nodeID, TID ) ;
      if ( nodeID > CLS_BASE_HANDLE_ID )
      {
         ossSnprintf( _name , SESSION_NAME_LEN, "NetID:%u,TID:%u",
                      nodeID - CLS_BASE_HANDLE_ID, TID ) ;
      }
      else
      {
         ossSnprintf( _name , SESSION_NAME_LEN, "NodeID:%u,TID:%u,Start:%s",
                      nodeID, TID, isStartActive() ? "active" : "passive" ) ;
      }
      _name [SESSION_NAME_LEN] = 0 ;
      PD_TRACE_EXIT ( SDB__CLSSN__MKNAME );
   }

   BOOLEAN _clsSession::isStartActive ()
   {
      return _startType == CLS_SESSION_ACTIVE ? TRUE : FALSE ;
   }

   void _clsSession::startType ( INT32 startType )
   {
      _startType = startType ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN__LOCK, "_clsSession::_lock" )
   INT32 _clsSession::_lock ()
   {
      INT32 rc = SDB_SYS ;
      PD_TRACE_ENTRY ( SDB__CLSSN__LOCK );
      if ( _pMeta && !_lockFlag )
      {
         _pMeta->getLatch()->get() ;
         _lockFlag = TRUE ;
         rc = SDB_OK ;
      }
      PD_TRACE_EXITRC ( SDB__CLSSN__LOCK, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN__UNLOCK, "_clsSession::_unlock" )
   INT32 _clsSession::_unlock ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN__UNLOCK );
      if ( _pMeta && _lockFlag )
      {
         _pMeta->getLatch()->release () ;
         _lockFlag = FALSE ;
      }
      PD_TRACE_EXIT ( SDB__CLSSN__UNLOCK );
      return SDB_OK ;
   }

   const CHAR *_clsSession::sessionName () const
   {
      return _name ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_WTATH, "_clsSession::waitAttach" )
   INT32 _clsSession::waitAttach ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_WTATH );
      _latchIn.get () ;
      PD_TRACE_EXIT ( SDB__CLSSN_WTATH );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_WTDTH, "_clsSession::waitDetach" )
   INT32 _clsSession::waitDetach ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_WTDTH );
      _latchOut.get () ;
      _latchOut.release () ;
      PD_TRACE_EXIT ( SDB__CLSSN_WTDTH );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_CPMSG, "_clsSession::copyMsg" )
   void * _clsSession::copyMsg( const CHAR *msg, UINT32 length )
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_CPMSG );
      void *p = NULL ;
      UINT32 buffPos = _decBuffPos ( _buffEnd ) ;
      if ( _buffArray[buffPos].isAlloc() &&
           _buffArray[buffPos].size >= length )
      {
         ossMemcpy( _buffArray[buffPos].pBuffer, msg, length ) ;
         _buffArray[buffPos].useFlag = CLS_BUFF_USING ;
         p = (void*)&_buffArray[buffPos] ;
         goto done ;
      }

      PD_LOG ( PDERROR, "Session[%s] copy msg failed[buffindex:%d, size:%d, "
               "flag:%d, message length:%d", sessionName(), buffPos,
               _buffArray[buffPos].size, _buffArray[buffPos].useFlag, length ) ;

   done :
      PD_TRACE_EXIT ( SDB__CLSSN_CPMSG );
      return p ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_FRNBUF, "_clsSession::frontBuffer" )
   clsBuffInfo *_clsSession::frontBuffer ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_FRNBUF );
      clsBuffInfo *p = NULL ;
      if ( _buffArray[_buffBegin].isInvalid() )
      {
         goto done ;
      }
      SDB_ASSERT ( _buffCount > 0 , "impossible" ) ;

      p = &_buffArray[_buffBegin] ;
   done :
      PD_TRACE_EXIT ( SDB__CLSSN_FRNBUF );
      return p ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_POPBUF, "_clsSession::popBuffer" )
   void _clsSession::popBuffer ()
   {
      PD_TRACE_ENTRY ( SDB__CLSSN_POPBUF );
      SDB_ASSERT ( _buffCount > 0 , "impossible" ) ;

      _buffArray[_buffBegin].pBuffer = NULL ;
      _buffArray[_buffBegin].size = 0 ;
      _buffArray[_buffBegin].useFlag = CLS_BUFF_INVALID ;
      _buffArray[_buffBegin].addTime = 0 ;

      --_buffCount ;
      _buffBegin = _incBuffPos( _buffBegin ) ;
      PD_TRACE_EXIT ( SDB__CLSSN_POPBUF );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSSN_PSHBUF, "_clsSession::pushBuffer" )
   INT32 _clsSession::pushBuffer ( CHAR * pBuffer, UINT32 size )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__CLSSN_PSHBUF );
      if ( _buffCount >= MAX_BUFFER_ARRAY_SIZE )
      {
         rc = SDB_CLS_BUFFER_FULL ;
         goto done ;
      }

      SDB_ASSERT ( _buffArray[_buffEnd].isInvalid (), "impossilbe" ) ;

      ++_buffCount ;
      _buffArray[_buffEnd].pBuffer = pBuffer ;
      _buffArray[_buffEnd].size = size ;
      _buffArray[_buffEnd].useFlag = CLS_BUFF_ALLOC ;
      _buffArray[_buffEnd].addTime = time( NULL ) ;

      _buffEnd = _incBuffPos( _buffEnd ) ;

   done :
      PD_TRACE_EXITRC ( SDB__CLSSN_PSHBUF, rc );
      return rc ;
   }

   UINT32 _clsSession::_incBuffPos ( UINT32 pos )
   {
      ++pos ;
      if ( pos < MAX_BUFFER_ARRAY_SIZE )
      {
         return pos ;
      }

      return 0 ;
   }

   UINT32 _clsSession::_decBuffPos ( UINT32 pos )
   {
      return pos ? pos - 1 : MAX_BUFFER_ARRAY_SIZE - 1 ;
   }

}


