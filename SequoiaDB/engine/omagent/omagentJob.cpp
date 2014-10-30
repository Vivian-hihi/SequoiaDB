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
#include "pmdEDU.hpp"

namespace engine
{

   /*
       omagent create standalone job
   */
   _omaCreateStandaloneJob::_omaCreateStandaloneJob (
                                             _omaInsDBBusTask *pTask )
   {
      _name = OMA_JOB_CREATE_STANDALONE;
      _status = OMA_JOB_STATUS_INIT ;
      _pTask = pTask ;
   }

   _omaCreateStandaloneJob::~_omaCreateStandaloneJob()
   {
   }

   RTN_JOB_TYPE _omaCreateStandaloneJob::type () const
   {
      return RTN_JOB_CREATESTANDALONE ;
   }

   const CHAR* _omaCreateStandaloneJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaCreateStandaloneJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaCreateStandaloneJob::doit()
   {
      INT32 rc = SDB_OK ;
      INT32 tmpRc = SDB_OK ;
      INT32 num = 0 ;
      vector<BSONObj> &standaloneInsInfo = _pTask->getInstallStandaloneInfo() ;
      vector<BSONObj>::iterator itr ;

      // change job status
      setJobStatus( OMA_JOB_STATUS_RUNNING ) ;
      // begin to run  
      num = standaloneInsInfo.size() ;
      if ( 0 == num )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "No install standalone info, rc = %d", rc ) ;
         goto error ;
      }
      itr = standaloneInsInfo.begin() ;
      for ( ; itr != standaloneInsInfo.end(); itr++ )
      {
         BSONObj retObj ;
         InstallInfo installInfo ;
         InstalledNode node ;
         CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
         const CHAR *pErrMsg = "" ;
         // get install standalone information
         rc = _getInstallInfo( *itr, installInfo ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to get install standalone info, "
                     "rc = %d", rc ) ;
            goto error ;
         }
         // init the info of the installing node
         node._role = ROLE_STANDALONE ;
         node._dataGroupName = "" ;
         node._hostName = installInfo._hostName.c_str() ;
         node._svcName = installInfo._svcName.c_str() ;
         // update install status for web before install standalone
         ossSnprintf( desc, OMA_BUFF_SIZE, "Installing standalone[%s:%s]",
                      installInfo._hostName.c_str(),
                      installInfo._svcName.c_str() ) ;
         rc = _updateInstallStatus( FALSE, SDB_OK, NULL, desc, NULL ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to update status before install "
                     "standalone, rc = %d", rc ) ;
            goto error ;
         }
         // install standalone
         _omaRunCreateStandaloneJob runCmd( _pTask->getVCoordSvcName(),
                                            installInfo ) ;
         rc = runCmd.init( NULL ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Job failed to init for creating standalone, "
                    "rc = %d", rc ) ;
            goto error ;
         }
         rc = runCmd.doit( retObj ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Job failed to create standalone, rc = %d", rc ) ;
            tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
            if ( tmpRc )
            {
               PD_LOG ( PDWARNING, "Get field[%s] failed, rc = %d",
                        OMA_FIELD_DETAIL, tmpRc ) ;
            }
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Failed to install standalone[%s:%s]: %s",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str(), pErrMsg ) ;
            PD_LOG_MSG ( PDERROR, desc ) ;
            _updateInstallStatus( FALSE, rc, pErrMsg, desc, &node ) ;
            goto error ;
         }
         else
         {
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Finish installing standalone[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG ( PDDEBUG, "Sucessed to install standalone[%s:%s]",
                     installInfo._hostName.c_str(),
                     installInfo._svcName.c_str() ) ;
            _updateInstallStatus( TRUE, SDB_OK, pErrMsg, desc, &node ) ;
         }
      }
      // set job status to be successful
      setJobStatus( OMA_JOB_STATUS_FINISH ) ;
   done:
      return rc ;
   error:
      // set job status to be failing
      setJobStatus( OMA_JOB_STATUS_FAIL ) ;
      goto done ;
   }

   INT32 _omaCreateStandaloneJob::_getInstallInfo( BSONObj &obj,
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
      // _sshPort
      rc = omaGetStringElement( obj, OMA_FIELD_SSHPORT, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SSHPORT, rc ) ;
      info._sshPort = pStr ;
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME       << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME       << 1 <<
                      OMA_OPTION_DBPATH        << 1 <<
                      OMA_FIELD_SDBUSER        << 1 <<
                      OMA_FIELD_SDBPASSWD      << 1 << 
                      OMA_FIELD_SDBUSERGROUP   << 1 <<
                      OMA_FIELD_USER           << 1 <<
                      OMA_FIELD_PASSWD         << 1 << 
                      OMA_FIELD_SSHPORT << 1 ) ;
      conf = obj.filterFieldsUndotted( pattern, false ) ;
      info._conf = conf.getOwned() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateStandaloneJob::_updateInstallStatus( BOOLEAN isFinish,
                                                        INT32 retRc,
                                                        const CHAR *pErrMsg,
                                                        const CHAR *pDesc,
                                                        InstalledNode *pNode )
   {
      INT32 rc = SDB_OK ;

      rc = _pTask->updateInstallStatus( isFinish, retRc, ROLE_STANDALONE,
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
       omagent create catalog job
   */
   _omaCreateCatalogJob::_omaCreateCatalogJob (
                                             _omaInsDBBusTask *pTask )
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
      INT32 num = 0 ;
      vector<BSONObj> &catalogInstallInfo = _pTask->getInstallCatalogInfo() ;
      vector<BSONObj>::iterator itr ;

      // change job status
      setJobStatus( OMA_JOB_STATUS_RUNNING ) ;
      // begin to run  
      num = catalogInstallInfo.size() ;
      if ( 0 == num )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG ( PDERROR, "No install catalog info, rc = %d", rc ) ;
         goto error ;
      }
      itr = catalogInstallInfo.begin() ;
      for ( ; itr != catalogInstallInfo.end(); itr++ )
      {
         BSONObj retObj ;
         InstallInfo installInfo ;
         CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
         const CHAR *pErrMsg = "" ;
         // get install catalog information
         rc = _getInstallInfo( *itr, installInfo ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to get install catalog info, "
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
         _omaRunCreateCatalogJob runCmd( _pTask->getVCoordSvcName(),
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
            PD_LOG( PDERROR, "Job failed to create catalog, rc = %d", rc ) ;
            // if we can't get field "detail", it means we failed in CPP,
            // we had not executed js file yet
            tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
            if ( tmpRc )
            {
               pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
               if ( NULL == pErrMsg )
               {
                  pErrMsg = "" ;
               }
            }
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Failed to install catalog[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG_MSG ( PDERROR, "%s: %s", desc, pErrMsg ) ;
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
            PD_LOG ( PDEVENT, "Sucessed to install catalog[%s:%s]",
                     installInfo._hostName.c_str(),
                     installInfo._svcName.c_str() ) ;
            _updateInstallStatus( TRUE, SDB_OK, pErrMsg, desc, &node ) ;
         }
      }
      // set job status to be successful
      setJobStatus( OMA_JOB_STATUS_FINISH ) ;
   done:
      return rc ;
   error:
      // set job status to be failing
      setJobStatus( OMA_JOB_STATUS_FAIL ) ;
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
      // _sshPort
      rc = omaGetStringElement( obj, OMA_FIELD_SSHPORT, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SSHPORT, rc ) ;
      info._sshPort = pStr ;
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME       << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME       << 1 <<
                      OMA_OPTION_DBPATH        << 1 <<
                      OMA_FIELD_SDBUSER        << 1 <<
                      OMA_FIELD_SDBPASSWD      << 1 << 
                      OMA_FIELD_SDBUSERGROUP   << 1 <<
                      OMA_FIELD_USER           << 1 <<
                      OMA_FIELD_PASSWD         << 1 <<
                      OMA_FIELD_SSHPORT        << 1 ) ;
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
         PD_LOG_MSG ( PDERROR, "Failed to update install catalog information, "
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
   _omaCreateCoordJob::_omaCreateCoordJob ( _omaInsDBBusTask *pTask )
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
      INT32 num = 0 ;
      vector<BSONObj> &coordInstallInfo = _pTask->getInstallCoordInfo() ;
      vector<BSONObj>::iterator itr ;

      // change job status
      setJobStatus( OMA_JOB_STATUS_RUNNING ) ;
      // check 
      num = coordInstallInfo.size() ;
      if ( 0 == num )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG ( PDERROR, "No install catalog info, rc = %d", rc ) ;
         goto error ;
      }
      itr = coordInstallInfo.begin() ;
      for ( ; itr != coordInstallInfo.end(); itr++ )
      {
         BSONObj retObj ;
         InstallInfo installInfo ;
         CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
         const CHAR *pErrMsg = "" ;

         // check install status and decide to go on or stop
         if ( _pTask->getIsInstallFail() )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDWARNING, "Stop installing coord, install has failed" ) ;
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
         _omaRunCreateCoordJob runCmd( _pTask->getVCoordSvcName(),
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
            PD_LOG( PDERROR, "Job failed to create coord, rc = %d", rc ) ;
            // if we can't get field "detail", it means we failed in CPP,
            // we had not executed js file yet
            tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
            if ( tmpRc )
            {
               pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
               if ( NULL == pErrMsg )
               {
                  pErrMsg = "" ;
               }
            }
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Failed to install coord[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG_MSG ( PDERROR, "%s: %s", desc, pErrMsg ) ;
            _updateInstallStatus( FALSE, rc, pErrMsg, desc, NULL ) ;
            goto error ;
         }
         else
         {
            InstalledNode node ;
            node._role = ROLE_COORD ;
            node._dataGroupName = "" ;
            node._hostName = installInfo._hostName.c_str() ;
            node._svcName = installInfo._svcName.c_str() ;
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Finish installing coord[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG ( PDEVENT, "Sucessed to install coord[%s:%s]",
                     installInfo._hostName.c_str(),
                     installInfo._svcName.c_str() ) ;
            _updateInstallStatus( TRUE, SDB_OK, pErrMsg, desc, &node ) ;
         }
      }
      // set job to be finish
      setJobStatus( OMA_JOB_STATUS_FINISH ) ;
   done:
      return rc ;
   error:
      // set job status to be failing
      setJobStatus( OMA_JOB_STATUS_FAIL ) ;
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
      // _sshPort
      rc = omaGetStringElement( obj, OMA_FIELD_SSHPORT, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SSHPORT, rc ) ;
      info._sshPort = pStr ;
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME       << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME       << 1 <<
                      OMA_OPTION_DBPATH        << 1 <<
                      OMA_FIELD_SDBUSER        << 1 <<
                      OMA_FIELD_SDBPASSWD      << 1 <<
                      OMA_FIELD_SDBUSERGROUP   << 1 <<
                      OMA_FIELD_USER           << 1 <<
                      OMA_FIELD_PASSWD         << 1 <<
                      OMA_FIELD_SSHPORT        << 1 ) ;
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
                                          _omaInsDBBusTask *pTask )
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
      INT32 num = 0 ;
      vector<BSONObj>::iterator itr ;
      vector<BSONObj> dataNodeInstallInfo ;

      // change job status
      setJobStatus( OMA_JOB_STATUS_RUNNING ) ;
      // get install data group information 
      rc = _pTask->getInstallDataGroupInfo( _groupname,
                                            dataNodeInstallInfo ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to get install data node information "
                      "in group %s, rc = %d", _groupname.c_str(), rc ) ;
         goto error ;
      }
      // check the install info
      num = dataNodeInstallInfo.size() ;
      if ( 0 == num )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG ( PDERROR, "No install data node info in group %s, rc = %d",
                      _groupname.c_str(), rc ) ;
         goto error ;
      }
      itr = dataNodeInstallInfo.begin() ;
      for ( ; itr != dataNodeInstallInfo.end(); itr++ )
      {
         BSONObj retObj ;
         InstallInfo installInfo ;
         CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
         const CHAR *pErrMsg = "" ;

         // check install's status and decide to do or stop
         if ( _pTask->getIsInstallFail() )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDWARNING, "Stop installing data node in group [%s], "
                     "install had failed", _groupname.c_str() ) ;
            goto error ;
         }
         // get installl data node information
         rc = _getInstallInfo( *itr, installInfo ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Failed to get install data node info, "
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
         _omaRunCreateDataNodeJob runCmd( _pTask->getVCoordSvcName(),
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
            PD_LOG( PDERROR, "Job failed to create data node, rc = %d", rc ) ;
            // if we can't get field "detail", it means we failed in CPP,
            // we had not executed js file yet
            tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
            if ( tmpRc )
            {
               pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
               if ( NULL == pErrMsg )
               {
                  pErrMsg = "" ;
               }
            }
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Failed to install data node[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG_MSG ( PDERROR, "%s: %s", desc, pErrMsg ) ;
            _updateInstallStatus( FALSE, rc, pErrMsg, desc, NULL ) ;
            goto error ;
         }
         else
         {
            InstalledNode node ;
            node._role = ROLE_DATA ;
            node._dataGroupName = _groupname ;
            node._hostName = installInfo._hostName.c_str() ;
            node._svcName = installInfo._svcName.c_str() ;
            ossSnprintf( desc, OMA_BUFF_SIZE,
                         "Finish installing data node[%s:%s]",
                         installInfo._hostName.c_str(),
                         installInfo._svcName.c_str() ) ;
            PD_LOG ( PDEVENT, "Sucessed to install data node[%s:%s]",
                     installInfo._hostName.c_str(),
                     installInfo._svcName.c_str() ) ;
            _updateInstallStatus( TRUE, SDB_OK, pErrMsg, desc, &node ) ;
         }
      }
      // set job status to be successful
      setJobStatus( OMA_JOB_STATUS_FINISH ) ;
   done:
      return rc ;
   error:
      // set job status to be failing
      setJobStatus( OMA_JOB_STATUS_FAIL ) ;
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
      // _sshPort
      rc = omaGetStringElement( obj, OMA_FIELD_SSHPORT, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Get field[%s] failed, rc: %d", OMA_FIELD_SSHPORT, rc ) ;
      installInfo._sshPort = pStr ;
      // _conf
      pattern = BSON( OMA_FIELD_HOSTNAME       << 1 <<
                      OMA_OPTION_DATAGROUPNAME << 1 <<
                      OMA_OPTION_SVCNAME       << 1 <<
                      OMA_OPTION_DBPATH        << 1 <<
                      OMA_FIELD_SDBUSER        << 1 <<
                      OMA_FIELD_SDBPASSWD      << 1 <<
                      OMA_FIELD_SDBUSERGROUP   << 1 <<
                      OMA_FIELD_USER           << 1 <<
                      OMA_FIELD_PASSWD         << 1 <<
                      OMA_FIELD_SSHPORT        << 1 ) ;
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
   _omaStartInsDBBusTaskJob::_omaStartInsDBBusTaskJob ( BSONObj &installInfo )
   {
      _isStandalone = FALSE ;
      _installInfoObj = installInfo.getOwned() ;
      _name = OMA_JOB_START_INSTALL_DB_BUSINESS ;
      _pTask = NULL ;
   }

   _omaStartInsDBBusTaskJob::~_omaStartInsDBBusTaskJob()
   {
   }

   RTN_JOB_TYPE _omaStartInsDBBusTaskJob::type () const
   {
      return RTN_JOB_STARTINSDBBUSTASK ;
   }

   const CHAR* _omaStartInsDBBusTaskJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaStartInsDBBusTaskJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaStartInsDBBusTaskJob::init()
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      BSONObj filter ;
      BSONObj commonFileds ;
      BSONObjBuilder builder ;
      string deplayMod ;
      const CHAR *pStr = NULL ;

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
      // get deployMod info from omsvc
      ele = _installInfoObj.getField( OMA_FIELD_DEPLOYMOD ) ;
      if ( String != ele.type() )
      {
         PD_LOG_MSG ( PDERROR, "Receive invalid content from omsvc" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      deplayMod = ele.String() ;
      if ( deplayMod == string(DEPLAY_SA) )
      {
         _isStandalone = TRUE ;
      }
      else if ( deplayMod == string(DEPLAY_DB) )
      {
         _isStandalone = FALSE ;
      }
      else
      {
         PD_LOG_MSG ( PDERROR, "Receive invalid deplay mode from omsvc" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
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
               PD_LOG_MSG( PDERROR, "Unknown role for install db business" ) ;
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaStartInsDBBusTaskJob::init2()
   {
      INT32 rc                         = SDB_OK ;
      _omaTaskMgr *pTaskMgr            = getTaskMgr() ;
      BSONObj otherInfo ;
      
      // remove last task with the same name
      rc = pTaskMgr->removeTask ( OMA_TASK_NAME_INSTALL_DB_BUSINESS ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to remove task[%s], "
                     "rc = %d", OMA_TASK_NAME_INSTALL_DB_BUSINESS, rc ) ;
         goto error ;
      }
      // create install db business task
      _pTask = SDB_OSS_NEW _omaInsDBBusTask( _taskID ) ;
      if ( !_pTask )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG( PDERROR, "Failed to create install db business task, "
                     "rc = %d", rc ) ;
         goto error ;
      }
      // register install db business task
      pTaskMgr->addTask( _pTask, _taskID ) ;
      rc = _pTask->init( _isStandalone, _standalone, _coord,
                         _catalog, _data, otherInfo ) ;
      if ( rc  )
      {
         PD_LOG_MSG( PDERROR, "Failed to init install db busniness task, "
                     "rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaStartInsDBBusTaskJob::doit()
   {
      INT32 rc = SDB_OK ;
      rc = _pTask->doit() ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to do install db busniness task, "
                     "rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      remove db business task rollback job
   */
   _omaStartRmDBBusTaskJob::_omaStartRmDBBusTaskJob ( BSONObj &uninstallInfo )
   {
      _isStandalone = FALSE ;
      _uninstallInfoObj = uninstallInfo.getOwned() ;
      _name = OMA_JOB_START_REMOVE_DB_BUSINESS ;
      _pTask = NULL ;
   }

   _omaStartRmDBBusTaskJob::~_omaStartRmDBBusTaskJob()
   {
   }

   RTN_JOB_TYPE _omaStartRmDBBusTaskJob::type () const
   {
      return RTN_JOB_STARTRMDBBUSTASK ;
   }

   const CHAR* _omaStartRmDBBusTaskJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaStartRmDBBusTaskJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaStartRmDBBusTaskJob::init()
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      BSONObj filter ;
      BSONObj commonFileds ;
      BSONObjBuilder builder ;
      string deplayMod ;
      const CHAR *pStr = NULL ;

      // parse arguments
      PD_LOG ( PDDEBUG, "Remove db business passes argument: %s",
               _uninstallInfoObj.toString( FALSE, TRUE ).c_str() ) ;
      // get taskID from omsvc
      ele = _uninstallInfoObj.getField( OMA_FIELD_TASKID ) ;
      if ( NumberInt != ele.type() && NumberLong != ele.type() )
      {
         PD_LOG_MSG ( PDERROR, "Receive invalid task id from omsvc" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      _taskID = ele.numberLong() ;
      // get deployMod info from omsvc
      ele = _uninstallInfoObj.getField( OMA_FIELD_DEPLOYMOD ) ;
      if ( String != ele.type() )
      {
         PD_LOG_MSG ( PDERROR, "Receive invalid content from omsvc" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      deplayMod = ele.String() ;
      if ( deplayMod == string(DEPLAY_SA) )
      {
         _isStandalone = TRUE ;
      }
      else if ( deplayMod == string(DEPLAY_DB) )
      {
         _isStandalone = FALSE ;
      }
      else
      {
         PD_LOG_MSG ( PDERROR, "Receive invalid deplay mode from omsvc" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // get common fields
      rc = omaGetStringElement ( _uninstallInfoObj, OMA_FIELD_AUTHUSER, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_FIELD_AUTHUSER, rc ) ;
      builder.append( OMA_FIELD_AUTHUSER, pStr ) ;
      rc = omaGetStringElement ( _uninstallInfoObj, OMA_FIELD_AUTHPASSWD, &pStr ) ;
      PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_FIELD_AUTHPASSWD, rc ) ;
      builder.append( OMA_FIELD_AUTHPASSWD, pStr ) ;
      commonFileds = builder.obj() ;
      // parse bson and get arguments info for js file
      ele = _uninstallInfoObj.getField ( OMA_FIELD_CONFIG ) ;
      if ( Array != ele.type() )
      {
         PD_LOG_MSG ( PDERROR, "Receive wrong format install "
                      "info from omsvc" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         BSONObjBuilder cataInfoBuilder ;
         BSONArrayBuilder bab ;
         BSONObjIterator itr( ele.embeddedObject() ) ;
         while ( itr.more() )
         {
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
               BSONObjBuilder bob ;
               BSONObj obj ;
               bob.appendElements( commonFileds ) ;
               rc = omaGetStringElement ( temp, OMA_OPTION_DATAGROUPNAME,
                                          &value ) ;
               PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_OPTION_DATAGROUPNAME, rc ) ;
               bob.append( OMA_FIELD_UNINSTALLGROUPNAME, value ) ;
               obj = bob.obj() ;
               _data.insert( pair<string, BSONObj>( string(value), obj ) ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_COORD,
                                       ossStrlen( ROLE_COORD ) ) )
            {
               _coord.insert( pair<string, BSONObj>( string(ROLE_COORD),
                                                     commonFileds ) ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_CATA,
                                       ossStrlen( ROLE_CATA ) ) )
            {
               BSONObjBuilder bob ;
               rc = omaGetStringElement ( temp, OMA_FIELD_HOSTNAME, &pStr ) ;
               PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_FIELD_HOSTNAME, rc ) ;
               bob.append( OMA_FIELD_HOSTNAME, pStr ) ;
               rc = omaGetStringElement ( temp, OMA_OPTION_CATANAME, &pStr ) ;
               PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_OPTION_CATANAME, rc ) ;
               bob.append( OMA_FIELD_CATASVCNAME, pStr ) ;
               bab.append( bob.obj() ) ;
               _catalog.insert( pair<string, BSONObj>( string(ROLE_CATA),
                                                       commonFileds ) ) ;
            }
            else if ( 0 == ossStrncmp( value, ROLE_STANDALONE,
                                       ossStrlen( ROLE_STANDALONE ) ) )
            {
               BSONObjBuilder bob ;
               rc = omaGetStringElement ( temp, OMA_FIELD_HOSTNAME, &pStr ) ;
               PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_FIELD_HOSTNAME, rc ) ;
               bob.append( OMA_FIELD_HOSTNAME, pStr ) ;
               rc = omaGetStringElement ( temp, OMA_FIELD_SVCNAME, &pStr ) ;
               PD_CHECK( SDB_OK == rc, rc, error, PDERROR, "Get field[%s] failed, "
                "rc: %d", OMA_FIELD_SVCNAME, rc ) ;
               bob.append( OMA_FIELD_SVCNAME, pStr ) ;
               _standalone.insert( 
                  pair<string, BSONObj>( string(ROLE_STANDALONE),
                                         bob.obj() ) ) ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG( PDERROR, "Unknown role for remove db business" ) ;
               goto error ;
            }
         }
         cataInfoBuilder.appendArray( OMA_FIELD_CATAADDR, bab.arr() ) ;
         _cataAddrInfo = cataInfoBuilder.obj() ;
      }

   done:
      return rc ;
   error :
      goto done ;
   }

   INT32 _omaStartRmDBBusTaskJob::init2()
   {
      INT32 rc                         = SDB_OK ;
      _omaTaskMgr *pTaskMgr            = getTaskMgr() ;
      
      // remove last task with the same name
      rc = pTaskMgr->removeTask ( OMA_TASK_NAME_REMOVE_DB_BUSINESS ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to remove task[%s], "
                     "rc = %d", OMA_TASK_NAME_REMOVE_DB_BUSINESS, rc ) ;
         goto error ;
      }
      // create remove db business task
      _pTask = SDB_OSS_NEW _omaRmDBBusTask( _taskID ) ;
      if ( !_pTask )
      {
         rc = SDB_OOM ;
         PD_LOG_MSG( PDERROR, "Failed to create remove db business task, "
                     "rc = %d", rc ) ;
         goto error ;
      }
      // register remove db business task
      pTaskMgr->addTask( _pTask, _taskID ) ;
      rc = _pTask->init( _isStandalone, _standalone, _coord,
                         _catalog, _data, _cataAddrInfo ) ;
      if ( rc  )
      {
         PD_LOG_MSG( PDERROR, "Failed to init remove db busniness task, "
                     "rc = %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaStartRmDBBusTaskJob::doit()
   {
      INT32 rc = SDB_OK ;
      rc = _pTask->doit() ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "Failed to do remove db busniness task, "
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
   _omaInsDBBusTaskRbJob::_omaInsDBBusTaskRbJob (
                                              BOOLEAN isStandalone,
                                              string &vCoordSvcName,
                                              _omaInsDBBusTask *pTask )
   {
      _name = OMA_JOB_ROLLBACK_INSTALL_DB_BUSINESS ;
      _isStandalone = isStandalone ;
      _vCoordSvcName = vCoordSvcName ;
      _pTask = pTask ;
   }

   _omaInsDBBusTaskRbJob::~_omaInsDBBusTaskRbJob()
   {
   }

   RTN_JOB_TYPE _omaInsDBBusTaskRbJob::type () const
   {
      return RTN_JOB_INSDBBUSTASKRB ;
   }

   const CHAR* _omaInsDBBusTaskRbJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaInsDBBusTaskRbJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaInsDBBusTaskRbJob::doit()
   {
      INT32 rc = SDB_OK ;
      const CHAR *pErrMsg = NULL ;
      RollbackInfo info ;
      BSONObj retObj ;

      // set task stage
      _pTask->setTaskStage( OMA_INSTALL_ROLLBACK ) ;
      // get rollback info
      rc = _getRollbackInfo ( info ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to get rollback info, rc = %d", rc ) ;
         goto error ;
      }
      if ( _isStandalone )
      {
         // rollback standalone
         rc = _rollbackStandalone( _vCoordSvcName,
                                   info._standaloneRollbackInfo ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to rollback standalone, rc = %d", rc ) ;
            goto error ;
         }
      }
      else
      {
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
      }
      // set rollback finish
      _pTask->setIsRollbackFinish( TRUE ) ;
 
   done:
      // for standalone, when doing rollback, if finish rollback, it means add  
      // business task finish, if rollback failed, it means add 
      // business task failed
      if ( _isStandalone )
      {
         // set task finish or fail
         if ( _pTask->getIsRollbackFinish() )
         {
            _pTask->setIsTaskFinish( TRUE ) ;
         }
         else if ( _pTask->getIsRollbackFail() )
         {
            _pTask->setIsTaskFail( TRUE ) ;
         }
         else
         {
            PD_LOG ( PDERROR, "Task[%s] in a unknown status",
                     _pTask->taskName() ) ;
#if defined (_DEBUG)
            ossPanic() ;
#endif
            rc = SDB_OMA_TASK_FAIL ;
            goto error ;
         }
      }
      // for cluster, when doing rollback, if succeed, we need to
      // remove virtual coord, and we judge whether task is succeeful or not,
      // but if fail, we don't remove virtual coord, and leave it for
      // cleaning the environment manually, but we need to mark task to be
      // failing
      else
      {
         if ( _pTask->getIsRollbackFinish() )
         {
            _pTask->removeVirtualCoord() ;
         }
         else if ( _pTask->getIsRollbackFail() )
         {
            _pTask->setIsTaskFail( TRUE ) ;
         }
      }
      return rc ;
   error:
      // set rollback failing
      _pTask->setIsRollbackFail( TRUE ) ;
      // set the error reason
      pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
      if ( NULL == pErrMsg )
      {
         _pTask->setErrDetail( "Failed to rollback in add db business task, "
                               "please do it manually" ) ;
      }
      else
      {
         _pTask->setErrDetail ( pErrMsg ) ;
      }
      goto done ;
   }

   INT32 _omaInsDBBusTaskRbJob::_getRollbackInfo( RollbackInfo &info )
   {
      INT32 rc = SDB_OK ;
      rc = _pTask->getInstalledNodeResult ( ROLE_STANDALONE,
                                            info._standaloneRollbackInfo ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get standalone rollback info, "
                  "rc = %d", rc ) ;
         goto error ;
      }
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

   INT32 _omaInsDBBusTaskRbJob::_rollbackStandalone (
                                  string &vCoordSvcName,
                                  map< string, vector< InstalledNode> > &info )
   {
      INT32 rc = SDB_OK ;
      INT32 tmpRc = SDB_OK ;
      BSONObj retObj ;
      CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
      const CHAR *pErrMsg = NULL ;
      _omaRunRollbackStandaloneJob rollbackStandalone ( vCoordSvcName, info ) ;
      map< string, vector< InstalledNode> >::iterator it = info.begin() ;
      
      if ( ( 1 != info.size() ) && ( string( ROLE_STANDALONE) != it->first ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG ( PDERROR, "Invalid standalone's rollback info" ) ;
         goto error ;
      }
      // rollback standalone
      rc = rollbackStandalone.init( NULL ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to init to rollback create standalone "
                      "rc = %d", rc ) ;
         goto error ;
      }
      rc = rollbackStandalone.doit( retObj ) ;
      if ( rc )
      {
         // if we can't get field "detail", it means we failed in CPP,
         // we had not executed js file yet
         tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
         if ( tmpRc )
         {
            pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
            if ( NULL == pErrMsg )
            {
               pErrMsg = "" ;
            }
         }
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Failed to rollback standalone" ) ;
         PD_LOG_MSG ( PDERROR, "%s: %s", desc, pErrMsg ) ;         
         goto error ;
      }
      PD_LOG ( PDEVENT, "The rollback standalone's result is: %s",
               retObj.toString(FALSE, TRUE).c_str() ) ;

   done:
      return rc ;
   error:
      goto done ; 
   }

   INT32 _omaInsDBBusTaskRbJob::_rollbackCoord (
                                  string &vCoordSvcName,
                                  map< string, vector< InstalledNode> > &info )
   {
      INT32 rc = SDB_OK ;
      INT32 tmpRc = SDB_OK ;
      CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
      const CHAR *pErrMsg = NULL ;
      BSONObj retObj ;
      _omaRunRollbackCoordJob rollbackCoord ( vCoordSvcName, info ) ;
      map< string, vector< InstalledNode> >::iterator it = info.begin() ;
      
      if ( ( 1 != info.size() ) && ( string( ROLE_COORD ) != it->first ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG ( PDERROR, "Invalid coord's rollback info" ) ;
         goto error ;
      }
      // rollback coord
      rc = rollbackCoord.init( NULL ) ;
      if ( rc )
      {
         PD_LOG_MSG ( PDERROR, "Failed to init to do rollback create coord "
                      "rc = %d", rc ) ;
         goto error ;
      }
      rc = rollbackCoord.doit( retObj ) ;
      if ( rc )
      {
         // if we can't get field "detail", it means we failed in CPP,
         // we had not executed js file yet
         tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
         if ( tmpRc )
         {
            pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
            if ( NULL == pErrMsg )
            {
               pErrMsg = "" ;
            }
         }
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Failed to rollback coord" ) ;
         PD_LOG_MSG ( PDERROR, "%s: %s", desc, pErrMsg ) ;   
         goto error ;
      }
      PD_LOG ( PDEVENT, "The rollback coord's result is: %s",
               retObj.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ; 
   }

   INT32 _omaInsDBBusTaskRbJob::_rollbackCatalog (
                                  string &vCoordSvcName,
                                  map< string, vector< InstalledNode> > &info )
   {
      INT32 rc = SDB_OK ;
      INT32 tmpRc = SDB_OK ;
      CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
      const CHAR *pErrMsg = NULL ;
      BSONObj retObj ;
      _omaRunRollbackCatalogJob rollbackCatalog ( vCoordSvcName, info ) ;
      map< string, vector< InstalledNode> >::iterator it = info.begin() ;

      if ( ( 1 != info.size() ) && ( string( ROLE_CATA ) != it->first ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG ( PDERROR, "Invalid catalog's rollback info" ) ;
         goto error ;
      }
      // rollback catalog
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
         // if we can't get field "detail", it means we failed in CPP,
         // we had not executed js file yet
         tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
         if ( tmpRc )
         {
            pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
            if ( NULL == pErrMsg )
            {
               pErrMsg = "" ;
            }
         }
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Failed to rollback catalog" ) ;
         PD_LOG_MSG ( PDERROR, "%s: %s", desc, pErrMsg ) ;     
         goto error ;
      }
      PD_LOG ( PDEVENT, "The rollback catalog's result is: %s",
               retObj.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaInsDBBusTaskRbJob::_rollbackDataNode (
                                  string &vCoordSvcName,
                                  map< string, vector< InstalledNode> > &info )
   {
      INT32 rc = SDB_OK ;
      INT32 tmpRc = SDB_OK ;
      CHAR desc [OMA_BUFF_SIZE + 1] = { 0 } ;
      const CHAR *pErrMsg = NULL ;
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
         // if we can't get field "detail", it means we failed in CPP,
         // we had not executed js file yet
         tmpRc = omaGetStringElement ( retObj, OMA_FIELD_DETAIL, &pErrMsg ) ;
         if ( tmpRc )
         {
            pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
            if ( NULL == pErrMsg )
            {
               pErrMsg = "" ;
            }
         }
         ossSnprintf( desc, OMA_BUFF_SIZE,
                      "Failed to rollback data groups" ) ;
         PD_LOG_MSG ( PDERROR, "%s: %s", desc, pErrMsg ) ;   
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
                                                    const CHAR *vCoordSvcName,
                                              _omaInsDBBusTask *pTask )
   {
      _vCoordSvcName  = vCoordSvcName ;
      _pTask = pTask ;
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
      INT32 tmpRc = SDB_OK ;
      BSONObj removeRet ;
      const CHAR *pErrMsg = NULL ;
      string str ;
      _omaRemoveVirtualCoord removeVCoord( _vCoordSvcName.c_str() ) ;
      rc = removeVCoord.removeVirtualCoord ( removeRet ) ;
      if ( rc )
      {
         // if we can't get field "detail", it means we failed in CPP,
         // we had not executed js file yet
         tmpRc = omaGetStringElement( removeRet, OMA_FIELD_DETAIL, &pErrMsg ) ;
         if ( rc )
         {
            pErrMsg = pmdGetThreadEDUCB()->getInfo( EDU_INFO_ERROR ) ;
            if ( NULL == pErrMsg )
            {
               pErrMsg = "" ;
            }
         }
         _pTask->setIsRemoveVCoordFail( TRUE ) ;
         str += "Failed to remove temporary coord: " ;
         str += pErrMsg ;
         _pTask->setErrDetail( str.c_str() ) ;
         PD_LOG_MSG ( PDERROR, "%s", str.c_str() ) ;
         goto error ;
      }
      else
      {
         PD_LOG ( PDEVENT, "Succeed to remove temporary coord" ) ;
         _pTask->setIsRemoveVCoordFinish( TRUE ) ;
      } 
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateStandaloneJob ( _omaInsDBBusTask *pTask,
                                    EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaCreateStandaloneJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaCreateStandaloneJob( pTask ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for creating "
                  "standalone job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start job, rc = %d", rc ) ;
         goto done ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateCatalogJob ( _omaInsDBBusTask *pTask,
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
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start job, rc = %d", rc ) ;
         goto done ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateCoordJob ( _omaInsDBBusTask *pTask,
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
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start job, rc = %d", rc ) ;
         goto done ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateDataJob ( const CHAR *pGroupName,
                              _omaInsDBBusTask *pTask,
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
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start job, rc = %d", rc ) ;
         goto done ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startInsDBBusTaskJob ( const CHAR *pInstallInfo, EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaStartInsDBBusTaskJob *pJob = NULL ;

      try
      {
         BSONObj info( pInstallInfo ) ;
         pJob = SDB_OSS_NEW _omaStartInsDBBusTaskJob( info ) ;
         if ( !pJob )
         {
            PD_LOG ( PDERROR, "Failed to alloc memory for starting "
                     "install db business job" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         rc = pJob->init() ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failded to init install db business task, "
                     "rc = %d", rc) ;
            goto error ;
         }
         rc = pJob->init2() ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failded to init install db business task, "
                     "rc = %d", rc) ;
            goto error ;
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Occur error, exception is: %s", e.what() ) ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start job, rc = %d", rc ) ;
         goto done ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startRmDBBusTaskJob ( const CHAR *pUninstallInfo, EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaStartRmDBBusTaskJob *pJob = NULL ;

      try
      {
         BSONObj info( pUninstallInfo ) ;
         pJob = SDB_OSS_NEW _omaStartRmDBBusTaskJob( info ) ;
         if ( !pJob )
         {
            PD_LOG ( PDERROR, "Failed to alloc memory for starting "
                     "install db business job" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         rc = pJob->init() ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failded to init install db business task, "
                     "rc = %d", rc) ;
            goto error ;
         }
         rc = pJob->init2() ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failded to init install db business task, "
                     "rc = %d", rc) ;
            goto error ;
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Occur error, exception is: %s", e.what() ) ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start job, rc = %d", rc ) ;
         goto done ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startInsDBBusTaskRbJob ( BOOLEAN isStandalone,
                                  string &vCoordSvcName,
                                  _omaInsDBBusTask *pTask,
                                  EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaInsDBBusTaskRbJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaInsDBBusTaskRbJob( isStandalone, 
                                                vCoordSvcName, pTask ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for "
                  "installing db business rollback job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start job, rc = %d", rc ) ;
         goto done ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   // TODO: let remove virtual coord to be a function
   // instead of to be an asyn job
   INT32 startRemoveVirtualCoordJob ( const CHAR *vCoordSvcName,
                                      _omaInsDBBusTask *pTask,
                                      EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaRemoveVirtualCoordJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaRemoveVirtualCoordJob( vCoordSvcName,
                                                    pTask ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for "
                  "removing temporary coord job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to start job, rc = %d", rc ) ;
         goto done ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

}
