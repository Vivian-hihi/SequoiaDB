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

