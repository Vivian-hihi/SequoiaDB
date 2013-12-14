#include "core.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pd.hpp"
#include "rtn.hpp"
#include "catDef.hpp"
#include "catCatalogManager.hpp"
#include "rtnPredicate.hpp"
#include "msgMessage.hpp"
#include "ixmIndexKey.hpp"

using namespace bson;
namespace engine
{
   catCatalogueManager::catCatalogueManager( pmdEDUCB *cb )
   {
      _pEduCB = cb;
   }

   INT32 catCatalogueManager::active()
   {
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

   INT32 catCatalogueManager::getGroupInfo( UINT32 groupID, bson::BSONObj &obj )
   {
      INT32 rc = SDB_OK;
      SINT64 queryContextID = -1;
      do
      {
         CHAR szBuf[ OP_MAXNAMELENGTH + 1 ] = {0};
         ossStrncpy ( szBuf, CAT_NODE_INFO_COLLECTION,
                      sizeof(CAT_NODE_INFO_COLLECTION) );
         BSONObj boSelector;
         BSONObj boOrderBy;
         BSONObj boHint;
         try
         {
            BSONObj boMatcher = BSON(CAT_GROUPID_NAME<<groupID);
         
            rc = rtnQuery ( szBuf, boSelector, boMatcher,
                        boOrderBy, boHint, 0, _pEduCB, 0,
                        -1, _pDmsCB, _pRtnCB, queryContextID );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                     "failed to query the collection(%s)",
                     CAT_NODE_INFO_COLLECTION );
               break;
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR, "received unexcepted error:%s",
                    e.what() );
            break;
         }
         CHAR *pBuffer       = NULL;
         SINT32 sBufLen      = 0;
         SINT32 sRecordNum   = 0;
         SINT64 sStartingPos = 0;
         rc = rtnGetMore ( queryContextID, 1, &pBuffer, sBufLen,
                           sRecordNum, sStartingPos, _pEduCB,
                           _pRtnCB );
         if ( rc != SDB_OK || NULL == pBuffer || sRecordNum <= 0)
         {
            PD_LOG ( PDERROR,
                  "getmore failed (collectionName:%s, sRecordNum=%d)",
                  CAT_NODE_INFO_COLLECTION, sRecordNum );
            break;
         }
         try
         {
            BSONObj boTmp( pBuffer );
            BSONObjBuilder bobResult;
            bobResult.appendElements( boTmp );
            obj = bobResult.obj();
         }
         catch ( std::exception &e )
         {
            rc = SDB_CORRUPTED_RECORD;
            PD_LOG ( PDERROR,
                    "invalid data is read from context buffer:%s",
                    e.what() );
            break;
         }
      }while ( FALSE );
      _pEduCB->contextDelete( queryContextID );
      _pRtnCB->contextDelete( queryContextID );
      return rc;
   }

   INT32 catCatalogueManager::assignGroups ( BSONObj &groupInfoArr,
                                             BSONObj &groupIdArr )
   {
      // now random access group info
      INT32 rc = SDB_OK;
      do
      {
         UINT32 groupID;
         rc = _pCatCB->getAGroupRand( groupID );
         if ( rc != SDB_OK )
         {
            PD_LOG( PDERROR, "failed to get group-info(no registered group)" );
            break;
         }
         BSONObj boGroupInfo;
         rc = getGroupInfo( groupID, boGroupInfo );
         if ( rc != SDB_OK )
         {
            PD_LOG( PDERROR, "failed to get group info(rc=%d)", rc );
            break;
         }
         try
         {
            BSONArrayBuilder babGroupIdArr;
            BSONElement beGroupID = boGroupInfo.getField( CAT_GROUPID_NAME );
            if ( beGroupID.eoo() || !beGroupID.isNumber() )
            {
               rc = SDB_INVALIDARG;
               PD_LOG( PDERROR, "failed to get the field: %s", CAT_GROUPID_NAME );
               break;
            }
            BSONObjBuilder bobGroupId;
            bobGroupId.append( beGroupID );
            babGroupIdArr.append( bobGroupId.obj() );
            groupIdArr = babGroupIdArr.arr();

            BSONArrayBuilder babGroupInfoArr;
            babGroupInfoArr.append( boGroupInfo );
            groupInfoArr = babGroupInfoArr.arr();
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG( PDERROR,
                  "catch unexpected error while parse group-info:%s",
                  e.what() );
            break;
         }
      }while ( FALSE );
      return rc;
   }

   INT32 catCatalogueManager::processDropCollectionSpace ( void *pMsg )
   {
      INT32 rc = SDB_OK;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent *)pMsg;
      MsgOpQuery *pDropReq = (MsgOpQuery *)(pEvent->data);
      MsgOpReply replyMsg;
      replyMsg.header.messageLength = sizeof( MsgOpReply );
      replyMsg.header.opCode = MSG_CAT_DROP_SPACE_RSP;
      replyMsg.header.TID = pDropReq->header.TID;
      replyMsg.header.routeID.value = 0;
      replyMsg.header.requestID = pDropReq->header.requestID;
      replyMsg.numReturned = 0;
      replyMsg.flags = SDB_OK;
      do
      {
         if ( _pCatCB->getStatus()!=SDB_CAT_PRIMARY )
         {
            rc = SDB_CLS_NOT_PRIMARY;
            PD_LOG ( PDWARNING,
                     "service deactive but received drop collection-space request" );
            break;
         }
         INT32 flag;
         CHAR *pCMDName;
         SINT64 numToSkip;
         SINT64 numToReturn;
         CHAR *pQuery = NULL;
         CHAR *pFieldSelector = NULL;
         CHAR *pOrderBy = NULL;
         CHAR *pHint = NULL;
         rc = msgExtractQuery( (CHAR *)pDropReq, &flag, &pCMDName, &numToSkip,
                              &numToReturn, &pQuery, &pFieldSelector,
                              &pOrderBy, &pHint );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "failed to parse drop collection-space request(rc=%d)",
                     rc );
            break;
         }
         std::string strSpaceName;
         try
         {
            BSONObj boQuery( pQuery );
            BSONElement beSpaceName = boQuery.getField(CAT_COLLECTION_SPACE_NAME);
            if ( beSpaceName.eoo() || beSpaceName.type()!=String )
            {
               rc = SDB_INVALIDARG;
               PD_LOG ( PDERROR,
                        "failed to process query collection-space info request,\
                        failed to get the field:%s", CAT_COLLECTION_SPACE_NAME );
               break;
            }
            strSpaceName = beSpaceName.str();
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                     "failed to process query collection-space info request,\
                     received unexpected error:%s",
                     e.what() );
            break;
         }
         rc = delCollectionSpace( strSpaceName );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "drop the collection-space(%s) failed(rc=%d)",
                     strSpaceName.c_str(), rc );
         }
         break;
      }while( FALSE );

      replyMsg.flags = rc;
      rc = _pCatCB->netWork()->syncSend( pEvent->handle, &replyMsg );
      return rc;
   }

   INT32 catCatalogueManager::delCollectionSpace( const  string & strSpaceName )
   {
      INT32 rc = SDB_OK;
      do
      {
         BSONObj boSpaceRecord;
         BOOLEAN isExist = FALSE;
         rc = checkIfSpaceExist( strSpaceName.c_str(), isExist, boSpaceRecord );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "failed to check if collection-space exist(rc=%d)",
                     rc );
            break;
         }
         if ( FALSE == isExist )
         {
            rc = SDB_DMS_CS_NOTEXIST;
            PD_LOG ( PDWARNING,
                     "delete collection-space failed,\
                     the collection-space(%s) is not exist",
                     strSpaceName.c_str() );
            break;
         }
         try
         {
            BSONElement beCollections = boSpaceRecord.getField( CAT_COLLECTION );
            if ( !beCollections.eoo() )
            {
               if ( beCollections.type() != Array )
               {
                  rc = SDB_INVALIDARG;
                  PD_LOG ( PDERROR,
                           "delete collection-space failed,\
                           the field(%s) is invalid", CAT_COLLECTION );
                  break;
               }
               BSONObjIterator i( beCollections.embeddedObject() );
               while ( i.more() )
               {
                  BSONElement beTmp = i.next();
                  BSONObj boTmp = beTmp.embeddedObject();
                  BSONElement beName = boTmp.getField( CAT_COLLECTION_NAME );
                  if ( beName.eoo() || beName.type() != String )
                  {
                     rc = SDB_INVALIDARG;
                     PD_LOG ( PDERROR,
                              "delete collection-space failed,\
                              failed to get the collection name" );
                     break;
                  }
                  std::string strName;
                  strName = strSpaceName + "." + beName.str();
                  BSONObjBuilder bobCatDeletor;
                  bobCatDeletor.append( CAT_COLLECTION_NAME, strName );
                  BSONObj boCatDeletor = bobCatDeletor.obj();
                  BSONObj boHint;
                  rc = rtnDelete( CAT_COLLECTION_INFO_COLLECTION,
                                  boCatDeletor, boHint, 0,_pEduCB,
                                  _pDmsCB, _pDpsCB,
                                  _pClsCB->getReplCB()->groupSize() );
                  if ( rc != SDB_OK )
                  {
                     PD_LOG ( PDERROR,
                              "delete collection-space failed,\
                              failed to delete the catalogue info(rc=%d)",
                              rc );
                     break;
                  }
               }
            }
            if ( SDB_OK == rc )
            {
               BSONObjBuilder bobSpaceDeletor;
               bobSpaceDeletor.append( CAT_COLLECTION_SPACE_NAME, strSpaceName );
               BSONObj boSpaceDeletor = bobSpaceDeletor.obj();
               BSONObj boHint;
               rc = rtnDelete( CAT_COLLECTION_SPACE_COLLECTION,
                               boSpaceDeletor, boHint, 0, _pEduCB,
                               _pDmsCB, _pDpsCB, _pClsCB->getReplCB()->groupSize() );
               if ( rc != SDB_OK )
               {
                  PD_LOG ( PDERROR,
                           "delete collection-space failed,\
                           failed to delete the collection space info(rc=%d)",
                           rc );
               }
            }
            break;
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                     "delete collection-space failed,\
                     occur unexpected error(%s)",
                     e.what() );
            break;
         }
      }while ( FALSE );
      return rc;
   }

   INT32 catCatalogueManager::processQuerySpaceInfo( void *pMsg )
   {
      INT32 rc = SDB_OK;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent *)pMsg;
      MsgOpQuery *pQueryReq = (MsgOpQuery *)(pEvent->data);
      MsgOpReply *pReply = NULL;
      do
      {
         if ( _pCatCB->getStatus() != SDB_CAT_PRIMARY )
         {
            rc = SDB_CLS_NOT_PRIMARY;
            PD_LOG ( PDWARNING,
                     "service deactive but received get collection-space info request" );
            break;
         }
         INT32 flag;
         CHAR *pCMDName;
         SINT64 numToSkip;
         SINT64 numToReturn;
         CHAR *pQuery = NULL;
         CHAR *pFieldSelector = NULL;
         CHAR *pOrderBy = NULL;
         CHAR *pHint = NULL;
         rc = msgExtractQuery( (CHAR *)pQueryReq, &flag, &pCMDName, &numToSkip,
                              &numToReturn, &pQuery, &pFieldSelector,
                              &pOrderBy, &pHint );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "failed to parse query collection-space info request(rc=%d)",
                     rc );
            break;
         }
         std::string strSpaceName;
         try
         {
            BSONObj boQuery( pQuery );
            BSONElement beSpaceName = boQuery.getField(CAT_COLLECTION_SPACE_NAME);
            if ( beSpaceName.eoo() || beSpaceName.type()!=String )
            {
               rc = SDB_INVALIDARG;
               PD_LOG ( PDERROR,
                        "failed to process query collection-space info request,\
                        failed to get the field:%s", CAT_COLLECTION_SPACE_NAME );
               break;
            }
            strSpaceName = beSpaceName.str();
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                     "failed to process query collection-space info request,\
                     received unexpected error:%s",
                     e.what() );
            break;
         }
         BSONObj boSpace;
         BOOLEAN isExist;
         rc = checkIfSpaceExist( strSpaceName.c_str(), isExist, boSpace );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                     "failed to process query collection-space info request,\
                     failed to check if collection space exist" );
            break;
         }
         if ( !isExist )
         {
            rc = SDB_DMS_CS_NOTEXIST;
            break;
         }
         SINT32 msgLen = sizeof(MsgOpReply) + boSpace.objsize();
         pReply = (MsgOpReply *)SDB_OSS_MALLOC( msgLen );
         if ( NULL == pReply )
         {
            rc = SDB_OOM;
            PD_LOG ( PDERROR,
                     "failed to process query collection-space info request,\
                     malloc failed(size=%d)", msgLen );
            break;
         }
         pReply->header.messageLength = msgLen;
         pReply->header.opCode = MSG_CAT_QUERY_SPACEINFO_RSP;
         pReply->header.TID = pQueryReq->header.TID;
         pReply->header.routeID.value = 0;
         pReply->header.requestID = pQueryReq->header.requestID;
         pReply->numReturned = 1;
         pReply->flags = 0;
         ossMemcpy( (CHAR *)pReply + sizeof(MsgOpReply),
                     boSpace.objdata(),
                     boSpace.objsize() );
      }while ( FALSE );

      if ( SDB_OK == rc && NULL != pReply )
      {
         rc = _pCatCB->netWork()->syncSend( pEvent->handle, pReply );
         SDB_OSS_FREE( pReply );
      }
      else
      {
         MsgOpReply replyMsg;
         replyMsg.header.messageLength = sizeof( MsgOpReply );
         replyMsg.header.opCode = MSG_CAT_QUERY_SPACEINFO_RSP;
         replyMsg.header.TID = pQueryReq->header.TID;
         replyMsg.header.routeID.value = 0;
         replyMsg.header.requestID = pQueryReq->header.requestID;
         replyMsg.numReturned = 0;
         replyMsg.flags = rc;
         rc = _pCatCB->netWork()->syncSend( pEvent->handle, &replyMsg );
      }
      return rc;
   }

   INT32 catCatalogueManager::processCreateCollectionSpace( void *pMsg )
   {
      INT32 rc = SDB_OK;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent*)pMsg;
      MsgOpQuery *pCreateReq = (MsgOpQuery*)(pEvent->data);
      MsgOpReply *pReply = NULL;
      do
      {
         if ( _pCatCB->getStatus() != SDB_CAT_PRIMARY )
         {
            rc = SDB_CLS_NOT_PRIMARY;
            PD_LOG( PDWARNING,
                  "service deactive but received create collection space request" );
            break;
         }
         INT32 flag;
         CHAR *pCommandName;
         SINT64 numToSkip;
         SINT64 numToReturn;
         CHAR *pQuery;
         CHAR *pFieldSelector;
         CHAR *pOrderBy;
         CHAR *pHint;
         rc = msgExtractQuery( (CHAR *)pCreateReq, &flag, &pCommandName,
                           &numToSkip, &numToReturn, &pQuery,
                           &pFieldSelector, &pOrderBy, &pHint );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                  "failed to parsed the msg:create-collection-space request(rc=%d)",
                  rc );
            break;
         }
         try
         {
            // get the collection-space name
            BSONObj boCreateObj ( pQuery );
            BSONElement beSpaceName = boCreateObj.getField( CAT_COLLECTION_SPACE_NAME );
            if ( beSpaceName.eoo() || beSpaceName.type()!=String )
            {
               PD_LOG ( PDERROR,
                        "failed to get the field: %s", CAT_COLLECTION_SPACE_NAME );
               rc = SDB_INVALIDARG;
               break;
            }
            // check if space exist
            BOOLEAN isSpaceExist = FALSE;
            std::string strSpaceName = beSpaceName.String();
            rc = dmsCheckCSName( strSpaceName.c_str() );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "check collection space name(%s) failed(rc=%d )",
                        strSpaceName.c_str(), rc );
               break;
            }
            BSONObj obj;
            rc = checkIfSpaceExist( strSpaceName.c_str(),
                                    isSpaceExist, obj );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "failed to check if collection space exist" );
               break;
            }
            if ( TRUE == isSpaceExist )
            {
               rc = SDB_DMS_CS_EXIST;
               PD_LOG ( PDERROR,
                        "create failed, the collection space(%s) is exist",
                        strSpaceName.c_str() );
               break;
            }
            BSONObj groupInfoArr;
            BSONObj groupIdArr;
            rc = assignGroups( groupInfoArr, groupIdArr );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "failed to get group info" );
               break;
            }
            BSONObjBuilder bobRecord;
            bobRecord.appendElements ( boCreateObj );
            bobRecord.appendArray( CAT_GROUP_NAME, groupIdArr );

            // insert to space collection
            BSONObj boRecord = bobRecord.obj();
            CHAR szBuf[OP_MAXNAMELENGTH+1]={0};
            ossStrncpy ( szBuf, CAT_COLLECTION_SPACE_COLLECTION,
                     sizeof(CAT_COLLECTION_SPACE_COLLECTION) );
            rc = rtnInsert ( szBuf, boRecord, 1, 0,
                           _pEduCB, _pDmsCB, _pDpsCB);
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "failed to insert the data to collection: %s",
                        CAT_COLLECTION_SPACE_COLLECTION );
               break;
            }
            // Build reply, add groupinfo: node list
            BSONObjBuilder bobReply;
            bobReply.appendArray( CAT_GROUP_NAME, groupInfoArr );
            BSONObj boReply = bobReply.obj();
            SINT32 msgLen = sizeof(MsgOpReply) + boReply.objsize();
            pReply = (MsgOpReply *)SDB_OSS_MALLOC( msgLen );
            if ( NULL == pReply )
            {
               rc = SDB_OOM;
               PD_LOG ( PDERROR,
                        "malloc failed (size=%d)", msgLen );
               break;
            }
            pReply->header.messageLength = msgLen;
            pReply->header.opCode = MSG_CAT_CREATE_COLLECTION_SPACE_RSP;
            pReply->header.TID = pCreateReq->header.TID;
            pReply->header.routeID.value = 0;
            pReply->header.requestID = pCreateReq->header.requestID;
            pReply->numReturned = 1;
            pReply->flags = 0;
            ossMemcpy( (CHAR *)pReply + sizeof(MsgOpReply),
                        boReply.objdata(),
                        boReply.objsize() );
         }
         catch (std::exception &e)
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                  "catch unexpected exception while parse create collectionspace request:%s",
                  e.what() );
         }
      }while ( FALSE );
      if ( SDB_OK == rc  && NULL != pReply )
      {
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, pReply );
         SDB_OSS_FREE ( pReply );
      }
      else
      {
         MsgOpReply replyMsg;
         replyMsg.header.messageLength = sizeof( MsgOpReply );
         replyMsg.header.opCode = MSG_CAT_CREATE_COLLECTION_SPACE_RSP;
         replyMsg.header.TID = pCreateReq->header.TID;
         replyMsg.header.routeID.value = 0;
         replyMsg.header.requestID = pCreateReq->header.requestID;
         replyMsg.numReturned = 0;
         replyMsg.flags = rc;
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, &replyMsg );
      }
      return rc;
   }

   INT32 catCatalogueManager::resolveCollectionName ( const CHAR *pInput, UINT32 inputLen,
                                 CHAR *pSpaceName, UINT32 spaceNameSize,
                                 CHAR *pCollectionName, UINT32 collectionNameSize )
   {
      // resolve collection-name: space.collection
      INT32 rc = SDB_OK;
      do
      {
         UINT32 curPos = 0;
         UINT32 i = 0;
         while ( curPos < inputLen && pInput[curPos] != '.'
               && i < spaceNameSize )
         {
            pSpaceName[ i++ ] = pInput[ curPos++ ];
         }
         if ( curPos >= inputLen || i >= spaceNameSize )
         {
            rc = SDB_INVALIDARG;
            break;
         }
         pSpaceName[i] = '\0';
         i = 0;
         ++curPos;
         while ( curPos < inputLen && i < collectionNameSize )
         {
            pCollectionName[ i++ ] = pInput[ curPos++ ];
         }
         pCollectionName[i] = '\0';
         if ( i >= collectionNameSize )
         {
            rc = SDB_INVALIDARG;
            break;
         }
      }while ( FALSE );
      return rc;
   }

   INT32 catCatalogueManager::checkIfCollectionExist( const char *pCollectionName,
                                                      BOOLEAN &isExist,
                                                      BSONObj &obj )
   {
      // check if collection exist
      INT32 rc = SDB_OK;
      SINT64 contextID = -1;
      do
      {
         try
         {
            BSONObj boSelector;
            BSONObj boMatcher = BSON(CAT_CATALOGNAME_NAME<<pCollectionName);
            BSONObj boOrderBy;
            BSONObj boHint;
            CHAR szCatalogCollection[ OP_MAXNAMELENGTH + 1 ] = {0};
            ossStrncpy(szCatalogCollection, CAT_COLLECTION_INFO_COLLECTION,
                     ossStrlen( CAT_COLLECTION_INFO_COLLECTION ));
            rc = rtnQuery( szCatalogCollection, boSelector, boMatcher,
                           boOrderBy, boHint, 0, _pEduCB, 0, 1, _pDmsCB,
                           _pRtnCB, contextID );
            if ( rc != SDB_OK )
            {
               if ( SDB_DMS_EOC == rc )
               {
                  rc = SDB_OK;
                  isExist = FALSE;
               }
               else
               {
                  PD_LOG ( PDERROR, "query failed (collection:%s, rc=%d)",
                           CAT_COLLECTION_INFO_COLLECTION, rc );
               }
               break;
            }
            CHAR *pBuffer = NULL;
            SINT32 bufLen = 0;
            SINT32 recordNum = 0;
            SINT64 startingPos = 0;
            rc = rtnGetMore( contextID, 1, &pBuffer, bufLen, recordNum,
                           startingPos, _pEduCB, _pRtnCB );
            if ( SDB_OK == rc )
            {
               if ( pBuffer != NULL && bufLen > 0 && recordNum > 0 )
               {
                  // the buffer will release, we should build a obj with buffer-owned
                  BSONObj boRecord( pBuffer );
                  BSONObjBuilder bobRecord;
                  bobRecord.appendElements( boRecord );
                  obj = bobRecord.obj();
                  isExist = TRUE;
                  break;
               }
            }
            else if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK;
               isExist = FALSE;
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR, "received unexpected error:%s", e.what() );
            break;
         }
      }while ( FALSE );
      _pEduCB->contextDelete( contextID );
      _pRtnCB->contextDelete( contextID );
      return rc;
   }

   INT32 catCatalogueManager::checkIfSpaceExist( const char *pSpaceName,
                                                BOOLEAN &isExist,
                                                BSONObj &obj )
   {
      INT32 rc = SDB_OK;
      SINT64 contextID = -1;

      // check if the collection-space name exist
      do
      {
         try
         {
            BSONObj boSelector;
            BSONObj boMatcher = BSON(CAT_COLLECTION_SPACE_NAME<<pSpaceName);
            BSONObj boOrderBy;
            BSONObj boHint;
            CHAR szCatalogCollection[ OP_MAXNAMELENGTH + 1 ] = {0};
            ossStrncpy(szCatalogCollection, CAT_COLLECTION_SPACE_COLLECTION,
                     ossStrlen( CAT_COLLECTION_SPACE_COLLECTION ));
            rc = rtnQuery( szCatalogCollection, boSelector, boMatcher,
                           boOrderBy, boHint, 0, _pEduCB, 0, 1, _pDmsCB,
                           _pRtnCB, contextID );
            if ( rc != SDB_OK )
            {
               if ( SDB_DMS_EOC == rc )
               {
                  rc = SDB_OK;
                  isExist = FALSE;
               }
               else
               {
                  PD_LOG ( PDERROR, "query failed (collection:%s, rc=%d)",
                           CAT_COLLECTION_INFO_COLLECTION, rc );
               }
               break;
            }
            CHAR *pBuffer = NULL;
            SINT32 bufLen = 0;
            SINT32 recordNum = 0;
            SINT64 startingPos = 0;
            rc = rtnGetMore( contextID, 1, &pBuffer, bufLen, recordNum,
                           startingPos, _pEduCB, _pRtnCB );
            if ( SDB_OK == rc )
            {
               if ( pBuffer != NULL && bufLen > 0 && recordNum > 0 )
               {
                  // the buffer will release, we should build a obj with buffer-owned
                  BSONObj boRecord( pBuffer );
                  BSONObjBuilder bobRecord;
                  bobRecord.appendElements( boRecord );
                  obj = bobRecord.obj();
                  isExist = TRUE;
                  break;
               }
            }
            else if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK;
               isExist = FALSE;
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG( PDWARNING,
                  "catch unexpected exception:%s",
                  e.what() );
            break;
         }
      }while ( FALSE );
      _pEduCB->contextDelete ( contextID );
      _pRtnCB->contextDelete ( contextID );
      return rc;
   }

   // this function is for catalog collection check
   INT32 catCatalogueManager::processQueryCatalogue ( void *pMsg )
   {
      INT32 rc = SDB_OK;
      SINT64 contextID = 0;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent*)pMsg;
      MsgCatQueryCatReq *pCatReq = (MsgCatQueryCatReq*)(pEvent->data);
      MsgOpReply *pReply = NULL;
      do
      {
         //TODO:get catalogue-info
         if ( _pCatCB->getStatus() != SDB_CAT_PRIMARY )
         {
            rc = SDB_CLS_NOT_PRIMARY;
            PD_LOG ( PDWARNING,
                  "service deactive but received query catalogue request" );
            break;
         }
         if ( pCatReq->header.messageLength <
              (INT32)sizeof(MsgCatQueryCatReq) )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                     "recived unexpected query catalogue request, "
                     "message length(%d) is invalied",
                     pCatReq->header.messageLength );
            break;
         }
         try
         {
            CHAR *pCollectionName;
            INT32 flag;
            SINT64 numToSkip;
            SINT64 numToReturn;
            CHAR *pQuery;
            CHAR *pFieldSelector;
            CHAR *pOrderBy;
            CHAR *pHint;
            rc = msgExtractQuery  ( (CHAR *)pCatReq, &flag, &pCollectionName,
                         &numToSkip, &numToReturn, &pQuery, &pFieldSelector,
                         &pOrderBy, &pHint );
            BSONObj matcher(pQuery);
            BSONObj selector(pFieldSelector);
            BSONObj orderBy(pOrderBy);
            BSONObj hit(pHint);
            CHAR szBuf[ OP_MAXNAMELENGTH + 1 ] = {0};
            ossStrncpy(szBuf, CAT_COLLECTION_INFO_COLLECTION,
                       OP_MAXNAMELENGTH );
            rc = rtnQuery( szBuf, selector, matcher, orderBy, hit, 0,
                        _pEduCB, 0, 1, _pDmsCB, _pRtnCB, contextID );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDWARNING, "failed to query the catalogue info(rc=%d)",
                        rc );
               break;
            }
            // note: one collection only has one catalogue-info record,
            //       so getmore must return only one record
            CHAR *pBuffer = NULL;
            SINT32 bufLen = 0;
            SINT32 recordNum = 0;
            SINT64 startingPos = 0;
            rc = rtnGetMore( contextID, 1, &pBuffer, bufLen, recordNum,
                           startingPos, _pEduCB, _pRtnCB );
            if ( rc != SDB_OK )
            {
               if ( SDB_DMS_EOC != rc )
                  PD_LOG ( PDWARNING, "failed to get the catalogue info(rc=%d)",
                           rc );
               else
                  // if we cannot find any collection, let's return NOTEXIST
                  rc = SDB_DMS_NOTEXIST ;
               break;
            }
            if ( NULL ==pBuffer || bufLen <= 0 || recordNum <= 0 )
            {
               rc = SDB_CAT_NO_MATCH_CATALOG;
               PD_LOG ( PDWARNING, "failed to find the match catalogue info" );
               break;
            }
            BSONObj boRecord( pBuffer );
            SINT32 msgLen = sizeof(MsgOpReply) + boRecord.objsize();
            pReply = (MsgOpReply *)SDB_OSS_MALLOC( msgLen );
            if ( NULL == pReply )
            {
               rc = SDB_OOM;
               PD_LOG ( PDERROR, "malloc failed (size=%d)", msgLen );
               break;
            }
            pReply->header.messageLength = msgLen;
            pReply->header.opCode = MSG_CAT_QUERY_CATALOG_RSP;
            pReply->header.TID = pCatReq->header.TID;
            pReply->header.requestID = pCatReq->header.requestID;
            pReply->header.routeID.value = 0;
            pReply->numReturned = 1;
            pReply->flags = 0;
            ossMemcpy((CHAR *)pReply + sizeof(MsgOpReply),
                     boRecord.objdata(),
                     boRecord.objsize() );
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                     "catch unexpected error while parse query catalogue request:%s",
                     e.what() );
            break;
         }
      }while ( FALSE );

      _pRtnCB->contextDelete ( contextID );

      if ( SDB_OK == rc && NULL != pReply )
      {
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, pReply );
         SDB_OSS_FREE ( pReply );
      }
      else
      {
         MsgOpReply replyMsg;
         replyMsg.header.messageLength = sizeof( MsgOpReply );
         replyMsg.header.opCode = MSG_CAT_QUERY_CATALOG_RSP;
         replyMsg.header.TID = pCatReq->header.TID;
         replyMsg.header.routeID.value = 0;
         replyMsg.header.requestID = pCatReq->header.requestID;
         replyMsg.numReturned = 0;
         replyMsg.flags = rc;
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, &replyMsg );
      }
      return rc;
   }

   INT32 catCatalogueManager::processDropCollection( void *pMsg )
   {
      INT32 rc = SDB_OK;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent*)pMsg;
      MsgOpQuery *pDropReq = ( MsgOpQuery *)(pEvent->data);
      MsgOpReply replyMsg;
      replyMsg.header.messageLength = sizeof( MsgOpReply );
      replyMsg.header.opCode = MSG_CAT_DROP_COLLECTION_RSP;
      replyMsg.header.TID = pDropReq->header.TID;
      replyMsg.header.routeID.value = 0;
      replyMsg.header.requestID = pDropReq->header.requestID;
      replyMsg.numReturned = 0;
      replyMsg.flags = rc;
      do
      {
         if ( _pCatCB->getStatus() != SDB_CAT_PRIMARY )
         {
            rc = SDB_CLS_NOT_PRIMARY;
            PD_LOG ( PDWARNING,
                     "service deactive but received drop collection request" );
            break;
         }
         INT32 flag;
         CHAR *pCommandName;
         SINT64 numToSkip;
         SINT64 numToReturn;
         CHAR *pQuery;
         CHAR *pFieldSelector;
         CHAR *pOrderBy;
         CHAR *pHint;
         rc = msgExtractQuery( (CHAR *)pDropReq, &flag, &pCommandName,
                           &numToSkip, &numToReturn, &pQuery,
                           &pFieldSelector, &pOrderBy, &pHint );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                  "failed to parsed the msg:drop-collection request(rc=%d)",
                  rc );
            break;
         }
         std::string strCollectionName;
         BSONObj boDeletor;
         try
         {
            boDeletor = BSONObj ( pQuery );
            BSONElement beName = boDeletor.getField( CAT_COLLECTION_NAME );
            if ( beName.eoo() || beName.type()!=String )
            {
               rc = SDB_INVALIDARG;
               PD_LOG ( PDERROR,
                        "failed to drop the collection, get collection name failed" );
               break;
            }
         }
         catch( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                  "failed to drop the collection(%s),\
                  occur unexpected error:%s",
                  strCollectionName.c_str(),
                  e.what() );
            break;
         }
         BSONObj boHint;
         rc = rtnDelete( CAT_COLLECTION_INFO_COLLECTION,
                         boDeletor, boHint, 0, _pEduCB,
                         _pDmsCB, _pDpsCB, _pClsCB->getReplCB()->groupSize() );
      }while ( FALSE );
      replyMsg.flags = rc;
      rc = _pCatCB->netWork()->syncSend( pEvent->handle, &replyMsg );
      return rc;
   }

   INT32 catCatalogueManager::processCreateCollection ( void *pMsg )
   {
      INT32 rc = SDB_OK;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent*)pMsg;
      MsgOpQuery *pCreateReq = (MsgOpQuery*)(pEvent->data);
      MsgOpReply *pReply = NULL;
      do
      {
         if ( _pCatCB->getStatus() != SDB_CAT_PRIMARY )
         {
            rc = SDB_CLS_NOT_PRIMARY;
            PD_LOG ( PDWARNING,
                  "service deactive but received create collection request" );
            break;
         }
         INT32 flag;
         CHAR *pCommandName;
         SINT64 numToSkip;
         SINT64 numToReturn;
         CHAR *pQuery;
         CHAR *pFieldSelector;
         CHAR *pOrderBy;
         CHAR *pHint;
         rc = msgExtractQuery( (CHAR *)pCreateReq, &flag, &pCommandName,
                           &numToSkip, &numToReturn, &pQuery,
                           &pFieldSelector, &pOrderBy, &pHint );
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR,
                  "failed to parsed the msg:create-collection request(rc=%d)",
                  rc );
            break;
         }
         try
         {
            BSONObj boCreateObj ( pQuery );
            BSONElement beNameTmp = boCreateObj.getField( CAT_COLLECTION_NAME );
            if ( beNameTmp.eoo() || beNameTmp.type()!=String )
            {
               PD_LOG ( PDERROR,
                        "failed to get the field: %s", CAT_COLLECTION_NAME );
               rc = SDB_INVALIDARG;
               break;
            }
            CHAR szSpace[ OP_MAXNAMELENGTH + 1 ] = {0};
            CHAR szCollection[ OP_MAXNAMELENGTH + 1 ] = {0};
            std::string strName = beNameTmp.String();
            rc = resolveCollectionName( strName.c_str(), strName.length(),
                                        szSpace, OP_MAXNAMELENGTH,
                                        szCollection, OP_MAXNAMELENGTH );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "failed to resolve collection name: %s",
                        strName.c_str() );
               rc = SDB_INVALIDARG;
               break;
            }
            // make sure the name is valid
            rc = dmsCheckCLName( szCollection );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR, "collection name is not valid: %s",
                        szCollection ) ;
               break ;
            }

            // get collection-space
            BOOLEAN isSpaceExist = FALSE;
            BSONObj boSpaceRecord;
            rc = checkIfSpaceExist( szSpace, isSpaceExist, boSpaceRecord );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "failed to check if collection space exist" );
               break;
            }
            if ( FALSE == isSpaceExist )
            {
               rc = SDB_DMS_CS_NOTEXIST;
               PD_LOG ( PDERROR,
                        "create failed, the collection space(%s) is not exist",
                        szSpace );
               break;
            }
            // check if collection exist
            BOOLEAN isCollectionExist = FALSE;
            BSONObj boCollectionRecord;
            rc = checkIfCollectionExist( strName.c_str(), isCollectionExist,
                                       boCollectionRecord );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "failed to check if collection exist" );
               break;
            }
            if ( TRUE == isCollectionExist )
            {
               rc = SDB_DMS_EXIST;
               PD_LOG ( PDERROR,
                        "create failed, the collection(%s) is exist",
                        strName.c_str() );
               break;
            }

            // update collection space record
            // basically we need to find the collection space, and then add the
            // collection into the Collection array in the record
            BSONObjBuilder bobMatcher;
            bobMatcher.append( CAT_COLLECTION_SPACE_NAME,  szSpace );
            BSONObj boMatcher = bobMatcher.obj();

            // build update request
            BSONObjBuilder bobAddObj;
            bobAddObj.append( CAT_COLLECTION_NAME, szCollection );
            BSONArrayBuilder babArray;
            babArray.append( bobAddObj.obj() );
            BSONObjBuilder bobUpdateComment;
            bobUpdateComment.appendArray( CAT_COLLECTION, babArray.arr() );
            BSONObj boUpdateComment = bobUpdateComment.obj();
            BSONObjBuilder bobUpdater;
            bobUpdater.append( "$addtoset", boUpdateComment );
            BSONObj boUpdater = bobUpdater.obj();

            BSONObj hint;
            CHAR szSpaceCollection[ OP_MAXNAMELENGTH + 1 ] = {0};
            ossStrncpy(szSpaceCollection, CAT_COLLECTION_SPACE_COLLECTION,
                     ossStrlen( CAT_COLLECTION_SPACE_COLLECTION ));
            // perform update for collection space record
            rc = rtnUpdate ( szSpaceCollection, boMatcher, boUpdater, hint,
                     0, _pEduCB, _pDmsCB, _pDpsCB, _pClsCB->getReplCB()->groupSize() );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR,
                        "failed to update the collection-space info(name:%s)",
                        strName.c_str() );
               break;
            }

            // assign a groupID
            BSONArrayBuilder babGroupIdArr ;
            BSONElement beGroupIDArr = boSpaceRecord.getField( CAT_GROUP_NAME );
            if ( beGroupIDArr.eoo() || beGroupIDArr.type() != Array )
            {
               rc = SDB_INVALIDARG;
               PD_LOG ( PDERROR, "failed to get the field: %s", CAT_GROUP_NAME );
               break;
            }
            BSONElement beGroupID;
            BSONObjIterator i( beGroupIDArr.embeddedObject() );
            while ( i.more() )
            {
               // get the first group in space
               // TODO: get a choose-algorithm
               BSONElement beTmp = i.next();
               BSONObj boTmp = beTmp.embeddedObject();
               babGroupIdArr.append( boTmp );
               beGroupID = boTmp.getField( CAT_GROUPID_NAME );
               break;
            }
            if ( beGroupID.eoo() || !beGroupID.isNumber() )
            {
               rc = SDB_INVALIDARG;
               PD_LOG ( PDERROR,
                     "process catalogue request failed, the field (%s) is invalid ",
                     CAT_GROUPID_NAME );
               break;
            }
            BSONElement beShardingKey
                           = boCreateObj.getField( CAT_SHARDINGKEY_NAME );
            CHAR szCatalogCollection[ OP_MAXNAMELENGTH + 1 ] = {0};
            BSONObj boCatalogRecord;
            rc = buildCatalogRecord ( strName.c_str(), beGroupID,
                                      beShardingKey, boCatalogRecord );
            if ( rc != SDB_OK )
            {
               PD_LOG ( PDERROR, "build catalogue record failed(rc=%d)", rc );
               break;
            }
            ossStrncpy(szCatalogCollection, CAT_COLLECTION_INFO_COLLECTION,
                     ossStrlen( CAT_COLLECTION_INFO_COLLECTION ));
            rc = rtnInsert( szCatalogCollection, boCatalogRecord, 1, 0,
                           _pEduCB, _pDmsCB, _pDpsCB );
            if ( rc != SDB_OK )
            {
               PD_LOG( PDERROR, "failed to add catalogue-info(rc=%d)", rc );
               break;
            }
            BSONObjBuilder bobReply;
            bobReply.append( CAT_CATALOGVERSION_NAME, CAT_VERSION_BEGIN );
            bobReply.appendArray( CAT_GROUP_NAME, babGroupIdArr.arr() );
            BSONObj boReply = bobReply.obj();
            SINT32 msgLen = sizeof(MsgOpReply) + boReply.objsize();
            pReply = (MsgOpReply *)SDB_OSS_MALLOC( msgLen );
            if ( NULL == pReply )
            {
               rc = SDB_OOM;
               PD_LOG ( PDERROR,
                        "malloc failed (size=%d)", msgLen );
               break;
            }
            pReply->header.messageLength = msgLen;
            pReply->header.opCode = MSG_CAT_CREATE_COLLECTION_RSP;
            pReply->header.TID = pCreateReq->header.TID;
            pReply->header.routeID.value = 0;
            pReply->header.requestID = pCreateReq->header.requestID;
            pReply->numReturned = 1;
            pReply->flags = 0;
            ossMemcpy( (CHAR *)pReply + sizeof(MsgOpReply),
                        boReply.objdata(),
                        boReply.objsize() );
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG( PDERROR,
                  "catch unexpected exception while parse create collectionspace request:%s",
                  e.what() );
            break;
         }
      }while ( FALSE );
      if ( SDB_OK == rc  && NULL != pReply )
      {
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, pReply );
         SDB_OSS_FREE ( pReply );
      }
      else
      {
         MsgOpReply replyMsg;
         replyMsg.header.messageLength = sizeof( MsgOpReply );
         replyMsg.header.opCode = MSG_CAT_CREATE_COLLECTION_RSP;
         replyMsg.header.TID = pCreateReq->header.TID;
         replyMsg.header.routeID.value = 0;
         replyMsg.header.requestID = pCreateReq->header.requestID;
         replyMsg.numReturned = 0;
         replyMsg.flags = rc;
         rc = _pCatCB->netWork()->syncSend ( pEvent->handle, &replyMsg );
      }
      return rc;
   }

   // building initialized low boundary for a given group id, with numFields
   // fields. output to outputObj
   INT32 catCatalogueManager::_buildInitBound ( INT32 groupID,
                                                INT32 numFields,
                                                BSONObj &outputObj )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder bo ;
      BSONObjBuilder boLow ;
      BSONObjBuilder boUp ;
      try
      {
         bo.append ( CAT_GROUPID_NAME, groupID ) ;
         // loop through number of fields
         for ( INT32 i = 0; i < numFields; ++i )
         {
            // for each fields we create MinKey
            boLow.appendMinKey ( "" ) ;
            boUp.appendMaxKey ( "" ) ;
         }
         // only add LowBound and UpBound in sharded environment
         if ( numFields )
         {
            bo.append ( CAT_LOWBOUND_NAME, boLow.obj() ) ;
            bo.append ( CAT_UPBOUND_NAME, boUp.obj() ) ;
         }
         outputObj = bo.obj () ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Failed to build init low bound: %s",
                  e.what() ) ;
         rc = SDB_SYS ;
      }
      return rc ;
   }

   // build catalog-info record:
   // { name: "SpaceName.CollectionName", Version: 1, ShardingKey: { Key1:1,
   // Key2:-1}, CataInfo: [ {GroupID:1000, LowBound: {"":MinKey, "":MinKey}},
   // {GroupID:2000, LowBound:{"":5,"":10}}, {GroupID:1000,
   // LowBound:{"":5,"":100}}]}
   INT32 catCatalogueManager::buildCatalogRecord( const CHAR *pCollectionName,
                                                  BSONElement &beGroupID,
                                                  BSONElement &beShardingKey,
                                                  BSONObj &catRecord )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder obCatalogRecord ;
      INT32 numFields = 0 ;
      BSONObj groupObj ;
      try
      {
         // create collection name
         // append "Name": "foo.test"
         obCatalogRecord.append ( CAT_CATALOGNAME_NAME, pCollectionName ) ;

         // create version
         // append "Version": 1
         obCatalogRecord.append ( CAT_CATALOGVERSION_NAME, CAT_VERSION_BEGIN ) ;

         if ( !beShardingKey.eoo() && beShardingKey.type() == Object )
         {
            // create partition info
            BSONArrayBuilder abCatalogInfo ;
            // append ShardingKey
            // ShardingKey: { Key1:1, Key2:-1 }
            obCatalogRecord.append ( beShardingKey ) ;
            BSONObj oShardingKey = beShardingKey.embeddedObject () ;

            // make sure the key definition is correct
            if ( FALSE == _ixmIndexKeyGen::validateKeyDef ( oShardingKey ) )
            {
               PD_LOG ( PDERROR, "Sharding key definition is not valid" ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            // get the number of fields in sharding key, so that we can create
            // low/up bounds
            numFields = oShardingKey.nFields () ;
         }
         // Note we are going to have CataInfo even in single-partitioned
         // collection with GroupID information. However we will not have
         // LowBound/UpBound info
         // once we get the number of fields, let's create low bound object
         rc = _buildInitBound ( beGroupID.numberInt(),
                                numFields,
                                groupObj ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to init low bound, rc = %d", rc ) ;
            goto error ;
         }
         // now we got groupObj = { GroupID: 1000, LowBound: { "": MinKey,
         // "": MinKey } }
         // append into catalog info array
         // CataInfo: [ { GroupID: 1000, LowBound: { "": MinKey, "": MinKey }
         // } ]
         abCatalogInfo.append ( groupObj ) ;
         // put CataInfo array into record
         obCatalogRecord.appendArray ( CAT_CATALOGINFO_NAME,
                                       abCatalogInfo.arr() ) ;
         catRecord = obCatalogRecord.obj () ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception during creating collection record: %s",
                  e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   // build catalogue-info record:
   // {  name: "SpaceName.CollectionName", Version: 1, 
   //    ShardingKey: { Key1: 1, Key2: -1 },
   //    cataInfo:
   //       [ { GroupID: 1000, Range: [ { MinKey, Key1Value }, { MaxKey, Key2Value } ] },
   //         { GroupID: 1001, Range: [ { Key1Value, MaxKey }, { Key2Value, MinKey } ] } ] }
   /*INT32 catCatalogueManager::buildCatalogRecord( const CHAR *pCollectonName,
                              BSONElement &beGroupID,
                              BSONElement &beShardingKey,
                              BSONObj &catRecord )
   {
      INT32 rc = SDB_OK;
      do
      {
         try
         {
            // create collection name
            BSONObjBuilder bobCatalogRecord;
            bobCatalogRecord.append( CAT_CATALOGNAME_NAME, pCollectonName );
            // create version field
            bobCatalogRecord.append( CAT_CATALOGVERSION_NAME, CAT_VERSION_BEGIN );

            // create partition info
            BSONObjBuilder bobCatalogPart;
            // let's create a single group with groupid, in the cataInfo
            bobCatalogPart.append( CAT_CATALOGGROUPID_NAME, beGroupID.numberInt() );
            // make sure sharding key is not empty and it's object
            if ( !beShardingKey.eoo() && beShardingKey.type() == Object )
            {
               bobCatalogRecord.append ( beShardingKey ) ;
               BSONObj boShardingKey = beShardingKey.embeddedObject();
               // validate key definition
               if ( FALSE == _ixmIndexKeyGen::validateKeyDef ( boShardingKey ) )
               {
                  PD_LOG ( PDERROR, "Sharding key definition is not valid" ) ;
                  rc = SDB_INVALIDARG ;
                  break ;
               }
               // loop through each element in the key
               BSONObjIterator i( boShardingKey );
               BSONArrayBuilder babRange;
               while ( TRUE )
               {
                  // each element represent a field
                  BSONElement beTmp = i.next();
                  if ( beTmp.eoo() )
                  {
                     break;
                  }
                  // append minkey for the field
                  //babRange.appendMinKey ( "" ) ;
                  // create start/stop key pair
                  rtnStartStopKey rangeKey;
                  // reset to min/max
                  rangeKey.reset();
                  rangeKey._startKey._inclusive = TRUE ;
                  // append range for the current field
                  babRange.append ( rangeKey.toBson());
               }
               // insert the single range into range array
               bobCatalogPart.appendArray ( CAT_CATALOGRANGE_NAME,
                                            babRange.arr() );
            }
            BSONArrayBuilder babCatalogInfo;
            babCatalogInfo.append(bobCatalogPart.obj());
            bobCatalogRecord.appendArray( CAT_CATALOGINFO_NAME, babCatalogInfo.arr() );
            catRecord = bobCatalogRecord.obj();
         }
         catch ( std::exception &e )
         {
            rc = SDB_INVALIDARG;
            PD_LOG ( PDERROR,
                  "catch unexpected error while build catalog-record:%s",
                  e.what() );
            break;
         }
      }while ( FALSE );
      return rc;
   }*/

   INT32 catCatalogueManager::processMsg( void *pMsg )
   {
      INT32 rc = SDB_OK;
      EvntCatalogInternalEvent *pEvent = (EvntCatalogInternalEvent *)pMsg;
      MsgHeader *pHeader = (MsgHeader *)(pEvent->data);
      switch ( pHeader->opCode )
      {
      case MSG_CAT_CREATE_COLLECTION_SPACE_REQ:
         {
            rc = processCreateCollectionSpace( pMsg );
            break;
         }
      case MSG_CAT_CREATE_COLLECTION_REQ:
         {
            rc = processCreateCollection( pMsg );
            break;
         }
      case MSG_CAT_QUERY_CATALOG_REQ:
         {
            rc = processQueryCatalogue( pMsg );
            break;
         }
      case MSG_CAT_DROP_COLLECTION_REQ:
         {
            rc = processDropCollection ( pMsg );
            break;
         }
      case MSG_CAT_DROP_SPACE_REQ:
         {
            rc = processDropCollectionSpace( pMsg );
            break;
         }
      case MSG_CAT_QUERY_SPACEINFO_REQ:
         {
            rc = processQuerySpaceInfo( pMsg );
            break;
         }
      default:
         {
            rc = SDB_UNKNOWN_MESSAGE;
            PD_LOG(PDWARNING,
                  "received unknown message (MessageType = %d)", pHeader->opCode );
            break;
         }
      }
      return rc;
   }

}
