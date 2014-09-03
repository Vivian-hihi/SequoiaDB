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

#include "omagentUtil.hpp"
#include "omagentJob.hpp"
#include "omagentCommand.hpp"

#define OMA_BUFF_SIZE        (1024)

namespace engine
{
   /*
       omagent create catalog job
   */
   _omaCreateCatalogJob::_omaCreateCatalogJob (
                                             _omaInstallDBBusinessTask *pTask )
   {
      _name = "create catalog job" ;
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
      vector<BSONObj> &catalogInstallInfo = _pTask->getInstallCatalogInfo() ;
      vector<BSONObj>::iterator itr ;

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
         CHAR buffer [OMA_BUFF_SIZE + 1] = { 0 } ;

         // check task's status and decide to do or stop
         OMA_TASK_STATUS taskStatus = _pTask->status() ;
         if ( OMA_TASK_STATUS_FAIL == taskStatus )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDEVENT, "Stop installing catalog, task has failed" ) ;
            goto done ;
         }
         // get installl catalog information
         rc = _getInstallInfo( *itr, installInfo ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to get install catalog info, "
                     "rc = %d", rc ) ;
            goto error ;
         }
         // update install status for web before install catalog
         ossSnprintf( buffer, OMA_BUFF_SIZE, "Installing catalog[%s:%s]",
                      installInfo._hostName.c_str(),
                      installInfo._svcName.c_str() ) ;
         rc = _updateInstallStatus( SDB_OK, NULL, buffer, FALSE, NULL ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to update status before install catalog "
                     "node, rc = %d", rc ) ;
            goto error ;
         }
         // install catalog
         _omaJobRunInstallCatalogCmd runCmd( installInfo ) ;
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
            PD_LOG( PDERROR, "Job failed to create catalog, rc = %d", rc ) ;
            goto error ;
         }
         // check the install result
         // TODO: retObj contant hostname, port
         // and this function will update install status at the same time
         rc = _checkInstallResult ( installInfo._hostName.c_str(),
                                    installInfo._svcName.c_str(),
                                    retObj ) ;
         if ( rc ) 
         {
            PD_LOG ( PDERROR, "Failed to check install result, rc = %d", rc ) ;
            goto error ;
         }
         // get next
         itr++ ;
      }
   done:
      return rc ;
   error:
      // set task status to be false
      _pTask->setStatus( OMA_TASK_STATUS_FAIL ) ;
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
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME << 1 <<
                      OMA_OPTION_DBPATH << 1 ) ;
      conf = obj.filterFieldsUndotted( pattern, false ) ;
      info._conf = conf.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateCatalogJob::_checkInstallResult( const CHAR *pHostName,
                                                    const CHAR *pSvcName,
                                                    BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      INT32 retRc = SDB_OK ;
      CHAR desc[OMA_BUFF_SIZE + 1] = { 0 } ;
      const CHAR *pErrMsg = "" ;
//      const CHAR *pHostName = "" ;
//      const CHAR *pSvcName = "" ;

      // extract return rc
      rc = omaGetIntElement ( obj, OMA_FIELD_RC, retRc ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                  OMA_FIELD_RC, rc ) ;
//         goto error ;
      }
/*
      rc = omaGetStringElement ( obj, OMA_FIELD_HOSTNAME, pHostName ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                  OMA_FIELD_HOSTNAME, rc ) ;
//         goto error ;
      }
      rc = omaGetStringElement ( obj, OMA_FIELD_SVCNAME,, pSvcName ) ;
      {
         PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                  OMA_FIELD_SVCNAME, rc ) ;
//         goto error ;
      }
*/
      if ( retRc )
      {
         rc = omaGetStringElement ( obj, OMA_FIELD_DETAIL, &pErrMsg ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                     OMA_FIELD_DETAIL, rc ) ;
//          goto error ;
         }
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Failed to install catalog[%s:%s]",
                      pHostName, pSvcName ) ;
         PD_LOG_MSG ( PDERROR, "Failed to install data node[%s:%s]: %s",
                      pHostName, pSvcName, pErrMsg ) ;
         _updateInstallStatus( retRc, pErrMsg, desc, FALSE, NULL ) ;
         rc = retRc ;
         goto error ;
      }
      else
      {
         InstalledNode node ;
         node._role = ROLE_CATA ;
         node._dataGroupName = "" ;
         node._hostName = pHostName ;
         node._svcName = pSvcName ;
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Finish installing catalog[%s:%s]",
                      pHostName, pSvcName ) ;
         PD_LOG ( PDDEBUG, "Sucessed to install catalog[%s:%s]",
                  pHostName, pSvcName ) ;
         _updateInstallStatus( retRc, pErrMsg, desc, TRUE, &node ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateCatalogJob::_updateInstallStatus(
                                                  INT32 retRc,
                                                  const CHAR *pErrMsg,
                                                  const CHAR *pDesc,
                                                  BOOLEAN isFinish,
                                                  InstalledNode *pNode )
   {
      INT32 rc = SDB_OK ;

      rc = _pTask->updateInstallResult( retRc, ROLE_CATA, pErrMsg,
                                        pDesc, NULL, isFinish, pNode ) ;
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

   /*
      omagent create coord job
   */
   _omaCreateCoordJob::_omaCreateCoordJob ( _omaInstallDBBusinessTask *pTask )
   {
      _name = "create coord job" ;
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
      vector<BSONObj> &coordInstallInfo = _pTask->getInstallCoordInfo() ;
      vector<BSONObj>::iterator itr ;

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
         CHAR buffer [OMA_BUFF_SIZE + 1] = { 0 } ;

         // check task's status and decide to do or stop
         OMA_TASK_STATUS taskStatus = _pTask->status() ;
         if ( OMA_TASK_STATUS_FAIL == taskStatus )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDEVENT, "Stop installing coord, task has failed" ) ;
            goto done ;
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
         ossSnprintf( buffer, OMA_BUFF_SIZE, "Installing coord[%s:%s]",
                      installInfo._hostName.c_str(),
                      installInfo._svcName.c_str() ) ;
         rc = _updateInstallStatus( SDB_OK, NULL, buffer, FALSE, NULL ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to update status before install catalog "
                     "node, rc = %d", rc ) ;
            goto error ;
         }
         // install coord
         _omaJobRunInstallCatalogCmd runCmd( installInfo ) ;
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
            PD_LOG( PDERROR, "Job failed to create coord, rc = %d", rc ) ;
            goto error ;
         }
         // check the install result
         // TODO: retObj contant hostname, port
         // and this function will update install status at the same time
         rc = _checkInstallResult ( installInfo._hostName.c_str(),
                                    installInfo._svcName.c_str(),
                                    retObj ) ;
         if ( rc ) 
         {
            PD_LOG ( PDERROR, "Failed to check install result, rc = %d", rc ) ;
            goto error ;
         }
         // get next
         itr++ ;
      }
   done:
      return rc ;
   error:
      // set task status to be false
      _pTask->setStatus( OMA_TASK_STATUS_FAIL ) ;
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
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME << 1 <<
                      OMA_OPTION_DBPATH << 1 ) ;
      conf = obj.filterFieldsUndotted( pattern, false ) ;
      info._conf = conf.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateCoordJob::_checkInstallResult( const CHAR *pHostName,
                                                  const CHAR *pSvcName,
                                                  BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      INT32 retRc = SDB_OK ;
      const CHAR *pErrMsg = "" ;
      CHAR desc[OMA_BUFF_SIZE + 1] = { 0 } ;

      // extract return rc
      rc = omaGetIntElement ( obj, OMA_FIELD_RC, retRc ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                  OMA_FIELD_RC, rc ) ;
//         goto error ;
      }
      if ( retRc )
      {
         rc = omaGetStringElement ( obj, OMA_FIELD_DETAIL, &pErrMsg ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                     OMA_FIELD_DETAIL, rc ) ;
//          goto error ;
         }
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Failed to install coord[%s:%s]",
                      pHostName, pSvcName ) ;
         PD_LOG_MSG ( PDERROR, "Failed to install data node[%s:%s]: %s",
                      pHostName, pSvcName, pErrMsg ) ;
         _updateInstallStatus( retRc, pErrMsg, desc, FALSE, NULL ) ;
         rc = retRc ;
         goto error ;
      }
      else
      {
         InstalledNode node ;
         node._role = ROLE_CATA ;
         node._dataGroupName = "" ;
         node._hostName = pHostName ;
         node._svcName = pSvcName ;
         ossSnprintf( desc, OMA_BUFF_SIZE, "Finish installing coord[%s:%s]",
                      pHostName, pSvcName ) ;
         PD_LOG ( PDDEBUG, "Sucessed to install coord[%s:%s]",
                  pHostName, pSvcName ) ;
         _updateInstallStatus( retRc, pErrMsg, desc, TRUE, &node ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateCoordJob::_updateInstallStatus(
                                                  INT32 retRc,
                                                  const CHAR *pErrMsg,
                                                  const CHAR *pDesc,
                                                  BOOLEAN isFinish,
                                                  InstalledNode *pNode )
   {
      INT32 rc = SDB_OK ;

      rc = _pTask->updateInstallResult( retRc, ROLE_COORD, pErrMsg,
                                        pDesc, NULL, isFinish, pNode ) ;
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
      vector<BSONObj>::iterator itr ;
      vector<BSONObj> dataNodeInstallInfo ;

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
         CHAR buffer [OMA_BUFF_SIZE + 1] = { 0 } ;

         // check task's status and decide to do or stop
         OMA_TASK_STATUS taskStatus = _pTask->status() ;
         if ( OMA_TASK_STATUS_FAIL == taskStatus )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDEVENT, "Stop installing data node in group %s, "
                     "task has failed", _groupname.c_str() ) ;
            goto done ;
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
         ossSnprintf( buffer, OMA_BUFF_SIZE, "Installing data node[%s:%s]",
                      installInfo._hostName.c_str(),
                      installInfo._svcName.c_str() ) ;
         rc = _updateInstallStatus( SDB_OK, NULL, buffer, FALSE, NULL ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to update status before "
                     "install data node, rc = %d", rc ) ;
            goto error ;
         }
         // install data node
         _omaJobRunInstallDataCmd runCmd( installInfo ) ;
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
            PD_LOG( PDERROR, "Job failed to create catalog, rc = %d", rc ) ;
            goto error ;
         }
         // check the install result
         // TODO: retObj contant hostname, port
         // and this function will update install status at the same time
         rc = _checkInstallResult ( installInfo._hostName.c_str(),
                                    installInfo._svcName.c_str(),
                                    retObj ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to check install result, rc = %d", rc ) ;
            goto error ;
         }
         // get next
         itr++ ;
      }

   done:
      return rc ;
   error:
      // set task status to be false
      _pTask->setStatus( OMA_TASK_STATUS_FAIL ) ;
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
/*
      // _confPath
      rc = omaGetStringElement( *it, OMA_OPTION_CONFPATH, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_OPTION_CONFPATH, rc ) ;
      installInfo._confPath = pStr ;
*/
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME << 1 <<
                      OMA_OPTION_DBPATH << 1 ) ;
      conf = obj.filterFieldsUndotted( pattern, false ) ;
      installInfo._conf = conf.getOwned() ; 

   done:
     return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateDataJob::_checkInstallResult( const CHAR *pHostName,
                                                 const CHAR *pSvcName,
                                                 BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      INT32 retRc = SDB_OK ;
      const CHAR *pErrMsg = "" ;
      CHAR desc[OMA_BUFF_SIZE + 1] = { 0 } ;

      // extract return rc
      rc = omaGetIntElement ( obj, OMA_FIELD_RC, retRc ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                  OMA_FIELD_RC, rc ) ;
//         goto error ;
      }
      if ( retRc )
      {
         rc = omaGetStringElement ( obj, OMA_FIELD_DETAIL, &pErrMsg ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Get field[%s] failed, rc = %d",
                     OMA_FIELD_DETAIL, rc ) ;
//          goto error ;
         }
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Failed to install data node[%s:%s]",
                      pHostName, pSvcName ) ;
         PD_LOG_MSG ( PDERROR, "Failed to install data node[%s:%s]: %s",
                      pHostName, pSvcName, pErrMsg ) ;
         _updateInstallStatus( retRc, pErrMsg, desc, FALSE, NULL ) ;
         rc = retRc ;
         goto error ;
      }
      else
      {
         InstalledNode node ;
         node._role = ROLE_CATA ;
         node._dataGroupName = _groupname ;
         node._hostName = pHostName ;
         node._svcName = pSvcName ;
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Finish installing data node[%s:%s]",
                      pHostName, pSvcName ) ;
         PD_LOG ( PDDEBUG, "Sucessed to install data node[%s:%s]",
                  pHostName, pSvcName ) ;
         _updateInstallStatus( retRc, pErrMsg, desc, TRUE, &node ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateDataJob::_updateInstallStatus( INT32 retRc,
                                                  const CHAR *pErrMsg,
                                                  const CHAR *pDesc,
                                                  BOOLEAN isFinish,
                                                  InstalledNode *pNode )
   {
      INT32 rc = SDB_OK ;

      rc = _pTask->updateInstallResult( retRc, ROLE_DATA, pErrMsg, pDesc,
                                        _groupname.c_str(), isFinish, pNode ) ;
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
      omagent remove virtual coord job
   */
   _omaRemoveVirtualCoordJob::_omaRemoveVirtualCoordJob (
                                                    const CHAR *hostName,
                                                    const CHAR *svcName,
                                                    const CHAR *vCoordSvcName )
   {
      _localHostName = hostName ;
      _omaSvcName    = svcName ; 
      _vCoordSvcName = vCoordSvcName ;
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
      return "remove virtual coord job" ;
   }

   BOOLEAN _omaRemoveVirtualCoordJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaRemoveVirtualCoordJob::doit()
   {
      INT32 rc = SDB_OK ;
      BOOLEAN hasVCoordRemove = FALSE ;
      _omaRemoveVirtualCoord removeVCoord( _localHostName.c_str(),
                                           _omaSvcName.c_str(),
                                           _vCoordSvcName.c_str() ) ;
      rc = removeVCoord.removeVirtualCoord ( hasVCoordRemove ) ;
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
         PD_LOG ( PDERROR, "Failed to alloc memory for creating data node job" ) ;
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

   INT32 startRemoveVirtualCoordJob ( const CHAR *hostName,
                                      const CHAR *svcName,
                                      const CHAR *vCoordSvcName,
                                      EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaRemoveVirtualCoordJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaRemoveVirtualCoordJob( hostName, svcName,
                                                    vCoordSvcName ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for remove virtual coord job" ) ;
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
