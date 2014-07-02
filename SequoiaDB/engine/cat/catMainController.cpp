
#include "core.hpp"
#include "catMainController.hpp"
#include "catalogueCB.hpp"
#include "pmdCB.hpp"
#include "pd.hpp"
#include "catDef.hpp"
#include "rtn.hpp"
#include "dpsLogWrapper.hpp"
#include "rtnCoord.hpp"
#include "msgMessage.hpp"
#include "msgAuth.hpp"
#include "../util/fromjson.hpp"
#include "pmdDef.hpp"
#include "pdTrace.hpp"
#include "catTrace.hpp"
#include "catCommon.hpp"

using namespace bson;
namespace engine
{
   catMainController::catMainController ()
   {
      _nodeManagerEDUID    = PMD_INVALID_EDUID ;
      _catalogManagerEDUID = PMD_INVALID_EDUID ;
      _pKrcb               = NULL ;
      _pEduMgr             = NULL ;
      _pCatCB              = NULL ;
      _pDmsCB              = NULL ;
      _pDpsCB              = NULL ;
      _pAuthCB             = NULL ;
      _pNodeMgrCB          = NULL ;
      _pCataMgrCB          = NULL ;
      _pClsCB              = NULL ;
      _pEDUCB              = NULL ;
   }

   catMainController::~catMainController()
   {
   }

   void catMainController::attachCB( pmdEDUCB * cb )
   {
      if ( EDU_TYPE_CATMAINCONTROLLER == cb->getType() )
      {
         _pEDUCB = cb ;
      }
      else if ( EDU_TYPE_CATCATALOGUEMANAGER == cb->getType() )
      {
         _pCataMgrCB = cb ;
         _catalogManagerEDUID = cb->getID() ;
      }
      else if ( EDU_TYPE_CATNODEMANAGER == cb->getType() )
      {
         _pNodeMgrCB = cb ;
         _nodeManagerEDUID = cb->getID() ;
      }
      _attachEvent.signalAll() ;
   }

   void catMainController::detachCB( pmdEDUCB * cb )
   {
      if ( EDU_TYPE_CATMAINCONTROLLER == cb->getType() )
      {
         _pEDUCB = NULL ;
      }
      else if ( EDU_TYPE_CATCATALOGUEMANAGER == cb->getType() )
      {
         _pCataMgrCB = NULL ;
      }
      else if ( EDU_TYPE_CATNODEMANAGER == cb->getType() )
      {
         _pNodeMgrCB = NULL ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_HANDLEMSG, "catMainController::handleMsg" )
   INT32 catMainController::handleMsg( const NET_HANDLE &handle,
                                       const _MsgHeader *header,
                                       const CHAR *msg )
   {
      SDB_ASSERT ( _pEduMgr && _pKrcb && _pCatCB && _pDmsCB && _pDpsCB,
                   "all of the members must be initialized before init "
                   "netfram" ) ;
      SDB_ASSERT ( NULL != header, "message-header should not be NULL" ) ;
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( SDB_CATMAINCT_HANDLEMSG ) ;
      PD_TRACE1 ( SDB_CATMAINCT_HANDLEMSG,
                  PD_PACK_INT ( header->opCode ) ) ;
      //get the msg-processor's eduID
      switch ( header->opCode )
      {
      case MSG_BS_QUERY_REQ:
         {
            rc = processQueryMsg( handle, msg );
            break;
         }

      case MSG_BS_GETMORE_REQ :
         {
            rc = processGetMoreMsg( handle, msg ) ;
            break ;
         }

      case MSG_BS_KILL_CONTEXT_REQ:
         {
            rc = processKillContext( handle, msg ) ;
            break;
         }

      case MSG_CAT_QUERY_DATA_GRP_REQ :
         {
            rc = processQueryDataGrp( handle, msg ) ;
            break ;
         }
      case MSG_CAT_QUERY_COLLECTIONS_REQ :
         {
            rc = processQueryCollections( handle, msg ) ;
            break ;
         }
      case MSG_CAT_QUERY_COLLECTIONSPACES_REQ :
         {
            rc = processQueryCollectionSpaces ( handle, msg ) ;
            break ;
         }
      case MSG_AUTH_VERIFY_REQ :
         {
            rc = processAuthenticate( handle, msg ) ;
            break ;
         }
      case MSG_AUTH_CRTUSR_REQ :
         {
            rc = processAuthCrt( handle, msg ) ;
            break ;
         }
      case MSG_AUTH_DELUSR_REQ :
         {
            rc = processAuthDel( handle, msg ) ;
            break ;
         }
      case MSG_COOR_CHECK_ROUTEID_REQ :
         {
            rc = processCheckRouteID( handle, msg );
            break;
         }
      case MSG_CAT_QUERY_DOMAIN_REQ :
         {
            rc = processQueryDomain ( handle, msg ) ;
            break ;
         }
      default :
         {
            rc = postMsg( handle, header );
            break ;
         }
      }
      PD_TRACE_EXITRC ( SDB_CATMAINCT_HANDLEMSG, rc ) ;
      return rc ;
   }

   void catMainController::handleClose( const NET_HANDLE & handle,
                                        _MsgRouteID id )
   {
      delContext( handle );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_POSTMSG, "catMainController::postMsg" )
   INT32 catMainController::postMsg( const NET_HANDLE &handle,
                                     const MsgHeader *header )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_CATMAINCT_POSTMSG ) ;
      PD_TRACE1 ( SDB_CATMAINCT_POSTMSG,
                  PD_PACK_INT ( header->opCode ) ) ;

      EvntCatalogInternalEvent *pEvent = NULL ;

      if ( MSG_CAT_CATALOGUE_BEGIN < (UINT32)header->opCode &&
           (UINT32)header->opCode < MSG_CAT_CATALOGUE_END )
      {
         if ( NULL == _pCataMgrCB )
         {
            rc = SDB_SYS ;
            goto error ;
         }
         rc = catBuildMsgEvent ( handle, header, pEvent ) ;
         PD_RC_CHECK( rc, PDERROR, "failed to build the event(rc=%d)", rc );
         _pCataMgrCB->postEvent(pmdEDUEvent ( PMD_EDU_EVENT_MSG,
                                              TRUE, (void *)pEvent ) ) ;
      }
      else if  ( MSG_CAT_NODE_BEGIN < (UINT32)header->opCode &&
                 (UINT32)header->opCode < MSG_CAT_NODE_END )
      {
         if ( NULL == _pNodeMgrCB )
         {
            rc = SDB_SYS ;
            goto error ;
         }
         rc = catBuildMsgEvent ( handle, header, pEvent ) ;
         PD_RC_CHECK( rc, PDERROR, "failed to build the event(rc=%d)", rc );
         _pNodeMgrCB->postEvent(pmdEDUEvent ( PMD_EDU_EVENT_MSG,
                                              TRUE, (void *)pEvent ) ) ;
      }
      else
      {
         rc = SDB_UNKNOWN_MESSAGE ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_CATMAINCT_POSTMSG, rc ) ;
      return rc;
   error:
      if ( pEvent != NULL )
      {
         if ( pEvent->data != NULL )
         {
            SDB_OSS_FREE ( pEvent->data );
         }
         SDB_OSS_FREE ( pEvent );
      }
      PD_LOG ( PDERROR, "Failed to process message(MessageType = %d, rc=%d)",
               header->opCode, rc ) ;
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_INIT, "catMainController::init" )
   INT32 catMainController::init()
   {
      INT32 rc             = SDB_OK ;
      _pKrcb               = pmdGetKRCB() ;
      _pEduMgr             = _pKrcb->getEDUMgr () ;
      _pDmsCB              = _pKrcb->getDMSCB() ;
      _pDpsCB              = _pKrcb->getDPSCB() ;
      _pRtnCB              = _pKrcb->getRTNCB() ;
      _pAuthCB             = _pKrcb->getAuthCB() ;
      _pClsCB              = _pKrcb->getClsCB() ;

      _pCatCB = _pKrcb->getCATLOGUECB() ;

      PD_TRACE_ENTRY ( SDB_CATMAINCT_INIT ) ;

      // after initializing, let's attempt to create collectionspace and
      // collections
      rc = _ensureMetadata () ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to create metadata collections/indexes, rc = %d",
                    rc ) ;

   done :
      PD_TRACE_EXITRC ( SDB_CATMAINCT_INIT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT__CREATESYSIDX, "catMainController::_createSysIndex" )
   INT32 catMainController::_createSysIndex ( const CHAR *pCollection,
                                              const CHAR *pIndex,
                                              pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj indexDef ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT__CREATESYSIDX ) ;
      PD_TRACE2 ( SDB_CATMAINCT__CREATESYSIDX,
                  PD_PACK_STRING ( pCollection ),
                  PD_PACK_STRING ( pIndex ) ) ;

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
      PD_TRACE_EXITRC ( SDB_CATMAINCT__CREATESYSIDX, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT__CREATESYSCOL, "catMainController::_createSysCollection" )
   INT32 catMainController::_createSysCollection ( const CHAR *pCollection,
                                                   pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT__CREATESYSCOL ) ;
      PD_TRACE1 ( SDB_CATMAINCT__CREATESYSCOL,
                  PD_PACK_STRING ( pCollection ) ) ;

      rc = catTestAndCreateCL( pCollection, cb, _pDmsCB, NULL, TRUE ) ;
      if ( rc )
      {
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( SDB_CATMAINCT__CREATESYSCOL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // NO NEED TO SYNC LOG FOR ANY CREATION
   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT__ENSUREMETADATA, "catMainController::_ensureMetadata" )
   INT32 catMainController::_ensureMetadata()
   {
      INT32 rc = SDB_OK ;
      pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT__ENSUREMETADATA ) ;

      // create SYSCAT.SYSNODES
      rc = _createSysCollection( CAT_NODE_INFO_COLLECTION, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createSysIndex ( CAT_NODE_INFO_COLLECTION,
                             CAT_NODEINFO_GROUPNAMEIDX, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createSysIndex ( CAT_NODE_INFO_COLLECTION,
                             CAT_NODEINFO_GROUPIDIDX, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      // create SYSCAT.SYSCOLLECTIONSPACES
      rc = _createSysCollection ( CAT_COLLECTION_SPACE_COLLECTION, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createSysIndex ( CAT_COLLECTION_SPACE_COLLECTION,
                             CAT_COLLECTION_SPACE_NAMEIDX, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      // create SYSCAT.SYSCOLLECTIONS
      rc = _createSysCollection ( CAT_COLLECTION_INFO_COLLECTION, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createSysIndex ( CAT_COLLECTION_INFO_COLLECTION,
                             CAT_COLLECTION_NAMEIDX, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      // create SYSCAT.SYSTASKS
      rc = _createSysCollection ( CAT_TASK_INFO_COLLECTION, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createSysIndex ( CAT_TASK_INFO_COLLECTION,
                             CAT_TASK_INFO_CLOBJIDX, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      // create SYSCAT.SYSDOMAINS
      rc = _createSysCollection ( CAT_DOMAIN_COLLECTION, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createSysIndex ( CAT_DOMAIN_COLLECTION,
                             CAT_DOMAIN_NAMEIDX, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      // create SYSCAT.SYSBUCKETS
      rc = _createSysCollection( CAT_BUCKET_COLLECTION, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createSysIndex( CAT_BUCKET_COLLECTION, CAT_BUCKET_BUCKETID_IDX,
                            cb ) ;
      if ( rc )
      {
         goto error ;
      }

      /// procedures
      rc = _createSysCollection ( CAT_PROCEDURES_COLLECTION, cb ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = _createSysIndex ( CAT_PROCEDURES_COLLECTION,
                             CAT_PROCEDURES_COLLECTION_INDEX, cb ) ;
      if ( rc )
      {
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( SDB_CATMAINCT__ENSUREMETADATA, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // when we activate the main controller, we should always assume there's
   // metadata collections exist
   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_ACTIVE, "catMainController::active" )
   INT32 catMainController::active()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT_ACTIVE ) ;

      rc = _pEduMgr->postEDUPost( _nodeManagerEDUID, PMD_EDU_EVENT_ACTIVE ) ;
      if ( SDB_OK == rc )
      {
         _pAuthCB->checkNeedAuth( _pEDUCB, TRUE ) ;
      }
      else
      {
         PD_RC_CHECK ( rc, PDERROR, "Failed to post edu, rc = %d", rc ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_CATMAINCT_ACTIVE, rc ) ;
      return rc ;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_DEACTIVE, "catMainController::deactive" )
   INT32 catMainController::deactive()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT_DEACTIVE ) ;

      rc = _pEduMgr->postEDUPost( _nodeManagerEDUID, PMD_EDU_EVENT_DEACTIVE );
      PD_RC_CHECK ( rc, PDERROR, "Failed to post edu, rc = %d", rc ) ;

   done:
      PD_TRACE_EXITRC ( SDB_CATMAINCT_DEACTIVE, rc ) ;
      return rc ;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_BUILDMSGEVENT, "catMainController::catBuildMsgEvent" )
   INT32 catMainController::catBuildMsgEvent ( const NET_HANDLE &handle,
                                               const MsgHeader *pMsg,
                                             EvntCatalogInternalEvent *&pEvent )
   {
      INT32 rc = SDB_OK ;
      MsgHeader *pEventData = NULL ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT_BUILDMSGEVENT ) ;
      // caller is responsible to free memory
      pEvent = (EvntCatalogInternalEvent *)SDB_OSS_MALLOC (
               sizeof(EvntCatalogInternalEvent));
      if ( NULL == pEvent )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "malloc failed(size = %d)",
                  sizeof(EvntCatalogInternalEvent) ) ;
         goto error ;
      }
      pEventData = (MsgHeader *)SDB_OSS_MALLOC( pMsg->messageLength ) ;
      if ( NULL == pEventData )
      {
         rc = SDB_OOM;
         PD_LOG ( PDERROR, "malloc failed(size = %d)",
                  pMsg->messageLength ) ;
         SDB_OSS_FREE ( pEvent ) ;
         pEvent = NULL ;
         goto error ;
      }
      ossMemcpy( (void *)pEventData, pMsg, pMsg->messageLength );
      pEvent->handle = handle;
      pEvent->data = (CHAR *)pEventData;
   done :
      PD_TRACE_EXITRC ( SDB_CATMAINCT_BUILDMSGEVENT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_GETMOREMSG, "catMainController::processGetMoreMsg" )
   INT32 catMainController::processGetMoreMsg ( const NET_HANDLE &handle,
                                                const CHAR *pMsg )
   {
      INT32 rc               = SDB_OK ;
      MsgOpGetMore *pGetMore = (MsgOpGetMore*)pMsg ;

      rtnContextBuf buffObj ;
      SINT64 startingPos     = 0 ;
      SINT32 msgLen          = 0 ;
      MsgOpReply *pReply     = NULL ;

      PD_TRACE_ENTRY ( SDB_CATMAINCT_GETMOREMSG ) ;
      // send the reply whether successful or not
      rc = rtnGetMore( pGetMore->contextID, pGetMore->numToReturn,
                       buffObj, startingPos, _pEDUCB, _pRtnCB ) ;
      if ( rc )
      {
         delContext( handle, pGetMore->contextID );
      }
      msgLen =  sizeof(MsgOpReply) + buffObj.size() ;
      // free by end of function
      pReply = (MsgOpReply *)SDB_OSS_MALLOC( msgLen );
      if ( NULL == pReply )
      {
         PD_LOG ( PDERROR, "Malloc error ( size = %d )", msgLen ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      pReply->header.messageLength = msgLen ;
      pReply->header.opCode        = MSG_BS_GETMORE_RES ;
      pReply->header.TID           = pGetMore->header.TID ;
      pReply->header.routeID.value = 0 ;
      pReply->header.requestID     = pGetMore->header.requestID ;
      pReply->contextID            = pGetMore->contextID ;
      pReply->startFrom            = startingPos ;
      pReply->numReturned          = buffObj.recordNum() ;
      pReply->flags                = rc ;
      PD_TRACE1 ( SDB_CATMAINCT_GETMOREMSG,
                  PD_PACK_INT ( rc ) ) ;
      if ( SDB_OK != rc && SDB_DMS_EOC != rc )
      {
         PD_LOG ( PDERROR, "Failed to get more, rc = %d", rc ) ;
      }
      ossMemcpy( (CHAR *)pReply + sizeof(MsgOpReply), buffObj.data(),
                 buffObj.size() ) ;
      rc = _pCatCB->netWork()->syncSend(handle, pReply);
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to syncSend, rc = %d", rc ) ;
         goto error ;
      }
   done :
      if ( pReply )
      {
         SDB_OSS_FREE( pReply ) ;
      }
      PD_TRACE_EXITRC ( SDB_CATMAINCT_GETMOREMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_KILLCONTEXT, "catMainController::processKillContext" )
   INT32 catMainController::processKillContext( const NET_HANDLE &handle,
                                                const CHAR *pMsg )
   {
      INT32 rc = SDB_OK ;
      INT32 contextNum = 0 ;
      INT64 *pContextIDs = NULL ;
      MsgOpReply msgReply;
      MsgOpKillContexts *pReq = (MsgOpKillContexts *)pMsg;
      msgReply.contextID = -1;
      msgReply.flags = SDB_OK;
      msgReply.numReturned = 0;
      msgReply.startFrom = 0;
      msgReply.header.messageLength = sizeof(MsgOpReply);
      msgReply.header.opCode = MSG_BS_KILL_CONTEXT_RES;
      msgReply.header.requestID = pReq->header.requestID;
      msgReply.header.routeID.value = 0;
      msgReply.header.TID = pReq->header.TID;

      PD_TRACE_ENTRY ( SDB_CATMAINCT_KILLCONTEXT ) ;
      do
      {
         rc = msgExtractKillContexts ( (CHAR *)pMsg,
                                       &contextNum, &pContextIDs ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG ( PDERROR,
                     "failed to parse the killcontexts request(rc=%d)",
                     rc ) ;
            break;
         }

         if ( contextNum > 0 )
         {
            PD_LOG ( PDDEBUG,
                     "KillContext: contextNum:%d\ncontextID: %lld",
                     contextNum, pContextIDs[0] ) ;
         }

         rc = rtnKillContexts ( contextNum, pContextIDs, _pEDUCB, _pRtnCB ) ;
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDWARNING,
                     "failed to delete the context(rc = %d),\
                     contextNum:%d\ncontextID: %lld",
                     rc, contextNum, pContextIDs[0] );
            break;
         }
      }while ( FALSE );
      msgReply.flags = rc;
      PD_TRACE1 ( SDB_CATMAINCT_KILLCONTEXT,
                  PD_PACK_INT ( rc ) ) ;
      rc = _pCatCB->netWork()->syncSend( handle, &msgReply );
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR,
                  "failed to send the message\
                  ( groupID=%d, nodeID=%d, serviceID=%d )",
                  pReq->header.routeID.columns.groupID,
                  pReq->header.routeID.columns.nodeID,
                  pReq->header.routeID.columns.serviceID );
      }
      PD_TRACE_EXITRC ( SDB_CATMAINCT_KILLCONTEXT, rc ) ;
      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_QUERYMSG, "catMainController::processQueryMsg" )
   INT32 catMainController::processQueryMsg( const NET_HANDLE &handle,
                                             const CHAR *pMsg )
   {
      INT32 rc = SDB_OK;
      MsgOpReply msgReply;
      MsgOpQuery *pReq = (MsgOpQuery *)pMsg;
      msgReply.contextID = -1;
      msgReply.flags = SDB_OK;
      msgReply.numReturned = 0;
      msgReply.startFrom = 0;
      msgReply.header.messageLength = sizeof(MsgOpReply);
      msgReply.header.opCode = MSG_BS_QUERY_RES;
      msgReply.header.requestID = pReq->header.requestID;
      msgReply.header.routeID.value = 0;
      msgReply.header.TID = pReq->header.TID;

      PD_TRACE_ENTRY ( SDB_CATMAINCT_QUERYMSG ) ;
      do
      {
         SINT64 contextID      = 0 ;
         INT32 flags           = 0 ;
         SINT64 numToSkip      = -1 ;
         SINT64 numToReturn    = -1 ;
         CHAR *pCollectionName = NULL ;
         CHAR *pQuery          = NULL ;
         CHAR *pFieldSelector  = NULL ;
         CHAR *pOrderBy        = NULL ;
         CHAR *pHint           = NULL ;
         rc = msgExtractQuery( (CHAR *)pMsg, &flags, &pCollectionName,
                              &numToSkip, &numToReturn, &pQuery,
                              &pFieldSelector, &pOrderBy, &pHint );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "failed to parse query request(rc=%d)",
                     rc );
            break;
         }
         BSONObj selector;
         BSONObj matcher;
         BSONObj orderBy;
         BSONObj hint;
         try
         {
            selector = BSONObj ( pFieldSelector );
            matcher = BSONObj ( pQuery );
            orderBy = BSONObj ( pOrderBy );
            hint = BSONObj ( pHint );
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                     "occured unexpected error:%s",
                     e.what() );
            break;
         }
         rc = rtnQuery( pCollectionName, selector, matcher, orderBy,
                     hint, flags, _pEDUCB, numToSkip, numToReturn,
                     _pDmsCB, _pRtnCB, contextID );
         if ( rc != SDB_OK )
         {
            if ( rc != SDB_DMS_EOC )
            {
               PD_LOG ( PDERROR,
                        "failed to query the collection:%s(rc=%d)",
                        pCollectionName, rc );
            }
            break;
         }
         addContext( handle, contextID );
         msgReply.contextID = contextID;
      }while ( FALSE );

      msgReply.flags = rc;
      PD_TRACE1 ( SDB_CATMAINCT_QUERYMSG,
                  PD_PACK_INT ( rc ) ) ;
      rc = _pCatCB->netWork()->syncSend ( handle, &msgReply );
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR,
                  "failed to send the message\
                  ( groupID=%d, nodeID=%d, serviceID=%d )",
                  pReq->header.routeID.columns.groupID,
                  pReq->header.routeID.columns.nodeID,
                  pReq->header.routeID.columns.serviceID );
      }
      PD_TRACE_EXITRC ( SDB_CATMAINCT_QUERYMSG, rc ) ;
      return rc;
   }

   INT32 catMainController::processQueryCollections ( const NET_HANDLE &handle,
                                                      const CHAR *pMsg )
   {
      return _processQueryRequest ( handle, pMsg,
                                    CAT_COLLECTION_INFO_COLLECTION ) ;
   }

   INT32 catMainController::processQueryCollectionSpaces (
         const NET_HANDLE &handle,
         const CHAR *pMsg )
   {
      return _processQueryRequest ( handle, pMsg,
                                    CAT_COLLECTION_SPACE_COLLECTION ) ;
   }

   INT32 catMainController::processQueryDataGrp( const NET_HANDLE &handle,
                                                 const CHAR *pMsg )
   {
      return _processQueryRequest ( handle, pMsg, CAT_NODE_INFO_COLLECTION ) ;
   }

   INT32 catMainController::processQueryDomain ( const NET_HANDLE &handle,
                                                 const CHAR *pMsg )
   {
      return _processQueryRequest ( handle, pMsg, CAT_DOMAIN_COLLECTION ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_QUERYREQUEST, "catMainController::_processQueryRequest" )
   INT32 catMainController::_processQueryRequest ( const NET_HANDLE &handle,
                                                   const CHAR *pMsg,
                                                   const CHAR *pCollectionName )
   {
      INT32 rc              = SDB_OK ;
      MsgOpReply msgReply ;
      MsgHeader *pMsgHeader = (MsgHeader *)( pMsg ) ;
      SINT64 contextID      = 0 ;
      SINT32 flags          = 0 ;
      SINT64 numToSkip      = -1 ;
      SINT64 numToReturn    = -1 ;
      CHAR *pCN             = NULL ;
      CHAR *pQuery          = NULL ;
      CHAR *pFieldSelector  = NULL ;
      CHAR *pOrderByBuffer  = NULL ;
      CHAR *pHintBuffer     = NULL ;
      INT32 iNameLen        = 0 ;

      PD_TRACE_ENTRY ( SDB_CATMAINCT_QUERYREQUEST ) ;

      PD_CHECK( pmdIsPrimary(), SDB_CLS_NOT_PRIMARY, reply, PDWARNING,
                "it is not primary node but received query request!" );

      rc = msgExtractQuery ( (CHAR *)pMsgHeader, &flags, &pCN,
                              &numToSkip, &numToReturn, &pQuery,
                             &pFieldSelector, &pOrderByBuffer,
                             &pHintBuffer ) ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Failed to read query packet, rc = %d", rc ) ;
         rc = SDB_INVALIDARG ;
         goto reply ;
      }
      iNameLen = ossStrlen(pCN) ;
      if ( iNameLen <= 0 || pCN[0]!='$')
      {
         PD_LOG ( PDERROR, "Invalid command-begin" ) ;
         rc = SDB_INVALIDARG ;
         goto reply ;
      }
      try
      {
         BSONObj matcher ( pQuery ) ;
         BSONObj selector ( pFieldSelector ) ;
         BSONObj orderBy ( pOrderByBuffer ) ;
         BSONObj hint ( pHintBuffer ) ;
         rc = rtnQuery( pCollectionName, selector,
                        matcher, orderBy, hint, flags,
                        _pEDUCB, numToSkip, numToReturn,  _pDmsCB,
                        _pRtnCB, contextID ) ;
         if ( rc != SDB_OK )
         {
            if ( rc != SDB_DMS_EOC )
            {
               PD_LOG ( PDERROR, "Failed to list data-node-groups (rc=%d)",
                        rc  ) ;
            }
            goto reply ;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Failed to create arg1 and arg2 for command: %s",
                  e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto reply ;
      }
   reply :
      msgReply.header.messageLength = sizeof(MsgOpReply);
      msgReply.header.opCode = MAKE_REPLY_TYPE( pMsgHeader->opCode );
      msgReply.header.TID = pMsgHeader->TID;
      msgReply.header.routeID.value = 0;
      msgReply.header.requestID = pMsgHeader->requestID;
      msgReply.contextID = contextID;
      msgReply.startFrom = 0;
      msgReply.numReturned = 0;

      if ( rc != SDB_OK )
      {
         msgReply.flags = rc;
         if ( SDB_PERM == rc)
         {
            msgReply.flags = SDB_CLS_NOT_PRIMARY ;
         }
      }
      else
      {
         addContext( handle, contextID );
         msgReply.flags = 0;
      }
      PD_TRACE1 ( SDB_CATMAINCT_QUERYREQUEST,
                  PD_PACK_INT ( msgReply.flags ) ) ;
      rc = _pCatCB->netWork()->syncSend ( handle, &msgReply );
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "failed to send the message(routeID=%lld)",
                  pMsgHeader->routeID.value);   //print the routeID, don't print handle,
                                                //because we can't get any
                                                //useful info from handle
         goto error ;
      }
   done :
      PD_TRACE_EXITRC ( SDB_CATMAINCT_QUERYREQUEST, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 catMainController::processMsg( void *pMsg )
   {
      MsgHeader *pHeader = (MsgHeader *)pMsg ;
      PD_LOG ( PDERROR, "received unknown message: %d", pHeader->opCode ) ;
      return SDB_UNKNOWN_MESSAGE ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_AUTHCRT, "catMainController::processAuthCrt" )
   INT32 catMainController::processAuthCrt( const NET_HANDLE &handle,
                                            const CHAR *pMsg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT_AUTHCRT ) ;
      MsgAuthCrtUsr *msg = ( MsgAuthCrtUsr * )pMsg ;
      BSONObj obj ;
      MsgAuthCrtReply reply ;

      if ( !pmdIsPrimary() )
      {
         rc = SDB_CLS_NOT_PRIMARY ;
         goto error ;
      }

      rc = extractAuthMsg( &(msg->header), obj ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      rc = _pAuthCB->createUsr( obj, _pEDUCB, _pCatCB->majoritySize() ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

   done:
      reply.header.res = rc ;
      PD_TRACE1 ( SDB_CATMAINCT_AUTHCRT,
                  PD_PACK_INT ( rc ) ) ;
      reply.header.header.TID = msg->header.TID ;
      reply.header.header.requestID = msg->header.requestID ;
      _pCatCB->netWork()->syncSend( handle, &reply );
      PD_TRACE_EXITRC ( SDB_CATMAINCT_AUTHCRT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_AUTHENTICATE, "catMainController::processAuthenticate" )
   INT32 catMainController::processAuthenticate( const NET_HANDLE &handle,
                                                 const CHAR *pMsg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT_AUTHENTICATE ) ;
      MsgAuthentication *msg = ( MsgAuthentication * )pMsg ;
      BSONObj obj ;
      MsgAuthReply reply ;

      if ( !pmdIsPrimary() )
      {
         rc = SDB_CLS_NOT_PRIMARY ;
         goto error ;
      }

      if ( !_pAuthCB->needAuthenticate() )
      {
         goto done ;
      }
      rc = extractAuthMsg( &(msg->header), obj ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      rc = _pAuthCB->authenticate( obj, _pEDUCB ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
   done:
      reply.header.res = rc ;
      PD_TRACE1 ( SDB_CATMAINCT_AUTHENTICATE,
                  PD_PACK_INT ( rc ) ) ;
      reply.header.header.TID = msg->header.TID ;
      reply.header.header.requestID = msg->header.requestID ;
      _pCatCB->netWork()->syncSend( handle, &reply );
      PD_TRACE_EXITRC ( SDB_CATMAINCT_AUTHENTICATE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_AUTHDEL, "catMainController::processAuthDel" )
   INT32 catMainController::processAuthDel( const NET_HANDLE &handle,
                                            const CHAR *pMsg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATMAINCT_AUTHDEL ) ;
      MsgAuthDelUsr *msg = ( MsgAuthDelUsr * )pMsg ;
      BSONObj obj ;
      MsgAuthDelReply reply ;

      if ( !pmdIsPrimary() )
      {
         rc = SDB_CLS_NOT_PRIMARY ;
         goto error ;
      }

      rc = extractAuthMsg( &(msg->header), obj ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      rc = _pAuthCB->removeUsr( obj, _pEDUCB, _pCatCB->majoritySize() ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

   done:
      reply.header.res = rc ;
      PD_TRACE1 ( SDB_CATMAINCT_AUTHDEL,
                  PD_PACK_INT ( rc ) ) ;
      reply.header.header.TID = msg->header.TID ;
      reply.header.header.requestID = msg->header.requestID ;
      _pCatCB->netWork()->syncSend( handle, &reply );
      PD_TRACE_EXITRC ( SDB_CATMAINCT_AUTHDEL, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION ( SDB_CATMAINCT_CHECKROUTEID, "catMainController::processCheckRouteID" )
   INT32 catMainController::processCheckRouteID( const NET_HANDLE &handle,
                                                 const CHAR *pMsg )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_CATMAINCT_CHECKROUTEID ) ;
      MsgCoordCheckRouteID *pMsgReq = (MsgCoordCheckRouteID *)pMsg;
      MsgOpReply reply;
      reply.contextID = -1;
      reply.numReturned = 0;
      reply.startFrom = 0;
      reply.header.messageLength = sizeof( MsgOpReply );
      reply.header.opCode = MSG_COOR_CHECK_ROUTEID_RSP;
      reply.header.requestID = pMsgReq->header.requestID;
      reply.header.routeID.value = 0;
      reply.header.TID = pMsgReq->header.TID;
      MsgRouteID localRouteID = _pCatCB->netWork()->localID();
      if ( pMsgReq->dstRouteID.columns.nodeID != localRouteID.columns.nodeID
         || pMsgReq->dstRouteID.columns.groupID != localRouteID.columns.groupID
         || pMsgReq->dstRouteID.columns.serviceID != localRouteID.columns.serviceID )
      {
         rc = SDB_INVALID_ROUTEID;
         PD_LOG ( PDERROR, "routeID is different from the local"
                  "RemoteRouteID(groupID=%u, nodeID=%u, serviceID=%u)"
                  "LocalRouteID(groupID=%u, nodeID=%u, serviceID=%u)",
                  pMsgReq->dstRouteID.columns.groupID,
                  pMsgReq->dstRouteID.columns.nodeID,
                  pMsgReq->dstRouteID.columns.serviceID,
                  localRouteID.columns.groupID,
                  localRouteID.columns.nodeID,
                  localRouteID.columns.serviceID );
      }
      reply.flags = rc;
      _pCatCB->netWork()->syncSend( handle, (void *)&reply );
      PD_TRACE_EXITRC ( SDB_CATMAINCT_CHECKROUTEID, rc ) ;
      return rc;
   }

   void catMainController::addContext( const UINT32 &handle,
                                       INT64 contextID )
   {
      PD_LOG( PDDEBUG, "add context( handle=%u, contextID=%lld )",
            handle, contextID );
      _contextLst.insert( std::make_pair( handle, contextID ) );
   }

   void catMainController::delContext( const UINT32 &handle )
   {
      PD_LOG ( PDDEBUG, "delete context( handle=%u )",
               handle );
      CONTEXT_LIST::iterator iterMap;
      iterMap = _contextLst.find( handle );
      while ( iterMap != _contextLst.end()
            && iterMap->first == handle )
      {
         _pRtnCB->contextDelete( iterMap->second, _pEDUCB );
         _contextLst.erase( iterMap++ );
      }
   }

   void catMainController::delContext( const UINT32 &handle, INT64 contextID )
   {
      PD_LOG ( PDDEBUG, "delete context( handle=%u, contextID=%lld )",
               handle, contextID );
      CONTEXT_LIST::iterator iterMap;
      iterMap = _contextLst.find( handle );
      while ( iterMap != _contextLst.end()
            && iterMap->first == handle )
      {
         if ( iterMap->second == contextID )
         {
            _contextLst.erase( iterMap );
            break;
         }
         ++iterMap;
      }
   }
}
