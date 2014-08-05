#ifndef OMAGENTTASK_HPP_
#define OMAGENTTASK_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "ossLatch.hpp"
#include "../bson/bson.h"
#include "omagent.hpp"
#include <map>
#include <vector>
#include <string>

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

      OMA_TASK_STATUS_END         = 10 // nothing should have this status
   } ;

   // omagent task
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

      public:
         virtual OMA_TASK_TYPE taskType () const = 0 ;
         virtual const CHAR*   taskName () const = 0 ;

      protected:
         UINT64                _taskID ;
         OMA_TASK_STATUS       _status ;
   } ;
   typedef _omaTask omaTask ;

   // task manager
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

   // get task manager
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
                     std::vector<BSONObj> data ) ;

         // start job
         INT32 doit() ;

         // respond query of install status
         INT32 getInstallStatus( BSONObj &result ) ;

      private:
         INT32 _installCatalog() ;
         INT32 _installCoord() ;
         INT32 _installData() ;

         std::vector<BSONObj>                           _coord ;
         std::vector<BSONObj>                           _catalog ;
         std::map< std::string, std::vector<BSONObj> >  _mapGroups ;

         InstallJobResult                               _coordResult ;
         InstallJobResult                               _catalogResult ;
         std::map<std::string, InstallJobResult>        _mapGroupsResult ;

         std::string                                    _taskName ;
         OMA_TASK_TYPE                                  _taskType ;

         BOOLEAN                                        _needRollBack ;
   } ;
   typedef _omaInstallDBBusinessTask omaInstallDBBusinessTask ;

}




#endif
