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

   /*
      _utilLightJobMgr define
   */
   _utilLightJobMgr::_utilLightJobMgr()
   {
   }

   _utilLightJobMgr::~_utilLightJobMgr()
   {
      SDB_ASSERT( isEmpty(), "Not empty" ) ;
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
      _queue.push( priorityJob( pJob, takeOver, priority ) ) ;
   }

   BOOLEAN _utilLightJobMgr::pop( priorityJob &job, INT64 millisec )
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


