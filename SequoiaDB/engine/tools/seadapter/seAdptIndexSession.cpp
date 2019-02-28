/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = seAdptIndexSession.cpp

   Descriptive Name = Index session on search engine adapter.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#include "msgMessage.hpp"
#include "seAdptIndexSession.hpp"
#include "seAdptMgr.hpp"

#define SEADPT_TID(sessionID)        ((UINT32)(sessionID & 0xFFFFFFFF))

namespace seadapter
{
   BEGIN_OBJ_MSG_MAP( _seAdptIndexSession, _pmdAsyncSession)
      ON_MSG( MSG_BS_QUERY_RES, handleQueryRes )
      ON_MSG( MSG_BS_GETMORE_RES, handleGetMoreRes )
      ON_MSG( MSG_CAT_QUERY_CATALOG_RSP, handleCatalogRes )
   END_OBJ_MSG_MAP()

   _seAdptIndexSession::_seAdptIndexSession( UINT64 sessionID,
                                             const seIndexMeta *idxMeta )
   : _pmdAsyncSession( sessionID ),
     _quit( FALSE ),
     _initialized( FALSE ),
     _meta( *idxMeta ),
     _stateInstance( NULL ),
     _targetState( CONSULT ),
     _dbAssist( NULL ),
     _searchEngine( NULL ),
     _tid( 0 ),
     _requestID( 0 ),
     _expectLID( SEADPT_INVALID_LID ),
     _lastExpectLID( SEADPT_INVALID_LID ),
     _lastCleanID( SEADPT_INVALID_LID )
   {
      SDB_ASSERT( idxMeta && idxMeta->getIdxDef().valid()
                  && !idxMeta->getIdxDef().isEmpty(),
                  "Index definition is invalid" ) ;
   }

   _seAdptIndexSession::~_seAdptIndexSession()
   {
      _cleanup() ;
   }

   SDB_SESSION_TYPE _seAdptIndexSession::sessionType() const
   {
      return SDB_SESSION_SE_INDEX ;
   }

   EDU_TYPES _seAdptIndexSession::eduType() const
   {
      return EDU_TYPE_SE_INDEX ;
   }

   INT32 _seAdptIndexSession::handleQueryRes( NET_HANDLE handle,
                                              MsgHeader* msg )
   {
      INT32 rc = SDB_OK ;

      if ( msg->requestID != currentRequestID() )
      {
         goto done ;
      }

      rc = _stateInstance->processQueryRes( this, handle, msg );
      PD_RC_CHECK( rc, PDERROR, "Process query reply failed[%d]", rc ) ;

      if ( _needStateTransition() )
      {
         rc = _stateTransition();
         PD_RC_CHECK( rc, PDERROR, "Indexer state transition failed[%d]", rc ) ;
      }

   done:
      return rc ;
   error:
      _quit = TRUE ;
      goto done ;
   }

   // When some records are fetched from data node, different operation will be
   // done based on the status.
   INT32 _seAdptIndexSession::handleGetMoreRes( NET_HANDLE handle,
                                                MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;

      if ( msg->requestID != currentRequestID() )
      {
         goto done ;
      }

      rc = _stateInstance->processGetMoreRes( this, handle, msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Process getmore reply failed[%d]", rc ) ;

      if ( _needStateTransition() )
      {
         rc = _stateTransition();
         PD_RC_CHECK( rc, PDERROR, "Indexer state transition failed[%d]", rc ) ;
      }

   done:
      return rc ;
   error:
      _quit = TRUE ;
      goto done ;
   }

   INT32 _seAdptIndexSession::handleCatalogRes( NET_HANDLE handle,
                                                MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;

      if ( msg->requestID != currentRequestID() )
      {
         goto done ;
      }

      rc = _stateInstance->processQueryRes( this, handle, msg );
      PD_RC_CHECK( rc, PDERROR, "Process catalog query reply failed[%d]", rc ) ;

      if ( _needStateTransition() )
      {
         rc = _stateTransition();
         PD_RC_CHECK( rc, PDERROR, "Indexer state transition failed[%d]", rc ) ;
      }

   done:
      return rc ;
   error:
      _quit = TRUE ;
      goto done ;
   }

   BOOLEAN _seAdptIndexSession::timeout( UINT32 interval )
   {
      return _quit ;
   }

   // Called by pmdAsyncSessionAgentEntryPoint. It will be invoked every one
   // second.
   void _seAdptIndexSession::onTimer( UINT64 timerID, UINT32 interval )
   {
      INT32 rc = SDB_OK ;

      if ( _quit )
      {
         // Do nothing anymore and wait to be released.
         goto done ;
      }

      // Only initialize the context one time.
      if ( !_initialized )
      {
         rc = _init( sdbGetSeAdapterCB()->getDBAssist(),
                     SEADPT_TID( sessionID() ) ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Initialize indexing context failed[%d]", rc ) ;
            goto error ;
         }
      }

      rc = _stateInstance->onTimer( this, interval );
      PD_RC_CHECK( rc, PDERROR, "Process timer event failed[%d]", rc ) ;

      if ( _needStateTransition() )
      {
         rc = _stateTransition();
         PD_RC_CHECK( rc, PDERROR, "Indexer state transition failed[%d]", rc ) ;
      }

   done:
      return ;
   error:
      _quit = TRUE ;
      goto done ;
   }

   const seIndexMeta *_seAdptIndexSession::indexMeta() const
   {
      return &_meta ;
   }

   seAdptDBAssist *_seAdptIndexSession::dbAssist() const
   {
      return _dbAssist ;
   }

   seAdptSEAssist *_seAdptIndexSession::seAssist()
   {
      return &_seAssist ;
   }

   void
   _seAdptIndexSession::triggerStateTransition( SEADPT_INDEXER_STATE targetState )
   {
      _targetState = targetState ;
   }

   INT32 _seAdptIndexSession::updateCataInfo( INT64 millisec )
   {
      return sdbGetSeAdapterCB()->updateCataInfo( millisec ) ;
   }

   BOOLEAN _seAdptIndexSession::validateMeta()
   {
      return ((_seIndexSessionMgr *)_pSessionMgr)->sessionMetaCheck( _meta ) ;
   }

   void _seAdptIndexSession::_onAttach()
   {
      PD_LOG( PDEVENT, "New index task starts: original collection[ %s ], "
                       "index[ %s ], capped collection[ %s ], search engine "
                       "index[ %s ], search engine type[ %s ]",
              _meta.getOrigCLName().c_str(), _meta.getOrigIdxName().c_str(),
              _meta.getCappedCLName().c_str(), _meta.getESIdxName().c_str(),
              _meta.getESTypeName().c_str() ) ;
   }

   void _seAdptIndexSession::_onDetach()
   {
      INT32 rc = SDB_OK ;
      CHAR *msg = NULL ;
      INT32 bufSize = 0 ;

      rc = msgBuildDisconnectMsg( &msg, &bufSize, nextRequestID(), _pEDUCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build disconnect message failed[%d]", rc ) ;
      ((MsgHeader *)msg)->TID = _tid ;
      rc = _dbAssist->sendToDataNode( (const MsgHeader *)msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send disconnect message to data node "
                   "failed[%d]", rc ) ;

   done:
      if ( msg )
      {
         msgReleaseBuffer( msg, _pEDUCB ) ;
      }
      return ;
   error:
      goto done ;
   }

   INT32 _seAdptIndexSession::_init( seAdptDBAssist *dbAssist, UINT32 tid )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( dbAssist, "DB assist is NULL" ) ;

      if ( _initialized )
      {
         goto done ;
      }

      rc = sdbGetSeAdapterCB()->getSeCltFactory()->create( &_searchEngine ) ;
      PD_RC_CHECK( rc, PDERROR, "Create search engine client failed[%d]", rc ) ;

      rc = _seAssist.init( _searchEngine,
                           sdbGetSeAdptOptions()->getBulkBuffSize() ) ;
      PD_RC_CHECK( rc, PDERROR, "Initialize search engine assistant failed[%d]",
                   rc ) ;

      // On the beginning, the state should be consultation.
      _stateInstance = _getStateInstance( CONSULT ) ;
      if ( !_stateInstance )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Allocate memory for state instance failed[%d]",
                 rc ) ;
         goto error ;
      }

      _dbAssist = dbAssist ;
      _tid = tid ;
      _initialized = TRUE ;

   done:
      return rc ;
   error:
      _cleanup() ;
      goto done ;
   }

   seAdptIndexerState*
   _seAdptIndexSession::_getStateInstance( SEADPT_INDEXER_STATE state )
   {
      seAdptIndexerState *instance = NULL ;

      switch ( state )
      {
      case CONSULT:
         instance = SDB_OSS_NEW _seAdptConsultState ;
         break ;
      case FULL_INDEX:
         instance = SDB_OSS_NEW _seAdptFullIndexState ;
         break ;
      default:
         instance = SDB_OSS_NEW _seAdptIncIndexState ;
         break ;
      }

      return instance ;
   }

   BOOLEAN _seAdptIndexSession::_needStateTransition() const
   {
      return _stateInstance->type() != _targetState ;
   }

   INT32 _seAdptIndexSession::_stateTransition()
   {
      INT32 rc = SDB_OK ;
      seAdptIndexerState *target = NULL ;
      SEADPT_INDEXER_STATE oldState = _stateInstance->type() ;

      if ( _targetState == _stateInstance->type() )
      {
         goto done ;
      }

      target = _getStateInstance( _targetState ) ;
      if ( !target )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Allocate memory for indexer state failed[%d]", rc ) ;
         goto error ;
      }

      SDB_OSS_DEL _stateInstance ;
      _stateInstance = target ;

      PD_LOG( PDEVENT, "Indexer state transition done[%s => %s]",
              seAdptGetIndexerStateDesp( oldState ),
              seAdptGetIndexerStateDesp( _targetState ) ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _seAdptIndexSession::_cleanup()
   {
      _dbAssist = NULL ;
      if ( _searchEngine )
      {
         SDB_OSS_DEL _searchEngine ;
         _searchEngine = NULL ;
      }

      if ( _stateInstance )
      {
         SDB_OSS_DEL _stateInstance ;
         _stateInstance = NULL ;
      }
   }
}

