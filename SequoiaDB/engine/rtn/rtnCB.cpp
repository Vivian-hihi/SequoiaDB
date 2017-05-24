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
#include "rtnContextSort.hpp"
#include "rtnContextLob.hpp"
#include "rtnContextShdOfLob.hpp"
#include "rtnContextListLob.hpp"
#include "catContextData.hpp"
#include "catContextNode.hpp"
#include "coordContext.hpp"
#include "dmsCB.hpp"
#include "rtnIxmKeySorter.hpp"
#include "../omsvc/omContextTransfer.hpp"

using namespace std;
namespace engine
{

   _SDB_RTNCB::_SDB_RTNCB()
      : _contextIdGenerator( 0 )
   {
      _enableMixCmp = FALSE ;
   }

   _SDB_RTNCB::~_SDB_RTNCB()
   {
      FOR_EACH_CMAP_ELEMENT_S( RTN_CTX_MAP, _contextMap )
      {
         SDB_OSS_DEL ((*it).second) ;
      }
      FOR_EACH_CMAP_ELEMENT_END ;

      _contextMap.clear() ;
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

      _enableMixCmp = pmdGetOptionCB()->isEnabledMixCmp() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _SDB_RTNCB::active ()
   {
      return SDB_OK ;
   }

   INT32 _SDB_RTNCB::deactive ()
   {
      return SDB_OK ;
   }

   INT32 _SDB_RTNCB::fini ()
   {
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
      BOOLEAN enableMixCmp = _enableMixCmp ;

      _enableMixCmp = pmdGetOptionCB()->isEnabledMixCmp() ;

      if ( _enableMixCmp != enableMixCmp )
      {
         // parameter changed, need to clear cached plans
         sdbGetDMSCB()->clearSUCaches() ;
      }
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

         SDB_OSS_DEL pContext ;
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
      {
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

         switch ( type )
         {
            case RTN_CONTEXT_DATA :
               (*context) = SDB_OSS_NEW rtnContextData ( _contextId,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_DUMP :
               (*context) = SDB_OSS_NEW rtnContextDump ( _contextId,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_COORD :
               (*context) = SDB_OSS_NEW rtnContextCoord ( _contextId,
                                                          pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_QGM :
               (*context) = SDB_OSS_NEW rtnContextQGM ( _contextId,
                                                        pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_TEMP :
               (*context) = SDB_OSS_NEW rtnContextTemp ( _contextId,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_SP :
               (*context) = SDB_OSS_NEW rtnContextSP ( _contextId,
                                                       pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_PARADATA :
               (*context) = SDB_OSS_NEW rtnContextParaData( _contextId,
                                                            pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_MAINCL :
               (*context) = SDB_OSS_NEW rtnContextMainCL( _contextId,
                                                         pEDUCB->getID() );
               break;
            case RTN_CONTEXT_SORT :
               (*context) = SDB_OSS_NEW rtnContextSort( _contextId,
                                                        pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_QGMSORT :
               (*context) = SDB_OSS_NEW rtnContextQgmSort( _contextId,
                                                            pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_DELCS :
               (*context) = SDB_OSS_NEW rtnContextDelCS( _contextId,
                                                            pEDUCB->getID() ) ;
               break;
            case RTN_CONTEXT_DELCL :
               (*context) = SDB_OSS_NEW rtnContextDelCL( _contextId,
                                                            pEDUCB->getID() ) ;
               break;
            case RTN_CONTEXT_DELMAINCL :
               (*context) = SDB_OSS_NEW rtnContextDelMainCL( _contextId,
                                                            pEDUCB->getID() ) ;
               break;
            case RTN_CONTEXT_EXPLAIN :
                (*context) = SDB_OSS_NEW rtnContextExplain( _contextId,
                                                            pEDUCB->getID() ) ;
                break ;
            case RTN_CONTEXT_LOB :
                 (*context) = SDB_OSS_NEW rtnContextLob( _contextId,
                                                         pEDUCB->getID() ) ;
                break ;
            case RTN_CONTEXT_SHARD_OF_LOB :
                 (*context) = SDB_OSS_NEW rtnContextShdOfLob( _contextId,
                                                              pEDUCB->getID() ) ;
                break ;
            case RTN_CONTEXT_LIST_LOB:
                 (*context) = SDB_OSS_NEW rtnContextListLob( _contextId,
                                                             pEDUCB->getID() ) ;
                break ;
            case RTN_CONTEXT_OM_TRANSFER:
                 (*context) = SDB_OSS_NEW omContextTransfer( _contextId,
                                                             pEDUCB->getID() ) ;
                 break;
            case RTN_CONTEXT_LOB_FETCHER:
                 (*context) = SDB_OSS_NEW rtnContextLobFetcher( _contextId,
                                                                pEDUCB->getID() ) ;
                 break ;

            /// Catalog contexts
            case RTN_CONTEXT_CAT_DROP_CS :
               (*context) = SDB_OSS_NEW catCtxDropCS( _contextId,
                                                      pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_CREATE_CL :
               (*context) = SDB_OSS_NEW catCtxCreateCL( _contextId,
                                                        pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_DROP_CL :
               (*context) = SDB_OSS_NEW catCtxDropCL( _contextId,
                                                      pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_ALTER_CL :
               (*context) = SDB_OSS_NEW catCtxAlterCL( _contextId,
                                                       pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_LINK_CL :
               (*context) = SDB_OSS_NEW catCtxLinkCL( _contextId,
                                                      pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_UNLINK_CL :
               (*context) = SDB_OSS_NEW catCtxUnlinkCL( _contextId,
                                                        pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_CREATE_IDX :
               (*context) = SDB_OSS_NEW catCtxCreateIdx( _contextId,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_DROP_IDX :
               (*context) = SDB_OSS_NEW catCtxDropIdx( _contextId,
                                                       pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_ACTIVE_GROUP :
               (*context) = SDB_OSS_NEW catCtxActiveGrp( _contextId,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_SHUTDOWN_GROUP :
               (*context) = SDB_OSS_NEW catCtxShutdownGrp( _contextId,
                                                           pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_REMOVE_GROUP :
               (*context) = SDB_OSS_NEW catCtxRemoveGrp( _contextId,
                                                         pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_CREATE_NODE :
               (*context) = SDB_OSS_NEW catCtxCreateNode( _contextId,
                                                          pEDUCB->getID() ) ;
               break ;
            case RTN_CONTEXT_CAT_REMOVE_NODE :
               (*context) = SDB_OSS_NEW catCtxRemoveNode( _contextId,
                                                          pEDUCB->getID() ) ;
               break ;

            default :
               PD_LOG( PDERROR, "Unknown context type: %d", type ) ;
               return SDB_SYS ;
         }

         if ( !(*context) )
         {
            return SDB_OOM ;
         }

         _contextMap.insert( _contextId, *context ) ;
         pEDUCB->contextInsert( _contextId ) ;
         contextID = _contextId ;
      }

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

