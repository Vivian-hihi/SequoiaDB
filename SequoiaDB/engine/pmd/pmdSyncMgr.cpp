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

   Source File Name = pmdSyncMgr.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/14/2016  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/


#include "pmdSyncMgr.hpp"
#include "pmdEnv.hpp"

namespace engine
{

   /*
      Global define
   */
   #define PMD_MIN_SYNC_JOB                  ( 2 )
   #define PMD_SYNC_JOB_TIMEOUT              ( 300 * OSS_ONE_SEC )
   #define PMD_SYNC_JOB_INTERVAL             ( OSS_ONE_SEC )

   /*
      _pmdSyncMgr implement
   */
   _pmdSyncMgr::_pmdSyncMgr()
   {
      _pMainUnit = NULL ;
      _curAgent = 0 ;
      _idleAgent = 0 ;
      _maxSyncJob = PMD_MIN_SYNC_JOB ;
      _syncDeep = FALSE ;
      _startCtrlJob = FALSE ;
      _pLogAccessor = NULL ;
      _completeLSN = ~0 ;
   }

   _pmdSyncMgr::~_pmdSyncMgr()
   {
   }

   void _pmdSyncMgr::setLogAccess( ILogAccessor *pLogAccess )
   {
      _pLogAccessor = pLogAccess ;
      _completeLSN = _pLogAccessor->getCurrentLsn().offset ;
   }

   void _pmdSyncMgr::setMainUnit( IDataSyncBase *pUnit )
   {
      _pMainUnit = pUnit ;
   }

   INT32 _pmdSyncMgr::init( UINT32 maxSyncJob,
                            BOOLEAN syncDeep )
   {
      _maxSyncJob = maxSyncJob ;
      if ( _maxSyncJob < PMD_MIN_SYNC_JOB )
      {
         _maxSyncJob = PMD_MIN_SYNC_JOB ;
      }
      _syncDeep = syncDeep ;

      return SDB_OK ;
   }

   void _pmdSyncMgr::fini()
   {
      SDB_ASSERT( 0 == _unitList.size(), "List size must be 0" ) ;
      SDB_ASSERT( 0 == _curAgent && 0 == _idleAgent,
                  "Agent must be 0" ) ;
   }

   void _pmdSyncMgr::exitJob( BOOLEAN isControl )
   {
      _unitLatch.get() ;

      --_curAgent ;
      --_idleAgent ;
      if ( isControl )
      {
         _startCtrlJob = FALSE ;
      }
      _checkAndStartJob( FALSE ) ;

      _unitLatch.release() ;
   }

   UINT64 _pmdSyncMgr::syncAndGetLastLSN()
   {
      if ( _pLogAccessor )
      {
         _completeLSN = _pLogAccessor->getCurrentLsn().offset ;
      }
      return _completeLSN ;
   }

   void _pmdSyncMgr::registerSync( IDataSyncBase *pSyncUnit )
   {
      ossScopedLock lock( &_unitLatch ) ;

      _unitList.push_back( pSyncUnit ) ;

      _checkAndStartJob( FALSE ) ;
   }

   void _pmdSyncMgr::unregSync( IDataSyncBase *pSyncUnit )
   {
      LIST_UNIT::iterator it ;

      ossScopedLock lock( &_unitLatch ) ;

      it = _unitList.begin() ;
      while( it != _unitList.end() )
      {
         if ( (*it) == pSyncUnit )
         {
            _unitList.erase( it ) ;
            break ;
         }
         ++it ;
      }
   }

   void _pmdSyncMgr::notifyChange()
   {
      _ntyEvent.signalAll() ;
   }

   IDataSyncBase* _pmdSyncMgr::dispatchUnit()
   {
      IDataSyncBase *pUnit = NULL ;
      LIST_UNIT::iterator it ;
      BOOLEAN force = FALSE ;

      ossScopedLock lock( &_unitLatch ) ;

      if ( _unitList.empty() )
      {
         return NULL ;
      }

      it = _unitList.begin() ;
      while( it != _unitList.end() )
      {
         pUnit = *it ;

         if ( pUnit->canSync( force ) )
         {
            _unitList.erase( it ) ;
            /// lock the unit
            pUnit->lock() ;
            /// dec idle agent
            --_idleAgent ;
            break ; 
         }
         else
         {
            pUnit = NULL ;
         }
         ++it ;
      }

      return pUnit ;
   }

   void _pmdSyncMgr::pushBackUnit( IDataSyncBase *pUnit )
   {
      ossScopedLock lock( &_unitLatch ) ;

      if ( !pUnit->isClosed() )
      {
         _unitList.push_back( pUnit ) ;
      }
      /// release the unit
      pUnit->unlock() ;
      /// inc idla agent
      ++_idleAgent ;
   }

   void _pmdSyncMgr::checkLoad()
   {
      LIST_UNIT::iterator it ;
      UINT32 readyNum = 0 ;
      BOOLEAN force = FALSE ;

      ossScopedLock lock( &_unitLatch ) ;

      it = _unitList.begin() ;
      while( it != _unitList.end() )
      {
         IDataSyncBase *pUnit = *it ;
         ++it ;
         if ( pUnit->canSync( force ) )
         {
            ++readyNum ;
         }
      }

      if ( readyNum > 0 )
      {
         /// wake up the agent
         _wakeUpEvent.signalAll() ;

         /// start agent
         while ( _curAgent < PMD_MIN_SYNC_JOB ||
                 ( readyNum / 10 > _idleAgent &&
                   _curAgent < _maxSyncJob ) )
         {
            if ( SDB_OK == pmdStartSyncJob( NULL, this,
                                            PMD_SYNC_JOB_TIMEOUT ) )
            {
               ++_curAgent ;
               ++_idleAgent ;
            }
            else
            {
               break ;
            }
         }
      }

      _ntyEvent.reset() ;
   }

   void _pmdSyncMgr::_checkAndStartJob( BOOLEAN needLock )
   {
      if ( needLock )
      {
         _unitLatch.get() ;
      }
      if ( _unitList.size() > 0 && FALSE == _startCtrlJob &&
           !pmdIsQuitApp() )
      {
         if ( SDB_OK == pmdStartSyncJob( NULL, this, -1 ) )
         {
            ++_curAgent ;
            ++_idleAgent ;
            _startCtrlJob = TRUE ;
         }
      }
      if ( needLock )
      {
         _unitLatch.release() ;
      }
   }

   /*
      _pmdSyncJob implement
   */
   _pmdSyncJob::_pmdSyncJob( pmdSyncMgr *pMgr, INT32 timeout )
   {
      _pMgr = pMgr ;
      _timeout = timeout ;
   }

   _pmdSyncJob::~_pmdSyncJob()
   {
   }

   RTN_JOB_TYPE _pmdSyncJob::type() const
   {
      return PMD_JOB_SYNC ;
   }

   const CHAR* _pmdSyncJob::name() const
   {
      if ( isControlJob() )
      {
         return "DATASYNC-JOB-D" ;
      }
      return "DATASYNC-JOB" ;
   }

   BOOLEAN _pmdSyncJob::isControlJob() const
   {
      return _timeout < 0 ? TRUE : FALSE ;
   }

   BOOLEAN _pmdSyncJob::muteXOn( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _pmdSyncJob::doit()
   {
      pmdEDUMgr *pEDUMgr = eduCB()->getEDUMgr() ;
      IDataSyncBase *pUnit = NULL ;
      UINT32 timeout = 0 ;
      BOOLEAN force = FALSE ;

      pEDUMgr->activateEDU( eduCB() ) ;

      while( !eduCB()->isForced() )
      {
         if ( isControlJob() )
         {
            _pMgr->checkLoad() ;

            if ( _pMgr->getMainUnit() &&
                 _pMgr->getMainUnit()->canSync( force ) )
            {
               _pMgr->getMainUnit()->sync( force, _pMgr->isSyncDeep(),
                                           _pMgr->syncAndGetLastLSN(),
                                           eduCB() ) ;
               eduCB()->incEventCount( 1 ) ;
            }
            else
            {
               _pMgr->getNtyEvent()->wait( PMD_SYNC_JOB_INTERVAL ) ;
            }
         }
         else
         {
            pEDUMgr->waitEDU( eduCB() ) ;

            /// sync unit
            pUnit = _pMgr->dispatchUnit() ;
            if ( pUnit )
            {
               timeout = 0 ;
               _doUnit( pUnit ) ;
               /// push back
               _pMgr->pushBackUnit( pUnit ) ;
            }
            else
            {
               _pMgr->getEvent()->reset() ;
               if ( SDB_TIMEOUT == _pMgr->getEvent()->wait( 5 * OSS_ONE_SEC ) )
               {
                  timeout += ( 5 * OSS_ONE_SEC ) ;
               }
               else
               {
                  timeout += 100 ;
               }

               if ( timeout >= (UINT32)_timeout )
               {
                  /// over _timeout millsecs, donothing, qiut the job
                  break ;
               }
            }
         }
      }

      _pMgr->exitJob( isControlJob() ) ;

      return SDB_OK ;
   }

   void _pmdSyncJob::_doUnit( IDataSyncBase *pUnit )
   {
      pmdEDUMgr *pEDUMgr = eduCB()->getEDUMgr() ;
      BOOLEAN force = FALSE ;

      pEDUMgr->activateEDU( eduCB() ) ;

      if ( pUnit->canSync( force ) )
      {
         pUnit->sync( force, _pMgr->isSyncDeep(),
                      _pMgr->getCompleteLSN(), eduCB() ) ;

         eduCB()->incEventCount( 1 ) ;
      }

      pEDUMgr->waitEDU( eduCB() ) ;
   }

   /*
      Local Function
   */
   INT32 pmdStartSyncJob( EDUID *pEDUID, pmdSyncMgr *pMgr, INT32 timeout )
   {
      INT32 rc = SDB_OK ;
      pmdSyncJob *pJob = NULL ;

      pJob = SDB_OSS_NEW pmdSyncJob( pMgr, timeout ) ;
      if ( !pJob )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Alloc cache job failed" ) ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID  ) ;
      /// neither failed or succeed, the pJob will release in job manager

   done:
      return rc ;
   error:
      goto done ;
   }

}


