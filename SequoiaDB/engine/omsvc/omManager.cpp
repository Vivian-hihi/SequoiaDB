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

   Source File Name = omManager.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "omManager.hpp"
#include "../bson/lib/md5.hpp"
#include "authCB.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "../bson/bsonobj.h"
#include "../util/fromjson.hpp"
#include "catCommon.hpp"
#include "../bson/lib/md5.hpp"

using namespace bson ;

namespace engine
{

   /*
      implement om manager
   */
   _omManager::_omManager()
   :_fixBufSize( SDB_PAGE_SIZE ),
    _rsManager(),
    _msgHandler( &_rsManager ),
    _netAgent( &_msgHandler )
   {
      _maxRestBodySize     = OM_REST_MAX_BODY_SIZE ;
      _restTimeout         = REST_TIMEOUT ;
      _sequence            = 1 ;

      _pKrcb               = NULL ;
      _pDmsCB              = NULL ;
   }

   _omManager::~_omManager()
   {
      SDB_ASSERT( _vecFixBuf.size() == 0, "Fix buff catch must be empty" ) ;
   }

   INT32 _omManager::init ()
   {
      INT32 rc           = SDB_OK ;
      
      // create collection space and collection
      _pKrcb  = pmdGetKRCB() ;
      _pDmsCB = _pKrcb->getDMSCB() ;
      _pRtnCB = _pKrcb->getRTNCB() ;

      // get options
      _wwwRootPath = pmdGetOptionCB()->getWWWPath() ;

      rc = _rsManager.init( getRouteAgent() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to init remote session manager, rc: %d",
                   rc ) ;

      rc = _initOmTables();
      PD_RC_CHECK ( rc, PDERROR, "Failed to initial the om tables rc = %d", 
                    rc ) ;
      
      rc = _restAdptor.init( _fixBufSize, _maxRestBodySize, _restTimeout ) ;

   done:
      return rc;
   error:
      goto done;
         
   }

   INT32 _omManager::_initOmTables() 
   {
      _pmdEDUCB *cb       = NULL ;
      INT32 rc            = SDB_OK ;
      BSONObjBuilder bsonBuilder ;
      SDB_AUTHCB *pAuthCB = NULL ;
      
      cb = pmdGetThreadEDUCB() ;

      // SYSDEPLOY.SYSCLUSTER
      rc = _createCollection ( OM_CS_DEPLOY_CL_CLUSTER, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_CLUSTER,
                                    OM_CS_DEPLOY_CL_CLUSTERIDX1, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      // SYSDEPLOY.SYSHOST
      rc = _createCollection ( OM_CS_DEPLOY_CL_HOST, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_HOST,
                                    OM_CS_DEPLOY_CL_HOSTIDX1, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_HOST,
                                    OM_CS_DEPLOY_CL_HOSTIDX2, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      // SYSDEPLOY.SYSBUSINESS
      rc = _createCollection ( OM_CS_DEPLOY_CL_BUSINESS, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_BUSINESS,
                             OM_CS_DEPLOY_CL_BUSINESSIDX1, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      // SYSDEPLOY.SYSCONFIGURE
      rc = _createCollection ( OM_CS_DEPLOY_CL_CONFIGURE, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      pAuthCB = pmdGetKRCB()->getAuthCB() ;
      pAuthCB->checkNeedAuth( cb, TRUE ) ;
      if ( !pAuthCB->needAuthenticate() )
      {
         md5::md5digest digest ;
         BSONObj obj ;
         bsonBuilder.append( SDB_AUTH_USER, OM_DEFAULT_LOGIN_USER ) ;
         md5::md5( ( const void * )OM_DEFAULT_LOGIN_PASSWD, 
                   ossStrlen( OM_DEFAULT_LOGIN_PASSWD ), digest) ;
         bsonBuilder.append( SDB_AUTH_PASSWD, md5::digestToString( digest ) ) ;
         obj = bsonBuilder.obj() ;
         rc = pAuthCB->createUsr( obj, cb ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to create default user:rc = %d",
                       rc ) ;
      }
      pAuthCB->checkNeedAuth( cb, TRUE ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::_createCollectionIndex ( const CHAR *pCollection,
                                              const CHAR *pIndex,
                                              pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj indexDef ;

      rc = fromjson ( pIndex, indexDef ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to build index object, rc = %d",
                    rc ) ;

      rc = catTestAndCreateIndex( pCollection, indexDef, cb, _pDmsCB,
                                  NULL, TRUE ) ;
      if ( rc )
      {
         goto error ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _omManager::_createCollection ( const CHAR *pCollection, pmdEDUCB *cb )
   {
      return catTestAndCreateCL( pCollection, cb, _pDmsCB, NULL, TRUE ) ;
   }

   INT32 _omManager::active ()
   {
      // set to primary
      pmdSetPrimary( TRUE ) ;

      return SDB_OK ;
   }

   INT32 _omManager::deactive ()
   {
      return SDB_OK ;
   }

   INT32 _omManager::fini ()
   {
      _rsManager.fini() ;

      // release fix buff catch
      _omLatch.get() ;
      for ( UINT32 i = 0 ; i < _vecFixBuf.size() ; ++i )
      {
         SDB_OSS_FREE( OM_FIX_BUFF_TO_PTR( _vecFixBuf[i] ) ) ;
      }
      _vecFixBuf.clear() ;
      _omLatch.release() ;

      // release session info
      restSessionInfo *pSessionInfo = NULL ;
      map<string, restSessionInfo*>::iterator it = _mapSessions.begin() ;
      while( it != _mapSessions.end() )
      {
         pSessionInfo = it->second ;
         pSessionInfo->releaseMem() ;
         SDB_OSS_DEL pSessionInfo ;
         ++it ;
      }
      _mapSessions.clear() ;
      _mapUser2Sessions.clear() ;

      return SDB_OK ;
   }

   INT32 _omManager::authenticate( BSONObj &obj, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      SDB_AUTHCB *pAuthCB = pmdGetKRCB()->getAuthCB() ;

      if ( !pAuthCB || !pAuthCB->needAuthenticate() )
      {
         goto done ;
      }

      rc = pAuthCB->authenticate( obj, cb ) ;
      if ( rc )
      {
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   CHAR* _omManager::allocFixBuf()
   {
      CHAR *pBuff = NULL ;

      // if fix buff catch is not empty, get from catch
      _omLatch.get() ;
      if ( _vecFixBuf.size() > 0 )
      {
         pBuff = _vecFixBuf.back() ;
         _vecFixBuf.pop_back() ;
      }
      _omLatch.release() ;

      if ( pBuff )
      {
         goto done ;
      }

      // alloc
      pBuff = ( CHAR* )SDB_OSS_MALLOC( OM_FIX_PTR_SIZE( _fixBufSize ) ) ;
      if ( !pBuff )
      {
         PD_LOG( PDERROR, "Alloc fix buff failed, size: %d",
                 OM_FIX_PTR_SIZE( _fixBufSize ) ) ;
         goto error ;
      }
      OM_FIX_PTR_HEADER( pBuff ) = _fixBufSize ;
      pBuff = OM_FIX_PTR_TO_BUFF( pBuff ) ;

   done:
      return pBuff ;
   error:
      goto done ;
   }

   void _omManager::releaseFixBuf( CHAR * pBuff )
   {
      SDB_ASSERT( pBuff, "Buff can't be NULL" ) ;
      SDB_ASSERT( OM_FIX_BUFF_HEADER( pBuff ) == _fixBufSize,
                  "Buff is not alloc by fix buff" ) ;

      // if fix buff catch is not full, push to catch
      _omLatch.get() ;
      if ( _vecFixBuf.size() < OM_FIX_BUFF_CATCH_NUMBER )
      {
         _vecFixBuf.push_back( pBuff ) ;
         pBuff = NULL ;
      }
      _omLatch.release() ;

      if ( pBuff )
      {
         SDB_OSS_FREE( OM_FIX_BUFF_TO_PTR( pBuff ) ) ;
      }
   }

   restSessionInfo* _omManager::attachSessionInfo( const string &id )
   {
      restSessionInfo *pSessionInfo = NULL ;

      _omLatch.get_shared() ;
      map<string, restSessionInfo*>::iterator it = _mapSessions.find( id ) ;
      if ( it != _mapSessions.end() )
      {
         pSessionInfo = it->second ;
         if ( pSessionInfo->isValid() )
         {
            pSessionInfo->_inNum.inc() ;
         }
         else
         {
            pSessionInfo = NULL ;
         }
      }
      _omLatch.release_shared() ;

      return pSessionInfo ;
   }

   void _omManager::detachSessionInfo( restSessionInfo * pSessionInfo )
   {
      SDB_ASSERT( pSessionInfo, "Session can't be NULL" ) ;
      pSessionInfo->_inNum.dec() ;
   }

   void _omManager::invalidSessionInfo( restSessionInfo *pSessionInfo )
   {
      SDB_ASSERT( pSessionInfo, "Session can't be NULL" ) ;
      pSessionInfo->invalidate() ;
   }

   restSessionInfo* _omManager::newSessionInfo( const string &userName,
                                                UINT32 localIP )
   {
      restSessionInfo *newSession = SDB_OSS_NEW restSessionInfo ;
      if( !newSession )
      {
         PD_LOG( PDERROR, "Alloc rest session info failed" ) ;
         goto error ;
      }

      // get lock
      _omLatch.get() ;
      newSession->_attr._sessionID = ossPack32To64( localIP, _sequence++ ) ;
      ossStrncpy( newSession->_attr._userName, userName.c_str(),
                  SESSION_USER_NAME_LEN ) ;
      // add to session map
      _mapSessions[ _makeID( newSession ) ] = newSession ;
      // add to user session map
      _add2UserMap( userName, newSession ) ;
      // attach session
      newSession->_inNum.inc() ;
      // release lock
      _omLatch.release() ;

   done:
      return newSession ;
   error:
      goto done ;
   }

   void _omManager::releaseSessionInfo (const string &sessionID )
   {
      //TODO: delete from _mapSessions & _mapUser2Sessions
   }

   void _omManager::_add2UserMap( const string &user,
                                  restSessionInfo *pSessionInfo )
   {
      map<string, vector<restSessionInfo*> >::iterator it ;
      it = _mapUser2Sessions.find( user ) ;
      // the user first session
      if ( it == _mapUser2Sessions.end() )
      {
         vector<restSessionInfo*> vecSession ;
         vecSession.push_back( pSessionInfo ) ;
         _mapUser2Sessions.insert( make_pair( user, vecSession ) ) ;
      }
      // the user already exist
      else
      {
         //vector<restSessionInfo*> &vecSession = it->second ;
         it->second.push_back( pSessionInfo ) ;
      }
   }

   string _omManager::_makeID( restSessionInfo * pSessionInfo )
   {
      UINT32 ip = 0 ;
      UINT32 seq = 0 ;
      ossUnpack32From64( pSessionInfo->_attr._sessionID, ip, seq ) ;
      CHAR tmp[9] = {0} ;
      ossSnprintf( tmp, sizeof(tmp)-1, "%08x", seq ) ;
      string strValue = md5::md5simpledigest( (const void*)pSessionInfo,
                                              pSessionInfo->getAttrSize() ) ;
      UINT32 size = strValue.size() ;
      strValue = strValue.substr( 0, size - ossStrlen( tmp ) ) ;
      strValue += tmp ;

      // set id
      pSessionInfo->_id = strValue ;
      return strValue ;
   }

   netRouteAgent* _omManager::getRouteAgent()
   {
      return &_netAgent ;
   }

   MsgRouteID _omManager::updateAgentInfo( const CHAR *pHost,
                                           const CHAR *pService )
   {
      MsgRouteID nodeID ;
      return nodeID ;
   }

   MsgRouteID _omManager::getAgentIDByHost( const CHAR *pHost )
   {
      MsgRouteID nodeID ;
      return nodeID ;
   }

   INT32 _omManager::getHostInfoByID( MsgRouteID routeID, string &host,
                                      string &servcie )
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   INT32 _omManager::sendMsgToAgent( const CHAR * pHost, MsgHeader *pMsg )
   {
      return SDB_OK ;
   }

   INT32 _omManager::saveInstallTask( string agentHost, string agentService ,
                                      BSONObj &taskInfo, 
                                      const BSONObj &confValue )
   {
      BSONObjBuilder builder ;
      INT32 rc = SDB_OK ;
      if ( isInstallTaskExist() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "previou task have not yet finished:task=%s",  
                 _omTaskInfo._taskID.c_str() ) ;
         goto error ;
      }

      _omTaskInfo._taskID        = taskInfo.getStringField( OM_BSON_TASKID ) ;
      _omTaskInfo._agentHostName = agentHost ;
      _omTaskInfo._agentSvcName  = agentService ;
      _omTaskInfo._detail        = taskInfo.getStringField( 
                                                       OM_REST_RES_DETAIL ) ;
      _omTaskInfo._isAllFinished = taskInfo.getBoolField( 
                                                       OM_BSON_ISFINISHED ) ;
      _omTaskInfo._progress      = taskInfo.getObjectField( 
                                                       OM_BSON_TASK_PROGRESS ) ;
      _omTaskInfo._confValue     = confValue.copy() ;
      _omTaskInfo._status        = OM_TASK_STATUS_DOING ;

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _omManager::isInstallTaskExist( )
   {
      if ( OM_TASK_STATUS_DOING == _omTaskInfo._status
           || OM_TASK_STATUS_ERROR_ROLLBACK == _omTaskInfo._status )
      {
         return TRUE ;
      }

      return FALSE ;
   }

   void _omManager::getTaskWriteLock()
   {
      _spinSlatch.get() ;
   }

   void _omManager::releaseTaskWriteLock()
   {
      _spinSlatch.release() ;
   }

   void _omManager::getInstallTask( INT32 &status, string &taskID, 
                                    bool &isAllFinished, string &detail, 
                                    BSONObj &progress )
   {
      status        = _omTaskInfo._status ;
      taskID        = _omTaskInfo._taskID ;
      isAllFinished = _omTaskInfo._isAllFinished ;
      detail        = _omTaskInfo._detail ;
      progress      = _omTaskInfo._progress ;
   }

   void _omManager::finishInstallTask( BSONObj &taskDetail )
   {
      _omTaskInfo._isAllFinished = true ;
      if ( OM_TASK_STATUS_ERROR_ROLLBACK == _omTaskInfo._status )
      {
         _omTaskInfo._status = OM_TASK_STATUS_ERROR_FINISH ;
      }
      else
      {
         _omTaskInfo._detail        = taskDetail.getStringField( 
                                                       OM_REST_RES_DETAIL );
         _omTaskInfo._progress      = taskDetail.getObjectField( 
                                                       OM_BSON_TASK_PROGRESS ) ;
         _omTaskInfo._status        = OM_TASK_STATUS_FINISH ;
         _omTaskInfo._isAllFinished = taskDetail.getBoolField( 
                                                       OM_BSON_ISFINISHED ) ;
      }
   }

   void _omManager::updateInstallTask( BSONObj &taskDetail )
   {
      if ( OM_TASK_STATUS_DOING == _omTaskInfo._status )
      {
         _omTaskInfo._progress = taskDetail.getObjectField( 
                                                       OM_BSON_TASK_PROGRESS ) ;
      }
   }

   void _omManager::rollBackTask( BSONObj &result )
   {
      INT32 rc          = SDB_OK ;
      _pmdEDUCB *cb     = pmdGetThreadEDUCB() ;
      CHAR* pContent    = NULL ;
      INT32 contentSize = 0 ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObjBuilder builder ;
      BSONObj msg ;

      if ( OM_TASK_STATUS_DOING != _omTaskInfo._status )
      {
         goto done ;
      }

      builder.append( OM_BSON_TASKID, _omTaskInfo._taskID ) ;
      msg = builder.obj() ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_ROLLBACK_INSTALL_REQ, 
                             0, 0, 0, -1, &msg, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "build query msg failed:rc=%d", rc ) ;
         goto error ;
      }

      remoteSession = getRSManager()->addSession( cb, OM_WAIT_SCAN_RES_INTERVAL, 
                                                  NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      rc = _sendMsgToLocalAgent( remoteSession, ( MsgHeader * )pContent ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      _omTaskInfo._status   = OM_TASK_STATUS_ERROR_ROLLBACK ;
      _omTaskInfo._detail   = result.getStringField( OM_REST_RES_DETAIL ) ;
      _omTaskInfo._progress = result.getObjectField( OM_BSON_TASK_PROGRESS ) ;
      _omTaskInfo._isAllFinished = result.getStringField( 
                                                       OM_BSON_ISFINISHED ) ;

   done:
      _clearSession( cb, remoteSession ) ;
      return;
   error:
      goto done ;
   }

   INT32 _omManager::_sendMsgToLocalAgent( pmdRemoteSession *remoteSession, 
                                           MsgHeader *pMsg )
   {
      MsgRouteID localAgentID ;
      INT32 rc = SDB_OK ;

      localAgentID = updateAgentInfo( _omTaskInfo._agentHostName.c_str(), 
                                      _omTaskInfo._agentSvcName.c_str() ) ;
      if ( NULL == remoteSession->addSubSession( localAgentID.value ) )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSubSession failed:id=%ld", localAgentID.value ) ;
         goto error ;
      }

      rc = remoteSession->sendMsg( pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send msg to localhost's agent failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::_receiveFromAgent( pmdRemoteSession *remoteSession,
                                        BSONObj &result )
   {
      VEC_SUB_SESSIONPTR subSessionVec ;
      INT32 rc           = SDB_OK ;
      MsgHeader *pRspMsg = NULL ;
      SINT32 flag        = 0 ;
      SINT64 contextID   = -1 ;
      SINT32 startFrom   = 0 ;
      SINT32 numReturned = 0 ;
      vector<BSONObj> objVec ;

      rc = remoteSession->waitReply( TRUE, &subSessionVec ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "wait reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( 1 != subSessionVec.size() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "unexpected session size:size=%d", 
                 subSessionVec.size() ) ;
         goto error ;
      }

      pRspMsg = subSessionVec[0]->getRspMsg() ;
      if ( NULL == pRspMsg )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive null response:rc=%d", rc ) ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)pRspMsg, &flag, &contextID, &startFrom, 
                            &numReturned, objVec ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "extract reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( 1 != objVec.size() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "unexpected response size:rc=%d", rc ) ;
         goto error ;
      }

      result = objVec[0] ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _omManager::_clearSession( _pmdEDUCB *cb, 
                                   pmdRemoteSession *remoteSession) 
   {
      if ( NULL != remoteSession )
      {
         pmdSubSession *pSubSession = NULL ;
         pmdSubSessionItr itr       = remoteSession->getSubSessionItr() ;
         while ( itr.more() )
         {
            pSubSession = itr.next() ;
            MsgHeader *pMsg = pSubSession->getReqMsg() ;
            if ( NULL != pMsg )
            {
               SDB_OSS_FREE( pMsg ) ;
            }
         }

         remoteSession->clearSubSession() ;
         getRSManager()->removeSession( remoteSession ) ;
      }
   }

   void _omManager::checkTaskStatus( string taskID )
   {
      bool isFinished ;
      BSONObj result ;
      INT32 rc          = SDB_OK ;
      _pmdEDUCB *cb     = pmdGetThreadEDUCB() ;
      MsgHeader *pMsg   = NULL ;
      CHAR* pContent    = NULL ;
      INT32 contentSize = 0 ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObjBuilder builder ;
      BSONObj msg ;
      builder.append( OM_BSON_TASKID, taskID ) ;
      msg = builder.obj() ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, OM_INSTALL_BUSINESS_REQ, 
                             0, 0, 0, -1, &msg, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "build query msg failed:rc=%d", rc ) ;
         goto error ;
      }
      
      remoteSession = getRSManager()->addSession( cb, OM_WAIT_SCAN_RES_INTERVAL, NULL ) ;
      if ( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSession failed" ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc   = _sendMsgToLocalAgent( remoteSession, pMsg ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         SDB_OSS_FREE( pContent ) ;
         goto error ;
      }

      rc = _receiveFromAgent( remoteSession, result ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "receive from agent failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = result.getIntField( OM_REST_RES_RETCODE ) ;
      if ( SDB_OK != rc )
      {
         rollBackTask( result ) ;
         string errorInfo = result.getStringField( OM_REST_RES_DETAIL ) ;
         PD_LOG( PDERROR, "%s", errorInfo.c_str(), rc ) ;
         goto error ;
      }

      isFinished = result.getBoolField( OM_BSON_ISFINISHED ) ;
      if ( isFinished )
      {
         finishInstallTask( result ) ;
      }
      else
      {
         updateInstallTask( result ) ;
      }

   done:
      _clearSession( cb, remoteSession ) ;
   error:
      goto done ;
   }

   /*
      get the global om manager object point
   */
   omManager* sdbGetOMManager()
   {
      static omManager s_omManager ;
      return &s_omManager ;
   }

}


