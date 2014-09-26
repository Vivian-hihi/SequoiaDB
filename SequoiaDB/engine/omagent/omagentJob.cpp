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

   Source File Name = omagentJob.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

//#include "omagentMsgDef.hpp"
#include "omagentUtil.hpp"
#include "omagentJob.hpp"
#include "omagentCommand.hpp"

namespace engine
{
   /*
       omagent create catalog job
   */
   _omaCreateCatalogJob::_omaCreateCatalogJob (
                                             _omaInstallDBBusinessTask *pTask )
   {
      _name = OMA_JOB_CREATE_CATALOG ;
      _status = OMA_JOB_STATUS_INIT ;
      _pTask = pTask ;
   }

   _omaCreateCatalogJob::~_omaCreateCatalogJob()
   {
   }

   RTN_JOB_TYPE _omaCreateCatalogJob::type () const
   {
      return RTN_JOB_CREATECATALOG ;
   }

   const CHAR* _omaCreateCatalogJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaCreateCatalogJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaCreateCatalogJob::doit()
   {
      INT32 rc = SDB_OK ;
      INT32 tmpRc = SDB_OK ;
      vector<BSONObj> &catalogInstallInfo = _pTask->getInstallCatalogInfo() ;
      vector<BSONObj>::iterator itr ;

      // change job status
      setJobStatus( OMA_JOB_STATUS_RUNNING ) ;
      // begin to run  
      if ( 0 == catalogInstallInfo.size() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "No install catalog info, rc = %d", rc ) ;
         goto error ;
      }
      itr = catalogInstallInfo.begin() ;
      while ( itr != catalogInstallInfo.end() )
      {
         BSONObj retObj ;
         InstallInfo installInfo ;
         CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
         const CHAR *pErrMsg = "" ;
         // get install catalog information
         rc = _getInstallInfo( *itr, installInfo ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to get install catalog info, "
                     "rc = %d", rc ) ;
            goto error ;
         }
         // update install status for web before install catalog
         ossSnprintf( desc, OMA_BUFF_SIZE, "Installing catalog[%s:%s]",
                      installInfo._hostName.c_str(),
                      installInfo._svcName.c_str() ) ;
         rc = _updateInstallStatus( FALSE, SDB_OK, NULL, desc, NULL ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to update status before install catalog "
                     "node, rc = %d", rc ) ;
            goto error ;
         }
         // install catalog
         _omaRunInstallCatalogJob runCmd( _pTask->getVCoordSvcName(),
                                          installInfo ) ;
         rc = runCmd.init( NULL ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Job failed to init for creating catalog, "
                    "rc = %d", rc ) ;
            goto error ;
         }
         rc = runCmd.doit( retObj ) ;
         if ( rc )
         {
            tmpRc = SDB_OK ; ;
            PD_LOG( PDERROR, "Job failed to create catalog, rc = %d", rc ) ;
            tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
            if ( tmpRc )
            {
               PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                        OMA_FIELD_DETAIL, tmpRc ) ;
            }
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Failed to install catalog[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG_MSG ( PDERROR, "Failed to install catalog[%s:%s]: %s",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str(), pErrMsg ) ;
            _updateInstallStatus( FALSE, rc, pErrMsg, desc, NULL ) ;
            goto error ;
         }
         else
         {
            InstalledNode node ;
            node._role = ROLE_CATA ;
            node._dataGroupName = "" ;
            node._hostName = installInfo._hostName.c_str() ;
            node._svcName = installInfo._svcName.c_str() ;
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Finish installing catalog[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG ( PDDEBUG, "Sucessed to install catalog[%s:%s]",
                     installInfo._hostName.c_str(),
                     installInfo._svcName.c_str() ) ;
            _updateInstallStatus( TRUE, SDB_OK, pErrMsg, desc, &node ) ;
         }
         // get next
         itr++ ;
      }
      // set job status to be successful
      setJobStatus( OMA_JOB_STATUS_FINISH ) ;
   done:
      _pTask->setIsTaskFinish( _pTask->isInstallFinish() ) ;
      _pTask->tryToRemoveVirtualCoord() ;
      return rc ;
   error:
      // set job status to be failing
      setJobStatus( OMA_JOB_STATUS_FAIL ) ;
      // set task status to be failing
      _pTask->setStatus( OMA_TASK_STATUS_FAIL ) ;
      // try to rollback
      rc = _pTask->tryToRollbackInternal() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Install db business task failed to rollback, "
                  "rc = %d", rc ) ;
      }
      goto done ;
   }

   INT32 _omaCreateCatalogJob::_getInstallInfo( BSONObj &obj,
                                                InstallInfo &info )
   {
      INT32 rc = SDB_OK ; 
      const CHAR *pStr = NULL ;
      BSONObj conf ;
      BSONObj pattern ;

      // _dataGroupName
      rc = omaGetStringElement( obj, OMA_OPTION_DATAGROUPNAME,
                                &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d",
                OMA_OPTION_DATAGROUPNAME, rc ) ;
      info._dataGroupName = pStr ;
      // _hostname
      rc = omaGetStringElement( obj, OMA_FIELD_HOSTNAME, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME, rc ) ;
      info._hostName = pStr ;
      // _svcName
      rc = omaGetStringElement( obj, OMA_OPTION_SVCNAME, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_SVCNAME, rc ) ;
      info._svcName = pStr ;
      // _dbPath
      rc = omaGetStringElement( obj, OMA_OPTION_DBPATH, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_DBPATH, rc ) ;
      info._dbPath = pStr ;
      // _sdbUser
      rc = omaGetStringElement( obj, OMA_FIELD_SDBUSER, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBUSER, rc ) ;
      info._sdbUser = pStr ;
      // _sdbPasswd
      rc = omaGetStringElement( obj, OMA_FIELD_SDBPASSWD, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBPASSWD, rc ) ;
      info._sdbPasswd = pStr ;
      // _sdbUserGroup
      rc = omaGetStringElement( obj, OMA_FIELD_SDBUSERGROUP, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBUSERGROUP, rc ) ;
      info._sdbUserGroup = pStr ;
      // _user
      rc = omaGetStringElement( obj, OMA_FIELD_USER, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_USER, rc ) ;
      info._user = pStr ;
      // _passwd
      rc = omaGetStringElement( obj, OMA_FIELD_PASSWD, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_PASSWD, rc ) ;
      info._passwd = pStr ;
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME       << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME       << 1 <<
                      OMA_OPTION_DBPATH        << 1 <<
                      OMA_FIELD_VCOORDSVCNAME  << 1 <<
                      OMA_FIELD_SDBUSER        << 1 <<
                      OMA_FIELD_SDBPASSWD      << 1 << 
                      OMA_FIELD_SDBUSERGROUP   << 1 <<
                      OMA_FIELD_USER           << 1 <<
                      OMA_FIELD_PASSWD         << 1 ) ;
      conf = obj.filterFieldsUndotted( pattern, false ) ;
      info._conf = conf.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateCatalogJob::_updateInstallStatus( BOOLEAN isFinish,
                                                     INT32 retRc,
                                                     const CHAR *pErrMsg,
                                                     const CHAR *pDesc,
                                                     InstalledNode *pNode )
   {
      INT32 rc = SDB_OK ;

      rc = _pTask->updateInstallStatus( isFinish, retRc, ROLE_CATA,
                                        pErrMsg, pDesc, NULL, pNode ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to update install catalog information, "
                  "rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      omagent create coord job
   */
   _omaCreateCoordJob::_omaCreateCoordJob ( _omaInstallDBBusinessTask *pTask )
   {
      _name =  OMA_JOB_CREATE_COORD;
      _status = OMA_JOB_STATUS_INIT ;
      _pTask = pTask ;
   }

   _omaCreateCoordJob::~_omaCreateCoordJob()
   {
   }

   RTN_JOB_TYPE _omaCreateCoordJob::type () const
   {
      return RTN_JOB_CREATECOORD ;
   }

   const CHAR* _omaCreateCoordJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaCreateCoordJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaCreateCoordJob::doit()
   {
      INT32 rc = SDB_OK ;
      INT32 tmpRc = SDB_OK ;
      OMA_TASK_STATUS taskStatus = OMA_TASK_STATUS_END ;
      vector<BSONObj> &coordInstallInfo = _pTask->getInstallCoordInfo() ;
      vector<BSONObj>::iterator itr ;

      // change job status
      setJobStatus( OMA_JOB_STATUS_RUNNING ) ;
      // check 
      if ( 0 == coordInstallInfo.size() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "No install catalog info, rc = %d", rc ) ;
         goto error ;
      }
      itr = coordInstallInfo.begin() ;
      while ( itr != coordInstallInfo.end() )
      {
         BSONObj retObj ;
         InstallInfo installInfo ;
         CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
         const CHAR *pErrMsg = "" ;

         // check task's status and decide to go on or stop
         taskStatus = _pTask->status() ;
         if ( OMA_TASK_STATUS_FAIL == taskStatus )
         {
            rc = SDB_OMA_TASK_FAIL ;
            PD_LOG ( PDWARNING, "Stop installing coord, task had failed" ) ;
            goto error ;
         }
         // get installl coord information
         rc = _getInstallInfo( *itr, installInfo ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to get install coord info, "
                     "rc = %d", rc ) ;
            goto error ;
         }
         // update install status for web before install coord
         ossSnprintf( desc, OMA_BUFF_SIZE, "Installing coord[%s:%s]",
                      installInfo._hostName.c_str(),
                      installInfo._svcName.c_str() ) ;
         rc = _updateInstallStatus( FALSE, SDB_OK, NULL, desc, NULL ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to update status before install coord, "
                     "rc = %d", rc ) ;
            goto error ;
         }
         // install coord
         _omaRunInstallCoordJob runCmd( _pTask->getVCoordSvcName(),
                                        installInfo ) ;
         rc = runCmd.init( NULL ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Job failed to init for creating coord, "
                    "rc = %d", rc ) ;
            goto error ;
         }
         rc = runCmd.doit( retObj ) ;
         if ( rc )
         {
            tmpRc = SDB_OK ; ;
            PD_LOG( PDERROR, "Job failed to create coord, rc = %d", rc ) ;
            tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
            if ( tmpRc )
            {
               PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                        OMA_FIELD_DETAIL, tmpRc ) ;
            }
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Failed to install coord[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG_MSG ( PDERROR, "Failed to install coord[%s:%s]: %s",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str(), pErrMsg ) ;
            _updateInstallStatus( FALSE, rc, pErrMsg, desc, NULL ) ;
            goto error ;
         }
         else
         {
            InstalledNode node ;
            node._role = ROLE_CATA ;
            node._dataGroupName = "" ;
            node._hostName = installInfo._hostName.c_str() ;
            node._svcName = installInfo._svcName.c_str() ;
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Finish installing coord[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG ( PDDEBUG, "Sucessed to install coord[%s:%s]",
                     installInfo._hostName.c_str(),
                     installInfo._svcName.c_str() ) ;
            _updateInstallStatus( TRUE, SDB_OK, pErrMsg, desc, &node ) ;
         }
         // get next
         itr++ ;
      }
      // set job status to be successful
      setJobStatus( OMA_JOB_STATUS_FINISH ) ;

      // ckeck task's status before return,
      // for task may be failing because of
      // other job's error
      taskStatus = _pTask->status() ;
      if ( OMA_TASK_STATUS_FAIL == taskStatus )
      {
         rc = SDB_OMA_TASK_FAIL ;
         PD_LOG ( PDWARNING, "Task[%s] had failed", _pTask->taskName() ) ;
         goto rollback ;
      }

   done:
      _pTask->setIsTaskFinish( _pTask->isInstallFinish() ) ;
      _pTask->tryToRemoveVirtualCoord() ;
      return rc ;
   error:
      // set job status to be failing
      setJobStatus( OMA_JOB_STATUS_FAIL ) ;
      // set task status to be false
      _pTask->setStatus( OMA_TASK_STATUS_FAIL ) ;
   rollback:
      tmpRc = rc ;
      // try to rollback
      rc = _pTask->tryToRollbackInternal() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Install db business task failed to rollback, "
                  "rc = %d", rc ) ;
      }
      rc = tmpRc ;
      goto done ;
   }

   INT32 _omaCreateCoordJob::_getInstallInfo( BSONObj &obj,
                                              InstallInfo &info )
   {
      INT32 rc = SDB_OK ; 
      const CHAR *pStr = NULL ;
      BSONObj conf ;
      BSONObj pattern ;

      // _hostname
      rc = omaGetStringElement( obj, OMA_FIELD_HOSTNAME, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME, rc ) ;
      info._hostName = pStr ;
      // _svcName
      rc = omaGetStringElement( obj, OMA_OPTION_SVCNAME, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_SVCNAME, rc ) ;
      info._svcName = pStr ;
      // _dbPath
      rc = omaGetStringElement( obj, OMA_OPTION_DBPATH, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_DBPATH, rc ) ;
      info._dbPath = pStr ;
      // _sdbUser
      rc = omaGetStringElement( obj, OMA_FIELD_SDBUSER, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBUSER, rc ) ;
      info._sdbUser = pStr ;
      // _sdbPasswd
      rc = omaGetStringElement( obj, OMA_FIELD_SDBPASSWD, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBPASSWD, rc ) ;
      info._sdbPasswd = pStr ;
      // _sdbUserGroup
      rc = omaGetStringElement( obj, OMA_FIELD_SDBUSERGROUP, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBUSERGROUP, rc ) ;
      info._sdbUserGroup = pStr ;
      // _user
      rc = omaGetStringElement( obj, OMA_FIELD_USER, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_USER, rc ) ;
      info._user = pStr ;
      // _passwd
      rc = omaGetStringElement( obj, OMA_FIELD_PASSWD, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_PASSWD, rc ) ;
      info._passwd = pStr ;
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME       << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME       << 1 <<
                      OMA_OPTION_DBPATH        << 1 <<
                      OMA_FIELD_VCOORDSVCNAME  << 1 <<
                      OMA_FIELD_SDBUSER        << 1 <<
                      OMA_FIELD_SDBPASSWD      << 1 <<
                      OMA_FIELD_SDBUSERGROUP   << 1 <<
                      OMA_FIELD_USER           << 1 <<
                      OMA_FIELD_PASSWD         << 1 ) ;
      conf = obj.filterFieldsUndotted( pattern, false ) ;
      info._conf = conf.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateCoordJob::_updateInstallStatus( BOOLEAN isFinish,
                                                   INT32 retRc,
                                                   const CHAR *pErrMsg,
                                                   const CHAR *pDesc,
                                                   InstalledNode *pNode )
   {
      INT32 rc = SDB_OK ;

      rc = _pTask->updateInstallStatus( isFinish, retRc, ROLE_COORD,
                                        pErrMsg, pDesc, NULL, pNode ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to update install coord information, "
                  "rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      omagent create data job
   */
   _omaCreateDataJob::_omaCreateDataJob ( const CHAR *pGroupName,
                                          _omaInstallDBBusinessTask *pTask )
   {
      _groupname = pGroupName ;
      _name = _name + "create data node in " + pGroupName  ;
      _status = OMA_JOB_STATUS_INIT ;
      _pTask = pTask ;
   }

   _omaCreateDataJob::~_omaCreateDataJob()
   {
   }

   RTN_JOB_TYPE _omaCreateDataJob::type () const
   {
      return RTN_JOB_CREATEDATA ;
   }

   const CHAR* _omaCreateDataJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaCreateDataJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaCreateDataJob::doit()
   {
      INT32 rc = SDB_OK ;
      INT32 tmpRc = SDB_OK ;
      vector<BSONObj>::iterator itr ;
      vector<BSONObj> dataNodeInstallInfo ;
      OMA_TASK_STATUS taskStatus = OMA_TASK_STATUS_END ;

      // change job status
      setJobStatus( OMA_JOB_STATUS_RUNNING ) ;
      // get install data group information 
      rc = _pTask->getInstallDataGroupInfo( _groupname,
                                            dataNodeInstallInfo ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get install data node information "
                  "in group %s, rc = %d", _groupname.c_str(), rc ) ;
         goto error ;
      }
      // check the install info
      if ( 0 == dataNodeInstallInfo.size() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "No install data node info in group %s, rc = %d",
                  _groupname.c_str(), rc ) ;
         goto error ;
      }
      itr = dataNodeInstallInfo.begin() ;
      while ( itr != dataNodeInstallInfo.end() )
      {
         BSONObj retObj ;
         InstallInfo installInfo ;
         CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
         const CHAR *pErrMsg = "" ;

         // check task's status and decide to do or stop
         taskStatus = _pTask->status() ;
         if ( OMA_TASK_STATUS_FAIL == taskStatus )
         {
            rc = SDB_OMA_TASK_FAIL ;
            PD_LOG ( PDWARNING, "Stop installing data node in group [%s], "
                     "task had failed", _groupname.c_str() ) ;
            goto error ;
         }
         // get installl data node information
         rc = _getInstallInfo( *itr, installInfo ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to get install data node info, "
                     "rc = %d", rc ) ;
            goto error ;
         }
         // update install status for web before install data node
         ossSnprintf( desc, OMA_BUFF_SIZE, "Installing data node[%s:%s]",
                      installInfo._hostName.c_str(),
                      installInfo._svcName.c_str() ) ;
         rc = _updateInstallStatus( FALSE, SDB_OK, NULL, desc, NULL ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to update status before "
                     "install data node, rc = %d", rc ) ;
            goto error ;
         }
         // install data node
         _omaRunInstallDataNodeJob runCmd( _pTask->getVCoordSvcName(),
                                           installInfo ) ;
         rc = runCmd.init( NULL ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Job failed to init for installing data node, "
                    "rc = %d", rc ) ;
            goto error ;
         }
         rc = runCmd.doit( retObj ) ;
         if ( rc )
         {
            tmpRc = SDB_OK ; ;
            PD_LOG( PDERROR, "Job failed to create data node, rc = %d", rc ) ;
            tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
            if ( tmpRc )
            {
               PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                        OMA_FIELD_DETAIL, tmpRc ) ;
            }
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Failed to install data node[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG_MSG ( PDERROR, "Failed to install data node[%s:%s]: %s",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str(), pErrMsg ) ;
            _updateInstallStatus( FALSE, rc, pErrMsg, desc, NULL ) ;
            goto error ;
         }
         else
         {
            InstalledNode node ;
            node._role = ROLE_CATA ;
            node._dataGroupName = _groupname ;
            node._hostName = installInfo._hostName.c_str() ;
            node._svcName = installInfo._svcName.c_str() ;
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Finish installing data node[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG ( PDDEBUG, "Sucessed to install data node[%s:%s]",
                     installInfo._hostName.c_str(),
                     installInfo._svcName.c_str() ) ;
            _updateInstallStatus( TRUE, SDB_OK, pErrMsg, desc, &node ) ;
         }
         // get next
         itr++ ;
      }
      // set job status to be successful
      // change job status
      setJobStatus( OMA_JOB_STATUS_FINISH ) ;
      // ckeck task's status before return,
      // for task may be failing because of 
      // other job's error
      taskStatus = _pTask->status() ;
      if ( OMA_TASK_STATUS_FAIL == taskStatus )
      {
         rc = SDB_OMA_TASK_FAIL ;
         PD_LOG ( PDWARNING, "Task[%s] had failed", _pTask->taskName() ) ;
         goto rollback ;
      }

   done:
      _pTask->setIsTaskFinish( _pTask->isInstallFinish() ) ;
      _pTask->tryToRemoveVirtualCoord() ;
      return rc ;
   error:
      // set job status to be failing
      setJobStatus( OMA_JOB_STATUS_FAIL ) ;
      // set task status to be false
      _pTask->setStatus( OMA_TASK_STATUS_FAIL ) ;
   rollback:
      tmpRc = rc ;
      // try to rollback
      rc = _pTask->tryToRollbackInternal() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Install db business task failed to rollback, "
                  "rc = %d", rc ) ;
      }
      rc = tmpRc ;
      goto done ;
   }

   INT32 _omaCreateDataJob::_getInstallInfo ( BSONObj &obj,
                                              InstallInfo &installInfo )
   {
      INT32 rc = SDB_OK ;
      const CHAR* pStr = "" ;
      BSONObj conf ;
      BSONObj pattern ;

      // _dataGroupName
      rc = omaGetStringElement( obj, OMA_OPTION_DATAGROUPNAME, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d",
                OMA_OPTION_DATAGROUPNAME, rc ) ;
      installInfo._dataGroupName = pStr ;
      // _hostname
      rc = omaGetStringElement( obj, OMA_FIELD_HOSTNAME, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_HOSTNAME, rc ) ;
      installInfo._hostName = pStr ;
      // _svcName
      rc = omaGetStringElement( obj, OMA_OPTION_SVCNAME, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_SVCNAME, rc ) ;
      installInfo._svcName = pStr ;
      // _dbPath
      rc = omaGetStringElement( obj, OMA_OPTION_DBPATH, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_DBPATH, rc ) ;
      installInfo._dbPath = pStr ;
      // _sdbUser
      rc = omaGetStringElement( obj, OMA_FIELD_SDBUSER, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBUSER, rc ) ;
      installInfo._sdbUser = pStr ;
      // _sdbPasswd
      rc = omaGetStringElement( obj, OMA_FIELD_SDBPASSWD, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBPASSWD, rc ) ;
      installInfo._sdbPasswd = pStr ;
      // _sdbUserGroup
      rc = omaGetStringElement( obj, OMA_FIELD_SDBUSERGROUP, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SDBUSERGROUP, rc ) ;
      installInfo._sdbUserGroup = pStr ;
      // _user
      rc = omaGetStringElement( obj, OMA_FIELD_USER, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_USER, rc ) ;
      installInfo._user = pStr ;
      // _passwd
      rc = omaGetStringElement( obj, OMA_FIELD_PASSWD, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_PASSWD, rc ) ;
      installInfo._passwd = pStr ;
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME       << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME       << 1 <<
                      OMA_OPTION_DBPATH        << 1 <<
                      OMA_FIELD_VCOORDSVCNAME  << 1 <<
                      OMA_FIELD_SDBUSER        << 1 <<
                      OMA_FIELD_SDBPASSWD      << 1 <<
                      OMA_FIELD_SDBUSERGROUP   << 1 <<
                      OMA_FIELD_USER           << 1 <<
                      OMA_FIELD_PASSWD         << 1 ) ;
      conf = obj.filterFieldsUndotted( pattern, false ) ;
      installInfo._conf = conf.getOwned() ; 

   done:
     return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateDataJob::_updateInstallStatus( BOOLEAN isFinish,
                                                  INT32 retRc,
                                                  const CHAR *pErrMsg,
                                                  const CHAR *pDesc,
                                                  InstalledNode *pNode )
   {
      INT32 rc = SDB_OK ;

      rc = _pTask->updateInstallStatus( isFinish, retRc,
                                        ROLE_DATA, pErrMsg, pDesc,
                                        _groupname.c_str(), pNode ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to update install data node information, "
                  "rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

  /*
      install db business task rollback job
   */
   _omaStartInstallDBBusinessTaskJob::_omaStartInstallDBBusinessTaskJob ( 
                                                         BSONObj &installInfo )
   {
//      ossMemset( _omaHostName, 0, OSS_MAX_HOSTNAME + 1 ) ;
//      ossMemset( _omaSvcName, 0, OSS_MAX_SERVICENAME + 1 ) ;
      ossMemset( _vCoordSvcName, 0, OSS_MAX_SERVICENAME + 1 ) ;
      _installInfoObj = installInfo.getOwned() ;
      _name = OMA_JOB_START_INSTALL_DB_BUSINESS ;
   }

   _omaStartInstallDBBusinessTaskJob::~_omaStartInstallDBBusinessTaskJob()
   {
   }

   RTN_JOB_TYPE _omaStartInstallDBBusinessTaskJob::type () const
   {
      return RTN_JOB_STARTINSDBBUSTASK ;
   }

   const CHAR* _omaStartInstallDBBusinessTaskJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaStartInstallDBBusinessTaskJob::muteXOn (
                                                    const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaStartInstallDBBusinessTaskJob::init()
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      BSONObj filter ;
      BSONObj commonFileds ;
      BSONObjBuilder builder ;
      BSONObj vCoordRet ;
      _omaCreateVirtualCoord createVCoord ;
      const CHAR *pStr = NULL ;

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
      // parse arguments
      PD_LOG ( PDDEBUG, "Add db business passes argument: %s",
               _installInfoObj.toString( FALSE, TRUE ).c_str() ) ;
      // get taskID from omsvc
      ele = _installInfoObj.getField( OMA_FIELD_TASKID ) ;
      if ( NumberInt != ele.type() && NumberLong != ele.type() )
      {
         PD_LOG_MSG ( PDERROR, "Receive invalid task id from omsvc" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      _taskID = ele.numberLong() ;
      // get common fields
      rc = omaGetStringElement ( _installInfoObj, OMA_FIELD_SDBUSER, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_FIELD_SDBUSER, rc ) ;
      builder.append( OMA_FIELD_SDBUSER, pStr ) ;
      rc = omaGetStringElement ( _installInfoObj, OMA_FIELD_SDBPASSWD, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_FIELD_SDBPASSWD, rc ) ;
      builder.append( OMA_FIELD_SDBPASSWD, pStr ) ;
      rc = omaGetStringElement ( _installInfoObj, OMA_FIELD_SDBUSERGROUP, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_FIELD_SDBUSERGROUP, rc ) ;
      builder.append( OMA_FIELD_SDBUSERGROUP, pStr ) ;
      commonFileds = builder.obj() ;
      // parse bson and get arguments info for js file
      ele = _installInfoObj.getField ( OMA_FIELD_CONFIG ) ;
      if ( Array != ele.type() )
      {
         PD_LOG_MSG ( PDERROR, "Receive wrong format install "
                      "info from omsvc" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
            BSONObjBuilder bob ;
            BSONObj info ;
            BSONObj temp ;
            const CHAR *value = NULL ;
            ele = itr.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG ( PDERROR, "Receive wrong format bson from omsvc" ) ;
               goto error ;
            }
            temp = ele.embeddedObject() ;
            bob.appendElements( temp ) ;
            bob.appendElements( commonFileds ) ;
            bob.appendElements( vCoordRet ) ;
            info = bob.obj() ;
            // category
            rc = omaGetStringElement ( temp, OMA_OPTION_ROLE, &value ) ;
            if ( rc )
            {
               PD_LOG_MSG ( PDERROR, "Get field[%s] failed, rc = %d",
                            OMA_OPTION_ROLE, rc ) ;
               goto error ;
            }
            if ( 0 == ossStrncmp( value, ROLE_DATA,
                                  ossStrlen( ROLE_DATA ) ) )
            {
               _data.push_back( info ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_COORD,
                                       ossStrlen( ROLE_COORD ) ) )
            {
               _coord.push_back( info ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_CATA,
                                       ossStrlen( ROLE_CATA ) ) )
            {
               _catalog.push_back( info ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_STANDALONE,
                                       ossStrlen( ROLE_STANDALONE ) ) )
            {
               _standalone.push_back( info ) ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG( PDERROR, "Unknown role for install db business[%s]",
                           temp.toString( FALSE, TRUE ).c_str() ) ;
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaStartInstallDBBusinessTaskJob::doit()
   {
      INT32 rc                         = SDB_OK ;
      _omaInstallDBBusinessTask *pTask = NULL ;
      _omaTaskMgr *pTaskMgr            = getTaskMgr() ;
      BSONObj otherInfo ;

      // init
      rc = init() ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to init to start install db "
                  "business task job, rc = %d", rc ) ;
         goto error ;
      }
      // remove last task with the same name
      rc = pTaskMgr->removeTask ( OMA_TASK_NAME_INSTALL_DB_BUSINESS ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to remove task[%s], "
                     "rc = %d", OMA_TASK_NAME_INSTALL_DB_BUSINESS, rc ) ;
         goto error ;
      }
      // create install db business task
      pTask = SDB_OSS_NEW _omaInstallDBBusinessTask( _taskID ) ;
      if ( !pTask )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG( PDERROR,
                     "Failed to create install db business task, rc = %d",
                     rc ) ;
         goto error ;
      }
      // register install db business task
      pTaskMgr->addTask( pTask, _taskID ) ;
      // start install db task
      otherInfo = BSON( OMA_FIELD_VCOORDSVCNAME << _vCoordSvcName ) ;
      rc = pTask->init( _coord, _catalog, _data, otherInfo ) ;
      if ( rc  )
      {
         PD_LOG_MSG( PDERROR,
                     "Failed to init install db busniness task, rc = %d",
                     rc ) ;
         goto error ;
      }
      rc = pTask->doit() ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR,
                     "Failed to do db busniness task, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaStartInstallDBBusinessTaskJob::_saveVCoordInfo( BSONObj &info )
   {
      INT32 rc                    = SDB_OK ;
//      const CHAR *pOmaHostName    = NULL ;
//      const CHAR *pOmaSvcName     = NULL ;
      const CHAR *pVCoordSvcName  = NULL ;

//      // get virtual info
//      PD_LOG ( PDDEBUG, "Create virtual coord return: %s",
//               info.toString(FALSE, TRUE).c_str() ) ;
//      rc = omaGetStringElement( info, OMA_FIELD_OMAHOSTNAME, &pOmaHostName ) ;
//      if ( rc )
//      {
//         PD_LOG ( PDERROR, "Failed to get filed[%s], rc = %s",
//                  OMA_FIELD_OMAHOSTNAME, rc ) ;
//         goto error ;
//      }
//      rc = omaGetStringElement( info, OMA_FIELD_OMASVCNAME, &pOmaSvcName ) ;
//      if ( rc )
//      {
//         PD_LOG ( PDERROR, "Failed to get filed[%s], rc = %s",
//                  OMA_FIELD_OMASVCNAME, rc ) ;
//         goto error ;
//      }
      rc = omaGetStringElement( info, OMA_FIELD_VCOORDSVCNAME, &pVCoordSvcName ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get filed[%s], rc = %s",
                  OMA_FIELD_VCOORDSVCNAME, rc ) ;
         goto error ;
      }
//      ossStrncpy( _omaHostName, pOmaHostName, OSS_MAX_HOSTNAME ) ;
//      ossStrncpy( _omaSvcName, pOmaSvcName, OSS_MAX_SERVICENAME ) ;
      ossStrncpy( _vCoordSvcName, pVCoordSvcName, OSS_MAX_SERVICENAME ) ;
   done:
      return rc ;
   error:
      goto done ;
   }



  /*
      install db business task rollback job
   */
   _omaInstallDBBusinessTaskRollbackJob::_omaInstallDBBusinessTaskRollbackJob (
                                              string &vCoordSvcName,
                                              _omaInstallDBBusinessTask *pTask )
   {
      _status = OMA_JOB_STATUS_INIT ;
      _name = OMA_JOB_ROLLBACK_INSTALL_DB_BUSINESS ;
      _vCoordSvcName = vCoordSvcName ;
      _pTask = pTask ;
   }

   _omaInstallDBBusinessTaskRollbackJob::~_omaInstallDBBusinessTaskRollbackJob()
   {
   }

   RTN_JOB_TYPE _omaInstallDBBusinessTaskRollbackJob::type () const
   {
      return RTN_JOB_INSDBBUSTASKRB ;
   }

   const CHAR* _omaInstallDBBusinessTaskRollbackJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaInstallDBBusinessTaskRollbackJob::muteXOn (
                                                    const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaInstallDBBusinessTaskRollbackJob::doit()
   {
      INT32 rc = SDB_OK ;
      RollbackInfo info ;
      BSONObj retObj ;

      rc = _getRollbackInfo ( info ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get rollback info, rc = %d", rc ) ;
         goto error ;
      }
      // rollback data nodes
      rc = _rollbackDataNode ( _vCoordSvcName,
                               info._dataGroupRollbackInfo ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to rollback data nodes, rc = %d", rc ) ;
         goto error ;
      }
      // rollback coord nodes
      rc = _rollbackCoord ( _vCoordSvcName,
                            info._coordRollbackInfo ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to rollback coord nodes, rc = %d", rc ) ;
         goto error ;
      }
      // rollback catalog nodes
      rc = _rollbackCatalog ( _vCoordSvcName,
                              info._catalogRollbackInfo ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to rollback catalog nodes, rc = %d", rc ) ;
         goto error ;
      }      
      // set task stage
      _pTask->setTaskStage( OMA_INSTALL_ROLLBACK ) ;
      _pTask->setIsTaskFinish( TRUE ) ;
 
   done:
      _pTask->tryToRemoveVirtualCoord() ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTaskRollbackJob::_getRollbackInfo(
                                                           RollbackInfo &info )
   {
      INT32 rc = SDB_OK ;
      rc = _pTask->getInstalledNodeResult ( ROLE_COORD,
                                            info._coordRollbackInfo ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get coord rollback info, rc = %d", rc ) ;
         goto error ;
      }
      rc = _pTask->getInstalledNodeResult ( ROLE_CATA,
                                            info._catalogRollbackInfo ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get catalog rollback info, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      rc = _pTask->getInstalledNodeResult ( ROLE_DATA,
                                            info._dataGroupRollbackInfo ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get datagroup rollback info, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      
   done:
      return rc ;
   error:
     goto done ;
   }

   INT32 _omaInstallDBBusinessTaskRollbackJob::_rollbackCoord (
                                  string &vCoordSvcName,
                                  map< string, vector< InstalledNode> > &info )
   {
      INT32 rc = SDB_OK ;
      BSONObj retObj ;
      _omaRunRollbackCoordJob rollbackCoord ( vCoordSvcName, info ) ;
      map< string, vector< InstalledNode> >::iterator it = info.begin() ;
      
      if ( ( 1 != info.size() ) && ( string( ROLE_COORD ) != it->first ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Invalid coord's rollback info" ) ;
         goto error ;
      }
      // rollback data group
      rc = rollbackCoord.init( NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to init to do rollback create coord "
                  "rc = %d", rc ) ;
         goto error ;
      }
      rc = rollbackCoord.doit( retObj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to do rollback create coord, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "The rollback coord's result is: %s",
               retObj.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ; 
   }

   INT32 _omaInstallDBBusinessTaskRollbackJob::_rollbackCatalog (
                                  string &vCoordSvcName,
                                  map< string, vector< InstalledNode> > &info )
   {
      INT32 rc = SDB_OK ;
      BSONObj retObj ;
      _omaRunRollbackCatalogJob rollbackCatalog ( vCoordSvcName, info ) ;
      map< string, vector< InstalledNode> >::iterator it = info.begin() ;

      if ( ( 1 != info.size() ) && ( string( ROLE_CATA ) != it->first ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Invalid catalog's rollback info" ) ;
         goto error ;
      }
      // rollback data group
      rc = rollbackCatalog.init( NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to init to do rollback create catalog "
                  "rc = %d", rc ) ;
         goto error ;
      }
      rc = rollbackCatalog.doit( retObj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to do rollback create catalog, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "The rollback catalog's result is: %s",
               retObj.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInstallDBBusinessTaskRollbackJob::_rollbackDataNode (
                                  string &vCoordSvcName,
                                  map< string, vector< InstalledNode> > &info )
   {
      INT32 rc = SDB_OK ;
      BSONObj retObj ;
      _omaRunRollbackDataNodeJob rollbackDataNode ( vCoordSvcName, info ) ;
      map< string, vector<InstalledNode> >::iterator it = info.begin() ;

      // rollback data group
      rc = rollbackDataNode.init( NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to init to do rollback create data "
                  "node job, rc = %d", rc ) ;
         goto error ;
      }
      rc = rollbackDataNode.doit( retObj ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to do rollback create data node job, "
                  "rc = %d", rc ) ;
         goto error ;
      }
      PD_LOG ( PDEVENT, "The rollback data groups' result is: %s",
               retObj.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

  /*
      omagent remove virtual coord job
   */
   _omaRemoveVirtualCoordJob::_omaRemoveVirtualCoordJob (
                                                    const CHAR *vCoordSvcName )
   {
      _vCoordSvcName  = vCoordSvcName ;
   }

   _omaRemoveVirtualCoordJob::~_omaRemoveVirtualCoordJob()
   {
   }

   RTN_JOB_TYPE _omaRemoveVirtualCoordJob::type () const
   {
      return RTN_JOB_REMOVEVIRTUALCOORD ;
   }

   const CHAR* _omaRemoveVirtualCoordJob::name () const
   {
      return OMA_JOB_REMOVE_VIRTUAL_COORD ;
   }

   BOOLEAN _omaRemoveVirtualCoordJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaRemoveVirtualCoordJob::doit()
   {
      INT32 rc = SDB_OK ;
      BSONObj removeRet ;
      _omaRemoveVirtualCoord removeVCoord( _vCoordSvcName.c_str() ) ;
      // TODO: how to deal with the result
      rc = removeVCoord.removeVirtualCoord ( removeRet ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to remove virtual coord, rc = %d", rc ) ;
         goto error ;
      }    
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateCatalogJob ( _omaInstallDBBusinessTask *pTask,
                                 EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaCreateCatalogJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaCreateCatalogJob( pTask ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for creating catalog job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateCoordJob ( _omaInstallDBBusinessTask *pTask,
                               EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaCreateCoordJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaCreateCoordJob( pTask ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for creating coord job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateDataJob ( const CHAR *pGroupName,
                              _omaInstallDBBusinessTask *pTask,
                              EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaCreateDataJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaCreateDataJob( pGroupName, pTask ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for "
                  "creating data node job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startStartInstallDBBusinessTaskJob ( const CHAR *pInstallInfo,
                                              EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaStartInstallDBBusinessTaskJob *pJob = NULL ;
      try
      {
         BSONObj info( pInstallInfo ) ;
         pJob = SDB_OSS_NEW _omaStartInstallDBBusinessTaskJob( info ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Occur error, exception is: %s", e.what() ) ;
         goto error ;
      }
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for starting "
                  "install db business job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;


   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startInstallDBBusinessTaskRollbackJob (
                                              string &vCoordSvcName,
                                              _omaInstallDBBusinessTask *pTask,
                                              EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaInstallDBBusinessTaskRollbackJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaInstallDBBusinessTaskRollbackJob( vCoordSvcName,
                                                               pTask ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for "
                  "installing db business rollback job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startRemoveVirtualCoordJob ( const CHAR *vCoordSvcName,
                                      EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaRemoveVirtualCoordJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaRemoveVirtualCoordJob( vCoordSvcName ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for "
                  "removing virtual coord job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

}
