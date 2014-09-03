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

   Source File Name = omagentTask.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/30/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENTTASK_HPP_
#define OMAGENTTASK_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "ossLatch.hpp"
#include "../bson/bson.h"
#include "omagent.hpp"
//#include "omagentJob.hpp"
#include <map>
#include <vector>
#include <string>

using namespace std ;
using namespace bson ;

#define OMA_INVALID_TASKID     (0)

namespace engine
{
   enum OMA_TASK_TYPE
   {
      OMA_TASK_INSTALL_DB         = 0, // install db business

      OMA_TASK_UNKNOW             = 255
   } ;

   enum OMA_TASK_STATUS
   {
      OMA_TASK_STATUS_READY       = 0, // when initially created
      OMA_TASK_STATUS_RUN         = 1, // when starts running
      OMA_TASK_STATUS_FINISH      = 2, // when finish doing something
      OMA_TASK_STATUS_FAIL        = 3, // when error happen

      OMA_TASK_STATUS_END         = 10 // nothing should have this status
   } ;

   /*
      omagent task
   */
   class _omaTask : public SDBObject
   {
      public:
         _omaTask ( UINT64 taskID ) : _taskID ( taskID )
         {
            _status = OMA_TASK_STATUS_READY ;
         }
         virtual ~_omaTask () {}

         UINT64          taskID () const { return _taskID ; }

         OMA_TASK_STATUS status () const { return _status ; }

         void setStatus( OMA_TASK_STATUS status ) { _status = status ; }

         INT32 setJobStatus( string &name, OMA_JOB_STATUS status ) ;

         OMA_JOB_STATUS getJobStatus( string &name ) ;

      public:
         virtual OMA_TASK_TYPE taskType () const = 0 ;

         virtual const CHAR*   taskName () const = 0 ;

      protected:
         UINT64                               _taskID ;
         OMA_TASK_STATUS                      _status ;
         map< string, OMA_JOB_STATUS >        _jobStatus ;
   } ;
   typedef _omaTask omaTask ;

   /*
      task manager
   */
   class _omaTaskMgr : public SDBObject
   {
      public:
         _omaTaskMgr ( UINT64 taskID = OMA_INVALID_TASKID ) ;
         ~_omaTaskMgr () ;

         UINT64     getTaskID () ;
         void       setTaskID ( UINT64 taskID ) ;

      public:
         INT32      addTask ( _omaTask *pTask,
                              UINT64 taskID = OMA_INVALID_TASKID ) ;
         INT32      removeTask ( _omaTask *pTask ) ;
         INT32      removeTask ( UINT64 taskID ) ;
         _omaTask*  findTask ( UINT64 taskID ) ;

      private:
         std::map<UINT64, _omaTask*>         _taskMap ;
         ossSpinSLatch                       _taskLatch ;
         UINT64                              _taskID ;
   } ;
   typedef _omaTaskMgr omaTaskMgr ;

   /*
      get task manager
   */
   _omaTaskMgr* getTaskMgr() ;

   // install database business
   class _omaInstallDBBusinessTask : public _omaTask
   {
      public:
         _omaInstallDBBusinessTask ( UINT64 taskID ) ;
         virtual ~_omaInstallDBBusinessTask () ;

      public:
         virtual OMA_TASK_TYPE taskType () const
         {
            return _taskType ;
         }
         virtual const CHAR*   taskName () const
         {
            return _taskName.c_str() ;
         }

      public:
         INT32 init( std::vector<BSONObj> coord,
                     std::vector<BSONObj> catalog,
                     std::vector<BSONObj> data,
                     const CHAR *localHostName,
                     const CHAR *omaSvcName,
                     const CHAR *vCoordSvcName ) ;

         // start job
         INT32 doit() ;

         // respond query of install status
         INT32 getInstallStatus( BSONObj &progress ) ;

      public:
         vector<BSONObj>& getInstallCatalogInfo() ;

         vector<BSONObj>& getInstallCoordInfo() ;

         INT32 getInstallDataGroupInfo( string &name,
                                      vector<BSONObj> &dataGroupInstallInfo ) ;
         INT32 updateInstallResult( INT32 retRc,
                                    const CHAR *pRole,
                                    const CHAR *pErrMsg,
                                    const CHAR *pDesc,
                                    const CHAR *pGroupName,
                                    BOOLEAN isFinish,
                                    InstalledNode *pNode ) ;
         BOOLEAN isFinish() ;

         void setStage ( const CHAR *stage ) { _stage = stage ; }
      
      private:
         INT32 _installCatalog() ;
         INT32 _installCoord() ;
         INT32 _installData() ;
         INT32 _removeVirtualCoord() ;

         // install info
         vector<BSONObj>                                _catalog ;
         vector<BSONObj>                                _coord ;
         map< string, vector<BSONObj> >                 _mapGroups ;
         
         // install result
         InstallResult                                  _catalogResult ;
         InstallResult                                  _coordResult ;
         map<string, InstallResult>                     _mapGroupsResult ;

         // reomve virtual coord
         string                                         _localHostName ;
         string                                         _omaSvcName ;
         string                                         _vCoordSvcName ;
         string                                         _taskName ;

         ossSpinSLatch                                  _jobLatch ;
         OMA_TASK_TYPE                                  _taskType ;
         string                                         _stage ;
         BOOLEAN                                        _needRollBack ;

   } ;
   typedef _omaInstallDBBusinessTask omaInstallDBBusinessTask ;

}




#endif
