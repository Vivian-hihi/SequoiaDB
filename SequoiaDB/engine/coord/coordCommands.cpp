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

   Source File Name = coordCommands.cpp

   Descriptive Name = Coord Commands

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   user command processing on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "coordCommands.hpp"
#include "pmd.hpp"
#include "rtnCB.hpp"
#include "pmdOptions.h"
#include "utilCommon.hpp"
#include "coordQueryOperator.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

using namespace bson ;

namespace engine
{

   /*
      _coordCMDTestCollectionSpace implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDTestCollectionSpace,
                                      CMD_NAME_TEST_COLLECTIONSPACE,
                                      TRUE ) ;
   _coordCMDTestCollectionSpace::_coordCMDTestCollectionSpace()
   {
   }

   _coordCMDTestCollectionSpace::~_coordCMDTestCollectionSpace()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_CMD_TESTCS_EXE, "_coordCMDTestCollectionSpace::execute" )
   INT32 _coordCMDTestCollectionSpace::execute( MsgHeader *pMsg,
                                                pmdEDUCB *cb,
                                                INT64 &contextID,
                                                rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_CMD_TESTCS_EXE ) ;
      SDB_RTNCB *pRtncb = pmdGetKRCB()->getRTNCB() ;
      coordCommandFactory *pFactory = coordGetFactory() ;
      coordOperator *pOperator = NULL ;
      rtnContextBuf buffObj ;

      contextID = -1 ;

      rc = pFactory->create( CMD_NAME_LIST_COLLECTIONSPACES, pOperator ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Create operator by name[%s] failed, rc: %d",
                 CMD_NAME_LIST_COLLECTIONSPACES, rc ) ;
         goto error ;
      }
      rc = pOperator->init( _pResource, cb, getTimeout() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init operator failed[%s], rc: %d",
                 pOperator->getName(), rc ) ;
         goto error ;
      }
      rc = pOperator->execute( pMsg, cb, contextID, buf ) ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Execute operator[%s] failed, rc: %d",
                  pOperator->getName(), rc ) ;
         goto error ;
      }

      /// get more
      rc = rtnGetMore( contextID, -1, buffObj, cb, pRtncb ) ;
      if ( rc )
      {
         contextID = -1 ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_DMS_CS_NOTEXIST ;
         }
         else
         {
            PD_LOG ( PDERROR, "getmore failed, rc: %d", rc ) ;
         }
      }

   done:
      if ( contextID >= 0 )
      {
         pRtncb->contextDelete( contextID, cb ) ;
         contextID = -1 ;
      }
      if ( pOperator )
      {
         pFactory->release( pOperator ) ;
      }
      PD_TRACE_EXITRC ( COORD_CMD_TESTCS_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDTestCollection implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDTestCollection,
                                      CMD_NAME_TEST_COLLECTION,
                                      TRUE ) ;
   _coordCMDTestCollection::_coordCMDTestCollection()
   {
   }

   _coordCMDTestCollection::~_coordCMDTestCollection()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_CMD_TESTCL_EXE, "_coordCMDTestCollection::execute" )
   INT32 _coordCMDTestCollection::execute( MsgHeader *pMsg,
                                           pmdEDUCB *cb,
                                           INT64 &contextID,
                                           rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_CMD_TESTCL_EXE ) ;
      SDB_RTNCB *pRtncb = pmdGetKRCB()->getRTNCB() ;
      coordCommandFactory *pFactory = coordGetFactory() ;
      coordOperator *pOperator = NULL ;
      rtnContextBuf buffObj ;

      contextID                        = -1 ;

      rc = pFactory->create( CMD_NAME_LIST_COLLECTIONS, pOperator ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Create operator by name[%s] failed, rc: %d",
                 CMD_NAME_LIST_COLLECTIONS, rc ) ;
         goto error ;
      }
      rc = pOperator->init( _pResource, cb, getTimeout() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Init operator failed[%s], rc: %d",
                 pOperator->getName(), rc ) ;
         goto error ;
      }
      rc = pOperator->execute( pMsg, cb, contextID, buf ) ;
      if ( rc != SDB_OK )
      {
         PD_LOG ( PDERROR, "Execute operator[%s] failed, rc: %d",
                  pOperator->getName(), rc ) ;
         goto error ;
      }

      rc = rtnGetMore( contextID, -1, buffObj, cb, pRtncb ) ;
      if ( rc )
      {
         contextID = -1 ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_DMS_NOTEXIST ;
         }
         else
         {
            PD_LOG ( PDERROR, "Getmore failed, rc: %d", rc ) ;
         }
      }

   done:
      if ( contextID >= 0 )
      {
         pRtncb->contextDelete( contextID, cb ) ;
         contextID = -1 ;
      }
      if ( pOperator )
      {
         pFactory->release( pOperator ) ;
      }
      PD_TRACE_EXITRC ( COORD_CMD_TESTCL_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCmdWaitTask implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCmdWaitTask,
                                      CMD_NAME_WAITTASK,
                                      TRUE ) ;
   _coordCmdWaitTask::_coordCmdWaitTask()
   {
   }

   _coordCmdWaitTask::~_coordCmdWaitTask()
   {
   }

   INT32 _coordCmdWaitTask::execute( MsgHeader *pMsg,
                                     pmdEDUCB *cb,
                                     INT64 &contextID,
                                     rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      SET_RC ignoreRC ;
      rtnContextCoord *pContext        = NULL ;
      rtnContextBuf buffObj ;
      pmdKRCB *pKRCB                   = pmdGetKRCB() ;
      contextID                        = -1 ;
      pMsg->opCode                     = MSG_CAT_QUERY_TASK_REQ ;
      pMsg->TID                        = cb->getTID() ;

      ignoreRC.insert( SDB_DMS_EOC ) ;
      ignoreRC.insert( SDB_CAT_TASK_NOTFOUND ) ;

      while ( TRUE )
      {
         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         rc = executeOnCataGroup( pMsg, cb, TRUE, &ignoreRC, &pContext, buf ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Query task on catalog failed, rc: %d", rc ) ;
            goto error ;
         }
         rc = pContext->getMore( -1, buffObj, cb ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Get more failed, rc: %d", rc ) ;
            goto error ;
         }

         pKRCB->getRTNCB()->contextDelete( pContext->contextID(), cb ) ;
         pContext = NULL ;
         ossSleep( OSS_ONE_SEC ) ;
      }

   done:
      if ( pContext )
      {
         pKRCB->getRTNCB()->contextDelete( pContext->contextID(),  cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCmdCancelTask implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCmdCancelTask,
                                      CMD_NAME_CANCEL_TASK,
                                      TRUE ) ;
   _coordCmdCancelTask::_coordCmdCancelTask()
   {
   }

   _coordCmdCancelTask::~_coordCmdCancelTask()
   {
   }

   INT32 _coordCmdCancelTask::execute( MsgHeader *pMsg,
                                       pmdEDUCB *cb,
                                       INT64 &contextID,
                                       rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      coordCommandFactory *pFactory    = NULL ;
      coordOperator *pOperator         = NULL ;
      BOOLEAN async                    = FALSE ;

      contextID                        = -1 ;

      CoordGroupList groupLst ;
      INT32 rcTmp = SDB_OK ;

      // extract msg
      CHAR *pQueryBuf = NULL ;
      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL, NULL, NULL, &pQueryBuf,
                            NULL, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to extract query msg, rc: %d", rc ) ;

      try
      {
         BSONObj matcher( pQueryBuf ) ;
         rc = rtnGetBooleanElement( matcher, FIELD_NAME_ASYNC, async ) ;
         if ( SDB_FIELD_NOT_EXIST == rc )
         {
            rc = SDB_OK ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to get field[%s], rc: %d",
                      FIELD_NAME_ASYNC, rc ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      pMsg->opCode                     = MSG_CAT_SPLIT_CANCEL_REQ ;

      rc = executeOnCataGroup( pMsg, cb, &groupLst, NULL, TRUE,
                               NULL, buf ) ;
      PD_RC_CHECK( rc, PDERROR, "Excute on catalog failed, rc: %d", rc ) ;

      pMsg->opCode                     = MSG_BS_QUERY_REQ ;
      // notify to data node
      rcTmp = executeOnDataGroup( pMsg, cb, groupLst,
                                  TRUE, NULL, NULL, NULL,
                                  buf ) ;
      if ( rcTmp )
      {
         PD_LOG( PDWARNING, "Failed to notify to data node, rc: %d", rcTmp ) ;
      }

      // if sync
      if ( !async )
      {
         pFactory = coordGetFactory() ;
         rc = pFactory->create( CMD_NAME_WAITTASK, pOperator ) ;
         PD_RC_CHECK( rc, PDERROR, "Create operator by name[%s] failed, rc: %d",
                      CMD_NAME_WAITTASK, rc ) ;
         rc = pOperator->init( _pResource, cb, getTimeout() ) ;
         PD_RC_CHECK( rc, PDERROR, "Init operator[%s] failed, rc: %d",
                      pOperator->getName(), rc ) ;
         rc = pOperator->execute( pMsg, cb, contextID, buf ) ;
         if ( rc )
         {
            goto error ;
         }
      }

   done:
      if ( pOperator )
      {
         pFactory->release( pOperator ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDSetSessionAttr implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDSetSessionAttr,
                                      CMD_NAME_SETSESS_ATTR,
                                      TRUE ) ;
   _coordCMDSetSessionAttr::_coordCMDSetSessionAttr()
   {
   }

   _coordCMDSetSessionAttr::~_coordCMDSetSessionAttr()
   {
   }

   //PD_TRACE_DECLARE_FUNCTION( COORD_SETSESSIONATTR_EXE, "_coordCMDSetSessionAttr::execute" )
   INT32 _coordCMDSetSessionAttr::execute( MsgHeader *pMsg,
                                           pmdEDUCB *cb,
                                           INT64 &contextID,
                                           rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_SETSESSIONATTR_EXE ) ;
      coordSessionPropSite *pPropSite = NULL ;
      pmdRemoteSessionSite *pSite = NULL ;
      // fill default-reply(delete success)
      contextID = -1 ;

      CHAR *pQuery                     = NULL ;
      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL, NULL, NULL,
                            &pQuery, NULL, NULL, NULL );
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to parse unlink collection request(rc=%d)",
                   rc ) ;

      pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;
      if ( pSite )
      {
         pPropSite = ( coordSessionPropSite* )pSite->getUserData() ;
      }
      if ( !pPropSite )
      {
         PD_LOG( PDERROR, "Session's prop site is NULL" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      try
      {
         BSONObj boQuery ;
         BSONElement bePreferRepl ;
         INT32 sessReplType = PREFER_REPL_TYPE_MIN ;

         boQuery = BSONObj( pQuery );
         bePreferRepl = boQuery.getField( FIELD_NAME_PREFERED_INSTANCE );
         PD_CHECK( bePreferRepl.type() == NumberInt, SDB_INVALIDARG, error,
                   PDERROR, "Failed to set session attribute, failed to get "
                   "the field[%s]", FIELD_NAME_PREFERED_INSTANCE );
         sessReplType = bePreferRepl.Int();
         PD_CHECK( sessReplType > PREFER_REPL_TYPE_MIN &&
                   sessReplType < PREFER_REPL_TYPE_MAX,
                   SDB_INVALIDARG, error, PDERROR,
                   "Failed to set preferedInstanace, invalid value[%d], "
                   "Value range:(%d~%d)", sessReplType,
                   PREFER_REPL_TYPE_MIN, PREFER_REPL_TYPE_MAX ) ;

         /// set and clear last nodes
         pPropSite->setPreferInsType( sessReplType ) ;
         pPropSite->clear() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Failed to set sessionAttr, received unexpected "
                 "error:%s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( COORD_SETSESSIONATTR_EXE, rc ) ;
      return rc;
   error:
      goto done;
   }

   /*
      _coordCMDTruncate implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDTruncate,
                                      CMD_NAME_TRUNCATE,
                                      FALSE ) ;
   _coordCMDTruncate::_coordCMDTruncate()
   {
   }

   _coordCMDTruncate::~_coordCMDTruncate()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION( COORD_TRUNCATE_EXE, "_coordCMDTruncate::execute" )
   INT32 _coordCMDTruncate::execute( MsgHeader *pMsg,
                                     pmdEDUCB *cb,
                                     INT64 &contextID,
                                     rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( COORD_TRUNCATE_EXE ) ;
      CHAR *option = NULL;
      BSONObj boQuery ;
      const CHAR *fullName = NULL ;
      rc = msgExtractQuery( ( CHAR * )pMsg, NULL, NULL,
                            NULL, NULL, &option, NULL,
                            NULL, NULL );
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract msg:%d", rc ) ;
         goto error ;
      }

      try
      {
         boQuery = BSONObj( option );
         BSONElement e = boQuery.getField( FIELD_NAME_COLLECTION );
         if ( String != e.type() )
         {
            PD_LOG( PDERROR, "invalid truncate msg:%s",
                    boQuery.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         fullName = e.valuestr() ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error;
      }

      rc = executeOnCL( pMsg, cb, fullName, FALSE, NULL, NULL,
                        NULL, NULL, buf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to truncate cl:%s, rc:%d",
                 fullName, rc ) ;
         goto error ;
      }
   done:
      if ( fullName )
      {
         PD_AUDIT_COMMAND( AUDIT_DDL, CMD_NAME_TRUNCATE, AUDIT_OBJ_CL,
                           fullName, rc, "" ) ;
      }
      PD_TRACE_EXITRC( COORD_TRUNCATE_EXE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

}

