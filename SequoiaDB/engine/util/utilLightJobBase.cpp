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

   Source File Name = utilLightJobBase.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/12/2019  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "utilLightJobBase.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"

namespace engine
{

   /*
      _utilLightJob implement
   */
   INT32 _utilLightJob::submit( BOOLEAN takeOver, INT32 priority )
   {
      INT32 rc = SDB_OK ;
      _utilLightJob *pJob = this ;
      utilLightJobMgr *pMgr = utilGetGlobalJobMgr() ;

      SDB_ASSERT( pMgr, "Global job manager is NULL" ) ;

      if ( !pMgr )
      {
         PD_LOG( PDERROR, "Global job manager is NULL" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      try
      {
         pMgr->push( pJob, takeOver, priority ) ;
         pJob = NULL ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_OOM ;
         goto error ;
      }

   done:
      if ( pJob && takeOver )
      {
         SDB_OSS_DEL pJob ;
      }
      return rc ;
   error:
      goto done ;
   }

   UINT64 _utilLightJob::expectAvgCost() const
   {
      return UTIL_LJOB_DFT_AVG_COST ;
   }

   /*
      _utilLightJobInfo implement
   */
   _utilLightJobInfo::_utilLightJobInfo()
   {
      reset() ;
   }

   _utilLightJobInfo::_utilLightJobInfo( utilLightJob *pJob,
                                         BOOLEAN takeOver,
                                         INT32 priority )
   {
      _pJob = pJob ;
      _takeOver = takeOver ;
      _priority = adjustPriority( 0 - priority ) ;

      _lastDoTime = 0 ;
      _lastCost = 0 ;
      _totalCost = 0 ;
      _totalTimes = 0 ;
   }

   _utilLightJobInfo::~_utilLightJobInfo()
   {
   }

   void _utilLightJobInfo::reset()
   {
      _pJob = NULL ;
      _takeOver = FALSE ;
      _priority = UTIL_LJOB_PRI_MID ;

      _lastDoTime = 0 ;
      _lastCost = 0 ;
      _totalCost = 0 ;
      _totalTimes = 0 ;
   }

   bool _utilLightJobInfo::operator< ( const _utilLightJobInfo &right ) const
   {
      if ( _priority < right._priority )
      {
         return true ;
      }
      return false ;
   }

   void _utilLightJobInfo::upPriority()
   {
      _priority = adjustPriority( _priority + 1 ) ;
   }
   
   void _utilLightJobInfo::downPriority()
   {
      _priority = adjustPriority( _priority - 1 ) ;
   }

   INT32 _utilLightJobInfo::adjustPriority( INT32 priority )
   {
      if ( priority < UTIL_LJOB_PRI_HIGHEST )
      {
         return UTIL_LJOB_PRI_HIGHEST ;
      }
      else if ( priority > UTIL_LJOB_PRI_LOWEST )
      {
         return UTIL_LJOB_PRI_LOWEST ;
      }
      return priority ;
   }

   void _utilLightJobInfo::release()
   {
      if ( _pJob && _takeOver )
      {
         SDB_OSS_DEL _pJob ;
      }
      _pJob = NULL ;
      _takeOver = FALSE ;
   }

   INT32 _utilLightJobInfo::doit( IExecutor *pExe,
                                  UTIL_LJOB_DO_RESULT &result )
   {
      INT32 rc = SDB_OK ;
      result = UTIL_LJOB_DO_FINISH ;

      if ( _pJob )
      {
         _lastDoTime = ossGetCurrentMicroseconds() ;

         rc = _pJob->doit( pExe, result ) ;

         UINT64 eTime = ossGetCurrentMicroseconds() ;

         if ( eTime >= _lastDoTime )
         {
            _lastCost = eTime - _lastDoTime ;
            _totalCost += _lastCost ;
         }
         ++_totalTimes ;
      }

      return rc ;
   }

   FLOAT64 _utilLightJobInfo::avgCost() const
   {
      if ( _totalTimes > 0 && _totalCost > 0 )
      {
         return (FLOAT64)_totalCost / _totalTimes ;
      }
      return 0.0 ;
   }

   UINT64 _utilLightJobInfo::expectAvgCost() const
   {
      if ( _pJob )
      {
         return _pJob->expectAvgCost() ;
      }
      return UTIL_LJOB_MIN_AVG_COST ;
   }

   void _utilLightJobInfo::resetStat()
   {
      _lastDoTime = 0 ;
      _lastCost = 0 ;
      _totalCost = 0 ;
      _totalTimes = 0 ;
   }

   /*
      _utilLightJobMgr define
   */
   _utilLightJobMgr::_utilLightJobMgr()
   {
   }

   _utilLightJobMgr::~_utilLightJobMgr()
   {
      SDB_ASSERT( isEmpty(), "Not empty" ) ;

      utilLightJobInfo job ;
      while( pop( job, 0 ) )
      {
         job.release() ;
      }
   }

   UINT32 _utilLightJobMgr::size()
   {
      return _queue.size() ;
   }

   BOOLEAN _utilLightJobMgr::isEmpty()
   {
      return _queue.empty() ;
   }

   void _utilLightJobMgr::push( utilLightJob *pJob,
                                BOOLEAN takeOver,
                                INT32 priority )
   {
      _queue.push( utilLightJobInfo( pJob, takeOver, priority ) ) ;
   }

   void _utilLightJobMgr::push( const utilLightJobInfo &job )
   {
      _queue.push( job ) ;
   }

   BOOLEAN _utilLightJobMgr::pop( utilLightJobInfo &job, INT64 millisec )
   {
      BOOLEAN ret = FALSE ;

      if ( millisec < 0 )
      {
         _queue.wait_and_pop( job ) ;
         ret = TRUE ;
      }
      else if ( 0 == millisec )
      {
         ret = _queue.try_pop( job ) ;
      }
      else
      {
         ret = _queue.timed_wait_and_pop( job, millisec ) ;
      }

      return ret ;
   }

   /*
      Global var
   */
   static _utilLightJobMgr* g_pJobMgr = NULL ;

   utilLightJobMgr* utilGetGlobalJobMgr()
   {
      return g_pJobMgr ;
   }

   void utilSetGlobalJobMgr( utilLightJobMgr *pJobMgr )
   {
      if ( NULL == g_pJobMgr )
      {
         g_pJobMgr = pJobMgr ;
      }
      else if ( NULL == pJobMgr )
      {
         SDB_ASSERT( g_pJobMgr->size() == 0,
                     "Total size must be 0" ) ;
         g_pJobMgr = pJobMgr ;
      }
      else
      {
         SDB_ASSERT( FALSE, "Job manager is already valid" ) ;
      }
   }

}


