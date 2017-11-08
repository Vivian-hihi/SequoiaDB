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
#include "pmd.hpp"
#include "rtn.hpp"
#include "rtnTrace.hpp"
#include "rtnExtDataHandler.hpp"

// Currently we set the size limit of capped collection to 30GB. This may change
// in the future.
#define RTN_CAPPED_CL_MAXSIZE       ( 30 * 1024 * 1024 * 1024LL )
#define RTN_CAPPED_CL_MAXRECNUM     0
#define RTN_FIELD_NAME_RID          "_rid"
#define RTN_FIELD_NAME_SOURCE       "_source"

namespace engine
{
   _rtnExtDataHandler::_rtnExtDataHandler()
   {
   }

   _rtnExtDataHandler::~_rtnExtDataHandler()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER__ADDOPRRECORD, "_rtnExtDataHandler::_addOprRecord" )
   INT32 _rtnExtDataHandler::_addOprRecord( const CHAR *name,
                                            _dmsExtOprType oprType,
                                            pmdEDUCB *cb,
                                            const bson::OID *dataOID,
                                            const BSONObj *dataObj,
                                            SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER__ADDOPRRECORD ) ;
      INT32 insertNum = 0 ;
      INT32 ignoreNum = 0 ;
      BSONObj objToInsert ;
      BSONObjBuilder objBuilder ;
      BOOLEAN oidRequired = FALSE ;
      BOOLEAN dataRequired = FALSE ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB () ;

      switch ( oprType )
      {
         case DMS_EXT_INSERT:  // insert
         case DMS_EXT_UPDATE:  // update
            oidRequired = TRUE ;
            dataRequired = TRUE ;
            break ;
         case DMS_EXT_DELETE:  // delete
            oidRequired = TRUE ;
            dataRequired = FALSE ;
            break ;
         case DMS_EXT_TRUNCATE:  // truncate
            oidRequired = FALSE ;
            dataRequired = FALSE ;
            break ;
         default:
            PD_LOG( PDERROR, "Invalid operation type[ %d ]", oprType ) ;
            rc = SDB_SYS ;
            goto error ;
      }

      if ( oidRequired && ( !dataOID || !dataOID->isSet() ) )
      {
         PD_LOG( PDERROR, "_id should be specified for current operation" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( dataRequired && !dataObj )
      {
         PD_LOG( PDERROR, "Data object is NULL for operation" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      try
      {
         // 1. Append operation type.
         objBuilder.append( FIELD_NAME_TYPE, oprType ) ;
         // 2. Append the _id as _rid.
         if ( oidRequired )
         {
            objBuilder.append( RTN_FIELD_NAME_RID, dataOID->str().c_str() ) ;
         }

         // 3. Append data if necessarry.
         if ( dataRequired )
         {
            objBuilder.append( RTN_FIELD_NAME_SOURCE, *dataObj ) ;
         }

         objToInsert = objBuilder.done() ;

         PD_LOG( PDDEBUG, "Operation record to insert: %s",
                 objToInsert.toString( FALSE, TRUE ).c_str() ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

      // Pass dpsCB as NULL as we don't want to write dps log. The replication
      // of external data totally relies on the original data.
      rc = rtnInsert( name, objToInsert, 1, 0,
                      cb, dmsCB, NULL, 1, &insertNum, &ignoreNum ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert record failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER__ADDOPRRECORD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER__GETTEXTIDXCSLIST, "_rtnExtDataHandler::_getTextIdxCSList" )
   void _rtnExtDataHandler::_getTextIdxCSList( const monCSSimple &csInfo,
                                               vector<string> &csNameVec )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER__GETTEXTIDXCSLIST ) ;

      // Traverse all the indices of all collections in the cs, to find out all
      // the text indexes.
      for ( MON_CL_SIM_VEC::const_iterator clItr = csInfo._clList.begin();
            clItr != csInfo._clList.end(); ++clItr )
      {
         for ( MON_IDX_LIST::const_iterator idxItr = clItr->_idxList.begin();
               idxItr != clItr->_idxList.end(); ++idxItr )
         {
            INT32 rcTmp = SDB_OK ;
            UINT16 idxType = IXM_EXTENT_TYPE_NONE ;
            rcTmp = idxItr->getIndexType( idxType ) ;
            if ( rcTmp )
            {
               if ( SDB_OK == rc )
               {
                  rc = rcTmp ;
               }
               // Only logging error, and process the remaining ones.
               PD_LOG( PDERROR, "Get type of index failed[ %d ], cs[ %s ],"
                       " cl[ %s ], index[ %s ]", rcTmp, csInfo._name,
                       clItr->_name, idxItr->getIndexName() ) ;
               continue ;
            }
            if ( IXM_EXTENT_HAS_TYPE( IXM_EXTENT_TYPE_TEXT, idxType ) )
            {
               string cappedCSName ;
               string cappedCLName ;
               buildNames( csInfo._name, clItr->_clname,
                           idxItr->getIndexName(), cappedCSName,
                           cappedCLName ) ;
               csNameVec.push_back( cappedCSName ) ;
            }
         }
      }

      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER__GETTEXTIDXCSLIST, rc ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDROPCS, "_rtnExtDataHandler::onDropCS" )
   INT32 _rtnExtDataHandler::onDropCS( const monCSSimple &csInfo, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDROPCS ) ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      vector< string > cappedCSs ;
      BOOLEAN hasTextIdx = FALSE ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      // It may fail when dropping some of the related collection spaces. In
      // that case, we logging the error, and remember the first error code, and
      // continue with the remainning ones. At the end, return the first error
      // code, if any.

      _getTextIdxCSList( csInfo, cappedCSs ) ;
      hasTextIdx = cappedCSs.size() > 0 ? TRUE : FALSE ;
      for ( vector<string>::iterator itr = cappedCSs.begin();
            itr != cappedCSs.end(); ++itr )
      {
         INT32 rcTmp = rtnDropCollectionSpaceCommand( itr->c_str(), cb,
                                                      dmsCB, NULL, TRUE ) ;
         if ( rcTmp )
         {
            if ( SDB_OK == rc )
            {
               rc = rcTmp ;
            }
            // Only logging error, and process the remaining ones.
            PD_LOG( PDERROR, "Drop collection space[ %s ] failed[ %d ]",
                    itr->c_str(), rcTmp ) ;
            continue ;
         }
      }

      if ( hasTextIdx )
      {
         rtnCB->incTextIdxVersion() ;
      }

      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDROPCS, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONUNLOADCS, "_rtnExtDataHandler::onUnloadCS" )
   INT32 _rtnExtDataHandler::onUnloadCS( const monCSSimple &csInfo,
                                         pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONUNLOADCS ) ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      vector< string > cappedCSs ;

      _getTextIdxCSList( csInfo, cappedCSs ) ;
      for ( vector<string>::iterator itr = cappedCSs.begin();
            itr != cappedCSs.end(); ++itr )
      {
         INT32 rcTmp = rtnUnloadCollectionSpace( itr->c_str(), cb, dmsCB ) ;
         if ( rcTmp )
         {
            if ( SDB_OK == rc )
            {
               rc = rcTmp ;
            }
            // Only logging error, and process the remaining ones.
            PD_LOG( PDERROR, "Unload collection space[ %s ] failed[ %d ]",
                    itr->c_str(), rcTmp ) ;
            continue ;
         }
      }

      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONUNLOADCS, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONCREATETEXTIDX, "_rtnExtDataHandler::onCreateTextIdx" )
   INT32 _rtnExtDataHandler::onCreateTextIdx( const CHAR *csName,
                                              const CHAR *clName,
                                              const CHAR *idxName,
                                              pmdEDUCB* cb,
                                              SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONCREATETEXTIDX ) ;
      string cappedCSName ;
      string cappedCLFullName ;
      BSONObj extOptions ;
      BSONObjBuilder builder ;
      SDB_DB_STATUS dbStatus = pmdGetKRCB()->getDBStatus() ;

      // During the full sync, let the capped collections to sync by themselves.
      if ( SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      buildNames( csName, clName, idxName, cappedCSName, cappedCLFullName ) ;

      rc = _prepareCSAndCL( cappedCSName.c_str(), cappedCLFullName.c_str(),
                            cb, dpsCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Create capped collection[ %s.%s ] "
                   "failed[ %d ]",
                   cappedCSName.c_str(), cappedCLFullName.c_str(), rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONCREATETEXTIDX, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX, "_rtnExtDataHandler::onDropTextIdx" )
   INT32 _rtnExtDataHandler::onDropTextIdx( const CHAR *csName,
                                            const CHAR *clName,
                                            const CHAR *idxName,
                                            pmdEDUCB* cb,
                                            SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX ) ;
      string cappedCSName ;
      string cappedCLName ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;
      SDB_DB_STATUS dbStatus = pmdGetKRCB()->getDBStatus() ;

      if ( SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      buildNames( csName, clName, idxName, cappedCSName, cappedCLName ) ;

      rc = rtnDropCollectionSpaceCommand( cappedCSName.c_str(), cb, dmsCB,
                                          dpscb, TRUE ) ;
      if ( SDB_DMS_CS_NOTEXIST == rc )
      {
         PD_LOG( PDWARNING, "Capped collection space[ %s ] not found when "
                 "dropping text index[ %s ] of collection[ %s.%s ]",
                 cappedCSName.c_str(), idxName, csName, clName ) ;
         rc = SDB_OK ;
      }
      else
      {
         PD_RC_CHECK( rc, PDERROR, "Drop capped collection space failed[ %d ]",
                      rc ) ;
      }
      rtnCB->incTextIdxVersion() ;
   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDROPTEXTIDX, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX, "_rtnExtDataHandler::onRebuildTextIdx" )
   INT32 _rtnExtDataHandler::onRebuildTextIdx( const CHAR *csName,
                                               const CHAR *clName,
                                               const CHAR *idxName,
                                               _pmdEDUCB *cb,
                                               SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX ) ;
      string cappedCSName ;
      string cappedCLName ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;

      // In case of full sync, leave all the capped collections to sync by
      // themselves, because we want them to be exactly the same with the ones
      // on primary node.
      if ( SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      buildNames( csName, clName, idxName, cappedCSName, cappedCLName ) ;

      // In case of rebuilding, we don't know whether the capped collections are
      // valid or not. So we directly drop and re-create the capped collection
      // again.
      if ( SDB_DB_REBUILDING == dbStatus )
      {
         rc = rtnDropCollectionSpaceCommand( cappedCSName.c_str(), cb,
                                             dmsCB, dpscb, TRUE ) ;
         if ( SDB_DMS_CS_NOTEXIST == rc )
         {
            PD_LOG( PDINFO, "Capped collection space[ %s ] not found when "
                    "trying to drop it", cappedCSName.c_str() ) ;
         }
         else
         {
            PD_RC_CHECK( rc, PDERROR, "Drop collectionspace[ %s ] failed[ %d ]",
                         cappedCSName.c_str(), rc);
         }
      }

      rc = _prepareCSAndCL( cappedCSName.c_str(), cappedCLName.c_str(),
                            cb, dpscb ) ;
      PD_RC_CHECK( rc, PDERROR, "Create capped collection[ %s.%s ] "
                   "failed[ %d ]",
                   cappedCSName.c_str(), cappedCLName.c_str(), rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONREBUILDTEXTIDX, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONINSERT, "_rtnExtDataHandler::onInsert" )
   INT32 _rtnExtDataHandler::onInsert( const CHAR *csName, const CHAR *clName,
                                       const CHAR *idxName, BSONObj &object,
                                       bson::OID &oid, INT32 flags,
                                       pmdEDUCB* cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONINSERT ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      string cappedCSName ;
      string cappedCLFullName ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      buildNames( csName, clName, idxName, cappedCSName, cappedCLFullName ) ;

      rc = _addOprRecord( cappedCLFullName.c_str(), DMS_EXT_INSERT, cb,
                          &oid, &object, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert operation record failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONINSERT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONDELETE, "_rtnExtDataHandler::onDelete" )
   INT32 _rtnExtDataHandler::onDelete( const CHAR *csName, const CHAR *clName,
                                       const CHAR *idxName, bson::OID &oid,
                                       pmdEDUCB* cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONDELETE ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      string cappedCSName ;
      string cappedCLFullName ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      buildNames( csName, clName, idxName, cappedCSName, cappedCLFullName ) ;

      rc = _addOprRecord( cappedCLFullName.c_str(), DMS_EXT_DELETE, cb,
                          &oid, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert Operation Record failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONDELETE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONUPDATE, "_rtnExtDataHandler::onUpdate" )
   INT32 _rtnExtDataHandler::onUpdate( const CHAR *csName, const CHAR *clName,
                                       const CHAR *idxName, BSONObj &object,
                                       bson::OID &oid, INT32 flags,
                                       pmdEDUCB* cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONUPDATE ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      string cappedCSName ;
      string cappedCLFullName ;

      if ( SDB_DB_REBUILDING == dbStatus || SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      buildNames( csName, clName, idxName, cappedCSName, cappedCLFullName ) ;

      rc = _addOprRecord( cappedCLFullName.c_str(), DMS_EXT_UPDATE, cb,
                          &oid, &object, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert Operation Record failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONUPDATE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER_ONTRUNCATE, "_rtnExtDataHandler::onTruncate" )
   INT32 _rtnExtDataHandler::onTruncate( const CHAR *csName, const CHAR *clName,
                                         const CHAR *idxName, pmdEDUCB* cb,
                                         SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER_ONTRUNCATE ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      string cappedCSName ;
      string cappedCLFullName ;

      if ( SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      buildNames( csName, clName, idxName, cappedCSName, cappedCLFullName ) ;

      // In case of rebuilding, we are not sure if the capped collection is
      // valid or not.
      if ( SDB_DB_REBUILDING == dbStatus )
      {
         rc = rtnTruncCollectionCommand( cappedCLFullName.c_str(), cb,
                                         krcb->getDMSCB(), NULL ) ;
         PD_RC_CHECK( rc, PDERROR, "Truncate collection[ %s ] failed[ %d ]",
                      cappedCLFullName.c_str(), rc ) ;
      }

      rc = _addOprRecord( cappedCLFullName.c_str(), DMS_EXT_TRUNCATE, cb,
                          NULL, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert Operation Record failed[ %d ]", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAHANDLER_ONTRUNCATE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAHANDLER__BUILDNAMES, "_rtnExtDataHandler::buildNames" )
   void _rtnExtDataHandler::buildNames( const CHAR *csName,
                                        const CHAR *clName,
                                        const CHAR *idxName,
                                        string &cappedCSName,
                                        string &cappedCLName )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAHANDLER__BUILDNAMES )  ;
      SDB_ASSERT( csName, "Collection space name can't be NULL")  ;
      SDB_ASSERT( clName, "Collection name can't be NULL")  ;
      SDB_ASSERT( idxName, "Index name can't be NULL" ) ;

      cappedCSName = string( SYS_PREFIX ) + "_" + csName + "_" +
                     clName + "_" + idxName ;
      cappedCLName = cappedCSName + "." + cappedCSName ;

      PD_TRACE_EXIT( SDB__RTNEXTDATAHANDLER__BUILDNAMES ) ;
   }

   INT32 _rtnExtDataHandler::_prepareCSAndCL( const CHAR *csName,
                                              const CHAR *clName,
                                              pmdEDUCB *cb,
                                              SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;
      BOOLEAN csCreated = FALSE ;
      BSONObjBuilder builder ;
      BSONObj extOptions ;

      rc = rtnCreateCollectionSpaceCommand( csName, cb, dmsCB,
                                            dpsCB, DMS_PAGE_SIZE_DFT,
                                            DMS_DO_NOT_CREATE_LOB,
                                            DMS_STORAGE_CAPPED, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Create capped collection space failed[ %d ]",
                   rc ) ;

      csCreated = TRUE ;

      try
      {
         builder.append( FIELD_NAME_SIZE, RTN_CAPPED_CL_MAXSIZE ) ;
         builder.append( FIELD_NAME_MAX, RTN_CAPPED_CL_MAXRECNUM ) ;
         // Set the OverWrite option as false.
         builder.appendBool( FIELD_NAME_OVERWRITE, FALSE ) ;
         extOptions = builder.done() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

      rc = rtnCreateCollectionCommand( clName,
                                       DMS_MB_ATTR_NOIDINDEX | DMS_MB_ATTR_CAPPED,
                                       cb, dmsCB, dpsCB,
                                       UTIL_COMPRESSOR_INVALID, 0,
                                       TRUE, &extOptions ) ;
      PD_RC_CHECK( rc, PDERROR, "Create capped collection[ %s ] failed[ %d ]",
                   clName, rc ) ;
      rtnCB->incTextIdxVersion() ;

   done:
      return rc ;
   error:
      if ( csCreated )
      {
         INT32 rcTmp = rtnDropCollectionSpaceCommand( csName, cb, dmsCB,
                                                      dpsCB, TRUE ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Drop collectionspace[ %s ] failed[ %d ]",
                    csName, rcTmp ) ;
         }
      }
      goto done ;
   }

   rtnExtDataHandler* getRtnExtDataHandler()
   {
      static rtnExtDataHandler s_rtnExtDataHandler ;
      return &s_rtnExtDataHandler ;
   }
}

