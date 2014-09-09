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
#include "ossProc.hpp"
#include "rtn.hpp"
#include "omManagerJob.hpp"


using namespace bson ;

namespace engine
{

   #define OM_WAIT_CB_ATTACH_TIMEOUT               ( 300 * OSS_ONE_SEC )

   /*
      Message Map
   */
   BEGIN_OBJ_MSG_MAP( _omManager, _pmdObjBase )

   END_OBJ_MSG_MAP()

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
      _checkSessionTimer   = NET_INVALID_TIMER_ID ;

      _hwRouteID.value     = MSG_INVALID_ROUTEID ;
      _hwRouteID.columns.groupID = 2 ;
      _hwRouteID.columns.nodeID  = 0 ;
      _hwRouteID.columns.serviceID = MSG_ROUTE_LOCAL_SERVICE ;

      _pKrcb               = NULL ;
      _pDmsCB              = NULL ;
      _hostVersion         = SDB_OSS_NEW omHostVersion() ;
   }

   _omManager::~_omManager()
   {
      SDB_ASSERT( _vecFixBuf.size() == 0, "Fix buff catch must be empty" ) ;
      if ( NULL != _hostVersion )
      {
         SDB_OSS_DEL _hostVersion ;
         _hostVersion = NULL ;
      }
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

      rc = _restoreTask() ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to restore task:rc=%d", 
                    rc ) ;

      //TODO: open this code
      rc = _createJobs() ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create jobs:rc=%d", 
                    rc ) ;

      rc = refreshVersions() ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to update cluster version:rc=%d", 
                    rc ) ;

      _readAgentPort() ;

      rc = _restAdptor.init( _fixBufSize, _maxRestBodySize, _restTimeout ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to init rest adptor, rc: %d", rc ) ;

      _checkTaskTimer = NET_INVALID_TIMER_ID ;

   done:
      return rc;
   error:
      goto done;

   }

   INT32 _omManager::_createJobs()
   {
      INT32 rc                = SDB_OK ;
      BOOLEAN returnResult    = FALSE ;
      omHostNotifierJob *pJob = NULL ;
      EDUID jobID             = PMD_INVALID_EDUID ;
      pJob = SDB_OSS_NEW omHostNotifierJob( this, _hostVersion ) ;
      if ( !pJob )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "failed to create omHostNotifierJob:rc=%d", rc ) ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, &jobID,
                                     returnResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "create omHostNotifierJob failed:rc=%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::refreshVersions()
   {
      INT32 rc = SDB_OK ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObjBuilder builder ;
      SINT64 contextID = -1 ;

      BSONObjBuilder resultBuilder ;
      BSONObj result ;
      pmdKRCB *pKrcb     = pmdGetKRCB() ;
      _SDB_DMSCB *pDMSCB = pKrcb->getDMSCB() ;
      _SDB_RTNCB *pRTNCB = pKrcb->getRTNCB() ;
      _pmdEDUCB *pEDUCB  = pmdGetThreadEDUCB() ;

      selector = BSON( OM_CLUSTER_FIELD_NAME << "" ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, matcher, order, hint, 0, 
                     pEDUCB, 0, -1, pDMSCB, pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "fail to query table:%s,rc=%d", 
                 OM_CS_DEPLOY_CL_CLUSTER, rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore( contextID, 1, buffObj, startingPos, pEDUCB, pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d", 
                    OM_CS_DEPLOY_CL_CLUSTER, rc ) ;
            goto error ;
         }

         BSONObj record( buffObj.data() ) ;
         string clusterName = record.getStringField( OM_CLUSTER_FIELD_NAME ) ;
         _hostVersion->incVersion( clusterName ) ;
      }
   done:
      return rc ;
   error:
      if ( -1 != contextID )
      {
         pRTNCB->contextDelete( contextID, pEDUCB ) ;
      }
      goto done ;
   }

   void _omManager::updateClusterVersion( string cluster )
   {
      _hostVersion->incVersion( cluster ) ;
   }

   void _omManager::removeClusterVersion( string cluster )
   {
      _hostVersion->removeVersion( cluster ) ;
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

      // SYSDEPLOY.SYSTASKINFO
      rc = _createCollection ( OM_CS_DEPLOY_CL_TASKINFO, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_TASKINFO,
                             OM_CS_DEPLOY_CL_TASKINFOIDX1, cb ) ;
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
      INT32 rc = SDB_OK ;
      pmdEDUMgr *pEDUMgr = pmdGetKRCB()->getEDUMgr() ;
      EDUID eduID = PMD_INVALID_EDUID ;

      // set to primary
      pmdSetPrimary( TRUE ) ;

      // start om manager edu
      rc = pEDUMgr->startEDU( EDU_TYPE_OMMGR, (_pmdObjBase*)this, &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start OM Manager edu, rc: %d", rc ) ;
      // register
      pEDUMgr->regSystemEDU( EDU_TYPE_OMMGR, eduID ) ;
      // wait attach
      rc = _attachEvent.wait( OM_WAIT_CB_ATTACH_TIMEOUT ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait OM Manager edu attach failed, rc: %d",
                   rc ) ;

      // register timer( must before start EDU_TYPE_OMNET )
      _checkSessionTimer = setTimer( 60 * OSS_ONE_SEC ) ;
      if ( NET_INVALID_TIMER_ID == _checkSessionTimer )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to set timer" ) ;
         goto error ;
      }

      // register task timer
      _checkTaskTimer = setTimer( 2 * OSS_ONE_SEC ) ;
      if ( NET_INVALID_TIMER_ID == _checkTaskTimer )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to set timer" ) ;
         goto error ;
      }

      // start om net
      rc = pEDUMgr->startEDU( EDU_TYPE_OMNET, (netRouteAgent*)&_netAgent,
                              &eduID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to start om net, rc: %d", rc ) ;
      // register
      pEDUMgr->regSystemEDU( EDU_TYPE_OMNET, eduID ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::deactive ()
   {
      // stop io
      _netAgent.stop() ;

      // kill check sessions timer
      if ( NET_INVALID_TIMER_ID != _checkSessionTimer )
      {
         killTimer( _checkSessionTimer ) ;
         _checkSessionTimer = NET_INVALID_TIMER_ID ;
      }

      if ( NET_INVALID_TIMER_ID != _checkTaskTimer )
      {
         killTimer( _checkTaskTimer ) ;
         _checkTaskTimer = NET_INVALID_TIMER_ID ;
      }

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

      _mapID2Host.clear() ;
      _mapHost2ID.clear() ;

      return SDB_OK ;
   }

   void _omManager::attachCB( _pmdEDUCB *cb )
   {
      _rsManager.registerEDU( cb ) ;
      _msgHandler.attach( cb ) ;
      _timerHandler.attach( cb ) ;
      _attachEvent.signalAll() ;
   }

   void _omManager::detachCB( _pmdEDUCB *cb )
   {
      _msgHandler.detach() ;
      _timerHandler.detach() ;
      _rsManager.unregEUD( cb ) ;
   }

   UINT32 _omManager::setTimer( UINT32 milliSec )
   {
      UINT32 timeID = NET_INVALID_TIMER_ID ;
      _netAgent.addTimer( milliSec, &_timerHandler, timeID ) ;
      return timeID ;
   }

   void _omManager::killTimer( UINT32 timerID )
   {
      _netAgent.removeTimer( timerID ) ;
   }

   void _omManager::onTimer( UINT64 timerID, UINT32 interval )
   {
      if ( timerID == _checkSessionTimer )
      {
         _checkSession( interval ) ;
      }
      else if ( timerID == _checkTaskTimer )
      {
         checkTaskStatus( _omTaskInfo._taskID ) ;
      }
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

   INT32 _omManager::authUpdatePasswd( string user, string oldPasswd,
                                       string newPasswd, pmdEDUCB *cb )
   {
      INT32 rc            = SDB_OK ;
      SDB_AUTHCB *pAuthCB = pmdGetKRCB()->getAuthCB() ;
      if ( NULL == pAuthCB )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = pAuthCB->updatePasswd( user, oldPasswd, newPasswd, cb ) ;

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

      if ( pSessionInfo )
      {
         pSessionInfo->lock() ;
      }

      return pSessionInfo ;
   }

   void _omManager::detachSessionInfo( restSessionInfo * pSessionInfo )
   {
      SDB_ASSERT( pSessionInfo, "Session can't be NULL" ) ;

      if ( pSessionInfo->isLock() )
      {
         pSessionInfo->unlock() ;
         pSessionInfo->_inNum.dec() ;
      }
   }

   void _omManager::_invalidSessionInfo( restSessionInfo *pSessionInfo )
   {
      SDB_ASSERT( pSessionInfo, "Session can't be NULL" ) ;
      pSessionInfo->invalidate() ;
   }

   void _omManager::_checkSession( UINT32 interval )
   {
      map<string, restSessionInfo*>::iterator it  ;
      restSessionInfo *pInfo = NULL ;

      _omLatch.get() ;
      it = _mapSessions.begin() ;
      while ( it != _mapSessions.end() )
      {
         pInfo = it->second ;
         if ( pInfo->isIn() )
         {
            ++it ;
            continue ;
         }

         if ( pInfo->isValid()  )
         {
            pInfo->onTimer( interval ) ;
            if ( pInfo->isTimeout( OM_REST_SESSION_TIMEOUT ) )
            {
               pInfo->invalidate() ;
            }
         }

         if ( !pInfo->isValid() )
         {
            _delFromUserMap( pInfo->_attr._userName, pInfo ) ;
            SDB_OSS_DEL pInfo ;
            _mapSessions.erase( it++ ) ;
            continue ;
         }
         ++it ;
      }
      _omLatch.release() ;
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

      if ( newSession )
      {
         newSession->lock() ;
      }

   done:
      return newSession ;
   error:
      goto done ;
   }

   void _omManager::releaseSessionInfo ( const string &sessionID )
   {
      restSessionInfo *pInfo = NULL ;
      map<string, restSessionInfo*>::iterator it ;

      _omLatch.get() ;
      it = _mapSessions.find( sessionID ) ;
      if ( it != _mapSessions.end() )
      {
         pInfo = it->second ;
         _delFromUserMap( pInfo->_attr._userName, pInfo ) ;

         if ( pInfo->isLock() )
         {
            detachSessionInfo( pInfo ) ;
         }

         // no use
         if ( !pInfo->isIn() )
         {
            SDB_OSS_DEL pInfo ;
            _mapSessions.erase( it ) ;
         }
         else
         {
            _invalidSessionInfo( pInfo ) ;
         }
      }
      _omLatch.release() ;
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
         it->second.push_back( pSessionInfo ) ;
      }
   }

   void _omManager::_delFromUserMap( const string &user,
                                     restSessionInfo *pSessionInfo )
   {
      map<string, vector<restSessionInfo*> >::iterator it ;
      it = _mapUser2Sessions.find( user ) ;
      if ( it != _mapUser2Sessions.end() )
      {
         vector<restSessionInfo*> &vecSessions = it->second ;
         vector<restSessionInfo*>::iterator itVec = vecSessions.begin() ;
         while ( itVec != vecSessions.end() )
         {
            if ( *itVec == pSessionInfo )
            {
               vecSessions.erase( itVec ) ;
               break ;
            }
            ++itVec ;
         }

         if ( vecSessions.size() == 0 )
         {
            _mapUser2Sessions.erase( it ) ;
         }
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

   MsgRouteID _omManager::_incNodeID()
   {
      ++_hwRouteID.columns.nodeID ;
      if ( 0 == _hwRouteID.columns.nodeID )
      {
         _hwRouteID.columns.nodeID = 1 ;
         ++_hwRouteID.columns.groupID ;
      }
      return _hwRouteID ;
   }

   MsgRouteID _omManager::updateAgentInfo( const string &host,
                                           const string &service )
   {
      MsgRouteID nodeID ;
      ossScopedLock lock( &_omLatch, EXCLUSIVE ) ;
      MAP_HOST2ID_IT it = _mapHost2ID.find( host ) ;
      if ( it != _mapHost2ID.end() )
      {
         omAgentInfo &info = it->second ;
         nodeID.value = info._id ;
         _mapID2Host.erase( info._id ) ;
         _netAgent.updateRoute( nodeID, host.c_str(), service.c_str() ) ;
         info._host = host ;
         info._service = service ;
      }
      else
      {
         nodeID = _incNodeID() ;
         omAgentInfo &info = _mapHost2ID[ host ] ;
         info._id = nodeID.value ;
         info._host = host ;
         info._service = service ;
         _mapID2Host[ info._id ] = &info ;
         _netAgent.updateRoute( nodeID, host.c_str(), service.c_str() ) ;
      }

      return nodeID ;
   }

   MsgRouteID _omManager::getAgentIDByHost( const string &host )
   {
      MsgRouteID nodeID ;
      nodeID.value = MSG_INVALID_ROUTEID ;
      ossScopedLock lock( &_omLatch, SHARED ) ;
      MAP_HOST2ID_IT it = _mapHost2ID.find( host ) ;
      if ( it != _mapHost2ID.end() )
      {
         nodeID.value = it->second._id ;
      }
      return nodeID ;
   }

   INT32 _omManager::getHostInfoByID( MsgRouteID routeID, string &host,
                                      string &service )
   {
      INT32 rc = SDB_OK ;
      omAgentInfo *pInfo = NULL ;

      ossScopedLock lock( &_omLatch, SHARED ) ;
      MAP_ID2HOSTPTR_IT it = _mapID2Host.find( routeID.value ) ;
      if ( it != _mapID2Host.end() )
      {
         pInfo = it->second ;
         host = pInfo->_host ;
         service = pInfo->_service ;
      }
      else
      {
         rc = SDB_CLS_NODE_NOT_EXIST ;
      }

      return rc ;
   }

   void _omManager::delAgent( const string &host )
   {
      ossScopedLock lock( &_omLatch, EXCLUSIVE ) ;
      MAP_HOST2ID_IT it = _mapHost2ID.find( host ) ;
      if ( it != _mapHost2ID.end() )
      {
         MsgRouteID nodeID ;
         nodeID.value = it->second._id ;
         _mapID2Host.erase( it->second._id ) ;
         _netAgent.delRoute( nodeID ) ;
         _mapHost2ID.erase( it ) ;
      }
   }

   void _omManager::delAgent( MsgRouteID routeID )
   {
      ossScopedLock lock( &_omLatch, EXCLUSIVE ) ;
      MAP_ID2HOSTPTR_IT it = _mapID2Host.find( routeID.value ) ;
      if ( it != _mapID2Host.end() )
      {
         MsgRouteID nodeID ;
         nodeID.value = it->first ;
         string host = it->second->_host ;
         _netAgent.delRoute( nodeID ) ;
         _mapID2Host.erase( it ) ;
         _mapHost2ID.erase( host ) ;
      }
   }

   INT32 _omManager::saveInstallTask( string agentHost, string agentService ,
                                      BSONObj &taskInfo, 
                                      const BSONObj &confValue )
   {
      BSONObjBuilder builder ;
      BSONElement taskElement ;
      CHAR taskID[ OM_INT32_LENGTH ] ;
      INT32 rc = SDB_OK ;
      if ( isInstallTaskExist() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "previous task have not yet finished:task=%s",  
                 _omTaskInfo._taskID.c_str() ) ;
         goto error ;
      }

      taskElement = taskInfo.getField( OM_BSON_TASKID ) ;
      ossLltoa( taskElement.numberLong(), taskID, OM_INT32_LENGTH ) ;
      _omTaskInfo._taskID        = taskID ;
      _omTaskInfo._agentHostName = agentHost ;
      _omTaskInfo._agentSvcName  = agentService ;
      _omTaskInfo._detail        = taskInfo.getStringField( 
                                                       OM_REST_RES_DETAIL ) ;
      _omTaskInfo._isAllFinished = taskInfo.getBoolField( 
                                                       OM_BSON_ISFINISHED ) ;
      _omTaskInfo._progress      = taskInfo.getObjectField( 
                                                       OM_BSON_TASK_PROGRESS 
                                                       ).copy() ;
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
      _taskLatch.get() ;
   }

   void _omManager::releaseTaskWriteLock()
   {
      _taskLatch.release() ;
   }

   void _omManager::getInstallTask( INT32 &status, string &taskID, 
                                    bool &isAllFinished, string &detail, 
                                    BSONObj &progress )
   {
      status        = _omTaskInfo._status ;
      taskID        = _omTaskInfo._taskID ;
      isAllFinished = _omTaskInfo._isAllFinished ;
      detail        = _omTaskInfo._detail ;
      progress      = _omTaskInfo._progress.copy() ;
   }

   INT32 _omManager::finishInstallTask( BSONObj &taskDetail )
   {
      INT32 rc = SDB_OK ;
      _omTaskInfo._isAllFinished = true ;
      string status = taskDetail.getStringField( OM_BSON_TASK_STATUS ) ;
      if ( status.compare( OM_TASK_STATUS_ROLLBACK ) == 0 )
      {
         _omTaskInfo._status = OM_TASK_STATUS_ERROR_ROLLBACK ;
      }

      if ( OM_TASK_STATUS_DOING == _omTaskInfo._status )
      {
         rc = _storeBusinessInfo() ;
         if ( SDB_OK != rc )
         {
            /*
            in case this situation happened.
               we should let OM try again.

            if it can't recovered. we should manually store it 
               and delete the task info
            */
            PD_LOG( PDERROR, "store business info failed:rc=%d", rc ) ;
            goto error ;
         }
         rc = _storeConfigInfo() ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "store config info failed:rc=%d", rc ) ;
            goto error ;
         }

         _omTaskInfo._detail   = taskDetail.getStringField( 
                                                       OM_REST_RES_DETAIL );
         _omTaskInfo._progress = taskDetail.getObjectField( 
                                                       OM_BSON_TASK_PROGRESS ) ;
      }

      _omTaskInfo._isAllFinished = true ;
      if ( OM_TASK_STATUS_DOING == _omTaskInfo._status )
      {
         _omTaskInfo._status = OM_TASK_STATUS_FINISH ;
      }
      else
      {
         _omTaskInfo._status = OM_TASK_STATUS_ERROR_FINISH ;
      }

      rc = removeTask( _omTaskInfo._taskID ) ;
      if ( SDB_OK != rc )
      {
         /*
            in case this situation happened.
            just delete the task info manually, and everything is OK.
         */
         PD_LOG( PDERROR, "task is done and delete task info failed:taskID=%s,"
                 "status=%d", _omTaskInfo._taskID.c_str(), _omTaskInfo._status, 
                 rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _omManager::updateInstallTask( BSONObj &taskDetail )
   {
      string status = taskDetail.getStringField( OM_BSON_TASK_STATUS ) ;
      if ( status.compare( OM_TASK_STATUS_ROLLBACK ) == 0 )
      {
         _omTaskInfo._status = OM_TASK_STATUS_ERROR_ROLLBACK ;
      }

      if ( OM_TASK_STATUS_DOING == _omTaskInfo._status )
      {
         _omTaskInfo._progress = taskDetail.getObjectField( 
                                                       OM_BSON_TASK_PROGRESS ) ;
      }
   }

   string _omManager::getLocalAgentPort()
   {
      return _localAgentPort ;
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

      if ( subSessionVec[0]->isDisconnect() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG(PDERROR, "session disconnected:id=%s,rc=%d", 
                routeID2String(subSessionVec[0]->getNodeID()).c_str(), rc ) ;
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

      if ( !isInstallTaskExist() )
      {
         goto done ;
      }

      builder.append( OM_BSON_TASKID, ossAtoll( taskID.c_str() ) ) ;
      msg = builder.obj() ;
      rc = msgBuildQueryMsg( &pContent, &contentSize, 
                             CMD_ADMIN_PREFIX OM_QUERY_PROGRESS, 
                             0, 0, 0, -1, &msg, NULL, NULL, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "build query msg failed:rc=%d", rc ) ;
         goto error ;
      }

      remoteSession = getRSManager()->addSession( cb, 
                                                  OM_WAIT_PROGRESS_RES_INTERVAL,
                                                  NULL ) ;
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
         remoteSession->clearSubSession() ;
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
         string errorInfo = result.getStringField( OM_REST_RES_DETAIL ) ;
         PD_LOG( PDERROR, "agent process error:detail=%s,rc=%d", 
                 errorInfo.c_str(), rc ) ;
         goto error ;
      }

      isFinished = result.getBoolField( OM_BSON_ISFINISHED ) ;
      getTaskWriteLock() ;
      if ( isFinished )
      {
         rc = finishInstallTask( result ) ;
         //TODO: get transactionID and add config to the table
      }
      else
      {
         updateInstallTask( result ) ;
      }
      releaseTaskWriteLock() ;

   done:
      _clearSession( cb, remoteSession ) ;
      return ;
   error:
      goto done ;
   }

   void _omManager::_readAgentPort()
   {
      INT32 rc = SDB_OK ;
      CHAR conf[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      po::options_description desc ( "Config options" ) ;
      po::variables_map vm ;
      CHAR hostport[OSS_MAX_HOSTNAME + 6] = { 0 } ;
      _localAgentPort = boost::lexical_cast<string>( SDBCM_DFT_PORT ) ;
      rc = ossGetHostName( hostport, OSS_MAX_HOSTNAME ) ;
      if ( rc != SDB_OK )
      {
         PD_LOG( PDERROR, "get host name failed:rc=%d", rc ) ;
         goto error ;
      }

      ossStrncat ( hostport, SDBCM_CONF_PORT, ossStrlen(SDBCM_CONF_PORT) ) ;

      desc.add_options()
         (SDBCM_CONF_DFTPORT, po::value<string>(), "sdbcm default "
         "listening port")
         (hostport, po::value<string>(), "sdbcm specified listening port")
      ;

      rc = ossGetEWD ( conf, OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get excutable file's working "
                  "directory" ) ;
         goto error ;
      }

      if ( ( ossStrlen ( conf ) + ossStrlen ( SDBCM_CONF_PATH_FILE ) + 2 ) >
           OSS_MAX_PATHSIZE )
      {
         PD_LOG ( PDERROR, "Working directory too long" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      ossStrncat( conf, OSS_FILE_SEP, 1 );
      ossStrncat( conf, SDBCM_CONF_PATH_FILE,
                  ossStrlen( SDBCM_CONF_PATH_FILE ) );
      rc = utilReadConfigureFile ( conf, desc, vm ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read configure file, rc = %d", rc ) ;
         goto error ;
      }
      else if ( vm.count( hostport ) )
      {
         _localAgentPort = vm[hostport].as<string>() ;
      }
      else if ( vm.count( SDBCM_CONF_DFTPORT ) )
      {
         _localAgentPort = vm[SDBCM_CONF_DFTPORT].as<string>() ;
      }
      else
      {
         _localAgentPort = boost::lexical_cast<string>( SDBCM_DFT_PORT ) ;
      }

   done:
      return ;
   error:
      goto done ;
   }

   INT32 _omManager::storeTaskInfo( string taskID, string taskType, 
                                    string agentHost, string agentService,
                                    const BSONObj &confValue, INT32 status )
   {
      INT32 rc     = SDB_OK ;
      pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      BSONObjBuilder builder ;
      BSONObj tmp  = BSON( OM_TASKINFO_FIELD_TASKID << taskID 
                           << OM_TASKINFO_FIELD_TYPE << taskType
                           << OM_TASKINFO_FIELD_AGENTHOST << agentHost 
                           << OM_TASKINFO_FIELD_AGENTSERVICE << agentService
                           << OM_TASKINFO_FIELD_INFO << confValue 
                           << OM_TASKINFO_FIELD_STATUS << status ) ;

      rc = rtnInsert( OM_CS_DEPLOY_CL_TASKINFO, tmp, 1, 0, cb );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to store taskinfo into table:%s,rc=%d", 
                 OM_CS_DEPLOY_CL_TASKINFO, rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::removeTask( string taskID )
   {
      INT32 rc          = SDB_OK ;
      pmdEDUCB *cb      = pmdGetThreadEDUCB() ;
      BSONObj condition = BSON( OM_TASKINFO_FIELD_TASKID << taskID ) ;
      BSONObj hint ;

      rc = rtnDelete( OM_CS_DEPLOY_CL_TASKINFO, condition, hint, 0, cb );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to delete taskinfo from table:%s,"
                     "taskID=%s,rc=%d", OM_CS_DEPLOY_CL_TASKINFO, 
                     taskID.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::updateTaskID( string oldID, long long newID )
   {
      CHAR taskID[OM_INT32_LENGTH] ;
      ossLltoa( newID, taskID, OM_INT32_LENGTH ) ;

      return updateTaskID( oldID, taskID ) ;
   }

   INT32 _omManager::updateTaskID( string oldID, string newID )
   {
      INT32 rc     = SDB_OK ;
      pmdEDUCB *cb = pmdGetThreadEDUCB() ; ;
      BSONObjBuilder builder ;
      BSONObj hint ;

      BSONObj condition = BSON( OM_TASKINFO_FIELD_TASKID << oldID ) ;
      BSONObj tmp       = BSON( OM_TASKINFO_FIELD_TASKID << newID ) ;
      BSONObj obj       = BSON( "$set" << tmp ) ;
      rc = rtnUpdate( OM_CS_DEPLOY_CL_TASKINFO, condition, obj, hint, 0, cb, 
                      _pDmsCB, _pKrcb->getDPSCB() ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to update taskid in table:%s,oldID=%s,"
                     "newID=%s,rc=%d", OM_CS_DEPLOY_CL_TASKINFO, 
                     oldID.c_str(), newID.c_str(), rc ) ;
         SDB_ASSERT( 0, "this should not happend!" ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::_insertConfigure( string hostName, string businessName ,
                                       BSONObj &oneNode )
   {
      pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      INT32 rc     = SDB_OK ;
      BSONArrayBuilder arrayBuilder ;
      BSONObj filter  = BSON( OM_BSON_FIELD_HOST_NAME << "" 
                              << OM_BSON_FIELD_HOST_USER << "" 
                              << OM_BSON_FIELD_HOST_PASSWD << "" ) ;
      BSONObj oneConf = oneNode.filterFieldsUndotted( filter, false ) ;
      arrayBuilder.append( oneConf ) ;
      BSONObj obj = BSON( OM_CONFIGURE_FIELD_BUSINESSNAME << businessName 
                          << OM_CONFIGURE_FIELD_HOSTNAME << hostName 
                          << OM_CONFIGURE_FIELD_CONFIG << arrayBuilder.arr() ) ;
      rc = rtnInsert( OM_CS_DEPLOY_CL_CONFIGURE, obj, 1, 0, cb );
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "failed to store config into table:%s,rc=%d", 
                     OM_CS_DEPLOY_CL_CONFIGURE, rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::_appendConfigure( string hostName, string businessName ,
                                       BSONObj &oneNode )
   {
      pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      INT32 rc     = SDB_OK ;
      BSONArrayBuilder arrayBuilder ;
      BSONObj filter  = BSON( OM_BSON_FIELD_HOST_NAME << "" 
                              << OM_BSON_FIELD_HOST_USER << "" 
                              << OM_BSON_FIELD_HOST_PASSWD << "" ) ;
      BSONObj oneConf = oneNode.filterFieldsUndotted( filter, false ) ;
      arrayBuilder.append( oneConf ) ;

      BSONObj condition = BSON( OM_CONFIGURE_FIELD_BUSINESSNAME << businessName 
                                << OM_CONFIGURE_FIELD_HOSTNAME << hostName );
      BSONObj tmp = BSON( OM_CONFIGURE_FIELD_CONFIG << arrayBuilder.arr() ) ;
      BSONObj obj = BSON( "$addtoset" << tmp ) ;
      {
         BSONObj hint ;
         rc = rtnUpdate( OM_CS_DEPLOY_CL_CONFIGURE, condition, obj, hint,
                         0, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to update config for %s in %s:rc=%d", 
                    hostName.c_str(), OM_CS_DEPLOY_CL_CONFIGURE, rc ) ;
            goto error ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _omManager::_isHostConfExist( string hostName, string businessName )
   {
      INT32 rc         = SDB_OK ;
      pmdEDUCB *cb     = pmdGetThreadEDUCB() ;
      BOOLEAN flag     = FALSE ;
      SINT64 contextID = -1 ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj orderBy ;
      BSONObj hint ;

      matcher = BSON( OM_CONFIGURE_FIELD_BUSINESSNAME << businessName 
                      << OM_CONFIGURE_FIELD_HOSTNAME << hostName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_CONFIGURE, selector, matcher, orderBy, 
                     hint, 0, cb, 0, -1, _pDmsCB, _pRtnCB, contextID ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "fail to query table:%s,rc=%d", 
                     OM_CS_DEPLOY_CL_CONFIGURE, rc ) ;
         goto done ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, cb, _pRtnCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               goto done ;
            }

            contextID = -1 ;
            PD_LOG_MSG( PDERROR, "failed to get record from table:%s,rc=%d", 
                        OM_CS_DEPLOY_CL_TASKINFO, rc ) ;
            goto done ;
         }

         flag = TRUE ;
         goto done ;
      }

   done:
      if ( -1 != contextID )
      {
         _pRtnCB->contextDelete ( contextID, cb ) ;
      }
      return flag ;
   }

   INT32 _omManager::_storeConfigInfo()
   {
      string businessName ;
      BSONObj configs ;
      INT32 rc      = SDB_OK ;
      BSONObj &conf = _omTaskInfo._confValue ;
      businessName  = conf.getStringField( OM_BSON_BUSINESS_NAME );
      configs       = conf.getObjectField( OM_BSON_FIELD_CONFIG ) ;
      {
         BSONObjIterator iter( configs ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            BSONObj oneNode = ele.embeddedObject() ;
            string hostName = oneNode.getStringField( 
                                                     OM_BSON_FIELD_HOST_NAME ) ;
            if ( _isHostConfExist( hostName, businessName ) )
            {
               rc = _appendConfigure( hostName, businessName, oneNode ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "append configure failed:host=%s,"
                                   "business=%s, node=%s, rc=%d", 
                                   hostName.c_str(), businessName.c_str(), 
                                   oneNode.toString(false, false).c_str(), 
                                   rc ) ;
                  goto error ;
               }
            }
            else
            {
               rc = _insertConfigure( hostName, businessName, oneNode ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "insert configure failed:host=%s,"
                                   "business=%s, node=%s, rc=%d", 
                          hostName.c_str(), businessName.c_str(), 
                          oneNode.toString(false, false).c_str(), rc ) ;
                  goto error ;
               }
            }
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::_storeBusinessInfo()
   {
      INT32 rc = SDB_OK ;
      string businessName ;
      string deployMod ;
      string businessType ;
      string clusterName ;
      BSONObj obj ;
      BSONObj configs ;
      pmdEDUCB *cb  = pmdGetThreadEDUCB() ;

      BSONObj &conf = _omTaskInfo._confValue ;
      businessName  = conf.getStringField( OM_BSON_BUSINESS_NAME );
      deployMod     = conf.getStringField( OM_BSON_DEPLOY_MOD ) ;
      businessType  = conf.getStringField( OM_BSON_BUSINESS_TYPE );
      clusterName   = conf.getStringField( OM_BSON_FIELD_CLUSTER_NAME );
      if ( businessName == "" || businessType == "" || clusterName == "" )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "bson miss field:businessName=%s,businessType=%s,"
                     "clusterName=%s", businessName.c_str(), 
                     businessType.c_str(), clusterName.c_str() ) ;
         SDB_ASSERT( 0, "bson miss field" ) ;
         goto error ;
      }

      obj = BSON( OM_BUSINESS_FIELD_NAME << businessName 
                  << OM_BSON_BUSINESS_TYPE << businessType 
                  << OM_BSON_DEPLOY_MOD << deployMod
                  << OM_BSON_FIELD_CLUSTER_NAME << clusterName ) ;
      rc = rtnInsert( OM_CS_DEPLOY_CL_BUSINESS, obj, 1, 0, cb );
      if ( rc )
      {
         if ( SDB_IXM_DUP_KEY != rc )
         {
            PD_LOG_MSG( PDERROR, "failed to store business into table:%s,rc=%d", 
                    OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
            goto error ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omManager::_restoreTask()
   {
      INT32 rc         = SDB_OK ;
      pmdEDUCB *cb     = pmdGetThreadEDUCB() ;
      SINT64 contextID = -1 ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj orderBy ;
      BSONObj hint ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_TASKINFO, selector, matcher, orderBy, hint, 
                     0, cb, 0, -1, _pDmsCB, _pRtnCB, contextID ) ;
      if ( rc )
      {
         PD_LOG_MSG( PDERROR, "fail to query table:%s,rc=%d", 
                     OM_CS_DEPLOY_CL_TASKINFO, rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, 1, buffObj, startingPos, cb, _pRtnCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               goto done ;
            }

            contextID = -1 ;
            PD_LOG_MSG( PDERROR, "failed to get record from table:%s,rc=%d", 
                        OM_CS_DEPLOY_CL_TASKINFO, rc ) ;
            goto error ;
         }

         BSONObj record( buffObj.data() ) ;
         _omTaskInfo._agentHostName = record.getStringField( 
                                              OM_TASKINFO_FIELD_AGENTHOST ) ;
         _omTaskInfo._agentSvcName  = record.getStringField( 
                                              OM_TASKINFO_FIELD_AGENTSERVICE ) ;
         _omTaskInfo._taskID        = record.getStringField( 
                                              OM_TASKINFO_FIELD_TASKID ) ;
         _omTaskInfo._isAllFinished = false ;
          _omTaskInfo._status       = record.getIntField( 
                                              OM_TASKINFO_FIELD_STATUS ) ;
         _omTaskInfo._confValue     = record.getObjectField( 
                                              OM_TASKINFO_FIELD_INFO ) ;
         _omTaskInfo._detail        = "" ;
         _omTaskInfo._progress      = BSONObj() ;
      }

   done:
      return rc ;
   error:
      if ( -1 != contextID )
      {
         _pRtnCB->contextDelete ( contextID, cb ) ;
      }
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


