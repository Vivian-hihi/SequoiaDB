#include "ossTypes.h"
#include "omagentTask.hpp"
#include "omagentJob.hpp"
#include "pmdDef.hpp"
namespace engine
{

   // omagent manager
   _omaTaskMgr::_omaTaskMgr ( UINT64 taskID )
   {
      _taskID = taskID ;
   }

   _omaTaskMgr::~_omaTaskMgr ()
   {
      std::map<UINT64, _omaTask*>::iterator it = _taskMap.begin() ;
      while ( it != _taskMap.end() )
      {
         SDB_OSS_DEL it->second ;
         ++it ;
      }
      _taskMap.clear() ;
   }

   UINT64 _omaTaskMgr::getTaskID ()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      ++_taskID ;
      return _taskID ;
   }

   INT32 _omaTaskMgr::addTask ( _omaTask * pTask, UINT64 taskID )
   {
      INT32 rc = SDB_OK ;
      _omaTask *indexTask = NULL ;

      if ( OMA_INVALID_TASKID == taskID )
      {
         taskID = pTask->taskID() ;
      }

      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;

      std::map<UINT64, _omaTask*>::iterator it ;
      it = _taskMap.find( taskID ) ;
      if ( it != _taskMap.end() )
      {
           indexTask = it->second ;
           PD_LOG ( PDWARNING, "Exist task[%lld,%s] mutex with new task[%lld,%s]",
                     indexTask->taskID(), indexTask->taskName(),
                     pTask->taskID(), pTask->taskName() ) ;
            rc = SDB_CLS_MUTEX_TASK_EXIST ;
            goto error ;

      }
      // add to map
      _taskMap[ taskID ] = pTask ;
   done:
      return rc ;
   error:
      SDB_OSS_DEL pTask ;
      goto done ;
   }

   INT32 _omaTaskMgr::removeTask ( UINT64 taskID )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      std::map<UINT64, _omaTask*>::iterator it = _taskMap.find ( taskID ) ;
      if ( it != _taskMap.end() )
      {
         SDB_OSS_DEL it->second ;
         _taskMap.erase ( it ) ;
      }
      return SDB_OK ;
   }

   INT32 _omaTaskMgr::removeTask ( _omaTask * pTask )
   {
      INT32 rc = SDB_OK ;
      rc = removeTask ( pTask->taskID () ) ;
      return rc ;
   }

   _omaTask* _omaTaskMgr::findTask ( UINT64 taskID )
   {
      ossScopedLock lock ( &_taskLatch, SHARED ) ;
      std::map<UINT64, _omaTask*>::iterator it = _taskMap.find ( taskID ) ;
      if ( it != _taskMap.end() )
      {
         return it->second ;
      }
      return NULL ;
   }

   // get omagent task manager
   _omaTaskMgr* getTaskMgr()
   {
      static _omaTaskMgr taskMgr ;
      return &taskMgr ;
   }


   // install database business
   _omaInstallDBBusinessTask::_omaInstallDBBusinessTask( UINT64 taskID )
   : _omaTask( taskID )
   {
      _taskName = "install db business task" ;
      _taskType = OMA_TASK_INSTALL_DB ;
      _needRollBack = FALSE ;
   }

   _omaInstallDBBusinessTask::~_omaInstallDBBusinessTask()
   {
   }

   INT32 _omaInstallDBBusinessTask::init( std::vector<BSONObj> coord,
                                          std::vector<BSONObj> catalog,
                                          std::vector<BSONObj> data )
   {
      INT32 rc = SDB_OK ;
      // init coord
      _coord = coord ;
      // init catalog
      _catalog = catalog ;
      // init data
      std::vector<BSONObj>::iterator it = data.begin() ;
      // let data node sort by group name
      while( it != data.end() )
      {
         const CHAR *name = NULL ;
         std::string key = "" ;
         rc = omaGetStringElement ( *it, OMA_OPTION_DATAGROUPNAME,
                                        &name ) ;
         PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                   "Get field[%s] failed, rc: %d",
                   OMA_OPTION_DATAGROUPNAME, rc ) ;
         key = std::string( name ) ;
         _mapGroups[key].push_back( *it ) ;
         it++ ;
      }
      // init data node result
      {
      std::map<std::string, std::vector<BSONObj> >::iterator iter ;
      iter = _mapGroups.begin() ;
      while ( iter != _mapGroups.end() )
      {
         std::string groupname = iter->first ;
         InstallJobResult jobResult ;
         jobResult._rc = 0 ;
         jobResult._totalNum = (iter->second).size() ;
         jobResult._finishNum = 0 ;
         _mapGroupsResult.insert( std::pair<std::string,
                                  InstallJobResult>( groupname, jobResult ) ) ;
         iter++ ;
      }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::doit()
   {
      INT32 rc = SDB_OK ;
      // create catalog job
      rc = _installCatalog() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start create catalog job, rc = %d", rc ) ;
         goto error ;
      }
/*
      // create coord job
      rc = _installCoord() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start create coord job, rc = %d", rc ) ;
         goto error ;
      }
*/
      // create data node job
      rc = _installData() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start create data node job, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }


   INT32 _omaInstallDBBusinessTask::_installCatalog()
   {
      INT32 rc = SDB_OK ;
      EDUID installCatalogJobID = PMD_INVALID_EDUID ;
      // start create catalog job
      rc = startCreateCatalogJob( _catalog, _catalogResult,
                                  &installCatalogJobID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start create catalog job, rc = %d", rc ) ;
         goto error ;
      }
      while ( rtnGetJobMgr()->findJob ( installCatalogJobID ) )
      {
         ossSleep ( OSS_ONE_SEC ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::_installCoord()
   {
      INT32 rc = SDB_OK ;
      EDUID installCoordJobID = PMD_INVALID_EDUID ;
      // start coord job
      rc = startCreateCoordJob( _coord, _coordResult,
                                &installCoordJobID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start create coord job, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::_installData()
   {
      INT32 rc = SDB_OK ;
      std::map< std::string, std::vector<BSONObj> >::iterator it ;
      it = _mapGroups.begin() ;
      while( it != _mapGroups.end() )
      {
         std::string groupname = it->first ;
         vector<BSONObj> dataInfo = it->second ;
         EDUID installDataJobID = PMD_INVALID_EDUID ;
         InstallJobResult &jobResult = _mapGroupsResult[groupname] ;
         // start data job
         rc = startCreateDataJob( groupname.c_str(), dataInfo, jobResult,
                                  &installDataJobID ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to start create data node job, rc = %d", rc ) ;
            goto error ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }


}
