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

   Source File Name = rtnBackgroundJob.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/06/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnBackgroundJob.hpp"
#include "rtn.hpp"
#include "ixm.hpp"
#include "dmsStorageUnit.hpp"
#include "dmsStorageLoadExtent.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

namespace engine
{
   rtnJobMgr * rtnGetJobMgr ()
   {
      static rtnJobMgr _jobMgr ( pmdGetKRCB()->getEDUMgr() ) ;
      return &_jobMgr ;
   }

   _rtnJobMgr::_rtnJobMgr ( pmdEDUMgr * eduMgr )
   {
      SDB_ASSERT ( eduMgr, "EDU Mgr can't be NULL" ) ;
      _eduMgr = eduMgr ;
   }

   _rtnJobMgr::~_rtnJobMgr ()
   {
      _eduMgr = NULL ;
   }

   UINT32 _rtnJobMgr::jobsCount ()
   {
      ossScopedLock lock ( &_latch, SHARED ) ;
      return _mapJobs.size() ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNJOBMGR_FINDJOB, "_rtnJobMgr::findJob" )
   _rtnBaseJob* _rtnJobMgr::findJob( EDUID eduID, INT32 *pResult )
   {
      PD_TRACE_ENTRY ( SDB__RTNJOBMGR_FINDJOB ) ;

      {
         ossScopedLock lock ( &_latch, SHARED ) ;
         std::map<EDUID, _rtnBaseJob*>::iterator it = _mapJobs.find( eduID ) ;
         if ( it != _mapJobs.end() )
         {
            PD_TRACE_EXIT ( SDB__RTNJOBMGR_FINDJOB ) ;
            return it->second ;
         }
      }

      INT32 res = SDB_OK ;
      {
         ossScopedLock lock ( &_latch, EXCLUSIVE ) ;
         std::map<EDUID, INT32>::iterator itRes = _mapResult.find( eduID ) ;
         if ( itRes != _mapResult.end() )
         {
            res = itRes->second ;
            _mapResult.erase( itRes ) ;
         }
      }
      if ( pResult )
      {
         *pResult = res ;
      }
      PD_TRACE_EXIT ( SDB__RTNJOBMGR_FINDJOB ) ;
      return NULL ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNJOBMGR_STARTJOB, "_rtnJobMgr::startJob" )
   INT32 _rtnJobMgr::startJob ( _rtnBaseJob * pJob,
                                RTN_JOB_MUTEX_TYPE type ,
                                EDUID * pEDUID,
                                BOOLEAN returnResult )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__RTNJOBMGR_STARTJOB ) ;
      EDUID newEDUID = 0 ;
      BOOLEAN isMuted = FALSE ;

      ossScopedLock lock ( &_latch, EXCLUSIVE ) ;

      // if mutex, need to stop
      if ( RTN_JOB_MUTEX_NONE != type )
      {
         _rtnBaseJob *itJob = NULL ;
         std::map<EDUID, _rtnBaseJob*>::iterator it = _mapJobs.begin() ;
         while ( it != _mapJobs.end() )
         {
            itJob = it->second ;
            if ( pJob->muteXOn( itJob ) || itJob->muteXOn( pJob ) )
            {
               isMuted = TRUE ;
               PD_LOG ( PDINFO, "Exist job[%s] mutex with new job[%s]",
                        itJob->name(), pJob->name() ) ;

               if ( RTN_JOB_MUTEX_RET == type )
               {
                  rc = SDB_RTN_MUTEX_JOB_EXIST ;
                  goto error ;
               }
               else if ( RTN_JOB_MUTEX_REUSE == type )
               {
                  if ( pEDUID )
                  {
                     *pEDUID = it->first ;
                     //_mapResult[newEDUID] = SDB_OK ;
                  }
                  SDB_OSS_DEL pJob ;
                  pJob = NULL ;
                  goto done ;
               }
               else
               {
                  _stopJob ( it->first ) ;
                  itJob->waitDetach () ;
               }
            }
            ++it ;
         }
      }

      if ( isMuted && RTN_JOB_MUTEX_STOP_RET == type )
      {
         rc = SDB_RTN_MUTEX_JOB_EXIST ;
         goto error ;
      }

      // start new edu
      rc = _eduMgr->startEDU( EDU_TYPE_BACKGROUND_JOB, (void*)pJob,
                              &newEDUID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Start background job[%s] failed, rc = %d",
                  pJob->name() , rc ) ;
         goto error ;
      }

      // wait edu attach in
      pJob->waitAttach () ;
      // add to map
      _mapJobs[newEDUID] = pJob ;
      //_mapResult[newEDUID] = SDB_OK ;

      if ( pEDUID )
      {
         *pEDUID = newEDUID ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__RTNJOBMGR_STARTJOB, rc ) ;
      return rc ;
   error:
      SDB_OSS_DEL pJob ;
      goto done ;
   }

   INT32 _rtnJobMgr::_stopJob ( EDUID eduID )
   {
      return _eduMgr->forceUserEDU ( eduID ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNJOBMGR__REMOVEJOB, "_rtnJobMgr::_removeJob" )
   INT32 _rtnJobMgr::_removeJob ( EDUID eduID, INT32 result )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__RTNJOBMGR__REMOVEJOB ) ;
      ossScopedLock lock ( &_latch, EXCLUSIVE ) ;
      /*std::map<EDUID, INT32>::iterator itRes = _mapResult.find( eduID ) ;
      if ( itRes != _mapResult.end() )
      {
         itRes->second = result ;
      }*/
      if ( result )
      {
         _mapResult[ eduID ] = result ;
      }

      std::map<EDUID, _rtnBaseJob*>::iterator it = _mapJobs.find ( eduID ) ;
      if ( it == _mapJobs.end() )
      {
         rc = SDB_RTN_JOB_NOT_EXIST ;
         goto error ;
      }

      // free memory and erase map item
      SDB_OSS_DEL it->second ;
      _mapJobs.erase ( it ) ;

   done:
      PD_TRACE_EXITRC ( SDB__RTNJOBMGR__REMOVEJOB, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   _rtnBaseJob::_rtnBaseJob ()
   {
      _pEDUCB = NULL ;
      _latchIn.try_get () ;
   }

   _rtnBaseJob::~_rtnBaseJob ()
   {
      _latchIn.release () ;
   }

   INT32 _rtnBaseJob::attachIn ( pmdEDUCB * cb )
   {
      _pEDUCB = cb ;
      _latchOut.try_get () ;
      _latchIn.release () ;
      return SDB_OK ;
   }

   INT32 _rtnBaseJob::attachOut ()
   {
      _latchOut.release () ;
      _pEDUCB = NULL ;
      return SDB_OK ;      
   }

   INT32 _rtnBaseJob::waitAttach ()
   {
      _latchIn.get () ;
      return SDB_OK ;
   }

   INT32 _rtnBaseJob::waitDetach ()
   {
      _latchOut.get () ;
      _latchOut.release () ;
      return SDB_OK ;
   }

   pmdEDUCB * _rtnBaseJob::eduCB ()
   {
      return _pEDUCB ;
   }

   ////////////////////////////////////////////////////////////////////////////
   // background job implements //
   ////////////////////////////////////////////////////////////////////////////

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNINDEXJOB__RTNINDEXJOB, "_rtnIndexJob::_rtnIndexJob" )
   _rtnIndexJob::_rtnIndexJob ( RTN_JOB_TYPE type, const CHAR *pCLName,
                                const BSONObj & indexObj, SDB_DPSCB * dpsCB)
   {
      PD_TRACE_ENTRY ( SDB__RTNINDEXJOB__RTNINDEXJOB ) ;
      _type = type ;
      ossMemcpy ( _clFullName, pCLName, DMS_COLLECTION_FULL_NAME_SZ ) ;
      _clFullName[DMS_COLLECTION_FULL_NAME_SZ] = 0 ;
      _indexObj = indexObj.copy() ;
      _dpsCB = dpsCB ;
      _dmsCB = pmdGetKRCB()->getDMSCB() ;
      PD_TRACE_EXIT ( SDB__RTNINDEXJOB__RTNINDEXJOB ) ;
   }

   _rtnIndexJob::~_rtnIndexJob ()
   {
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNINDEXJOB_INIT, "_rtnIndexJob::init ()" )
   INT32 _rtnIndexJob::init ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__RTNINDEXJOB_INIT ) ;
      dmsStorageUnitID suID = DMS_INVALID_SUID ;

      switch ( _type )
      {
         case RTN_JOB_CREATE_INDEX :
            {
               _jobName = "CreateIndex-" ;
               // need to get the index name
               _indexName = _indexObj.getStringField( IXM_NAME_FIELD ) ;
            }
            break ;
         case RTN_JOB_DROP_INDEX :
            {
               _jobName = "DropIndex-" ;
               // need to get the index name
               _indexEle = _indexObj.getField( IXM_NAME_FIELD ) ;
               if ( _indexEle.eoo() )
               {
                  _indexEle = _indexObj.firstElement () ;
               }

               if ( jstOID == _indexEle.type() )
               {
                  OID oid ;
                  const CHAR *pCLShortName = NULL ;
                  dmsStorageUnit *su = NULL ;
                  dmsMBContext *mbContext = NULL ;
                  dmsExtentID idxExtent = DMS_INVALID_EXTENT ;

                  rc = rtnResolveCollectionNameAndLock ( _clFullName, _dmsCB,
                                                         &su, &pCLShortName,
                                                         suID ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG ( PDERROR, "Failed to resolve collection name %s",
                              _clFullName ) ;
                     goto error ;
                  }

                  rc = su->data()->getMBContext( &mbContext, pCLShortName,
                                                 SHARED ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG ( PDERROR, "Lock collection[%s] failed, rc = %d",
                              _clFullName, rc ) ;
                     goto error ;
                  }

                  _indexEle.Val( oid ) ;
                  // get index extent
                  rc = su->index()->getIndexCBExtent( mbContext, oid,
                                                      idxExtent ) ;
                  if ( SDB_OK != rc )
                  {
                     su->data()->releaseMBContext( mbContext ) ;
                     PD_LOG ( PDERROR, "Get collection[%s] indexCB extent "
                              "failed, rc = %d", _clFullName, rc ) ;
                     goto error ;
                  }

                  ixmIndexCB indexCB ( idxExtent, su->index(), NULL ) ;
                  _indexName = indexCB.getName() ;

                  su->data()->releaseMBContext( mbContext ) ;
                  _dmsCB->suUnlock( suID ) ;
                  suID = DMS_INVALID_SUID ;
               }
               else
               {
                  _indexName = _indexEle.str () ;
               }
            }
            break ;
         default :
            _jobName = "UnknowIndexJob" ;
            PD_LOG ( PDERROR, "Index job not support this type[%d]", _type ) ;
            rc = SDB_INVALIDARG ;
            break ;
      }

      if ( SDB_OK == rc )
      {
         _jobName += _clFullName ;
         _jobName += "[" ;
         _jobName += _indexName ;
         _jobName += "]" ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__RTNINDEXJOB_INIT, rc ) ;
      return rc ;
   error:
      if ( DMS_INVALID_SUID != suID )
      {
         _dmsCB->suUnlock( suID ) ;
      }
      goto done ;
   }

   const CHAR* _rtnIndexJob::getIndexName () const
   {
      return _indexName.c_str() ;
   }

   RTN_JOB_TYPE _rtnIndexJob::type () const
   {
      return _type ;
   }

   const CHAR* _rtnIndexJob::name () const
   {
      return _jobName.c_str() ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNINDEXJOB_MUTEXON, "_rtnIndexJob::muteXOn" )
   BOOLEAN _rtnIndexJob::muteXOn ( const _rtnBaseJob * pOther )
   {
      PD_TRACE_ENTRY ( SDB__RTNINDEXJOB_MUTEXON ) ;
      BOOLEAN ret = FALSE;
      if ( RTN_JOB_CREATE_INDEX != pOther->type() &&
           RTN_JOB_DROP_INDEX != pOther->type() )
      {
         ret = FALSE ;
         goto done ;
      }

      {
         _rtnIndexJob *pIndexJob = ( _rtnIndexJob* )pOther ;

         if ( 0 == ossStrcmp ( getIndexName(), pIndexJob->getIndexName() ) )
         {
            ret = TRUE ;
            goto done ;
         }
      }
   done :
      PD_TRACE_EXIT ( SDB__RTNINDEXJOB_MUTEXON ) ;
      return ret ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNINDEXJOB_DOIT , "_rtnIndexJob::doit" )
   INT32 _rtnIndexJob::doit ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__RTNINDEXJOB_DOIT ) ;

      switch ( _type )
      {
         case RTN_JOB_CREATE_INDEX :
            rc = rtnCreateIndexCommand( _clFullName, _indexObj, eduCB(),
                                        _dmsCB, _dpsCB, TRUE ) ;
            break ;
         case RTN_JOB_DROP_INDEX :
            rc = rtnDropIndexCommand( _clFullName, _indexEle, eduCB(),
                                      _dmsCB, _dpsCB, TRUE ) ;
            break ;
         default :
            PD_LOG ( PDERROR, "Index job not support this type[%d]", _type ) ;
            rc = SDB_INVALIDARG ;
            break ;
      }

      PD_TRACE_EXITRC ( SDB__RTNINDEXJOB_DOIT, rc ) ;
      return rc ;
   }

   RTN_JOB_TYPE _rtnLoadJob::type () const
   {
      return RTN_JOB_LOAD ;
   }

   const CHAR* _rtnLoadJob::name () const
   {
      return _jobName.c_str() ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNLOADJOB_MUTEXON, "_rtnLoadJob::muteXOn" )
   BOOLEAN _rtnLoadJob::muteXOn ( const _rtnBaseJob * pOther )
   {
      PD_TRACE_ENTRY ( SDB__RTNLOADJOB_MUTEXON ) ;
      BOOLEAN ret = FALSE;
      PD_TRACE_EXIT ( SDB__RTNLOADJOB_MUTEXON ) ;
      return ret ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__RTNLOADJOB_DOIT , "_rtnLoadJob::doit" )
   INT32 _rtnLoadJob::doit ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__RTNLOADJOB_DOIT ) ;
      dmsStorageUnitID  suID     = DMS_INVALID_CS ;
      dmsStorageUnit   *su       = NULL ;
      pmdKRCB          *krcb     = pmdGetKRCB () ;
      SDB_DMSCB        *dmsCB    = krcb->getDMSCB () ;
      pmdEDUMgr        *eduMgr   = krcb->getEDUMgr () ;
      pmdEDUCB         *eduCB    = eduMgr->getEDU() ;
      dmsStorageLoadOp dmsLoadExtent ;
      std::set<monCollectionSpace> csList ;
      std::set<monCollectionSpace>::iterator it ;

      if ( SDB_ROLE_STANDALONE != krcb->getDBRole() &&
           SDB_ROLE_DATA != krcb->getDBRole() )
      {
         goto done ;
      }

      dmsCB->dumpInfo ( csList ) ;

      for ( it = csList.begin(); it != csList.end(); ++it )
      {
         std::set<monCollection> clList ;
         std::set<monCollection>::iterator itCollection ;
         rc = rtnCollectionSpaceLock ( (*it)._name,
                                       dmsCB,
                                       FALSE,
                                       &su,
                                       suID ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to lock collection space, rc=%d", rc ) ;
            goto error ;
         }
   
         dmsLoadExtent.init ( su ) ;

         su->dumpInfo ( clList ) ;
         for ( itCollection = clList.begin();
               itCollection != clList.end();
               ++itCollection )
         {
            dmsMBContext *mbContext = NULL ;
            UINT16 collectionFlag = 0 ;
            const CHAR *pCLNameTemp = NULL ;
            const CHAR *pCLName = (*itCollection)._name ;

            if ( ( ossStrlen ( pCLName ) > DMS_COLLECTION_FULL_NAME_SZ ) ||
                    ( NULL == ( pCLNameTemp = ossStrrchr ( pCLName, '.' ))) )
            {
               PD_LOG ( PDERROR, "collection name is not valid: %s",
                        pCLName ) ;
               continue ;
            }

            rc = su->data()->getMBContext( &mbContext, pCLNameTemp + 1,
                                           EXCLUSIVE ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Failed to lock collection: %s, rc: %d",
                       pCLName, rc ) ;
               continue ;
            }
            collectionFlag = mbContext->mb()->_flag ;

            // unlock collection

            if ( DMS_IS_MB_FLAG_LOAD_LOAD ( collectionFlag ) )
            {
               PD_LOG ( PDEVENT, "Start Rollback" ) ;
               rc = dmsLoadExtent.loadRollbackPhase ( mbContext ) ;
               if ( rc )
               {
                  su->data()->releaseMBContext( mbContext ) ;
                  PD_LOG ( PDERROR, "Failed to load Rollback Phase, rc=%d", rc ) ;
                  continue ;
               }
               dmsLoadExtent.clearFlagLoadLoad ( mbContext->mb() ) ;
            }
            if ( DMS_IS_MB_FLAG_LOAD_BUILD ( collectionFlag ) )
            {
               PD_LOG ( PDEVENT, "Start loadBuild" ) ;
               rc = dmsLoadExtent.loadBuildPhase ( mbContext,
                                                   eduCB ) ;
               if ( rc )
               {
                  su->data()->releaseMBContext( mbContext ) ;
                  PD_LOG ( PDERROR, "Failed to load build Phase, rc=%d", rc ) ;
                  continue ;
               }
               dmsLoadExtent.clearFlagLoadBuild ( mbContext->mb() ) ;
            }
            if ( DMS_IS_MB_LOAD ( collectionFlag ) )
            {
               PD_LOG ( PDEVENT, "Start clear load flag" ) ;
               dmsLoadExtent.clearFlagLoad ( mbContext->mb() ) ;
            }

            su->data()->releaseMBContext( mbContext ) ;
         }
         dmsCB->suUnlock ( suID ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__RTNLOADJOB_DOIT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

}

