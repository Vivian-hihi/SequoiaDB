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

   Source File Name = omagentJob.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/30/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_JOB_HPP_
#define OMAGENT_JOB_HPP_
#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "pmd.hpp"
#include "omagent.hpp"
//#include "omagentJobRunCmd.hpp"
#include "omagentTask.hpp"
#include "omagentCommand.hpp"
#include "rtnBackgroundJob.hpp"
#include <string>
#include <vector>

using namespace bson ;

namespace engine
{
   class _omaInstallDBBusinessTask ;

   /*
      create catalog job
   */
   class _omaCreateCatalogJob : public _rtnBaseJob
   {
      public:
         _omaCreateCatalogJob ( _omaInstallDBBusinessTask *pTask ) ;
         virtual ~_omaCreateCatalogJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;

         OMA_JOB_STATUS getJobStatus ()
         {
            return _status ;
         }
         void setJobStatus ( OMA_JOB_STATUS status )
         {
            _status = status ;
            _pTask->setJobStatus( _name, status ) ;
         }
      private:

         INT32 _getInstallInfo( BSONObj &obj, InstallInfo &installInfo ) ;
/*
         INT32 _checkInstallResult( const CHAR *pHostName,
                                    const CHAR *pSvcName,
                                    BSONObj &obj ) ;
*/
         INT32 _updateInstallStatus( BOOLEAN isFinish,
                                     INT32 retRc,
                                     const CHAR *pErrMsg,
                                     const CHAR *pDesc,
                                     InstalledNode *pNode ) ;
      private:
         OMA_JOB_STATUS                      _status ;
         string                              _name ;
         _omaInstallDBBusinessTask*          _pTask ;
   } ;

   /*
      create coord job
   */
   class _omaCreateCoordJob : public _rtnBaseJob
   {
      public:
         _omaCreateCoordJob ( _omaInstallDBBusinessTask *pTask ) ;
         virtual ~_omaCreateCoordJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;

         OMA_JOB_STATUS getJobStatus ()
         {
            return _status ;
         }
         void setJobStatus ( OMA_JOB_STATUS status )
         {
            _status = status ;
            _pTask->setJobStatus( _name, status ) ;
         }
      private:

         INT32 _getInstallInfo( BSONObj &obj, InstallInfo &installInfo ) ;
/*
         INT32 _checkInstallResult( const CHAR *pHostName,
                                    const CHAR *pSvcName,
                                    BSONObj &obj ) ;
*/
         INT32 _updateInstallStatus( BOOLEAN isFinish,
                                     INT32 retRc,
                                     const CHAR *pErrMsg,
                                     const CHAR *pDesc,
                                     InstalledNode *pNode ) ;

      private:
         OMA_JOB_STATUS                      _status ;
         string                              _name ;
         _omaInstallDBBusinessTask*          _pTask ;

   } ;

   /*
      create data job
   */
   class _omaCreateDataJob : public _rtnBaseJob
   {
      public:
         _omaCreateDataJob ( const CHAR *pGroupName,
                             _omaInstallDBBusinessTask *pTask ) ;
         virtual ~_omaCreateDataJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;

         OMA_JOB_STATUS getJobStatus ()
         {
            return _status ;
         }
         void setJobStatus ( OMA_JOB_STATUS status )
         {
            _status = status ;
            _pTask->setJobStatus( _name, status ) ;
         }
      private:

         INT32 _getInstallInfo( BSONObj &obj, InstallInfo &installInfo ) ;
/*
         INT32 _checkInstallResult( const CHAR *pHostName,
                                    const CHAR *pSvcName,
                                    BSONObj &obj ) ;
*/
         INT32 _updateInstallStatus( BOOLEAN isFinish,
                                     INT32 retRc,
                                     const CHAR *pErrMsg,
                                     const CHAR *pDesc,
                                     InstalledNode *pNode ) ;
     private:
         string                              _groupname ;
         OMA_JOB_STATUS                      _status ;
         string                              _name ;
         _omaInstallDBBusinessTask*          _pTask ;
   } ;

   /*
      start install db business task job
   */
   class _omaStartInstallDBBusinessTaskJob : public _rtnBaseJob
   {
      public:
         _omaStartInstallDBBusinessTaskJob ( BSONObj &installInfo ) ;
         virtual ~_omaStartInstallDBBusinessTaskJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;
         INT32                init() ;
/*
      public:

         OMA_JOB_STATUS getJobStatus ()
         {
            return _status ;
         }
         void setJobStatus ( OMA_JOB_STATUS status )
         {
            _status = status ;
         }
*/
      private:
         INT32 _saveVCoordInfo( BSONObj &info ) ;
/*
         // virtual coord info
         CHAR _omaHostName[OSS_MAX_HOSTNAME + 1] ;
         CHAR _omaSvcName[OSS_MAX_SERVICENAME + 1] ;
*/
         CHAR _vCoordSvcName[OSS_MAX_SERVICENAME + 1] ;
         // raw install info
         BSONObj                     _installInfoObj ;
         // install info after category
         vector<BSONObj>             _coord ;
         vector<BSONObj>             _catalog ;
         vector<BSONObj>             _data ;
         vector<BSONObj>             _standalone ;
         // task info
         INT64                       _taskID ;
//         _omaTaskMgr*                _taskMgr ;
         string                      _name ;
   } ;

   /*
      install db business task rollback internal job
   */
   class _omaInstallDBBusinessTaskRollbackJob : public _rtnBaseJob
   {
      public:
         _omaInstallDBBusinessTaskRollbackJob ( string &vCoordSvcName,
                                                _omaInstallDBBusinessTask *pTask ) ;
         virtual ~_omaInstallDBBusinessTaskRollbackJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;

      public:

         OMA_JOB_STATUS getJobStatus ()
         {
            return _status ;
         }
         void setJobStatus ( OMA_JOB_STATUS status )
         {
            _status = status ;
         }
         
      private:
         INT32 _getRollbackInfo ( RollbackInfo &info ) ;
         INT32 _rollbackCoord( string &vCoordSvcName,
                               map< string, vector<InstalledNode> > &info ) ;
         INT32 _rollbackCatalog( string &vCoordSvcName,
                                 map< string, vector<InstalledNode> > &info ) ;
         INT32 _rollbackDataNode( string &vCoordSvcName, 
                                  map< string,vector<InstalledNode> > &info ) ;
         
         string                              _vCoordSvcName ;
         OMA_JOB_STATUS                      _status ;
         string                              _name ;
         _omaInstallDBBusinessTask*          _pTask ;
   } ;

   /*
      create remove virtual coord job
   */
   class _omaRemoveVirtualCoordJob : public _rtnBaseJob
   {
      public:
         _omaRemoveVirtualCoordJob ( const CHAR *vCoordSvcName ) ;
         virtual ~_omaRemoveVirtualCoordJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;

         OMA_JOB_STATUS getJobStatus ()
         {
            return _status ;
         }
         void setJobStatus ( OMA_JOB_STATUS status )
         {
            _status = status ;
         }

      private:
         OMA_JOB_STATUS            _status ;
         string                    _omaHostName ;
         string                    _omaSvcName ;
         string                    _vCoordSvcName ;
   } ;


   // start create catalog job
   INT32 startCreateCatalogJob ( _omaInstallDBBusinessTask *pTask,
                                 EDUID *pEDUID ) ;
   // start create coord job
   INT32 startCreateCoordJob ( _omaInstallDBBusinessTask *pTask,
                               EDUID *pEDUID ) ;
   // start create data job
   INT32 startCreateDataJob ( const CHAR *pGroupName,
                              _omaInstallDBBusinessTask *pTask,
                              EDUID *pEDUID ) ;
   // start install db business task job
   INT32 startStartInstallDBBusinessTaskJob ( const CHAR *pInstallInfo,
                                              EDUID *pEDUID ) ;
   // start install db business task rollback job
   INT32 startInstallDBBusinessTaskRollbackJob (
                                              string &vCoordSvcName,
                                              _omaInstallDBBusinessTask *pTask,
                                              EDUID *pEDUID ) ;
   // start create remove virtual coord job
   INT32 startRemoveVirtualCoordJob ( const CHAR *vCoordSvcName,
                                      EDUID *pEDUID ) ;


}



#endif
