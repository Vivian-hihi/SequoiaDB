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

   Source File Name = coordContext.cpp

   Descriptive Name = Coord Context

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains Runtime Context helper
   functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/04/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "coordContext.hpp"
#include "pmd.hpp"
#include "coordCB.hpp"
#include "coordUtil.hpp"
#include "coordRemoteSession.hpp"
#include "msgMessageFormat.hpp"
#include "msgMessage.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

using namespace bson ;

namespace engine
{

   /*
      _rtnContextCoord implement
   */

   RTN_CTX_AUTO_REGISTER(_rtnContextCoord, RTN_CONTEXT_COORD, "COORD")

   _rtnContextCoord::_rtnContextCoord( INT64 contextID, UINT64 eduID,
                                       BOOLEAN preRead )
   :_rtnContextMain( contextID, eduID )
   {
      _preRead          = preRead ;
      _needReOrder      = FALSE ;

      _pSite            = NULL ;
      _pSession         = NULL ;
   }

   _rtnContextCoord::~_rtnContextCoord ()
   {
      pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      killSubContexts( cb ) ;

      if ( _pSession )
      {
         _pSite->removeSession( _pSession->sessionID() ) ;
      }
   }

   void _rtnContextCoord::getErrorInfo( INT32 rc,
                                        pmdEDUCB *cb,
                                        rtnContextBuf &buffObj )
   {
      if ( rc && _nokRC.size() > 0 )
      {
         CoordCB *pCoord = pmdGetKRCB()->getCoordCB() ;
         coordResource *pResource = pCoord->getResource() ;
         buffObj = coordBuildErrorObj( pResource, rc, cb, &_nokRC ) ;
      }
   }

   UINT32 _rtnContextCoord::getCachedRecordNum()
   {
      UINT32 recordNum = 0 ;
      SUB_ORDERED_CTX_MAP::iterator it = _orderedContextMap.begin() ;
      while( it != _orderedContextMap.end() )
      {
         rtnSubContext *pSub = it->second ;
         recordNum += pSub->recordNum() ;
         ++it ;
      }
      if ( _numToSkip > recordNum )
      {
         recordNum = 0 ;
      }
      else if ( _numToSkip > 0 )
      {
         recordNum -= _numToSkip ;
      }

      return recordNum + _rtnContextBase::getCachedRecordNum() ;
   }

   void _rtnContextCoord::killSubContexts( pmdEDUCB * cb )
   {
      UINT32 tid = 0 ;
      coordSubContext *pSubContext  = NULL ;
      pmdSubSession *pSub = NULL ;

      if ( cb )
      {
         tid = cb->getTID() ;
         // get all pre-read reply
         _getPrepareNodesData( cb, TRUE ) ;
      }

      // push all ordered context to prepare map
      SUB_ORDERED_CTX_MAP::iterator itSub = _orderedContextMap.begin() ;
      while ( _orderedContextMap.end() != itSub )
      {
         rtnSubContext* rtnSubCtx = itSub->second ;
         pSubContext = dynamic_cast<coordSubContext*>( rtnSubCtx ) ;
         _prepareContextMap.insert( EMPTY_CONTEXT_MAP::value_type(
                                    pSubContext->getRouteID().value,
                                    pSubContext ) ) ;
         ++itSub ;
      }
      _orderedContextMap.clear() ;

      EMPTY_CONTEXT_MAP::iterator it = _emptyContextMap.begin() ;
      while ( it != _emptyContextMap.end() )
      {
         _prepareContextMap.insert( EMPTY_CONTEXT_MAP::value_type(
                                    it->first,
                                    it->second ) ) ;
         ++it ;
      }
      _emptyContextMap.clear() ;

      // kill sub context
      if ( cb && !cb->isInterrupted() )
      {
         MsgOpKillContexts killMsg ;
         MsgRouteID routeID ;
         killMsg.header.messageLength = sizeof ( MsgOpKillContexts ) ;
         killMsg.header.opCode = MSG_BS_KILL_CONTEXT_REQ ;
         killMsg.header.TID = tid ;
         killMsg.header.routeID.value = 0;
         killMsg.ZERO = 0;
         killMsg.numContexts = 1 ;

         it = _prepareContextMap.begin() ;
         while ( it != _prepareContextMap.end() )
         {
            SINT64 contextID = -1 ;
            pSubContext = it->second ;
            contextID = pSubContext->contextId() ;
            if ( -1 == contextID )
            {
               // Ignore invalid context ID
               ++it ;
               continue ;
            }
            routeID = pSubContext->getRouteID() ;
            killMsg.contextIDs[0] = contextID ;

            PD_LOG( PDDEBUG, "Send kill context[ContextID:%lld] to node[%s]",
                    contextID, routeID2String( routeID ).c_str() ) ;

            pSub = _pSession->addSubSession( routeID.value ) ;
            pSub->setReqMsg( (MsgHeader*)&killMsg, PMD_EDU_MEM_NONE ) ;
            _pSession->sendMsg( pSub ) ;

            ++it ;
         }

         if ( _prepareContextMap.size() > 0 )
         {
            /// recv reply
            _pSession->waitReply1( TRUE ) ;
         }
      }

      // release all context
      it = _prepareContextMap.begin() ;
      while ( it != _prepareContextMap.end() )
      {
         pSubContext = it->second ;
         SDB_OSS_DEL pSubContext ;
         ++it ;
      }
      _prepareContextMap.clear() ;
   }

   std::string _rtnContextCoord::name() const
   {
      return "COORD" ;
   }

   RTN_CONTEXT_TYPE _rtnContextCoord::getType () const
   {
      return RTN_CONTEXT_COORD ;
   }

   INT32 _rtnContextCoord::open( const BSONObj &orderBy,
                                 const BSONObj &selector,
                                 INT64 numToReturn,
                                 INT64 numToSkip,
                                 BOOLEAN preRead )
   {
      INT32 rc = SDB_OK ;
      pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      coordSessionPropSite *pPropSite = NULL ;
      INT64 timeout = -1 ;

      if ( _isOpened )
      {
         rc = SDB_DMS_CONTEXT_IS_OPEN ;
         goto error ;
      }

      _pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;
      if ( !_pSite )
      {
         PD_LOG( PDERROR, "Session[%s] is invalid: remote site is NULL",
                 cb->getName() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      pPropSite = ( coordSessionPropSite* )_pSite->getUserData() ;
      if ( pPropSite )
      {
         timeout = pPropSite->getOprTimeout() ;
      }
      _pSession = _pSite->addSession( timeout, NULL ) ;
      if ( !_pSession )
      {
         PD_LOG( PDERROR, "Create remote session failed in session[%s]",
                 cb->getName() ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      _orderBy = orderBy.getOwned() ;
      _numToReturn = numToReturn ;
      _numToSkip = numToSkip > 0 ? numToSkip : 0 ;
      _preRead = preRead ;

      _keyGen = SDB_OSS_NEW _ixmIndexKeyGen( _orderBy ) ;
      PD_CHECK( _keyGen != NULL, SDB_OOM, error, PDERROR,
               "malloc failed!" ) ;
      if ( !selector.isEmpty() )
      {
         rc = _selector.loadPattern ( selector ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to load selector pattern rc: %d",
                    rc ) ;
            goto error ;
         }
      }

      _isOpened = TRUE ;
      _hitEnd = FALSE ;

      if ( 0 == _numToReturn )
      {
         _hitEnd = TRUE ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::reopen ()
   {
      if ( _isOpened )
      {
         return SDB_DMS_CONTEXT_IS_OPEN ;
      }
      if ( !eof() || !isEmpty() )
      {
         return SDB_SYS ;
      }

      _nokRC.clear() ;
      _resetTotalRecords( numRecords() ) ;
      _isOpened = TRUE ;

      return SDB_OK ;
   }

   INT32 _rtnContextCoord::_send2EmptyNodes( pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      MsgOpGetMore msgReq ;
      MsgRouteID routeID ;
      EMPTY_CONTEXT_MAP::iterator emptyIter ;
      pmdSubSession *pSub = NULL ;

      if ( _emptyContextMap.size() == 0 )
      {
         goto done ;
      }

      msgFillGetMoreMsg( msgReq, cb->getTID(), -1, -1, 0 ) ;

      emptyIter = _emptyContextMap.begin() ;
      while( emptyIter != _emptyContextMap.end() )
      {
         if ( -1 == emptyIter->second->contextId() )
         {
            SDB_OSS_DEL emptyIter->second ;
            emptyIter = _emptyContextMap.erase( emptyIter ) ;
            continue ;
         }

         routeID.value = emptyIter->first ;
         msgReq.contextID = emptyIter->second->contextId() ;

         pSub = _pSession->addSubSession( routeID.value ) ;
         pSub->setReqMsg( (MsgHeader*)&msgReq, PMD_EDU_MEM_NONE ) ;

         rc = _pSession->sendMsg( pSub ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Send get more message[ContextID:%lld] to "
                    "node[%s] failed, rc: %d", msgReq.contextID,
                    routeID2String( routeID ).c_str(), rc ) ;
            goto error ;
         }
         else
         {
            PD_LOG( PDDEBUG, "Send get more message[ContextID:%lld] to "
                    "node[%s] succeed", msgReq.contextID,
                    routeID2String( routeID ).c_str() ) ;
         }

         _prepareContextMap.insert( EMPTY_CONTEXT_MAP::value_type(
                                    emptyIter->first, emptyIter->second ) ) ;
         emptyIter = _emptyContextMap.erase( emptyIter ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::_getPrepareNodesData( pmdEDUCB * cb,
                                                 BOOLEAN waitAll )
   {
      INT32 rc = SDB_OK ;
      MsgOpReply *pReply = NULL ;

      pmdSubSession *pSub = NULL ;
      pmdSubSessionItr itr ;

      rc = _pSession->waitReply1( waitAll ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Wait reply failed, rc: %d", rc ) ;
         goto error ;
      }

      itr = _pSession->getSubSessionItr( PMD_SSITR_REPLY ) ;
      while ( itr.more() )
      {
         pSub = itr.next() ;
         pReply = ( MsgOpReply* )pSub->getRspMsg( TRUE ) ;
         pSub->resetForResend() ;

         if ( pReply->header.messageLength < (INT32)sizeof( MsgOpReply ) )
         {
            _delPrepareContext( pReply->header.routeID ) ;
            rc = SDB_INVALIDARG ;
            PD_LOG ( PDERROR, "Get data failed, received invalid message "
                     "from node(groupID=%u, nodeID=%u, serviceID=%u, "
                     "messageLength=%d)",
                     pReply->header.routeID.columns.groupID,
                     pReply->header.routeID.columns.nodeID,
                     pReply->header.routeID.columns.serviceID,
                     pReply->header.messageLength ) ;
            break ;
         }
         else if ( pReply->flags )
         {
            _delPrepareContext( pReply->header.routeID ) ;

            if ( SDB_DMS_EOC != pReply->flags )
            {
               PD_LOG ( PDERROR, "Get data failed, failed to get data "
                        "from node (groupID=%u, nodeID=%u, serviceID=%u, "
                        "flag=%d)", pReply->header.routeID.columns.groupID,
                        pReply->header.routeID.columns.nodeID,
                        pReply->header.routeID.columns.serviceID,
                        pReply->flags ) ;
               rc = pReply->flags ;
               _nokRC[ pReply->header.routeID.value ] =
                  coordErrorInfo( pReply ) ;
               break ;
            }
            else
            {
               // release data
               SDB_OSS_FREE( (CHAR*)pReply ) ;
               pReply = NULL ;
            }
         }
         else
         {
            rc = _appendSubData( (CHAR*)pReply ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to append the data, rc: %d", rc ) ;
               break ;
            }
            pReply = NULL ;
         }
      } // end while

      if ( rc )
      {
         goto error ;
      }

   done:
      if ( pReply )
      {
         SDB_OSS_FREE( (CHAR*)pReply ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextCoord::_toString( stringstream &ss )
   {
      if ( !_orderBy.isEmpty() )
      {
         ss << ",Orderby:" << _orderBy.toString().c_str() ;
      }
      if ( _numToReturn > 0 )
      {
         ss << ",NumToReturn:" << _numToReturn ;
      }
      if ( _numToSkip > 0 )
      {
         ss << ",NumToSkip:" << _numToSkip ;
      }
   }

   INT32 _rtnContextCoord::_reOrderSubContext()
   {
      INT32 rc = SDB_OK ;

      if ( _needReOrder && 1 == _orderedContextMap.size() && requireOrder() )
      {
         rtnSubContext* subCtx = _orderedContextMap.begin()->second ;

         _orderedContextMap.clear() ;

         rc = _saveNonEmptyOrderedSubCtx( subCtx ) ;
         if ( rc != SDB_OK )
         {
            SDB_OSS_DEL subCtx ;
            PD_LOG ( PDERROR, "Failed to get orderKey failed, rc: %d", rc ) ;
            goto error ;
         }
      }

   done:
      _needReOrder = FALSE ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::_appendSubData( CHAR * pData )
   {
      INT32 rc = SDB_OK ;
      MsgOpReply *pReply = (MsgOpReply *)pData ;
      EMPTY_CONTEXT_MAP::iterator iter ;
      coordSubContext *pSubContext = NULL ;

      if ( pReply->header.opCode != MSG_BS_GETMORE_RES ||
           (UINT32)pReply->header.messageLength < sizeof( MsgOpReply ) )
      {
         rc = SDB_UNKNOWN_MESSAGE ;
         PD_LOG ( PDERROR, "Failed to append the data, invalid data"
                  "(opCode=%d, messageLength=%d)", pReply->header.opCode,
                  pReply->header.messageLength ) ;
         goto error ;
      }

      iter = _prepareContextMap.find( pReply->header.routeID.value ) ;
      if ( _prepareContextMap.end() == iter )
      {
         rc = SDB_INVALIDARG;
         PD_LOG ( PDERROR, "Failed to append the data, no match context"
                  "(groupID=%u, nodeID=%u, serviceID=%u)",
                  pReply->header.routeID.columns.groupID,
                  pReply->header.routeID.columns.nodeID,
                  pReply->header.routeID.columns.serviceID ) ;
         goto error ;
      }

      pSubContext = iter->second ;
      SDB_ASSERT( pSubContext != NULL, "subContext can't be NULL" ) ;

      if ( pSubContext->contextId() != pReply->contextID )
      {
         rc = SDB_INVALIDARG;
         PD_LOG ( PDERROR, "Failed to append the data, no match context"
                  "(expectContextID=%lld, contextID=%lld)",
                  pSubContext->contextId(), pReply->contextID ) ;
         goto error ;
      }

      // after appendData success, the data-pointer is manage by subContext.
      // if the data-pointer will be delete by others, the clearData should be
      // called first.
      pSubContext->appendData( pReply ) ;

      if ( !requireOrder() )
      {
         _prepareContextMap.erase( iter ) ;

         try
         {
            _orderedContextMap.insert(
               SUB_ORDERED_CTX_MAP::value_type( _emptyKey, pSubContext ) ) ;
         }
         catch( std::exception& e )
         {
            rc = SDB_SYS ;
            pSubContext->clearData() ;
            SDB_OSS_DEL pSubContext ;
            PD_LOG( PDERROR, "occur unexpected error:%s", e.what() );
            goto error ;
         }
      }
      else
      {
         if ( _needReOrder )
         {
            rc = _reOrderSubContext() ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Re-order sub context last record "
                       "failed, rc: %d", rc ) ;
               goto error ;
            }
         }

         _prepareContextMap.erase( iter ) ;

         rc = _saveNonEmptyOrderedSubCtx( pSubContext ) ;
         if ( rc != SDB_OK )
         {
            pSubContext->clearData() ;
            SDB_OSS_DEL pSubContext ;
            PD_LOG ( PDERROR, "Failed to save sub ctx by order, rc=%d", rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::addSubContext( MsgRouteID routeID,
                                          SINT64 contextID )
   {
      INT32 rc = SDB_OK ;
      EMPTY_CONTEXT_MAP::iterator iter ;
      coordSubContext *pSubContext = NULL ;

      if ( !_isOpened || NULL == _pSession )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE ;
         goto error ;
      }

      iter = _emptyContextMap.find( routeID.value ) ;
      if ( iter != _emptyContextMap.end() )
      {
         rc = SDB_INVALIDARG;
         PD_LOG( PDERROR, "Repeat to add sub-context (groupID=%u, nodeID=%u, "
                 "serviceID=%u, oldContextID=%lld, newContextID=%lld)",
                 routeID.columns.groupID, routeID.columns.nodeID,
                 routeID.columns.serviceID, iter->second->contextId(),
                 contextID ) ;
         goto error ;
      }

      pSubContext = SDB_OSS_NEW coordSubContext( _orderBy,
                                                 _keyGen,
                                                 contextID,
                                                 routeID ) ;
      if ( NULL == pSubContext )
      {
         rc = SDB_OOM;
         PD_LOG ( PDERROR, "Failed to alloc memory" ) ;
         goto error ;
      }

      _emptyContextMap.insert( EMPTY_CONTEXT_MAP::value_type( routeID.value,
                               pSubContext ) ) ;

      PD_LOG( PDDEBUG,
              "add sub context (groupID=%u, nodeID=%u, contextID=%lld)",
              routeID.columns.groupID,
              routeID.columns.nodeID,
              contextID) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::addSubContext( MsgOpReply * pReply,
                                          BOOLEAN & takeOver )
   {
      INT32 rc = SDB_OK ;
      takeOver = FALSE ;
      BOOLEAN isEmpty = FALSE ;

      SDB_ASSERT ( NULL != pReply, "reply can't be NULL" ) ;

      if ( _orderedContextMap.empty() && _emptyContextMap.empty() &&
           _prepareContextMap.empty() )
      {
         isEmpty = TRUE ;
      }

      rc = addSubContext( pReply->header.routeID, pReply->contextID ) ;
      if ( rc )
      {
         goto error ;
      }

      // query with return data
      if ( pReply->numReturned > 0 )
      {
         EMPTY_CONTEXT_MAP::iterator it ;
         it = _emptyContextMap.find( pReply->header.routeID.value ) ;
         SDB_ASSERT( it != _emptyContextMap.end(), "System error" ) ;

         if ( !_needReOrder && isEmpty )
         {
            _needReOrder = TRUE ;
         }

         _prepareContextMap.insert( EMPTY_CONTEXT_MAP::value_type(
                                    it->first, it->second ) ) ;
         _emptyContextMap.erase( it ) ;

         pReply->header.opCode = MSG_BS_GETMORE_RES ;
         rc = _appendSubData( (CHAR*)pReply ) ;
         if ( SDB_OK == rc )
         {
            takeOver = TRUE ;
         }
         else
         {
            PD_LOG( PDERROR, "Append sub data failed, rc: %d", rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextCoord::addSubDone( pmdEDUCB * cb )
   {
      _send2EmptyNodes( cb ) ;
   }

   void _rtnContextCoord::_delPrepareContext( const MsgRouteID & routeID )
   {
      EMPTY_CONTEXT_MAP::iterator iter =
         _prepareContextMap.find( routeID.value ) ;

      if ( iter != _prepareContextMap.end() )
      {
         coordSubContext *pSubContext = iter->second ;

         if ( pSubContext != NULL )
         {
            SDB_OSS_DEL pSubContext ;
         }
         _prepareContextMap.erase ( iter ) ;
      }
   }

   BOOLEAN _rtnContextCoord::_requireExplicitSorting () const
   {
      return requireOrder() ;
   }

   INT32 _rtnContextCoord::_prepareSubCtxData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      if ( _emptyContextMap.size() == 0 &&
           _prepareContextMap.size() == 0 )
      {
         goto done ;
      }

      rc = _send2EmptyNodes( cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Send request to empty nodes failed, rc: %d",
                   rc ) ;

      rc = _getPrepareNodesData( cb, requireOrder() ) ;
      PD_RC_CHECK( rc, PDERROR, "Get data from prepare nodes failed, rc: %d",
                   rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::_prepareAllSubCtxDataByOrder( _pmdEDUCB *cb )
   {
      return _prepareSubCtxData( cb ) ;
   }
   
   INT32 _rtnContextCoord::_saveEmptyOrderedSubCtx( rtnSubContext* subCtx )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( NULL != subCtx, "subCtx should be not null" ) ;

      try
      {
         coordSubContext* coordSubCtx = dynamic_cast<coordSubContext*>( subCtx ) ;
         _emptyContextMap.insert(
            EMPTY_CONTEXT_MAP::value_type(
               coordSubCtx->getRouteID().value, coordSubCtx ) ) ;
      }
      catch( std::exception& e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "occur unexpected error:%s", e.what() );
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::_getNonEmptyNormalSubCtx( _pmdEDUCB *cb, rtnSubContext*& subCtx )
   {
      INT32 rc = SDB_OK ;
      SUB_ORDERED_CTX_MAP::iterator iter ;

      subCtx = NULL ;

      while ( _orderedContextMap.size() == 0 )
      {
         if ( _emptyContextMap.size() + _prepareContextMap.size() == 0 )
         {
            rc = SDB_DMS_EOC ;
            goto error ;
         }

         rc = _prepareSubCtxData( cb ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }

      SDB_ASSERT( _orderedContextMap.size() != 0, "_orderedContextMap should not be empty" ) ;

      iter = _orderedContextMap.begin() ;
      subCtx = iter->second ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::_saveEmptyNormalSubCtx( rtnSubContext* subCtx )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( NULL != subCtx, "subCtx can't be NULL" ) ;
      SDB_ASSERT( subCtx->recordNum() == 0, "sub ctx is not empty" ) ;

      // move empty sub ctx from _orderedContextMap to _emptyContextMap

      // generally, subctx is first element of _orderedContextMap
      // so don't worry about performance
      for ( SUB_ORDERED_CTX_MAP::iterator iter = _orderedContextMap.begin() ;
            iter != _orderedContextMap.end() ;
            ++iter )
      {
         if ( iter->second == subCtx )
         {
            _orderedContextMap.erase ( iter ) ;
            break ;
         }
      }

      rc = _saveEmptyOrderedSubCtx( subCtx ) ;
      if ( SDB_OK != rc )
      {
         // rtnContextMain will delete subCtx
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextCoord::_saveNonEmptyNormalSubCtx( rtnSubContext* subCtx )
   {
      SDB_ASSERT( NULL != subCtx, "subCtx can't be NULL" ) ;
      SDB_ASSERT( subCtx->recordNum() > 0, "sub ctx is empty" ) ;

      // sub ctx is in _orderedContextMap,
      // no need to do anything
      return SDB_OK ;
   }

   INT32 _rtnContextCoord::_doAfterPrepareData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      if ( _numToReturn != 0 && !_hitEnd && _preRead )
      {
         rc = _send2EmptyNodes( cb ) ;
      }

      return rc ;
   }

   /*
      _coordSubContext implement
   */
   _coordSubContext::_coordSubContext ( BSONObj& orderBy,
                              _ixmIndexKeyGen* keyGen,
                              INT64 contextID,
                              MsgRouteID routeID )
   : _rtnSubContext( orderBy, keyGen, contextID ),
     _routeID( routeID )
   {
      _pData = NULL ;
      _curOffset = 0 ;
      _recordNum = 0 ;
   }

   _coordSubContext::~_coordSubContext ()
   {
      if ( NULL != _pData )
      {
         SDB_OSS_FREE ( _pData ) ;
         _pData = NULL;
      }
   }

   INT32 _coordSubContext::remainLength()
   {
      if ( _pData->header.messageLength > _curOffset )
      {
         return _pData->header.messageLength - _curOffset ;
      }
      return 0 ;
   }

   INT32 _coordSubContext::truncate ( INT32 num )
   {
      INT32 rc = SDB_OK ;
      INT32 offset = _curOffset ;
      INT32 recordNum = 0 ;

      if ( num >= _recordNum )
      {
         goto done ;
      }

      while( ossAlign4( (UINT32)offset ) < (UINT32)_pData->header.messageLength &&
             recordNum < num )
      {
         offset = ossAlign4( (UINT32)offset ) ;
         try
         {
            BSONObj objTemp( (CHAR *)_pData + offset ) ;
            offset += objTemp.objsize() ;
            ++recordNum ;
         }
         catch( std::exception& e )
         {
            PD_LOG( PDERROR, "Failed to create bson object: %s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }

      if ( offset < _pData->header.messageLength )
      {
         _pData->header.messageLength = offset ;
      }

      if ( recordNum < _recordNum )
      {
         _recordNum = recordNum ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COSUBCON_APPENDDATA, "coordSubContext::appendData" )
   void _coordSubContext::appendData( MsgOpReply * pReply )
   {
      PD_TRACE_ENTRY ( SDB_COSUBCON_APPENDDATA ) ;
      SDB_ASSERT( pReply != NULL, "pReply can't be NULL" ) ;

      if ( _pData != NULL )
      {
         SDB_ASSERT ( _recordNum <= 0, "the buffer must be empty" ) ;
         SDB_OSS_FREE( _pData ) ;
      }
      _routeID = pReply->header.routeID ;
      _pData = pReply ;
      _recordNum = pReply->numReturned ;
      _curOffset = ossAlign4( (UINT32)sizeof( MsgOpReply ) ) ;
      _isOrderKeyChange = TRUE ;
      PD_TRACE_EXIT ( SDB_COSUBCON_APPENDDATA ) ;
   }

   void _coordSubContext::clearData()
   {
      _pData = NULL; //don't delete it, the fun-caller will delete it
      _curOffset = 0;
      _recordNum = 0;
      _isOrderKeyChange = TRUE;
   }

   MsgRouteID _coordSubContext::getRouteID()
   {
      return _routeID;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COSUBCON_FRONT, "coordSubContext::front" )
   const CHAR* _coordSubContext::front ()
   {
      PD_TRACE_ENTRY ( SDB_COSUBCON_FRONT ) ;
      if ( _recordNum > 0 && _pData->header.messageLength > _curOffset )
      {
         PD_TRACE_EXIT ( SDB_COSUBCON_FRONT ) ;
         return ( (CHAR *)_pData + _curOffset ) ;
      }
      else
      {
         PD_TRACE_EXIT ( SDB_COSUBCON_FRONT ) ;
         return NULL ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COSUBCON_POP, "coordSubContext::pop" )
   INT32 _coordSubContext::pop()
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_COSUBCON_POP ) ;
      do
      {
         if ( _curOffset >= _pData->header.messageLength )
         {
            SDB_ASSERT( FALSE, "data-buffer is empty!" );
            rc = SDB_RTN_COORD_CACHE_EMPTY ;
            PD_LOG ( PDWARNING, "Failed to pop the data, reach the end of the "
                     "buffer" ) ;
            break;
         }
         try
         {
            BSONObj boRecord( (CHAR *)_pData + _curOffset ) ;
            _curOffset += boRecord.objsize() ;
            _curOffset = ossAlign4( (UINT32)_curOffset ) ;
            _isOrderKeyChange = TRUE ;
            --_recordNum ;
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR, "Failed to pop the data, occur unexpected "
                     "error(%s)", e.what() ) ;
         }
      }while ( FALSE ) ;

      PD_TRACE_EXITRC ( SDB_COSUBCON_POP, rc ) ;
      return rc;
   }

   INT32 _coordSubContext::popN( INT32 num )
   {
      INT32 rc = SDB_OK ;
      while ( num > 0 )
      {
         rc = pop() ;
         if ( rc != SDB_OK )
         {
            break ;
         }
         --num ;
      }
      return rc;
   }

   INT32 _coordSubContext::popAll()
   {
      _recordNum = 0 ;
      _curOffset = _pData->header.messageLength ;
      _isOrderKeyChange = TRUE ;
      return SDB_OK ;
   }

   INT32 _coordSubContext::recordNum()
   {
      return _recordNum ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COSUBCON_GETORDERKEY, "coordSubContext::getOrderKey" )
   INT32 _coordSubContext::getOrderKey( rtnOrderKey &orderKey )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_COSUBCON_GETORDERKEY ) ;
      do
      {
         if ( !_isOrderKeyChange )
         {
            break ;
         }
         if ( _recordNum <= 0 )
         {
            _orderKey.clear() ;
            break ;
         }
         try
         {
            BSONObj boRecord( (CHAR *)_pData + _curOffset ) ;
            rc = _orderKey.generateKey( boRecord, _keyGen ) ;
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR, "Failed to get order-key(rc=%d)", rc ) ;
               break ;
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR, "Failed to get order-key, occur unexpected "
                     "error:%s", e.what() ) ;
            break ;
         }
      }while ( FALSE ) ;

      if ( SDB_OK == rc )
      {
         orderKey = _orderKey ;
      }

      PD_TRACE_EXITRC ( SDB_COSUBCON_GETORDERKEY, rc ) ;
      return rc;
   }
}

