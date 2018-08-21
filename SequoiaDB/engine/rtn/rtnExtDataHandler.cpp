/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = rtnExtDataHandler.cpp

   Descriptive Name = External data process handler for rtn.

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          14/04/2017  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#include "rtn.hpp"
#include "rtnTrace.hpp"
#include "rtnExtDataHandler.hpp"
#include "../bson/lib/md5.hpp"

#define RTN_TEXTIDX_MAX_NUM        64

namespace engine
{
   _rtnExtDataHandler::_rtnExtDataHandler( rtnExtDataProcessorMgr *edpMgr )
   {
      _edpMgr = edpMgr ;
   }

   _rtnExtDataHandler::~_rtnExtDataHandler()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_GETEXTDATANAME, "_rtnExtDataHandler::getExtDataName" )
   INT32 _rtnExtDataHandler::getExtDataName( utilCLUniqueID clUniqID,
                                             const CHAR *idxName,
                                             CHAR *extName,
                                             UINT32 buffSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_GETEXTDATANAME ) ;
      ostringstream name ;

      SDB_ASSERT( idxName, "Index name is NULL") ;
      SDB_ASSERT( extName, "Buffer is empty" ) ;

      if ( UTIL_INVALID_UNIQUEID == clUniqID )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Collection unique id is invalid" ) ;
         goto error ;
      }

      name << SYS_PREFIX"_" << clUniqID << "_" << idxName ;
      if ( buffSize < name.str().length() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Buffer size[ %d ] is too small, expected[ %d ]",
                 buffSize, name.str().length() ) ;
      }
      if ( extName )
      {
         ossSnprintf( extName, buffSize, name.str().c_str() ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_GETEXTDATANAME, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_CHECK, "_rtnExtDataHandler::check" )
   INT32 _rtnExtDataHandler::check( DMS_EXTOPR_TYPE type,
                                    const CHAR *csName,
                                    const CHAR *clName,
                                    const CHAR *idxName,
                                    const BSONObj *object,
                                    const BSONObj *objNew,
                                    pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_CHECK ) ;

      rtnExtDataProcessor *processor = NULL ;
      std::vector<rtnExtDataProcessor *> processors ;

      if ( idxName )
      {
         rc = _edpMgr->getProcessorByIdx( csName, clName, idxName, SHARED,
                                          processor ) ;
         PD_RC_CHECK( rc, PDERROR, "Get external processor failed[ %d ]", rc ) ;
      }
      else if ( clName )
      {
         rc = _edpMgr->getProcessorsByCL( csName, clName, SHARED, processors ) ;
         PD_RC_CHECK( rc, PDERROR, "Get external processors failed[ %d ]",
                      rc ) ;
         SDB_ASSERT( processors.size() <= 1, "Processor number is wrong" ) ;
         if ( 0 == processors.size() )
         {
            goto done ;
         }
         processor = processors.front() ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Collection is empty for check" ) ;
         goto error ;
      }

      if ( processor )
      {
         rc = processor->check( type, object, objNew ) ;
         PD_RC_CHECK( rc, PDERROR, "Processor check failed[ %d ]", rc ) ;
      }

   done:
      if ( processor )
      {
         _edpMgr->unlockProcessor( processor->getID(), SHARED ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_CHECK, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONOPENTEXTIDX, "_rtnExtDataHandler::onOpenTextIdx" )
   INT32 _rtnExtDataHandler::onOpenTextIdx( const CHAR *csName,
                                            const CHAR *clName,
                                            ixmIndexCB &indexCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONOPENTEXTIDX ) ;
      rtnExtDataProcessor *processor = NULL ;

      // For compatibility with version 3.0.
      // From version 3.0.1, external data name(apped cs name) is stored in
      // indexCB. Version 3.0 didn't do that. And the external name generation
      // are different. So during upgrade, for text indices created with sdb
      // version 3.0. generate the name with the old rule and append it to the
      // indexCB.
      if ( !_hasExtName( indexCB ) )
      {
         rc = _extendIndexDef( csName, clName, indexCB ) ;
         PD_RC_CHECK( rc, PDERROR, "Extend index definition failed[ %d ]",
                      rc ) ;
      }

      rc = _edpMgr->createProcessor( csName, clName, indexCB.getName(),
                                     indexCB.getExtDataName(),
                                     indexCB.keyPattern(), processor, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Create external processor failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONOPENTEXTIDX, rc ) ;
      return rc ;
   error:
      if ( processor )
      {
         _edpMgr->delProcessor( &processor ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDELCS, "_rtnExtDataHandler::onDelCS" )
   INT32 _rtnExtDataHandler::onDelCS( const CHAR *csName, pmdEDUCB *cb,
                                      BOOLEAN removeFiles, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDELCS ) ;
      rtnExtDropCSCtx *context = NULL ;

      context = (rtnExtDropCSCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_DROPCS,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, cb, removeFiles, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Open external delete cs context failed[ %d ]",
                   rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDELCS, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDELCL, "_rtnExtDataHandler::onDelCL" )
   INT32 _rtnExtDataHandler::onDelCL( const CHAR *csName, const CHAR *clName,
                                      pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDELCL ) ;
      rtnExtDropCLCtx *context = NULL ;

      context = (rtnExtDropCLCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_DROPCL,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Open external delete cs context failed[ %d ]",
                   rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDELCL, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONCRTTEXTIDX, "_rtnExtDataHandler::onCrtTextIdx" )
   INT32 _rtnExtDataHandler::onCrtTextIdx( utilCLUniqueID clUniqID,
                                           const BSONObj &index,
                                           BSONObj &newIndex,
                                           pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONCRTTEXTIDX ) ;
      BSONObj keys = index.getObjectField( IXM_KEY_FIELD ) ;
      const CHAR *idxName = index.getStringField( IXM_NAME_FIELD ) ;

      SDB_UNUSED( cb ) ;
      SDB_UNUSED( dpscb ) ;

      SDB_ASSERT( idxName, "index name is null") ;

      if ( _edpMgr->number() >= RTN_TEXTIDX_MAX_NUM )
      {
         rc = SDB_OSS_UP_TO_LIMIT ;
         PD_LOG( PDERROR, "Max number of text indices[%d] has been created",
                 RTN_TEXTIDX_MAX_NUM ) ;
         goto error ;
      }

      if ( keys.hasField( "_id" ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Text index can't include _id field" ) ;
         goto error ;
      }

      // Append the external data name into the index definition.
      // Example: "ExtDataName" : "SYS_123456789_idx"
      {
         BSONObjBuilder builder ;
         CHAR extName[ DMS_MAX_EXT_NAME_SIZE + 1 ] = { 0 };

         getExtDataName( clUniqID, idxName, extName,
                         DMS_MAX_EXT_NAME_SIZE + 1 ) ;

         builder.appendElements( index ) ;
         builder.append( FIELD_NAME_EXT_DATA_NAME, extName,
                         DMS_MAX_EXT_NAME_SIZE + 1 ) ;
         newIndex = builder.obj() ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONCRTTEXTIDX, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX, "_rtnExtDataHandler::onDropTextIdx" )
   INT32 _rtnExtDataHandler::onDropTextIdx( const CHAR *extName,
                                            pmdEDUCB *cb,
                                            SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX ) ;
      SDB_DB_STATUS dbStatus = pmdGetKRCB()->getDBStatus() ;
      rtnExtDropIdxCtx *context = NULL ;

      if ( SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      context = (rtnExtDropIdxCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_DROPIDX,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      // If the current context is not DROPIDX, the current operation may be
      // dropping cs or cl. Nothing should be done in that cases.
      if ( DMS_EXTOPR_TYPE_DROPIDX == context->getType() )
      {
         rc = context->open( _edpMgr, extName, cb, dpscb ) ;
         PD_RC_CHECK( rc, PDERROR, "Open external drop context failed[ %d ]",
                      rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX, "_rtnExtDataHandler::onRebuildTextIdx" )
   INT32 _rtnExtDataHandler::onRebuildTextIdx( const CHAR *csName,
                                               const CHAR *clName,
                                               const CHAR *idxName,
                                               const CHAR *extName,
                                               const BSONObj &keyDef,
                                               pmdEDUCB *cb,
                                               SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX ) ;
      rtnExtRebuildIdxCtx *context = NULL ;

      SDB_ASSERT( cb, "cb should not be NULL" ) ;

      context = (rtnExtRebuildIdxCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_REBUILDIDX,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, idxName,
                          extName, keyDef, cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Open external rebuild context failed[ %d ]",
                   rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONINSERT, "_rtnExtDataHandler::onInsert" )
   INT32 _rtnExtDataHandler::onInsert( const CHAR *extName,
                                       const BSONObj &object,
                                       pmdEDUCB* cb,
                                       SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONINSERT ) ;
      SDB_DB_STATUS dbStatus = pmdGetKRCB()->getDBStatus() ;
      rtnExtInsertCtx *context = NULL ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      context = (rtnExtInsertCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_INSERT,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, extName, object, cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context for insert failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONINSERT, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         INT32 rcTmp = _contextMgr.delContext( context->getID(), cb ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Delete context failed[ %d ]", rcTmp ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDELETE, "_rtnExtDataHandler::onDelete" )
   INT32 _rtnExtDataHandler::onDelete( const CHAR *extName,
                                       const BSONObj &object,
                                       pmdEDUCB* cb,
                                       SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDELETE ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      rtnExtDeleteCtx *context = NULL ;
      BSONObj processData ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      context = (rtnExtDeleteCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_DELETE,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, extName, object, cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context for insert failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDELETE, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         INT32 rcTmp = _contextMgr.delContext( context->getID(), cb ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Delete context failed[ %d ]", rcTmp ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONUPDATE, "_rtnExtDataHandler::onUpdate" )
   INT32 _rtnExtDataHandler::onUpdate( const CHAR *extName,
                                       const BSONObj &orignalObj,
                                       const BSONObj &newObj,
                                       pmdEDUCB* cb,
                                       SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONUPDATE ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      rtnExtUpdateCtx *context = NULL ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      context = (rtnExtUpdateCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_UPDATE,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, extName, orignalObj,
                          newObj, cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context for insert failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONUPDATE, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         INT32 rcTmp = _contextMgr.delContext( context->getID(), cb ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Delete context failed[ %d ]", rcTmp ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONTRUNCATECL, "_rtnExtDataHandler::onTruncateCL" )
   INT32 _rtnExtDataHandler::onTruncateCL( const CHAR *csName,
                                           const CHAR *clName,
                                           pmdEDUCB *cb,
                                           SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONTRUNCATECL ) ;
      rtnExtTruncateCtx *context = NULL ;
      vector<rtnExtDataProcessor *> processors ;

      context = (rtnExtTruncateCtx *)_contextMgr.findContext( cb->getTID() ) ;
      if ( !context )
      {
         rc = _contextMgr.createContext( DMS_EXTOPR_TYPE_TRUNCATE,
                                         cb, (rtnExtContextBase**)&context ) ;
         PD_RC_CHECK( rc, PDERROR, "Create new context failed[ %d ]", rc ) ;
      }

      rc = context->open( _edpMgr, csName, clName, cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context for truncate failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONTRUNCATECL, rc ) ;
      return rc ;
   error:
      if ( context )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      goto done ;
   }

   INT32 _rtnExtDataHandler::onRenameCS( const CHAR *oldCSName,
                                         const CHAR *newCSName,
                                         pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      // TODO: YSD
      return SDB_OK ;
   }

   INT32 _rtnExtDataHandler::onRenameCL( const CHAR *csName,
                                         const CHAR *oldCLName,
                                         const CHAR *newCLName,
                                         pmdEDUCB *cb, SDB_DPSCB *dpscb )
   {
      // TODO: YSD
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_DONE, "_rtnExtDataHandler::done" )
   INT32 _rtnExtDataHandler::done( DMS_EXTOPR_TYPE type, pmdEDUCB *cb,
                                   SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_DONE ) ;
      rtnExtContextBase *context = _contextMgr.findContext( cb->getTID() ) ;
      BOOLEAN ownContext = FALSE ;
      if ( !context )
      {
         goto done ;
      }

      ownContext = ( context->getType() == type ) ;
      if ( !ownContext )
      {
         goto done ;
      }

      rc = context->done( cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Final step of current operation failed[ %d ]",
                      rc) ;

   done:
      if ( context && ownContext )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_DONE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ABORTOPERATION, "_rtnExtDataHandler::abortOperation" )
   INT32 _rtnExtDataHandler::abortOperation( DMS_EXTOPR_TYPE type,
                                             pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ABORTOPERATION ) ;
      rtnExtContextBase *context = _contextMgr.findContext( cb->getTID() ) ;
      BOOLEAN ownContext = FALSE ;
      if ( !context )
      {
         goto done ;
      }

      ownContext = ( context->getType() == type ) ;
      if ( !ownContext )
      {
         goto done ;
      }

      rc = context->abort( cb, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Final step of current operation failed[ %d ]",
                   rc) ;

   done:
      if ( context && ownContext )
      {
         _contextMgr.delContext( context->getID(), cb ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ABORTOPERATION, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _rtnExtDataHandler::_hasExtName( const ixmIndexCB &indexCB )
   {
      return ( ossStrlen( indexCB.getExtDataName() ) > 0 ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER__EXTENDINDEXDEF, "_rtnExtDataHandler::_extendIndexDef" )
   INT32 _rtnExtDataHandler::_extendIndexDef( const CHAR *csName,
                                              const CHAR *clName,
                                              ixmIndexCB &indexCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER__EXTENDINDEXDEF ) ;
      string extName ;
      BSONObj extendObj ;
      _getExtDataNameV1( csName, clName, indexCB.getName(), extName )  ;

      try
      {
         extendObj = BSON( FIELD_NAME_EXT_DATA_NAME << extName.c_str() ) ;
         indexCB.extendDef( extendObj.firstElement() ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER__EXTENDINDEXDEF, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER__GETEXTDATANAMEV1, "_rtnExtDataHandler::_getExtDataNameV1" )
   void _rtnExtDataHandler::_getExtDataNameV1( const CHAR *csName,
                                               const CHAR *clName,
                                               const CHAR *idxName,
                                               string &extName )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER__GETEXTDATANAMEV1 ) ;
      string srcStr = string( csName ) + string( clName ) + string( idxName ) ;
      UINT32 hashVal = ossHash( srcStr.c_str() ) ;
      string md5Val = md5::md5simpledigest( srcStr.c_str() ) ;
      ostringstream name ;
      name << SYS_PREFIX"_" << hashVal << md5Val.substr( 0, 4 ) ;
      extName = name.str() ;
      PD_TRACE_EXIT( SDB__RTNEXTDATAHANDLER__GETEXTDATANAMEV1 ) ;
   }

   rtnExtDataHandler* rtnGetExtDataHandler()
   {
      static rtnExtDataHandler s_edh( rtnGetExtDataProcessorMgr() ) ;
      return &s_edh ;
   }
}

