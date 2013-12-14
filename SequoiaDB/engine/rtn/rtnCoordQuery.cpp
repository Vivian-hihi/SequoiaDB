
#include "coordSession.hpp"
#include "rtnCoord.hpp"
#include "rtnContext.hpp"
#include "rtnCB.hpp"
#include "msg.h"
#include "coordCB.hpp"
#include "rtnCoordCommon.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "msgMessage.hpp"
#include "rtnCoordQuery.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "rtn.hpp"
#include "rtnCoordCommands.hpp"
#include "pmdEDU.hpp"

using namespace bson;

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOQUERY_QUERYTODNGROUP, "rtnCoordQuery::queryToDataNodeGroup" )
   INT32 rtnCoordQuery::queryToDataNodeGroup( CHAR *pBuffer,
                                              CoordGroupList &groupLst,
                                              CoordGroupList &sendGroupLst,
                                              netMultiRouteAgent *pRouteAgent,
                                              pmdEDUCB *cb,
                                              rtnContextCoord *pContext,
                                              BOOLEAN sendToPrimary )
   {
      INT32 rc = SDB_OK;
      INT32 rcTmp = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOQUERY_QUERYTODNGROUP ) ;
      BOOLEAN isNeedRetry = FALSE;
      BOOLEAN hasRetry = FALSE;

      do
      {
         hasRetry = isNeedRetry;
         isNeedRetry = FALSE;
         REQUESTID_MAP sendNodes;
         rcTmp= rtnCoordSendRequestToNodeGroups( pBuffer, groupLst,
                                               sendToPrimary, pRouteAgent, cb,
                                               sendNodes );
         if ( rcTmp != SDB_OK )
         {
            // don't break,
            // save the sub-context, then clear it while delete context
            PD_LOG ( PDERROR, "Failed to query on data-node, send request "
                     "failed(rc=%d)", rcTmp ) ;
            if ( SDB_OK == rc )
            {
               rc = rcTmp;
            }
         }

         REPLY_QUE replyQue;
         rcTmp = rtnCoordGetReply( cb, sendNodes, replyQue, MSG_BS_QUERY_RES ) ;
         if ( rcTmp != SDB_OK )
         {
            // received interrupt
            PD_LOG ( PDWARNING, "Failed to query on data-node, get reply "
                     "failed(rc=%d)", rcTmp );
            if ( SDB_APP_INTERRUPT == rcTmp || SDB_OK == rc )
            {
               rc = rcTmp;
            }
         }

         while ( !replyQue.empty() )
         {
            MsgOpReply *pReply = NULL;
            pReply = (MsgOpReply *)(replyQue.front());
            replyQue.pop();
            rcTmp = pReply->flags;
            UINT32 groupID = pReply->header.routeID.columns.groupID;

            if ( rcTmp != SDB_OK )
            {
               if ( SDB_DMS_EOC == rcTmp )
               {
                  sendGroupLst[ groupID ] = groupID;
                  groupLst.erase( groupID );
                  rcTmp = SDB_OK;
               }
               else
               {
                  cb->getCoordSession()->removeLastNode( groupID );
                  if ( SDB_CLS_FULL_SYNC == rcTmp && !hasRetry )
                  {
                     CoordGroupInfoPtr groupInfoTmp ;
                     rcTmp = rtnCoordGetGroupInfo( cb, groupID, TRUE,
                                                   groupInfoTmp );
                     isNeedRetry = TRUE ;
                  }
               }
            }
            else
            {
               rcTmp = pContext->addSubContext( pReply->header.routeID,
                                                pReply->contextID );
               if ( SDB_OK == rcTmp )
               {
                  sendGroupLst[ groupID ] = groupID;
                  groupLst.erase( groupID );
               }
            }

            if ( rcTmp != SDB_OK )
            {
               if ( SDB_OK == rc || SDB_CLS_COORD_NODE_CAT_VER_OLD == rc )
               {
                  rc = rcTmp;
               }
               PD_LOG( PDERROR, "failed to query on data node"
                     "(groupID=%u, nodeID=%u, serviceID=%u, rc=%d)",
                     pReply->header.routeID.columns.groupID,
                     pReply->header.routeID.columns.nodeID,
                     pReply->header.routeID.columns.serviceID,
                     rcTmp );
            }
            SDB_OSS_FREE( pReply );
         }

         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR, "Failed to query on data-node(rc=%d)", rc ) ;
            break;
         }
      }while ( isNeedRetry ) ;

      PD_TRACE_EXITRC ( SDB_RTNCOQUERY_QUERYTODNGROUP, rc ) ;
      return rc;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOQUERY_GETNODEGROUPS, "rtnCoordQuery::getNodeGroups" )
   INT32 rtnCoordQuery::getNodeGroups( const CoordCataInfoPtr &cataInfo,
                                       BSONObj &queryObj,
                                       const CoordGroupList &sendGroupLst,
                                       CoordGroupList &groupLst )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOQUERY_GETNODEGROUPS ) ;

      cataInfo->getGroupByMatcher( queryObj, groupLst ) ;
      PD_CHECK( groupLst.size() > 0, SDB_CAT_NO_MATCH_CATALOG, error, PDERROR,
               "failed to get match groups" );
      {
      CoordGroupList::const_iterator iterList
                              = sendGroupLst.begin();
      while( iterList != sendGroupLst.end() )
      {
         groupLst.erase( iterList->first );
         ++iterList;
      }
      }

   done:
      PD_TRACE_EXITRC ( SDB_RTNCOQUERY_GETNODEGROUPS, rc ) ;
      return rc;
   error:
      goto done;
   }

   INT32 rtnCoordQuery::execute( CHAR *pReceiveBuffer, SINT32 packSize,
                                 CHAR **ppResultBuffer, pmdEDUCB *cb,
                                 MsgOpReply &replyHeader,
                                 BSONObj **ppErrorObj )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      SDB_RTNCB *pRtncb                = pKrcb->getRTNCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      SINT64 contextID                 = -1;
      rtnContextCoord *pContext        = NULL ;
      BOOLEAN isNeedRefresh            = FALSE;
      BOOLEAN hasRefresh               = FALSE;
      BSONObj boQuery;
      BSONObj boOrderBy;
      CoordGroupList                   sendGroupList ;

      // fill default-reply(query success)
      MsgHeader*pHeader                = (MsgHeader *)pReceiveBuffer;
      replyHeader.header.messageLength = sizeof( MsgOpReply );
      replyHeader.header.opCode        = MSG_BS_QUERY_RES;
      replyHeader.header.requestID     = pHeader->requestID;
      replyHeader.header.routeID.value = 0;
      replyHeader.header.TID           = pHeader->TID;
      replyHeader.contextID            = -1;
      replyHeader.flags                = SDB_OK;
      replyHeader.numReturned          = 0;
      replyHeader.startFrom            = 0;

      INT32 flag = 0;
      CHAR *pCollectionName = NULL;
      SINT64 numToSkip = 0;
      SINT64 numToReturn = 0;
      CHAR *pQuery = NULL;
      CHAR *pFieldSelector = NULL;
      CHAR *pOrderBy = NULL;
      CHAR *pHint = NULL;
      BSONObj *err = NULL ;
      MsgOpQuery *pSrc = (MsgOpQuery *)pReceiveBuffer;

      rc = msgExtractQuery( pReceiveBuffer, &flag, &pCollectionName,
                            &numToSkip, &numToReturn, &pQuery,
                            &pFieldSelector, &pOrderBy, &pHint );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to parse query request(rc=%d)", rc );

      // process command
      if ( pCollectionName != NULL && '$' == pCollectionName[0] )
      {
         rtnCoordCommand *pCmdProcesser = NULL;
         rtnCoordProcesserFactory *pProcesserFactory
                  = pCoordcb->getProcesserFactory();
         pCmdProcesser = pProcesserFactory->getCommandProcesser(
            pCollectionName );
         PD_CHECK( pCmdProcesser != NULL, SDB_INVALIDARG, error, PDERROR,
                  "unknown command:%s", pCollectionName );
         rc = pCmdProcesser->execute( pReceiveBuffer, packSize,
                                      ppResultBuffer, cb, replyHeader,
                                      &err );
         SDB_ASSERT( NULL == err, "impossible" );
         PD_RC_CHECK( rc, PDERROR, "failed to execute the command(command:%s, "
                    "rc=%d)", pCollectionName, rc );
         contextID = replyHeader.contextID;
         goto done;
      }

      // process query
      try
      {
         boQuery = BSONObj( pQuery );
         boOrderBy = BSONObj( pOrderBy );
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK( SDB_INVALIDARG, PDERROR,
                     "occur unexpected error:%s",
                     e.what() );
      }
      rc = pRtncb->contextNew( RTN_CONTEXT_COORD, (rtnContext**)&pContext,
                              contextID, cb );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to allocate context(rc=%d)", rc );
      rc = pContext->open( boOrderBy, numToReturn, numToSkip );
      PD_RC_CHECK( rc, PDERROR,
                  "open context failed(rc=%d)", rc );
      pSrc->header.routeID.value = 0;
      pSrc->header.TID = cb->getTID();
      if ( pSrc->numToReturn > 0 && pSrc->numToSkip > 0 )
      {
         // some record may skip on coord,
         // so the num of records from data-node must
         // more than "numToReturn + numToSkip"
         pSrc->numToReturn += pSrc->numToSkip;
      }
      pSrc->numToSkip = 0;
      do
      {
         hasRefresh = isNeedRefresh;
         CoordCataInfoPtr cataInfo;
         rc = rtnCoordGetCataInfo( cb, pCollectionName,
                                 isNeedRefresh, cataInfo );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to get the catalog info(collection:%s)",
                     pCollectionName );
         pSrc->version = cataInfo->getVersion();
         if ( cataInfo->isMainCL() )
         {
            // query on main collection
            CoordSubCLlist subCLList ;
            CoordGroupSubCLMap groupSubCLMap;
            rc = cataInfo->getMatchSubCLs( boQuery, subCLList );
            PD_CHECK( SDB_OK == rc, rc, error_retry, PDWARNING,
                     "failed to get match sub collection(rc=%d)",
                     rc );
            rc = rtnCoordGetSubCLsByGroups( subCLList, sendGroupList,
                                          cb, groupSubCLMap );
            PD_CHECK( SDB_OK == rc, rc, error_retry, PDWARNING,
                     "failed to get sub-collection info(rc=%d)",
                     rc );
            rc = queryOnMainCL( groupSubCLMap, pSrc, cb,
                              pRouteAgent, sendGroupList,
                              pContext );
         }
         else
         {
            CoordGroupList groupList ;
            // query on normal collection
            rc = getNodeGroups( cataInfo, boQuery,
                              sendGroupList, groupList );
            PD_CHECK( SDB_OK == rc, rc, error_retry, PDWARNING,
                     "failed to get match sharding(rc=%d)",
                     rc );
            rc = queryToDataNodeGroup( (CHAR *)pSrc, groupList, sendGroupList,
                                       pRouteAgent, cb, pContext );
            PD_CHECK( SDB_OK == rc, rc, error_retry, PDWARNING,
                     "query on data node failed(rc=%d)",
                     rc );
         }
   error_retry:
         if ( rc != SDB_OK && rc != SDB_APP_INTERRUPT && !hasRefresh )
         {
            isNeedRefresh = TRUE;
         }
         else
         {
            isNeedRefresh = FALSE;
         }

      }while( isNeedRefresh );
      PD_RC_CHECK( rc, PDERROR, "query failed(rc=%d)", rc );

      replyHeader.contextID = contextID;
      pContext->addSubDone( cb );
   done:
      return rc;
   error:
      replyHeader.flags = rc;
      if ( contextID >= 0 )
      {
         pRtncb->contextDelete( contextID, cb );
         contextID = -1;
         pContext = NULL;
      }
      goto done;
   }

   INT32 rtnCoordQuery::queryOnMainCL( CoordGroupSubCLMap &groupSubCLMap,
                                       MsgOpQuery *pSrc,
                                       pmdEDUCB *cb,
                                       netMultiRouteAgent *pRouteAgent,
                                       CoordGroupList &sendGroupList,
                                       rtnContextCoord *pContext )
   {
      INT32 rc = SDB_OK;
      INT32 rcTmp = SDB_OK;
      CHAR *pBuffer = NULL;
      INT32 bufferSize = 0;
      REPLY_QUE replyQue;
      SDB_ASSERT( pContext, "pContext can't be NULL!" );
      try
      {
         CoordGroupSubCLMap::iterator iterGroup;
         REQUESTID_MAP sendNodes;

         INT32 flag;
         CHAR *pCollectionName;
         SINT64 numToSkip;
         SINT64 numToReturn;
         CHAR *pQuery;
         CHAR *pFieldSelector;
         CHAR *pOrderBy;
         CHAR *pHint;
         BSONObj boQuery;
         BSONObj boFieldSelector;
         BSONObj boOrderBy;
         BSONObj boHint;
         rc = msgExtractQuery( (CHAR *)pSrc, &flag, &pCollectionName,
                              &numToSkip, &numToReturn, &pQuery,
                              &pFieldSelector, &pOrderBy, &pHint );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to parse query message(rc=%d)", rc );
         boQuery = BSONObj( pQuery );
         boFieldSelector = BSONObj( pFieldSelector );
         boOrderBy = BSONObj( pOrderBy );
         boHint = BSONObj( pHint );

         iterGroup = groupSubCLMap.begin();
         while( iterGroup != groupSubCLMap.end() )
         {
            BSONArrayBuilder babSubCL;
            CoordGroupInfoPtr groupInfo;
            CoordSubCLlist::iterator iterSubCL
                                    = iterGroup->second.begin();
            while( iterSubCL != iterGroup->second.end() )
            {
               babSubCL.append( *iterSubCL );
               ++iterSubCL;
            }
            BSONObjBuilder bobNewQuery;
            bobNewQuery.appendElements( boQuery );
            bobNewQuery.appendArray( CAT_SUBCL_NAME, babSubCL.arr() );
            BSONObj boNewQuery = bobNewQuery.obj();
            rc = msgBuildQueryMsg( &pBuffer, &bufferSize, pCollectionName,
                                 flag, 0, numToSkip, numToReturn, &boNewQuery,
                                 &boFieldSelector, &boOrderBy, &boHint );
            PD_CHECK( SDB_OK == rc, rc, RECV_MSG, PDERROR,
                     "failed to build query message(rc=%d)", rc );
            {
               MsgOpQuery *pReqMsg = (MsgOpQuery *)pBuffer;
               pReqMsg->version = pSrc->version;
               pReqMsg->w = pSrc->w;
            }
            rc = rtnCoordGetGroupInfo( cb, iterGroup->first, FALSE, groupInfo );
            PD_CHECK( SDB_OK == rc, rc, RECV_MSG, PDERROR,
                     "failed to get group info(groupId=%u, rc=%d)",
                     iterGroup->first, rc );
            rc = rtnCoordSendRequestToOne( pBuffer, groupInfo, sendNodes,
                                          pRouteAgent, MSG_ROUTE_SHARD_SERVCIE, cb );
            PD_CHECK( SDB_OK == rc, rc, RECV_MSG, PDERROR,
                     "failed to send request(rc=%d)", rc );
            ++iterGroup;
         }

      RECV_MSG:
         rcTmp = rtnCoordGetReply( cb, sendNodes, replyQue, MSG_BS_QUERY_RES );
         if ( SDB_OK != rcTmp )
         {
            PD_LOG( PDWARNING, "failed to get reply(rcTmp=%d)", rcTmp );
            if ( SDB_APP_INTERRUPT == rcTmp || SDB_OK == rc )
            {
               // if it is interrupt, go to error
               // the session will be released while process interrupt
               rc = rcTmp;
            }
         }
         while( !replyQue.empty() )
         {
            MsgOpReply *pReply = NULL;
            pReply = (MsgOpReply *)(replyQue.front());
            replyQue.pop();
            rcTmp = pReply->flags;
            UINT32 groupID = pReply->header.routeID.columns.groupID;
            if ( rcTmp != SDB_OK )
            {
               if ( SDB_OK == rc )
               {
                  rc = rcTmp;
               }
               if ( SDB_DMS_EOC == rcTmp )
               {
                  sendGroupList[ groupID ] = groupID;
                  groupSubCLMap.erase( groupID );
                  rcTmp = SDB_OK;
               }
               else
               {
                  CoordGroupInfoPtr groupInfoTmp;
                  cb->getCoordSession()->removeLastNode( groupID );
                  rcTmp = rtnCoordGetGroupInfo( cb, groupID, TRUE,
                                                groupInfoTmp );
               }
            }
            else
            {
               rcTmp = pContext->addSubContext( pReply->header.routeID,
                                                pReply->contextID );
               if ( SDB_OK == rcTmp )
               {
                  sendGroupList[ groupID ] = groupID;
                  groupSubCLMap.erase( groupID );
               }
            }
            if ( rcTmp != SDB_OK && SDB_OK == rc )
            {
               rc = rcTmp;
            }
            SDB_OSS_FREE( pReply );
         }
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK( SDB_INVALIDARG, PDERROR,
                     "occur unexpected error:%s",
                     e.what() );
      }
   done:
      if ( pBuffer != NULL )
      {
         SDB_OSS_FREE( pBuffer );
         pBuffer = NULL;
      }
      return rc;
   error:
      goto done;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETMORE_EXECUTE, "rtnCoordGetmore::execute" )
   INT32 rtnCoordGetmore::execute( CHAR *pReceiveBuffer, SINT32 packSize,
                                   CHAR **ppResultBuffer, pmdEDUCB *cb,
                                   MsgOpReply &replyHeader,
                                   BSONObj **ppErrorObj )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETMORE_EXECUTE ) ;
      SDB_ASSERT ( pReceiveBuffer, "input buffer can't be NULL")
      SDB_ASSERT ( ppResultBuffer, "result buffer can't be NULL")
      SDB_ASSERT ( cb, "educb can't be NULL")

      // init the reply-msg
      MsgOpGetMore *pGetMoreMsg = (MsgOpGetMore *)pReceiveBuffer;
      replyHeader.header.messageLength = sizeof( MsgOpReply );
      replyHeader.header.opCode = MSG_BS_GETMORE_RES;
      replyHeader.header.TID = pGetMoreMsg->header.TID;
      replyHeader.header.routeID.value = 0;
      replyHeader.header.requestID = pGetMoreMsg->header.requestID;
      replyHeader.flags = SDB_OK ;
      replyHeader.startFrom = 0;
      replyHeader.numReturned = 0;
      replyHeader.contextID = -1;

      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();
      rtnContext *pContext = NULL;
      SINT64 contextID = 0;

      do
      {
         if ( packSize < (INT32)sizeof( MsgOpGetMore ))
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                  "the message size is error(expect:%d,received:%d)",
                  sizeof(MsgOpGetMore), packSize );
            break;
         }

         pContext = pRtncb->contextFind ( pGetMoreMsg->contextID );
         if ( NULL == pContext )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDWARNING,
                     "getmore failed, invalid contextID(%lld)",
                     pGetMoreMsg->contextID );
            break;
         }
         if ( pContext->eduID() != cb->getID() )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDWARNING, "Getmore failed, invalid eduID(expect:%lld, "
                     "current:%lld)", pContext->eduID(), cb->getID() ) ;
            break;
         }

         contextID = pContext->contextID() ;

         /*if ( NULL != pContext->_accPlan )
         {
            CHAR *ppBufferCurrent = NULL ;
            SINT32 bufferLength = 0 ;
            SINT32 numRecords = 0 ;
            SINT64 startingPos = 0 ;
            rc = rtnGetMoreQgmScan( pContext, &ppBufferCurrent,
                                    bufferLength, numRecords,
                                    startingPos ) ;
            if ( SDB_OK != rc )
            {
               if ( SDB_DMS_EOC != rc )
               {
                  PD_LOG( PDERROR,
                          "getmore failed, failed to get the data(rc=%d)",
                          rc );
               }
            }
         }
         else
         {
            rc = getData( pContext, pRouteAgent, cb, pGetMoreMsg->numToReturn );
            if ( rc != SDB_OK && rc != SDB_DMS_EOC )
            {
               PD_LOG( PDERROR,
                     "getmore failed, failed to get the data(rc=%d)",
                     rc );
               break;
            }
         }*/
      }while ( FALSE );

      replyHeader.flags = rc ;
      if ( rc != SDB_OK && contextID >= 0 )
      {
         pRtncb->contextDelete ( contextID, cb ) ;
      }
      else
      {
         /*replyHeader.contextID = contextID;
         replyHeader.header.messageLength = sizeof ( MsgOpReply )
                                    + pContext->_bufferEndOffset
                                    - pContext->_bufferCurrentOffset;
         replyHeader.numReturned = pContext->_bufferNumRecords;
         *ppResultBuffer = pContext->_pResultBuffer;
         pContext->_bufferEndOffset = 0;
         pContext->_bufferCurrentOffset = 0;
         pContext->_bufferNumRecords = 0;*/
      }
      PD_TRACE_EXITRC ( SDB_RTNCOGETMORE_EXECUTE, rc ) ;
      return rc;
   }
   
   PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOGETMOR_GETDATA, "rtnCoordGetmore::getData" )
   INT32 rtnCoordGetmore::getData( rtnContextData *pContext,
                     netMultiRouteAgent *pRouteAgent,
                     pmdEDUCB *cb,
                     const SINT32 numToReturn )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_RTNCOGETMOR_GETDATA ) ;
      do
      {
/*         if ( pContext->_bufferNumRecords > 0 )
         {
            break;
         }
         rc = pContext->getSubData( numToReturn );
         if ( rc != SDB_RTN_COORD_CACHE_EMPTY )
         {
            break;
         }
         EMPTY_CONTEXTID_MAP emptyContextLst;
         pContext->getEmptyContext( emptyContextLst );
         if ( emptyContextLst.empty() )
         {
            rc = SDB_DMS_EOC;
            PD_LOG ( PDINFO, "get EOC" );
            break;
         }
         MsgOpGetMore getMoreReq;
         msgFillGetMoreMsg( getMoreReq, cb->getTID(), -1, numToReturn, 0 );

         REQUESTID_MAP sendNodes;
         EMPTY_CONTEXTID_MAP::iterator emptyIter = emptyContextLst.begin();
         while ( emptyIter != emptyContextLst.end() )
         {
            MsgRouteID routeID;
            routeID.value = emptyIter->first;
            SINT64 contextID = emptyIter->second;
            getMoreReq.contextID = contextID;
            rc = rtnCoordSendRequestToNode( (void *)(&getMoreReq), routeID,
                                          pRouteAgent, cb, sendNodes );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "get data failed, failed to send getmore request to "
                        "node( groupID=%u, nodeID=%u, serviceID=%u, "
                        "contextID=%lld )",
                        routeID.columns.groupID,
                        routeID.columns.nodeID,
                        routeID.columns.serviceID,
                        contextID );
               break;
            }
            ++emptyIter;
         }
         if ( rc != SDB_OK )
         {
            break;
         }
         REPLY_QUE replyQue;
         rc = rtnCoordGetReply( cb, sendNodes, replyQue, MSG_BS_GETMORE_RES );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "get data failed, failed to get the reply(rc=%d)",
                     rc );
            break;
         }
         while ( !replyQue.empty() )
         {
            CHAR *pData = NULL;
            pData = replyQue.front();
            replyQue.pop();
            if ( SDB_OK == rc || SDB_DMS_EOC == rc )
            {
               MsgOpReply *pReply = (MsgOpReply *)pData;
               if ( pReply->header.messageLength < (INT32)sizeof( MsgOpReply ))
               {
                  rc = SDB_INVALIDARG;
                  PD_LOG ( PDERROR,
                           "get data failed, received invalid message from node\
                           (groupID=%u, nodeID=%u, serviceID=%u, messageLength=%d)",
                           pReply->header.routeID.columns.groupID,
                           pReply->header.routeID.columns.nodeID,
                           pReply->header.routeID.columns.serviceID,
                           pReply->header.messageLength );
               }
               else
               {
                  if ( pReply->flags != SDB_OK )
                  {
                     if ( pReply->flags != SDB_DMS_EOC )
                     {
                        PD_LOG ( PDERROR,
                                 "get data failed, failed to get data from node\
                                 (groupID=%u, nodeID=%u, serviceID=%u, flag=%d)",
                                 pReply->header.routeID.columns.groupID,
                                 pReply->header.routeID.columns.nodeID,
                                 pReply->header.routeID.columns.serviceID,
                                 pReply->flags );
                     }
                     else
                     {
                        pContext->delEmptyContext(pReply->header.routeID);
                     }
                     rc = pReply->flags;
                  }
                  else
                  {
                     rc = pContext->appendSubData( pData );
                     if ( rc != SDB_OK )
                     {
                        PD_LOG ( PDERROR,
                                 "get data failed, failed to append the data(rc=%d)",
                                 rc );
                     }
                  }
               }
            }
            if ( rc != SDB_OK && pData != NULL )
            {
               SDB_OSS_FREE ( pData );
            }
         }
         if ( rc != SDB_OK && rc != SDB_DMS_EOC )
         {
            break;
         } */
      }while ( TRUE );
      PD_TRACE_EXITRC ( SDB_RTNCOGETMOR_GETDATA, rc ) ;
      return rc;
   }

   INT32 rtnCoordGetmore::sendRequest( rtnContext *pContext,
                                       pmdEDUCB *cb,
                                       netMultiRouteAgent *pRouteAgent,
                                       const SINT32 numToReturn )
   {
      INT32 rc = SDB_OK;
      /*MsgOpGetMore msgReq;
      EMPTY_CONTEXT_MAP::iterator emptyIter;
      PD_CHECK( !(pContext->_emptyContextMap.empty()), SDB_DMS_EOC,
               done, PDINFO, "no empty node" );
      msgFillGetMoreMsg( msgReq, cb->getTID(), -1, numToReturn, 0 );
      emptyIter = pContext->_emptyContextMap.begin();
      while( emptyIter != pContext->_emptyContextMap.end() )
      {
         MsgRouteID routeID;
         routeID.value = emptyIter->first;
         SINT64 contextID = emptyIter->second->getContextID();
         msgReq.contextID = contextID;
         rc = rtnCoordSendRequestToNode( (void *)(&msgReq), routeID, pRouteAgent,
                                       cb, pContext->_prepareNodeMap );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "failed to send getmore request to "
                     "node( groupID=%u, nodeID=%u, serviceID=%u, "
                     "contextID=%lld, rc=%d )",
                     routeID.columns.groupID,
                     routeID.columns.nodeID,
                     routeID.columns.serviceID,
                     contextID, rc );
            break;
         }
         pContext->_prepareContextMap.insert(
                        EMPTY_CONTEXT_MAP::value_type( emptyIter->first,
                                                       emptyIter->second ));
         pContext->_emptyContextMap.erase( emptyIter++ );
      }
   done:*/
      return rc;
   }
}
