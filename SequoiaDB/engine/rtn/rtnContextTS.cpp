/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = rtnContextTS.cpp

   Descriptive Name = RunTime Text Search Context

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/30/2017  YSD Split from rtnContextData.cpp

   Last Changed =

*******************************************************************************/
#include "rtnContextTS.hpp"
#include "rtn.hpp"
#include "pmdController.hpp"

namespace engine
{
   _rtnRSHandler::_rtnRSHandler()
   {
   }

   _rtnRSHandler::~_rtnRSHandler()
   {
   }

   INT32 _rtnRSHandler::onSendFailed( _pmdRemoteSession *pSession,
                                      _pmdSubSession **ppSub,
                                      INT32 flag )
   {
      return SDB_OK ;
   }

   void _rtnRSHandler::onReply( _pmdRemoteSession *pSession,
                                _pmdSubSession **ppSub,
                                const MsgHeader *pReply,
                                BOOLEAN isPending )
   {

   }

   INT32 _rtnRSHandler::onSendConnect( _pmdSubSession *pSub,
                                       const MsgHeader *pReq,
                                       BOOLEAN isFirst )
   {
      return SDB_OK ;
   }

   RTN_CTX_AUTO_REGISTER( _rtnContextTS, RTN_CONTEXT_TS, "TS")

   _rtnContextTS::_rtnContextTS( INT64 contextID, UINT64 eduID )
   : _rtnContextBase( contextID, eduID )
   {
      _eduCB = NULL ;
      _remoteSessionSite = NULL ;
      _remoteSession = NULL ;
      _subCtxID = -1 ;
      _extNodeId = 0 ;
   }

   _rtnContextTS::~_rtnContextTS()
   {
      if ( _remoteSession )
      {
         _remoteSession->clearSubSession() ;
         if ( _remoteSessionSite )
         {
            _remoteSessionSite->removeSession( _remoteSession ) ;
         }
      }

      if ( sdbGetPMDController()->getRSManager() && _eduCB )
      {
         sdbGetPMDController()->getRSManager()->unregEUD( _eduCB ) ;
      }

      if ( -1 != _subCtxID )
      {
         pmdGetKRCB()->getRTNCB()->contextDelete( _subCtxID, _eduCB ) ;
      }
   }

   std::string _rtnContextTS::name() const
   {
      return "TS" ;
   }

   RTN_CONTEXT_TYPE _rtnContextTS::getType() const
   {
      return RTN_CONTEXT_TS ;
   }

   _dmsStorageUnit* _rtnContextTS::getSU()
   {
      return NULL ;
   }

   INT32 _rtnContextTS::open( const rtnQueryOptions &options,
                                  pmdEDUCB *eduCB )
   {
      INT32 rc = SDB_OK ;
      MsgHeader *queryMsg = NULL ;
      INT32 msgSize = 0 ;

      SDB_ASSERT( eduCB, "eduCB should not be NULL" ) ;

      _isOpened = TRUE ;
      _hitEnd = FALSE ;

      rc = _prepareRemoteSession( eduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Prepare remote session failed[ %d ]", rc ) ;

      // 1. Store the query items.
      // 2. Send the query to search engine adapter, and get the replay. Then
      //    call query interface again, to get a sub context id.
      _options = options ;
      rc = _options.getOwned() ;
      PD_RC_CHECK( rc, PDERROR, "Get owned of query options failed[ %d ]",
                   rc ) ;

      PD_LOG( PDDEBUG, "Options for search: %s", options.toString().c_str() ) ;

      // Format the message, and send it to search engine adapter.
      rc = options.toQueryMsg( (CHAR **)&queryMsg, msgSize, eduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build query message from options failed[ %d ]",
                   rc ) ;

      queryMsg->opCode = MSG_BS_QUERY_REQ ;

      // Send query message to search engine adapter.
      rc = _sendToRemote( queryMsg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send query message to remote failed[ %d ]",
                   rc ) ;

      rc = _waitAndProcessRemoteReply() ;
      PD_RC_CHECK( rc, PDERROR, "Wait and process reply from search engine "
                   "adapter failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      _isOpened = FALSE ;
      _hitEnd = TRUE ;
      goto done ;
   }

   INT32 _rtnContextTS::_prepareData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      rtnContextBuf objBuff ;

      // 1. Get the OIDs from search engine.
      // 2. Reorgnize the query.
      // 3. Do the query with a sub context and fetch the data.

      // get the final data records, and call rtnContextBase::append().

      rc = rtnGetMore( _subCtxID, _options._limit, objBuff, cb, rtnCB ) ;
      if ( rc )
      {
         // If the return code is EOC, get another query from search engine
         // adapter.
         if ( SDB_DMS_EOC == rc )
         {
            rtnCB->contextDelete( _subCtxID, cb ) ;
            _subCtxID = -1 ;

            // Another new query will be started, and in case of query success,
            // we can go on to get more data.
            rc = _getMoreFromRemote( cb ) ;
            PD_RC_CHECK( rc, PDERROR, "Get more from remote failed[ %d ]",
                         rc ) ;
            rc = rtnGetMore( _subCtxID, _options._limit, objBuff, cb, rtnCB ) ;
            PD_RC_CHECK( rc, PDERROR, "Get more data failed[ %d ]", rc ) ;
         }
         else
         {
            PD_LOG( PDERROR, "Get more data failed[ %d ]", rc ) ;
            goto error ;
         }
      }

      // Append the results to the result buffer, when get more succeed.
      rc = appendObjs( objBuff.data(), objBuff.size(),
                       objBuff.recordNum(), TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Append objects to text search context "
                   "failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextTS::_prepareRemoteSession( pmdEDUCB *eduCB )
   {
      INT32 rc = SDB_OK ;
      pmdRemoteSessionMgr *rsMgr = NULL ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      // Register a remote session.
      rsMgr = sdbGetPMDController()->getRSManager() ;
      SDB_ASSERT( rsMgr, "Remote session manager is not able to be NULL" ) ;

      // Register the current edu in remote session manager.
      _remoteSessionSite = rsMgr->registerEDU( eduCB ) ;
      _eduCB = eduCB ;

      _remoteSession = _remoteSessionSite->addSession( -1, &_rsHandler ) ;
      if ( !_remoteSession )
      {
         PD_LOG( PDERROR, "Add session to session site failed" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _extNodeId = rtnCB->getExtNodeId() ;
      if ( 0 == _extNodeId )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "External data service may have exit" ) ;
         goto error ;
      }

      (void)_remoteSession->addSubSession( _extNodeId ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextTS::_getMoreFromRemote( pmdEDUCB *eduCB )
   {
      INT32 rc = SDB_OK ;
      MsgHeader *msg = NULL ;
      INT32 msgSize = 0 ;
      INT32 numToReturn = 0 ;
      INT64 contextID = 0 ;
      UINT64 reqID = 0 ;

      rc = msgBuildGetMoreMsg( (CHAR **)&msg, &msgSize, numToReturn,
                               contextID, reqID, eduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Build get more message failed[ %d ]", rc ) ;

      msg->opCode = MSG_BS_GETMORE_REQ ;

      rc = _sendToRemote( msg ) ;
      PD_RC_CHECK( rc, PDERROR, "Send get more message to remote failed[ %d ]",
                   rc ) ;

      rc = _waitAndProcessRemoteReply() ;
      PD_RC_CHECK( rc, PDERROR, "Wait and process reply for get more from "
                   "search engine adapter failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextTS::_sendToRemote( const MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;
      pmdSubSession *subSession = NULL ;

      subSession = _remoteSession->getSubSession( _extNodeId ) ;
      if ( !subSession )
      {
         PD_LOG( PDERROR, "Sub session does not exist" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      subSession->setReqMsg( (MsgHeader *)msg, PMD_EDU_MEM_NONE ) ;
      subSession->resetForResend() ;
      rc = _remoteSession->sendMsg( subSession ) ;
      PD_RC_CHECK( rc, PDERROR, "Send message to search engine adapter "
                   "failed[ %d ]", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextTS::_waitAndProcessRemoteReply()
   {
      INT32 rc = SDB_OK ;
      MsgOpReply *reply = NULL ;
      pmdSubSession *subSession = NULL ;
      INT32 flag = 0 ;
      INT64 contextID = 0 ;
      INT32 startFrom = 0 ;
      INT32 numReturned = 0 ;
      vector< BSONObj > objList ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      rc = _remoteSession->waitReply( TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait reply failed[ %d ]", rc ) ;

      subSession = _remoteSession->getSubSession( _extNodeId ) ;
      if ( !subSession )
      {
         PD_LOG( PDERROR, "Get subsession of search engine adapter "
                 "failed[ %d ]", rc ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      reply = (MsgOpReply *)subSession->getRspMsg( FALSE ) ;

      rc = msgExtractReply( (CHAR *)reply, &flag, &contextID, &startFrom,
                            &numReturned, objList ) ;
      PD_RC_CHECK( rc, PDERROR, "Extract query respond message failed[ %d ]",
                   rc ) ;
      // 4 objects are expected: matcher, selector, order by, hint.
      if ( objList.size() != 4 )
      {
         PD_LOG( PDERROR, "Respond message size is wrong, expect[ %d ], "
                 "actual[ %d ]", 4, objList.size() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // Do a query, and get another subcontext.
      rc = rtnQuery( _options._fullName, objList[1], objList[0], objList[2],
                     objList[3], _options._flag, _remoteSession->getEDUCB(),
                     _options._skip, _options._limit, dmsCB, rtnCB,
                     _subCtxID ) ;
      PD_RC_CHECK( rc, PDERROR, "Query data on collection[ %s ] failed[ %d ]",
                   _options._fullName, rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }
}

