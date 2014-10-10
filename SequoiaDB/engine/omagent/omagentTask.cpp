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

   Source File Name = omagentTask.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#include "ossTypes.h"
#include "omagentUtil.hpp"
#include "omagentTask.hpp"
#include "omagentJob.hpp"
#include "pmdDef.hpp"

namespace engine
{

   /*
      omagent task
   */
   OMA_TASK_STATUS _omaTask::status ()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ; 
      return _status ;
   }
 
   void _omaTask::setStatus( OMA_TASK_STATUS status )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;   
      _status = status ;
   }

   INT32 _omaTask::setJobStatus( string &name, OMA_JOB_STATUS status )
   {
      INT32 rc = SDB_OK ;
      ossScopedLock lock ( &_jobLatch, EXCLUSIVE ) ;

      if ( name.empty() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Invalid job name" ) ;
         goto error ;
      }
      PD_LOG ( PDDEBUG, "Job[%s] set status[%d]",
               name.c_str(), status ) ;
      _jobStatus[name] = status ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaTask::getJobStatus( string &name, OMA_JOB_STATUS &status )
   {
      INT32 rc =SDB_OK ;
      map< string, OMA_JOB_STATUS >::iterator it ;

      it = _jobStatus.find( name ) ;
      if ( _jobStatus.end() != it )
      {
         status = it->second ;
      }
      else
      {
         PD_LOG ( PDERROR, "Failed to get job status, no such job named %s",
                  name.c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      omagent manager
   */
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
      UINT64 id = OMA_INVALID_TASKID ;
      std::map<UINT64, _omaTask*>::iterator it ;

      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      while ( TRUE )
      {
         id = ++_taskID ;
         it = _taskMap.find( id ) ;
         if ( it == _taskMap.end() )
         {
            break ;
         }
      }
      
      return id ;
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

   INT32 _omaTaskMgr::removeTask ( const CHAR *pTaskName )
   {
      INT32 rc = SDB_OK ;
      std::map<UINT64, _omaTask*>::iterator it = _taskMap.begin() ;
      PD_LOG( PDDEBUG, "There are [%d] task kept in task manager, "
              "the removing task is[%s]", _taskMap.size(), pTaskName ) ;
      for ( ; it != _taskMap.end(); it++ )
      {
         _omaTask *pTask = it->second ;
         const CHAR *name = pTask->taskName() ;
         PD_LOG ( PDDEBUG, "The task is [%s]", name ) ;
         if ( 0 == ossStrncmp( name, pTaskName, ossStrlen(pTaskName) ) )
         {
            rc = removeTask( pTask ) ;
            // when remove old task, must stop iterate
            break ;
         }
      }
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


   /*
      install database business
   */
   _omaInstallDBBusinessTask::_omaInstallDBBusinessTask( UINT64 taskID )
   : _omaTask( taskID )
   {
      _taskType             = OMA_TASK_INSTALL_DB ;
      _taskName             = OMA_TASK_NAME_INSTALL_DB_BUSINESS ;
      _stage                = OMA_INSTALL_INSTALL ;
      _isInstallFinish      = FALSE ;
      _isRollbackFinish     = FALSE ;
      _isRemoveVCoordFinish = FALSE ;
      _isTaskFinish         = FALSE ;
      _isInstallFail        = FALSE ;
      _isRollbackFail       = FALSE ;
      _isRemoveVCoordFail   = FALSE ;
      _isTaskFail           = FALSE ;
      _vCoordSvcName        = "" ;
      ossMemset( _detail, 0, OMA_BUFF_SIZE + 1 ) ;
   }

   _omaInstallDBBusinessTask::~_omaInstallDBBusinessTask()
   {
   }

   INT32 _omaInstallDBBusinessTask::init( BOOLEAN isStandalone,
                                          vector<BSONObj> standalone,
                                          vector<BSONObj> coord,
                                          vector<BSONObj> catalog,
                                          vector<BSONObj> data,
                                          BSONObj &other )
   {
      INT32 rc = SDB_OK ;
      vector<BSONObj>::iterator it ;
      map<string, vector<BSONObj> >::iterator iter ;
      _isStandalone = isStandalone ;
      // in case of standalone
      if ( isStandalone )
      {
         // init _standalone and _standaloneResult
         _standalone = standalone ;
         _standaloneResult._rc = SDB_OK ;
         _standaloneResult._totalNum = _standalone.size() ;
         _standaloneResult._finishNum = 0 ;
      }
      else // in case of cluster
      {
         // init _coord and _coordResult
         _coord = coord ;
         _coordResult._rc = SDB_OK ;
         _coordResult._totalNum = _coord.size() ;
         _coordResult._finishNum = 0 ;
         // init _catalog and _catalogResult
         _catalog = catalog ;
         _catalogResult._rc = SDB_OK ;
         _catalogResult._totalNum = _catalog.size() ;
         _catalogResult._finishNum = 0 ;
         // init _mapGroups and _mapGroupsResult
         it = data.begin() ;
         // let data node sort by group name
         while( it != data.end() )
         {
            const CHAR *name = NULL ;
            string key = "" ;
            rc = omaGetStringElement ( *it, OMA_OPTION_DATAGROUPNAME, &name ) ;
            PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                      "Get field[%s] failed, rc: %d",
                      OMA_OPTION_DATAGROUPNAME, rc ) ;
            key = string( name ) ;
            _mapGroups[key].push_back( *it ) ;
            it++ ;
         }
         // init data node result
         iter = _mapGroups.begin() ;
         while ( iter != _mapGroups.end() )
         {
            string groupname = iter->first ;
            InstallResult jobResult ;
            jobResult._rc = 0 ;
            jobResult._totalNum = (iter->second).size() ;
            jobResult._finishNum = 0 ;
            _mapGroupsResult.insert( std::pair<string,
                                     InstallResult>( groupname, jobResult ) ) ;
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
      // in case of standalone
      if ( _isStandalone )
      {
         rc = _installStandalone() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to install standalone, rc = %d", rc ) ;
            goto error ;
         }
      }
      else // in case of cluster
      { 
         // create virtual catalog
         rc = _installVirtualCatalog() ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to create virtual catalog, rc = %d", rc ) ;
            goto error ;
         }
         // create catalog job
         rc = _installCatalog() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to start create catalog job, rc = %d", rc ) ;
            goto error ;
         }
         // create coord job
         rc = _installCoord() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to start create coord job, rc = %d", rc ) ;
            goto error ;
         }
         // create data node job
         rc = _installData() ;
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

   void _omaInstallDBBusinessTask::setTaskStage( OMA_INSTALL_DB_STAGE stage )
   {
      _stage = stage ;
   }

   void _omaInstallDBBusinessTask::setIsInstallFinish( BOOLEAN isFinish )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      _isInstallFinish = isFinish ;
   }   

   void _omaInstallDBBusinessTask::setIsRollbackFinish( BOOLEAN isFinish )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      _isRollbackFinish = isFinish ;
   } 

   void _omaInstallDBBusinessTask::setIsRemoveVCoordFinish( BOOLEAN isFinish )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      _isRemoveVCoordFinish = isFinish ;
   }

   void _omaInstallDBBusinessTask::setIsTaskFinish( BOOLEAN isFinish )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      _isTaskFinish = isFinish ;
   }

   void _omaInstallDBBusinessTask::setIsInstallFail( BOOLEAN isFail )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      _isInstallFail = isFail ;
   }

   void _omaInstallDBBusinessTask::setIsRollbackFail( BOOLEAN isFail )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      _isRollbackFail = isFail ;
   }   

   void _omaInstallDBBusinessTask::setIsRemoveVCoordFail( BOOLEAN isFail )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      _isRemoveVCoordFail = isFail ;
   }   

   void _omaInstallDBBusinessTask::setIsTaskFail( BOOLEAN isFail )
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      _isTaskFail = isFail ;
   }   

   BOOLEAN _omaInstallDBBusinessTask::getIsInstallFinish()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      return _isInstallFinish ;
   }

   BOOLEAN _omaInstallDBBusinessTask::getIsRollbackFinish()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      return _isRollbackFinish ;
   }

   BOOLEAN _omaInstallDBBusinessTask::getIsRemoveVCoordFinish()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      return _isRemoveVCoordFinish ;
   }

   BOOLEAN _omaInstallDBBusinessTask::getIsTaskFinish()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      return _isTaskFinish ;
   }

   BOOLEAN _omaInstallDBBusinessTask::getIsInstallFail()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      return _isInstallFail ;
   }

   BOOLEAN _omaInstallDBBusinessTask::getIsRollbackFail()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      return _isRollbackFail ;
   }

   BOOLEAN _omaInstallDBBusinessTask::getIsRemoveVCoordFail()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      return _isRemoveVCoordFail ;
   }
 
   BOOLEAN _omaInstallDBBusinessTask::getIsTaskFail()
   {
      ossScopedLock lock ( &_taskLatch, EXCLUSIVE ) ;
      return _isTaskFail ;
   }

   void _omaInstallDBBusinessTask::setErrDetail( const CHAR *pErrDetail )
   {
      ossSnprintf( _detail, OMA_BUFF_SIZE, pErrDetail ) ;
   }

   vector<BSONObj>& _omaInstallDBBusinessTask::getInstallStandaloneInfo()
   {
      return _standalone;
   }

   vector<BSONObj>& _omaInstallDBBusinessTask::getInstallCatalogInfo()
   {
      return _catalog ;
   }

   vector<BSONObj>& _omaInstallDBBusinessTask::getInstallCoordInfo()
   {
      return _coord ;
   }

   INT32 _omaInstallDBBusinessTask::getInstallDataGroupInfo( string &name,
                                        vector<BSONObj> &dataGroupInstallInfo )
   {
      INT32 rc  = SDB_OK ;
      map< string, vector<BSONObj> >::iterator it ;

      it = _mapGroups.find( name ) ;
      if ( it != _mapGroups.end() )
      {
         dataGroupInstallInfo = it->second ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "No group[%s] install info", name.c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::updateInstallStatus( BOOLEAN isFinish,
                                                         INT32 retRc,
                                                         const CHAR *pRole,
                                                         const CHAR *pErrMsg,
                                                         const CHAR *pDesc,
                                                         const CHAR *pGroupName,
                                                         InstalledNode *pNode )
   {
      INT32 rc = SDB_OK ;
      ossScopedLock lock ( &_jobLatch, EXCLUSIVE ) ;

      // check argument
      if ( NULL == pRole )
      {
         PD_LOG ( PDERROR,
                  "Not speciefy role for updating install result" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      if ( ( 0 == ossStrncmp( pRole, ROLE_DATA, ossStrlen( ROLE_DATA ) ) ) &&
           ( NULL == pGroupName ) )
      {
         PD_LOG ( PDERROR,
                  "Not speciefy data group for updating install result" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      if ( ( TRUE == isFinish ) && ( NULL == pNode ) )
      {
         PD_LOG ( PDERROR, "The info of finish installed node "
                  "is empty for register" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      if ( NULL == pErrMsg ) pErrMsg = "" ;
      if ( NULL == pDesc ) pDesc = "" ;
      if ( NULL == pGroupName ) pGroupName = "" ;

      // update the install result
      if ( 0 == ossStrncmp( pRole, ROLE_DATA, ossStrlen( ROLE_DATA ) ) )
      {
         map<string, InstallResult>::iterator it ; 
         string groupname = pGroupName ;
         it = _mapGroupsResult.find( groupname ) ;
         if ( it != _mapGroupsResult.end() )
         {
            InstallResult &result = it->second ;
            result._desc = pDesc ;
            if ( retRc )
            {
               result._rc = retRc ;
               result._errMsg = pErrMsg ;
               goto done ;
            }
            if ( isFinish )
            {
               result._finishNum++ ;
               result._installedNodes.push_back( *pNode ) ;
            }
         }
      }
      else if ( 0 == ossStrncmp( pRole, ROLE_COORD,
                                 ossStrlen( ROLE_COORD ) ) )
      {
         _coordResult._desc = pDesc ;
         if ( retRc )
         {
            _coordResult._rc = retRc ;
            _coordResult._errMsg = pErrMsg ;
            goto done ;
         }
         if ( isFinish )
         {
            _coordResult._finishNum++ ;
            _coordResult._installedNodes.push_back( *pNode ) ;
         }
      }
      else if ( 0 == ossStrncmp( pRole, ROLE_CATA,
                                 ossStrlen( ROLE_CATA ) ) )
      {
         _catalogResult._desc = pDesc ;
         if ( retRc )
         {
            _catalogResult._rc = retRc ;
            _catalogResult._errMsg = pErrMsg ;
            goto done ;
         }
         if ( isFinish )
         {
            _catalogResult._finishNum++ ;
            _catalogResult._installedNodes.push_back( *pNode ) ;
         }
      }
      else if ( 0 == ossStrncmp( pRole, ROLE_STANDALONE,
                                 ossStrlen( ROLE_STANDALONE) ) )
      {
         _standaloneResult._desc = pDesc ;
         if ( retRc )
         {
            _standaloneResult._rc = retRc ;
            _standaloneResult._errMsg = pErrMsg ;
            // though this node failed to be created, it's info
            // had been registed in remote sdbcm, we need to remove
            // those info, so we need to keep this node's install info
            // if it's offered
            if ( NULL != pNode )
            {
               _standaloneResult._installedNodes.push_back( *pNode ) ;
            }
            goto done ;
         }
         if ( isFinish )
         {
            _standaloneResult._finishNum++ ;
         }
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR,
                  "Failed to update install result, rc = %d", rc ) ;
         goto error ;
      }
      // check whether it's time to set task's status or 
      // remove virtual coord
      if ( isInstallFinish() )
      {
         setIsInstallFinish( TRUE ) ;
         if ( _isStandalone )
         {
            setIsTaskFinish( TRUE ) ;
            goto done ;
         }
         else
         {
            // start an async job to remove virtual coord
            rc = removeVirtualCoord() ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed remove virtual coord, rc = %d", rc ) ;
               goto error ;
            }
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::getInstalledNodeResult ( const CHAR *pRole,
                                   map< string, vector<InstalledNode> >& info )
   {
      INT32 rc = SDB_OK ;
      
      if ( 0 == ossStrncmp( ROLE_DATA, pRole, ossStrlen(ROLE_DATA) ) )
      {
         map< string, InstallResult >::iterator it = _mapGroupsResult.begin() ;
         for( ; it != _mapGroupsResult.end(); it++ )
         {
            vector< InstalledNode > &nodes = (it->second)._installedNodes ;
            info.insert (
               pair< string, vector<InstalledNode> >( string(it->first), nodes )
            ) ;
         }
      }
      else if ( 0 == ossStrncmp( ROLE_CATA, pRole, ossStrlen(ROLE_CATA) ) )
      {
         vector< InstalledNode > &nodes = _catalogResult._installedNodes ;
         info.insert (
            pair< string, vector<InstalledNode> >( string(ROLE_CATA), nodes )
         ) ;
      }
      else if ( 0 == ossStrncmp( ROLE_COORD, pRole, ossStrlen(ROLE_COORD) ) )
      {
         vector< InstalledNode > &nodes = _coordResult._installedNodes ;
         info.insert (
            pair< string, vector<InstalledNode> >( string(ROLE_COORD), nodes )
         ) ;
      }
      else if ( 0 == ossStrncmp( ROLE_STANDALONE, pRole,
                                 ossStrlen(ROLE_STANDALONE) ) )
      {
         vector< InstalledNode > &nodes = _standaloneResult._installedNodes ;
         info.insert (
         pair< string, vector<InstalledNode> >( string(ROLE_STANDALONE), nodes )
         ) ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Invalid role for get installed node result" ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _omaInstallDBBusinessTask::isInstallFinish ()
   {
      // TODO: need to add a lock different with update install statue?

      // in case of standalone
      if ( _isStandalone )
      {
         if ( _standaloneResult._totalNum == _standaloneResult._finishNum )
         {
            return TRUE ;
         }
         else
         {
            return FALSE ;
         }
      }
      else // in case of cluster
      {
         map<string, InstallResult>::iterator it ;
         if ( _catalogResult._totalNum > _catalogResult._finishNum )
         {
            return FALSE ;
         }
         if ( _coordResult._totalNum > _coordResult._finishNum )
         {
            return FALSE ;
         }
         it = _mapGroupsResult.begin() ;
         while( it != _mapGroupsResult.end() )
         {
            InstallResult &result = it->second ;
            if ( result._totalNum > result._finishNum )
            {
               return FALSE ;
            }
            it++ ;
         }
         return TRUE ;
      }
   }

   INT32 _omaInstallDBBusinessTask::getInstallStatus ( BSONObj &progress )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bob ;
      BSONArrayBuilder bab ;
      BSONObj standaloneResult ;
      BSONObj coordResult ;
      BSONObj catalogResult ;
      const CHAR *pStage = NULL ;
      
      // while task has failed
      if ( _isTaskFail )
      {
         PD_LOG_MSG ( PDERROR,"Task[%s] had failed, please check "
                      "the dialog for more detail", taskName() ) ;
         rc = SDB_OMA_TASK_FAIL ;
         goto done ;
      }
      // test which stage task is in
      if ( OMA_INSTALL_INSTALL == _stage )
      {
         pStage = STAGE_INSTALL ;
      }
      else if ( OMA_INSTALL_ROLLBACK == _stage )
      {
         pStage = STAGE_ROLLBACK ;
      }
      else
      {
         PD_LOG ( PDERROR, "Invalid task's stage" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      try
      {
         // taskID
         bob.append( OMA_FIELD_TASKID, (SINT64)_taskID ) ;
         // isFinish
         bob.appendBool( OMA_FIELD_ISFINISH, _isTaskFinish ) ;
         // status
         bob.append( OMA_FIELD_STATUS, pStage ) ;
         // ErrMsg( fatal err, like failed to rollback or
         // failed to remove virtual coord)
         bob.append( OMA_FIELD_ERRMSG, _detail ) ;
         // in case of standalone
         if ( _isStandalone )
         {
            // get standalone status
            standaloneResult = BSON ( OMA_FIELD_NAME
                                      << OMA_FIELD_STANDALONE
                                      << OMA_FIELD_TOTALCOUNT
                                      << _standaloneResult._totalNum
                                      << OMA_FIELD_INSTALLEDCOUNT
                                      << _standaloneResult._finishNum
                                      << OMA_FIELD_DESC
                                      << _standaloneResult._desc.c_str() ) ;
            bab.append ( standaloneResult ) ;
         }
         else // in case of cluster
         {
            // get coord status
            coordResult = BSON ( OMA_FIELD_NAME
                                 << OMA_FIELD_COORD
                                 << OMA_FIELD_TOTALCOUNT
                                 << _coordResult._totalNum
                                 << OMA_FIELD_INSTALLEDCOUNT
                                 << _coordResult._finishNum
                                 << OMA_FIELD_DESC
                                 << _coordResult._desc.c_str() ) ;
            bab.append ( coordResult ) ;
            // get catalog status
            catalogResult = BSON ( OMA_FIELD_NAME
                                   << OMA_FIELD_CATALOG
                                   << OMA_FIELD_TOTALCOUNT
                                   << _catalogResult._totalNum
                                   << OMA_FIELD_INSTALLEDCOUNT
                                   << _catalogResult._finishNum
                                   << OMA_FIELD_DESC
                                   << _catalogResult._desc.c_str() ) ;
            bab.append ( catalogResult ) ;
            // get data group status
            std::map< string, InstallResult >::iterator it ;
            it = _mapGroupsResult.begin() ;
            while ( it != _mapGroupsResult.end() )
            {
               string groupname = it->first ;
               InstallResult &result = it->second ;
               BSONObj groupResult ;
               groupResult = BSON ( OMA_FIELD_NAME
                                    << groupname.c_str()
                                    << OMA_FIELD_TOTALCOUNT
                                    << result._totalNum
                                    << OMA_FIELD_INSTALLEDCOUNT
                                    << result._finishNum
                                    << OMA_FIELD_DESC
                                    << result._desc.c_str() ) ;
               bab.append ( groupResult ) ;
               it++ ;
            }
         }
         // set return result
         bob.appendArray( OMA_FIELD_PROGRESS, bab.arr() ) ;
         progress = bob.obj() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR,
                  "Failed to get install db business progress: %s",
                  e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::updateInstallJobStatus( string &name,
                                                         OMA_JOB_STATUS status )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN needRollback = FALSE ;
      map< string, OMA_JOB_STATUS >::iterator it ;
      ossScopedLock lock ( &_jobLatch, EXCLUSIVE ) ;
      // set job status
      rc = setJobStatus( name, status ) ;
      if ( rc )
      {
         PD_LOG ( PDWARNING, "Failed to set job[%s] status, rc = %d",
                  name.c_str(), rc ) ;
      }
      if ( OMA_JOB_STATUS_FAIL == status )
      {
         setIsInstallFail( TRUE ) ;
      }
      else
      {
         goto done ;
      }
      // check whether is there any job failed or not,
      // and whether it's the time to rollback
      for ( it = _jobStatus.begin(); it != _jobStatus.end(); it++ )
      {
         PD_LOG ( PDDEBUG, "Job[%s]'s status is : %d",
                  it->first.c_str(), it->second ) ;
         if( OMA_JOB_STATUS_RUNNING == it->second )
         {
            // some job is still running, can't rollback
            PD_LOG ( PDDEBUG, "Some jobs are still running "
                     "in task[%s]", _taskName.c_str() ) ;
            goto done ;
         }
         else if ( OMA_JOB_STATUS_FAIL == it->second )
         {
            PD_LOG ( PDWARNING, "Some jobs are failing, need to rollback" ) ;
            needRollback = TRUE ;
         }
      }
      if ( TRUE == needRollback )
      {
         PD_LOG ( PDWARNING, "Start to rollback.." ) ;
         // start a async job to rollback task
         rc = rollbackInternal() ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to rollback in add db business task, "
                    "rc = %d", rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::rollbackInternal()
   {
      INT32 rc = SDB_OK ;
      EDUID jobID = PMD_INVALID_EDUID ;
      // set task stage
      setTaskStage ( OMA_INSTALL_ROLLBACK ) ;
      rc = startInsDBBusTaskRbJob ( _isStandalone, _vCoordSvcName,
                                    this, &jobID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start to roolback in add db business task "
                 "rc = %d", rc ) ;
         goto error ;
      }
      // wait until rollback is finish
      while ( rtnGetJobMgr()->findJob( jobID ) )
      {
         ossSleep ( OSS_ONE_SEC ) ;
      }
   done:
      return rc ;
   error:
      setIsRollbackFail( TRUE ) ;
      setErrDetail( "Failed to rollback in add "
                    "db business task, please do it manually" ) ;
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::_saveVCoordInfo( BSONObj &info )
   {
      INT32 rc                    = SDB_OK ;
      const CHAR *pVCoordSvcName  = NULL ;
      rc = omaGetStringElement( info, OMA_FIELD_VCOORDSVCNAME, &pVCoordSvcName ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get filed[%s], rc = %s",
                  OMA_FIELD_VCOORDSVCNAME, rc ) ;
         goto error ;
      }
      _vCoordSvcName = pVCoordSvcName ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::_installVirtualCatalog()
   {
      INT32 rc = SDB_OK ;
      BSONObj vCoordRet ;
      _omaCreateVirtualCoord createVCoord ;
      
      // create virtual coord and save it's info for future
      rc = createVCoord.createVirtualCoord( vCoordRet ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to create virtual coord, rc = %d", rc ) ;
         goto error ;
      }
      rc = _saveVCoordInfo( vCoordRet ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to save virtual coord install result, "
                  "rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::_installStandalone()
   {
      INT32 rc = SDB_OK ;
      EDUID createStandaloneJobID = PMD_INVALID_EDUID ;
      // start create standalone job
      rc = startCreateStandaloneJob( this, &createStandaloneJobID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start create standalone job, rc = %d", rc ) ;
         goto error ;
      }
      while ( rtnGetJobMgr()->findJob ( createStandaloneJobID ) )
      {
         ossSleep ( OSS_ONE_SEC ) ;
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
      rc = startCreateCatalogJob( this, &installCatalogJobID ) ;
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
      // test install status, and decide go on or stop task
      if ( getIsInstallFail() )
      {
         PD_LOG ( PDWARNING, "Install had failed, no need to install coord" ) ;
         goto done ;
      }
      if ( getIsInstallFinish() )
      {
         goto done ;
      }
      // start coord job
      rc = startCreateCoordJob( this, &installCoordJobID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start create coord job, rc = %d", rc ) ;
         goto error ;
      }
      // wait until this job finish
      while ( rtnGetJobMgr()->findJob ( installCoordJobID ) )
      {
         ossSleep ( OSS_ONE_SEC ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::_installData()
   {
      INT32 rc = SDB_OK ;
      map< string, vector<BSONObj> >::iterator it ;
      it = _mapGroups.begin() ;
      while( it != _mapGroups.end() )
      {
         string groupname = it->first ;
         EDUID installDataJobID = PMD_INVALID_EDUID ;
         // test install status, and decide go on or stop task
         if ( getIsInstallFail() )
         {
            PD_LOG ( PDWARNING, "Install had failed, no need to install "
                     "data group[%s]", groupname.c_str() ) ;
            goto done ;
         }
         if ( getIsInstallFinish() )
         {
            goto done ;
         }
         // start data job
         rc = startCreateDataJob( groupname.c_str(), this,
                                  &installDataJobID ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to start create data node job, rc = %d", rc ) ;
            goto error ;
         }
         it++ ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTask::removeVirtualCoord()
   {
      INT32 rc = SDB_OK ;
      EDUID jobID = PMD_INVALID_EDUID ;
      // start remove virtual coord job
      rc = startRemoveVirtualCoordJob( _vCoordSvcName.c_str(), this, &jobID ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start remove virtual coord job, "
                 "rc = %d", rc ) ;
         goto error ;
      }
      // wait until job is finish
      while ( rtnGetJobMgr()->findJob ( jobID ) )
      {
         ossSleep ( OSS_ONE_SEC ) ;
      } 
      // set task finish or fail
      if ( _isRemoveVCoordFinish && !_isRollbackFail)
      {
         setIsTaskFinish( TRUE ) ;
      }
      else if ( _isRemoveVCoordFail || _isRollbackFail )
      {
         setIsTaskFail( TRUE ) ;
      }
      else
      {
         PD_LOG ( PDERROR, "Task[%s] in a unknown status", taskName() ) ;
#if defined (_DEBUG)
         ossPanic() ;
#endif
         rc = SDB_OMA_TASK_FAIL ;
         goto error ;
      }
   done:
      return rc ;
   error:
      // set remove virtual coord fail detail
      setIsRemoveVCoordFail( TRUE ) ;
      setIsTaskFail( TRUE ) ;
      setErrDetail( "Failed to remove virtual coord, please do it manually" ) ;
      goto done ;
   }

}
