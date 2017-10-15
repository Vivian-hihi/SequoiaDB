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

   Source File Name = rtnCB.cpp

   Descriptive Name = Runtime Control Block

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   control block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "rtnCB.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "dmsCB.hpp"
#include "rtnIxmKeySorter.hpp"

#include "pmdController.hpp"

using namespace std;
namespace engine
{
   _rtnMsgHandler::_rtnMsgHandler( _pmdRemoteSessionMgr *pRSManager )
   {
      _pRSManager = pRSManager ;
   }

   _rtnMsgHandler::~_rtnMsgHandler()
   {
   }

   void _rtnMsgHandler::attach( _pmdEDUCB *cb )
   {
   }

   void _rtnMsgHandler::detach()
   {
   }

   INT32 _rtnMsgHandler::handleMsg( const NET_HANDLE &handle,
                                    const _MsgHeader *header,
                                    const CHAR *msg )
   {
      INT32 rc = SDB_OK ;

      // main cb msg
      if ( header->TID == 0 )
      {
         /*
         CHAR *pNewMsg = NULL ;
         SDB_ASSERT( _pMainCB, "Main cb can't be NULL" ) ;
         if ( !_pMainCB )
         {
            PD_LOG( PDERROR, "Main cb handler is null when recv "
                    "msg[opCode:%d]", header->opCode ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         pNewMsg = (CHAR*)SDB_OSS_MALLOC( header->messageLength + 1 ) ;
         if ( !pNewMsg )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Failed to alloc memory for msg[opCode: %d, "
                    "len: %d], rc: %d", header->opCode, header->messageLength,
                    rc ) ;
            goto error ;
         }

         // copy msg
         ossMemcpy( pNewMsg, msg, header->messageLength ) ;
         pNewMsg[ header->messageLength ] = 0 ;
         // push event
         _pMainCB->postEvent( pmdEDUEvent( PMD_EDU_EVENT_MSG,
                                           PMD_EDU_MEM_ALLOC,
                                           pNewMsg, (UINT64)handle ) ) ;
         */
      }
      // session msg
      else
      {
         SDB_ASSERT( _pRSManager, "Remote Session Manager can't be NULL" ) ;
         rc = _pRSManager->pushMessage( handle, header ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Push message[%s] failed, rc: %d",
                    msg2String( header, MSG_MASK_ALL, 0 ).c_str(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnMsgHandler::handleClose( const NET_HANDLE &handle, _MsgRouteID id )
   {
      SDB_ASSERT( _pRSManager, "Remote session manager can't be NULL" ) ;

      _pRSManager->handleClose( handle, id ) ;
   }

   void _rtnMsgHandler::handleConnect( const NET_HANDLE &handle,
                                       _MsgRouteID id,
                                       BOOLEAN isPositive )
   {
   }

   _SDB_RTNCB::_SDB_RTNCB()
      : _contextIdGenerator( 0 ),
        _textIdxVersion((INT64)RTN_INIT_TEXT_INDEX_VERSION)
   {
      _msgHandler = NULL ;
      _routeAgent = NULL ;
      _extNodeId = 0 ;
   }

   _SDB_RTNCB::~_SDB_RTNCB()
   {
      FOR_EACH_CMAP_ELEMENT_S( RTN_CTX_MAP, _contextMap )
      {
         SDB_OSS_DEL ((*it).second) ;
      }
      FOR_EACH_CMAP_ELEMENT_END ;

      _contextMap.clear() ;

      if ( _msgHandler )
      {
         SDB_OSS_DEL _msgHandler ;
      }

      if ( _routeAgent )
      {
         SDB_OSS_DEL _routeAgent ;
      }
   }

   INT32 _SDB_RTNCB::init ()
   {
      INT32 rc = SDB_OK ;

      rtnIxmKeySorterCreator* creator = SDB_OSS_NEW _rtnIxmKeySorterCreator() ;
      if ( NULL == creator )
      {
         PD_LOG ( PDERROR, "failed to create _rtnIxmKeySorterCreator" ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      sdbGetDMSCB()->setIxmKeySorterCreator( creator ) ;

      // Remote session manager should be enabled on data node to support text
      // search.
      if ( SDB_ROLE_DATA == pmdGetDBRole() )
      {
         _msgHandler = SDB_OSS_NEW rtnMsgHandler( &_rsMgr ) ;
         if ( !_msgHandler )
         {
            PD_LOG( PDERROR, "Allocate memory for message handler failed, "
                    "size[ %d ]", sizeof( rtnMsgHandler ) ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         // The route agent will be updated when the search engine adapter
         // registers.
         _routeAgent = SDB_OSS_NEW netRouteAgent( _msgHandler ) ;
         if ( !_routeAgent )
         {
            PD_LOG( PDERROR, "Allocate memory for route agent failed, "
                    "size[ %d ]", sizeof( netRouteAgent ) ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         rc = _rsMgr.init( _routeAgent, NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Init remote session manager failed[ %d ]",
                      rc ) ;
         sdbGetPMDController()->setRSManager( &_rsMgr ) ;
      }

      // The error of initialization of APM could be ignore
      // Only data and catalog nodes could initialize plan cache
      _accessPlanManager.init(
            ( SDB_ROLE_DATA == pmdGetDBRole() ||
              SDB_ROLE_CATALOG == pmdGetDBRole() ||
              SDB_ROLE_STANDALONE == pmdGetDBRole() ) ?
             pmdGetOptionCB()->getPlanBuckets() : 0,
            (OPT_PLAN_CACHE_LEVEL) pmdGetOptionCB()->getPlanCacheLevel(),
            pmdGetOptionCB()->isEnabledMixCmp() ) ;

   done:
      return rc ;
   error:
      // In case of error, we can release resources here, or leave it to the
      // fini or destructor.
      if ( _msgHandler )
      {
         SDB_OSS_DEL _msgHandler ;
         _msgHandler = NULL ;
      }
      if ( _routeAgent )
      {
         SDB_OSS_DEL _routeAgent ;
         _routeAgent = NULL ;
      }
      goto done ;
   }

   INT32 _SDB_RTNCB::active ()
   {
      INT32 rc = SDB_OK ;

      if ( SDB_ROLE_DATA == pmdGetDBRole() )
      {
         pmdEDUMgr *eduMgr = pmdGetKRCB()->getEDUMgr() ;
         EDUID eduID = PMD_INVALID_EDUID ;

         rc = eduMgr->startEDU( EDU_TYPE_RTNNETWORK, (void *)_routeAgent,
                                &eduID ) ;
         PD_RC_CHECK( rc, PDERROR, "Start external search network failed[ %d ]",
                      rc ) ;
         eduMgr->regSystemEDU( EDU_TYPE_RTNNETWORK, eduID ) ;
         rc = eduMgr->waitUntil( eduID, PMD_EDU_RUNNING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait external search net active failed[ %d ]",
                      rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _SDB_RTNCB::deactive ()
   {
      return SDB_OK ;
   }

   INT32 _SDB_RTNCB::fini ()
   {
      _accessPlanManager.clear() ;

      dmsIxmKeySorterCreator* creator = sdbGetDMSCB()->getIxmKeySorterCreator() ;
      if ( NULL != creator )
      {
         SDB_OSS_DEL( creator ) ;
         sdbGetDMSCB()->setIxmKeySorterCreator( NULL ) ;
      }

      return SDB_OK ;
   }

   void _SDB_RTNCB::onConfigChange ()
   {
      _accessPlanManager.reinit(
            (OPT_PLAN_CACHE_LEVEL) pmdGetOptionCB()->getPlanCacheLevel(),
            pmdGetOptionCB()->isEnabledMixCmp() ) ;
   }

   rtnContext* _SDB_RTNCB::contextFind ( SINT64 contextID, _pmdEDUCB *cb )
   {
      rtnContext *pContext = NULL ;
      std::pair<rtnContext*, bool> ret = _contextMap.find( contextID ) ;
      if ( ret.second )
      {
         if ( cb && !cb->contextFind( contextID ) )
         {
            PD_LOG ( PDWARNING, "Context %lld does not owned by "
                     "current session", contextID ) ;
         }
         else
         {
            pContext = ret.first ;
         }
      }
      return pContext ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_RTNCB_CONTEXTDEL, "_SDB_RTNCB::contextDelete" )
   void _SDB_RTNCB::contextDelete ( SINT64 contextID, pmdEDUCB *cb )
   {
      PD_TRACE_ENTRY ( SDB__SDB_RTNCB_CONTEXTDEL ) ;

      rtnContext *pContext = NULL ;

      if ( cb )
      {
         cb->contextDelete( contextID ) ;
      }

      {
         pair<rtnContext*, bool> ret = _contextMap.find( contextID ) ;
         if ( ret.second )
         {
            _contextMap.erase( contextID ) ;
            pContext = ret.first ;
         }
      }

      if ( pContext )
      {
         INT32 reference = pContext->getReference() ;
         pContext->waitForPrefetch() ;

         /// wait for sync
         if ( pContext->isWrite() && pContext->getDPSCB() &&
              pContext->getW() > 1 )
         {
            pContext->getDPSCB()->completeOpr( cb, pContext->getW() ) ;
         }

         sdbGetRTNContextBuilder()->release( pContext ) ;
         PD_LOG( PDDEBUG, "delete context(contextID=%lld, reference: %d)",
                 contextID, reference ) ;
      }

      PD_TRACE_EXIT ( SDB__SDB_RTNCB_CONTEXTDEL ) ;
      return ;
   }

   SINT32 _SDB_RTNCB::contextNew ( RTN_CONTEXT_TYPE type,
                                   rtnContext **context,
                                   SINT64 &contextID,
                                   _pmdEDUCB * pEDUCB )
   {
      SDB_ASSERT ( context, "context pointer can't be NULL" ) ;

      // if hit max signed 64 bit integer?
      if ( _contextIdGenerator.fetch() < 0 )
      {
         return SDB_SYS ;
      }

      INT64 _contextId = _contextIdGenerator.inc() ;
      if ( _contextId < 0 )
      {
         return SDB_SYS ;
      }

      (*context) = sdbGetRTNContextBuilder()->create(
                     type, _contextId, pEDUCB->getID() ) ;

      if ( !(*context) )
      {
         return SDB_OOM ;
      }

      _contextMap.insert( _contextId, *context ) ;
      pEDUCB->contextInsert( _contextId ) ;
      contextID = _contextId ;

      PD_LOG ( PDDEBUG, "Create new context(contextID=%lld, type: %d[%s])",
               contextID, type, getContextTypeDesp(type) ) ;

      return SDB_OK ;
   }

   /*
      get global rtn cb
   */
   SDB_RTNCB* sdbGetRTNCB ()
   {
      static SDB_RTNCB s_rtnCB ;
      return &s_rtnCB ;
   }

}

