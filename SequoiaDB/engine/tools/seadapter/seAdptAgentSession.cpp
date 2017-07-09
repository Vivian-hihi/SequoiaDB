#include "seAdptAgentSession.hpp"
#include "rtnContextBuff.hpp"
#include "msgMessage.hpp"

namespace engine
{
   #define TEXT_QUERY_COND_KEY      "$text"

   BEGIN_OBJ_MSG_MAP( _seAdptAgentSession, _pmdAsyncSession)
      ON_MSG( MSG_AUTH_VERIFY_REQ, _onAuthReq )
      ON_MSG( MSG_BS_QUERY_REQ, _onOPMsg )
      ON_MSG( MSG_BS_GETMORE_REQ, _onOPMsg )
   END_OBJ_MSG_MAP()

   _seAdptAgentSession::_seAdptAgentSession( UINT64 sessionID )
   : _pmdAsyncSession( sessionID )
   {
      _seCltMgr = sdbGetSeCltMgr() ;
      _esClt = NULL ;
      _context = NULL ;
   }

   _seAdptAgentSession::~_seAdptAgentSession()
   {
   }

   SDB_SESSION_TYPE _seAdptAgentSession::sessionType() const
   {
      return SDB_SESSION_SE_AGENT ;
   }

   EDU_TYPES _seAdptAgentSession::eduType() const
   {
      return EDU_TYPE_SE_AGENT ;
   }

   void _seAdptAgentSession::onRecieve( const NET_HANDLE netHandle,
                                        MsgHeader *msg )
   {
   }

   BOOLEAN _seAdptAgentSession::timeout( UINT32 interval )
   {
      return FALSE ;
   }

   void _seAdptAgentSession::onTimer( UINT64 timerID, UINT32 interval )
   {
   }

   void _seAdptAgentSession::_onAttach()
   {
   }

   void _seAdptAgentSession::_onDetach()
   {
      PD_LOG( PDDEBUG, "Indexer session detached" ) ;
   }

   INT32 _seAdptAgentSession::_onOPMsg( NET_HANDLE handle, MsgHeader *msg )
   {
      INT32 rc = SDB_OK ;

      MsgOpReply reply ;
      BSONObj resultObj;
      const CHAR *msgBody = NULL ;
      INT32 bodySize = 0 ;
      utilCommObjBuff objBuff ;

      PD_LOG( PDEVENT, "Session[ %s ] process message for client thread[ %u ]",
              sessionName(), msg->TID ) ;

      switch ( msg->opCode )
      {
         case MSG_BS_QUERY_REQ:
         case MSG_SEADPT_Q_REWT_REQ:
            rc = _onRewriteQuery( msg, objBuff ) ;
            if ( SDB_OK == rc )
            {
               msgBody = objBuff.data() ;
               bodySize = objBuff.dataSize() ;
            }
            break ;
         case MSG_BS_GETMORE_REQ:
         case MSG_SEADPT_Q_REWT_MORE_REQ:
            rc = _onRewriteQueryMore( msg, objBuff ) ;
            if ( SDB_OK == rc )
            {
               msgBody = objBuff.data() ;
               bodySize = objBuff.dataSize() ;
            }
            break ;
         default:
            break ;

      }

      reply.header.opCode = MAKE_REPLY_TYPE( msg->opCode ) ;
      reply.header.messageLength = sizeof( MsgOpReply ) ;
      reply.header.requestID = msg->requestID ;
      reply.header.TID = msg->TID ;
      reply.header.routeID.value = 0 ;
      reply.numReturned = objBuff.getObjNum() ;
      reply.flags = rc ;

      reply.header.messageLength += bodySize ;
      rc = _reply( &reply, msgBody, bodySize ) ;
      PD_RC_CHECK( rc, PDERROR, "Reply the message failed[ %d ]" ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _seAdptAgentSession::_isCommand ( const CHAR *name )
   {
      if ( name && name[0] == '$' )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   // Generic query request handler.
   INT32 _seAdptAgentSession::_onQueryReqMsg( MsgHeader *msg,
                                              utilCommObjBuff &objBuff )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      CHAR *pCollectionName = NULL ;
      SINT64 numToSkip = 0 ;
      SINT64 numToReturn = 0 ;
      CHAR *pQuery = NULL ;
      CHAR *pFieldSelector = NULL ;
      CHAR *pOrderBy = NULL ;
      CHAR *pHint = NULL ;
      BSONObj query ;
      BSONObj selector ;
      BSONObj orderBy ;
      BSONObj hint ;
      std::string indexName ;
      std::string typeName ;
      std::string result ;

      objBuff.reset() ;
      _scrollID = "" ;

      rc = msgExtractQuery( (CHAR *)msg, &flag, &pCollectionName,
                            &numToSkip, &numToReturn, &pQuery,
                            &pFieldSelector, &pOrderBy, &pHint ) ;
      PD_RC_CHECK( rc, PDERROR, "Parse query request failed[ %d ]", rc ) ;

      if ( _isCommand( pCollectionName ) )
      {
         goto done ;
      }

      try
      {
         BSONObj matcher( pQuery ) ;
         std::string queryCond ;
         PD_LOG( PDEVENT, "Matcher: %s", matcher.toString().c_str() ) ;

         rc = _getQueryCond( matcher, queryCond ) ;
         PD_RC_CHECK( rc, PDERROR, "Get query condition from matcher "
                      "failed[ %d ], matcher: %s",
                      rc, matcher.toString().c_str() ) ;

         if ( !_esClt )
         {
            rc = _seCltMgr->getSeClt( &_esClt ) ;
            PD_RC_CHECK( rc, PDERROR, "Get search engine client failed[ %d ]",
                         rc ) ;
         }

         rc = _getIndexAndType( pCollectionName, pHint,
                                msg->routeID.columns.groupID,
                                indexName, typeName ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Get index name type from message failed[ %d ]", rc ) ;

         if ( _esClt )
         {
            rc = _esClt->initScroll( _scrollID, indexName.c_str(),
                                     typeName.c_str(),
                                     queryCond, objBuff ) ;
            PD_RC_CHECK( rc, PDERROR, "Initialize scroll failed[ %d ]", rc ) ;
         }

         PD_LOG( PDEVENT, "Result: %s", result.c_str() ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Session[%s] failed to create matcher for QUERY: %s",
                  sessionName(), e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // Handle get more message, return more results, until the end.
   INT32 _seAdptAgentSession::_onGetMoreReqMsg( MsgHeader *msg,
                                                utilCommObjBuff &objBuff )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;

      rc = _esClt->scrollNext( _scrollID, objBuff ) ;
      PD_RC_CHECK( rc, PDERROR, "Scroll with id[ %s ] failed[ %d ]",
                   _scrollID.c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;

   }

   // Handle message MSG_SEADPT_Q_REWT_REQ.
   // It will analyze the original query, fetch the neccessary data, and rewrite
   // the items, then return the new message.
   INT32 _seAdptAgentSession::_onRewriteQuery( MsgHeader *msg,
                                               utilCommObjBuff &objBuff,
                                               pmdEDUCB *eduCB )
   {
      INT32 rc = SDB_OK ;
      INT32 flag = 0 ;
      CHAR *pCollectionName = NULL ;
      SINT64 numToSkip = 0 ;
      SINT64 numToReturn = 0 ;
      CHAR *pQuery = NULL ;
      CHAR *pFieldSelector = NULL ;
      CHAR *pOrderBy = NULL ;
      CHAR *pHint = NULL ;
      BSONObj query ;
      BSONObj selector ;
      BSONObj orderBy ;
      BSONObj hint ;

      string indexName ;
      string typeName ;
      utilCommObjBuff resultObjs ;

      rc = msgExtractQuery( (CHAR *)msg, &flag, &pCollectionName,
                            &numToSkip, &numToReturn, &pQuery, &pFieldSelector,
                            &pOrderBy, &pHint ) ;
      PD_RC_CHECK( rc, PDERROR, "Session[ %s ] extract query message "
                   "failed[ %d ]", sessionName(), rc ) ;

      try
      {
         BSONObj matcher( pQuery ) ;
         BSONObj selector ( pFieldSelector ) ;
         BSONObj orderBy ( pOrderBy ) ;
         BSONObj hint ( pHint ) ;

         if ( !_esClt )
         {
            rc = _seCltMgr->getSeClt( &_esClt ) ;
            PD_RC_CHECK( rc, PDERROR, "Get search engine client failed[ %d ]",
                         rc ) ;
         }

         // Free the original context.
         if ( _context )
         {
            SDB_OSS_DEL _context ;
            _context = NULL ;
         }

         rc = _getIndexAndType( pCollectionName, pHint,
                                msg->routeID.columns.groupID,
                                indexName, typeName ) ;

         _context = SDB_OSS_NEW seAdptContextQuery( indexName, typeName,
                                                    _esClt ) ;
         if ( !_context )
         {
            PD_LOG( PDERROR, "Allocate memory for context failed" ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         rc = _context->open( matcher, selector, orderBy,
                              hint, objBuff, eduCB ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Open context for rewrite query failed[ %d ]", rc ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Session[ %s ] create BSON objects for query items "
                 "failed: %s", sessionName(), e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto done ;
      }

   done:
      return rc ;
   error:
      if ( _context )
      {
         SDB_OSS_DEL _context ;
         _context = NULL ;
      }
      goto done ;
   }

   // Generate another new message.
   INT32 _seAdptAgentSession::_onRewriteQueryMore( MsgHeader *msg,
                                                   utilCommObjBuff &objBuff,
                                                   pmdEDUCB *eduCB )
   {
      INT32 rc = SDB_OK ;

      rc = _context->getMore( 1, objBuff ) ;
      PD_RC_CHECK( rc, PDERROR, "Get rewrite query failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // Send reply message. It contains a header, and an optinal body.
   INT32 _seAdptAgentSession::_reply( MsgOpReply *header, const CHAR *buff,
                                      UINT32 size )
   {
      INT32 rc = SDB_OK ;

      if ( size > 0 )
      {
         rc = routeAgent()->syncSend( _netHandle, (MsgHeader *)header,
                                      (void *)buff, size ) ;
      }
      else
      {
         rc = routeAgent()->syncSend( _netHandle, (void *)header ) ;
      }

      PD_RC_CHECK( rc, PDERROR, "Session[ %s ] send reply message failed[ %d ]",
                   sessionName(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptAgentSession::_defaultMsgFunc ( NET_HANDLE handle,
                                                MsgHeader * msg )
   {
      return _onOPMsg( handle, msg ) ;
   }

   INT32 _seAdptAgentSession::_onAuthReq( NET_HANDLE handle, MsgHeader* msg )
   {
      MsgOpReply reply ;
      rtnContextBuf buffObj ;

      reply.header.opCode = MAKE_REPLY_TYPE( msg->opCode ) ;
      reply.header.messageLength = sizeof( MsgOpReply ) ;
      reply.header.requestID = msg->requestID ;
      reply.header.TID = msg->TID ;
      reply.header.routeID.value = 0 ;
      reply.flags = SDB_OK ;

      return _reply( &reply, buffObj.data(), buffObj.size() );
   }

   INT32 _seAdptAgentSession::_getIndexAndType( const CHAR *pCollectionName,
                                                const CHAR *pHint,
                                                UINT32 groupID,
                                                std::string &indexName,
                                                std::string &typeName )
   {
      INT32 rc = SDB_OK ;
      std::string::size_type pos1 = 0 ;
      std::string::size_type pos2 = 0 ;
      CHAR type[ UTIL_SE_MAX_TYPE_SZ + 1 ] = { 0 } ;

      indexName = std::string( pCollectionName ) ;

      pos1 = indexName.find( '.' ) ;
      pos2 = indexName.rfind( '.' ) ;
      if ( ( std::string::npos == pos1 ) || ( pos1 != pos2 )
           || ( 0 == pos1 ) || ( ossStrlen( pCollectionName ) == pos1 ) )
      {
         PD_LOG( PDERROR, "Collection name format is wrong: %s",
                 pCollectionName ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // Erase the dot and the the index name.
      indexName.erase( pos1, 1 ) ;
      try
      {
         BSONObj hintObj( pHint ) ;
         // TODO: change name of the index.
         // indexName += "idx" ;
      }
      catch ( std::exception &e )
      {

      }

      ossItoa( groupID, type, UTIL_SE_MAX_TYPE_SZ ) ;
      typeName = string( type ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptAgentSession::_getQueryCond( const BSONObj &matcher,
                                             std::string &queryStr )
   {
      INT32 rc = SDB_OK ;

      BSONElement ele = matcher.firstElement() ;
      if ( Object == ele.type() )
      {
         BSONElement subEle = ele.Obj().firstElement() ;
         if ( 0 == ossStrcmp( FIELD_NAME_TEXT, subEle.fieldName() ) )
         {
            if ( 1 != matcher.nFields() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "Only one query condition should be specified "
                       "for text search, actually: %d", matcher.nFields() ) ;
               goto error ;
            }

            if ( String == subEle.type() )
            {
               queryStr = subEle.valuestr() ;
            }
            else if ( Object == subEle.type() )
            {
               queryStr = subEle.Obj().jsonString() ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "Query conditioin type[%d] for text "
                       "search is wrong", subEle.type() ) ;
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }
}

