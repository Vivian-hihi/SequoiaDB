
#include "core.hpp"
#include "pmdCB.hpp"
#include "pd.hpp"
#include "rtn.hpp"
#include "catDef.hpp"
#include "catCatalogManager.hpp"
#include "rtnPredicate.hpp"
#include "msgMessage.hpp"
#include "ixmIndexKey.hpp"
#include "pdTrace.hpp"
#include "catTrace.hpp"
#include "catCommon.hpp"
#include "clsCatalogAgent.hpp"

using namespace bson;

namespace engine
{
   catCatalogueManager::catCatalogueManager( pmdEDUCB *cb )
   {
      _pEduCB = cb;
   }

   INT32 catCatalogueManager::active()
   {
      _taskMgr.setTaskID( catGetMaxTaskID( _pEduCB ) ) ;
      INT32 rc = SDB_OK;
      return rc;
   }

   INT32 catCatalogueManager::deactive()
   {
      INT32 rc = SDB_OK;
      return rc;
   }

   INT32 catCatalogueManager::init()
   {
      _pKrcb   = pmdGetKRCB();
      _pDmsCB  = _pKrcb->getDMSCB();
      _pDpsCB  = _pKrcb->getDPSCB();
      _pCatCB  = _pKrcb->getCATLOGUECB();
      _pRtnCB  = _pKrcb->getRTNCB();
      _pClsCB  = _pKrcb->getClsCB();
      return SDB_OK;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_DROPCS, "catCatalogueManager::processCmdDropCollectionSpace" )
   INT32 catCatalogueManager::processCmdDropCollectionSpace ( const CHAR *pQuery )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_DROPCS ) ;

      try
      {
         BSONObj boQuery( pQuery ) ;
         BSONElement beSpaceName =
            boQuery.getField ( CAT_COLLECTION_SPACE_NAME ) ;
         PD_CHECK ( beSpaceName.type() == String, SDB_INVALIDARG, error,
                    PDERROR, "Field[%s] type[%d] is not String",
                    CAT_COLLECTION_SPACE_NAME, beSpaceName.type() ) ;
         PD_TRACE1 ( SDB_CATALOGMGR_DROPCS,
                     PD_PACK_STRING ( beSpaceName.valuestr() ) ) ;
         rc = catRemoveCSEx( beSpaceName.valuestr(), _pEduCB, _pDmsCB, _pDpsCB,
                             _majoritySize() ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to drop collection space %s, "
                       "rc = %d", beSpaceName.valuestr(), rc ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_DROPCS, rc ) ;
      return rc;
   error :
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_CRT_PROCEDURES, "catCatalogueManager::processCmdCrtProcedures")
   INT32 catCatalogueManager::processCmdCrtProcedures( void *pMsg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_CATALOGMGR_CRT_PROCEDURES ) ;
      try
      {
         BSONObj func( (const CHAR *)pMsg ) ;
         BSONObj parsed ;
         rc = catPraseFunc( func, parsed ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to parse store procedures:%s",
                    func.toString().c_str() ) ;
            goto error ;
         }

         rc = rtnInsert( CAT_PROCEDURES_COLLECTION,
                         parsed, 1, 0,
                         _pEduCB, _pDmsCB, _pDpsCB, _majoritySize() ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to add func:%s", 
                    parsed.toString().c_str() ) ;
            goto error ;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s",e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB_CATALOGMGR_CRT_PROCEDURES, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_RM_PROCEDURES, "catCatalogueManager::processCmdRmProcedures")
   INT32 catCatalogueManager::processCmdRmProcedures( void *pMsg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_CATALOGMGR_RM_PROCEDURES ) ;
      try
      {
         BSONObj obj( (const CHAR *)pMsg ) ;
         BSONElement name = obj.getField( FIELD_NAME_FUNC ) ;
         if ( name.eoo() || String != name.type() )
         {
            PD_LOG( PDERROR, "invalid type of func name[%s:%d]",
                    name.toString().c_str(), name.type()  ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         {
         BSONObjBuilder builder ;
         BSONObj deletor ;
         BSONObj dummy ;
         BSONObj func ;
         builder.appendAs( name, FMP_FUNC_NAME ) ;
         deletor = builder.obj() ;

         rc = catGetOneObj( CAT_PROCEDURES_COLLECTION,
                            dummy, deletor, dummy,
                            _pEduCB, func ) ;
         if ( SDB_DMS_EOC == rc )
         {
            PD_LOG( PDERROR, "func %s is not exist",
                    deletor.toString().c_str() ) ;
            rc = SDB_FMP_FUNC_NOT_EXIST ;
            goto error ;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get func:%s, rc=%d",
                    deletor.toString().c_str(), rc ) ; 
            goto error ;
         }

         rc = rtnDelete( CAT_PROCEDURES_COLLECTION, 
                         deletor, BSONObj(),
                         0, _pEduCB, _pDmsCB, _pDpsCB, _majoritySize() ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to rm func:%s",
                    deletor.toString().c_str() ) ;
            goto error ;
         }
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      
   done:
      PD_TRACE_EXITRC( SDB_CATALOGMGR_RM_PROCEDURES, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_QUERYSPACEINFO, "catCatalogueManager::processCmdQuerySpaceInfo" )
   INT32 catCatalogueManager::processCmdQuerySpaceInfo( const CHAR * pQuery,
                                                        CHAR * * ppReplyBody,
                                                        UINT32 & replyBodyLen,
                                                        INT32 & returnNum )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_QUERYSPACEINFO ) ;
      const CHAR *csName = NULL ;
      BSONObj boSpace ;
      BOOLEAN isExist = FALSE ;

      try
      {
         BSONObj boQuery( pQuery ) ;
         rtnGetStringElement( boQuery,  CAT_COLLECTION_SPACE_NAME, &csName ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                      CAT_COLLECTION_SPACE_NAME, rc ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Occur exception: %s", e.what() ) ;
         goto error ;
      }

      PD_TRACE1 ( SDB_CATALOGMGR_QUERYSPACEINFO, PD_PACK_STRING ( csName ) ) ;

      rc = catCheckSpaceExist( csName, isExist, boSpace, _pEduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Check collection space[%s] exist failed, "
                   "rc: %d", csName, rc ) ;
      PD_TRACE1 ( SDB_CATALOGMGR_QUERYSPACEINFO,PD_PACK_INT ( isExist ) ) ;

      if ( !isExist )
      {
         rc = SDB_DMS_CS_NOTEXIST ;
         goto done ;
      }

      returnNum = 1 ;
      replyBodyLen = boSpace.objsize() ;
      *ppReplyBody = ( CHAR* )SDB_OSS_MALLOC( replyBodyLen ) ;
      PD_CHECK( *ppReplyBody, SDB_OOM, error, PDERROR,
                "Failed to alloc memry, size: %d", replyBodyLen ) ;

      ossMemcpy( *ppReplyBody, boSpace.objdata(), replyBodyLen ) ;

   done:
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_QUERYSPACEINFO, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // this function is for catalog collection check
   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_QUERYCATALOG, "catCatalogueManager::processQueryCatalogue" )
   INT32 catCatalogueManager::processQueryCatalogue ( void *pMsg )
   {
      INT32 rc                         = SDB_OK;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_QUERYCATALOG ) ;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent*)pMsg;
      MsgCatQueryCatReq *pCatReq       = (MsgCatQueryCatReq*)(pEvent->data);
      MsgOpReply *pReply               = NULL;

      // make sure we are on catalog primary
      PD_CHECK ( _pCatCB->isPrimary(),
                 SDB_CLS_NOT_PRIMARY, error, PDWARNING,
                 "service deactive but received query catalogue request" ) ;

      // sanity check, header can't be too small
      PD_CHECK ( pCatReq->header.messageLength >=
                 (INT32)sizeof(MsgCatQueryCatReq),
                 SDB_INVALIDARG, error, PDERROR,
                 "recived unexpected query catalogue request, "
                 "message length(%d) is invalied",
                 pCatReq->header.messageLength ) ;
      // extract and query
      try
      {
         CHAR *pCollectionName = NULL ;
         SINT32 flag           = 0 ;
         SINT64 numToSkip      = 0 ;
         SINT64 numToReturn    = -1 ;
         CHAR *pQuery          = NULL ;
         CHAR *pFieldSelector  = NULL ;
         CHAR *pOrderBy        = NULL ;
         CHAR *pHint           = NULL ;
         rc = msgExtractQuery  ( (CHAR *)pCatReq, &flag, &pCollectionName,
                                 &numToSkip, &numToReturn, &pQuery,
                                 &pFieldSelector, &pOrderBy, &pHint ) ;
         BSONObj matcher(pQuery);
         BSONObj selector(pFieldSelector);
         BSONObj orderBy(pOrderBy);
         BSONObj hint(pHint);
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to extract message, rc = %d", rc ) ;
         // perform catalog query, result buffer will be placed in pReply, and
         // we are responsible to free it by end of the function
         rc = catQueryAndGetMore ( &pReply, CAT_COLLECTION_INFO_COLLECTION,
                                   selector, matcher, orderBy, hint, flag,
                                   _pEduCB, numToSkip, numToReturn ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to query from catalog, rc = %d", rc ) ;
         // check for how many records were returned
         // need to make sure returned record must be one
         // 1) if returned = 0, it means collection does not exist
         // 2) if returned > 1, it means possible catalog corruption
         PD_CHECK ( pReply->numReturned >= 1, SDB_DMS_NOTEXIST, error,
                    PDWARNING, "Collection does not exist" ) ;
         PD_CHECK ( pReply->numReturned <= 1, SDB_CAT_CORRUPTION, error,
                    PDSEVERE,
                    "More than one records returned for query, "
                    "possible catalog corruption" ) ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception during query catalogue request:%s",
                       e.what() ) ;
      }
      pReply->header.opCode        = MSG_CAT_QUERY_CATALOG_RSP ;
      pReply->header.TID           = pCatReq->header.TID ;
      pReply->header.requestID     = pCatReq->header.requestID ;
      pReply->header.routeID.value = 0 ;
   done :
      if ( SDB_OK == rc && NULL != pReply )
      {
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, pReply );
         SDB_OSS_FREE ( pReply );
      }
      else
      {
         MsgOpReply replyMsg;
         replyMsg.header.messageLength = sizeof( MsgOpReply );
         replyMsg.header.opCode        = MSG_CAT_QUERY_CATALOG_RSP;
         replyMsg.header.TID           = pCatReq->header.TID;
         replyMsg.header.routeID.value = 0;
         replyMsg.header.requestID     = pCatReq->header.requestID;
         replyMsg.numReturned          = 0;
         replyMsg.flags                = rc;
         replyMsg.contextID            = -1 ;
         PD_TRACE1 ( SDB_CATALOGMGR_QUERYCATALOG,
                     PD_PACK_INT ( rc ) ) ;
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, &replyMsg );
      }
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_QUERYCATALOG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_DROPCOLLECTION, "catCatalogueManager::processCmdDropCollection" )
   INT32 catCatalogueManager::processCmdDropCollection( const CHAR *pQuery )
   {
      INT32 rc                         = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_DROPCOLLECTION ) ;

      const CHAR *strName              = NULL ;

      try
      {
         BSONObj boDeletor = BSONObj ( pQuery ) ;
         BSONElement beName = boDeletor.getField( CAT_COLLECTION_NAME ) ;
         PD_CHECK ( String == beName.type(), SDB_INVALIDARG, error,
                    PDERROR, "Failed to drop the collection, get collection "
                    "name failed, type: %d", beName.type() ) ;
         strName = beName.valuestr() ;
         PD_TRACE1 ( SDB_CATALOGMGR_DROPCOLLECTION,
                     PD_PACK_STRING ( strName ) ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = catRemoveCLEx( strName, _pEduCB, _pDmsCB, _pDpsCB,
                          _majoritySize() ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to drop collection %s from catalog, "
                    "rc = %d", strName, rc ) ;

   done :
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_DROPCOLLECTION, rc ) ;
      return rc;
   error :
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_QUERYTASK, "catCatalogueManager::processQueryTask" )
   INT32 catCatalogueManager::processQueryTask ( void *pMsg )
   {
      INT32 rc                         = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_QUERYTASK ) ;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent *)pMsg ;
      MsgCatQueryTaskReq *pTaskRequest = (MsgCatQueryTaskReq*)(pEvent->data) ;
      MsgCatQueryTaskRes *pReply       = NULL ;
      INT32 flag                       = 0 ;
      SINT64 numToSkip                 = 0 ;
      SINT64 numToReturn               = 0 ;
      CHAR *pQuery                     = NULL ;
      CHAR *pFieldSelector             = NULL ;
      CHAR *pOrderBy                   = NULL ;
      CHAR *pHint                      = NULL ;
      CHAR *pCollectionName            = NULL ;

      // make sure we are primary in order to query tasks
      PD_CHECK ( _pCatCB->isPrimary(),
                 SDB_CLS_NOT_PRIMARY, error, PDWARNING,
                 "service deactive but received query catalogue request" ) ;

      // sanity check, the query length should be at least header size
      PD_CHECK ( pTaskRequest->header.messageLength >=
                 (INT32)sizeof(MsgCatQueryTaskReq),
                 SDB_INVALIDARG, error, PDERROR,
                 "received unexpected query task request, "
                 "message length(%d) is invalid",
                 pTaskRequest->header.messageLength ) ;

      try
      {
         // extract the request message
         rc = msgExtractQuery ( (CHAR*)pTaskRequest, &flag, &pCollectionName,
                                &numToSkip, &numToReturn, &pQuery,
                                &pFieldSelector, &pOrderBy, &pHint ) ;
         BSONObj matcher ( pQuery ) ;
         BSONObj selector ( pFieldSelector ) ;
         BSONObj orderBy ( pOrderBy );
         BSONObj hint ( pHint ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to extract message, rc = %d", rc ) ;
         // pReply will be allocated by catQueryAndGetMore, we are 
         // responsible to free the memory
         rc = catQueryAndGetMore ( &pReply, CAT_TASK_INFO_COLLECTION,
                                   selector, matcher, orderBy, hint, flag,
                                   _pEduCB, numToSkip, numToReturn ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to perform query from catalog, rc = %d", rc ) ;

         // If there's no task satisfy the request, let's return SDB_CAT_TASK_NOTFOUND,
         // otherwise return all tasks satisfy the request
         PD_CHECK ( pReply->numReturned >= 1, SDB_CAT_TASK_NOTFOUND, error,
                    PDINFO, "Task does not exist" ) ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception when extracting query task: %s",
                       e.what() ) ;
      }
      // construct reply header to match the request
      pReply->header.opCode        = MSG_CAT_QUERY_TASK_RSP ;
      pReply->header.TID           = pTaskRequest->header.TID ;
      pReply->header.requestID     = pTaskRequest->header.requestID ;
      pReply->header.routeID.value = 0 ;

   done :
      if ( SDB_OK == rc && pReply )
      {
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, pReply );
         SDB_OSS_FREE ( pReply ) ;
      }
      else
      {
         // if something wrong happened, return a reply with rc
         MsgOpReply replyMsg;
         replyMsg.header.messageLength = sizeof( MsgOpReply );
         replyMsg.header.opCode        = MSG_CAT_QUERY_TASK_RSP ;
         replyMsg.header.TID           = pTaskRequest->header.TID;
         replyMsg.header.routeID.value = 0;
         replyMsg.header.requestID     = pTaskRequest->header.requestID;
         replyMsg.numReturned          = 0;
         replyMsg.flags                = rc;
         replyMsg.contextID            = -1 ;
         PD_TRACE1 ( SDB_CATALOGMGR_QUERYTASK, PD_PACK_INT ( rc ) ) ;
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, &replyMsg );
      }
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_QUERYTASK, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_ALTERCOLLECTION, "catCatalogueManager::processAlterCollection" )
   INT32 catCatalogueManager::processAlterCollection ( void *pMsg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_ALTERCOLLECTION ) ;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent*)pMsg;
      MsgOpQuery *pAlterReq            = (MsgOpQuery*)(pEvent->data) ;
      INT32 flag                       = 0 ;
      CHAR *pCommandName               = NULL ;
      SINT64 numToSkip                 = 0 ;
      SINT64 numToReturn               = -1 ;
      CHAR *pQuery                     = NULL ;
      CHAR *pFieldSelector             = NULL ;
      CHAR *pOrderBy                   = NULL ;
      CHAR *pHint                      = NULL ;
      // preconstruct reply message
      MsgOpReply replyMsg;
      replyMsg.header.messageLength = sizeof( MsgOpReply );
      replyMsg.header.opCode = MSG_CAT_ALTER_COLLECTION_RSP;

      // collection name related
      const CHAR *strName              = NULL ;
      BSONObj options ;
      BOOLEAN isCollectionExist        = FALSE ;
      BSONObj boCollectionRecord ;
      BSONObjBuilder bbUpdateRequest ;
      BSONObj boUpdateRequest ;
      BSONObj matcher ;
      BSONObj hint ;
      // we shouldn't alter collection on non-primary catalog
      PD_CHECK ( _pCatCB->isPrimary(),
                 SDB_CLS_NOT_PRIMARY, error, PDWARNING,
                 "service deactive but received alter collection request" );
      // extract the request
      rc = msgExtractQuery( (CHAR *)pAlterReq, &flag, &pCommandName,
                            &numToSkip, &numToReturn, &pQuery,
                            &pFieldSelector, &pOrderBy, &pHint );
      PD_RC_CHECK ( rc, PDERROR,
                    "failed to parsed the msg:create-collection request(rc=%d)",
                    rc ) ;
      try
      {
         BSONObj boAlterObj ( pQuery ) ;
         // make sure collection name exists
         BSONElement beName = boAlterObj.getField ( CAT_COLLECTION_NAME ) ;
         BSONElement beOptions = boAlterObj.getField ( CAT_OPTIONS_NAME ) ;
         PD_CHECK ( String == beName.type(), SDB_INVALIDARG, error, PDERROR,
                    "Invalid field %s", CAT_COLLECTION_NAME ) ;
         strName = beName.valuestr() ;
         PD_TRACE1 ( SDB_CATALOGMGR_ALTERCOLLECTION,
                     PD_PACK_STRING ( strName ) ) ;
         // make sure options exists
         PD_CHECK ( Object == beOptions.type(), SDB_INVALIDARG, error, PDERROR,
                    "Invalid field %s", CAT_OPTIONS_NAME ) ;
         // make sure each elements are valid
         BSONObjIterator i( beOptions.embeddedObject() );
         while ( i.more() )
         {
            BSONElement beTmp = i.next();
            const CHAR *pFieldName = beTmp.fieldName () ;
            // if we are changing w, let's set update request
            if ( ossStrcmp ( CAT_CATALOG_W_NAME, pFieldName ) == 0 )
            {
               PD_CHECK ( NumberInt == beTmp.type(), SDB_INVALIDARG, error,
                          PDERROR,
                          "Invalid field %s", CAT_CATALOG_W_NAME ) ;
               bbUpdateRequest.append ( "$set", beTmp.wrap() ) ;
            }
            else
            {
               // for any other request, let's mark invalid
               PD_RC_CHECK ( SDB_INVALIDARG, PDERROR,
                             "Invalid field %s in options for alter collection",
                             pFieldName ) ;
            }
         }
         // make sure collection exists
         rc = catCheckCollectionExist ( strName, isCollectionExist,
                                        boCollectionRecord, _pEduCB ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to detect collection existence, rc = %d", rc ) ;
         // build update request and search condition
         boUpdateRequest = bbUpdateRequest.obj () ;
         matcher = BSON ( CAT_COLLECTION_NAME << strName ) ;
         // perform update
         rc = rtnUpdate ( CAT_COLLECTION_INFO_COLLECTION,
                          matcher, boUpdateRequest, hint,
                          0, _pEduCB, _pDmsCB, _pDpsCB,
                          _majoritySize() );
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to alter collection, rc = %d", rc ) ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception hit when parsing alter collection: %s",
                       e.what() ) ;
      }
   done :
      replyMsg.header.TID = pAlterReq->header.TID;
      replyMsg.header.routeID.value = 0;
      replyMsg.header.requestID = pAlterReq->header.requestID;
      replyMsg.numReturned = 0;
      replyMsg.flags = rc;
      PD_TRACE1 ( SDB_CATALOGMGR_ALTERCOLLECTION, PD_PACK_INT ( rc ) ) ;
      rc = _pCatCB->netWork()->syncSend ( pEvent->handle, &replyMsg );
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_ALTERCOLLECTION, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_CREATECS, "catCatalogueManager::processCmdCreateCS" )
   INT32 catCatalogueManager::processCmdCreateCS( const CHAR * pQuery,
                                                  const CHAR *pSelector,
                                                  CHAR * * ppReplyBody,
                                                  UINT32 & replyBodyLen,
                                                  INT32 & returnNum )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_CREATECS ) ;
      INT32 groupID = CAT_INVALID_GROUPID ;

      try
      {
         BSONObj groupObj ;
         BSONObj query( pQuery ) ;
         BSONObj selector( pSelector ) ;
         rc = _createCS( query, selector, groupID ) ;
         PD_RC_CHECK( rc, PDERROR, "Create collection space failed, rc: %d",
                      rc ) ;

         rc = catGetGroupObj( (UINT32)groupID, groupObj, _pEduCB ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get group obj by id[%d], rc: %d",
                      groupID, rc ) ;

         // reply construct
         {
            returnNum = 1 ;

            BSONObjBuilder replyBuild ;
            BSONObjBuilder sub( replyBuild.subarrayStart( CAT_GROUP_NAME ) ) ;
            sub.append( "0", groupObj ) ;
            sub.done() ;
            BSONObj replyObj = replyBuild.obj() ;

            replyBodyLen = replyObj.objsize() ;
            *ppReplyBody = (CHAR*)SDB_OSS_MALLOC( replyBodyLen ) ;

            PD_CHECK( *ppReplyBody, SDB_OOM, error, PDERROR,
                      "Failed to alloc memry, size: %d", replyBodyLen ) ;

            ossMemcpy( *ppReplyBody, replyObj.objdata(), replyBodyLen ) ;
         }
      }
      catch( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Occurred exception: %s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_CREATECS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 catCatalogueManager::processCmdCreateCL( const CHAR *pQuery,
                                                  const CHAR *pSelector,
                                                  CHAR **ppReplyBody,
                                                  UINT32 &replyBodyLen,
                                                  INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;
      INT32 groupID = CAT_INVALID_GROUPID ;

      try
      {
         BSONObj query( pQuery ) ;
         BSONObj selector( pSelector ) ;
         rc = _createCL( query, selector, groupID ) ;
         PD_RC_CHECK( rc, PDERROR, "Create collection failed, rc: %d", rc ) ;

         // reply construct
         {
            returnNum = 1 ;

            BSONObjBuilder replyBuild ;
            replyBuild.append( CAT_CATALOGVERSION_NAME, CAT_VERSION_BEGIN ) ;
            BSONObjBuilder sub( replyBuild.subarrayStart( CAT_GROUP_NAME ) ) ;
            sub.append( "0", BSON( CAT_GROUPID_NAME << groupID ) ) ;
            sub.done() ;
            BSONObj replyObj = replyBuild.obj() ;

            replyBodyLen = replyObj.objsize() ;
            *ppReplyBody = (CHAR*)SDB_OSS_MALLOC( replyBodyLen ) ;

            PD_CHECK( *ppReplyBody, SDB_OOM, error, PDERROR,
                      "Failed to alloc memry, size: %d", replyBodyLen ) ;

            ossMemcpy( *ppReplyBody, replyObj.objdata(), replyBodyLen ) ;
         }
      }
      catch( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Occurred exception: %s", e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catCatalogueManager::processCmdSplit( const CHAR * pQuery,
                                               INT32 opCode,
                                               CHAR * * ppReplyBody,
                                               UINT32 & replyBodyLen,
                                               INT32 & returnNum,
                                               BOOLEAN &fillPeerRouteID )
   {
      INT32 rc = SDB_OK ;
      const CHAR *clFullName = NULL ;
      clsCatalogSet *pCataSet = NULL ;
      INT32 groupID = CAT_INVALID_GROUPID ;
      UINT64 taskID = CLS_INVALID_TASKID ;

      try
      {
         BSONObj cataObj ;
         BOOLEAN clExist = FALSE ;
         BSONObj query( pQuery ) ;

         if ( MSG_CAT_SPLIT_PREPARE_REQ == opCode ||
              MSG_CAT_SPLIT_READY_REQ == opCode ||
              MSG_CAT_SPLIT_CHGMETA_REQ == opCode )
         {
            rc = rtnGetStringElement( query, CAT_COLLECTION_NAME,
                                      &clFullName ) ;
            PD_RC_CHECK( rc, PDERROR, "Get split collection name failed, "
                         "rc: %d, info: %s", rc, query.toString().c_str() ) ;

            // get catalog info
            rc = catCheckCollectionExist( clFullName, clExist, cataObj,
                                          _pEduCB ) ;
            PD_RC_CHECK( rc, PDERROR, "Check collection exist failed, rc: %d",
                         rc ) ;
            PD_CHECK( clExist, SDB_DMS_NOTEXIST, error, PDWARNING,
                      "Collection[%s] is no longer existed", clFullName ) ;

            // update catalog set
            pCataSet = SDB_OSS_NEW clsCatalogSet( clFullName, TRUE ) ;
            PD_CHECK( pCataSet, SDB_OOM, error, PDERROR, "Alloc failed" ) ;
            rc = pCataSet->updateCatSet( cataObj, 0 ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to update catalog set, cata "
                         "info: %s, rc: %d", cataObj.toString().c_str(), rc ) ;

            // check collection sharding
            PD_CHECK( pCataSet->isSharding(), SDB_COLLECTION_NOTSHARD, error,
                      PDERROR, "Collection[%s] is not sharding, can't split",
                      clFullName ) ;
         }

         // dispatch
         switch ( opCode )
         {
            case MSG_CAT_SPLIT_PREPARE_REQ :
               rc = catSplitPrepare( query, clFullName, pCataSet,
                                     groupID, _pEduCB ) ;
               break ;
            case MSG_CAT_SPLIT_READY_REQ :
               rc = catSplitReady( query, clFullName, pCataSet, groupID,
                                   _taskMgr, _pEduCB, _majoritySize(),
                                   &taskID ) ;
               break ;
            case MSG_CAT_SPLIT_CANCEL_REQ :
               rc = catSplitCancel( query, _pEduCB, groupID, _majoritySize() ) ;
               break ;
            case MSG_CAT_SPLIT_START_REQ :
               fillPeerRouteID = TRUE ;
               rc = catSplitStart( query, _pEduCB, _majoritySize() ) ;
               break ;
            case MSG_CAT_SPLIT_CHGMETA_REQ :
               fillPeerRouteID = TRUE ;
               rc = catSplitChgMeta( query, clFullName, pCataSet, _pEduCB,
                                     _majoritySize() ) ;
               break ;
            case MSG_CAT_SPLIT_CLEANUP_REQ :
               fillPeerRouteID = TRUE ;
               rc = catSplitCleanup( query, _pEduCB, _majoritySize() ) ;
               break ;
            case MSG_CAT_SPLIT_FINISH_REQ :
               fillPeerRouteID = TRUE ;
               rc = catSplitFinish( query, _pEduCB, _majoritySize() ) ;
               break ;
            default :
               rc = SDB_INVALIDARG ;
               break ;
         }

         PD_RC_CHECK( rc, PDERROR, "Split collection failed, opCode: %d, "
                      "rc: %d", opCode, rc ) ;

         // reply construct
         if ( CAT_INVALID_GROUPID != groupID )
         {
            returnNum = 1 ;

            BSONObjBuilder replyBuild ;
            if ( pCataSet )
            {
               replyBuild.append( CAT_CATALOGVERSION_NAME,
                                  pCataSet->getVersion() ) ;
            }
            if ( CLS_INVALID_TASKID != taskID )
            {
               replyBuild.append( CAT_TASKID_NAME, (long long)taskID ) ;
            }
            BSONObjBuilder sub( replyBuild.subarrayStart( CAT_GROUP_NAME ) ) ;
            sub.append( "0", BSON( CAT_GROUPID_NAME << groupID ) ) ;
            sub.done() ;
            BSONObj replyObj = replyBuild.obj() ;

            replyBodyLen = replyObj.objsize() ;
            *ppReplyBody = (CHAR*)SDB_OSS_MALLOC( replyBodyLen ) ;

            PD_CHECK( *ppReplyBody, SDB_OOM, error, PDERROR,
                      "Failed to alloc memry, size: %d", replyBodyLen ) ;

            ossMemcpy( *ppReplyBody, replyObj.objdata(), replyBodyLen ) ;
         }
      }
      catch( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Occurred exception: %s", e.what() ) ;
         goto error ;
      }

   done:
      if ( pCataSet )
      {
         SDB_OSS_DEL pCataSet ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 catCatalogueManager::_checkCSObj( const BSONObj & infoObj,
                                           catCSInfo & csInfo )
   {
      INT32 rc = SDB_OK ;

      csInfo._pCSName = NULL ;
      csInfo._domainName = NULL ;
      csInfo._pageSize = DMS_PAGE_SIZE_DFT ;

      BSONObjIterator it( infoObj ) ;
      while ( it.more() )
      {
         BSONElement ele = it.next() ;

         // name
         if ( 0 == ossStrcmp( ele.fieldName(), CAT_COLLECTION_SPACE_NAME ) )
         {
            PD_CHECK( String == ele.type(), SDB_INVALIDARG, error, PDERROR,
                      "Field[%s] type[%d] error", CAT_COLLECTION_NAME,
                      ele.type() ) ;
            csInfo._pCSName = ele.valuestr() ;
         }
         // page size
         else if ( 0 == ossStrcmp( ele.fieldName(), CAT_PAGE_SIZE_NAME ) )
         {
            PD_CHECK( ele.isNumber(), SDB_INVALIDARG, error, PDERROR,
                      "Field[%s] type[%d] error", CAT_PAGE_SIZE_NAME,
                      ele.type() ) ;
            csInfo._pageSize = ele.numberInt() ;

            // check size value
            PD_CHECK ( csInfo._pageSize == DMS_PAGE_SIZE4K ||
                       csInfo._pageSize == DMS_PAGE_SIZE8K ||
                       csInfo._pageSize == DMS_PAGE_SIZE16K ||
                       csInfo._pageSize == DMS_PAGE_SIZE32K ||
                       csInfo._pageSize == DMS_PAGE_SIZE64K, SDB_INVALIDARG,
                       error, PDERROR, "PageSize must be 4K/8K/16K/32K/64K" ) ;
         }
         // domain name
         else if ( 0 == ossStrcmp( ele.fieldName(), CAT_DOMAINNAME_NAME ) )
         {
            PD_CHECK( String == ele.type(), SDB_INVALIDARG, error, PDERROR,
                      "Field[%s] type[%d] error", CAT_DOMAINNAME_NAME,
                      ele.type() ) ;
            csInfo._domainName = ele.valuestr() ;
         }
         else
         {
            PD_RC_CHECK ( SDB_INVALIDARG, PDERROR,
                          "Unexpected field[%s] in create collection space "
                          "command", ele.toString().c_str() ) ;
         }
      }

      PD_CHECK( csInfo._pCSName, SDB_INVALIDARG, error, PDERROR,
                "Collection space name not set" ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catCatalogueManager::_checkAndBuildCataRecord( const BSONObj &infoObj,
                                                        UINT32 &fieldMask,
                                                        catCollectionInfo &clInfo )
   {
      INT32 rc = SDB_OK ;

      clInfo._pCLName            = NULL ;
      clInfo._replSize           = 1 ;
      clInfo._enSureShardIndex   = true ;
      clInfo._pShardingType      = CAT_SHARDING_TYPE_RANGE ;
      clInfo._shardPartition     = CAT_SHARDING_PARTITION_DEFAULT ;
      clInfo._isHash             = FALSE ;
      clInfo._isSharding         = FALSE ;
      clInfo._isMainCL           = false;

      fieldMask = 0 ;

      BSONObjIterator it( infoObj ) ;
      while ( it.more() )
      {
         BSONElement eleTmp = it.next() ;

         // collection name
         if ( ossStrcmp( eleTmp.fieldName(), CAT_COLLECTION_NAME ) == 0 )
         {
            PD_CHECK( String == eleTmp.type(), SDB_INVALIDARG, error, PDERROR,
                      "Field[%s] type[%d] error", CAT_COLLECTION_NAME,
                      eleTmp.type() ) ;
            clInfo._pCLName = eleTmp.valuestr() ;
            fieldMask |= CAT_MASK_CLNAME ;
         }
         // sharding key
         else if ( ossStrcmp( eleTmp.fieldName(),
                              CAT_SHARDINGKEY_NAME ) == 0 )
         {
            PD_CHECK( Object == eleTmp.type(), SDB_INVALIDARG, error, PDERROR,
                      "Field[%s] type[%d] error", CAT_SHARDINGKEY_NAME,
                      eleTmp.type() ) ;
            clInfo._shardingKey = eleTmp.embeddedObject() ;
            PD_CHECK( _ixmIndexKeyGen::validateKeyDef( clInfo._shardingKey ),
                      SDB_INVALIDARG, error, PDERROR,
                      "Sharding key[%s] definition is invalid",
                      clInfo._shardingKey.toString().c_str() ) ;
            fieldMask |= CAT_MASK_SHDKEY ;
            clInfo._isSharding = TRUE ;
         }
         // repl size
         else if ( ossStrcmp( eleTmp.fieldName(), CAT_CATALOG_W_NAME ) == 0 )
         {
            PD_CHECK( NumberInt == eleTmp.type(), SDB_INVALIDARG, error,
                      PDERROR, "Field[%s] type[%d] error", CAT_CATALOG_W_NAME,
                      eleTmp.type() ) ;
            clInfo._replSize = eleTmp.numberInt() ;
            if ( clInfo._replSize <= 0 )
            {
               clInfo._replSize = CLS_REPLSET_MAX_NODE_SIZE ;
            }

            PD_CHECK( clInfo._replSize <= CLS_REPLSET_MAX_NODE_SIZE,
                      SDB_INVALIDARG, error, PDERROR,
                      "Field[%s] value should less than or equal to %d",
                      CAT_CATALOG_W_NAME, CLS_REPLSET_MAX_NODE_SIZE ) ;
            fieldMask |= CAT_MASK_REPLSIZE ;
         }
         // ensure sharding index
         else if ( ossStrcmp( eleTmp.fieldName(), CAT_ENSURE_SHDINDEX ) == 0 )
         {
            PD_CHECK( Bool == eleTmp.type(), SDB_INVALIDARG, error,
                      PDERROR, "Field[%s] type[%d] error", CAT_ENSURE_SHDINDEX,
                      eleTmp.type() ) ;
            clInfo._enSureShardIndex = eleTmp.Bool() ;
            fieldMask |= CAT_MASK_SHDIDX ;
         }
         // sharding type
         else if ( ossStrcmp( eleTmp.fieldName(), CAT_SHARDING_TYPE ) == 0 )
         {
            PD_CHECK( String == eleTmp.type(), SDB_INVALIDARG, error,
                      PDERROR, "Field[%s] type[%d] error", CAT_SHARDING_TYPE,
                      eleTmp.type() ) ;

            // check string value
            clInfo._pShardingType = eleTmp.valuestr() ;
            PD_CHECK( 0 == ossStrcmp( clInfo._pShardingType,
                                      CAT_SHARDING_TYPE_HASH ) ||
                      0 == ossStrcmp( clInfo._pShardingType,
                                      CAT_SHARDING_TYPE_RANGE ),
                      SDB_INVALIDARG, error, PDERROR,
                      "Field[%s] value[%s] should be[%s/%s]",
                      CAT_SHARDING_TYPE, clInfo._pShardingType,
                      CAT_SHARDING_TYPE_HASH, CAT_SHARDING_TYPE_RANGE ) ;
            fieldMask |= CAT_MASK_SHDTYPE ;

            if ( 0 == ossStrcmp( clInfo._pShardingType,
                                 CAT_SHARDING_TYPE_HASH ) )
            {
               clInfo._isHash = TRUE ;
            }
         }
         // sharding partition
         else if ( ossStrcmp( eleTmp.fieldName(),
                              CAT_SHARDING_PARTITION ) == 0 )
         {
            PD_CHECK( NumberInt == eleTmp.type(), SDB_INVALIDARG, error,
                      PDERROR, "Field[%s] type[%d] error",
                      CAT_SHARDING_PARTITION, eleTmp.type() ) ;
            clInfo._shardPartition = eleTmp.numberInt() ;
            // must be the power of 2
            PD_CHECK( ossIsPowerOf2( (UINT32)clInfo._shardPartition ),
                      SDB_INVALIDARG, error, PDERROR,
                      "Field[%s] value must be power of 2",
                      CAT_SHARDING_PARTITION ) ;
            PD_CHECK( clInfo._shardPartition >= CAT_SHARDING_PARTITION_MIN &&
                      clInfo._shardPartition <= CAT_SHARDING_PARTITION_MAX,
                      SDB_INVALIDARG, error, PDERROR, "Field[%s] value[%d] "
                      "should between in[%d, %d]", CAT_SHARDING_PARTITION,
                      clInfo._shardPartition, CAT_SHARDING_PARTITION_MIN,
                      CAT_SHARDING_PARTITION_MAX ) ;
            fieldMask |= CAT_MASK_SHDPARTITION ;
         }
         // compression flag
         else if ( ossStrcmp ( eleTmp.fieldName(),
                               CAT_COMPRESSED ) == 0 )
         {
            PD_CHECK( Bool == eleTmp.type(), SDB_INVALIDARG, error,
                      PDERROR, "Field[%s] type[%d] error",
                      CAT_COMPRESSED, eleTmp.type() ) ;
            clInfo._isCompressed = eleTmp.boolean() ;
            fieldMask |= CAT_MASK_COMPRESSED ;
         }
         // main-collection flag
         else if ( ossStrcmp( eleTmp.fieldName(),
                              CAT_IS_MAINCL ) == 0 )
         {
            PD_CHECK( Bool == eleTmp.type(), SDB_INVALIDARG, error,
                      PDERROR, "Field[%s] type[%d] error",
                      CAT_IS_MAINCL, eleTmp.type() ) ;
            clInfo._isMainCL = eleTmp.boolean() ;
            fieldMask |= CAT_MASK_ISMAINCL;
         }
         else
         {
            PD_RC_CHECK ( SDB_INVALIDARG, PDERROR,
                          "Unexpected field[%s] in create collection command",
                          eleTmp.toString().c_str() ) ;
         }
      }
      if ( clInfo._isMainCL )
      {
         PD_CHECK ( clInfo._isSharding,
                    SDB_NO_SHARDINGKEY, error, PDERROR,
                    "main-collection must have ShardingKey!" );
         PD_CHECK ( !clInfo._isHash,
                    SDB_INVALID_MAIN_CL_TYPE, error, PDERROR,
                    "the sharding-type of main-collection must be range!" );
      }

      PD_CHECK( clInfo._pCLName, SDB_INVALIDARG, error, PDERROR,
                "Collection name not set" ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catCatalogueManager::_assignGroup( vector < INT32 > * pGoups,
                                            INT32 & groupID )
   {
      INT32 rc = SDB_OK ;

      if ( !pGoups || pGoups->size() == 0 )
      {
         rc = _pCatCB->getAGroupRand( groupID ) ;
      }
      else
      {
         UINT32 size = pGoups->size() ;
         groupID = (*pGoups)[ ossRand() % size ] ;
      }

      return rc ;
   }

   INT32 catCatalogueManager::_checkGroupInDomain( const CHAR * groupName,
                                                   const CHAR * domainName,
                                                   BOOLEAN & existed,
                                                   INT32 *pGroupID )
   {
      INT32 rc = SDB_OK ;
      existed = FALSE ;

      BSONObj groupInfo ;

      // Check group exist
      rc = catGetGroupObj( groupName, groupInfo, _pEduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Get group[%s] info failed, rc: %d",
                   groupName, rc ) ;

      // Get group ID
      if ( pGroupID )
      {
         rtnGetIntElement( groupInfo, CAT_GROUPID_NAME, *pGroupID ) ;
      }

      // SYSTEM DOMAIN
      if ( 0 == ossStrcmp( domainName, CAT_SYS_DOMAIN_NAME ) )
      {
         existed = TRUE ;
      }
      // USER DOMAIN
      else
      {
         // Check domain exist
         BSONObj domainObj ;
         map<string, INT32> groups ;
         rc = catGetDomainObj( domainName, domainObj, _pEduCB ) ;
         PD_RC_CHECK( rc, PDERROR, "Get domain[%s] failed, rc: %d",
                      domainName, rc ) ;

         rc = catGetDomainGroups( domainObj,  groups ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get groups from domain info[%s], "
                      "rc: %d", domainObj.toString().c_str(), rc ) ;

         if ( groups.find( groupName ) != groups.end() )
         {
            existed = TRUE ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catCatalogueManager::_createCS( BSONObj & createObj,
                                         BSONObj & selector,
                                         INT32 & groupID )
   {
      INT32 rc = SDB_OK ;

      catCSInfo csInfo ;
      string    strGroupName ;

      const CHAR *csName = NULL ;
      const CHAR *domainName = NULL ;
      const CHAR *groupName = NULL ;

      BOOLEAN isSpaceExist = FALSE ;
      BSONObj spaceObj ;
      BSONObj domainObj ;
      vector< INT32 >  domainGroups ;

      // check cs obj
      rc = _checkCSObj( createObj, csInfo ) ;
      PD_RC_CHECK( rc, PDERROR, "Check create collection space obj[%s] failed,"
                   "rc: %d", createObj.toString().c_str(), rc ) ;
      csName = csInfo._pCSName ;
      domainName = csInfo._domainName ;

      // specical group name
      rc = rtnGetStringElement( selector, CAT_GROUPNAME_NAME, &groupName ) ;
      if ( SDB_FIELD_NOT_EXIST == rc )
      {
         rc = SDB_OK ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                   CAT_GROUPNAME_NAME, rc ) ;

      // name check
      rc = dmsCheckCSName( csName ) ;
      PD_RC_CHECK( rc, PDERROR, "Check collection space name[%s] failed, rc: "
                   "%d", csName, rc ) ;

      // check collection space is whether existed or not
      rc = catCheckSpaceExist( csName, isSpaceExist, spaceObj, _pEduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to check collection space existed, rc: "
                   "%d", rc ) ;
      PD_TRACE1 ( SDB_CATALOGMGR_CREATECS, PD_PACK_INT ( isSpaceExist ) ) ;
      PD_CHECK( FALSE == isSpaceExist, SDB_DMS_CS_EXIST, error, PDERROR,
                "Collection space[%s] is already existed", csName ) ;

      // check domain name
      if ( domainName )
      {
         rc = catGetDomainObj( domainName, domainObj, _pEduCB ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get domain[%s] obj, rc: %d",
                      domainName, rc ) ;
         rc = catGetDomainGroups( domainObj, domainGroups ) ;
         PD_RC_CHECK( rc, PDERROR, "Get domain[%s] groups failed, rc: %d",
                      domainObj.toString().c_str(), rc ) ;
      }

      // check group name
      if ( groupName )
      {
         BOOLEAN existed = FALSE ;
         rc = _checkGroupInDomain( groupName,
                                   domainName ? domainName : CAT_SYS_DOMAIN_NAME,
                                   existed, &groupID ) ;
         PD_RC_CHECK( rc, PDERROR, "Check group in domain failed, rc: %d", rc ) ;
         PD_CHECK( existed, SDB_CAT_GROUP_NOT_IN_DOMAIN, error, PDERROR,
                   "Group[%s] is not in domain[%s]", groupName, domainName ) ;

         // set group name
         strGroupName = groupName ;
      }

      // assign group
      if ( CAT_INVALID_GROUPID == groupID )
      {
         rc = _assignGroup( &domainGroups, groupID ) ;
         PD_RC_CHECK( rc, PDERROR, "Assign group for collection space[%s] "
                      "failed, rc: %d", csName, rc ) ;
         catGroupID2Name( groupID, strGroupName, _pEduCB ) ;
      }

      // insert new record
      {
         BSONObjBuilder newBuilder ;
         newBuilder.appendElements( createObj ) ;
         BSONObjBuilder sub( newBuilder.subarrayStart( CAT_GROUP_NAME ) ) ;
         sub.append( "0", BSON( CAT_GROUPID_NAME << groupID <<
                                CAT_GROUPNAME_NAME << strGroupName ) ) ;
         sub.done() ;
         BSONObjBuilder sub1( newBuilder.subarrayStart( CAT_COLLECTION ) ) ;
         sub1.done() ;
         BSONObj newObj = newBuilder.obj() ;

         rc = rtnInsert( CAT_COLLECTION_SPACE_COLLECTION, newObj, 1, 0,
                         _pEduCB, _pDmsCB, _pDpsCB, _majoritySize() ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to insert collection space obj[%s] "
                      " to collection[%s], rc: %d", newObj.toString().c_str(),
                      CAT_COLLECTION_SPACE_COLLECTION, rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_CREATECOLLECTION, "catCatalogueManager::_createCL" )
   INT32 catCatalogueManager::_createCL( BSONObj & createObj,
                                         BSONObj & selector,
                                         INT32 &groupID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_CREATECOLLECTION ) ;

      groupID    = CAT_INVALID_GROUPID ;

      UINT32 fieldMask = 0 ;
      catCollectionInfo clInfo ;
      const CHAR *collectionName = NULL ;
      BSONObj newCLRecordObj ;

      CHAR szSpace[ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] = {0} ;
      CHAR szCollection[ DMS_COLLECTION_NAME_SZ + 1 ] = {0} ;

      BOOLEAN isSpaceExist = FALSE ;
      BSONObj boSpaceRecord ;
      BOOLEAN isCollectionExist = FALSE ;
      BSONObj boCollectionRecord ;

      const CHAR *domainName = CAT_SYS_DOMAIN_NAME ;
      const CHAR *specGroup  = NULL ;
      string  strGroupName ;

      // check createObj
      rc = _checkAndBuildCataRecord( createObj, fieldMask, clInfo ) ;
      PD_RC_CHECK( rc, PDERROR, "Check create collection obj[%s] failed, rc: %d",
                   createObj.toString().c_str(), rc ) ;
      collectionName = clInfo._pCLName ;

      PD_TRACE1 ( SDB_CATALOGMGR_CREATECOLLECTION,
                  PD_PACK_STRING ( collectionName ) ) ;

      // split collection full name to csname and clname
      rc = catResolveCollectionName( collectionName,
                                     ossStrlen ( collectionName ),
                                     szSpace, DMS_COLLECTION_SPACE_NAME_SZ,
                                     szCollection, DMS_COLLECTION_NAME_SZ );
      PD_RC_CHECK ( rc, PDERROR, "Failed to resolve collection name: %s",
                    collectionName ) ;

      // make sure the name is valid
      rc = dmsCheckCLName( szCollection, FALSE ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to check collection name: %s, rc = %d",
                    szCollection, rc ) ;

      // get collection-space
      rc = catCheckSpaceExist( szSpace, isSpaceExist,
                               boSpaceRecord, _pEduCB ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to check if collection space exist, "
                    "rc = %d", rc );
      PD_CHECK ( isSpaceExist, SDB_DMS_CS_NOTEXIST, error, PDERROR,
                 "Create failed, the collection space(%s) is not exist",
                 szSpace ) ;

      PD_TRACE1 ( SDB_CATALOGMGR_CREATECOLLECTION,
                  PD_PACK_INT ( isSpaceExist ) ) ;

      // check if collection exist
      rc = catCheckCollectionExist( collectionName, isCollectionExist,
                                    boCollectionRecord, _pEduCB ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to check if collection exist, rc = %d", rc ) ;
      PD_CHECK ( !isCollectionExist, SDB_DMS_EXIST, error, PDERROR,
                 "Create failed, the collection(%s) exists",
                 collectionName ) ;

      PD_TRACE1 ( SDB_CATALOGMGR_CREATECOLLECTION,
                  PD_PACK_INT ( isCollectionExist ) ) ;

      // check special group
      rc = rtnGetStringElement( selector, CAT_GROUPNAME_NAME, &specGroup ) ;
      if ( SDB_FIELD_NOT_EXIST == rc )
      {
         rc = SDB_OK ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                   CAT_GROUPNAME_NAME, rc ) ;

      // has specific the group name
      if ( specGroup && !clInfo._isMainCL )
      {
         BOOLEAN exist = FALSE ;
         // get cs domain name
         rc = rtnGetStringElement( boSpaceRecord, CAT_DOMAINNAME_NAME,
                                   &domainName ) ;
         if ( SDB_FIELD_NOT_EXIST == rc )
         {
            domainName = CAT_SYS_DOMAIN_NAME ;
            rc = SDB_OK ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                      CAT_DOMAINNAME_NAME, rc ) ;
         rc = _checkGroupInDomain( specGroup, domainName, exist, &groupID ) ;
         PD_RC_CHECK( rc, PDERROR, "Check group in domain failed, rc: %d", rc ) ;
         PD_CHECK( exist, SDB_CAT_GROUP_NOT_IN_DOMAIN, error, PDERROR,
                   "Group[%s] is not in domain[%s]", specGroup, domainName ) ;

         strGroupName = specGroup ;
      }

      // assing a group id
      if ( CAT_INVALID_GROUPID == groupID && !clInfo._isMainCL )
      {
         vector< INT32 > csGroupIDs ;
         rc = catGetCSGroups( boSpaceRecord, csGroupIDs ) ;
         PD_RC_CHECK( rc, PDERROR, "Get space group id failed, rc: %d", rc ) ;
         rc = _assignGroup( &csGroupIDs, groupID ) ;
         PD_RC_CHECK( rc, PDERROR, "Assign group id failed, rc: %d", rc ) ;

         catGroupID2Name( groupID, strGroupName, _pEduCB ) ;
      }

      // build new collection record obj
      rc = _buildCatalogRecord( clInfo, groupID, strGroupName.c_str(),
                                newCLRecordObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Build new collection catalog record failed, "
                   "rc: %d", rc ) ;

      PD_TRACE1 ( SDB_CATALOGMGR_CREATECOLLECTION,
                  PD_PACK_STRING ( newCLRecordObj.toString().c_str() ) ) ;

      // insert to collection
      rc = rtnInsert( CAT_COLLECTION_INFO_COLLECTION, newCLRecordObj,
                      1, 0, _pEduCB, _pDmsCB, _pDpsCB, _majoritySize() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed insert record[%s] to collection[%s], "
                   "rc: %d", newCLRecordObj.toString().c_str(),
                   CAT_COLLECTION_INFO_COLLECTION, rc ) ;

      // update collection space info
      rc = catAddCL2CS( szSpace, szCollection, &groupID, strGroupName.c_str(),
                        _pEduCB, _pDmsCB, _pDpsCB, _majoritySize() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Update collection to space failed, rc: %d", rc ) ;
         goto rollback ;
      }

   done :
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_CREATECOLLECTION, rc ) ;
      return rc ;
   error :
      goto done ;
   rollback:
      catRemoveCL( collectionName , _pEduCB, _pDmsCB, _pDpsCB,
                   _majoritySize() ) ;
      goto done ;
   }

   // build catalogue-info record:
   // {  Name: "SpaceName.CollectionName", Version: 1, 
   //    ShardingKey: { Key1: 1, Key2: -1 },
   //    CataInfo:
   //       [ { GroupID: 1000, LowBound:{ "":MinKey,"":MaxKey }, UpBound:{"":MaxKey,"":MinKey} } ] }
   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_BUILDCATALOGRECORD, "catCatalogueManager::_buildCatalogRecord" )
   INT32 catCatalogueManager::_buildCatalogRecord( const catCollectionInfo & clInfo,
                                                   INT32 groupID,
                                                   const CHAR *groupName,
                                                   BSONObj & catRecord )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_BUILDCATALOGRECORD ) ;

      BSONObjBuilder builder ;

      builder.append( CAT_CATALOGNAME_NAME, clInfo._pCLName ) ;
      builder.append( CAT_CATALOGVERSION_NAME, CAT_VERSION_BEGIN ) ;
      builder.append( CAT_CATALOG_W_NAME, clInfo._replSize ) ;
      if ( clInfo._isCompressed )
      {
         UINT32 attr = 0 ;
         attr |= DMS_MB_ATTR_COMPRESSED ;
         builder.append( CAT_ATTRIBUTE_NAME, attr ) ;
      }
      if ( clInfo._isSharding )
      {
         builder.append( CAT_SHARDINGKEY_NAME, clInfo._shardingKey ) ;
         builder.append( CAT_ENSURE_SHDINDEX, clInfo._enSureShardIndex ) ;
         builder.append( CAT_SHARDING_TYPE, clInfo._pShardingType ) ;
         if( clInfo._isHash )
         {
            builder.append( CAT_SHARDING_PARTITION, clInfo._shardPartition ) ;
         }
      }
      if ( clInfo._isMainCL )
      {
         builder.append( CAT_IS_MAINCL, clInfo._isMainCL );
      }
      else
      {
         // cata info build
         BSONObjBuilder sub( builder.subarrayStart( CAT_CATALOGINFO_NAME ) ) ;
         BSONObjBuilder cataItemBd ( sub.subobjStart ( sub.numStr(0) ) ) ;
         cataItemBd.append ( CAT_CATALOGGROUPID_NAME, groupID ) ;
         if ( groupName )
         {
            cataItemBd.append ( CAT_GROUPNAME_NAME, groupName ) ;
         }
         if ( clInfo._isSharding )
         {
            // add LowBound and UpBound
            BSONObj lowBound, upBound ;

            if ( !clInfo._isHash )
            {
               Ordering order = Ordering::make( clInfo._shardingKey ) ;
               rc = _buildInitBound ( clInfo._shardingKey.nFields(), order ,
                                      lowBound, upBound ) ;
            }
            else
            {
               rc =_buildHashBound( lowBound, upBound, clInfo._shardPartition ) ;
            }
            PD_RC_CHECK( rc, PDERROR, "Build cata info bound failed, rc: %d", rc ) ;

            cataItemBd.append ( CAT_LOWBOUND_NAME, lowBound ) ;
            cataItemBd.append ( CAT_UPBOUND_NAME, upBound ) ;
         }
         cataItemBd.done () ;
         sub.done () ;
      }

      catRecord = builder.obj () ;

   done:
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_BUILDCATALOGRECORD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_BUILDINITBOUND, "catCatalogueManager::_buildInitBound" )
   INT32 catCatalogueManager::_buildInitBound ( UINT32 fieldNum,
                                                const Ordering & order,
                                                BSONObj & lowBound,
                                                BSONObj & upBound )
   {
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_BUILDINITBOUND ) ;
      PD_TRACE1 ( SDB_CATALOGMGR_BUILDINITBOUND,
                  PD_PACK_UINT ( fieldNum ) ) ;
      BSONObjBuilder lowBoundBD ;
      BSONObjBuilder upBoundBD ;

      UINT32 index = 0 ;
      while ( index < fieldNum )
      {
         if ( order.get( (int)index ) == 1 )
         {
            lowBoundBD.appendMinKey ( "" ) ;
            upBoundBD.appendMaxKey ( "" ) ;
         }
         else
         {
            lowBoundBD.appendMaxKey ( "" ) ;
            upBoundBD.appendMinKey ( "" ) ;
         }
         ++index ;
      }
      lowBound = lowBoundBD.obj () ;
      upBound = upBoundBD.obj () ;
      PD_TRACE_EXIT ( SDB_CATALOGMGR_BUILDINITBOUND ) ;
      return SDB_OK ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR_PROCESSMSG, "catCatalogueManager::processMsg" )
   INT32 catCatalogueManager::processMsg( void *pMsg )
   {
      INT32 rc = SDB_OK;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent *)pMsg;
      MsgHeader *pHeader = (MsgHeader *)(pEvent->data);
      PD_TRACE_ENTRY ( SDB_CATALOGMGR_PROCESSMSG ) ;
      PD_TRACE1 ( SDB_CATALOGMGR_PROCESSMSG,
                  PD_PACK_INT ( pHeader->opCode ) ) ;

      switch ( pHeader->opCode )
      {
      // command dispatch, need the second dispath in the function
      case MSG_CAT_CREATE_COLLECTION_REQ :
      case MSG_CAT_CREATE_COLLECTION_SPACE_REQ :
      case MSG_CAT_SPLIT_PREPARE_REQ :
      case MSG_CAT_SPLIT_READY_REQ :
      case MSG_CAT_SPLIT_CANCEL_REQ :
      case MSG_CAT_SPLIT_START_REQ :
      case MSG_CAT_SPLIT_CHGMETA_REQ :
      case MSG_CAT_SPLIT_CLEANUP_REQ :
      case MSG_CAT_SPLIT_FINISH_REQ :
      case MSG_CAT_QUERY_SPACEINFO_REQ :
      case MSG_CAT_DROP_COLLECTION_REQ :
      case MSG_CAT_CRT_PROCEDURES_REQ :
      case MSG_CAT_RM_PROCEDURES_REQ :
      case MSG_CAT_DROP_SPACE_REQ :
      case MSG_CAT_LINK_CL_REQ :
      case MSG_CAT_UNLINK_CL_REQ :
         {
            rc = processCommandMsg( pMsg, TRUE ) ;
            break;
         }
      case MSG_CAT_ALTER_COLLECTION_REQ:
         {
            rc = processAlterCollection( pMsg ) ;
            break ;
         }
      case MSG_CAT_QUERY_CATALOG_REQ:
         {
            rc = processQueryCatalogue( pMsg );
            break;
         }
      case MSG_CAT_QUERY_TASK_REQ:
         {
            rc = processQueryTask ( pMsg ) ;
            break ;
         }
      default:
         {
            rc = SDB_UNKNOWN_MESSAGE;
            PD_LOG(PDWARNING,
                  "received unknown message (MessageType = %d)", pHeader->opCode );
            break;
         }
      }
      PD_TRACE_EXITRC ( SDB_CATALOGMGR_PROCESSMSG, rc ) ;
      return rc;
   }

   INT32 catCatalogueManager::processCommandMsg( void * pMsg, BOOLEAN writable )
   {
      INT32 rc = SDB_OK ;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent *)pMsg ;
      MsgOpQuery *pQueryReq = (MsgOpQuery *)( pEvent->data ) ;

      MsgOpReply replyHeader ;
      CHAR       *replyData = NULL ;
      UINT32     replyDataLen = 0 ;
      INT32      returnNum    = 0 ;
      BOOLEAN    fillPeerRouteID = FALSE ;

      INT32 flag = 0 ;
      CHAR *pCMDName = NULL ;
      INT64 numToSkip = 0 ;
      INT64 numToReturn = 0 ;
      CHAR *pQuery = NULL ;
      CHAR *pFieldSelector = NULL ;
      CHAR *pOrderBy = NULL ;
      CHAR *pHint = NULL ;

      // init reply msg
      replyHeader.header.messageLength = sizeof( MsgOpReply ) ;
      replyHeader.contextID = -1 ;
      replyHeader.flags = SDB_OK ;
      replyHeader.numReturned = 0 ;
      replyHeader.startFrom = 0 ;
      _fillRspHeader( &(replyHeader.header), &(pQueryReq->header) ) ;

      // extract msg
      rc = msgExtractQuery( pEvent->data, &flag, &pCMDName, &numToSkip,
                            &numToReturn, &pQuery, &pFieldSelector,
                            &pOrderBy, &pHint ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to extract query msg, rc: %d", rc ) ;

      if ( writable && !_pCatCB->isPrimary() )
      {
         rc = SDB_CLS_NOT_PRIMARY ;
         PD_LOG ( PDWARNING, "Service deactive but received command: %s,"
                  "opCode: %d", pCMDName, pQueryReq->header.opCode ) ;
         goto error ;
      }

      // the second dispatch msg
      switch ( pQueryReq->header.opCode )
      {
         case MSG_CAT_CREATE_COLLECTION_REQ :
            rc = processCmdCreateCL( pQuery, pFieldSelector, &replyData,
                                     replyDataLen, returnNum ) ;
            break ;
         case MSG_CAT_CREATE_COLLECTION_SPACE_REQ :
            rc = processCmdCreateCS( pQuery, pFieldSelector, &replyData,
                                     replyDataLen, returnNum ) ;
            break ;
         case MSG_CAT_SPLIT_PREPARE_REQ :
         case MSG_CAT_SPLIT_READY_REQ :
         case MSG_CAT_SPLIT_CANCEL_REQ :
         case MSG_CAT_SPLIT_START_REQ :
         case MSG_CAT_SPLIT_CHGMETA_REQ :
         case MSG_CAT_SPLIT_CLEANUP_REQ :
         case MSG_CAT_SPLIT_FINISH_REQ :
            rc = processCmdSplit( pQuery, pQueryReq->header.opCode,
                                  &replyData, replyDataLen, returnNum,
                                  fillPeerRouteID ) ;
            break ;
         case MSG_CAT_QUERY_SPACEINFO_REQ :
            rc = processCmdQuerySpaceInfo( pQuery, &replyData, replyDataLen,
                                           returnNum ) ;
            break ;
         case MSG_CAT_DROP_COLLECTION_REQ :
            rc = processCmdDropCollection( pQuery ) ;
            break ;
         case MSG_CAT_DROP_SPACE_REQ :
            rc = processCmdDropCollectionSpace( pQuery ) ;
            break ;
         case MSG_CAT_CRT_PROCEDURES_REQ :
            rc = processCmdCrtProcedures( pQuery ) ;
            break ;
         case MSG_CAT_RM_PROCEDURES_REQ :
            rc = processCmdRmProcedures( pQuery ) ;
            break ;
         case MSG_CAT_LINK_CL_REQ :
            rc = processCmdLinkCollection( pQuery, &replyData,
                                          replyDataLen, returnNum );
            break;
         case MSG_CAT_UNLINK_CL_REQ :
            rc = processCmdUnlinkCollection( pQuery, &replyData,
                                          replyDataLen, returnNum );
            break;
         default :
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "Recieved unknow command: %s, opCode: %d",
                    pCMDName, pQueryReq->header.opCode ) ;
            break ;
      }

      PD_RC_CHECK( rc, PDERROR, "Process command[%s] failed, opCode: %d, "
                   "rc: %d", pCMDName, pQueryReq->header.opCode, rc ) ;

   done:
      if ( fillPeerRouteID )
      {
         replyHeader.header.routeID.value = pQueryReq->header.routeID.value ;
      }

      // send reply
      if ( 0 == replyDataLen )
      {
         rc = _pCatCB->netWork()->syncSend( pEvent->handle,
                                            (void*)&replyHeader ) ;
      }
      else
      {
         replyHeader.header.messageLength += replyDataLen ;
         replyHeader.numReturned = returnNum ;
         rc = _pCatCB->netWork()->syncSend( pEvent->handle,
                                            &(replyHeader.header),
                                            (void*)replyData, replyDataLen ) ;
      }
      if ( replyData )
      {
         SDB_OSS_FREE( replyData ) ;
      }
      return rc ;
   error:
      replyHeader.flags = rc ;
      goto done ;
   }

   void catCatalogueManager::_fillRspHeader( MsgHeader * rspMsg,
                                             const MsgHeader * reqMsg )
   {
      rspMsg->opCode = MAKE_REPLY_TYPE( reqMsg->opCode ) ;
      rspMsg->requestID = reqMsg->requestID ;
      rspMsg->routeID.value = 0 ;
      rspMsg->TID = reqMsg->TID ;
   }

   INT32 catCatalogueManager::_sendFailedRsp( NET_HANDLE handle,
                                              INT32 res,
                                              MsgHeader * reqMsg )
   {
      MsgInternalReplyHeader reply ;
      reply.res = res ;
      reply.header.messageLength = sizeof( MsgInternalReplyHeader ) ;
      _fillRspHeader( &(reply.header), reqMsg ) ;
      return _pCatCB->netWork()->syncSend( handle, (void*)&reply ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATALOGMGR__BUILDHASHBOUND, "catCatalogueManager::_buildHashBound" )
   INT32 catCatalogueManager::_buildHashBound( BSONObj& lowBound,
                                               BSONObj& upBound,
                                               INT32 paritition )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_CATALOGMGR__BUILDHASHBOUND ) ;

      lowBound = BSON("" << CAT_HASH_LOW_BOUND ) ;
      upBound = BSON("" << paritition )  ;

      PD_TRACE_EXITRC( SDB_CATALOGMGR__BUILDHASHBOUND, rc ) ;
      return rc ;
   }

   INT16 catCatalogueManager::_majoritySize()
   {
      return (INT16)( _pClsCB->getReplCB()->groupSize() / 2 + 1 ) ;
   }

   INT32 catCatalogueManager::processCmdLinkCollection( const CHAR *pQuery,
                                                        CHAR **ppReplyBody,
                                                        UINT32 &replyBodyLen,
                                                        INT32 &returnNum )
   {
      INT32 rc = SDB_OK;
      std::string strMainCLName;
      std::string strSubCLName;
      BSONObj boLowBound;
      BSONObj boUpBound;
      std::vector<UINT32>  groupList;
      try
      {
         BSONObj boQuery( pQuery );
         BSONElement beMainCLName = boQuery.getField( CAT_COLLECTION_NAME );
         PD_CHECK( beMainCLName.type() == String, SDB_INVALIDARG, error,
                   PDERROR, "failed to link the collection, get field(%s) "
                   "failed!", CAT_COLLECTION_NAME );
         strMainCLName = beMainCLName.str();
         PD_CHECK( !strMainCLName.empty(), SDB_INVALIDARG, error, PDERROR,
                   "invalid field:%s", CAT_COLLECTION_NAME );

         {
         BSONElement beSubCLName = boQuery.getField( CAT_SUBCL_NAME );
         PD_CHECK( beSubCLName.type() == String, SDB_INVALIDARG, error, PDERROR,
                   "failed to link the collection, get field(%s) failed!",
                   CAT_SUBCL_NAME );
         strSubCLName = beSubCLName.str();
         PD_CHECK( !strSubCLName.empty(), SDB_INVALIDARG, error, PDERROR,
                   "invalid field:%s", CAT_SUBCL_NAME );
         }

         {
         BSONElement beLowBound = boQuery.getField( CAT_LOWBOUND_NAME );
         PD_CHECK( beLowBound.type() == Object, SDB_INVALIDARG, error, PDERROR,
                   "invalid field:%s", CAT_LOWBOUND_NAME );
         boLowBound = beLowBound.embeddedObject();
         }

         {
         BSONElement beUpBound = boQuery.getField( CAT_UPBOUND_NAME );
         PD_CHECK( beUpBound.type() == Object, SDB_INVALIDARG, error, PDERROR,
                   "invalid field:%s", CAT_UPBOUND_NAME );
         boUpBound = beUpBound.embeddedObject();
         }

         rc = catLinkCL( strMainCLName.c_str(), strSubCLName.c_str(),
                        boLowBound, boUpBound, _pEduCB, _pDmsCB,
                        _pDpsCB, _majoritySize(), groupList );
         PD_RC_CHECK( rc, PDERROR,
                      "failed to link the sub-collection(%s) "
                      "to main-collection(%s)(rc=%d)",
                      strMainCLName.c_str(), strSubCLName.c_str(), rc );

         {
         returnNum = 1;
         BSONArrayBuilder babGroup;
         UINT32 i = 0;
         for( ; i < groupList.size(); i++ )
         {
            //TODO:add group info, unlinkcl
            BSONObj boTmp = BSON( CAT_GROUPID_NAME << groupList[i] );
            babGroup.append( boTmp );
         }
         BSONObjBuilder bobGroup;
         bobGroup.appendArray( CAT_GROUP_NAME, babGroup.arr() );
         BSONObj boGroup = bobGroup.obj();
         replyBodyLen = boGroup.objsize();
         *ppReplyBody = ( CHAR *)SDB_OSS_MALLOC( replyBodyLen );
         PD_CHECK( *ppReplyBody, SDB_OOM, error, PDERROR,
                   "malloc failed(size=%d)", replyBodyLen );
         ossMemcpy( *ppReplyBody, boGroup.objdata(), replyBodyLen );
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG;
         goto error ;
      }
   done:
      return rc;
   error:
      goto done;
   }

   INT32 catCatalogueManager::processCmdUnlinkCollection( const CHAR *pQuery,
                                                          CHAR **ppReplyBody,
                                                          UINT32 &replyBodyLen,
                                                          INT32 &returnNum )
   {
      INT32 rc = SDB_OK;
      std::string strMainCLName;
      std::string strSubCLName;
      std::vector<UINT32>  groupList;
      try
      {
         BSONObj boQuery( pQuery );
         BSONElement beMainCLName = boQuery.getField( CAT_COLLECTION_NAME );
         PD_CHECK( beMainCLName.type() == String, SDB_INVALIDARG, error,
                   PDERROR, "failed to link the collection, get field(%s) "
                   "failed!", CAT_COLLECTION_NAME );
         strMainCLName = beMainCLName.str();
         PD_CHECK( !strMainCLName.empty(), SDB_INVALIDARG, error, PDERROR,
                   "invalid field:%s", CAT_COLLECTION_NAME );

         {
         BSONElement beSubCLName = boQuery.getField( CAT_SUBCL_NAME );
         PD_CHECK( beSubCLName.type() == String, SDB_INVALIDARG, error, PDERROR,
                   "failed to link the collection, get field(%s) failed!",
                   CAT_SUBCL_NAME );
         strSubCLName = beSubCLName.str();
         PD_CHECK( !strSubCLName.empty(), SDB_INVALIDARG, error, PDERROR,
                   "invalid field:%s", CAT_SUBCL_NAME );
         }

         rc = catUnlinkCL( strMainCLName.c_str(), strSubCLName.c_str(),
                           _pEduCB, _pDmsCB, _pDpsCB, _majoritySize(),
                           groupList );
         PD_RC_CHECK( rc, PDERROR,
                      "failed to unlink the sub-collection(%s) "
                      "from main-collection(%s)(rc=%d)",
                      strMainCLName.c_str(), strSubCLName.c_str(), rc );

         {
         returnNum = 1;
         BSONArrayBuilder babGroup;
         UINT32 i = 0;
         for( ; i < groupList.size(); i++ )
         {
            //TODO:add group info, unlinkcl
            BSONObj boTmp = BSON( CAT_GROUPID_NAME << groupList[i] );
            babGroup.append( boTmp );
         }
         BSONObjBuilder bobGroup;
         bobGroup.appendArray( CAT_GROUP_NAME, babGroup.arr() );
         BSONObj boGroup = bobGroup.obj();
         replyBodyLen = boGroup.objsize();
         *ppReplyBody = ( CHAR *)SDB_OSS_MALLOC( replyBodyLen );
         PD_CHECK( *ppReplyBody, SDB_OOM, error, PDERROR,
                   "malloc failed(size=%d)", replyBodyLen );
         ossMemcpy( *ppReplyBody, boGroup.objdata(), replyBodyLen );
         }

      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG;
         goto error ;
      }
   done:
      return rc;
   error:
      goto done;
   }

}
