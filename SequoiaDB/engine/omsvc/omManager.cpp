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
#include "rtn.hpp"


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
      rc = rtnTestCollectionSpaceCommand( OM_CS_DEPLOY, _pDmsCB ) ;
      if ( rc )
      {
         if ( SDB_DMS_CS_NOTEXIST == rc )
         {
            // if collection space was not exist, let's create one
            rc = rtnCreateCollectionSpaceCommand ( OM_CS_DEPLOY, cb, _pDmsCB, 
                                                   NULL, DMS_PAGE_SIZE_DFT, 
                                                   TRUE, FALSE ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to create %s collection space, rc = %d",
                          OM_CS_DEPLOY, rc ) ;
         }
         else
         {
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to test collection space %s, rc = %d",
                          OM_CS_DEPLOY, rc ) ;
         }
      }

      // SYSDEPLOY.SYSCLUSTER
      rc = _createCollection ( OM_CS_DEPLOY_CL_CLUSTER, cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create collection %s, rc = %d",
                    OM_CS_DEPLOY_CL_CLUSTER, rc ) ;

      //TODO: check the index's content to make sure the index key is the same
      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_CLUSTER,
                                    OM_CS_DEPLOY_CL_CLUSTERIDX1, cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create index %s, rc = %d",
                    OM_CS_DEPLOY_CL_CLUSTERIDX1, rc ) ;

      // SYSDEPLOY.SYSHOST
      rc = _createCollection ( OM_CS_DEPLOY_CL_HOST, cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create collection %s, rc = %d",
                    OM_CS_DEPLOY_CL_HOST, rc ) ;

      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_HOST,
                                    OM_CS_DEPLOY_CL_HOSTIDX1, cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create index %s, rc = %d",
                    OM_CS_DEPLOY_CL_HOSTIDX1, rc ) ;

      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_HOST,
                                    OM_CS_DEPLOY_CL_HOSTIDX2, cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create index %s, rc = %d",
                    OM_CS_DEPLOY_CL_HOSTIDX2, rc ) ;

      // SYSDEPLOY.SYSBUSINESS
      rc = _createCollection ( OM_CS_DEPLOY_CL_BUSINESS, cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create collection %s, rc = %d",
                    OM_CS_DEPLOY_CL_BUSINESS, rc ) ;

      rc = _createCollectionIndex ( OM_CS_DEPLOY_CL_BUSINESS,
                             OM_CS_DEPLOY_CL_BUSINESSIDX1, cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create index %s, rc = %d",
                    OM_CS_DEPLOY_CL_BUSINESSIDX1, rc ) ;

      // SYSDEPLOY.SYSCONFIGURE
      rc = _createCollection ( OM_CS_DEPLOY_CL_CONFIGURE, cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create collection %s, rc = %d",
                    OM_CS_DEPLOY_CL_CONFIGURE, rc ) ;

      pAuthCB = pmdGetKRCB()->getAuthCB() ;
      pAuthCB->checkNeedAuth( cb, TRUE ) ;
      if ( !pAuthCB->needAuthenticate() )
      {
         BSONObj obj ;
         bsonBuilder.append( SDB_AUTH_USER, OM_DEFAULT_LOGIN_USER ) ;
         bsonBuilder.append( SDB_AUTH_PASSWD, OM_DEFAULT_LOGIN_PASSWD ) ;
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
      
      // attempt to create index, NULL for dpscb for no-logging
      rc = rtnCreateIndexCommand ( pCollection,
                                   indexDef, cb, _pDmsCB, NULL, TRUE ) ;
      if ( rc )
      {
         // if index already exist, let's set rc = ok
         if ( SDB_IXM_REDEF == rc )
         {
            rc = SDB_OK ;
         }
         else
         {
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to create index for %s\n"
                          "Index def: %s\nrc = %d",
                          pCollection, pIndex, rc ) ;
         }
      }
      
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _omManager::_createCollection ( const CHAR *pCollection, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      rc = rtnFindCollection ( pCollection, _pDmsCB ) ;
      if ( rc )
      {
         if ( SDB_DMS_NOTEXIST == rc )
         {
            // if the collection does not exist, let's create one
            // NULL for dpscb to disable logging, no compression
            rc = rtnCreateCollectionCommand ( pCollection,
                                              0, cb, _pDmsCB, NULL, 0,
                                              TRUE ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to create %s collection, rc = %d",
                          pCollection, rc ) ;
         }
         else
         {
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to find collection %s, rc = %d",
                          pCollection, rc ) ;
         }
      }
      
   done :
      return rc ;
   error :
      goto done ;
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

   INT32 _omManager::sendMsgToAgent( const CHAR * pHost, MsgHeader *pMsg )
   {
      return SDB_OK ;
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


