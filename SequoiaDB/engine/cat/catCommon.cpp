/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = catCommon.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/07/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmd.hpp"
#include "pmdCB.hpp"
#include "catCommon.hpp"
#include "pdTrace.hpp"
#include "catTrace.hpp"
#include "rtn.hpp"
#include "fmpDef.hpp"
#include "clsCatalogAgent.hpp"

using namespace bson ;

namespace engine
{

   PD_TRACE_DECLARE_FUNCTION ( SDB_CATGROUPNAMEVALIDATE, "catGroupNameValidate" ) ;
   INT32 catGroupNameValidate ( const CHAR *pName, BOOLEAN isSys )
   {
      INT32 rc = SDB_INVALIDARG ;
      PD_TRACE_ENTRY ( SDB_CATGROUPNAMEVALIDATE ) ;

      if ( !pName || pName[0] == '\0' )
      {
         PD_LOG ( PDINFO, "group name can't be empty" ) ;
         goto error ;
      }
      PD_TRACE2 ( SDB_CATGROUPNAMEVALIDATE, PD_PACK_STRING ( pName ),
                  PD_PACK_UINT ( isSys ) ) ;
      // name is within valid length
      if ( ossStrlen ( pName ) > OSS_MAX_GROUPNAME_SIZE )
      {
         PD_LOG ( PDINFO, "group name %s is too long",
                  pName ) ;
         goto error ;
      }
      // group name should not start from SYS nor $ if it's not SYSTEM created
      if ( !isSys &&
           ( ossStrncmp ( pName, "SYS", ossStrlen ( "SYS" ) ) == 0 ||
             ossStrncmp ( pName, "$", ossStrlen ( "$" ) ) == 0 ) )
      {
         PD_LOG ( PDINFO, "group name should not start with SYS nor $: %s",
                  pName ) ;
         goto error ;
      }
      // there shouldn't be any dot in the name
      if ( ossStrchr ( pName, '.' ) != NULL )
      {
         PD_LOG ( PDINFO, "group name should not contain dot(.): %s",
                  pName ) ;
         goto error ;
      }

      rc = SDB_OK ;

   done :
      PD_TRACE_EXITRC ( SDB_CATGROUPNAMEVALIDATE, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 catDomainNameValidate( const CHAR * pName )
   {
      return catGroupNameValidate( pName, FALSE ) ;
   }

   INT32 catResolveCollectionName ( const CHAR *pInput, UINT32 inputLen,
                                    CHAR *pSpaceName, UINT32 spaceNameSize,
                                    CHAR *pCollectionName,
                                    UINT32 collectionNameSize )
   {
      INT32 rc = SDB_OK ;
      UINT32 curPos = 0 ;
      UINT32 i = 0 ;

      while ( pInput[curPos] != '.' )
      {
         if ( curPos >= inputLen || i >= spaceNameSize )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         pSpaceName[ i++ ] = pInput[ curPos++ ] ;
      }
      pSpaceName[i] = '\0' ;

      i = 0 ;
      ++curPos ;
      while ( curPos < inputLen )
      {
         if ( i >= collectionNameSize )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         pCollectionName[ i++ ] = pInput[ curPos++ ] ;
      }
      pCollectionName[i] = '\0' ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catQueryAndGetMore ( MsgOpReply **ppReply,
                              const CHAR *collectionName,
                              BSONObj &selector,
                              BSONObj &matcher,
                              BSONObj &orderBy,
                              BSONObj &hint,
                              SINT32 flags,
                              pmdEDUCB *cb,
                              SINT64 numToSkip,
                              SINT64 numToReturn )
   {
      INT32 rc               = SDB_OK ;
      SINT64 contextID       = -1 ;
      pmdKRCB *pKRCB          = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB        = pKRCB->getDMSCB() ;
      SDB_RTNCB *rtnCB        = pKRCB->getRTNCB() ;
      SINT32 replySize       = sizeof(MsgOpReply) ;
      SINT32 replyBufferSize = 0 ;

      // first initialize reply buffer, note the caller is responsible to free
      // the memory
      rc = rtnReallocBuffer ( (CHAR**)ppReply, &replyBufferSize, replySize,
                              SDB_PAGE_SIZE ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to realloc buffer, rc = %d", rc ) ;
      ossMemset ( *ppReply, 0, replySize ) ;

      // perform query
      rc = rtnQuery ( collectionName, selector, matcher, orderBy, hint, flags,
                      cb, numToSkip, numToReturn, dmsCB, rtnCB, contextID ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to perform query, rc = %d", rc ) ;

      // extract all results
      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         SINT64 startingPos = 0 ;
         rc = rtnGetMore ( contextID, -1, buffObj, startingPos, cb, rtnCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               contextID = -1 ;
               rc = SDB_OK ;
               break ;
            }
            PD_LOG( PDERROR, "Failed to retreive record, rc = %d", rc ) ;
            goto error ;
         }

         // reply is always 4 bytes aligned like in context
         replySize = ossRoundUpToMultipleX ( replySize, 4 ) ;
         rc = rtnReallocBuffer ( (CHAR**)ppReply, &replyBufferSize,
                                 replySize + buffObj.size(), SDB_PAGE_SIZE ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to realloc buffer, rc = %d", rc ) ;

         // copy the new records from context buffer to reply buffer
         ossMemcpy ( &((CHAR*)(*ppReply))[replySize], buffObj.data(),
                     buffObj.size() ) ;
         (*ppReply)->numReturned += buffObj.recordNum() ;
         // update the current offset of reply
         replySize               += buffObj.size() ;
      }
      // finally update reply header
      (*ppReply)->header.messageLength = replySize ;
      (*ppReply)->flags                = SDB_OK ;
      (*ppReply)->contextID            = -1 ;

   done :
      if ( -1 != contextID )
      {
         rtnCB->contextDelete ( contextID, cb ) ;
      }
      return rc ;
   error :
      if ( *ppReply )
      {
         SDB_OSS_FREE( (CHAR*)(*ppReply) ) ;
         *ppReply = NULL ;
      }
      goto done ;
   }

   INT32 catGetOneObj( const CHAR * collectionName,
                       BSONObj & selector,
                       BSONObj & matcher,
                       BSONObj & hint,
                       pmdEDUCB * cb,
                       BSONObj & obj )
   {
      INT32 rc                = SDB_OK ;
      SINT64 contextID        = -1 ;
      pmdKRCB *pKRCB          = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB        = pKRCB->getDMSCB() ;
      SDB_RTNCB *rtnCB        = pKRCB->getRTNCB() ;
      BSONObj dummyObj ;

      rtnContextBuf buffObj ;
      INT64 startingPos       = 0 ;

      // query
      rc = rtnQuery( collectionName, selector, matcher, dummyObj, hint,
                     0, cb, 0, 1, dmsCB, rtnCB, contextID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to query from %s, rc: %d",
                   collectionName, rc ) ;

      // get more
      rc = rtnGetMore( contextID, 1, buffObj, startingPos, cb, rtnCB ) ;
      if ( rc )
      {
         if ( SDB_DMS_EOC == rc )
         {
            contextID = -1 ;
         }
         goto error ;
      }

      // copy obj
      try
      {
         BSONObj resultObj( buffObj.data() ) ;
         obj = resultObj.copy() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         goto error ;
      }

   done:
      if ( -1 != contextID )
      {
         buffObj.release() ;
         rtnCB->contextDelete( contextID, cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 catGetGroupObj( const CHAR * groupName, BSONObj & obj, pmdEDUCB *cb  )
   {
      INT32 rc           = SDB_OK;

      BSONObj dummyObj ;
      BSONObj boMatcher = BSON( CAT_GROUPNAME_NAME << groupName );

      rc = catGetOneObj( CAT_NODE_INFO_COLLECTION, dummyObj, boMatcher,
                         dummyObj, cb, obj ) ;
      if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_CLS_GRP_NOT_EXIST ;
      }
      else if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 boMatcher.toString().c_str(), CAT_NODE_INFO_COLLECTION, rc ) ;
         goto error ;
      }

   done :
      return rc;
   error :
      goto done ;
   }

   INT32 catGetGroupObj( UINT32 groupID, BSONObj &obj, pmdEDUCB *cb )
   {
      INT32 rc           = SDB_OK;
      BSONObj dummyObj ;
      BSONObj boMatcher = BSON( CAT_GROUPID_NAME << groupID );

      rc = catGetOneObj( CAT_NODE_INFO_COLLECTION, dummyObj, boMatcher,
                         dummyObj, cb, obj ) ;
      if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_CLS_GRP_NOT_EXIST ;
      }
      else if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 boMatcher.toString().c_str(), CAT_NODE_INFO_COLLECTION, rc ) ;
         goto error ;
      }

   done :
      return rc;
   error :
      goto done ;
   }

   INT32 catGetGroupObj( UINT16 nodeID, BSONObj &obj, pmdEDUCB *cb )
   {
      INT32 rc           = SDB_OK;
      BSONObj dummyObj ;
      BSONObj boMatcher = BSON( FIELD_NAME_GROUP"."FIELD_NAME_NODEID << nodeID );

      rc = catGetOneObj( CAT_NODE_INFO_COLLECTION, dummyObj, boMatcher,
                         dummyObj, cb, obj ) ;
      if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_CLS_NODE_NOT_EXIST ;
      }
      else if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 boMatcher.toString().c_str(), CAT_NODE_INFO_COLLECTION, rc ) ;
         goto error ;
      }

   done :
      return rc;
   error :
      goto done ;
   }

   INT32 catGroupCheck( const CHAR * groupName, BOOLEAN & exist, pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj boGroupInfo ;
      rc = catGetGroupObj( groupName, boGroupInfo, cb ) ;
      if ( SDB_OK == rc )
      {
         exist = TRUE ;
      }
      else if ( SDB_CLS_GRP_NOT_EXIST == rc )
      {
         rc = SDB_OK;
         exist = FALSE;
      }
      return rc ;
   }

   INT32 catServiceCheck( const CHAR * hostName, const CHAR * serviceName,
                          BOOLEAN & exist, pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj groupInfo ;
      BSONObj dummyObj ;
      BSONObj match = BSON( FIELD_NAME_GROUP << BSON( "$elemMatch" <<
                            BSON( FIELD_NAME_SERVICE"."FIELD_NAME_NAME <<
                                  serviceName << FIELD_NAME_HOST <<
                                  hostName )) ) ;

      rc = catGetOneObj( CAT_NODE_INFO_COLLECTION, dummyObj, match,
                         dummyObj, cb, groupInfo ) ;
      if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_OK ;
         exist = FALSE ;
      }
      else if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 match.toString().c_str(), CAT_NODE_INFO_COLLECTION, rc ) ;
      }
      else
      {
         exist = TRUE ;
      }

      return rc ;
   }

   INT32 catGroupID2Name( INT32 groupID, string & groupName, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj groupObj ;
      const CHAR *name = NULL ;

      rc = catGetGroupObj( (UINT32)groupID, groupObj, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get group obj by id[%d], rc: %d",
                   groupID, rc ) ;

      rc = rtnGetStringElement( groupObj, CAT_GROUPNAME_NAME, &name ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                   CAT_GROUPNAME_NAME, rc ) ;

      groupName = name ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catGroupName2ID( const CHAR * groupName, INT32 & groupID,
                          pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj groupObj ;

      rc = catGetGroupObj( groupName, groupObj, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get group obj by name[%s], rc: %d",
                   groupName, rc ) ;

      rc = rtnGetIntElement( groupObj, CAT_GROUPID_NAME, groupID ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                   CAT_GROUPID_NAME, rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catGetDomainObj( const CHAR * domainName, BSONObj & obj, pmdEDUCB * cb )
   {
      INT32 rc           = SDB_OK;
      BSONObj dummyObj ;
      BSONObj boMatcher = BSON( CAT_DOMAINNAME_NAME << domainName );

      rc = catGetOneObj( CAT_DOMAIN_COLLECTION, dummyObj, boMatcher,
                         dummyObj, cb, obj ) ;
      if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_CAT_DOMAIN_NOT_EXIST ;
      }
      else if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 boMatcher.toString().c_str(), CAT_DOMAIN_COLLECTION, rc ) ;
         goto error ;
      }

   done :
      return rc;
   error :
      goto done ;
   }

   INT32 catDomainCheck( const CHAR * domainName, BOOLEAN & exist,
                         pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;

      rc = catGetDomainObj( domainName, obj, cb ) ;
      if ( SDB_OK == rc )
      {
         exist = TRUE ;
      }
      else if ( SDB_CAT_DOMAIN_NOT_EXIST == rc )
      {
         rc = SDB_OK ;
         exist = FALSE ;
      }
      return rc ;
   }

   INT32 catGetDomainGroups( const BSONObj & domain,
                             map < string, INT32 > & groups )
   {
      INT32 rc = SDB_OK ;
      const CHAR *groupName = NULL ;
      INT32 groupID = CAT_INVALID_GROUPID ;

      BSONElement beGroups = domain.getField( CAT_GROUP_NAME ) ;
      if ( beGroups.eoo() )
      {
         goto done ;
      }

      if ( Array != beGroups.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Domin group type error, Domain info: %s",
                 domain.toString().c_str() ) ;
         goto error ;
      }

      {
         BSONObjIterator i( beGroups.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONElement ele = i.next() ;
            PD_CHECK( Object == ele.type(), SDB_INVALIDARG, error, PDERROR,
                      "Domain group ele type is not object, Domain info: %s",
                      domain.toString().c_str() ) ;

            BSONObj boGroup = ele.embeddedObject() ;
            rc = rtnGetStringElement( boGroup, CAT_GROUPNAME_NAME,
                                      &groupName ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                         CAT_GROUPNAME_NAME, rc ) ;
            rc = rtnGetIntElement( boGroup, CAT_GROUPID_NAME, groupID ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                         CAT_GROUPID_NAME, rc ) ;

            groups[ groupName ] = groupID ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catGetDomainGroups( const BSONObj &domain,
                             vector< INT32 > &groupIDs )
   {
      INT32 rc = SDB_OK ;
      INT32 groupID = CAT_INVALID_GROUPID ;

      BSONElement beGroups = domain.getField( CAT_GROUP_NAME ) ;
      if ( beGroups.eoo() )
      {
         goto done ;
      }

      if ( Array != beGroups.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Domin group type error, Domain info: %s",
                 domain.toString().c_str() ) ;
         goto error ;
      }

      {
         BSONObjIterator i( beGroups.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONElement ele = i.next() ;
            PD_CHECK( Object == ele.type(), SDB_INVALIDARG, error, PDERROR,
                      "Domain group ele type is not object, Domain info: %s",
                      domain.toString().c_str() ) ;

            BSONObj boGroup = ele.embeddedObject() ;
            rc = rtnGetIntElement( boGroup, CAT_GROUPID_NAME, groupID ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                         CAT_GROUPID_NAME, rc ) ;

            groupIDs.push_back( groupID ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catAddCL2CS( const CHAR * csName, const CHAR * clName,
                      INT32 *pGroupID, const CHAR *groupName,
                      pmdEDUCB * cb, SDB_DMSCB * dmsCB, SDB_DPSCB * dpsCB,
                      INT16 w )
   {
      INT32 rc = SDB_OK ;

      BSONObj boMatcher = BSON( CAT_COLLECTION_SPACE_NAME << csName ) ;

      BSONObjBuilder updateBuild ;
      BSONObjBuilder sub( updateBuild.subobjStart("$addtoset") ) ;

      if ( clName )
      {
         BSONObj newCLObj = BSON( CAT_COLLECTION_NAME << clName ) ;
         BSONObjBuilder sub1( sub.subarrayStart( CAT_COLLECTION ) ) ;
         sub1.append( "0", newCLObj ) ;
         sub1.done() ;
      }

      if ( pGroupID && *pGroupID != CAT_INVALID_GROUPID )
      {
         BSONObjBuilder groupBuilder ;
         groupBuilder.append( CAT_GROUPID_NAME, *pGroupID ) ;
         if ( groupName && *groupName != '\0' )
         {
            groupBuilder.append( CAT_GROUPNAME_NAME, groupName ) ;
         }
         BSONObj newGroupObj = groupBuilder.obj() ;
         BSONObjBuilder sub2( sub.subarrayStart( CAT_GROUP_NAME ) ) ;
         sub2.append( "0", newGroupObj ) ;
         sub2.done() ;
      }

      sub.done() ;
      BSONObj updator = updateBuild.obj() ;
      BSONObj hint ;

      rc = rtnUpdate( CAT_COLLECTION_SPACE_COLLECTION, boMatcher, updator,
                      hint, 0, cb, dmsCB, dpsCB, w ) ;

      PD_RC_CHECK( rc, PDERROR, "Failed to update collection: %s, match: %s, "
                   "updator: %s, rc: %d", CAT_COLLECTION_SPACE_COLLECTION,
                   boMatcher.toString().c_str(),
                   updator.toString().c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catDelCLFromCS( const CHAR * csName, const CHAR * clName,
                         pmdEDUCB * cb, SDB_DMSCB * dmsCB, SDB_DPSCB * dpsCB,
                         INT16 w )
   {
      INT32 rc = SDB_OK ;

      BSONObj modifier = BSON( "$pull" << BSON( CAT_COLLECTION <<
                               BSON( CAT_COLLECTION_NAME << clName ) ) ) ;
      BSONObj matcher = BSON( CAT_COLLECTION_SPACE_NAME << csName ) ;
      BSONObj dummy ;

      rc = rtnUpdate( CAT_COLLECTION_SPACE_COLLECTION, matcher, modifier,
                      dummy, 0, cb, dmsCB, dpsCB, w ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to update collection: %s, match: %s, "
                   "updator: %s, rc: %d", CAT_COLLECTION_SPACE_COLLECTION,
                   matcher.toString().c_str(), modifier.toString().c_str(),
                   rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catRestoreCS( const CHAR * csName, const BSONObj & oldInfo,
                       pmdEDUCB * cb, SDB_DMSCB * dmsCB,
                       SDB_DPSCB * dpsCB, INT16 w )
   {
      INT32 rc = SDB_OK ;

      BSONObj boMatcher = BSON( CAT_COLLECTION_SPACE_NAME << csName ) ;
      BSONObj updator   = BSON( "$set" << oldInfo ) ;
      BSONObj hint ;

      rc = rtnUpdate( CAT_COLLECTION_SPACE_COLLECTION, boMatcher, updator,
                      hint, 0, cb, dmsCB, dpsCB, w ) ;
      
      PD_RC_CHECK( rc, PDERROR, "Failed to update collection: %s, match: %s, "
                   "updator: %s, rc: %d", CAT_COLLECTION_SPACE_COLLECTION,
                   boMatcher.toString().c_str(),
                   updator.toString().c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catGetCSGroups( const BSONObj & csObj, vector < INT32 > & groups )
   {
      INT32 rc = SDB_OK ;

      BSONElement beGroups = csObj.getField( CAT_GROUP_NAME ) ;
      if ( beGroups.eoo() )
      {
         goto done ;
      }

      if ( Array != beGroups.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "CS group type error, CS info: %s",
                 csObj.toString().c_str() ) ;
         goto error ;
      }

      {
         BSONObjIterator i( beGroups.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONElement ele = i.next() ;
            PD_CHECK( Object == ele.type(), SDB_INVALIDARG, error, PDERROR,
                      "CS group ele type is not Object, CS info: %s",
                      csObj.toString().c_str() ) ;
            {
               INT32 groupID = 0 ;
               BSONObj boGroup = ele.embeddedObject() ;
               rc = rtnGetIntElement( boGroup, CAT_GROUPID_NAME, groupID ) ;
               PD_RC_CHECK( rc, PDERROR, "Get CS group id failed, rc: %d, "
                            "CS info: %s", rc, csObj.toString().c_str() ) ;
               groups.push_back( groupID ) ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catCheckSpaceExist( const char *pSpaceName, BOOLEAN &isExist,
                             BSONObj &obj, pmdEDUCB *cb )
   {
      INT32 rc           = SDB_OK ;
      isExist            = FALSE ;

      BSONObj matcher = BSON( CAT_COLLECTION_SPACE_NAME << pSpaceName ) ;
      BSONObj dummyObj ;

      rc = catGetOneObj( CAT_COLLECTION_SPACE_COLLECTION, dummyObj, matcher,
                         dummyObj, cb, obj ) ;
      if ( SDB_DMS_EOC == rc )
      {
         isExist = FALSE ;
         rc = SDB_OK ;
      }
      else if ( SDB_OK == rc )
      {
         isExist = TRUE ;
      }
      else
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 matcher.toString().c_str(), CAT_COLLECTION_SPACE_COLLECTION,
                 rc ) ;
         goto error ;
      }

   done :
      return rc;
   error :
      goto done ;
   }

   INT32 catRemoveCL( const CHAR * clFullName, pmdEDUCB * cb,
                      SDB_DMSCB * dmsCB, SDB_DPSCB * dpsCB, INT16 w )
   {
      INT32 rc = SDB_OK ;

      BSONObj boMatcher = BSON( CAT_CATALOGNAME_NAME << clFullName ) ;
      BSONObj dummyObj ;

      rc = rtnDelete( CAT_COLLECTION_INFO_COLLECTION, boMatcher, dummyObj,
                      0, cb, dmsCB, dpsCB, w ) ;
      
      PD_RC_CHECK( rc, PDERROR, "Failed to del record from collection: %s, "
                   "match: %s, rc: %d", CAT_COLLECTION_INFO_COLLECTION,
                   boMatcher.toString().c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catCheckCollectionExist( const char *pCollectionName,
                                  BOOLEAN &isExist,
                                  BSONObj &obj,
                                  pmdEDUCB *cb )
   {
      INT32 rc           = SDB_OK ;
      isExist            = FALSE ;

      BSONObj matcher = BSON( CAT_CATALOGNAME_NAME << pCollectionName ) ;
      BSONObj dummyObj ;

      rc = catGetOneObj( CAT_COLLECTION_INFO_COLLECTION, dummyObj, matcher,
                         dummyObj, cb, obj ) ;
      if ( SDB_DMS_EOC == rc )
      {
         isExist = FALSE ;
         rc = SDB_OK ;
      }
      else if ( SDB_OK == rc )
      {
         isExist = TRUE ;
      }
      else
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 matcher.toString().c_str(), CAT_COLLECTION_INFO_COLLECTION,
                 rc ) ;
         goto error ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 catUpdateCatalog( const CHAR * clFullName, const BSONObj & cataInfo,
                           pmdEDUCB * cb, INT16 w )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      SDB_DPSCB *dpsCB = krcb->getDPSCB() ;
      BSONObj dummy ;
      BSONObj match = BSON( CAT_CATALOGNAME_NAME << clFullName ) ;
      BSONObj updator = BSON( "$inc" << BSON( CAT_VERSION_NAME << 1 ) <<
                              "$set" << cataInfo ) ;

      rc = rtnUpdate( CAT_COLLECTION_INFO_COLLECTION, match, updator, dummy, 0,
                      cb, dmsCB, dpsCB, w ) ;
      PD_RC_CHECK( rc, PDSEVERE, "Failed to update collection[%s] catalog info"
                   "[%s], rc: %d", clFullName, cataInfo.toString().c_str(),
                   rc ) ;      

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catAddTask( BSONObj & taskObj, pmdEDUCB * cb, INT16 w )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      SDB_DPSCB *dpsCB = krcb->getDPSCB() ;

      rc = rtnInsert( CAT_TASK_INFO_COLLECTION, taskObj, 1, 0, cb, dmsCB,
                      dpsCB, w ) ;

      if ( rc )
      {
         if ( SDB_IXM_DUP_KEY == rc )
         {
            rc = SDB_TASK_EXIST ;
         }
         else
         {
            PD_LOG( PDERROR, "Failed insert obj[%s] to collection[%s]",
                    taskObj.toString().c_str(), CAT_TASK_INFO_COLLECTION ) ;
         }
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catGetTask( UINT64 taskID, BSONObj & obj, pmdEDUCB * cb )
   {
      INT32 rc           = SDB_OK;
      BSONObj dummyObj ;
      BSONObj boMatcher = BSON( CAT_TASKID_NAME << (INT64)taskID ) ;

      rc = catGetOneObj( CAT_TASK_INFO_COLLECTION, dummyObj, boMatcher,
                         dummyObj, cb, obj ) ;
      if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_CAT_TASK_NOTFOUND ;
         goto error ;
      }
      else if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 boMatcher.toString().c_str(), CAT_TASK_INFO_COLLECTION, rc ) ;
         goto error ;
      }

   done :
      return rc;
   error :
      goto done ;
   }

   INT32 catGetTaskStatus( UINT64 taskID, INT32 & status, pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      BSONObj taskObj ;

      rc = catGetTask( taskID, taskObj, cb ) ;
      PD_RC_CHECK( rc, PDWARNING, "Get task[%lld] failed, rc: %d", taskID, rc ) ;

      rc = rtnGetIntElement( taskObj, CAT_STATUS_NAME, status ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to get field[%s], rc: %d",
                   CAT_STATUS_NAME, rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT64 catGetMaxTaskID( pmdEDUCB * cb )
   {
      INT64 taskID            = CLS_INVALID_TASKID ;
      INT32 rc                = SDB_OK ;
      SINT64 contextID        = -1 ;
      pmdKRCB *pKRCB          = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB        = pKRCB->getDMSCB() ;
      SDB_RTNCB *rtnCB        = pKRCB->getRTNCB() ;
      BSONObj dummyObj ;
      BSONObj orderby = BSON( CAT_TASKID_NAME << -1 ) ;

      rtnContextBuf buffObj ;
      INT64 startingPos       = 0 ;

      // query
      rc = rtnQuery( CAT_TASK_INFO_COLLECTION, dummyObj, dummyObj, orderby,
                     dummyObj, 0, cb, 0, 1, dmsCB, rtnCB, contextID ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to query from %s, rc: %d",
                   CAT_TASK_INFO_COLLECTION, rc ) ;

      // get more
      rc = rtnGetMore( contextID, 1, buffObj, startingPos, cb, rtnCB ) ;
      if ( rc )
      {
         if ( SDB_DMS_EOC == rc )
         {
            contextID = -1 ;
         }
         goto error ;
      }

      // copy obj
      try
      {
         BSONObj resultObj( buffObj.data() ) ;
         BSONElement ele = resultObj.getField( CAT_TASKID_NAME ) ;
         if ( !ele.isNumber() )
         {
            PD_LOG( PDWARNING, "Failed to get field[%s], type: %d",
                    CAT_TASKID_NAME, ele.type() ) ;
            goto error ;
         }
         taskID = (INT64)ele.numberLong() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         goto error ;
      }

   done:
      if ( -1 != contextID )
      {
         buffObj.release() ;
         rtnCB->contextDelete( contextID, cb ) ;
      }
      return taskID ;
   error:
      goto done ;
   }

   INT32 catUpdateTaskStatus( UINT64 taskID, INT32 status, pmdEDUCB * cb,
                              INT16 w )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      SDB_DPSCB *dpsCB = krcb->getDPSCB() ;
      BSONObj taskObj ;
      BSONObj dummy ;
      BSONObj match = BSON( CAT_TASKID_NAME << (INT64)taskID ) ;
      BSONObj updator = BSON( "$set" << BSON( CAT_STATUS_NAME << status ) ) ;

      rc = catGetTask( taskID, taskObj, cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Get task[%lld] failed, rc: %d", taskID, rc ) ;

      rc = rtnUpdate( CAT_TASK_INFO_COLLECTION, match, updator, dummy, 0,
                      cb, dmsCB, dpsCB, w ) ;
      PD_RC_CHECK( rc, PDERROR, "Update task[%lld] status to [%d] failed, "
                   "rc: %d", taskID, status, rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catRemoveTask( UINT64 taskID, pmdEDUCB *cb, INT16 w )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      SDB_DPSCB *dpsCB = krcb->getDPSCB() ;
      BSONObj taskObj ;
      BSONObj match = BSON( CAT_TASKID_NAME << (INT64)taskID ) ;
      BSONObj dummy ;

      rc = catGetTask( taskID, taskObj, cb ) ;
      if ( rc )
      {
         goto error ;
      }

      rc = rtnDelete( CAT_TASK_INFO_COLLECTION, match, dummy, 0, cb, dmsCB,
                      dpsCB, w ) ;
      PD_RC_CHECK( rc, PDERROR, "Delete task[%s] failed, rc: %d",
                   taskObj.toString().c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catRemoveTask( BSONObj &match, pmdEDUCB *cb, INT16 w )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      SDB_DPSCB *dpsCB = krcb->getDPSCB() ;
      BSONObj taskObj ;
      BSONObj dummyObj ;

      rc = catGetOneObj( CAT_TASK_INFO_COLLECTION, dummyObj, match,
                         dummyObj, cb, taskObj ) ;
      if ( SDB_DMS_EOC == rc )
      {
         rc = SDB_CAT_TASK_NOTFOUND ;
         goto error ;
      }
      else if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get obj(%s) from %s, rc: %d",
                 match.toString().c_str(), CAT_TASK_INFO_COLLECTION, rc ) ;
         goto error ;
      }

      rc = rtnDelete( CAT_TASK_INFO_COLLECTION, match, dummyObj, 0, cb,
                      dmsCB, dpsCB, w ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to remove task from collection[%s], "
                   "rc: %d, del cond: %s", rc , match.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catRemoveCLEx( const CHAR * clFullName, pmdEDUCB * cb,
                        SDB_DMSCB * dmsCB, SDB_DPSCB * dpsCB, INT16 w,
                        BOOLEAN delSubCL )
   {
      INT32 rc = SDB_OK ;
      CHAR szCLName[ DMS_COLLECTION_NAME_SZ + 1 ] = {0} ;
      CHAR szCSName[ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] = {0} ;
      BOOLEAN isExist                  = FALSE ;
      BSONObj clObj;
      clsCatalogSet cataInfo( clFullName );

      rc = catCheckCollectionExist( clFullName, isExist, clObj, cb );
      PD_RC_CHECK ( rc, PDERROR, "Failed to drop collection %s from catalog, "
                    "check collection failed(rc = %d)", clFullName, rc ) ;
      PD_CHECK( isExist, SDB_DMS_NOTEXIST, error, PDERROR,
               "collection %s is not exist" ,clFullName);

      rc = cataInfo.updateCatSet( clObj );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to parse catalog info(rc=%d)",
                  rc );
      /*PD_CHECK( cataInfo.getMainCLName().empty(),
               SDB_ILL_RM_SUB_CL, error, PDERROR,
               "illegal remove sub-collection" );*/
      

      rc = catResolveCollectionName( clFullName, ossStrlen(clFullName),
                                     szCSName, DMS_COLLECTION_SPACE_NAME_SZ,
                                     szCLName, DMS_COLLECTION_NAME_SZ ) ;
      PD_RC_CHECK( rc, PDWARNING, "Resolve collection name[%s] failed, rc: %d",
                   clFullName, rc ) ;

      try
      {
         BSONObj matcher = BSON( CAT_COLLECTION_NAME << clFullName ) ;

         // 1) Remove all collection task
         rc = catRemoveTask( matcher, cb, w ) ;
         if ( rc && SDB_CAT_TASK_NOTFOUND != rc )
         {
            goto error ;
         }
         rc = SDB_OK ;

         // 2) Remove the collection info
         rc = catRemoveCL( clFullName, cb, dmsCB, dpsCB, w ) ;
         if ( rc )
         {
            goto error ;
         }

         // 3) Pull collection from collection space info
         rc = catDelCLFromCS( szCSName, szCLName, cb, dmsCB, dpsCB, w ) ;
         if ( rc )
         {
            goto error ;
         }

         // 4) Update maincl catalog-info( if it is sub-collection )
         if ( !cataInfo.getMainCLName().empty() )
         {
            BOOLEAN isMainExist = FALSE;
            BSONObj mainCLObj;
            clsCatalogSet mainCataInfo( cataInfo.getMainCLName().c_str() );
            rc = catCheckCollectionExist( cataInfo.getMainCLName().c_str(),
                                       isMainExist, mainCLObj, cb );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to get main-collection info(rc=%d)",
                        rc );
            if (!isMainExist )
            {
               goto done;
            }
            rc = mainCataInfo.updateCatSet( mainCLObj );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to parse catalog-info of main-collection(%s)",
                        cataInfo.getMainCLName().c_str() );
            if ( !mainCataInfo.isMainCL() )
            {
               PD_LOG( PDWARNING, "main-collection have been changed" );
               goto done;
            }

            rc = mainCataInfo.delSubCL( clFullName );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to delete the sub-collection(rc=%d)",
                        rc );
            {
            BSONObj newMainCLObj = mainCataInfo.toCataInfoBson();
            rc = catUpdateCatalog( cataInfo.getMainCLName().c_str(),
                                 newMainCLObj, cb, w );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to update the catalog of main-collection(%s)",
                        cataInfo.getMainCLName().c_str() );
            }
         }
         // 5) delete sub-collection( if it is main-collection )
         else if ( cataInfo.isMainCL() )
         {
            std::vector< std::string > subCLLst;
            std::vector< std::string >::iterator iterLst;
            rc = cataInfo.getSubCLList( subCLLst );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to get sub-collection list(rc=%d)" );
            iterLst = subCLLst.begin();
            while( iterLst != subCLLst.end() )
            {
               std::vector<UINT32>  groupList;
               rc = catUnlinkCL( clFullName, iterLst->c_str(), cb,
                                 dmsCB, dpsCB, w, groupList );
               if ( SDB_DMS_NOTEXIST == rc )
               {
                  rc = SDB_OK;
                  ++iterLst;
                  continue;
               }
               PD_RC_CHECK( rc, PDERROR,
                           "failed to unlink the sub-collection(%s) "
                           "from main-collection(%s)(rc=%d)",
                           clFullName, iterLst->c_str(), rc );
               if ( delSubCL )
               {
                  rc = catRemoveCLEx( iterLst->c_str(), cb, dmsCB, dpsCB, w );
                  PD_CHECK( SDB_OK == rc || SDB_DMS_NOTEXIST == rc, rc, error,
                           PDERROR,
                           "failed to remove the sub-collection(%s)(rc=%d)",
                           iterLst->c_str(), rc );
                  rc = SDB_OK;
               }
               ++iterLst;
            }
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catRemoveCSEx( const CHAR * csName, pmdEDUCB * cb, SDB_DMSCB * dmsCB,
                        SDB_DPSCB * dpsCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      BSONObj boSpace ;
      BOOLEAN exist = FALSE ;

      rc = catCheckSpaceExist( csName, exist, boSpace, cb ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to check space exist, rc: %d", rc ) ;
      PD_CHECK( exist, SDB_DMS_CS_NOTEXIST, error, PDWARNING,
                "Collection space[%s] does not exist", csName ) ;

      try
      {
         BSONObj matcher = BSON( CAT_COLLECTION_SPACE_NAME << csName ) ;
         BSONObj dummy ;

         // 1) remove all collection for each
         BSONElement ele = boSpace.getField( CAT_COLLECTION ) ;
         if ( Array == ele.type() )
         {
            string clFullName ;
            BSONObj boTmp ;
            const CHAR *pCLName = NULL ;

            BSONObjIterator i ( ele.embeddedObject() ) ;
            while ( i.more() )
            {
               BSONElement beTmp = i.next() ;
               PD_CHECK( Object == beTmp.type(), SDB_CAT_CORRUPTION, error,
                         PDERROR, "Invalid collection record field type: %d",
                         beTmp.type() ) ;
               boTmp = beTmp.embeddedObject() ;
               rc = rtnGetStringElement( boTmp, CAT_COLLECTION_NAME, &pCLName ) ;
               PD_CHECK( SDB_OK == rc, SDB_CAT_CORRUPTION, error, PDERROR,
                         "Get field[%s] failed, rc: %d", CAT_COLLECTION_NAME,
                         rc ) ;

               clFullName = csName ;
               clFullName += "." ;
               clFullName += pCLName ;

               rc = catRemoveCLEx( clFullName.c_str(), cb, dmsCB, dpsCB, w ) ;
               PD_RC_CHECK( rc, PDWARNING, "Failed to remove collection[%s], "
                            "rc: %d", clFullName.c_str(), rc ) ;
            }
         }
         else if ( !ele.eoo() )
         {
            PD_LOG( PDERROR, "Invalid collection field[%s] type: %d",
                    CAT_COLLECTION, ele.type() ) ;
            rc = SDB_CAT_CORRUPTION ;
            goto error ;
         }

         // 2) remove collection space item
         rc = rtnDelete( CAT_COLLECTION_SPACE_COLLECTION, matcher, dummy,
                         0, cb, dmsCB, dpsCB, w ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to delete collection space[%s] item"
                      ", rc: %d", csName, rc ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catPraseFunc( const BSONObj &func, BSONObj &parsed )
   {
      INT32 rc = SDB_OK ;
      BSONElement fValue = func.getField( FMP_FUNC_VALUE ) ;
      BSONElement fType = func.getField( FMP_FUNC_TYPE ) ;
      if ( fValue.eoo() || fType.eoo() )
      {
         PD_LOG( PDERROR, "failed to find specific element from func:%s",
                 func.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( Code != fValue.type() || NumberInt != fType.type())
      {
         PD_LOG( PDERROR, "invalid type of func element:%d, %d",
                 fValue.type(), fType.type() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         BSONObjBuilder builder ;
         const CHAR *nameBegin = NULL ;
         BOOLEAN appendBegun = FALSE ;
         std::string name ;
         const CHAR *fStr = ossStrstr(fValue.valuestr(),
                                      FMP_FUNCTION_DEF) ;
         if ( NULL == fStr )
         {
            PD_LOG( PDERROR, "can not find \"function\" in funcelement" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         nameBegin = fStr + ossStrlen( FMP_FUNCTION_DEF ) ;
         while ( '\0' != *nameBegin )
         {
            if ( '(' == *nameBegin )
            {
               break ;
            }
            else if ( ' ' == *nameBegin && appendBegun )
            {
               break ;
            }
            else if ( ' ' != *nameBegin )
            {
               name.append( 1, *nameBegin ) ;
               appendBegun = TRUE ;
               ++nameBegin ;
            }
            else
            {
               ++nameBegin ;
            }
         }

         if ( name.empty() )
         {
            PD_LOG( PDERROR, "can not find func name" ) ;
            rc = SDB_INVALIDARG ;
         }

         builder.append( FMP_FUNC_NAME, name ) ;
         builder.append( fValue ) ;
         builder.append( fType ) ;
         parsed = builder.obj() ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catLinkCL( const CHAR *mainCLName, const CHAR *subCLName,
                     BSONObj &boLowBound, BSONObj &boUpBound,
                     pmdEDUCB *cb, SDB_DMSCB * dmsCB, SDB_DPSCB * dpsCB,
                     INT16 w, std::vector<UINT32>  &groupList )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isSubExist = FALSE;
      BOOLEAN isMainExist = FALSE;
      BSONObj subCLObj;
      BSONObj mainCLObj;
      BOOLEAN hasUpdateSubCL = FALSE;
      clsCatalogSet cataInfo( mainCLName );

      try
      {
         // check sub-collection
         rc = catCheckCollectionExist( subCLName, isSubExist, subCLObj, cb );
         PD_RC_CHECK(rc, PDERROR,
                     "failed to get sub-collection info(rc=%d)",
                     rc );
         PD_CHECK( isSubExist, SDB_DMS_NOTEXIST, error, PDERROR,
                  "sub-collection is not exist!" );
         {
         BSONElement beMainCLName = subCLObj.getField( CAT_MAINCL_NAME );
         if ( beMainCLName.type() == String )
         {
            std::string strMainCLName = beMainCLName.str();
            PD_CHECK( 0 == ossStrcmp( strMainCLName.c_str(), mainCLName ),
                     SDB_RELINK_SUB_CL, error, PDERROR,
                     "duplicate link sub-collection(%s), "
                     "the original main-collection is %s",
                     subCLName, strMainCLName.c_str() );
            hasUpdateSubCL = TRUE;
         }
         }

         {
         // sub-collection could not be a main-collection
         BSONElement beIsMainCL = subCLObj.getField( CAT_IS_MAINCL );
         PD_CHECK( !beIsMainCL.booleanSafe(), SDB_INVALID_SUB_CL, error, PDERROR,
                  "sub-collection could not be a main-collection!" );
         }

         {
         // get sub-collection group-list
         BSONElement beCataInfo = subCLObj.getField( CAT_CATALOGINFO_NAME );
         BSONObj boCataInfo;
         PD_CHECK( beCataInfo.type() == Array, SDB_INVALIDARG, error, PDERROR,
                  "invalid sub-collecton, failed to get the field(%s)",
                  CAT_CATALOGINFO_NAME );
         boCataInfo = beCataInfo.embeddedObject();
         BSONObjIterator iterArr( boCataInfo );
         while ( iterArr.more() )
         {
            BSONElement beTmp = iterArr.next();
            PD_CHECK( beTmp.type() == Object, SDB_INVALIDARG, error, PDERROR,
                     "invalid catalog info(%s)", subCLName );
            BSONObj boTmp = beTmp.embeddedObject();
            BSONElement beGroupId = boTmp.getField( CAT_GROUPID_NAME );
            PD_CHECK( beGroupId.isNumber(), SDB_INVALIDARG, error, PDERROR,
                     "failed to get the field(%s)", CAT_GROUPID_NAME );
            groupList.push_back( beGroupId.numberInt() );
         }
         PD_CHECK( groupList.size() != 0, SDB_SYS, error, PDERROR,
                  "the collection(%s) has no group-info!", subCLName );
         }

         // check main-collection
         rc = catCheckCollectionExist( mainCLName, isMainExist, mainCLObj, cb );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to get main-collection info(rc=%d)",
                     rc );
         PD_CHECK( isMainExist, SDB_DMS_NOTEXIST, error, PDERROR,
                  "main-collection is not exist!" );
         rc = cataInfo.updateCatSet( mainCLObj );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to parse catalog-info of main-collection(%s)",
                     mainCLName );
         PD_CHECK( cataInfo.isMainCL(), SDB_INVALID_MAIN_CL, error, PDERROR,
                  "source collection must be main-collection!" );
         SDB_ASSERT( cataInfo.isRangeSharding(), "main-collection must be range-sharding!" );

         rc = cataInfo.addSubCL( subCLName, boLowBound, boUpBound );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to add sub-collection(rc=%d)",
                     rc );

         // update sub-collection catalog info
         if ( !hasUpdateSubCL )
         {
            BSONObjBuilder subClBuilder;
            subClBuilder.appendElements( subCLObj );
            subClBuilder.append( CAT_MAINCL_NAME, mainCLName );
            BSONObj newSubCLObj = subClBuilder.done();
            rc = catUpdateCatalog( subCLName, newSubCLObj, cb, w );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to update the catalog of sub-collection(%s)",
                        subCLName );
            hasUpdateSubCL = TRUE;
         }

         // update main-collection catalog info
         {
         BSONObj newMainCLObj = cataInfo.toCataInfoBson();
         rc = catUpdateCatalog( mainCLName, newMainCLObj, cb, w );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to update the catalog of main-collection(%s)",
                     mainCLName );
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 catUnlinkCL( const CHAR *mainCLName, const CHAR *subCLName,
                     pmdEDUCB *cb, SDB_DMSCB * dmsCB, SDB_DPSCB * dpsCB,
                     INT16 w, std::vector<UINT32>  &groupList )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isSubExist = FALSE;
      BOOLEAN isMainExist = FALSE;
      BSONObj subCLObj;
      BSONObj mainCLObj;
      BOOLEAN needUpdateSubCL = FALSE;
      clsCatalogSet cataInfo( mainCLName );
      try
      {
         // check sub-collection
         rc = catCheckCollectionExist( subCLName, isSubExist, subCLObj, cb );
         PD_RC_CHECK(rc, PDERROR,
                     "failed to get sub-collection info(rc=%d)",
                     rc );
         PD_CHECK( isSubExist, SDB_DMS_NOTEXIST, error, PDERROR,
                  "sub-collection is not exist!" );
         {
         BSONElement beMainCLName = subCLObj.getField( CAT_MAINCL_NAME );
         if ( beMainCLName.type() == String )
         {
            std::string strMainCLName = beMainCLName.str();
            PD_CHECK( 0 == ossStrcmp( strMainCLName.c_str(), mainCLName ),
                     SDB_INVALIDARG, error, PDERROR,
                     "failed to unlink sub-collection(%s), "
                     "the original main-collection is %s not %s",
                     subCLName, strMainCLName.c_str(), mainCLName );
            needUpdateSubCL = TRUE;
         }
         }

         {
         // get sub-collection group-list
         BSONElement beCataInfo = subCLObj.getField( CAT_CATALOGINFO_NAME );
         BSONObj boCataInfo;
         PD_CHECK( beCataInfo.type() == Array, SDB_INVALIDARG, error, PDERROR,
                  "invalid sub-collecton, failed to get the field(%s)",
                  CAT_CATALOGINFO_NAME );
         boCataInfo = beCataInfo.embeddedObject();
         BSONObjIterator iterArr( boCataInfo );
         while ( iterArr.more() )
         {
            BSONElement beTmp = iterArr.next();
            PD_CHECK( beTmp.type() == Object, SDB_INVALIDARG, error, PDERROR,
                     "invalid catalog info(%s)", subCLName );
            BSONObj boTmp = beTmp.embeddedObject();
            BSONElement beGroupId = boTmp.getField( CAT_GROUPID_NAME );
            PD_CHECK( beGroupId.isNumber(), SDB_INVALIDARG, error, PDERROR,
                     "failed to get the field(%s)", CAT_GROUPID_NAME );
            groupList.push_back( beGroupId.numberInt() );
         }
         PD_CHECK( groupList.size() != 0, SDB_SYS, error, PDERROR,
                  "the collection(%s) has no group-info!", subCLName );
         }

         // check main-collection
         rc = catCheckCollectionExist( mainCLName, isMainExist, mainCLObj, cb );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to get main-collection info(rc=%d)",
                     rc );
         if ( isMainExist )
         {
            rc = cataInfo.updateCatSet( mainCLObj );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to parse catalog-info of main-collection(%s)",
                        mainCLName );
            PD_CHECK( cataInfo.isMainCL(), SDB_INVALID_MAIN_CL, error, PDERROR,
                     "source collection must be main-collection!" );

            rc = cataInfo.delSubCL( subCLName );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to delete the sub-collection(rc=%d)",
                        rc );
         }

         // update sub-collection catalog info
         if ( needUpdateSubCL )
         {
            BSONObjBuilder subClBuilder;
            subClBuilder.appendElements( subCLObj );
            subClBuilder.append( CAT_MAINCL_NAME, mainCLName );
            BSONObj newSubCLObj = subClBuilder.done();
            rc = catUpdateCatalog( subCLName, newSubCLObj, cb, w );

            BSONObj emptyObj;
            BSONObj match = BSON( CAT_CATALOGNAME_NAME << subCLName );
            BSONObj updator = BSON( "$inc" << BSON( CAT_VERSION_NAME << 1 ) <<
                                    "$unset" << BSON( CAT_MAINCL_NAME << "" ) );
            rc = rtnUpdate( CAT_COLLECTION_INFO_COLLECTION, match, updator,
                           emptyObj, 0, cb, dmsCB, dpsCB, w );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to update the catalog of sub-collection(%s)",
                        subCLName );
            needUpdateSubCL = FALSE;
         }

         // update main-collection catalog info
         if ( isMainExist )
         {
            BSONObj newMainCLObj = cataInfo.toCataInfoBson();
            rc = catUpdateCatalog( mainCLName, newMainCLObj, cb, w );
            PD_RC_CHECK( rc, PDERROR,
                        "failed to update the catalog of main-collection(%s)",
                        mainCLName );
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }
}


