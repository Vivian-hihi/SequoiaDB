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

   Source File Name = utilLightJobBase.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/12/2019  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef UTIL_LIGHT_JOB_BASE_HPP__
#define UTIL_LIGHT_JOB_BASE_HPP__

#include "sdbInterface.hpp"
#include "utilPooledObject.hpp"
#include "ossPriorityQueue.hpp"

namespace engine
{

   /*
      UTIL_LJOB_PRIORITY define
   */
   #define UTIL_LJOB_PRI_HIGHEST          ( -20 )
   #define UTIL_LJOB_PRI_HIGH             ( -10 )
   #define UTIL_LJOB_PRI_MID              ( 0 )
   #define UTIL_LJOB_PRI_LOW              ( 10 )
   #define UTIL_LJOB_PRI_LOWEST           ( 20 )

   /*
      _utilLightJob define
   */
   class _utilLightJob : public _utilPooledObject
   {
      public:
         enum DO_RESULT
         {
            DO_FINISH,
            DO_CONT
         } ;

         _utilLightJob() {}
         virtual ~_utilLightJob() {}

         INT32       submit( BOOLEAN takeOver = TRUE,
                             INT32 priority = UTIL_LJOB_PRI_MID ) ;

      public:
         virtual const CHAR*     name() const = 0 ;
         virtual INT32           doit( IExecutor *pExe,
                                       DO_RESULT &result ) = 0 ;

   } ;
   typedef _utilLightJob utilLightJob ;

   /*
      _utilLightJobMgr define
   */
   class _utilLightJobMgr : public SDBObject
   {
      struct _priorityJob
      {
         public:
            utilLightJob   *_pJob ;

         private:
            INT32          _priority ;
            BOOLEAN        _takeOver ;

         public:
            _priorityJob( utilLightJob *pJob,
                          BOOLEAN takeOver = TRUE,
                          INT32 priority = UTIL_LJOB_PRI_MID )
            {
               _pJob = pJob ;
               _takeOver = takeOver ;
               _priority = adjustPriority( 0 - priority ) ;
            }
            _priorityJob()
            {
               _pJob = NULL ;
               _takeOver = FALSE ;
               _priority = UTIL_LJOB_PRI_MID ;
            }

            bool operator< ( const _priorityJob &right ) const
            {
               if ( _priority < right._priority )
               {
                  return true ;
               }
               return false ;
            }

            void upPriority()
            {
               _priority = adjustPriority( _priority + 1 ) ;
            }

            void downPriority()
            {
               _priority = adjustPriority( _priority - 1 ) ;
            }

            INT32 adjustPriority( INT32 priority )
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
      } ;
      typedef _priorityJob priorityJob ;

      public:
         _utilLightJobMgr() ;
         ~_utilLightJobMgr() ;

      public:

         UINT32      size() ;
         BOOLEAN     isEmpty() ;

         void        push( utilLightJob *pJob,
                           BOOLEAN takeOver = TRUE,
                           INT32 priority = UTIL_LJOB_PRI_MID ) ;

         BOOLEAN     pop( priorityJob &job, INT64 millisec ) ;

      private:
         ossPriorityQueue<priorityJob>          _queue ;

   } ;
   typedef _utilLightJobMgr utilLightJobMgr ;

   /*
      Global functions
   */
   utilLightJobMgr* utilGetGlobalJobMgr() ;
   void utilSetGlobalJobMgr( utilLightJobMgr *pJobMgr ) ;

}

#endif //UTIL_LIGHT_JOB_BASE_HPP__

