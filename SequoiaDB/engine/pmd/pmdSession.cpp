/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/04/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdSession.hpp"
#include "pmdEDU.hpp"
#include "pmdCommon.hpp"
#include "msgMessage.hpp"
#include "rtn.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"

using namespace bson ;

namespace engine
{
   const UINT32 SESSION_SOCKET_DFT_TIMEOUT = 10 ;
   const UINT32 SESSION_MEM_ALIGMENT_SIZE  = 1024 ;
   const UINT32 SESSION_MAX_CATCH_SIZE     = 16*1024*1024 ;

   /*
      _pmdSession implement
   */

   // message map define
   BEGIN_OBJ_MSG_MAP( _pmdSession, _clsObjBase )

   END_OBJ_MSG_MAP()

   _pmdSession::_pmdSession( SOCKET fd )
   :_socket( &fd, SESSION_SOCKET_DFT_TIMEOUT )
   {
      _pEDUCB  = NULL ;
      _eduID   = PMD_INVALID_EDUID ;
      _pBuff   = NULL ;
      _buffLen = 0 ;

      _totalCatchSize   = 0 ;
      _totalMemSize     = 0 ;

      _socket.disableNagle() ;

      // make session name
      CHAR tmpName [ 128 ] = {0} ;
      _socket.getPeerAddress( tmpName, sizeof( tmpName ) -1 ) ;
      _sessionName = tmpName ;
      _sessionName += ":" ;
      ossSnprintf( tmpName, sizeof( tmpName ) -1, "%d",
                   _socket.getPeerPort() ) ;
      _sessionName += tmpName ;
   }

   _pmdSession::~_pmdSession()
   {
      clear() ;
   }

   void _pmdSession::clear ()
   {
      // release buff
      if ( _pBuff )
      {
         SDB_OSS_FREE( _pBuff ) ;
         _pBuff = NULL ;
      }
      _buffLen = 0 ;

      // clean catch
      CATCH_MAP_IT it = _catchMap.begin() ;
      while ( it != _catchMap.end() )
      {
         SDB_OSS_FREE( it->second ) ;
         _totalCatchSize -= it->first ;
         _totalMemSize -= it->first ;
         ++it ;
      }
      _catchMap.clear() ;
      SDB_ASSERT( _totalCatchSize == 0 , "Catch size is error" ) ;
   }

   void _pmdSession::attach( _pmdEDUCB * cb )
   {
      SDB_ASSERT( cb, "cb can't be NULL" ) ;
      _pEDUCB = cb ;
      _eduID  = cb->getID() ;
      _onAttach() ;
   }

   void _pmdSession::dettach ()
   {
      _onDetach() ;
      _pEDUCB = NULL ;
   }

   const CHAR* _pmdSession::sessionName () const
   {
      return _sessionName.c_str() ;
   }

   INT32 _pmdSession::allocBuff( INT32 len, CHAR **ppBuff, INT32 &buffLen )
   {
      INT32 rc = SDB_OK ;

      // first alloc from catch
      if ( _totalCatchSize >= len && _allocFromCatch( len, ppBuff, buffLen ) )
      {
         goto done ;
      }

      // malloc
      len = ossRoundUpToMultipleX( len, SESSION_MEM_ALIGMENT_SIZE ) ;
      *ppBuff = ( CHAR* )SDB_OSS_MALLOC( len ) ;
      if( !**ppBuff )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Session[%s] malloc memory[size: %d] failed",
                 sessionName(), len ) ;
         goto error ;
      }
      buffLen = len ;

      // update meta info
      _totalMemSize += buffLen ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _pmdSession::releaseBuff( CHAR *pBuff, INT32 buffLen )
   {
      if ( buffLen > SESSION_MAX_CATCH_SIZE )
      {
         SDB_OSS_FREE( pBuff ) ;
         _totalMemSize -= buffLen ;
      }
      else
      {
         // add to catch
         _catchMap.insert( std::make_pair( buffLen, pBuff ) ) ;
         _totalCatchSize += buffLen ;

         // re-org catch
         while ( _totalCatchSize > SESSION_MAX_CATCH_SIZE )
         {
            CATCH_MAP_IT it = _catchMap.begin() ;
            SDB_OSS_FREE( it->second ) ;
            _totalMemSize -= it->first ;
            _totalCatchSize -= it->first ;
            _catchMap.erase( it ) ;
         }
      }
   }

   CHAR* _pmdSession::getBuff( INT32 len )
   {
      if ( _buffLen < len )
      {
         if ( _pBuff )
         {
            SDB_OSS_FREE( _pBuff ) ;
            _pBuff = NULL ;
         }
         _buffLen = 0 ;

         len = ossRoundUpToMultipleX( len, SESSION_MEM_ALIGMENT_SIZE ) ;
         _pBuff = ( CHAR* )SDB_OSS_MALLOC( len ) ;
         if ( !_pBuff )
         {
            PD_LOG( PDERROR, "Session[%s] alloc memory[size: %d] failed",
                    sessionName(), len ) ;
            goto error ;
         }
      }

   done:
      return _pBuff ;
   error:
      goto done ;
   }

   BOOLEAN _pmdSession::_allocFromCatch( INT32 len, CHAR **ppBuff,
                                         INT32 &buffLen )
   {
      CATCH_MAP_IT it = _catchMap.lower_bound( len ) ;
      if ( it != _catchMap.end() )
      {
         *ppBuff = it->second ;
         buffLen = it->first ;
         _catchMap.erase( it ) ;
         _totalCatchSize -= buffLen ;
         return TRUE ;
      }
      return FALSE ;
   }

   void _pmdSession::disconnect()
   {
      _socket.close() ;
   }

   INT32 _pmdSession::sendData( const CHAR * pData, INT32 size,
                                INT32 timeout, BOOLEAN block,
                                INT32 flags )
   {
      INT32 rc = SDB_OK ;
      INT32 sentSize = 0 ;
      INT32 totalSentSize = 0 ;
      INT32 realTimeout = timeout < 0 ? OSS_SOCKET_DFT_TIMEOUT : timeout ;

      while ( TRUE )
      {
         if ( _pEDUCB && _pEDUCB->isForced () )
         {
            rc = SDB_APP_FORCED ;
            goto done ;
         }
         rc = _socket.send ( &pData[totalSentSize], size-totalSentSize,
                             sentSize, realTimeout, flags, block ) ;
         totalSentSize += sentSize ;
         if ( timeout < 0 && SDB_TIMEOUT == rc )
         {
            continue ;
         }
         break ;
      }

   done :
      if ( totalSentSize > 0 )
      {
         pmdGetKRCB()->getMonDBCB()->svcNetOutAdd( totalSentSize ) ;
      }
      return rc ;
   }

   INT32 _pmdSession::recvData( CHAR * pData, INT32 size, INT32 timeout,
                                BOOLEAN block, INT32 flags )
   {
      INT32 rc = SDB_OK ;
      INT32 receivedSize = 0 ;
      INT32 totalReceivedSize = 0 ;
      INT32 realTimeout = timeout < 0 ? OSS_SOCKET_DFT_TIMEOUT : timeout ;

      while ( TRUE )
      {
         if ( _pEDUCB && _pEDUCB->isForced () )
         {
            rc = SDB_APP_FORCED ;
            goto done ;
         }
         rc = _socket.recv ( &pData[totalReceivedSize], size-totalReceivedSize,
                             receivedSize, realTimeout, flags, block ) ;
         totalReceivedSize += receivedSize ;
         if ( timeout < 0 && SDB_TIMEOUT == rc )
         {
            continue ;
         }
         break ;
      }

   done :
      if ( totalReceivedSize > 0 )
      {
         pmdGetKRCB()->getMonDBCB()->svcNetInAdd( totalReceivedSize ) ;
      }
      return rc ;
   }

   /*
      _pmdLocalSession implement
   */

   BEGIN_OBJ_MSG_MAP( _pmdLocalSession, pmdSession )

   END_OBJ_MSG_MAP()

   _pmdLocalSession::_pmdLocalSession( SOCKET fd )
   :pmdSession( fd )
   {
      _authOK  = FALSE ;
   }

   _pmdLocalSession::~_pmdLocalSession()
   {
   }

   UINT64 _pmdLocalSession::identifyID()
   {
      return ossPack32To64( _socket.getLocalIP(), _socket.getLocalPort() ) ;
   }

   INT32 _pmdLocalSession::_defaultMsgFunc( NET_HANDLE handle, MsgHeader *msg )
   {
      return SDB_OK ;
   }

   INT32 _pmdLocalSession::_onAuth( MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;

      if ( SDB_ROLE_STANDALONE == pmdGetDBRole() ) // not auth
      {
         _authOK = TRUE ;
         goto done ;
      }
      else
      {
         MsgHeader *pAuthRes = NULL ;
         shardCB *pShard = pmdGetKRCB()->getShardCB() ;
         BOOLEAN hasRetry = FALSE ;

         while ( TRUE )
         {
            rc = pShard->syncSend( msg, CATALOG_GROUPID, TRUE, &pAuthRes ) ;
            if ( SDB_OK != rc )
            {
               rc = pShard->syncSend( msg, CATALOG_GROUPID, FALSE, &pAuthRes ) ;
               PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to send auth "
                            "req to catalog, rc=%d", sessionName(), rc ) ;
            }
            if ( NULL == pAuthRes )
            {
               rc = SDB_SYS ;
               PD_LOG( PDERROR, "syncsend return ok but res is NULL" ) ;
               goto error ;
            }
            rc = (( MsgInternalReplyHeader *)pAuthRes)->res ;
            SDB_OSS_FREE( (BYTE*)pAuthRes ) ;
            pAuthRes = NULL ;

            if ( SDB_CLS_NOT_PRIMARY == rc && !hasRetry )
            {
               hasRetry = TRUE ;
               pShard->updateCatGroup( TRUE, CLS_SHARD_TIMEOUT ) ;
               continue ;
            }
            else if ( rc )
            {
               PD_LOG( PDERROR, "Session[%s] auth failed, rc: %d",
                       sessionName(), rc ) ;
               goto error ;
            }
            else
            {
               _authOK = TRUE ;
            }
            break ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onSysInfoRequest( const CHAR * msg )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN endianConvert = FALSE ;
      MsgSysInfoReply reply ;

      MsgSysInfoReply *pReply = &reply ;
      INT32 replySize = sizeof(reply) ;

      rc = msgExtractSysInfoRequest ( (CHAR*)msg, endianConvert ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to extract sys info "
                    "request, rc = %d", sessionName(), rc ) ;

      // reply
      rc = msgBuildSysInfoReply ( (CHAR**)&pReply, &replySize ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to build sys info reply, "
                    "rc = %d", sessionName(), rc ) ;

      rc = sendData ( (const CHAR*)pReply, replySize ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to send packet, rc = %d",
                    sessionName(), rc ) ;

   done :
      return rc ;
   error :
      disconnect() ;
      goto done ;
   }

   INT32 _pmdLocalSession::_onOPMsg( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;


   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onInsertReqMsg( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pInsertor = NULL ;
      INT32 count = 0 ;

      rc = msgExtractInsert( (CHAR *)msg, &flag, &pCollectionName,
                             &pInsertor, count ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extrace insert msg failed, rc: %d",
                   sessionName(), rc ) ;

      try
      {
         BSONObj insertor( pInsertor ) ;
         // add list op info
         MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                             "CL:%s, Insertors:%s, count: %d",
                             pCollectionName,
                             insertor.toString().c_str(),
                             count ) ;

         PD_LOG ( PDDEBUG, "Session[%s] insert objs: %s\ncount: %d\n"
                  "collection: %s", sessionName(), insertor.toString().c_str(),
                  count, pCollectionName ) ;
 
         rc = rtnInsert( pCollectionName, insertor, count, flag, _pEDUCB ) ;
         PD_RC_CHECK( rc, PDERROR, "Session[%s] insert objs[%s, count:%d, "
                      "collection: %s] failed, rc: %d", sessionName(),
                      insertor.toString().c_str(), count, pCollectionName,
                      rc ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Session[%s] insert objs occur exception: %s",
                 sessionName(), e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onUpdateReqMsg( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flags = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pSelectorBuffer = NULL ;
      CHAR *pUpdatorBuffer = NULL ;
      CHAR *pHintBuffer = NULL ;

      rc = msgExtractUpdate( (CHAR*)msg, &flags, &pCollectionName,
                             &pSelectorBuffer, &pUpdatorBuffer,
                             &pHintBuffer );
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract update message failed, "
                   "rc: %d", sessionName(), rc ) ;

      try
      {
         BSONObj selector( pSelectorBuffer );
         BSONObj updator( pUpdatorBuffer );
         BSONObj hint( pHintBuffer );
         // add last op info
         MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                             "CL:%s, Match:%s, Updator:%s, Hint:%s",
                             pCollectionName,
                             selector.toString().c_str(),
                             updator.toString().c_str(),
                             hint.toString().c_str() ) ;

         PD_LOG ( PDDEBUG, "Session[%s] Update: selctor: %s\nupdator: %s\n"
                  "hint: %s", sessionName(), selector.toString().c_str(),
                  updator.toString().c_str(), hint.toString().c_str() ) ;

         rc = rtnUpdate( pCollectionName, selector, updator, hint, 
                         flags, _pEDUCB, _pDMSCB, _pDPSCB ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Session[%s] Failed to create selector and updator "
                  "for update: %s", sessionName(), e.what () ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onDelReqMsg( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;
      INT32 flags = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pDeletorBuffer = NULL ;
      CHAR *pHintBuffer = NULL ;

      rc = msgExtractDelete ( (CHAR *)msg , &flags, &pCollectionName, 
                              &pDeletorBuffer, &pHintBuffer ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract delete msg failed, rc: %d",
                   sessionName(), rc ) ;

      try
      {
         BSONObj deletor ( pDeletorBuffer ) ;
         BSONObj hint ( pHintBuffer ) ;
         // add last op info
         MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                            "CL:%s, Deletor:%s, Hint:%s",
                            pCollectionName,
                            deletor.toString().c_str(),
                            hint.toString().c_str() ) ;

         PD_LOG ( PDDEBUG, "Session[%s] Delete: deletor: %s\nhint: %s",
                  sessionName(), deletor.toString().c_str(), 
                  hint.toString().c_str() ) ;

         rc = rtnDelete( pCollectionName, deletor, hint, flags, _pEDUCB, 
                         _pDMSCB, _pDPSCB ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Session[%s] Failed to create deletor for "
                  "DELETE: %s", sessionName(), e.what () ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onInterruptMsg( NET_HANDLE handle, MsgHeader * msg )
   {
      PD_LOG ( PDEVENT, "Session[%s] recieved interrupt msg", sessionName() ) ;

      // delete all contextID, rollback transaction
      if ( _pEDUCB )
      {
         INT64 contextID = -1 ;
         while ( -1 != ( contextID = _pEDUCB->contextPeek() ) )
         {
            _pRTNCB->contextDelete ( contextID, NULL ) ;
         }

         INT32 rcTmp = rtnTransRollback( _pEDUCB, _pDPSCB );
         if ( rcTmp )
         {
            PD_LOG ( PDERROR, "Failed to rollback(rc=%d)", rcTmp );
         }
         _pEDUCB->clearTransInfo() ;
      }

      return SDB_OK ;
   }

   INT32 _pmdLocalSession::_onMsgReqMsg( NET_HANDLE handle, MsgHeader * msg )
   {
      return rtnMsg( (MsgOpMsg*)msg ) ;
   }

   INT32 _pmdLocalSession::_onQueryReqMsg( NET_HANDLE handle, MsgHeader * msg,
                                           INT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      INT32 flags = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pQueryBuff = NULL ;
      CHAR *pFieldSelector = NULL ;
      CHAR *pOrderByBuffer = NULL ;
      CHAR *pHintBuffer = NULL ;
      INT64 numToSkip = -1 ;
      INT64 numToReturn = -1 ;
      _rtnCommand *pCommand = NULL ;

      rc = msgExtractQuery ( (CHAR *)msg, &flags, &pCollectionName,
                             &numToSkip, &numToReturn, &pQueryBuff,
                             &pFieldSelector, &pOrderByBuffer, &pHintBuffer ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract query msg failed, rc: %d",
                   sessionName(), rc ) ;

      if ( !rtnIsCommand ( pCollectionName ) )
      {
         try
         {
            BSONObj matcher ( pQueryBuff ) ;
            BSONObj selector ( pFieldSelector ) ;
            BSONObj orderBy ( pOrderByBuffer ) ;
            BSONObj hint ( pHintBuffer ) ;
            // add last op info
            MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                               "CL:%s, Match:%s, Selector:%s, OrderBy:%s, "
                               "Hint:%s", pCollectionName,
                               matcher.toString().c_str(),
                               selector.toString().c_str(),
                               orderBy.toString().c_str(),
                               hint.toString().c_str() ) ;

            PD_LOG ( PDDEBUG, "Session[%s] Query: matcher: %s\nselector: "
                     "%s\norderBy: %s\nhint:%s", sessionName(),
                     matcher.toString().c_str(), selector.toString().c_str(),
                     orderBy.toString().c_str(), hint.toString().c_str() ) ;

            rc = rtnQuery( pCollectionName, selector, matcher, orderBy,
                           hint, flags, _pEDUCB, numToSkip, numToReturn,
                           _pDMSCB, _pRTNCB, contextID, NULL, TRUE ) ;
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR, "Session[%s] Failed to create matcher and "
                     "selector for QUERY: %s", sessionName(), e.what () ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      else
      {
         rc = rtnParserCommand( pCollectionName, &pCommand ) ;

         if ( SDB_OK != rc )
         {
            PD_LOG ( PDERROR, "Parse command[%s] failed[rc:%d]",
                     pCollectionName, rc ) ;
            goto error ;
         }

         rc = rtnInitCommand( pCommand , flags, numToSkip, numToReturn,
                              pQueryBuff, pFieldSelector, pOrderByBuffer,
                              pHintBuffer ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         PD_LOG ( PDDEBUG, "Command: %s", pCommand->name () ) ;

         //run command
         rc = rtnRunCommand( pCommand, CMD_SPACE_SERVICE_LOCAL,
                             _pEDUCB, _pDMSCB, _pRTNCB,
                             _pDPSCB, 1, &contextID ) ;
      }

   done:
      if ( pCommand )
      {
         rtnReleaseCommand( &pCommand ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onGetMoreReqMsg( MsgHeader * msg,
                                             rtnContextBuf &buffObj,
                                             INT32 &startingPos,
                                             INT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      INT32 numToRead = 0 ;
      INT64 startPos64 = 0 ;

      rc = msgExtractGetMore ( (CHAR*)msg, &numToRead, &contextID ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract get more msg failed, "
                   "rc: %d", sessionName(), rc ) ;

      // add last op info
      MON_SAVE_OP_DETAIL( _pEDUCB->getMonAppCB(), msg->opCode,
                          "ContextID:%lld, NumToRead:%d",
                          contextID, numToRead ) ;

      PD_LOG ( PDDEBUG, "GetMore: contextID:%lld\nnumToRead: %d", contextID,
               numToRead ) ;

      rc = rtnGetMore ( contextID, numToRead, buffObj, startPos64,
                        _pEDUCB, _pRTNCB ) ;

      startingPos = ( INT32 )startPos64 ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onKillContextsReqMsg( NET_HANDLE handle,
                                                  MsgHeader *msg )
   {
      PD_LOG ( PDDEBUG, "session[%s] _onKillContextsReqMsg", sessionName() ) ;

      INT32 rc = SDB_OK ;
      INT32 contextNum = 0 ;
      INT64 *pContextIDs = NULL ;

      rc = msgExtractKillContexts ( (CHAR*)msg, &contextNum, &pContextIDs ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract kill contexts msg failed, "
                   "rc: %d", sessionName(), rc ) ;

      if ( contextNum > 0 )
      {
         PD_LOG ( PDDEBUG, "KillContext: contextNum:%d\ncontextID: %lld",
                  contextNum, pContextIDs[0] ) ;
      }

      rc = rtnKillContexts ( contextNum, pContextIDs, _pEDUCB, _pRTNCB ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onSQLMsg( NET_HANDLE handle, MsgHeader *msg,
                                      INT64 &contextID )
   {
      CHAR *sql = NULL ;
      INT32 rc = SDB_OK ;
      SQL_CB *sqlcb = pmdGetKRCB()->getSqlCB() ;

      rc = msgExtractSql( (CHAR*)msg, &sql ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extract sql msg failed, rc: %d",
                   sessionName(), rc ) ;

      rc = sqlcb->exec( sql, _pEDUCB, contextID ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onTransBeginMsg ()
   {
      INT32 rc = SDB_OK ;
      if ( pmdGetDBRole() != SDB_ROLE_STANDALONE )
      {
         rc = SDB_PERM ;
         PD_LOG( PDERROR, "In sharding mode, couldn't execute "
                 "transaction operation from local service" ) ;
         goto error ;
      }
      else
      {
         rc = rtnTransBegin( _pEDUCB ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onTransCommitMsg ()
   {
      INT32 rc = SDB_OK ;
      if ( pmdGetDBRole() != SDB_ROLE_STANDALONE )
      {
         rc = SDB_PERM ;
         PD_LOG( PDERROR, "In sharding mode, couldn't execute "
                 "transaction operation from local service" ) ;
         goto error ;
      }
      else
      {
         rc = rtnTransCommit( _pEDUCB, _pDPSCB ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onTransRollbackMsg ()
   {
      INT32 rc = SDB_OK ;
      if ( pmdGetDBRole() != SDB_ROLE_STANDALONE )
      {
         rc = SDB_PERM ;
         PD_LOG( PDERROR, "In sharding mode, couldn't execute "
                 "transaction operation from local service" ) ;
         goto error ;
      }
      else
      {
         rc = rtnTransRollback( _pEDUCB, _pDPSCB ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onAggrReqMsg( NET_HANDLE handle, MsgHeader *msg,
                                          INT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      CHAR *pCollectionName = NULL ;
      CHAR *pObjs = NULL ;
      INT32 count = 0 ;
      INT32 flags = 0 ;

      rc = msgExtractAggrRequest( (CHAR*)msg, &pCollectionName,
                                  &pObjs, count, &flags ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[%s] extrace aggr msg failed, rc: %d",
                   sessionName(), rc ) ;

      try
      {
         BSONObj objs( pObjs ) ;
         rc = rtnAggregate( pCollectionName, objs, count, flags, _pEDUCB,
                            _pDMSCB, contextID ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Session[%s] occurred exception in aggr: %s",
                 sessionName(), e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

}


