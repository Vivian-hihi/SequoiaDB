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

   Source File Name = rtnContextDel.hpp

   Descriptive Name = RunTime Delete Operation Context

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          5/26/2017   David Li  Split from rtnContext.cpp

   Last Changed =

*******************************************************************************/
#include "rtnContextDel.hpp"
#include "rtn.hpp"
#include "dpsOp2Record.hpp"
#include "clsMgr.hpp"
#include "rtnTrace.hpp"

namespace engine
{
   RTN_CTX_AUTO_REGISTER(_rtnContextDelCS, RTN_CONTEXT_DELCS, "DELCS")

   _rtnContextDelCS::_rtnContextDelCS( SINT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _status = DELCSPHASE_0;
      _pDmsCB = pmdGetKRCB()->getDMSCB() ;
      _pDpsCB = pmdGetKRCB()->getDPSCB() ;
      _pCatAgent = pmdGetKRCB()->getClsCB ()->getCatAgent () ;
      _pTransCB = pmdGetKRCB()->getTransCB();
      _gotDmsCBWrite = FALSE;
      _gotLogSize = 0;
      _logicCSID = DMS_INVALID_LOGICCSID;
      ossMemset( _name, 0, DMS_COLLECTION_SPACE_NAME_SZ + 1 );
   }

   _rtnContextDelCS::~_rtnContextDelCS()
   {
      pmdEDUMgr *eduMgr = pmdGetKRCB()->getEDUMgr() ;
      pmdEDUCB *cb = eduMgr->getEDUByID( eduID() ) ;
      if ( DELCSPHASE_1 == _status )
      {
         INT32 rcTmp = SDB_OK;
         rcTmp = rtnDropCollectionSpaceP1Cancel( _name, cb, _pDmsCB,
                                                 getDPSCB() );
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "failed to cancel drop cs(name:%s, rc=%d)",
                    _name, rcTmp );
         }
         _status = DELCSPHASE_0;
      }
      _clean( cb );
   }

   std::string _rtnContextDelCS::name() const
   {
      return "DELCS" ;
   }

   RTN_CONTEXT_TYPE _rtnContextDelCS::getType () const
   {
      return RTN_CONTEXT_DELCS;
   }

   INT32 _rtnContextDelCS::open( const CHAR *pCollectionName,
                                 _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      dpsMergeInfo info ;
      dpsLogRecord &record = info.getMergeBlock().record();

      SDB_ASSERT( pCollectionName, "pCollectionName can't be null!" );
      PD_CHECK( pCollectionName, SDB_INVALIDARG, error, PDERROR,
                "pCollectionName is null!" );
      rc = dmsCheckCSName( pCollectionName );
      PD_RC_CHECK( rc, PDERROR, "Invalid cs name(name:%s)",
                   pCollectionName );

      ossStrncpy( _name, pCollectionName, DMS_COLLECTION_SPACE_NAME_SZ ) ;

      /// test collection space exist
      rc = rtnTestCollectionSpaceCommand( pCollectionName, _pDmsCB ) ;
      if ( SDB_DMS_CS_NOTEXIST == rc )
      {
         /// ignore collection space not exist
         PD_LOG( PDINFO, "Ignored error[%d] when drop collection space[%s]",
                 rc, pCollectionName ) ;
         rc = SDB_OK ;
         _isOpened = TRUE ;
         goto done ;
      }

      if ( NULL != getDPSCB() )
      {
         // reserved log-size
         UINT32 logRecSize = 0;
         rc = dpsCSDel2Record( pCollectionName, record ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to build record:%d",rc ) ;

         rc = getDPSCB()->checkSyncControl( record.alignedLen(), cb ) ;
         PD_RC_CHECK( rc, PDERROR, "Check sync control failed, rc: %d", rc ) ;

         logRecSize = record.alignedLen() ;
         rc = _pTransCB->reservedLogSpace( logRecSize, cb );
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to reserved log space(length=%u)",
                      logRecSize );
         _gotLogSize = logRecSize ;
      }

      rc = _pDmsCB->writable ( cb ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "dms is not writable, rc = %d", rc ) ;
      _gotDmsCBWrite = TRUE;

      rc = _tryLock( pCollectionName, cb );
      PD_RC_CHECK( rc, PDERROR, "Failed to lock, rc: %d", rc ) ;

      rc = rtnDropCollectionSpaceP1( _name, cb, _pDmsCB, getDPSCB() );
      PD_RC_CHECK( rc, PDERROR, "Failed to drop cs in phase1, rc: %d", rc );
      _status = DELCSPHASE_1 ;
      _isOpened = TRUE ;

   done:
      return rc;
   error:
      _clean( cb );
      goto done;
   }

   void _rtnContextDelCS::_toString( stringstream &ss )
   {
      ss << ",Name:" << _name
         << ",GotLogSize:" << _gotLogSize
         << ",GotDMSWrite:" << _gotDmsCBWrite
         << ",LogicalID:" << _logicCSID
         << ",Step:" << _status ;
   }

   INT32 _rtnContextDelCS::getMore( INT32 maxNumToReturn,
                                    rtnContextBuf &buffObj,
                                    _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      SDB_RTNCB *pRtnCB = sdbGetRTNCB() ;
      clsCB *pClsCB = sdbGetClsCB() ;
      shardCB *pShdMgr = pClsCB->getShardCB() ;
      clsTaskMgr *pTaskMgr = pmdGetKRCB()->getClsCB()->getTaskMgr() ;
      vector< string > subCLs ;
      vector< string >::iterator it ;
      _utilSet< string > mainCLs ;
      _utilSet< string >::iterator mainIter ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE;
         goto error ;
      }

      _pCatAgent->lock_w() ;
      _pCatAgent->clearBySpaceName( _name, &subCLs, &mainCLs ) ;
      _pCatAgent->release_w() ;

      it = subCLs.begin() ;
      while( it != subCLs.end() )
      {
         if ( SDB_OK != pShdMgr->syncUpdateCatalog( (*it).c_str() ) )
         {
            _pCatAgent->lock_w() ;
            _pCatAgent->clear( (*it).c_str() ) ;
            _pCatAgent->release_w() ;
         }
         pClsCB->invalidateCata( (*it).c_str() ) ;
         ++it ;
      }
      pClsCB->invalidateCata( _name ) ;

      // Clear main collection plans
      mainIter = mainCLs.begin() ;
      while ( mainIter != mainCLs.end() )
      {
         const CHAR * mainCLName = ( *mainIter ).c_str() ;
         // Clear plan cache in self
         pRtnCB->getAPM()->invalidateCLPlans( mainCLName ) ;
         // Clear plan cache in secondary nodes
         pClsCB->invalidatePlan( mainCLName ) ;
         ++ mainIter ;
      }

      /// already drop phrase1
      if ( DELCSPHASE_1 == _status )
      {
         rc = rtnDropCollectionSpaceP2( _name, cb, _pDmsCB, getDPSCB() ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to drop cs in phase2(%d)", rc ) ;
         _status = DELCSPHASE_0 ;
         _clean( cb ) ;
      }
      else
      {
         //It is main cs, which hasn't data file, we should invalidate plan here.
         //If it is normal cs which its data file exists on data node,
         //invalidate plan will be executed in dms by calling onDropCS().
         pRtnCB->getAPM()->invalidateSUPlans( _name ) ;
         pClsCB->invalidateCache( _name, DPS_LOG_INVALIDCATA_TYPE_PLAN ) ;
      }

      /// close context
      _isOpened = FALSE ;
      rc = SDB_DMS_EOC ;

      /// wait all collection space's task finished
      cb->writingDB( FALSE ) ;
      while( pTaskMgr->taskCountByCS( _name ) > 0 )
      {
         pTaskMgr->waitTaskEvent() ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextDelCS::_tryLock( const CHAR *pCollectionName,
                                     _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      if ( getDPSCB() )
      {
         dmsStorageUnitID suID = DMS_INVALID_CS;
         UINT32 logicCSID = DMS_INVALID_LOGICCSID;
         dmsStorageUnit *su = NULL;
         UINT32 length = ossStrlen ( pCollectionName );
         PD_CHECK( (length > 0 && length <= DMS_SU_NAME_SZ), SDB_INVALIDARG,
                   error, PDERROR, "Invalid length of collectionspace name:%s",
                   pCollectionName );

         rc = _pDmsCB->nameToSUAndLock( pCollectionName, suID, &su );
         PD_RC_CHECK(rc, PDERROR, "lock collection space(%s) failed(rc=%d)",
                     pCollectionName, rc );
         logicCSID = su->LogicalCSID();
         _pDmsCB->suUnlock ( suID ) ;
         rc = _pTransCB->transLockTryX( cb, logicCSID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Get transaction-lock of CS(%s) failed(rc=%d)",
                      pCollectionName, rc ) ;
         _logicCSID = logicCSID ;
      }
   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextDelCS::_releaseLock( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      if ( cb && getDPSCB() && ( _logicCSID != DMS_INVALID_LOGICCSID ) )
      {
         _pTransCB->transLockRelease( cb, _logicCSID );
         _logicCSID = DMS_INVALID_LOGICCSID;
      }
      return rc;
   }

   void _rtnContextDelCS::_clean( _pmdEDUCB *cb )
   {
      INT32 rcTmp = SDB_OK;
      rcTmp = _releaseLock( cb );
      if ( rcTmp )
      {
         PD_LOG( PDERROR, "releas lock failed, rc: %d", rcTmp );
      }
      if ( _gotDmsCBWrite )
      {
         _pDmsCB->writeDown ( cb ) ;
         _gotDmsCBWrite = FALSE;
      }
      if ( _gotLogSize > 0 )
      {
         _pTransCB->releaseLogSpace( _gotLogSize, cb );
         _gotLogSize = 0;
      }
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextDelCL, RTN_CONTEXT_DELCL, "DELCL")

   _rtnContextDelCL::_rtnContextDelCL( SINT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _pDmsCB        = pmdGetKRCB()->getDMSCB() ;
      _pDpsCB        = pmdGetKRCB()->getDPSCB() ;
      _pCatAgent     = pmdGetKRCB()->getClsCB()->getCatAgent () ;
      _pTransCB      = pmdGetKRCB()->getTransCB();
      _gotDmsCBWrite = FALSE ;
      _hasLock       = FALSE ;
      _hasDropped    = FALSE ;
      _mbContext     = NULL ;
      _su            = NULL ;
      _clShortName   = NULL ;
      ossMemset( _collectionName, 0, sizeof( _collectionName ) ) ;
   }

   _rtnContextDelCL::~_rtnContextDelCL()
   {
      pmdEDUMgr *eduMgr    = pmdGetKRCB()->getEDUMgr() ;
      pmdEDUCB *cb         = eduMgr->getEDUByID( eduID() ) ;
      _clean( cb ) ;
   }

   INT32 _rtnContextDelCL::_tryLock( const CHAR *pCollectionName,
                                     _pmdEDUCB *cb )
   {
      INT32 rc                = SDB_OK ;
      dmsStorageUnitID suID   = DMS_INVALID_CS ;
      IDmsExtDataHandler *extHandler = NULL ;

      ossStrncpy( _collectionName, pCollectionName,
                  DMS_COLLECTION_FULL_NAME_SZ ) ;

      rc = rtnResolveCollectionNameAndLock ( _collectionName, _pDmsCB,
                                             &_su, &_clShortName,
                                             suID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to resolve collection name"
                   "(collection:%s, rc: %d)", _collectionName, rc ) ;

      // lock collection
      if ( getDPSCB() )
      {
         rc = _su->data()->getMBContext( &_mbContext, _clShortName,
                                         EXCLUSIVE ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pCollectionName, rc ) ;

         rc = _pTransCB->transLockTryX( cb, _su->LogicalCSID(),
                                        _mbContext->mbID() ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Get transaction-lock of collection(%s) failed(rc=%d)",
                      pCollectionName, rc ) ;
         _hasLock = TRUE ;
      }

      extHandler = _su->data()->getExtDataHandler() ;
      if ( extHandler )
      {
         if ( !_mbContext )
         {
            rc = _su->data()->getMBContext( &_mbContext, _clShortName,
                                            EXCLUSIVE ) ;
            PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                         "rc: %d", pCollectionName, rc ) ;
         }

         rc = extHandler->onDelCL( _su->CSName(), _clShortName, cb ) ;
         PD_RC_CHECK( rc, PDERROR, "External operation on delete cl failed, "
                      "rc: %d", rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextDelCL::_releaseLock( _pmdEDUCB *cb )
   {
      if ( cb && _hasLock )
      {
         _pTransCB->transLockRelease( cb, _su->LogicalCSID(),
                                      _mbContext->mbID() ) ;
         _hasLock = FALSE ;
      }
      return SDB_OK ;
   }

   std::string _rtnContextDelCL::name() const
   {
      return "DELCL" ;
   }

   RTN_CONTEXT_TYPE _rtnContextDelCL::getType () const
   {
      return RTN_CONTEXT_DELCL ;
   }

   INT32 _rtnContextDelCL::open( const CHAR *pCollectionName,
                                 _pmdEDUCB *cb, INT16 w )
   {
      INT32 rc = SDB_OK ;

      /// set w info
      _w = w ;

      SDB_ASSERT( pCollectionName, "pCollectionName can't be null!" );
      PD_CHECK( pCollectionName, SDB_INVALIDARG, error, PDERROR,
               "pCollectionName is null!" );
      rc = dmsCheckFullCLName( pCollectionName );
      PD_RC_CHECK( rc, PDERROR, "Invalid collection name(name:%s)",
                   pCollectionName ) ;

      rc = _pDmsCB->writable ( cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Database is not writable, rc = %d", rc ) ;
      _gotDmsCBWrite = TRUE ;

      rc = _tryLock( pCollectionName, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to lock(rc=%d)", rc ) ;
      _isOpened = TRUE ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextDelCL::getMore( INT32 maxNumToReturn,
                                    rtnContextBuf &buffObj,
                                    _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      SDB_RTNCB * pRtnCB = pmdGetKRCB()->getRTNCB() ;
      clsCB * pClsCB = pmdGetKRCB()->getClsCB() ;
      clsTaskMgr * pTaskMgr = pClsCB->getTaskMgr() ;
      CHAR mainCL[ DMS_COLLECTION_FULL_NAME_SZ + 1 ] = { '\0' } ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE;
         goto error ;
      }

      _pCatAgent->lock_w () ;
      _pCatAgent->clear ( _collectionName, mainCL ) ;
      _pCatAgent->release_w () ;
      pClsCB->invalidateCata( _collectionName ) ;

      // Clear catalog info and cached plans of main-collection if needed
      if ( '\0' != mainCL[ 0 ] )
      {
         _pCatAgent->lock_w() ;
         _pCatAgent->clear( mainCL ) ;
         _pCatAgent->release_w() ;
         pRtnCB->getAPM()->invalidateCLPlans( mainCL ) ;
         pClsCB->invalidateCache( mainCL, DPS_LOG_INVALIDCATA_TYPE_CATA |
                                          DPS_LOG_INVALIDCATA_TYPE_PLAN ) ;
      }

      // drop collection
      rc = _su->data()->dropCollection ( _clShortName, cb, getDPSCB(),
                                         TRUE, _mbContext ) ;
      if ( rc )
      {
         // Ignore SDB_DMS_NOTEXIST, which means the CL mignt be deleted already
         if ( SDB_DMS_NOTEXIST == rc )
         {
            PD_LOG ( PDWARNING, "Collection %s doesn't exist, ignored in drop "
                     "collection, rc: %d", _collectionName, rc ) ;
            rc = SDB_OK ;
         }
         else
         {
            PD_LOG ( PDERROR, "Failed to drop collection %s, rc: %d",
                     _collectionName, rc ) ;
            goto error ;
         }
      }

      _hasDropped = TRUE ;

      _clean( cb ) ;
      _isOpened = FALSE ;
      rc = SDB_DMS_EOC ;

      /// wait all collection's task finished
      cb->writingDB( FALSE ) ;

      {
         UINT32 waitCnt = 0 ;
         while( pTaskMgr->taskCountByCL( _collectionName ) > 0 )
         {
            pTaskMgr->waitTaskEvent() ;
            waitCnt ++ ;

            // Log the task list after waiting over 10 minutes
            if ( waitCnt > 600 )
            {
               PD_LOG( PDDEBUG, "DropCL [%s] is waiting for split tasks:\n%s",
                       _collectionName,
                       pTaskMgr->dumpTasks( CLS_TASK_UNKNOW ).c_str() ) ;
               waitCnt = 0 ;
            }

         }
      }

   done:
      return rc;
   error:
      goto done;
   }

   void _rtnContextDelCL::_toString( stringstream &ss )
   {
      ss << ",Name:" << _collectionName
         << ",GotDMSWrite:" << _gotDmsCBWrite
         << ",HasLock:" << _hasLock
         << ",HasDropped:" << _hasDropped ;
   }

   void _rtnContextDelCL::_clean( _pmdEDUCB *cb )
   {
      INT32 rcTmp = SDB_OK;
      IDmsExtDataHandler *extHandler = NULL ;

      if ( _su && _su->data() )
      {
         extHandler = _su->data()->getExtDataHandler() ;
         if ( extHandler )
         {
            if ( _hasDropped )
            {
               extHandler->done( DMS_EXTOPR_TYPE_DROPCL, cb ) ;
            }
            else
            {
               extHandler->abortOperation( DMS_EXTOPR_TYPE_DROPCL, cb ) ;
            }
         }
      }

      rcTmp = _releaseLock( cb ) ;
      if ( rcTmp )
      {
         PD_LOG( PDERROR, "release lock failed, rc: %d", rcTmp ) ;
      }
      if ( _su && _mbContext )
      {
         _su->data()->releaseMBContext( _mbContext ) ;
      }

      // unlock su
      if ( _pDmsCB && _su )
      {
         string csname = _su->CSName() ;
         _pDmsCB->suUnlock ( _su->CSID() ) ;
         _su = NULL ;

         if ( _hasDropped )
         {
            // ignore errors
            _pDmsCB->dropEmptyCollectionSpace( csname.c_str(),
                                               cb, getDPSCB() ) ;
         }
      }

      if ( _gotDmsCBWrite )
      {
         _pDmsCB->writeDown( cb ) ;
         _gotDmsCBWrite = FALSE ;
      }
      _isOpened = FALSE ;
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextDelMainCL, RTN_CONTEXT_DELMAINCL, "DELMAINCL")

   _rtnContextDelMainCL::_rtnContextDelMainCL( SINT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _pCatAgent     = pmdGetKRCB()->getClsCB()->getCatAgent() ;
      _pRtncb        = pmdGetKRCB()->getRTNCB();
      _version       = -1 ;
      _lockDms       = FALSE ;
      ossMemset( _name, 0, DMS_COLLECTION_FULL_NAME_SZ + 1 );
   }

   _rtnContextDelMainCL::~_rtnContextDelMainCL()
   {
      pmdEDUMgr *eduMgr = pmdGetKRCB()->getEDUMgr() ;
      pmdEDUCB *cb = eduMgr->getEDUByID( eduID() ) ;
      _clean( cb );
   }

   void _rtnContextDelMainCL::_clean( _pmdEDUCB *cb )
   {
      SUBCL_CONTEXT_LIST::iterator iter = _subContextList.begin();
      while( iter != _subContextList.end() )
      {
         if ( iter->second != -1 )
         {
            _pRtncb->contextDelete( iter->second, cb );
         }
         iter = _subContextList.erase( iter );
      }
   }

   std::string _rtnContextDelMainCL::name() const
   {
      return "DELMAINCL" ;
   }

   RTN_CONTEXT_TYPE _rtnContextDelMainCL::getType () const
   {
      return RTN_CONTEXT_DELMAINCL;
   }

   INT32 _rtnContextDelMainCL::open( const CHAR *pCollectionName,
                                     vector< string > &subCLList,
                                     INT32 version,
                                     _pmdEDUCB *cb,
                                     INT16 w )
   {
      INT32 rc = SDB_OK ;
      vector< string >::iterator iter ;
      rtnContextDelCL *delContext   = NULL ;
      SINT64 contextID              = -1 ;

      _version                      = version ;

      SDB_ASSERT( pCollectionName, "pCollectionName can't be null!" ) ;
      PD_CHECK( pCollectionName, SDB_INVALIDARG, error, PDERROR,
                "pCollectionName is null!" ) ;

      rc = dmsCheckFullCLName( pCollectionName ) ;
      PD_RC_CHECK( rc, PDERROR, "Invalid collection name[%s])",
                   pCollectionName ) ;

      // we need to check dms writable when invalidate cata/plan/statistics
      rc = pmdGetKRCB()->getDMSCB()->writable ( cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Database is not writable, rc: %d", rc ) ;
      _lockDms = TRUE ;

      /// open sub collection context
      iter = subCLList.begin() ;
      while( iter != subCLList.end() )
      {
         rc = _pRtncb->contextNew( RTN_CONTEXT_DELCL,
                                   (rtnContext **)&delContext,
                                   contextID, cb ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create sub-context of sub-"
                      "collection[%s] in drop collection[%s], rc: %d",
                      (*iter).c_str(), pCollectionName, rc ) ;

         rc = delContext->open( (*iter).c_str(), cb, w ) ;
         if ( rc != SDB_OK )
         {
            _pRtncb->contextDelete( contextID, cb ) ;
            if ( SDB_DMS_NOTEXIST == rc )
            {
               ++iter;
               continue;
            }
            PD_LOG( PDERROR, "Failed to open sub-context of sub-"
                    "collection[%s] in drop collection[%s], rc: %d",
                    (*iter).c_str(), pCollectionName, rc ) ;
            goto error;
         }
         _subContextList[ *iter ] = contextID ;
         ++iter ;
      }

      ossStrcpy( _name, pCollectionName ) ;
      _isOpened = TRUE;
   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextDelMainCL::getMore( INT32 maxNumToReturn,
                                        rtnContextBuf &buffObj,
                                        _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      INT32 curVer = -1 ;
      _clsCatalogSet *pCataSet = NULL ;
      SUBCL_CONTEXT_LIST::iterator iterCtx ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE;
         goto error ;
      }

      /// get last catalog info
      _pCatAgent->lock_r() ;
      pCataSet = _pCatAgent->collectionSet( _name ) ;
      if ( pCataSet )
      {
         curVer = pCataSet->getVersion() ;
      }
      _pCatAgent->release_r() ;

      if ( -1 != curVer && curVer != _version )
      {
         rc = SDB_CLS_COORD_NODE_CAT_VER_OLD ;
         goto error ;
      }

      /// drop sub collections
      iterCtx = _subContextList.begin() ;
      while( iterCtx != _subContextList.end() )
      {
         rtnContextBuf buffObj;
         rc = rtnGetMore( iterCtx->second, -1, buffObj, cb, _pRtncb ) ;
         PD_CHECK( SDB_DMS_EOC == rc || SDB_DMS_NOTEXIST == rc,
                   rc, error, PDERROR,
                   "Failed to del sub-collection, rc: %d",
                   rc ) ;
         rc = SDB_OK ;
         iterCtx = _subContextList.erase( iterCtx ) ;
      }

      /// clear main collection's catalog info
      _pCatAgent->lock_w () ;
      _pCatAgent->clear ( _name ) ;
      _pCatAgent->release_w () ;

      // Clear cached main-collection plans
      _pRtncb->getAPM()->invalidateCLPlans( _name ) ;

      // Tell secondary nodes to clear catalog and plan caches
      sdbGetClsCB()->invalidateCache( _name,
                                      DPS_LOG_INVALIDCATA_TYPE_CATA |
                                      DPS_LOG_INVALIDCATA_TYPE_PLAN ) ;

      _isOpened = FALSE ;
      rc = SDB_DMS_EOC ;

   done:
      if ( _lockDms )
      {
         pmdGetKRCB()->getDMSCB()->writeDown( cb ) ;
         _lockDms = FALSE ;
      }
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextDelMainCL::_toString( stringstream &ss )
   {
      ss << ",Name:" << _name
         << ",Version:" << _version ;
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextRenameCS, RTN_CONTEXT_RENAMECS, "RENAMECS")

   _rtnContextRenameCS::_rtnContextRenameCS( SINT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _pDmsCB     = pmdGetKRCB()->getDMSCB() ;
      _pDpsCB     = pmdGetKRCB()->getDPSCB() ;
      _pCatAgent  = pmdGetKRCB()->getClsCB ()->getCatAgent () ;
      _pTransCB   = pmdGetKRCB()->getTransCB();
      _lockDMS    = FALSE ;
      _logicCSID  = DMS_INVALID_LOGICCSID ;
      ossMemset( _oldName, 0, DMS_COLLECTION_SPACE_NAME_SZ + 1 ) ;
      ossMemset( _newName, 0, DMS_COLLECTION_SPACE_NAME_SZ + 1 ) ;
   }

   _rtnContextRenameCS::~_rtnContextRenameCS()
   {
      pmdEDUMgr *eduMgr    = pmdGetKRCB()->getEDUMgr() ;
      pmdEDUCB *cb         = eduMgr->getEDUByID( eduID() ) ;
      _releaseLock( cb ) ;
   }

   std::string _rtnContextRenameCS::name() const
   {
      return "RENAMECS" ;
   }

   RTN_CONTEXT_TYPE _rtnContextRenameCS::getType () const
   {
      return RTN_CONTEXT_RENAMECS ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCTXRENAMECS_OPEN, "_rtnContextRenameCS::open" )
   INT32 _rtnContextRenameCS::open( const CHAR *pCSName,
                                    const CHAR *pNewCSName,
                                    _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNCTXRENAMECS_OPEN ) ;

      /// check cs name
      SDB_ASSERT( pCSName, "cs name can't be null!" );
      PD_CHECK( pCSName, SDB_INVALIDARG, error, PDERROR,
                "cs name is null!" );

      SDB_ASSERT( pNewCSName, "new cs name can't be null!" );
      PD_CHECK( pNewCSName, SDB_INVALIDARG, error, PDERROR,
                "new cs name is null!" );

      rc = dmsCheckCSName( pCSName );
      PD_RC_CHECK( rc, PDERROR, "Invalid cs name[%s]", pCSName );

      rc = dmsCheckCSName( pNewCSName );
      PD_RC_CHECK( rc, PDERROR, "Invalid cs name[%s]", pNewCSName );

      ossStrncpy( _oldName, pCSName, DMS_COLLECTION_SPACE_NAME_SZ ) ;
      ossStrncpy( _newName, pNewCSName, DMS_COLLECTION_SPACE_NAME_SZ ) ;

      /// test collection space exist
      rc = rtnTestCollectionSpaceCommand( pCSName, _pDmsCB ) ;
      if ( SDB_DMS_CS_NOTEXIST == rc )
      {
         PD_LOG( PDERROR,
                 "Collection space[%s] does not exists, rc: %d",
                 pCSName, rc ) ;
         goto done ;
      }

      rc = rtnTestCollectionSpaceCommand( pNewCSName, _pDmsCB ) ;
      if ( SDB_OK == rc )
      {
         rc = SDB_DMS_CS_EXIST ;
         PD_LOG( PDERROR,
                 "Collection space[%s] already exists, rc: %d",
                 pNewCSName, rc ) ;
         goto done ;
      }

      /// lock
      rc = _tryLock( pCSName, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to lock, rc: %d", rc ) ;

      _isOpened = TRUE ;

   done:
      PD_TRACE_EXITRC( SDB__RTNCTXRENAMECS_OPEN, rc ) ;
      return rc;
   error:
      _releaseLock( cb ) ;
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCTXRENAMECS_GETMORE, "_rtnContextRenameCS::getMore" )
   INT32 _rtnContextRenameCS::getMore( INT32 maxNumToReturn,
                                       rtnContextBuf &buffObj,
                                       _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNCTXRENAMECS_GETMORE ) ;

      SDB_RTNCB *pRtnCB = sdbGetRTNCB() ;
      clsCB *pClsCB = sdbGetClsCB() ;
      shardCB *pShdMgr = pClsCB->getShardCB() ;
      vector< string > subCLs ;
      vector< string >::iterator it ;
      _utilSet< string > mainCLs ;
      _utilSet< string >::iterator mainIter ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE;
         goto error ;
      }

      _pCatAgent->lock_w() ;
      _pCatAgent->clearBySpaceName( _oldName, &subCLs, &mainCLs ) ;
      _pCatAgent->release_w() ;

      it = subCLs.begin() ;
      while( it != subCLs.end() )
      {
         if ( SDB_OK != pShdMgr->syncUpdateCatalog( (*it).c_str() ) )
         {
            _pCatAgent->lock_w() ;
            _pCatAgent->clear( (*it).c_str() ) ;
            _pCatAgent->release_w() ;
         }
         pClsCB->invalidateCata( (*it).c_str() ) ;
         ++it ;
      }
      pClsCB->invalidateCata( _oldName ) ;

      // Clear main collection plans
      mainIter = mainCLs.begin() ;
      while ( mainIter != mainCLs.end() )
      {
         const CHAR * mainCLName = ( *mainIter ).c_str() ;
         // Clear plan cache in self
         pRtnCB->getAPM()->invalidateCLPlans( mainCLName ) ;
         // Clear plan cache in secondary nodes
         pClsCB->invalidatePlan( mainCLName ) ;
         ++ mainIter ;
      }

      rc = _pDmsCB->renameCollectionSpace( _oldName, _newName, cb, _pDpsCB ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to rename collectionspace from [%s] to [%s], rc: %d",
                   _oldName, _newName, rc ) ;

      /// close context
      _releaseLock( cb ) ;
      _isOpened = FALSE ;
      rc = SDB_DMS_EOC ;

   done:
      PD_TRACE_EXITRC( SDB__RTNCTXRENAMECS_GETMORE, rc ) ;
      return rc;
   error:
      goto done;
   }

   void _rtnContextRenameCS::_toString( stringstream &ss )
   {
      ss << ",Name:" << _oldName
         << ",NewName:" << _newName
         << ",LockDMS:" << _lockDMS
         << ",LogicalID:" << _logicCSID ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCTXRENAMECS__TRYLOCK, "_rtnContextRenameCS::_tryLock" )
   INT32 _rtnContextRenameCS::_tryLock( const CHAR *pCSName, _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNCTXRENAMECS__TRYLOCK ) ;

      INT16 i = 0 ;
      while ( ( rc = _pDmsCB->blockWrite( cb ) )  &&
              ( i < RTN_RENAME_BLOCKWRITE_TIMES ) )
      {
         ossSleep( RTN_RENAME_BLOCKWRITE_INTERAL ) ;
         i++ ;
      }
      if ( SDB_DMS_STATE_NOT_COMPATIBLE == rc )
      {
         PD_LOG( PDERROR, "Rename cs/cl is mutually exclusive with "
                 "other rename cs/cl" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Block dms write failed, rc: %d", rc ) ;
      _lockDMS = TRUE ;

      if ( getDPSCB() )
      {
         dmsStorageUnitID suID = DMS_INVALID_CS;
         UINT32 logicCSID = DMS_INVALID_LOGICCSID;
         dmsStorageUnit *su = NULL;

         rc = _pDmsCB->nameToSUAndLock( pCSName, suID, &su );
         PD_RC_CHECK(rc, PDERROR, "lock collection space[%s] failed, rc: %d",
                     pCSName, rc );
         logicCSID = su->LogicalCSID();
         _pDmsCB->suUnlock ( suID ) ;

         rc = _pTransCB->transLockTryX( cb, logicCSID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Get transaction-lock of CS[%s] failed, rc: %d",
                      pCSName, rc ) ;
         _logicCSID = logicCSID ;
      }
   done:
      PD_TRACE_EXITRC( SDB__RTNCTXRENAMECS__TRYLOCK, rc ) ;
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCTXRENAMECS__RELLOCK, "_rtnContextRenameCS::_releaseLock" )
   INT32 _rtnContextRenameCS::_releaseLock( _pmdEDUCB *cb )
   {
      PD_TRACE_ENTRY( SDB__RTNCTXRENAMECS__RELLOCK ) ;

      if ( cb && getDPSCB() && ( _logicCSID != DMS_INVALID_LOGICCSID ) )
      {
         _pTransCB->transLockRelease( cb, _logicCSID );
         _logicCSID = DMS_INVALID_LOGICCSID;
      }

      if ( _lockDMS )
      {
         _pDmsCB->unblockWrite( cb ) ;
         _lockDMS = FALSE;
      }

      PD_TRACE_EXIT( SDB__RTNCTXRENAMECS__RELLOCK ) ;
      return SDB_OK ;
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextRenameCL, RTN_CONTEXT_RENAMECL, "RENAMECL")

   _rtnContextRenameCL::_rtnContextRenameCL( SINT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _pDmsCB        = pmdGetKRCB()->getDMSCB() ;
      _pDpsCB        = pmdGetKRCB()->getDPSCB() ;
      _pCatAgent     = pmdGetKRCB()->getClsCB()->getCatAgent () ;
      _pTransCB      = pmdGetKRCB()->getTransCB() ;
      _lockDMS       = FALSE ;
      _mbID          = DMS_INVALID_MBID ;
      _su            = NULL ;
      ossMemset( _clShortName, 0, sizeof( _clShortName ) ) ;
      ossMemset( _newCLShortName, 0, sizeof( _newCLShortName ) ) ;
      ossMemset( _clFullName, 0, sizeof( _clFullName ) ) ;
   }

   _rtnContextRenameCL::~_rtnContextRenameCL()
   {
      pmdEDUMgr *eduMgr    = pmdGetKRCB()->getEDUMgr() ;
      pmdEDUCB *cb         = eduMgr->getEDUByID( eduID() ) ;
      _releaseLock( cb ) ;
   }

   std::string _rtnContextRenameCL::name() const
   {
      return "RENAMECL" ;
   }

   RTN_CONTEXT_TYPE _rtnContextRenameCL::getType () const
   {
      return RTN_CONTEXT_RENAMECL ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCTXRENAMECL_OPEN, "_rtnContextRenameCL::open" )
   INT32 _rtnContextRenameCL::open( const CHAR *csName, const CHAR *clShortName,
                                    const CHAR *newCLShortName,
                                    _pmdEDUCB *cb, INT16 w )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNCTXRENAMECL_OPEN ) ;

      CHAR newCLFullName[ DMS_COLLECTION_FULL_NAME_SZ + 1 ] = { 0 } ;

      /// set w info
      _w = w ;

      /// check cs name
      SDB_ASSERT( csName, "cs name can't be null!" );
      PD_CHECK( csName, SDB_INVALIDARG, error, PDERROR,
                "cs name is null!" );

      rc = dmsCheckCSName( csName );
      PD_RC_CHECK( rc, PDERROR, "Invalid cs name[%s]", csName ) ;

      /// check cl name
      SDB_ASSERT( clShortName, "collection name can't be null!" );
      PD_CHECK( clShortName, SDB_INVALIDARG, error, PDERROR,
                "collection name is null!" );

      SDB_ASSERT( newCLShortName, "new collection name can't be null!" );
      PD_CHECK( newCLShortName, SDB_INVALIDARG, error, PDERROR,
                "new collection name is null!" );

      rc = dmsCheckCLName( clShortName );
      PD_RC_CHECK( rc, PDERROR, "Invalid collection name[%s]", clShortName ) ;

      rc = dmsCheckCLName( newCLShortName );
      PD_RC_CHECK( rc, PDERROR, "Invalid collection name[%s]", newCLShortName );

      ossStrncpy( _clShortName, clShortName, DMS_COLLECTION_NAME_SZ ) ;
      ossStrncpy( _newCLShortName, newCLShortName, DMS_COLLECTION_NAME_SZ ) ;
      ossSnprintf( _clFullName, DMS_COLLECTION_FULL_NAME_SZ,
                   "%s.%s", csName, clShortName ) ;
      ossSnprintf( newCLFullName, DMS_COLLECTION_FULL_NAME_SZ,
                   "%s.%s", csName, newCLShortName ) ;

      /// test collection space exist
      rc = rtnTestCollectionCommand( _clFullName, _pDmsCB ) ;
      if ( SDB_DMS_NOTEXIST == rc )
      {
         PD_LOG( PDERROR,
                 "Collection[%s] does not exists, rc: %d",
                 _clFullName, rc ) ;
         goto done ;
      }

      rc = rtnTestCollectionSpaceCommand( newCLFullName, _pDmsCB ) ;
      if ( SDB_OK == rc )
      {
         rc = SDB_DMS_EXIST ;
         PD_LOG( PDERROR,
                 "Collection[%s] already exists, rc: %d",
                 newCLFullName, rc ) ;
         goto done ;
      }

      /// lock
      rc = _tryLock( csName, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to lock, rc: %d", rc ) ;

      _isOpened = TRUE ;

   done:
      PD_TRACE_EXITRC( SDB__RTNCTXRENAMECL_OPEN, rc ) ;
      return rc ;
   error:
      _releaseLock( cb ) ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCTXRENAMECL_GETMORE, "_rtnContextRenameCL::getMore" )
   INT32 _rtnContextRenameCL::getMore( INT32 maxNumToReturn,
                                       rtnContextBuf &buffObj,
                                       _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNCTXRENAMECL_GETMORE ) ;

      SDB_RTNCB * pRtnCB = pmdGetKRCB()->getRTNCB() ;
      clsCB * pClsCB = pmdGetKRCB()->getClsCB() ;
      CHAR mainCL[ DMS_COLLECTION_FULL_NAME_SZ + 1 ] = { '\0' } ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE;
         goto error ;
      }

      _pCatAgent->lock_w () ;
      _pCatAgent->clear( _clFullName, mainCL ) ;
      _pCatAgent->release_w () ;
      pClsCB->invalidateCata( _clFullName ) ;

      // Clear catalog info and cached plans of main-collection if needed
      if ( '\0' != mainCL[ 0 ] )
      {
         _pCatAgent->lock_w() ;
         _pCatAgent->clear( mainCL ) ;
         _pCatAgent->release_w() ;
         pRtnCB->getAPM()->invalidateCLPlans( mainCL ) ;
         pClsCB->invalidateCache( mainCL, DPS_LOG_INVALIDCATA_TYPE_CATA |
                                          DPS_LOG_INVALIDCATA_TYPE_PLAN ) ;
      }

      // rename collection
      rc = _su->data()->renameCollection ( _clShortName, _newCLShortName,
                                           cb, _pDpsCB ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to rename collection from [%s] to [%s], rc: %d",
                   _clShortName, _newCLShortName, rc ) ;

      _releaseLock( cb ) ;
      _isOpened = FALSE ;
      rc = SDB_DMS_EOC ;

   done:
      PD_TRACE_EXITRC( SDB__RTNCTXRENAMECL_GETMORE, rc ) ;
      return rc;
   error:
      goto done;
   }

   void _rtnContextRenameCL::_toString( stringstream &ss )
   {
      ss << ",Name:" << _clFullName
         << ",NewCLName:" << _newCLShortName
         << ",LockDMS:" << _lockDMS
         << ",MBID:" << _mbID ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCTXRENAMECL__TRYLOCK, "_rtnContextRenameCL::_tryLock" )
   INT32 _rtnContextRenameCL::_tryLock( const CHAR *csName, _pmdEDUCB *cb )
   {
      INT32 rc                = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNCTXRENAMECL__TRYLOCK ) ;

      dmsStorageUnitID suID   = DMS_INVALID_CS ;
      dmsMBContext* mbContext = NULL ;
      UINT16 mbID             = DMS_INVALID_MBID ;

      INT16 i = 0 ;
      while ( ( rc = _pDmsCB->blockWrite( cb ) )  &&
              ( i < RTN_RENAME_BLOCKWRITE_TIMES ) )
      {
         ossSleep( RTN_RENAME_BLOCKWRITE_INTERAL ) ;
         i++ ;
      }
      if ( SDB_DMS_STATE_NOT_COMPATIBLE == rc )
      {
         PD_LOG( PDERROR, "Rename cs/cl is mutually exclusive with "
                 "other rename cs/cl" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Block dms write failed, rc: %d", rc ) ;
      _lockDMS = TRUE ;

      if ( getDPSCB() )
      {
         rc = _pDmsCB->nameToSUAndLock( csName, suID, &_su, SHARED );
         PD_RC_CHECK( rc, PDERROR, "lock collection space[%s] failed, rc: %d",
                      csName, rc );

         rc = _su->data()->getMBContext( &mbContext, _clShortName, SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", _clShortName, rc ) ;

         mbID = mbContext->mbID() ;
         _su->data()->releaseMBContext( mbContext ) ;
         _pDmsCB->suUnlock ( suID ) ;

         rc = _pTransCB->transLockTryX( cb, _su->LogicalCSID(), mbID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Get transaction-lock of collection[%s] failed, rc: %d",
                      _clFullName, rc ) ;
         _mbID = mbID ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNCTXRENAMECL__TRYLOCK, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNCTXRENAMECL__RELLOCK, "_rtnContextRenameCL::_releaseLock" )
   INT32 _rtnContextRenameCL::_releaseLock( _pmdEDUCB *cb )
   {
      PD_TRACE_ENTRY( SDB__RTNCTXRENAMECL__RELLOCK ) ;

      if ( cb && _mbID != DMS_INVALID_MBID )
      {
         _pTransCB->transLockRelease( cb, _su->LogicalCSID(), _mbID ) ;
         _mbID = DMS_INVALID_MBID ;
      }

      if ( _lockDMS )
      {
         _pDmsCB->unblockWrite( cb ) ;
         _lockDMS = FALSE;
      }

      PD_TRACE_EXIT( SDB__RTNCTXRENAMECL__RELLOCK ) ;
      return SDB_OK ;
   }
}

