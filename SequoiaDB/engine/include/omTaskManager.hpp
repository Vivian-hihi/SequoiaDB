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

   Source File Name = omTaskManager.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/12/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_TASKMANAGER_HPP_
#define OM_TASKMANAGER_HPP_

#include "rtnCB.hpp"
#include "pmd.hpp"
#include "dmsCB.hpp"
#include "omManager.hpp"
#include <map>
#include <string>

#include <vector>
#include <string>
#include <map>

using namespace std ;
using namespace bson ;

namespace engine
{
   class omTaskManager ;

   class omTaskBase : public SDBObject
   {
      public:
         omTaskBase( omManager *om ) ;
         virtual ~omTaskBase() ;

      public:
         virtual INT32     cancel() = 0 ;

         virtual INT32     enable() = 0 ;

         virtual INT32     getProgress( BSONObj &progress ) = 0 ;

         virtual string    getType() = 0 ;

         virtual UINT64    getTaskID() = 0 ;

         virtual string    getStatus() = 0 ;

         virtual INT32     updateProgress() = 0 ;

         virtual BOOLEAN   isEnable() = 0 ;

      protected:
         INT32             _receiveFromAgent( pmdRemoteSession *remoteSession,
                                              SINT32 &flag, BSONObj &result ) ;
         INT32             _sendMsgToAgent( string host, string port,
                                            pmdRemoteSession *remoteSession, 
                                            MsgHeader *pMsg ) ;
         void              _clearSession( _pmdEDUCB *cb, 
                                          pmdRemoteSession *remoteSession ) ;

      protected:
         omManager         *_om ;
   };

   class omInstallTask : public omTaskBase
   {
      public:
         omInstallTask( omManager *om ) ;
         virtual ~omInstallTask() ;

      public:
         // create a new task, insert into table OM_CS_DEPLOY_CL_TASKINFO
         INT32             init( const string &agentHost, 
                                 const string &agentService,
                                 BSONObj &conf, UINT64 taskID ) ;

         INT32             restore( BSONObj &record ) ;

      public:
         virtual INT32     cancel() ;

         virtual INT32     enable() ;

         virtual INT32     getProgress( BSONObj &progress ) ;

         virtual string    getType() ;

         virtual UINT64    getTaskID() ;

         virtual string    getStatus() ;

         virtual INT32     updateProgress() ;

         virtual BOOLEAN   isEnable() ;

      protected:

         INT32             _getProgressFromAgent( BSONObj &response ) ;
         INT32             _storeBusinessInfo() ;
         INT32             _isHostConfExist( string hostName, 
                                             string businessName ) ;
         INT32             _appendConfigure( string hostName, 
                                             string businessName,
                                             BSONObj &oneNode ) ;
         INT32             _insertConfigure( string hostName, 
                                             string businessName ,
                                             BSONObj &oneNode ) ;
         INT32             _storeConfigInfo() ;
         INT32             _storeProgressToTable() ;
         INT32             _finishTask() ;

      private:
         ossSpinSLatch     _lock ;
         bool              _isEnable ;
         bool              _isFinished ;
         UINT64            _taskID ;
         string            _taskType ;
         string            _taskStatus ;
         string            _agentHost ;
         string            _agentService ;
         BSONObj           _progress ;
         BSONObj           _conf ;
   } ;


   class omTaskManager : public SDBObject
   {
      public:
         omTaskManager( omManager *om ) ;
         ~omTaskManager() ;

      public:
         INT32             createInstallTask( const string &agentHost, 
                                              const string &agentService, 
                                              BSONObj &confValue, 
                                              UINT64 &taskID ) ;
         INT32             restoreTask() ;

         INT32             cancelTask( UINT64 taskID ) ;

         INT32             enableTask( UINT64 taskID ) ;

         INT32             getProgress( UINT64 taskID, BSONObj &progress ) ;

         INT32             run() ;

      private:
         BOOLEAN           _isTaskExist( string taskType, UINT64 &taskID ) ;
         UINT64            _generateTaskID() ;
         INT32             _restoreInstallTask( BSONObj &record ) ;
         void              _addTaskToMap( omTaskBase *task ) ;

      private:
         omManager                *_om ;
         ossSpinSLatch            _lock ;
         map< UINT64, shared_ptr<omTaskBase> > _mapTasks ;
         typedef map< UINT64, shared_ptr<omTaskBase> >::iterator 
                                                            MAP_TASK_INTER ;
         typedef map< UINT64, shared_ptr<omTaskBase> >::value_type 
                                                            MAP_TASK_VALUETYPE ;
         UINT64                   _maxTaskID ;
   } ;
}

#endif /* OM_TASKMANAGER_HPP_ */



