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

   Source File Name = coordCommandData.cpp

   Descriptive Name = Coord Commands for Data Management

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   user command processing on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/27/2017  XJH Init
   Last Changed =

*******************************************************************************/

#include "coordCommandData.hpp"
#include "msgMessage.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

using namespace bson;

namespace engine
{
   /*
      _coordDataCMD2Phase implement
   */
   _coordDataCMD2Phase::_coordDataCMD2Phase()
   {
   }

   _coordDataCMD2Phase::~_coordDataCMD2Phase()
   {
   }

   INT32 _coordDataCMD2Phase::_generateDataMsg ( MsgHeader *pMsg,
                                                 pmdEDUCB *cb,
                                                 coordCMDArguments *pArgs,
                                                 const vector<BSONObj> &cataObjs,
                                                 CHAR **ppMsgBuf,
                                                 INT32 *pBufSize )
   {
      pMsg->opCode = MSG_BS_QUERY_REQ ;
      *ppMsgBuf = (CHAR*)pMsg ;
      *pBufSize = pMsg->messageLength ;

      return SDB_OK ;
   }

   void _coordDataCMD2Phase::_releaseDataMsg( CHAR *pMsgBuf,
                                              INT32 bufSize,
                                              pmdEDUCB *cb )
   {
   }

   INT32 _coordDataCMD2Phase::_generateRollbackDataMsg ( MsgHeader *pMsg,
                                                         pmdEDUCB *cb,
                                                         coordCMDArguments *pArgs,
                                                         CHAR **ppMsgBuf,
                                                         INT32 *pBufSize )
   {
      *ppMsgBuf = (CHAR*)pMsg ;
      *pBufSize = pMsg->messageLength ;

      return SDB_OK ;
   }

   void _coordDataCMD2Phase::_releaseRollbackDataMsg( CHAR *pMsgBuf,
                                                      INT32 bufSize,
                                                      pmdEDUCB *cb )
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DATA2PHASE_DOONCATA, "_coordDataCMD2Phase::_doOnCataGroup" )
   INT32 _coordDataCMD2Phase::_doOnCataGroup( MsgHeader *pMsg,
                                              pmdEDUCB *cb,
                                              rtnContextCoord **ppContext,
                                              coordCMDArguments *pArgs,
                                              CoordGroupList *pGroupLst,
                                              vector<BSONObj> *pReplyObjs )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_DATA2PHASE_DOONCATA ) ;

      rtnContextCoord *pContext = NULL ;
      coordCataSel cataSel ;

      if ( _flagUpdateBeforeCata() && _flagDoOnCollection() )
      {
         rc = cataSel.bind( _pResource, pArgs->_targetName.c_str(),
                            cb, TRUE, TRUE ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get or update collection[%s]'s catalog info "
                    "failed on command[%s], rc: %d",
                    pArgs->_targetName.c_str(), getName(), rc ) ;
            goto error ;
         }
      }

   retry:
      if ( _flagUpdateBeforeCata() && _flagDoOnCollection() )
      {
         MsgOpQuery *pOpMsg = (MsgOpQuery *)pMsg ;
         pOpMsg->version = cataSel.getCataPtr()->getVersion() ;
      }

      rc = _coordCMD2Phase::_doOnCataGroup( pMsg, cb, &pContext, pArgs,
                                            pGroupLst, pReplyObjs ) ;
      if ( rc && _flagUpdateBeforeCata() && _flagDoOnCollection() )
      {
         if ( checkRetryForCLOpr( rc, NULL, cataSel, pMsg,
                                  cb, rc, NULL, TRUE ) )
         {
            _groupSession.getGroupCtrl()->incRetry() ;
            goto retry ;
         }
      }
      if ( rc )
      {
         PD_LOG( PDERROR, "Do on catalog failed on command[%s, targe:%s], "
                 "rc: %d", getName(), pArgs->_targetName.c_str(), rc ) ;
         goto error ;
      }

   done :
      if ( pContext )
      {
         (*ppContext) = pContext ;
      }
      PD_TRACE_EXITRC ( COORD_DATA2PHASE_DOONCATA, rc ) ;
      return rc ;
   error :
      if ( pContext )
      {
         SDB_RTNCB *pRtnCB = pmdGetKRCB()->getRTNCB() ;
         pRtnCB->contextDelete( pContext->contextID(), cb ) ;
         pContext = NULL ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DATA2PHASE_DOONDATA, "_coordDataCMD2Phase::_doOnDataGroup" )
   INT32 _coordDataCMD2Phase::_doOnDataGroup ( MsgHeader *pMsg,
                                               pmdEDUCB *cb,
                                               rtnContextCoord **ppContext,
                                               coordCMDArguments *pArgs,
                                               const CoordGroupList &groupLst,
                                               const vector<BSONObj> &cataObjs,
                                               CoordGroupList &sucGroupLst )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_DATA2PHASE_DOONDATA ) ;

      // For DropCL/DropCS, this will guarantee data updates are
      // started before catalog updates
      if ( _flagDoOnCollection() )
      {
         rc = executeOnCL( pMsg, cb, pArgs->_targetName.c_str(),
                           _flagUpdateBeforeData(),
                           _flagUseGrpLstInCoord() ? NULL : &groupLst,
                           &(pArgs->_ignoreRCList), &sucGroupLst,
                           ppContext, pArgs->_pBuf ) ;
      }
      else
      {
         rc = executeOnDataGroup( pMsg, cb, groupLst, TRUE,
                                  &(pArgs->_ignoreRCList), NULL,
                                  ppContext, pArgs->_pBuf ) ;
      }

      if ( rc )
      {
         PD_LOG( PDERROR, "Do on data group failed on command[%s, targe:%s], "
                 "rc: %d", getName(), pArgs->_targetName.c_str(), rc ) ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_DATA2PHASE_DOONDATA, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DATA2PHASE_DOAUDIT "_coordDataCMD2Phase::_doAudit" )
   INT32 _coordDataCMD2Phase::_doAudit ( coordCMDArguments *pArgs, INT32 rc )
   {
      PD_TRACE_ENTRY ( COORD_DATA2PHASE_DOAUDIT ) ;

      if ( !pArgs->_targetName.empty() )
      {
         PD_AUDIT_COMMAND( AUDIT_DDL, getName(),
                           _flagDoOnCollection() ? AUDIT_OBJ_CL : AUDIT_OBJ_CS,
                           pArgs->_targetName.c_str(), rc, "Option: %s",
                           pArgs->_boQuery.toString().c_str() ) ;
      }

      PD_TRACE_EXIT ( COORD_DATA2PHASE_DOAUDIT ) ;
      return SDB_OK ;
   }

   /*
      _coordDataCMD3Phase implement
   */
   _coordDataCMD3Phase::_coordDataCMD3Phase()
   {
   }

   _coordDataCMD3Phase::~_coordDataCMD3Phase()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DATA3PHASE_DOONCATA2, "_coordDataCMD3Phase::_doOnCataGroupP2" )
   INT32 _coordDataCMD3Phase::_doOnCataGroupP2 ( MsgHeader *pMsg,
                                                 pmdEDUCB *cb,
                                                 rtnContextCoord **ppContext,
                                                 coordCMDArguments *pArgs,
                                                 const CoordGroupList &pGroupLst )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_DATA3PHASE_DOONCATA2 ) ;

      rc = _processContext( cb, ppContext, 1 ) ;

      PD_TRACE_EXITRC ( COORD_DATA3PHASE_DOONCATA2, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DATA3PHASE_DOONDATA2, "_coordDataCMD3Phase::_doOnDataGroupP2" )
   INT32 _coordDataCMD3Phase::_doOnDataGroupP2 ( MsgHeader *pMsg,
                                                 pmdEDUCB *cb,
                                                 rtnContextCoord **ppContext,
                                                 coordCMDArguments *pArgs,
                                                 const CoordGroupList &groupLst,
                                                 const vector<BSONObj> &cataObjs )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_DATA3PHASE_DOONDATA2 ) ;

      rc = _processContext( cb, ppContext, 1 ) ;

      PD_TRACE_EXITRC ( COORD_DATA3PHASE_DOONDATA2, rc ) ;
      return rc ;
   }

   /*
      _coordCMDCreateDomain implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDCreateDomain,
                                      CMD_NAME_CREATE_DOMAIN,
                                      FALSE ) ;
   _coordCMDCreateDomain::_coordCMDCreateDomain()
   {
   }

   _coordCMDCreateDomain::~_coordCMDCreateDomain()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_CREATEDOMAIN_EXE, "_coordCMDCreateDomain::execute" )
   INT32 _coordCMDCreateDomain::execute( MsgHeader *pMsg,
                                         pmdEDUCB *cb,
                                         INT64 &contextID,
                                         rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_CREATEDOMAIN_EXE ) ;

      contextID = -1 ;

      MsgOpQuery *forward  = (MsgOpQuery *)pMsg;
      forward->header.opCode = MSG_CAT_CREATE_DOMAIN_REQ ;

      _printDebug ( (const CHAR*)pMsg, getName() ) ;

      rc = executeOnCataGroup ( pMsg, cb, TRUE, NULL, NULL, buf ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Execute on catalog failed in command[%s], "
                  "rc: %d", getName(), rc ) ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_CREATEDOMAIN_EXE, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
      _coordCMDDropDomain implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDDropDomain,
                                      CMD_NAME_DROP_DOMAIN,
                                      FALSE ) ;
   _coordCMDDropDomain::_coordCMDDropDomain()
   {
   }

   _coordCMDDropDomain::~_coordCMDDropDomain()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DROPDOMAIN_EXE, "_coordCMDDropDomain::execute" )
   INT32 _coordCMDDropDomain::execute( MsgHeader *pMsg,
                                       pmdEDUCB *cb,
                                       INT64 &contextID,
                                       rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_DROPDOMAIN_EXE ) ;

      contextID = -1 ;

      MsgOpQuery *forward  = (MsgOpQuery *)pMsg ;
      forward->header.opCode = MSG_CAT_DROP_DOMAIN_REQ ;

      _printDebug ( (const CHAR*)pMsg, getName() ) ;

      rc = executeOnCataGroup ( pMsg, cb, TRUE, NULL, NULL, buf ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Execute on catalog failed in command[%s], "
                  "rc: %d", getName(), rc ) ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_DROPDOMAIN_EXE, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
      _coordCMDAlterDomain implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDAlterDomain,
                                      CMD_NAME_ALTER_DOMAIN,
                                      FALSE ) ;
   _coordCMDAlterDomain::_coordCMDAlterDomain()
   {
   }

   _coordCMDAlterDomain::~_coordCMDAlterDomain()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_ALTERDOMAIN_EXE, "_coordCMDAlterDomain::execute" )
   INT32 _coordCMDAlterDomain::execute( MsgHeader *pMsg,
                                        pmdEDUCB *cb,
                                        INT64 &contextID,
                                        rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_ALTERDOMAIN_EXE ) ;

      contextID = -1 ;

      MsgOpQuery *forward  = (MsgOpQuery *)pMsg;
      forward->header.opCode = MSG_CAT_ALTER_DOMAIN_REQ;

      _printDebug ( (const CHAR*)pMsg, getName() ) ;

      rc = executeOnCataGroup ( pMsg, cb, TRUE, NULL, NULL, buf ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Execute on catalog failed in command[%s], "
                  "rc: %d", getName(), rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( COORD_ALTERDOMAIN_EXE, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
      _coordCMDCreateCollectionSpace implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDCreateCollectionSpace,
                                      CMD_NAME_CREATE_COLLECTIONSPACE,
                                      FALSE ) ;
   _coordCMDCreateCollectionSpace::_coordCMDCreateCollectionSpace()
   {
   }

   _coordCMDCreateCollectionSpace::~_coordCMDCreateCollectionSpace()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_CREATECS_EXE, "_coordCMDCreateCollectionSpace::execute" )
   INT32 _coordCMDCreateCollectionSpace::execute( MsgHeader *pMsg,
                                                  pmdEDUCB *cb,
                                                  INT64 &contextID,
                                                  rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_CREATECS_EXE ) ;

      CHAR *pQuery = NULL ;

      // fill default-reply
      contextID = -1 ;
      MsgOpQuery *pCreateReq = (MsgOpQuery *)pMsg;

      try
      {
         BSONObj boQuery ;
         BSONElement ele ;
         const CHAR *pCSName = NULL ;

         _printDebug ( (const CHAR*)pMsg, getName() ) ;

         rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL,
                               NULL, NULL, &pQuery, NULL, NULL, NULL ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to extract message, rc: %d", rc ) ;

         boQuery = BSONObj( pQuery ) ;
         ele = boQuery.getField( FIELD_NAME_NAME ) ;
         if ( String != ele.type() )
         {
            PD_LOG( PDERROR, "Get field[%s] failed from obj[%s] in "
                    "command[%s]", FIELD_NAME_NAME, boQuery.toString().c_str(),
                    getName() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         pCSName = ele.valuestr() ;

         pCreateReq->header.opCode = MSG_CAT_CREATE_COLLECTION_SPACE_REQ ;
         // execute create collection on catalog
         rc = executeOnCataGroup ( pMsg, cb, TRUE, NULL, NULL, buf ) ;
         /// AUDIT
         PD_AUDIT_COMMAND( AUDIT_DDL, getName(), AUDIT_OBJ_CS,
                           pCSName, rc, "Option:%s",
                           boQuery.toString().c_str() ) ;
         /// CHECK ERRORS
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to create collection space[%s], rc: %d",
                     pCSName, rc ) ;
            goto error ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( COORD_CREATECS_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDDropCollectionSpace implement
   */
   _coordCMDDropCollectionSpace::_coordCMDDropCollectionSpace()
   {
   }

   _coordCMDDropCollectionSpace::~_coordCMDDropCollectionSpace()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DROPCS_PARSEMSG, "_coordCMDDropCollectionSpace::_parseMsg" )
   INT32 _coordCMDDropCollectionSpace::_parseMsg ( MsgHeader *pMsg,
                                                   coordCMDArguments *pArgs )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_DROPCS_PARSEMSG ) ;

      try
      {
         rc = rtnGetSTDStringElement( pArgs->_boQuery,
                                      CAT_COLLECTION_SPACE_NAME,
                                      pArgs->_targetName ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get field[%s] failed on command[%s], "
                    "rc: %d", CAT_COLLECTION_SPACE_NAME, getName(), rc ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         if ( pArgs->_targetName.empty() )
         {
            PD_LOG( PDERROR, "Collectionspace name is empty in command[%s]",
                    getName() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         // Add ignore return codes
         pArgs->_ignoreRCList.insert( SDB_DMS_CS_NOTEXIST ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_DROPCS_PARSEMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _coordCMDDropCollectionSpace::_generateCataMsg ( MsgHeader *pMsg,
                                                          pmdEDUCB *cb,
                                                          coordCMDArguments *pArgs,
                                                          CHAR **ppMsgBuf,
                                                          INT32 *pBufSize )
   {
      pMsg->opCode = MSG_CAT_DROP_SPACE_REQ ;
      *ppMsgBuf = (CHAR*)pMsg ;
      *pBufSize = pMsg->messageLength ;

      return SDB_OK ;
   }

   void _coordCMDDropCollectionSpace::_releaseCataMsg( CHAR *pMsgBuf,
                                                       INT32 bufSize,
                                                       pmdEDUCB *cb )
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DROPCS_DOCOMPLETE, "_coordCMDDropCollectionSpace::_doComplete" )
   INT32 _coordCMDDropCollectionSpace::_doComplete ( MsgHeader *pMsg,
                                                     pmdEDUCB * cb,
                                                     coordCMDArguments *pArgs )
   {
      PD_TRACE_ENTRY ( COORD_DROPCS_DOCOMPLETE ) ;

      vector< string > subCLSet ;
      _pResource->removeCataInfoByCS( pArgs->_targetName.c_str(), &subCLSet ) ;

      /// clear relate sub collection's catalog info
      vector< string >::iterator it = subCLSet.begin() ;
      while( it != subCLSet.end() )
      {
         _pResource->removeCataInfo( (*it).c_str() ) ;
         ++it ;
      }

      PD_TRACE_EXIT ( COORD_DROPCS_DOCOMPLETE ) ;
      return SDB_OK ;
   }

   /*
      _coordCMDCreateCollection implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDCreateCollection,
                                      CMD_NAME_CREATE_COLLECTION,
                                      FALSE ) ;
   _coordCMDCreateCollection::_coordCMDCreateCollection()
   {
   }

   _coordCMDCreateCollection::~_coordCMDCreateCollection()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_CREATECL_PARSEMSG, "_coordCMDCreateCollection::_parseMsg" )
   INT32 _coordCMDCreateCollection::_parseMsg ( MsgHeader *pMsg,
                                                coordCMDArguments *pArgs )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( COORD_CREATECL_PARSEMSG ) ;

      try
      {
         BOOLEAN isMainCL = FALSE ;

         rc = rtnGetSTDStringElement( pArgs->_boQuery, CAT_COLLECTION_NAME,
                                      pArgs->_targetName ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get field[%s] failed on command[%s], "
                    "rc: %d", CAT_COLLECTION_NAME, getName(), rc ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         if ( pArgs->_targetName.empty() )
         {
            PD_LOG( PDERROR, "Collection name is empty in command[%s]",
                    getName() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         rc = rtnGetBooleanElement( pArgs->_boQuery, FIELD_NAME_ISMAINCL,
                                    isMainCL ) ;
         if ( SDB_FIELD_NOT_EXIST == rc )
         {
            isMainCL = FALSE ;
            rc = SDB_OK ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Get field[%s] failed on command[%s], rc: %d",
                    FIELD_NAME_ISMAINCL, getName(), rc ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         if ( isMainCL )
         {
            // Check sharding keys
            BSONObj boShardingKey ;
            // Check sharding type
            string shardingType ;

            rc = rtnGetObjElement( pArgs->_boQuery, FIELD_NAME_SHARDINGKEY,
                                   boShardingKey ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Get field[%s] failed on command[%s], rc: %d",
                       FIELD_NAME_SHARDINGKEY, getName(), rc ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }

            rc = rtnGetSTDStringElement( pArgs->_boQuery, FIELD_NAME_SHARDTYPE,
                                         shardingType ) ;
            if ( SDB_OK == rc )
            {
               if ( 0 != shardingType.compare( FIELD_NAME_SHARDTYPE_HASH ) )
               {
                  PD_LOG( PDERROR, "Sharding type must be range in "
                          "main colllection" ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
            }
            else if ( SDB_FIELD_NOT_EXIST != rc )
            {
               PD_LOG( PDERROR, "Get field[%s] failed on command[%s], rc: %d",
                       FIELD_NAME_SHARDTYPE_HASH, getName(), rc ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }

         // Add ignored error return code
         pArgs->_ignoreRCList.insert( SDB_DMS_EXIST ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_CREATECL_PARSEMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _coordCMDCreateCollection::_generateCataMsg ( MsgHeader *pMsg,
                                                       pmdEDUCB *cb,
                                                       coordCMDArguments *pArgs,
                                                       CHAR **ppMsgBuf,
                                                       INT32 *pBufSize )
   {
      pMsg->opCode = MSG_CAT_CREATE_COLLECTION_REQ ;
      *ppMsgBuf = pMsg ;
      *pBufSize = pMsg->messageLength ;

      return SDB_OK ;
   }

   void _coordCMDCreateCollection::_releaseCataMsg( CHAR *pMsgBuf,
                                                    INT32 bufSize,
                                                    pmdEDUCB *cb )
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_CREATECL_GENROLLBACKMSG, "_coordCMDCreateCollection::_generateRollbackDataMsg" )
   INT32 _coordCMDCreateCollection::_generateRollbackDataMsg( MsgHeader *pMsg,
                                                              pmdEDUCB *cb,
                                                              coordCMDArguments *pArgs,
                                                              CHAR **ppMsgBuf,
                                                              INT32 *pBufSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_CREATECL_GENROLLBACKMSG ) ;

      rc = msgBuildDropCLMsg( ppMsgBuf, pBufSize,
                              pArgs->_targetName.c_str(),
                              0, cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Build rollback message failed on command[%s], "
                 "rc: %d", getName(), rc ) ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_CREATECL_GENROLLBACKMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   void _coordCMDCreateCollection::_releaseRollbackDataMsg( CHAR *pMsgBuf,
                                                            INT32 bufSize,
                                                            pmdEDUCB *cb )
   {
      if ( pMsgBuf )
      {
         msgReleaseBuffer( pMsgBuf, cb ) ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_CREATECL_ROLLBACKONDATA, "_coordCMDCreateCollection::_rollbackOnDataGroup" )
   INT32 _coordCMDCreateCollection::_rollbackOnDataGroup ( MsgHeader *pMsg,
                                                           pmdEDUCB *cb,
                                                           coordCMDArguments *pArgs,
                                                           const CoordGroupList &groupLst )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_CREATECL_ROLLBACKONDATA ) ;

      SET_RC ignoreRC ;
      rtnContextCoord *pCtxForData = NULL ;

      ignoreRC.insert( SDB_DMS_NOTEXIST ) ;
      ignoreRC.insert( SDB_DMS_CS_NOTEXIST ) ;

      rc = executeOnCL( pMsg, cb, pArgs->_targetName.c_str(),
                        TRUE, &groupLst, &ignoreRC, NULL,
                        &pCtxForData, pArgs->_pBuf ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Rollback-phase1 command[%s, target:%s] on "
                 "data group failed, rc: %d", getName(),
                 pArgs->_targetName.c_str(), rc ) ;
         goto error ;
      }

      if ( pCtxForData )
      {
         rc = _processContext( cb, &pCtxForData, -1 ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Rollback-phase2 command[%s, targe:%s] on "
                    "data group failed, rc: %d", getName(),
                    pArgs->_targetName.c_str(), rc ) ;
            goto error ;
         }
      }

      // Clear Coord catalog info
      _pResource->removeCataInfo( pArgs->_targetName.c_str() ) ;

   done :
      if ( pCtxForData )
      {
         pmdKRCB *pKrcb = pmdGetKRCB();
         _SDB_RTNCB *pRtncb = pKrcb->getRTNCB();
         pRtncb->contextDelete ( pCtxForData->contextID(), cb ) ;
      }
      PD_TRACE_EXITRC ( COORD_CREATECL_ROLLBACKONDATA, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
      _coordCMDDropCollection implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDDropCollection,
                                      CMD_NAME_DROP_COLLECTION,
                                      FALSE ) ;
   _coordCMDDropCollection::_coordCMDDropCollection()
   {
   }

   _coordCMDDropCollection::~_coordCMDDropCollection()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DROPCL_PARSEMSG, "_coordCMDDropCollection::_parseMsg" )
   INT32 _coordCMDDropCollection::_parseMsg ( MsgHeader *pMsg,
                                              coordCMDArguments *pArgs )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_DROPCL_PARSEMSG ) ;

      try
      {
         rc = rtnGetSTDStringElement( pArgs->_boQuery, CAT_COLLECTION_NAME,
                                      pArgs->_targetName ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get field[%s] failed on command[%s], rc: %d",
                    CAT_COLLECTION_NAME, getName(), rc ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         if ( pArgs->_targetName.empty() )
         {
            PD_LOG( PDERROR, "Collection name is empty in command[%s]",
                    getName() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         pArgs->_ignoreRCList.insert( SDB_DMS_NOTEXIST ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_DROPCL_PARSEMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _coordCMDDropCollection::_generateCataMsg ( MsgHeader *pMsg,
                                                     pmdEDUCB *cb,
                                                     coordCMDArguments *pArgs,
                                                     CHAR **ppMsgBuf,
                                                     INT32 *pBufSize )
   {
      pMsg->opCode = MSG_CAT_DROP_COLLECTION_REQ ;
      *ppMsgBuf = (CHAR*)pMsg ;
      pBufSize = pMsg->messageLength ;

      return SDB_OK ;
   }

   void _coordCMDDropCollection::_releaseCataMsg( CHAR *pMsgBuf,
                                                  INT32 bufSize,
                                                  pmdEDUCB *cb )
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DROPCL_GENDATAMSG, "_coordCMDDropCollection::_generateDataMsg" )
   INT32 _coordCMDDropCollection::_generateDataMsg ( MsgHeader *pMsg,
                                                     pmdEDUCB *cb,
                                                     coordCMDArguments *pArgs,
                                                     const vector<BSONObj> &cataObjs,
                                                     CHAR **ppMsgBuf,
                                                     INT32 *pBufSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_DROPCL_GENDATAMSG ) ;

      /// alloc message
      rc = _coordDataCMD3Phase::_generateDataMsg( pMsg, cb, pArgs,
                                                  cataObjs, ppMsgBuf,
                                                  pBufSize ) ;
      if ( rc )
      {
         goto error ;
      }
      else if ( cataObjs.size() > 0 )
      {
         try
         {
            CoordCataInfoPtr cataPtr ;
            BSONObj objCata ;
            BSONElement beCollection = cataObjs[0].getField( CAT_COLLECTION ) ;
            if ( Object == beCollection.type() )
            {
               objCata = beCollection.embeddedObject() ;
               // The catalog info of collection maybe too old
               // The reply from Catalog implies that info need to be updated
               PD_LOG( PDDEBUG, "Updating catalog info of collection [%s]",
                       pArgs->_targetName.c_str() ) ;
               rc = coordInitCataPtrFromObj( objCata, cataPtr ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Init catalog info from obj[%s] failed, "
                          "collection:%s, rc: %d", pArgs->_targetName.c_str(),
                          objCata.toString().c_str(), rc ) ;
                  goto error ;
               }
               _pResource->addCataInfo( cataPtr ) ;
               ((MsgOpQuery*)ppMsgBuf)->version = cataPtr->getVersion() ;
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_SYS ;
            PD_LOG ( PDERROR, "Occur exception when parse catalog "
                     "object info: %s", e.what() ) ;
            goto error ;
         }
      }

   done :
      PD_TRACE_EXITRC( COORD_DROPCL_GENDATAMSG, rc ) ;
      return rc ;
   error :
      if ( *ppMsgBuf )
      {
         _coordDataCMD3Phase::_releaseDataMsg( *ppMsgBuf, *pBufSize, cb ) ;
         *ppMsgBuf = NULL ;
         *pBufSize = 0 ;
      }
      goto done ;
   }

   void _coordCMDDropCollection::_releaseDataMsg( CHAR *pMsgBuf,
                                                  INT32 bufSize,
                                                  pmdEDUCB *cb )
   {
      _coordDataCMD3Phase::_releaseDataMsg( pMsgBuf, bufSize, cb ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_DROPCL_DOCOMPLETE, "_coordCMDDropCollection::_doComplete" )
   INT32 _coordCMDDropCollection::_doComplete ( MsgHeader *pMsg,
                                                pmdEDUCB *cb,
                                                coordCMDArguments *pArgs )
   {
      PD_TRACE_ENTRY ( COORD_DROPCL_DOCOMPLETE) ;

      _pResource->removeCataInfoWithMain( pArgs->_targetName.c_str() ) ;

      PD_TRACE_EXIT ( COORD_DROPCL_DOCOMPLETE ) ;
      return SDB_OK ;
   }

   /*
      _coordCMDAlterCollection implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDAlterCollection,
                                      CMD_NAME_ALTER_COLLECTION,
                                      FALSE ) ;
   _coordCMDAlterCollection::_coordCMDAlterCollection()
   {
   }

   _coordCMDAlterCollection::~_coordCMDAlterCollection()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_ALTERCL_PARSEMSG, "_coordCMDAlterCollection::_parseMsg" )
   INT32 _coordCMDAlterCollection::_parseMsg ( MsgHeader *pMsg,
                                               coordCMDArguments *pArgs )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_ALTERCL_PARSEMSG ) ;

      try
      {
         BOOLEAN isOldAlterCMD = FALSE ;

         rc = rtnGetSTDStringElement( pArgs->_boQuery, CAT_COLLECTION_NAME,
                                      pArgs->_targetName ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get failed[%s] failed on command[%s], rc: %d",
                    CAT_COLLECTION_NAME, getName(), rc ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         if ( pArgs->_targetName.empty() )
         {
            PD_LOG( PDERROR, "Collection name is empty in command[%s]",
                    getName() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         if ( pArgs->_boQuery.getField( FIELD_NAME_VERSION ).eoo() )
         {
            isOldAlterCMD = TRUE ;
         }

         PD_LOG ( PDDEBUG, "Alter collection command is %s",
                  isOldAlterCMD ? "old" : "new" ) ;

         if ( isOldAlterCMD )
         {
            /// we only want to update data's catalog version.
            pArgs->_ignoreRCList.insert( SDB_MAIN_CL_OP_ERR ) ;
            pArgs->_ignoreRCList.insert( SDB_CLS_COORD_NODE_CAT_VER_OLD ) ;
         }
         else
         {
            pArgs->_ignoreRCList.insert( SDB_IXM_REDEF ) ;
            pArgs->_ignoreRCList.insert( SDB_IXM_NOTEXIST ) ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_ALTERCL_PARSEMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _coordCMDAlterCollection::_generateCataMsg ( MsgHeader *pMsg,
                                                      pmdEDUCB *cb,
                                                      coordCMDArguments *pArgs,
                                                      CHAR **ppMsgBuf,
                                                      INT32 *pBufSize )
   {
      pMsg->opCode = MSG_CAT_ALTER_COLLECTION_REQ ;
      *ppMsgBuf = ( CHAR* )pMsg ;
      *pBufSize = pMsg->messageLength ;

      return SDB_OK ;
   }

   void _coordCMDAlterCollection::_releaseCataMsg( CHAR *pMsgBuf,
                                                   INT32 bufSize,
                                                   pmdEDUCB *cb )
   {
   }

   /*
      _coordCMDLinkCollection implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDLinkCollection,
                                      CMD_NAME_LINK_CL,
                                      FALSE ) ;
   _coordCMDLinkCollection::_coordCMDLinkCollection()
   {
   }

   _coordCMDLinkCollection::~_coordCMDLinkCollection()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_LINKCL_PARSEMSG, "_coordCMDLinkCollection::_parseMsg" )
   INT32 _coordCMDLinkCollection::_parseMsg ( MsgHeader *pMsg,
                                              coordCMDArguments *pArgs )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_LINKCL_PARSEMSG ) ;

      try
      {
         BSONObj lowBound, upBound ;

         rc = rtnGetSTDStringElement( pArgs->_boQuery, CAT_SUBCL_NAME,
                                      _subCLName ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get field[%s] failed on command[%s], rc: %d",
                    CAT_SUBCL_NAME, getName(), rc ) ;
            goto error ;
         }
         if ( _subCLName.empty() )
         {
            PD_LOG( PDERROR, "Sub collection name is empty in command[%s]",
                    getName() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         rc = rtnGetSTDStringElement( pArgs->_boQuery, CAT_COLLECTION_NAME,
                                      pArgs->_targetName ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get field[%s] failed on command[%s], rc: %d",
                    CAT_COLLECTION_NAME, getName(), rc ) ;
            goto error ;
         }
         if ( pArgs->_targetName.empty() )
         {
            PD_LOG( PDERROR, "Collection name is empty in command[%s]",
                    getName() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         rc = rtnGetObjElement( pArgs->_boQuery, CAT_LOWBOUND_NAME,
                                lowBound ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get field[%s] failed on command[%s], rc: %d",
                    CAT_LOWBOUND_NAME, getName(), rc ) ;
            goto error ;
         }

         rc = rtnGetObjElement( pArgs->_boQuery, CAT_UPBOUND_NAME,
                                upBound ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Get field[%s] failed on command[%s], rc: %d",
                    CAT_UPBOUND_NAME, getName(), rc ) ;
            goto error ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_LINKCL_PARSEMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _coordCMDLinkCollection::_generateCataMsg ( MsgHeader *pMsg,
                                                     pmdEDUCB *cb,
                                                     coordCMDArguments *pArgs,
                                                     CHAR **ppMsgBuf,
                                                     INT32 *pBufSize )
   {
      pMsg->opCode = MSG_CAT_LINK_CL_REQ ;
      *ppMsgBuf = (CHAR*)pMsg ;
      *pBufSize = pMsg->messageLength ;

      return SDB_OK ;
   }

   void _coordCMDLinkCollection::_releaseCataMsg( CHAR *pMsgBuf,
                                                  INT32 bufSize,
                                                  pmdEDUCB *cb )
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_LINKCL_GENROLLBACKMSG, "_coordCMDLinkCollection::_generateRollbackDataMsg" )
   INT32 _coordCMDLinkCollection::_generateRollbackDataMsg ( MsgHeader *pMsg,
                                                             pmdEDUCB *cb,
                                                             coordCMDArguments *pArgs,
                                                             CHAR **ppMsgBuf,
                                                             INT32 *pBufSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_LINKCL_GENROLLBACKMSG ) ;

      rc = msgBuildUnlinkCLMsg( ppMsgBuf, &pBufSize,
                                pArgs->_targetName.c_str(),
                                _subCLName.c_str(), 0, cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Build rollback message failed on command[%s], "
                 "rc: %d", getName(), rc ) ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_LINKCL_GENROLLBACKMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   void _coordCMDLinkCollection::_releaseRollbackDataMsg( CHAR *pMsgBuf,
                                                          INT32 bufSize,
                                                          pmdEDUCB *cb )
   {
      if ( pMsgBuf )
      {
         msgReleaseBuffer( pMsgBuf, cb ) ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_LINKCL_ROLLBACKONDATA, "_coordCMDLinkCollection::_rollbackOnDataGroup" )
   INT32 _coordCMDLinkCollection::_rollbackOnDataGroup ( MsgHeader *pMsg,
                                                         pmdEDUCB *cb,
                                                         coordCMDArguments *pArgs,
                                                         const CoordGroupList &groupLst )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_LINKCL_ROLLBACKONDATA ) ;

      rc = executeOnCL( pMsg, cb, pArgs->_targetName.c_str(),
                        TRUE, &groupLst, &(pArgs->_ignoreRCList), NULL,
                        NULL, pArgs->_pBuf ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Rollback command[%s, target:%s, sub:%s] on data "
                 "group failed, rc: %d", getName(),
                 pArgs->_targetName.c_str(), _subCLName.c_str(), rc ) ;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( COORD_LINKCL_ROLLBACKONDATA, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
      _coordCMDUnlinkCollection implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDUnlinkCollection,
                                      CMD_NAME_UNLINK_CL,
                                      FALSE ) ;
   _coordCMDUnlinkCollection::_coordCMDUnlinkCollection()
   {
   }

   _coordCMDUnlinkCollection::~_coordCMDUnlinkCollection()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDUNLINKCL_PARSEMSG, "rtnCoordCMDUnlinkCollection::_parseMsg" )
   INT32 rtnCoordCMDUnlinkCollection::_parseMsg ( MsgHeader *pMsg,
                                                  _rtnCMDArguments *pArgs )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( CMD_RTNCOCMDUNLINKCL_PARSEMSG ) ;

      _rtnCMDUnlinkCLArgs *pSelfArgs = ( _rtnCMDUnlinkCLArgs * )pArgs ;

      try
      {
         string mainCLName, subCLName ;

         rc = rtnGetSTDStringElement( pSelfArgs->_boQuery, CAT_SUBCL_NAME,
                                      subCLName ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to %s: failed to get the field [%s] from query",
                      _getCommandName(), CAT_SUBCL_NAME ) ;
         PD_CHECK( !subCLName.empty(),
                   SDB_INVALIDARG, error, PDERROR,
                   "Failed to %s: sub-collection name can't be empty!",
                   _getCommandName() ) ;

         rc = rtnGetSTDStringElement( pSelfArgs->_boQuery, CAT_COLLECTION_NAME,
                                      mainCLName ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to %s: failed to get the field [%s] from query",
                      _getCommandName(), CAT_COLLECTION_NAME ) ;
         PD_CHECK( !mainCLName.empty(),
                   SDB_INVALIDARG, error, PDERROR,
                   "Failed to %s: main-collection name can't be empty!",
                   _getCommandName() ) ;

         pSelfArgs->_targetName = mainCLName ;
         pSelfArgs->_subCLName = subCLName ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( CMD_RTNCOCMDUNLINKCL_PARSEMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDUNLINKCL_GENCATAMSG, "rtnCoordCMDUnlinkCollection::_generateCataMsg" )
   INT32 rtnCoordCMDUnlinkCollection::_generateCataMsg ( MsgHeader *pMsg,
                                                         pmdEDUCB *cb,
                                                         _rtnCMDArguments *pArgs,
                                                         CHAR **ppMsgBuf,
                                                         MsgHeader **ppCataMsg )
   {
      PD_TRACE_ENTRY ( CMD_RTNCOCMDUNLINKCL_GENCATAMSG ) ;

      pMsg->opCode = MSG_CAT_UNLINK_CL_REQ ;
      (*ppCataMsg) = pMsg ;

      PD_TRACE_EXIT ( CMD_RTNCOCMDUNLINKCL_GENCATAMSG ) ;

      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDUNLINKCL_GENROLLBACKMSG, "rtnCoordCMDUnlinkCollection::_generateRollbackDataMsg" )
   INT32 rtnCoordCMDUnlinkCollection::_generateRollbackDataMsg ( MsgHeader *pMsg,
                                                                 _rtnCMDArguments *pArgs,
                                                                 CHAR **ppMsgBuf,
                                                                 MsgHeader **ppRollbackMsg )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( CMD_RTNCOCMDUNLINKCL_GENROLLBACKMSG ) ;

      INT32 bufSize = 0 ;

      _rtnCMDUnlinkCLArgs *pSelfArgs = ( _rtnCMDUnlinkCLArgs * )pArgs ;

      // The Data Group doesn't care about lowBound and upBound
      rc = msgBuildLinkCLMsg( ppMsgBuf, &bufSize,
                              pSelfArgs->_targetName.c_str(),
                              pSelfArgs->_subCLName.c_str(),
                              NULL, NULL, 0 ) ;
      PD_RC_CHECK ( rc, PDWARNING,
                    "Failed to rollback %s on [%s/%s]: "
                    "failed to build link message, rc: %d",
                    _getCommandName(),
                    pSelfArgs->_targetName.c_str(),
                    pSelfArgs->_subCLName.c_str(),
                    rc ) ;

      (*ppRollbackMsg) = (MsgHeader *)(*ppMsgBuf) ;

   done :
      PD_TRACE_EXITRC ( CMD_RTNCOCMDUNLINKCL_GENROLLBACKMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDUNLINKCL_ROLLBACK, "rtnCoordCMDUnlinkCollection::_rollbackOnDataGroup" )
   INT32 rtnCoordCMDUnlinkCollection::_rollbackOnDataGroup ( MsgHeader *pMsg,
                                                             pmdEDUCB *cb,
                                                             _rtnCMDArguments *pArgs,
                                                             const CoordGroupList &groupLst )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( CMD_RTNCOCMDUNLINKCL_ROLLBACK ) ;

      rc = executeOnCL( pMsg, cb, pArgs->_targetName.c_str(),
                        TRUE, &groupLst, &(pArgs->_ignoreRCList), NULL,
                        NULL, pArgs->_pBuf ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to rollback %s on [%s], rc: %d",
                   _getCommandName(), pArgs->_targetName.c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC ( CMD_RTNCOCMDUNLINKCL_ROLLBACK, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
    * rtnCoordCMDSplit implement
    */
   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOCMDSP_GETCLCOUNT, "rtnCoordCMDSplit::_getCLCount" )
   INT32 rtnCoordCMDSplit::_getCLCount( const CHAR * clFullName,
                                       CoordGroupList & groupList,
                                       pmdEDUCB *cb,
                                       UINT64 & count,
                                       rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( SDB_RTNCOCMDSP_GETCLCOUNT ) ;

      pmdKRCB *pKRCB = pmdGetKRCB () ;
      SDB_RTNCB *pRtncb = pKRCB->getRTNCB() ;
      rtnContext *pContext = NULL ;
      count = 0 ;
      CoordGroupList tmpGroupList = groupList ;

      BSONObj collectionObj ;
      BSONObj dummy ;
      rtnContextBuf buffObj ;

      collectionObj = BSON( FIELD_NAME_COLLECTION << clFullName ) ;

      // send getcount to node
      rc = rtnCoordNodeQuery( CMD_ADMIN_PREFIX CMD_NAME_GET_COUNT,
                              dummy, dummy, dummy, collectionObj,
                              0, 1, tmpGroupList, cb, &pContext,
                              clFullName, 0, buf ) ;

      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to get count from source node, rc: %d",
                    rc ) ;

      rc = pContext->getMore( -1, buffObj, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR,
                 "Failed to get count from source node: get-more failed, rc: %d",
                 rc ) ;
         goto error ;
      }
      else
      {
         // get count data
         BSONObj countObj ( buffObj.data() ) ;
         BSONElement beTotal = countObj.getField( FIELD_NAME_TOTAL );
         PD_CHECK( beTotal.isNumber(), SDB_INVALIDARG, error,
                   PDERROR,
                   "Failed to get count from source node, "
                   "failed to get the field [%s]",
                   FIELD_NAME_TOTAL ) ;
         count = beTotal.numberLong() ;
      }

   done :
      if ( pContext )
      {
         SINT64 contextID = pContext->contextID() ;
         pRtncb->contextDelete ( contextID, cb ) ;
      }
      PD_TRACE_EXITRC ( SDB_RTNCOCMDSP_GETCLCOUNT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOCMDSP_EXE, "rtnCoordCMDSplit::execute" )
   INT32 rtnCoordCMDSplit::execute( MsgHeader *pMsg,
                                    pmdEDUCB *cb,
                                    INT64 &contextID,
                                    rtnContextBuf *buf )
   {
      INT32 rc                         = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNCOCMDSP_EXE ) ;
      pmdKRCB *pKRCB                   = pmdGetKRCB () ;
      SDB_RTNCB *pRtncb                = pKRCB->getRTNCB() ;
      CoordCB *pCoordcb                = pKRCB->getCoordCB () ;
      contextID                        = -1 ;

      CHAR *pCommandName               = NULL ;
      CHAR *pQuery                     = NULL ;

      CHAR szSource [ OSS_MAX_GROUPNAME_SIZE + 1 ] = {0} ;
      CHAR szTarget [ OSS_MAX_GROUPNAME_SIZE + 1 ] = {0} ;
      const CHAR *strName              = NULL ;
      CHAR *splitReadyBuffer           = NULL ;
      INT32 splitReadyBufferSz         = 0 ;
      CHAR *splitQueryBuffer           = NULL ;
      INT32 splitQueryBufferSz         = 0 ;
      MsgOpQuery *pSplitQuery          = NULL ;
      UINT64 taskID                    = CLS_INVALID_TASKID ;
      BOOLEAN async                    = FALSE ;
      BSONObj taskInfoObj ;

      BSONObj boShardingKey ;
      CoordCataInfoPtr cataInfo ;
      BSONObj boKeyStart ;
      BSONObj boKeyEnd ;
      FLOAT64 percent = 0.0 ;

      // first round we perform prepare, so catalog node is able to do sanity
      // check for collection name and nodes
      MsgOpQuery *pSplitReq            = (MsgOpQuery *)pMsg ;
      pSplitReq->header.opCode         = MSG_CAT_SPLIT_PREPARE_REQ ;

      CoordGroupList groupLst ;
      vector<BSONObj> boRecv ;
      CoordGroupList groupDstLst ;

      INT32 preferedType = 0 ;

      if ( cb->getCoordSession() &&
           PREFER_REPL_MASTER != cb->getCoordSession()->getPreferReplType() )
      {
         preferedType = cb->getCoordSession()->getPreferReplType() ;
         cb->getCoordSession()->setPreferReplType( PREFER_REPL_MASTER ) ;
      }

      /******************************************************************
       *              PREPARE PHASE                                     *
       ******************************************************************/
      // send request to catalog
      rc = executeOnCataGroup ( pMsg, cb, &groupLst, NULL, TRUE,
                                NULL, NULL, buf ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Split failed on catalog, rc: %d", rc ) ;

      // here, in groupLst there should be one and only one group, for SOURCE
      // send request to data-node to find the partitioning key
      // Extract the SplitQuery Field and build a query request to send to data
      // node
      rc = msgExtractQuery ( (CHAR*)pSplitReq, NULL, &pCommandName,
                             NULL, NULL, &pQuery,
                             NULL, NULL, NULL ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to extract query, rc: %d", rc ) ;
      try
      {
         /***************************************************************
          *             DO SOME VALIDATION HERE                         *
          ***************************************************************/
         BSONObj boQuery ( pQuery ) ;

         // get collection name and query
         BSONElement beName = boQuery.getField ( CAT_COLLECTION_NAME ) ;
         BSONElement beSplitQuery =
               boQuery.getField ( CAT_SPLITQUERY_NAME ) ;
         BSONElement beSplitEndQuery ;
         BSONElement beSource = boQuery.getField ( CAT_SOURCE_NAME ) ;
         BSONElement beTarget = boQuery.getField ( CAT_TARGET_NAME ) ;
         BSONElement beAsync  = boQuery.getField ( FIELD_NAME_ASYNC ) ;
         percent = boQuery.getField( CAT_SPLITPERCENT_NAME ).numberDouble() ;
         // collection name verify
         PD_CHECK ( !beName.eoo() && beName.type () == String,
                    SDB_INVALIDARG, error, PDERROR,
                    "Failed to process split prepare, unable to find "
                    "collection name field" ) ;
         // now strName is the name of collection
         strName = beName.valuestr() ;
         // get source group name
         PD_CHECK ( !beSource.eoo() && beSource.type() == String,
                    SDB_INVALIDARG, error, PDERROR,
                    "Unable to find source field" ) ;
         rc = catGroupNameValidate ( beSource.valuestr() ) ;
         PD_CHECK ( SDB_OK == rc, SDB_INVALIDARG, error, PDERROR,
                    "Source name is not valid: %s",
                    beSource.valuestr() ) ;
         ossStrncpy ( szSource, beSource.valuestr(), sizeof(szSource) ) ;

         // get target group name
         PD_CHECK ( !beTarget.eoo() && beTarget.type() == String,
                    SDB_INVALIDARG, error, PDERROR,
                    "Unable to find target field" ) ;
         rc = catGroupNameValidate ( beTarget.valuestr() ) ;
         PD_CHECK ( SDB_OK == rc, SDB_INVALIDARG, error, PDERROR,
                    "Target name is not valid: %s",
                    beTarget.valuestr() ) ;
         ossStrncpy ( szTarget, beTarget.valuestr(), sizeof(szTarget) ) ;

         // async check
         if ( Bool == beAsync.type() )
         {
            async = beAsync.Bool() ? TRUE : FALSE ;
         }
         else if ( !beAsync.eoo() )
         {
            PD_LOG( PDERROR, "Field[%s] type[%d] error", FIELD_NAME_ASYNC,
                    beAsync.type() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         // make sure we have either split value or split query
         if ( !beSplitQuery.eoo() )
         {
            PD_CHECK ( beSplitQuery.type() == Object,
                       SDB_INVALIDARG, error, PDERROR,
                       "Split is not defined or not valid" ) ;
            beSplitEndQuery = boQuery.getField ( CAT_SPLITENDQUERY_NAME ) ;
            if ( !beSplitEndQuery.eoo() )
            {
               PD_CHECK ( beSplitEndQuery.type() == Object,
                          SDB_INVALIDARG, error, PDERROR,
                          "Split is not defined or not valid" ) ;
            }
         }
         else
         {
            PD_CHECK( percent > 0.0 && percent <= 100.0,
                      SDB_INVALIDARG, error, PDERROR,
                      "Split percent value is error" ) ;
         }

         // get sharding key, always get the newest version from catalog
         rc = rtnCoordGetCataInfo ( cb, strName, TRUE, cataInfo ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to get cata info for collection %s, rc: %d",
                       strName, rc ) ;
         // sharding key must exist, we should NEVER hit this check because the
         // check already done in catalog in PREPARE phase
         cataInfo->getShardingKey ( boShardingKey ) ;
         PD_CHECK ( !boShardingKey.isEmpty(),
                    SDB_COLLECTION_NOTSHARD, error, PDWARNING,
                    "Collection must be sharded: %s",
                    strName ) ;

         /*********************************************************************
          *           GET THE SHARDING KEY VALUE FROM SOURCE                  *
          *********************************************************************/
         if ( cataInfo->getCatalogSet()->isHashSharding() )
         {
            if ( !beSplitQuery.eoo() )
            {
               BSONObj tmpStart = beSplitQuery.embeddedObject() ;
               BSONObjBuilder tmpStartBuilder ;
               tmpStartBuilder.appendElementsWithoutName( tmpStart ) ;
               boKeyStart = tmpStartBuilder.obj() ;
               if ( !beSplitEndQuery.eoo() )
               {
                  BSONObj tmpEnd = beSplitEndQuery.embeddedObject() ;
                  BSONObjBuilder tmpEndBuilder ;
                  tmpEndBuilder.appendElementsWithoutName( tmpEnd ) ;
                  boKeyEnd = tmpEndBuilder.obj() ;
               }
            }
         }
         else
         {
            if ( beSplitQuery.eoo())
            {
               rc = _getBoundByPercent( strName, percent, cataInfo,
                                        groupLst, cb, boKeyStart, boKeyEnd,
                                        buf ) ;
            }
            else
            {
               rc = _getBoundByCondition( strName,
                                          beSplitQuery.embeddedObject(),
                                          beSplitEndQuery.eoo() ?
                                          BSONObj():
                                          beSplitEndQuery.embeddedObject(),
                                          groupLst,
                                          cb,
                                          cataInfo,
                                          boKeyStart, boKeyEnd,
                                          buf ) ;
            }

            PD_RC_CHECK( rc, PDERROR, "Failed to get bound, rc: %d",
                         rc ) ;
         }
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception when query from remote node: %s",
                       e.what() ) ;
      }

      /************************************************************************
       *         SHARDING READY REQUEST                                       *
       ************************************************************************/
      // now boKeyStart contains the key we want to split, let's construct a new
      // request for split ready
      try
      {
         BSONObj boSend ;

         // construct the record that we are going to send to catalog
         boSend = BSON ( CAT_COLLECTION_NAME << strName <<
                         CAT_SOURCE_NAME << szSource <<
                         CAT_TARGET_NAME << szTarget <<
                         CAT_SPLITPERCENT_NAME << percent <<
                         CAT_SPLITVALUE_NAME << boKeyStart <<
                         CAT_SPLITENDVALUE_NAME << boKeyEnd <<
                         CAT_TASKID_NAME << (long long)taskID ) ;
         taskInfoObj = boSend ;
         rc = msgBuildQueryMsg ( &splitReadyBuffer, &splitReadyBufferSz,
                                 CMD_ADMIN_PREFIX CMD_NAME_SPLIT, 0,
                                 0, 0, -1, &boSend, NULL,
                                 NULL, NULL ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to build query message, rc: %d",
                       rc ) ;
         pSplitReq                        = (MsgOpQuery *)splitReadyBuffer ;
         pSplitReq->header.opCode         = MSG_CAT_SPLIT_READY_REQ ;
         pSplitReq->version               = cataInfo->getVersion() ;

         rc = executeOnCataGroup ( (MsgHeader*)pSplitReq, cb, &groupDstLst,
                                   &boRecv, TRUE, NULL, NULL, buf ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to execute split ready on catalog, rc: %d",
                       rc ) ;

         // Get task ID
         if ( boRecv.empty() )
         {
            PD_LOG( PDERROR, "Failed to get task id from result msg" ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         taskID = (UINT64)(boRecv.at(0).getField( CAT_TASKID_NAME ).numberLong()) ;

         // Construct split query request to destination Data group
         boSend = BSON( CAT_TASKID_NAME << (long long)taskID ) ;
         rc = msgBuildQueryMsg( &splitQueryBuffer, &splitQueryBufferSz,
                                CMD_ADMIN_PREFIX CMD_NAME_SPLIT, 0,
                                0, 0, -1, &boSend, NULL,
                                NULL, NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to build query message, rc: %d",
                      rc ) ;
         pSplitQuery                      = (MsgOpQuery *)splitQueryBuffer ;
         pSplitQuery->version             = cataInfo->getVersion() ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception when building split ready message: %s",
                       e.what() ) ;
      }
      /************************************************************************
       *           SHARDING START REQUEST                                     *
       ************************************************************************/
      // before sending to data node, we have to convert the request to QUERY
      pSplitReq->header.opCode = MSG_BS_QUERY_REQ ;
      rc = executeOnCL( (MsgHeader *)splitReadyBuffer, cb, strName,
                        FALSE, &groupDstLst, NULL, NULL, NULL, buf ) ;
      // If we get error here, something big happend. We have marked ready to
      // split on catalog but data node refused to do so.
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to execute split on data node, rc: %d",
                   rc ) ;

      // if sync, need to wait task finished
      if ( !async )
      {
         rtnCoordProcesserFactory *pFactory = pCoordcb->getProcesserFactory() ;
         rtnCoordCommand *pCmd = pFactory->getCommandProcesser(
                                 COORD_CMD_WAITTASK ) ;
         SDB_ASSERT( pCmd, "wait task command not found" ) ;
         rc = pCmd->execute( (MsgHeader*)splitQueryBuffer, cb,
                             contextID, buf ) ;
         if ( rc )
         {
            PD_LOG( PDWARNING,
                    "Wait task [%lld] failed, rc: %d",
                    taskID, rc ) ;
            rc = SDB_OK ;
            /// can not report error, because split already created
         }
      }
      else // return taskid to client
      {
         rtnContextDump *pContext = NULL ;
         rc = pRtncb->contextNew( RTN_CONTEXT_DUMP, (rtnContext**)&pContext,
                                  contextID, cb ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create context, rc: %d", rc ) ;
         rc = pContext->open( BSONObj(), BSONObj(), 1, 0 ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to open context, rc: %d", rc ) ;
         pContext->append( BSON( CAT_TASKID_NAME << (long long)taskID ) ) ;
      }

   done :
      if ( 0 != preferedType && cb->getCoordSession() )
      {
         cb->getCoordSession()->setPreferReplType( preferedType ) ;
      }
      if ( pCommandName && strName )
      {
         PD_AUDIT_COMMAND( AUDIT_DDL, pCommandName + 1, AUDIT_OBJ_CL,
                           strName, rc, "Option:%s, TaskID:%llu",
                           taskInfoObj.toString().c_str(), taskID ) ;
      }
      SAFE_OSS_FREE( splitReadyBuffer ) ;
      SAFE_OSS_FREE( splitQueryBuffer ) ;
      PD_TRACE_EXITRC ( SDB_RTNCOCMDSP_EXE, rc ) ;
      return rc ;
   error :
      if ( CLS_INVALID_TASKID != taskID )
      {
         // Need to delete catContext allocated with taskID, send the cancel
         // request anyway
         INT32 tmpRC = SDB_OK ;
         if ( !pSplitQuery )
         {
            // Construct the cancel request with task ID
            BSONObj boCancel = BSON( CAT_TASKID_NAME << (long long)taskID ) ;
            tmpRC = msgBuildQueryMsg( &splitQueryBuffer, &splitQueryBufferSz,
                                      CMD_ADMIN_PREFIX CMD_NAME_SPLIT, 0,
                                      0, 0, -1, &boCancel, NULL,
                                      NULL, NULL ) ;
            if ( SDB_OK == tmpRC )
            {
               pSplitQuery = (MsgOpQuery *)splitQueryBuffer ;
            }
            else
            {
               PD_LOG( PDWARNING,
                       "Failed to execute split cancel on catalog, rc: %d",
                       tmpRC ) ;
            }
         }
         if ( pSplitQuery )
         {
            // Send the request
            pSplitQuery->header.opCode = MSG_CAT_SPLIT_CANCEL_REQ ;
            tmpRC = executeOnCataGroup ( (MsgHeader*)pSplitQuery,
                                         cb, TRUE, NULL, NULL, buf ) ;
            if ( tmpRC )
            {
               PD_LOG( PDWARNING,
                       "Failed to execute split cancel on catalog, rc: %d",
                       tmpRC ) ;
            }
         }
      }
      if ( SDB_RTN_INVALID_HINT == rc )
      {
         rc = SDB_COORD_SPLIT_NO_SHDIDX ;
      }
      if ( -1 != contextID )
      {
         pRtncb->contextDelete( contextID, cb ) ;
         contextID = -1 ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOCMDSP_GETBOUNDRONDATA, "rtnCoordCMDSplit::_getBoundRecordOnData" )
   INT32 rtnCoordCMDSplit::_getBoundRecordOnData( const CHAR *cl,
                                                  const BSONObj &condition,
                                                  const BSONObj &hint,
                                                  const BSONObj &sort,
                                                  INT32 flag,
                                                  INT64 skip,
                                                  CoordGroupList &groupList,
                                                  pmdEDUCB *cb,
                                                  BSONObj &shardingKey,
                                                  BSONObj &record,
                                                  rtnContextBuf *buf )
   {
      PD_TRACE_ENTRY( SDB_RTNCOCMDSP_GETBOUNDRONDATA ) ;
      INT32 rc = SDB_OK ;
      BSONObj empty ;
      rtnContext *context = NULL ;
      rtnContextBuf buffObj ;
      BSONObj obj ;

      // check condition has invalid fileds
      if ( !condition.okForStorage() )
      {
         PD_LOG( PDERROR,
                 "Condition [%s] has invalid field name",
                 condition.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( condition.isEmpty() )
      {
         rc = rtnCoordNodeQuery( cl, condition, empty, sort,
                                 hint, skip, 1, groupList,
                                 cb, &context, NULL, flag, buf ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to query from data group, rc: %d",
                       rc ) ;
         rc = context->getMore( -1, buffObj, cb ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         else
         {
            obj = BSONObj( buffObj.data() ) ;
         }
      }
      else
      {
         obj = condition ;
      }

      // product split key
      {
         PD_LOG ( PDINFO,
                  "Split found record %s",
                  obj.toString().c_str() ) ;
         // we need to compare with boShardingKey and extract the partition key
         ixmIndexKeyGen keyGen ( shardingKey ) ;
         BSONObjSet keys ;
         BSONObjSet::iterator keyIter ;
         rc = keyGen.getKeys ( obj, keys, NULL, TRUE ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to extract keys\nkeyDef: %s\n"
                       "record: %s\nrc: %d", shardingKey.toString().c_str(),
                       obj.toString().c_str(), rc ) ;
         // make sure there is one and only one element in the keys
         PD_CHECK ( keys.size() == 1,
                    SDB_INVALID_SHARDINGKEY, error, PDWARNING,
                    "There must be a single key generate for "
                    "sharding\nkeyDef = %s\nrecord = %s\n",
                    shardingKey.toString().c_str(),
                    obj.toString().c_str() ) ;

         keyIter = keys.begin () ;
         record = (*keyIter).copy() ;

         // validate key does not contains Undefined
         /*{
            BSONObjIterator iter ( record ) ;
            while ( iter.more () )
            {
               BSONElement e = iter.next () ;
               PD_CHECK ( e.type() != Undefined, SDB_CLS_BAD_SPLIT_KEY,
                          error, PDERROR, "The split record does not contains "
                          "a valid key\nRecord: %s\nShardingKey: %s\n"
                          "SplitKey: %s", obj.toString().c_str(),
                          shardingKey.toString().c_str(),
                          record.toString().c_str() ) ;
            }
         }*/

        PD_LOG ( PDINFO, "Split found key %s", record.toString().c_str() ) ;
     }

   done:
      if ( NULL != context )
      {
         SINT64 contextID = context->contextID() ;
         pmdGetKRCB()->getRTNCB()->contextDelete( contextID, cb ) ;
      }
      PD_TRACE_EXITRC( SDB_RTNCOCMDSP_GETBOUNDRONDATA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOCMDSP__GETBOUNDBYC, "rtnCoordCMDSplit::_getBoundByCondition" )
   INT32 rtnCoordCMDSplit::_getBoundByCondition( const CHAR *cl,
                                                 const BSONObj &begin,
                                                 const BSONObj &end,
                                                 CoordGroupList &groupList,
                                                 pmdEDUCB *cb,
                                                 CoordCataInfoPtr &cataInfo,
                                                 BSONObj &lowBound,
                                                 BSONObj &upBound,
                                                 rtnContextBuf *buf )
   {
      PD_TRACE_ENTRY( SDB_RTNCOCMDSP__GETBOUNDBYC ) ;
      INT32 rc = SDB_OK ;
      /// coord send will clear group list.
      CoordGroupList grpTmp = groupList ;
      BSONObj shardingKey ;
      cataInfo->getShardingKey( shardingKey ) ;
      PD_CHECK ( !shardingKey.isEmpty(),
                 SDB_COLLECTION_NOTSHARD, error, PDWARNING,
                 "Collection must be sharded: %s",
                 cl ) ;

      rc = _getBoundRecordOnData( cl, begin, BSONObj(),BSONObj(),
                                  0, 0, grpTmp, cb,
                                  shardingKey, lowBound, buf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR,
                 "Failed to get begin bound, rc: %d",
                 rc ) ;
         goto error ;
      }

      if ( !end.isEmpty() )
      {
         grpTmp = groupList ;
         rc = _getBoundRecordOnData( cl, end, BSONObj(),BSONObj(),
                                     0, 0, grpTmp, cb,
                                     shardingKey, upBound, buf ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR,
                    "Failed to get end bound, rc: %d",
                    rc ) ;
            goto error ;
         }
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOCMDSP__GETBOUNDBYC, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCOCMDSP__GETBOUNDBYP, "rtnCoordCMDSplit::_getBoundByPercent" )
   INT32 rtnCoordCMDSplit::_getBoundByPercent( const CHAR *cl,
                                               FLOAT64 percent,
                                               CoordCataInfoPtr &cataInfo,
                                               CoordGroupList &groupList,
                                               pmdEDUCB *cb,
                                               BSONObj &lowBound,
                                               BSONObj &upBound,
                                               rtnContextBuf *buf )
   {
      PD_TRACE_ENTRY( SDB_RTNCOCMDSP__GETBOUNDBYP ) ;
      INT32 rc = SDB_OK ;
      BSONObj shardingKey ;
      cataInfo->getShardingKey ( shardingKey ) ;
      CoordGroupList grpTmp = groupList ;
      PD_CHECK ( !shardingKey.isEmpty(),
                 SDB_COLLECTION_NOTSHARD, error, PDWARNING,
                 "Collection must be sharded: %s",
                 cl ) ;

      // if split percent is 100.0%, get the group low bound
      if ( 100.0 - percent < OSS_EPSILON )
      {
         rc = cataInfo->getGroupLowBound( grpTmp.begin()->second,
                                          lowBound ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to get group [%d] low bound, rc: %d",
                      grpTmp.begin()->second, rc ) ;
      }
      else
      {
         UINT64 totalCount = 0 ;
         INT64 skipCount = 0 ;
         INT32 flag = 0 ;
         BSONObj hint ;
         while ( TRUE )
         {
            rc = _getCLCount( cl, grpTmp, cb, totalCount, buf ) ;
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to get collection count, rc: %d",
                         rc ) ;
            if ( 0 == totalCount )
            {
               rc = SDB_DMS_EMPTY_COLLECTION ;
               PD_LOG( PDDEBUG, "collection [%s] is empty", cl ) ;
               break ;
            }

            skipCount = (INT64)(totalCount * ( ( 100 - percent ) / 100 ) ) ;
            hint = BSON( "" << "" ) ;
            // Must use sorted index for query
            flag = FLG_QUERY_WITHOUT_SORT ;

            /// sort by shardingKey that if $shard index does not exist
            /// can still match index.
            rc = _getBoundRecordOnData( cl, BSONObj(), hint, shardingKey,
                                        flag, skipCount, grpTmp,
                                        cb, shardingKey, lowBound, buf ) ;
            if ( SDB_DMS_EOC == rc )
            {
               continue ;
            }
            else if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR,
                       "Failed to get bound from data, rc: %d",
                       rc ) ;
               goto error ;
            }
            else
            {
               break ;
            }
         }
      }

      /// upbound always be empty.
      upBound = BSONObj() ;
   done:
      PD_TRACE_EXITRC( SDB_RTNCOCMDSP__GETBOUNDBYP, rc ) ;
      return rc ;
   error:
      goto done ;
   }


   /*
    * rtnCoordCMDCreateIndex implement
    */
   rtnCoordCMD2Phase::_rtnCMDArguments *rtnCoordCMDCreateIndex::_generateArguments ()
   {
      return SDB_OSS_NEW _rtnCMDCreateIndexArgs () ;
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDCREATEIDX_PARSEMSG, "rtnCoordCMDCreateIndex::_parseMsg" )
   INT32 rtnCoordCMDCreateIndex::_parseMsg ( MsgHeader *pMsg,
                                             _rtnCMDArguments *pArgs )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( CMD_RTNCOCMDCREATEIDX_PARSEMSG ) ;

      _rtnCMDCreateIndexArgs *pSelfArgs = ( _rtnCMDCreateIndexArgs * )pArgs ;

      try
      {
         string clName, idxName ;
         BSONObj boIndex ;

         rc = rtnGetSTDStringElement( pSelfArgs->_boQuery, CAT_COLLECTION,
                                      clName ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to %s: failed to get the field [%s] from query",
                      _getCommandName(), CAT_COLLECTION ) ;
         PD_CHECK( !clName.empty(),
                   SDB_INVALIDARG, error, PDERROR,
                   "Failed to %s: collection name can't be empty!",
                   _getCommandName() ) ;

         rc = rtnGetObjElement( pSelfArgs->_boQuery, FIELD_NAME_INDEX, boIndex ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to %s: failed to get the field [%s] from query",
                      _getCommandName(), FIELD_NAME_INDEX ) ;

         // get embedded index name
         rc = rtnGetSTDStringElement( boIndex, IXM_FIELD_NAME_NAME, idxName );
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to %s: failed to get the field [%s] from query",
                       _getCommandName(), IXM_FIELD_NAME_NAME ) ;
         PD_CHECK( !idxName.empty(),
                   SDB_INVALIDARG, error, PDERROR,
                   "Failed to %s: index name can't be empty!",
                   _getCommandName() ) ;

         pSelfArgs->_targetName = clName ;
         pSelfArgs->_indexName = idxName ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( CMD_RTNCOCMDCREATEIDX_PARSEMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDCREATEIDX_GENCATAMSG, "rtnCoordCMDCreateIndex::_generateCataMsg" )
   INT32 rtnCoordCMDCreateIndex::_generateCataMsg( MsgHeader *pMsg,
                                                   pmdEDUCB *cb,
                                                   _rtnCMDArguments *pArgs,
                                                   CHAR **ppMsgBuf,
                                                   MsgHeader **ppCataMsg )
   {
      PD_TRACE_ENTRY ( CMD_RTNCOCMDCREATEIDX_GENCATAMSG ) ;

      pMsg->opCode = MSG_CAT_CREATE_IDX_REQ ;
      (*ppCataMsg) = pMsg ;

      PD_TRACE_EXIT ( CMD_RTNCOCMDCREATEIDX_GENCATAMSG ) ;

      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDCREATEIDX_GENROLLBACKMSG, "rtnCoordCMDCreateIndex::_generateRollbackDataMsg" )
   INT32 rtnCoordCMDCreateIndex::_generateRollbackDataMsg ( MsgHeader *pMsg,
                                                            _rtnCMDArguments *pArgs,
                                                            CHAR **ppMsgBuf,
                                                            MsgHeader **ppRollbackMsg )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( CMD_RTNCOCMDCREATEIDX_GENROLLBACKMSG ) ;

      INT32 bufSize = 0 ;

      _rtnCMDCreateIndexArgs *pSelfArgs = ( _rtnCMDCreateIndexArgs * ) pArgs ;

      rc = msgBuildDropIndexMsg( ppMsgBuf, &bufSize,
                                 pSelfArgs->_targetName.c_str(),
                                 pSelfArgs->_indexName.c_str(), 0 ) ;
      PD_RC_CHECK ( rc, PDWARNING,
                    "Failed to rollback %s on [%s]: "
                    "failed to build drop index message, rc: %d",
                    _getCommandName(), pSelfArgs->_targetName.c_str(), rc ) ;

      (*ppRollbackMsg) = (MsgHeader *)(*ppMsgBuf) ;

   done :
      PD_TRACE_EXITRC ( CMD_RTNCOCMDCREATEIDX_GENROLLBACKMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDCREATEIDX_ROLLBACK, "rtnCoordCMDCreateIndex::_rollbackOnDataGroup" )
   INT32 rtnCoordCMDCreateIndex::_rollbackOnDataGroup ( MsgHeader *pMsg,
                                                        pmdEDUCB *cb,
                                                        _rtnCMDArguments *pArgs,
                                                        const CoordGroupList &groupLst )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( CMD_RTNCOCMDCREATEIDX_ROLLBACK ) ;

      CoordCataInfoPtr cataInfo ;
      SET_RC ignoreRC ;

      // let's get most current version again
      rc = rtnCoordGetCataInfo( cb, pArgs->_targetName.c_str(), FALSE, cataInfo ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to rollback %s on [%s]: "
                   "catalog info for collection[%s] failed, rc: %d)",
                   _getCommandName(), pArgs->_targetName.c_str(),
                   pArgs->_targetName.c_str(), rc ) ;
      PD_CHECK( !cataInfo->isMainCL(),
                SDB_OK, error, PDWARNING,
                "Failed to rollback %s on [%s]:"
                "main-collection create index failed and will not rollback",
                _getCommandName(), pArgs->_targetName.c_str() ) ;

      ignoreRC.insert( SDB_IXM_NOTEXIST ) ;
      rc = executeOnCL( pMsg, cb, pArgs->_targetName.c_str(),
                        FALSE, &groupLst, &ignoreRC, NULL,
                        NULL, pArgs->_pBuf ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to rollback %s on [%s], rc: %d",
                   _getCommandName(), pArgs->_targetName.c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC ( CMD_RTNCOCMDCREATEIDX_ROLLBACK, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   /*
    * rtnCoordCMDDropIndex define
    */
   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDDROPIDX_PARSEMSG, "rtnCoordCMDDropIndex::_parseMsg" )
   INT32 rtnCoordCMDDropIndex::_parseMsg ( MsgHeader *pMsg,
                                           _rtnCMDArguments *pArgs )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( CMD_RTNCOCMDDROPIDX_PARSEMSG ) ;

      try
      {
         string clName ;

         rc = rtnGetSTDStringElement( pArgs->_boQuery, CAT_COLLECTION,
                                      clName ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to %s: failed to get the field [%s] from query",
                      _getCommandName(), CAT_COLLECTION ) ;
         PD_CHECK( !clName.empty(),
                   SDB_INVALIDARG, error, PDERROR,
                   "Failed to %s: collection name can't be empty!",
                   _getCommandName() ) ;

         pArgs->_targetName = clName ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG;
         goto error ;
      }

   done :
      PD_TRACE_EXITRC ( CMD_RTNCOCMDDROPIDX_PARSEMSG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( CMD_RTNCOCMDDROPIDX_GENCATAMSG, "rtnCoordCMDDropIndex::_generateCataMsg" )
   INT32 rtnCoordCMDDropIndex::_generateCataMsg( MsgHeader *pMsg,
                                                 pmdEDUCB *cb,
                                                 _rtnCMDArguments *pArgs,
                                                 CHAR **ppMsgBuf,
                                                 MsgHeader **ppCataMsg )
   {
      PD_TRACE_ENTRY ( CMD_RTNCOCMDDROPIDX_GENCATAMSG ) ;

      pMsg->opCode = MSG_CAT_DROP_IDX_REQ ;
      (*ppCataMsg) = pMsg ;

      PD_TRACE_EXIT ( CMD_RTNCOCMDDROPIDX_GENCATAMSG ) ;

      return SDB_OK ;
   }
}
