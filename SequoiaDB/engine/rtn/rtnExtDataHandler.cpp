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

   Source File Name = rtnExtDataHandler.cpp

   Descriptive Name = External data process handler for rtn.

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          14/04/2017  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#include "pmd.hpp"
#include "rtn.hpp"
#include "rtnTrace.hpp"
#include "rtnExtDataHandler.hpp"
#include "rtnExtDataProcessor.hpp"

#define RTN_TEXTIDX_MAX_NUM        64

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTCONTEXTBASE__RTNEXTCONTEXTBASE, "_rtnExtContextBase::_rtnExtContextBase" )
   _rtnExtContextBase::_rtnExtContextBase( DMS_EXTOPR_TYPE type )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTCONTEXTBASE__RTNEXTCONTEXTBASE ) ;
      _id = 0 ;
      _processorMgr = NULL ;
      _type = type ;
      _processorLocked = FALSE ;
      _lockType = SHARED ;
      _processorLocked = FALSE ;
      PD_TRACE_EXIT( SDB__RTNEXTCONTEXTBASE__RTNEXTCONTEXTBASE ) ;
   }

   _rtnExtContextBase::~_rtnExtContextBase()
   {
      _cleanup() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTCONTEXTBASE_DONE, "_rtnExtContextBase::done" )
   INT32 _rtnExtContextBase::done( pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTCONTEXTBASE_DONE ) ;

      rc = _onDone( cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Operation _onDone failed[ %d ]", rc ) ;

   done:
      _cleanup() ;
      PD_TRACE_EXITRC( SDB__RTNEXTCONTEXTBASE_DONE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTCONTEXTBASE_ABORT, "_rtnExtContextBase::abort" )
   INT32 _rtnExtContextBase::abort( pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTCONTEXTBASE_ABORT ) ;

      rc = _onAbort( cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Operation _onAbort failed[ %d ]", rc ) ;

   done:
      _cleanup() ;
      PD_TRACE_EXITRC( SDB__RTNEXTCONTEXTBASE_ABORT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTCONTEXTBASE__CLEANUP, "_rtnExtContextBase::_cleanup" )
   void _rtnExtContextBase::_cleanup()
   {
      PD_TRACE_ENTRY( SDB__RTNEXTCONTEXTBASE__CLEANUP ) ;
      if ( _processorLocked )
      {
         _processorMgr->unlockProcessors( _processors, _lockType ) ;
         _processors.clear() ;
         _processorLocked = FALSE ;
      }
      PD_TRACE_EXIT( SDB__RTNEXTCONTEXTBASE__CLEANUP ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTCONTEXTBASE_APPENDPROCESSORS, "_rtnExtContextBase::appendProcessors" )
   void _rtnExtContextBase::_appendProcessors( const vector< rtnExtDataProcessor * >& processorVec )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTCONTEXTBASE_APPENDPROCESSORS ) ;
      _processors.insert( _processors.end(), processorVec.begin(),
                          processorVec.end() ) ;
      PD_TRACE_EXIT( SDB__RTNEXTCONTEXTBASE_APPENDPROCESSORS ) ;
   }

   _rtnExtRebuildIdxCtx::_rtnExtRebuildIdxCtx()
   : _rtnExtContextBase( DMS_EXTOPR_TYPE_REBUILDIDX )
   {
   }

   _rtnExtRebuildIdxCtx::~_rtnExtRebuildIdxCtx()
   {
      if ( _processorLocked )
      {
         _processorMgr->unlockProcessors( _processors, _lockType ) ;
         _processorLocked = FALSE ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTREBUILDIDXCTX_OPEN, "_rtnExtRebuildIdxCtx::open" )
   INT32 _rtnExtRebuildIdxCtx::open( rtnExtDataProcessorMgr *processorMgr,
                                     const CHAR *csName, const CHAR *clName,
                                     const CHAR *idxName,
                                     const BSONObj &idxKeyDef, pmdEDUCB *cb,
                                     SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTREBUILDIDXCTX_OPEN ) ;
      BOOLEAN newProcessor = FALSE ;
      BOOLEAN processorAdded = FALSE ;
      SDB_DB_STATUS dbStatus = pmdGetKRCB()->getDBStatus() ;
      rtnExtDataProcessor *processor = NULL ;
      std::vector<rtnExtDataProcessor *> processors ;

      // Index will be rebuilt in the following cases:
      // 1. One new index is created.
      // 2. In case of full sync.
      // 3. Rebuilding after crash.
      // If rebuild index is called when db is in normal status, it means one
      // new index is being created. In that case, processor and new capped
      // collection should be created.
      // In case of full sync, leave all the capped collections to sync by
      // themselves, because we want them to be exactly the same with the ones
      // on primary node. The processors will be deleted if the index is
      // dropped. So we need to add them again here.
      // In any other cases( rebuilding, for example ), the processors are added
      // during opening of storage. So no need to add.
      BOOLEAN create = ( SDB_DB_NORMAL == dbStatus
                         || SDB_DB_FULLSYNC == dbStatus ) ? TRUE : FALSE ;

      SDB_ASSERT( cb, "eduCB is NULL" ) ;

      _processorMgr = processorMgr ;

      rc = processorMgr->getProcessorsAndLock( csName, clName, idxName,
                                               EXCLUSIVE, processors ) ;
      PD_RC_CHECK( rc, PDERROR, "Get external processor for index "
                   "failed[ %d ]", rc ) ;
      _processorMgr = processorMgr ;

      if ( 0 == processors.size() )
      {
         if ( create )
         {
            rc = processorMgr->createProcessor( csName, clName, idxName,
                                                idxKeyDef, processor ) ;
            PD_RC_CHECK( rc, PDERROR, "Create processor for collection[ %s.%s ]"
                         " and index[ %s ] failed[ %d ]", csName, clName,
                         idxName, rc ) ;
            newProcessor = TRUE ;
            rc = processor->doRebuild( cb, NULL ) ;
            PD_RC_CHECK( rc, PDERROR, "Rebuild of index failed[ %d ]", rc ) ;

            rc = processorMgr->addProcessor( processor ) ;
            PD_RC_CHECK( rc, PDERROR, "Add processor for index failed[ %d ]",
                         rc ) ;
            processorAdded = TRUE ;
         }
         else
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Get external data processor failed[ %d ]", rc ) ;
            goto error ;
         }
      }
      else
      {
         _processorLocked = TRUE ;
         _lockType = EXCLUSIVE ;
         rc = processors.front()->doRebuild( cb, NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Rebuild of index failed[ %d ]", rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTREBUILDIDXCTX_OPEN, rc ) ;
      return rc ;
   error:
      if ( newProcessor )
      {
         if ( processorAdded )
         {
            _processorMgr->delProcessor( &processor ) ;
         }
         else
         {
            _processorMgr->destroyProcessor( processor ) ;
         }
      }
      goto done ;
   }

   _rtnExtDataOprCtx::_rtnExtDataOprCtx( DMS_EXTOPR_TYPE type )
   : _rtnExtContextBase( type )
   {
   }

   _rtnExtDataOprCtx::~_rtnExtDataOprCtx()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAOPRCTX_DONE, "_rtnExtDataOprCtx::done" )
   INT32 _rtnExtDataOprCtx::_onDone( pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAOPRCTX_DONE ) ;
      for ( EDP_VEC_ITR itr = _processors.begin(); itr != _processors.end();
            ++itr )
      {
         rc = (*itr)->done( cb ) ;
         PD_RC_CHECK( rc, PDERROR, "Processor done operation failed[ %d ]", rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAOPRCTX_DONE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   _rtnExtInsertCtx::_rtnExtInsertCtx()
   : _rtnExtDataOprCtx( DMS_EXTOPR_TYPE_INSERT )
   {
   }

   _rtnExtInsertCtx::~_rtnExtInsertCtx()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTINSERTCTX_OPEN, "_rtnExtInsertCtx::open" )
   INT32 _rtnExtInsertCtx::open( rtnExtDataProcessorMgr *processorMgr,
                                 const CHAR *csName, const CHAR *clName,
                                 const CHAR *idxName, const BSONObj &object,
                                 pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTINSERTCTX_OPEN ) ;
      std::vector<rtnExtDataProcessor *> processors ;

      SDB_ASSERT( processorMgr && csName && clName && idxName,
                  "Invalid argument" ) ;

      _processorMgr = processorMgr ;
      rc = processorMgr->getProcessorsAndLock( csName, clName, idxName,
                                               EXCLUSIVE, processors ) ;
      PD_RC_CHECK( rc, PDERROR, "Get external processor failed[ %d ]", rc ) ;
      _lockType = EXCLUSIVE ;
      SDB_ASSERT( processors.size() <= 1, "Processor number is wrong" ) ;
      if ( 0 == processors.size() )
      {
         goto done ;
      }

      _appendProcessors( processors ) ;
      _processorLocked = TRUE ;

      rc = processors.back()->processInsert( object, cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Process insert failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTINSERTCTX_OPEN, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   _rtnExtDeleteCtx::_rtnExtDeleteCtx()
   : _rtnExtDataOprCtx( DMS_EXTOPR_TYPE_DELETE )
   {
   }

   _rtnExtDeleteCtx::~_rtnExtDeleteCtx()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDELETECTX_OPEN, "_rtnExtInsertCtx::open" )
   INT32 _rtnExtDeleteCtx::open( rtnExtDataProcessorMgr *processorMgr,
                                 const CHAR *csName, const CHAR *clName,
                                 const CHAR *idxName, const BSONObj &object,
                                 pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDELETECTX_OPEN ) ;
      std::vector<rtnExtDataProcessor *> processors ;

      SDB_ASSERT( processorMgr && csName && clName && idxName,
                  "Invalid argument" ) ;

      _processorMgr = processorMgr ;
      rc = processorMgr->getProcessorsAndLock( csName, clName, idxName,
                                               EXCLUSIVE, processors ) ;
      PD_RC_CHECK( rc, PDERROR, "Get external processor failed[ %d ]", rc ) ;
      _lockType = EXCLUSIVE ;
      SDB_ASSERT( processors.size() <= 1, "Processor number is wrong" ) ;
      if ( 0 == processors.size() )
      {
         goto done ;
      }

      _appendProcessors( processors ) ;
      _processorLocked = TRUE ;

      rc = processors.back()->processDelete( object, cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Process delete failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDELETECTX_OPEN, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   _rtnExtUpdateCtx::_rtnExtUpdateCtx()
   : _rtnExtDataOprCtx( DMS_EXTOPR_TYPE_UPDATE )
   {
   }

   _rtnExtUpdateCtx::~_rtnExtUpdateCtx()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTUPDATECTX_OPEN, "_rtnExtUpdateCtx::open" )
   INT32 _rtnExtUpdateCtx::open( rtnExtDataProcessorMgr *processorMgr,
                                 const CHAR *csName, const CHAR *clName,
                                 const CHAR *idxName, const BSONObj &oldObj,
                                 const BSONObj &newObj, pmdEDUCB *cb,
                                 SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTUPDATECTX_OPEN ) ;
      std::vector<rtnExtDataProcessor *> processors ;

      SDB_ASSERT( processorMgr && csName && clName && idxName,
                  "Invalid argument" ) ;

      _processorMgr = processorMgr ;
      rc = processorMgr->getProcessorsAndLock( csName, clName, idxName,
                                               EXCLUSIVE, processors ) ;
      PD_RC_CHECK( rc, PDERROR, "Get external processor failed[ %d ]", rc ) ;
      _lockType = EXCLUSIVE ;
      SDB_ASSERT( processors.size() <= 1, "Processor number is wrong" ) ;
      if ( 0 == processors.size() )
      {
         goto done ;
      }

      _appendProcessors( processors ) ;
      _processorLocked = TRUE ;

      rc = processors.back()->processUpdate( oldObj, newObj, cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Process update failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTUPDATECTX_OPEN, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   _rtnExtDropOprCtx::_rtnExtDropOprCtx( DMS_EXTOPR_TYPE type )
   : _rtnExtContextBase( type ),
     _removeFiles( FALSE )
   {
   }

   _rtnExtDropOprCtx::~_rtnExtDropOprCtx()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDROPOPRCTX_OPEN, "_rtnExtDropOprCtx::open" )
   INT32 _rtnExtDropOprCtx::open( rtnExtDataProcessorMgr *processorMgr,
                                  const CHAR *csName, const CHAR *clName,
                                  const CHAR *idxName, pmdEDUCB *cb,
                                  BOOLEAN removeFiles, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDROPOPRCTX_OPEN ) ;
      std::vector<rtnExtDataProcessor *> processors ;
      vector <rtnExtDataProcessor *> processorVecP1 ;

      _processorMgr = processorMgr ;
      rc = processorMgr->getProcessorsAndLock( csName, clName, idxName,
                                               EXCLUSIVE, processors ) ;
      PD_RC_CHECK( rc, PDERROR, "Get processors failed[ %d ]", rc ) ;
      if ( 0 == processors.size() )
      {
         goto done ;
      }
      _processorLocked = TRUE ;
      _lockType = EXCLUSIVE ;
      _removeFiles = removeFiles ;

      for ( vector<rtnExtDataProcessor *>::iterator itr = processors.begin();
            itr != processors.end(); ++itr )
      {
         if ( _removeFiles )
         {
            rc = (*itr)->doDropP1( cb, NULL ) ;
         }
         else
         {
            rc = (*itr)->doUnload( cb, NULL ) ;
         }
         PD_RC_CHECK( rc, PDERROR, "Processor drop operation failed[ %d ]",
                      rc ) ;
         processorVecP1.push_back( *itr ) ;
      }

   done:
      if ( _processorLocked )
      {
         _appendProcessors( processors ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDROPOPRCTX_OPEN, rc ) ;
      return rc ;
   error:
      if ( _removeFiles )
      {
         for ( vector<rtnExtDataProcessor *>::iterator itr = _processors.begin();
               itr != _processors.end(); ++itr )
         {
            (*itr)->doDropP1Cancel( cb, NULL ) ;
         }
         for ( vector<rtnExtDataProcessor *>::iterator itr = processorVecP1.begin();
               itr != processorVecP1.end(); ++itr )
         {
            (*itr)->doDropP1Cancel( cb, NULL ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDROPOPRCTX_DONE, "_rtnExtDropOprCtx::done" )
   INT32 _rtnExtDropOprCtx::_onDone( pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      INT32 ret = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDROPOPRCTX_DONE ) ;

      // For drop operation, the processors need to be removed.
      for ( EDP_VEC_ITR itr = _processors.begin(); itr != _processors.end();)
      {
         if ( _removeFiles )
         {
            ret = (*itr)->doDropP2( cb, dpscb ) ;
            if ( ret )
            {
               PD_LOG( PDERROR, "Do drop phase 2 failed[ %d ]", rc ) ;
               if ( SDB_OK == rc )
               {
                  rc = ret ;
               }
            }
         }
         // Unlock and delete all processors.
         _processorMgr->unlockProcessor( (*itr)->getName(), EXCLUSIVE ) ;
         _processorMgr->delProcessor( &(*itr ) ) ;
         _processors.erase( itr ) ;
      }
      _processorLocked = FALSE ;
      if ( rc )
      {
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDROPOPRCTX_DONE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDROPOPRCTX_ABORT, "_rtnExtDropOprCtx::abort" )
   INT32 _rtnExtDropOprCtx::_onAbort( pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDROPOPRCTX_ABORT ) ;
      for ( EDP_VEC_ITR itr = _processors.begin(); itr != _processors.end(); )
      {
         (*itr)->doDropP1Cancel( cb, dpscb ) ;
         _processors.erase( itr ) ;
      }
      PD_TRACE_EXIT( SDB__RTNEXTDROPOPRCTX_ABORT ) ;
      return SDB_OK ;
   }

   _rtnExtTruncateCtx::_rtnExtTruncateCtx()
   : _rtnExtContextBase( DMS_EXTOPR_TYPE_TRUNCATE )
   {
   }

   _rtnExtTruncateCtx::~_rtnExtTruncateCtx()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTTRUNCATECTX_OPEN, "_rtnExtTruncateCtx::open" )
   INT32 _rtnExtTruncateCtx::open( rtnExtDataProcessorMgr *processorMgr,
                                   const CHAR *csName, const CHAR *clName,
                                   pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTTRUNCATECTX_OPEN ) ;
      std::vector<rtnExtDataProcessor *> processors ;
      vector<rtnExtDataProcessor *> processorP1 ;

      _processorMgr = processorMgr ;
      rc = processorMgr->getProcessorsAndLock( csName, clName, NULL,
                                               EXCLUSIVE, processors ) ;
      PD_RC_CHECK( rc, PDERROR, "Get processors failed[ %d ]", rc ) ;
      if ( 0 == processors.size() )
      {
         goto done ;
      }
      _processorLocked = TRUE ;
      _lockType = EXCLUSIVE ;

      for ( vector<rtnExtDataProcessor *>::iterator itr = processors.begin();
            itr != processors.end(); ++itr )
      {
         rc = (*itr)->doDropP1( cb, NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Drop phase 1 failed[ %d ]", rc ) ;
         processorP1.push_back( *itr ) ;
      }

   done:
      if ( _processorLocked )
      {
         _appendProcessors( processors ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTTRUNCATECTX_OPEN, rc ) ;
      return rc ;
   error:
      INT32 rcTmp = SDB_OK ;
      for ( vector<rtnExtDataProcessor *>::iterator itr = _processors.begin();
            itr != _processors.end(); ++itr )
      {
         (*itr)->doDropP1Cancel( cb, NULL ) ;
      }
      for ( vector<rtnExtDataProcessor *>::iterator itr = processorP1.begin();
            itr != processorP1.end(); ++itr )
      {
         rcTmp = (*itr)->doDropP1Cancel( cb, NULL ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Drop phase 1 cancel failed[ %d ]", rc ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTTRUNCATECTX_DONE, "_rtnExtTruncateCtx::done" )
   INT32 _rtnExtTruncateCtx::_onDone( pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTTRUNCATECTX_DONE ) ;

      for ( vector<rtnExtDataProcessor *>::iterator itr = _processors.begin();
            itr != _processors.end(); ++itr )
      {
         rc = (*itr)->doDropP2( cb, NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Drop phase 2 failed[ %d ]", rc ) ;
         rc = (*itr)->doRebuild( cb, dpscb ) ;
         PD_RC_CHECK( rc, PDERROR, "External data rebuild failed[ %d ]", rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTTRUNCATECTX_DONE, rc ) ;
      return rc ;
   error:
      for ( vector<rtnExtDataProcessor *>::iterator itr = _processors.begin();
            itr != _processors.end();)
      {
         (*itr)->doDropP1Cancel( cb, NULL ) ;
         _processors.erase( itr ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTTRUNCATECTX_ABORT, "_rtnExtTruncateCtx::abort" )
   INT32 _rtnExtTruncateCtx::_onAbort( pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTTRUNCATECTX_ABORT ) ;
      for ( vector<rtnExtDataProcessor *>::iterator itr = _processors.begin();
            itr != _processors.end(); )
      {
         (*itr)->doDropP1Cancel( cb, dpscb ) ;
         _processors.erase( itr ) ;
      }
      PD_TRACE_EXIT( SDB__RTNEXTTRUNCATECTX_ABORT ) ;
      return SDB_OK ;
   }

   _rtnExtContextMgr::_rtnExtContextMgr()
   {
   }

   _rtnExtContextMgr::~_rtnExtContextMgr()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTCONTEXTMGR_FINDCONTEXT, "_rtnExtContextMgr::findContext" )
   rtnExtContextBase* _rtnExtContextMgr::findContext( UINT32 contextID )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTCONTEXTMGR_FINDCONTEXT ) ;
      rtnExtContextBase *context = NULL ;
      ossScopedRWLock lock( &_mutex, SHARED ) ;
      std::pair< rtnExtContextBase*, bool > ret = _contextMap.find( contextID ) ;
      if ( ret.second )
      {
         context = ret.first ;
      }
      PD_TRACE_EXIT( SDB__RTNEXTCONTEXTMGR_FINDCONTEXT ) ;
      return context ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTCONTEXTMGR_CREATECONTEXT, "_rtnExtContextMgr::createContext" )
   INT32 _rtnExtContextMgr::createContext( DMS_EXTOPR_TYPE type,
                                           pmdEDUCB *cb,
                                           rtnExtContextBase** context )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTCONTEXTMGR_CREATECONTEXT ) ;
      rtnExtContextBase *newCtx = NULL ;
      UINT32 ctxID = 0 ;

      switch ( type )
      {
      case DMS_EXTOPR_TYPE_INSERT:
         newCtx = SDB_OSS_NEW rtnExtInsertCtx ;
         break ;
      case DMS_EXTOPR_TYPE_DELETE:
         newCtx = SDB_OSS_NEW rtnExtDeleteCtx ;
         break ;
      case DMS_EXTOPR_TYPE_UPDATE:
         newCtx = SDB_OSS_NEW rtnExtUpdateCtx ;
         break ;
      case DMS_EXTOPR_TYPE_DROPCS:
         newCtx = SDB_OSS_NEW rtnExtDropCSCtx ;
         break ;
      case DMS_EXTOPR_TYPE_DROPCL:
         newCtx = SDB_OSS_NEW rtnExtDropCLCtx ;
         break ;
      case DMS_EXTOPR_TYPE_DROPIDX:
         newCtx = SDB_OSS_NEW rtnExtDropIdxCtx ;
         break ;
      case DMS_EXTOPR_TYPE_TRUNCATE:
         newCtx = SDB_OSS_NEW rtnExtTruncateCtx ;
         break ;
      case DMS_EXTOPR_TYPE_REBUILDIDX:
         newCtx = SDB_OSS_NEW rtnExtRebuildIdxCtx ;
         break ;
      default:
         SDB_ASSERT( FALSE, "Invalid context type" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !newCtx )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Alocate memory for context failed" ) ;
         goto error ;
      }

      ctxID = cb->getTID() ;
      newCtx->setID( ctxID ) ;

      {
         ossScopedRWLock lock( &_mutex, EXCLUSIVE ) ;
         _contextMap.insert( ctxID, newCtx ) ;
         if ( context )
         {
            *context = newCtx ;
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTCONTEXTMGR_CREATECONTEXT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTCONTEXTMGR_DELCONTEXT, "_rtnExtContextMgr::delContext" )
   INT32 _rtnExtContextMgr::delContext( UINT32 contextID, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTCONTEXTMGR_DELCONTEXT ) ;
      rtnExtContextBase *context = NULL ;
      std::pair< rtnExtContextBase*, bool > ret ;

      {
         ossScopedRWLock lock( &_mutex, SHARED ) ;
         ret = _contextMap.find( contextID ) ;
      }
      if ( ret.second )
      {
         context = ret.first ;
         ossScopedRWLock wLock( &_mutex, EXCLUSIVE ) ;
         _contextMap.erase( contextID ) ;
         SDB_OSS_DEL context ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTCONTEXTMGR_DELCONTEXT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   _rtnExtDataHandler::_rtnExtDataHandler( rtnExtDataProcessorMgr *edpMgr )
   {
      _edpMgr = edpMgr ;
   }

   _rtnExtDataHandler::~_rtnExtDataHandler()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_GETEXTDATANAME, "_rtnExtDataHandler::getExtDataName" )
   INT32 _rtnExtDataHandler::getExtDataName( const CHAR *csName,
                                             const CHAR *clName,
                                             const CHAR *idxName,
                                             CHAR *buff, UINT32 buffSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_GETEXTDATANAME ) ;
      SDB_ASSERT( csName && clName && idxName, "Names should not be NULL" ) ;
      if ( !buff )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Buffer is not valid" ) ;
         goto error ;
      }
      else if ( buffSize < DMS_COLLECTION_FULL_NAME_SZ + 1 )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Buffer size too small" ) ;
         goto error ;
      }
      rtnExtDataProcessor::genExtDataNames( csName, clName, idxName, NULL, 0,
                                            buff, buffSize ) ;
   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_GETEXTDATANAME, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_CHECK, "_rtnExtDataHandler::check" )
   INT32 _rtnExtDataHandler::check( DMS_EXTOPR_TYPE type, const CHAR *csName,
                                    const CHAR *clName, const CHAR *idxName,
                                    const BSONObj *object,
                                    const BSONObj *objNew, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_CHECK ) ;

      std::vector<rtnExtDataProcessor *> processors ;

      rc = _edpMgr->getProcessorsAndLock( csName, clName, idxName, SHARED,
                                          processors ) ;
      PD_RC_CHECK( rc, PDERROR, "Get processors for cs[ %s ] and cl[ %s ] "
                   "failed[ %d ]", csName, clName, rc ) ;

      SDB_ASSERT( processors.size() <= 1, "Processor number is wrong" ) ;
      if ( 0 == processors.size() )
      {
         goto done ;
      }

      {
         rtnExtDataProcessor *processor = processors.front() ;
         if ( processor )
         {
            rc = processor->check( type, object, objNew ) ;
            PD_RC_CHECK( rc, PDERROR, "Processor check failed[ %d ]", rc ) ;
         }
      }

   done:
      if ( processors.size() > 0 )
      {
         _edpMgr->unlockProcessors( processors, SHARED ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_CHECK, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONOPENTEXTIDX, "_rtnExtDataHandler::onOpenTextIdx" )
   INT32 _rtnExtDataHandler::onOpenTextIdx( const CHAR *csName,
                                            const CHAR *clName,
                                            const CHAR *idxName,
                                            const BSONObj &idxKeyDef )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONOPENTEXTIDX ) ;
      rtnExtDataProcessor *processor = NULL ;

      rc = _edpMgr->createProcessor( csName, clName, idxName,
                                     idxKeyDef, processor ) ;
      PD_RC_CHECK( rc, PDERROR, "Create external processor for cl[ %s.%s ], "
                   "and index[%s] failed[ %d ]", csName, clName, idxName, rc ) ;
      rc = _edpMgr->addProcessor( processor ) ;
      PD_RC_CHECK( rc, PDERROR, "Add external processor failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONOPENTEXTIDX, rc ) ;
      return rc ;
   error:
      if ( processor )
      {
         _edpMgr->destroyProcessor( processor ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDELCS, "_rtnExtDataHandler::onDelCS" )
   INT32 _rtnExtDataHandler::onDelCS( const CHAR *csName, pmdEDUCB *cb,
                                      BOOLEAN removeFiles, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDELCS ) ;
      rtnExtDropOprCtx *context = NULL ;

      context = (rtnExtDropOprCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_DROPCS,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, NULL, NULL, cb, removeFiles, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Open external delete cs context failed[ %d ]",
                   rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDELCS, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDELCL, "_rtnExtDataHandler::onDelCL" )
   INT32 _rtnExtDataHandler::onDelCL( const CHAR *csName, const CHAR *clName,
                                      pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDELCL ) ;
      rtnExtDropOprCtx *context = NULL ;

      context = (rtnExtDropOprCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_DROPCL,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, NULL, cb, TRUE, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Open external delete cs context failed[ %d ]",
                   rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDELCL, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONCRTTEXTIDX, "_rtnExtDataHandler::onCrtTextIdx" )
   INT32 _rtnExtDataHandler::onCrtTextIdx( const CHAR *csName,
                                           const CHAR *clName,
                                           const CHAR *idxName,
                                           pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONCRTTEXTIDX ) ;

      if ( _edpMgr->number() >= RTN_TEXTIDX_MAX_NUM )
      {
         rc = SDB_OSS_UP_TO_LIMIT ;
         PD_LOG( PDERROR, "Max number of text indices[%d] has been created",
                 RTN_TEXTIDX_MAX_NUM ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONCRTTEXTIDX, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX, "_rtnExtDataHandler::onDropTextIdx" )
   INT32 _rtnExtDataHandler::onDropTextIdx( const CHAR *csName,
                                            const CHAR *clName,
                                            const CHAR *idxName,
                                            pmdEDUCB *cb,
                                            SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX ) ;
      SDB_DB_STATUS dbStatus = pmdGetKRCB()->getDBStatus() ;
      rtnExtDropOprCtx *context = NULL ;

      if ( SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      context = (rtnExtDropOprCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_DROPIDX,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      // If the current context is not DROPIDX, the current operation may be
      // dropping cs or cl. Nothing should be done in that cases.
      if ( DMS_EXTOPR_TYPE_DROPIDX == context->getType() )
      {
         rc = context->open( _edpMgr, csName, clName,
                             idxName, cb, TRUE, NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Open external drop context failed[ %d ]",
                      rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX, "_rtnExtDataHandler::onRebuildTextIdx" )
   INT32 _rtnExtDataHandler::onRebuildTextIdx( const CHAR *csName,
                                               const CHAR *clName,
                                               const CHAR *idxName,
                                               const BSONObj &idxKeyDef,
                                               pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX ) ;
      rtnExtRebuildIdxCtx *context = NULL ;

      SDB_ASSERT( csName && clName && idxName && cb,
                  "Names and cb should not be NULL" ) ;

      context = (rtnExtRebuildIdxCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_REBUILDIDX,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, idxName,
                          idxKeyDef, cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Open external rebuild context failed[ %d ]",
                   rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONINSERT, "_rtnExtDataHandler::onInsert" )
   INT32 _rtnExtDataHandler::onInsert( const CHAR *csName, const CHAR *clName,
                                       const CHAR *idxName,
                                       const ixmIndexCB &indexCB,
                                       const BSONObj &object, _pmdEDUCB* cb,
                                       SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONINSERT ) ;
      SDB_DB_STATUS dbStatus = pmdGetKRCB()->getDBStatus() ;
      rtnExtInsertCtx *context = NULL ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      context = (rtnExtInsertCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_INSERT,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, idxName, object, cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context for insert failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONINSERT, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         INT32 rcTmp = _contextMgr.delContext( context->getID(), cb ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Delete context failed[ %d ]", rcTmp ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDELETE, "_rtnExtDataHandler::onDelete" )
   INT32 _rtnExtDataHandler::onDelete( const CHAR *csName, const CHAR *clName,
                                       const CHAR *idxName,
                                       const ixmIndexCB &indexCB,
                                       const BSONObj &object, _pmdEDUCB* cb,
                                       SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDELETE ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      rtnExtDeleteCtx *context = NULL ;
      BSONObj processData ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      context = (rtnExtDeleteCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_DELETE,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, idxName, object, cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context for insert failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDELETE, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         INT32 rcTmp = _contextMgr.delContext( context->getID(), cb ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Delete context failed[ %d ]", rcTmp ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONUPDATE, "_rtnExtDataHandler::onUpdate" )
   INT32 _rtnExtDataHandler::onUpdate( const CHAR *csName, const CHAR *clName,
                                       const CHAR *idxName,
                                       const ixmIndexCB &indexCB,
                                       const BSONObj &orignalObj,
                                       const BSONObj &newObj,
                                       pmdEDUCB* cb,
                                       SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONUPDATE ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      rtnExtUpdateCtx *context = NULL ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      context = (rtnExtUpdateCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_UPDATE,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, idxName, orignalObj,
                          newObj, cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context for insert failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONUPDATE, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         INT32 rcTmp = _contextMgr.delContext( context->getID(), cb ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Delete context failed[ %d ]", rcTmp ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONTRUNCATECL, "_rtnExtDataHandler::onTruncateCL" )
   INT32 _rtnExtDataHandler::onTruncateCL( const CHAR *csName,
                                           const CHAR *clName,
                                           pmdEDUCB *cb,
                                           SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONTRUNCATECL ) ;
      rtnExtTruncateCtx *context = NULL ;
      vector<rtnExtDataProcessor *> processors ;

      context = (rtnExtTruncateCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_TRUNCATE,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context for truncate failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONTRUNCATECL, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_DONE, "_rtnExtDataHandler::done" )
   INT32 _rtnExtDataHandler::done( DMS_EXTOPR_TYPE type, pmdEDUCB *cb,
                                   SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_DONE ) ;
      rtnExtContextBase *context = _contextMgr.findContext( cb->getTID() ) ;
      BOOLEAN ownContext = FALSE ;
      if ( !context )
      {
         goto done ;
      }

      ownContext = ( context->getType() == type ) ;
      if ( !ownContext )
      {
         goto done ;
      }

      rc = context->done( cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Final step of current operation failed[ %d ]",
                      rc) ;

   done:
      if ( context && ownContext )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_DONE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ABORTOPERATION, "_rtnExtDataHandler::abortOperation" )
   INT32 _rtnExtDataHandler::abortOperation( DMS_EXTOPR_TYPE type,
                                             pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ABORTOPERATION ) ;
      rtnExtContextBase *context = _contextMgr.findContext( cb->getTID() ) ;
      BOOLEAN ownContext = FALSE ;
      if ( !context )
      {
         goto done ;
      }

      ownContext = ( context->getType() == type ) ;
      if ( !ownContext )
      {
         goto done ;
      }

      rc = context->abort( cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Final step of current operation failed[ %d ]",
                   rc) ;

   done:
      if ( context && ownContext )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ABORTOPERATION, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   rtnExtDataHandler* rtnGetExtDataHandler()
   {
      static rtnExtDataHandler s_edh( rtnGetExtDataProcessorMgr() ) ;
      return &s_edh ;
   }
}

