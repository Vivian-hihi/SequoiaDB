/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dmsCB.cpp

   Descriptive Name = Data Management Service Control Block

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains code logic for
   data management control block, which is the metatdata information for DMS
   component.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "dmsCB.hpp"
#include "dms.hpp"
#include "dmsStorageUnit.hpp"
#include "ossLatch.hpp"
#include "ossUtil.hpp"
#include "monDMS.hpp"
#include "dmsTempCB.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "dmsTrace.hpp"
#include "dpsOp2Record.hpp"

using namespace std;
namespace engine
{

   /*
      _SDB_DMS_CSCB implement
   */
   _SDB_DMS_CSCB::~_SDB_DMS_CSCB()
   {
      if ( _su )
      {
         SDB_OSS_DEL _su ;
      }
   }

   /*
      _SDB_DMSCB implement
   */
   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB__LGCSCBNMMAP, "_SDB_DMSCB::_logCSCBNameMap" )
   void _SDB_DMSCB::_logCSCBNameMap ()
   {
      PD_TRACE_ENTRY ( SDB__SDB_DMSCB__LGCSCBNMMAP );
#if defined (_WINDOWS)
      std::map<const CHAR*, dmsStorageUnitID, cmp_cscb>::const_iterator it ;
#elif defined (_LINUX)
      std::map<const CHAR*, dmsStorageUnitID>::const_iterator it ;
#endif
      for ( it = _cscbNameMap.begin(); it != _cscbNameMap.end() ;
            it ++ )
      {
         PD_LOG ( PDDEBUG, "%s\n", it->first ) ;
      }
      PD_TRACE_EXIT ( SDB__SDB_DMSCB__LGCSCBNMMAP );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB__CSCBNMINST, "_SDB_DMSCB::_CSCBNameInsert" )
   INT32 _SDB_DMSCB::_CSCBNameInsert ( const CHAR *pName, UINT32 topSequence,
                                       _dmsStorageUnit *su,
                                       dmsStorageUnitID &suID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SDB_DMSCB__CSCBNMINST );
      SDB_DMS_CSCB *cscb = NULL ;
      if ( 0 == _freeList.size() )
      {
         rc = SDB_DMS_SU_OUTRANGE ;
         goto error ;
      }
      cscb = SDB_OSS_NEW SDB_DMS_CSCB(pName, topSequence, su) ;
      if ( !cscb )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory to insert cscb" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      suID = _freeList.back() ;
      su->_setCSID( suID ) ;
      su->_setLogicalCSID( _logicalSUID++ ) ;
      _freeList.pop_back() ;
      _cscbNameMap[cscb->_name] = suID ;
      _cscbVec[suID] = cscb ;

   done :
      PD_TRACE_EXITRC ( SDB__SDB_DMSCB__CSCBNMINST, rc );
      return rc ;
   error :
      goto done ;
   }

   SDB_DMS_CSCB *_SDB_DMSCB::_CSCBNameLookup ( const CHAR *pName )
   {
#if defined (_WINDOWS)
      std::map<const CHAR*, dmsStorageUnitID, cmp_cscb>::const_iterator it ;
#elif defined (_LINUX)
      std::map<const CHAR*, dmsStorageUnitID>::const_iterator it ;
#endif
      if ( _cscbNameMap.end() == (it = _cscbNameMap.find(pName)) )
      {
         return NULL ;
      }
      return _cscbVec[(*it).second] ;
   }

   INT32 _SDB_DMSCB::_CSCBNameLookupAndLock ( const CHAR *pName,
                                              dmsStorageUnitID &suID,
                                              SDB_DMS_CSCB **cscb,
                                              OSS_LATCH_MODE lockType,
                                              INT32 millisec )
   {
      INT32 rc = SDB_OK;
      SDB_ASSERT( cscb, "cscb can't be null!" );
      suID = DMS_INVALID_CS ;
      *cscb = NULL;
#if defined (_WINDOWS)
      std::map<const CHAR*, dmsStorageUnitID, cmp_cscb>::const_iterator it ;
#elif defined (_LINUX)
      std::map<const CHAR*, dmsStorageUnitID>::const_iterator it ;
#endif
      if ( _cscbNameMap.end() == (it = _cscbNameMap.find(pName)) )
      {
         return SDB_DMS_CS_NOTEXIST ;
      }
      dmsStorageUnitID temp_suID = (*it).second ;
      if ( NULL == _cscbVec[temp_suID] )
      {
         return SDB_DMS_CS_NOTEXIST ;
      }

      if ( EXCLUSIVE == lockType )
      {
         rc = _latchVec[temp_suID]->lock_w( millisec ) ;
      }
      else
      {
         rc = _latchVec[temp_suID]->lock_r( millisec ) ;
      }
      if ( SDB_OK == rc )
      {
         suID = temp_suID ;
         *cscb = _cscbVec[suID] ;
      }
      return rc;
   }

   void _SDB_DMSCB::_CSCBRelease ( dmsStorageUnitID suID,
                                   OSS_LATCH_MODE lockType )
   {
      if ( EXCLUSIVE == lockType )
      {
         _latchVec[suID]->release_w();
      }
      else
      {
         _latchVec[suID]->release_r() ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB__CSCBNMREMV, "_SDB_DMSCB::_CSCBNameRemove" )
   INT32 _SDB_DMSCB::_CSCBNameRemove ( const CHAR *pName,
                                       _pmdEDUCB *cb,
                                       SDB_DPSCB *dpsCB,
                                       SDB_DMS_CSCB *&pCSCB,
                                       BOOLEAN &hasLocked )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SDB_DMSCB__CSCBNMREMV );
      dmsStorageUnitID suID ;
      UINT32 csLID = ~0 ;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB();
      BOOLEAN isTransLocked = FALSE;
      BOOLEAN isReserved = FALSE;
      UINT32 logRecSize = 0;
      dpsMergeInfo info ;
      dpsLogRecord &record = info.getMergeBlock().record();
#if defined (_WINDOWS)
      std::map<const CHAR*, dmsStorageUnitID, cmp_cscb>::const_iterator it ;
#elif defined (_LINUX)
      std::map<const CHAR*, dmsStorageUnitID>::const_iterator it ;
#endif
      pCSCB = NULL ;
      _mutex.get_shared () ;
      if ( _cscbNameMap.end() == (it = _cscbNameMap.find( pName )))
      {
         _mutex.release_shared () ;
         rc = SDB_DMS_CS_NOTEXIST ;
         goto error ;
      }
      suID = (*it).second ;
      _mutex.release_shared () ;

      if ( NULL != dpsCB )
      {
         // reserved log-size
         rc = dpsCSDel2Record( pName, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build record:%d",rc ) ;
            goto error ;
         }
         logRecSize = record.alignedLen() ;
         rc = pTransCB->reservedLogSpace( logRecSize );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to reserved log space(length=%u)",
                     logRecSize );
         isReserved = TRUE ;
      }

   retry :
      // now let's lock the collectionspace, if we can't lock it, let's return
      // false. we shouldn't wait forever
      //if ( !_latchVec[suID]->try_get() )
      if ( !hasLocked )
      {
         if ( SDB_OK != _latchVec[suID]->lock_w( OSS_ONE_SEC ) )
         {
            rc = SDB_LOCK_FAILED ;
            goto error ;
         }
         hasLocked = TRUE;
      }
      if ( !_mutex.try_get() )
      {
         _latchVec[suID]->release_w () ;
         hasLocked = FALSE;
         goto retry ;
      }

      _latchVec[suID]->release_w () ;
      hasLocked = FALSE;

      // there is a small timing hole before getting the latch, so we have
      // to get current suID again to verify
      if ( _cscbNameMap.end() == (it = _cscbNameMap.find(pName)) ||
           suID != (*it).second )
      {
         // if we no longer able to find the collectionspace, or the same
         // name maps to another suID, then let's get out of here
         //_latchVec[suID]->release () ;
         _mutex.release () ;
         rc = SDB_DMS_CS_NOTEXIST ;
         goto error ;
      }
      if ( _cscbVec[suID] )
      {
         pCSCB = _cscbVec[suID] ;
         SDB_ASSERT ( pCSCB->_su, "su can't be null" ) ;
         csLID = pCSCB->_su->LogicalCSID () ;
         if ( cb )
         {
            rc = pTransCB->transLockTryX( cb, csLID );
            if ( rc )
            {
               _mutex.release () ;
               PD_LOG ( PDERROR, "Failed to lock collection-space(rc=%d)", rc ) ;
               goto error ;
            }
            isTransLocked = TRUE ;
         }
         _cscbVec[suID] = NULL ;
      }

      _cscbNameMap.erase(pName) ;
      _freeList.push_back ( suID ) ;

      // log here
      if ( dpsCB )
      {
         DPS_LSN_OFFSET offset ;
         info.setInfoEx( csLID, ~0, DMS_INVALID_EXTENT ) ;
         rc = dpsCB->prepare ( info ) ;
         if ( rc )
         {
            _mutex.release () ;
            PD_LOG ( PDERROR, "Failed to insert cscrt into log, rc = %d", rc ) ;
            goto error ;
         }
         dpsCB->writeData( info ) ;
         if ( cb && pmdGetKRCB()->getDBRole() != SDB_ROLE_STANDALONE )
         {
            if ( info.hasDummy() )
            {
               offset = info.getDummyBlock().record().head()._lsn ;
               cb->insertLsn ( offset ) ;
               pmdGetKRCB()->getReplCB()->notify ( csLID, ~0,
                                                   DMS_INVALID_EXTENT,
                                                   offset ) ;
            }
            offset = info.getMergeBlock().record().head()._lsn ;
            cb->insertLsn ( offset ) ;
            pmdGetKRCB()->getReplCB()->notify( csLID, ~0,
                                               DMS_INVALID_EXTENT,
                                               offset ) ;
         }
      }

      _mutex.release () ;

   done :
      SDB_ASSERT( !hasLocked, "space-lock unreleased!" );
      if ( isTransLocked )
      {
         pTransCB->transLockRelease( cb, csLID );
      }
      if ( isReserved )
      {
         pTransCB->releaseLogSpace( logRecSize );
      }
      PD_TRACE_EXITRC ( SDB__SDB_DMSCB__CSCBNMREMV, rc );
      return rc ;
   error :
      pCSCB = NULL ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB__CSCBNMMAPCLN, "_SDB_DMSCB::_CSCBNameMapCleanup" )
   void _SDB_DMSCB::_CSCBNameMapCleanup ()
   {
      PD_TRACE_ENTRY ( SDB__SDB_DMSCB__CSCBNMMAPCLN );
#if defined (_WINDOWS)
      std::map<const CHAR*, dmsStorageUnitID, cmp_cscb>::const_iterator it ;
#elif defined (_LINUX)
      std::map<const CHAR*, dmsStorageUnitID>::const_iterator it ;
#endif
      for ( it = _cscbNameMap.begin(); it != _cscbNameMap.end(); it++ )
      {
         dmsStorageUnitID suID = (*it).second ;
         _latchVec[suID]->lock_w () ;
         _freeList.push_back ( suID ) ;
         if ( _cscbVec[suID] )
         {
            SDB_OSS_DEL _cscbVec[suID] ;
            _cscbVec[suID] = NULL ;
         }
         _latchVec[suID]->release_w () ;
      }
      _cscbNameMap.clear() ;
      PD_TRACE_EXIT ( SDB__SDB_DMSCB__CSCBNMMAPCLN );
   }

   INT32 _SDB_DMSCB::writable( _pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN locked = FALSE ;

   retry:
      _stateMtx.get () ;
      locked = TRUE ;

      switch ( _dmsCBState )
      {
      case DMS_STATE_BACKUP :
         rc = SDB_RTN_IN_BACKUP ;
         break ;
      case DMS_STATE_REBUILD :
         rc = SDB_RTN_IN_REBUILD ;
         break ;
      default :
         break ;
      }
      if ( SDB_OK == rc )
      {
         ++_writeCounter ;
      }
      else if ( cb )
      {
         _stateMtx.release() ;
         locked = FALSE ;

         while ( TRUE )
         {
            if ( cb->isInterrupted() )
            {
               rc = SDB_APP_INTERRUPT ;
               break ;
            }
            rc = _backEvent.wait( OSS_ONE_SEC ) ;
            if ( SDB_OK == rc )
            {
               goto retry ;
            }
         }
      }

      if ( locked )
      {
         _stateMtx.release() ;
      }
      return rc;
   }

   INT32 _SDB_DMSCB::registerBackup()
   {
      INT32 rc = SDB_OK ;
      _stateMtx.get();
      if ( DMS_STATE_NORMAL != _dmsCBState )
      {
         if ( DMS_STATE_BACKUP == _dmsCBState )
            rc = SDB_BACKUP_HAS_ALREADY_START ;
         else
            rc = SDB_DMS_STATE_NOT_COMPATIBLE ;
         _stateMtx.release () ;
         goto done;
      }
      _dmsCBState = DMS_STATE_BACKUP ;
      _stateMtx.release () ;

      while ( TRUE )
      {
         _stateMtx.get();
         if ( 0 == _writeCounter )
         {
            _backEvent.reset() ;
            _stateMtx.release();
            goto done;
         }
         else
         {
            _stateMtx.release();
            ossSleepmillis( DMS_CHANGESTATE_WAIT_LOOP );
         }
      }
   done:
      return rc;
   }

   INT32 _SDB_DMSCB::registerRebuild()
   {
      INT32 rc = SDB_OK ;
      _stateMtx.get();
      if ( DMS_STATE_NORMAL != _dmsCBState )
      {
         if ( DMS_STATE_REBUILD == _dmsCBState )
            rc = SDB_REBUILD_HAS_ALREADY_START ;
         else
            rc = SDB_DMS_STATE_NOT_COMPATIBLE ;
         _stateMtx.release () ;
         goto done;
      }
      _dmsCBState = DMS_STATE_REBUILD ;
      _stateMtx.release () ;

      while ( TRUE )
      {
         _stateMtx.get();
         if ( 0 == _writeCounter )
         {
            _stateMtx.release();
            goto done;
         }
         else
         {
            _stateMtx.release();
            ossSleepmillis( DMS_CHANGESTATE_WAIT_LOOP );
         }
      }
   done :
      return rc ;
   }

   INT32 _SDB_DMSCB::nameToSUAndLock ( const CHAR *pName,
                                       dmsStorageUnitID &suID,
                                       _dmsStorageUnit **su,
                                       OSS_LATCH_MODE lockType,
                                       INT32 millisec )
   {
      INT32 rc = SDB_OK;
      SDB_DMS_CSCB *cscb = NULL;
      SDB_ASSERT( su, "su can't be null!" );
      if ( !pName )
      {
         return SDB_INVALIDARG ;
      }
      DMSCB_SLOCK
      rc = _CSCBNameLookupAndLock( pName, suID,
                                 &cscb, lockType,
                                 millisec ) ;
      if ( SDB_OK == rc )
      {
         *su = cscb->_su;
      }
      return rc ;
   }

   _dmsStorageUnit *_SDB_DMSCB::nameToSU ( const CHAR *pName )
   {
      if ( !pName )
      {
         return NULL ;
      }
      DMSCB_SLOCK
      SDB_DMS_CSCB *cscb = _CSCBNameLookup( pName );
      if ( cscb )
      {
         return cscb->_su ;
      }
      return NULL ;
   }

   _dmsStorageUnit *_SDB_DMSCB::suLock ( dmsStorageUnitID suID )
   {
      DMSCB_SLOCK
      if ( NULL == _cscbVec[suID] )
      {
         return NULL ;
      }
      _latchVec[suID]->lock_r() ;
      return _cscbVec[suID]->_su ;
   }

   void _SDB_DMSCB::suUnlock ( dmsStorageUnitID suID,
                              OSS_LATCH_MODE lockType )
   {
      _CSCBRelease( suID, lockType ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB_ADDCS, "_SDB_DMSCB::addCollectionSpace" )
   INT32 _SDB_DMSCB::addCollectionSpace(const CHAR * pName,
                                        UINT32 topSequence,
                                        _dmsStorageUnit * su,
                                        _pmdEDUCB *cb,
                                        SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      dmsStorageUnitID suID ;
      SDB_DMS_CSCB *cscb = NULL ;
      BOOLEAN isReserved = FALSE;
      UINT32 logRecSize = 0;
      dpsMergeInfo info ;
      dpsLogRecord &record = info.getMergeBlock().record();
      INT32 pageSize = 0 ;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB();
      PD_TRACE_ENTRY ( SDB__SDB_DMSCB_ADDCS );
      DMSCB_XLOCK
      if ( !pName || !su )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      cscb = _CSCBNameLookup( pName ) ;
      if ( cscb )
      {
         rc = SDB_DMS_CS_EXIST ;
         goto error ;
      }

      pageSize = su->getPageSize() ;
      if ( NULL != dpsCB )
      {
         // reserved log-size
         rc = dpsCSCrt2Record( pName, pageSize, record ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to build record:%d",rc ) ;
            goto error ;
         }
         logRecSize = record.alignedLen() ;
         rc = pTransCB->reservedLogSpace( logRecSize );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to reserved log space(length=%u)",
                     logRecSize );
         isReserved = TRUE ;
      }

      rc = _CSCBNameInsert ( pName, topSequence, su, suID ) ;
      // write dps
      if ( SDB_OK == rc && dpsCB )
      {
         UINT32 suLID = su->LogicalCSID() ;
         DPS_LSN_OFFSET offset = 0 ;
         info.setInfoEx( suLID, ~0, DMS_INVALID_EXTENT ) ;
         rc = dpsCB->prepare ( info ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to insert cscrt into log, rc = %d", rc ) ;
            goto error ;
         }
         dpsCB->writeData( info ) ;
         if ( cb && pmdGetKRCB()->getDBRole() != SDB_ROLE_STANDALONE )
         {
            if ( info.hasDummy() )
            {
               offset = info.getDummyBlock().record().head()._lsn ;
               cb->insertLsn( offset ) ;
               pmdGetKRCB()->getReplCB()->notify( suLID, ~0, DMS_INVALID_EXTENT,
                                                  offset ) ;
            }
            offset = info.getMergeBlock().record().head()._lsn ;
            cb->insertLsn ( offset ) ;
            pmdGetKRCB()->getReplCB()->notify( suLID, ~0, DMS_INVALID_EXTENT,
                                               offset ) ;
         }
      }
   done :
      if ( isReserved )
      {
         pTransCB->releaseLogSpace( logRecSize );
      }
      PD_TRACE_EXITRC ( SDB__SDB_DMSCB_ADDCS, rc );
      return rc ;
   error :
      goto done ;
   }

   // note:, the _dmsStorageUnit object will NOT be deleted
   // caller is responsible to delete the object AFTER removing it from DMSCB
   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB_DROPCS, "_SDB_DMSCB::dropCollectionSpace" )
   INT32 _SDB_DMSCB::dropCollectionSpace ( const CHAR *pName, _pmdEDUCB *cb,
                                           SDB_DPSCB *dpsCB,
                                           BOOLEAN &hasLocked )
   {
      INT32 rc = SDB_OK ;
      SDB_DMS_CSCB *pCSCB = NULL ;

      PD_TRACE_ENTRY ( SDB__SDB_DMSCB_DROPCS ) ;
      if ( !pName )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      {
         _mutex.get_shared() ;
         SDB_DMS_CSCB *cscb = _CSCBNameLookup( pName ) ;
         if ( !cscb )
         {
            _mutex.release_shared() ;
            rc = SDB_DMS_CS_NOTEXIST ;
            goto error ;
         }
         dmsStorageUnit *su = cscb->_su ;
         SDB_ASSERT ( su, "storage unit pointer can't be NULL" )

         // release the DMSCB latch before attempting to drop the collectionspace
         _mutex.release_shared() ;
         rc = _CSCBNameRemove ( pName, cb, dpsCB, pCSCB, hasLocked ) ;

         if ( SDB_OK == rc && pCSCB )
         {
            // if remove file failed, we can do nothing
            rc = pCSCB->_su->remove() ;
            SDB_OSS_DEL pCSCB ;

            if ( rc )
            {
               goto error ;
            }
         }
      }
   done :
      PD_TRACE_EXITRC ( SDB__SDB_DMSCB_DROPCS, rc );
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB_DUMPINFO, "_SDB_DMSCB::dumpInfo" )
   void _SDB_DMSCB::dumpInfo ( std::set<monCollection> &collectionList,
                               BOOLEAN sys )
   {
      PD_TRACE_ENTRY ( SDB__SDB_DMSCB_DUMPINFO );
      dmsStorageUnit *su = NULL ;
      DMSCB_SLOCK
#if defined (_WINDOWS)
      std::map<const CHAR*, dmsStorageUnitID, cmp_cscb>::const_iterator it ;
#elif defined (_LINUX)
      std::map<const CHAR*, dmsStorageUnitID>::const_iterator it ;
#endif
      for ( it = _cscbNameMap.begin(); it != _cscbNameMap.end(); it++ )
      {
         su = NULL ;
         dmsStorageUnitID suID = (*it).second ;
         //ossScopedLock lock ( _latchVec[suID], SHARED ) ;
         ossScopedRWLock lock ( _latchVec[suID], SHARED ) ;
         SDB_DMS_CSCB *cscb = _cscbVec[suID] ;
         if ( !cscb )
            continue ;
         su = cscb->_su ;
         SDB_ASSERT ( su, "storage unit pointer can't be NULL" )

         if ( ( !sys && dmsIsSysCSName(su->CSName()) ) ||
              ( ossStrcmp ( su->CSName(), SDB_DMSTEMP_NAME ) == 0 ) )
         {
            continue ;
         }
         su->dumpInfo ( collectionList, sys ) ;
      } // for ( it = _cscbNameMap.begin(); it != _cscbNameMap.end(); it++ )
      PD_TRACE_EXIT ( SDB__SDB_DMSCB_DUMPINFO );
   }  // void dumpInfo

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB_DUMPINFO2, "_SDB_DMSCB::dumpInfo" )
   void _SDB_DMSCB::dumpInfo ( std::set<monCollectionSpace> &csList,
                               BOOLEAN sys )
   {
      PD_TRACE_ENTRY ( SDB__SDB_DMSCB_DUMPINFO2 );
      dmsStorageUnit *su = NULL ;
      DMSCB_SLOCK
#if defined (_WINDOWS)
      std::map<const CHAR*, dmsStorageUnitID, cmp_cscb>::const_iterator it ;
#elif defined (_LINUX)
      std::map<const CHAR*, dmsStorageUnitID>::const_iterator it ;
#endif
      for ( it = _cscbNameMap.begin(); it != _cscbNameMap.end(); it++ )
      {
         dmsStorageUnitID suID = (*it).second ;
         ossScopedRWLock lock ( _latchVec[suID], SHARED ) ;
         SDB_DMS_CSCB *cscb = _cscbVec[suID] ;
         if ( !cscb )
         {
            continue ;
         }
         su = cscb->_su ;
         SDB_ASSERT ( su, "storage unit pointer can't be NULL" )
         if ( !sys && dmsIsSysCSName(cscb->_name) )
         {
            continue ;
         }
         // do not dump temp cs
         else if ( dmsIsSysCSName(cscb->_name) &&
                   0 == ossStrcmp(cscb->_name, SDB_DMSTEMP_NAME ) )
         {
            continue ;
         }
         monCollectionSpace cs ;
         ossMemset ( cs._name, 0, sizeof(cs._name) ) ;
         ossStrncpy ( cs._name, cscb->_name, DMS_COLLECTION_SPACE_NAME_SZ);
         cs._pageSize = su->getPageSize() ;
         su->dumpInfo ( cs._collections, sys ) ;
         csList.insert ( cs ) ;
      }
      PD_TRACE_EXIT ( SDB__SDB_DMSCB_DUMPINFO2 );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__SDB_DMSCB_DUMPINFO3, "_SDB_DMSCB::dumpInfo" )
   void _SDB_DMSCB::dumpInfo ( std::set<monStorageUnit> &storageUnitList,
                               BOOLEAN sys )
   {
      PD_TRACE_ENTRY ( SDB__SDB_DMSCB_DUMPINFO3 );
      dmsStorageUnit *su = NULL ;
      DMSCB_SLOCK
#if defined (_WINDOWS)
      std::map<const CHAR*, dmsStorageUnitID, cmp_cscb>::const_iterator it ;
#elif defined (_LINUX)
      std::map<const CHAR*, dmsStorageUnitID>::const_iterator it ;
#endif
      for ( it = _cscbNameMap.begin(); it != _cscbNameMap.end(); it++ )
      {
         su = NULL ;
         dmsStorageUnitID suID = (*it).second ;
         ossScopedRWLock lock ( _latchVec[suID], SHARED ) ;
         SDB_DMS_CSCB *cscb = _cscbVec[suID] ;
         if ( !cscb )
         {
            continue ;
         }
         su = cscb->_su ;
         SDB_ASSERT ( su, "storage unit pointer can't be NULL" )
         if ( !sys && dmsIsSysCSName(su->CSName()) )
         {
            continue ;
         }
         su->dumpInfo ( storageUnitList, sys ) ;
      } // for ( it = _cscbNameMap.begin(); it != _cscbNameMap.end(); it++ )
      PD_TRACE_EXIT ( SDB__SDB_DMSCB_DUMPINFO3 );
   }  // void dumpInfo

   dmsTempCB *_SDB_DMSCB::getTempCB ()
   {
      return &_tempCB ;
   }

}

