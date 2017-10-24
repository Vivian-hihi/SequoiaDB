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

   Source File Name = seAdptAgentSession.cpp

   Descriptive Name = Agent session on search engine adapter.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#include "seAdptAgentSession.hpp"
#include "rtnContextBuff.hpp"
#include "msgMessage.hpp"
#include "seAdptUtil.hpp"

namespace engine
{
   BEGIN_OBJ_MSG_MAP( _seAdptAgentSession, _pmdAsyncSession)
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
            rc = _onQueryReq( msg, objBuff ) ;
            if ( SDB_OK == rc )
            {
               msgBody = objBuff.data() ;
               bodySize = objBuff.dataSize() ;
            }
            break ;
         case MSG_BS_GETMORE_REQ:
            rc = _onGetmoreReq( msg, objBuff ) ;
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

   // Handle query message from data node.
   // It will analyze the original query, fetch the neccessary data, and rewrite
   // the items, then return the new message.
   INT32 _seAdptAgentSession::_onQueryReq( MsgHeader *msg,
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

      string indexName ;
      string typeName ;
      utilCommObjBuff resultObjs ;
      seIdxMetaMgr *idxMetaCache = NULL ;
      BOOLEAN cacheLocked = FALSE ;

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
         IDX_META_VEC idxMetas ;
         seAdptNameParser nameParser ;
         const CHAR *origIdxName = NULL ;

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

         idxMetaCache = sdbGetSeAdapterCB()->getIdxMetaCache() ;
         idxMetaCache->lock( SHARED ) ;
         cacheLocked = TRUE ;
         rc = idxMetaCache->getIdxMetas( pCollectionName, idxMetas ) ;
         PD_RC_CHECK( rc, PDERROR, "Get index meta data for collection[ %s ] "
                      "failed[ %d ]", pCollectionName, rc ) ;

         // If there is only one index, use it for the search.
         // If there are more than one index, name of the index to be used
         // should be specified in the hint. Otherwise, error will be returned.
         if ( 0 == idxMetas.size() )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "No index on the collection[ %s ]",
                    pCollectionName ) ;
            goto error ;
         }
         else if ( 1 == idxMetas.size() )
         {
            indexName = idxMetas.front().getEsIdxName() ;
         }
         else
         {
            if ( hint.isEmpty() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "Text index name should be specified in hint "
                       "when there are multiple text indices") ;
               goto error ;
            }

            if ( 1 != hint.nFields() )
            {
               PD_LOG( PDERROR, "1 object is expected in hint. Actual[ %d ]",
                       hint.nFields() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }

            origIdxName = hint.getStringField( "" );
            if ( 0 == ossStrcmp( origIdxName, "" ) )
            {
               PD_LOG( PDERROR, "No valid index name specified in hint" ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }

            rc = nameParser.parse( pCollectionName, origIdxName ) ;
            PD_RC_CHECK( rc, PDERROR, "Parse collection[ %s ] and "
                         "index[ %s ] names failed[ %d ], skip ",
                         pCollectionName, origIdxName, rc ) ;
            indexName = nameParser.getTargetIdxName() ;
         }

         idxMetaCache->unlock() ;
         cacheLocked = FALSE ;

         typeName = sdbGetSeAdapterCB()->getDataNodeGrpName() ;

         _context = SDB_OSS_NEW seAdptContextQuery( indexName,
                                                    typeName, _esClt ) ;
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
      if ( cacheLocked )
      {
         SDB_ASSERT( idxMetaCache, "Index meta cache should not be NULL" ) ;
         idxMetaCache->unlock( SHARED ) ;
      }
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
   INT32 _seAdptAgentSession::_onGetmoreReq( MsgHeader *msg,
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

