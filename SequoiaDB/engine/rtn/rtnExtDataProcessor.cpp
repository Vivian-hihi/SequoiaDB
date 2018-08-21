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

   Source File Name = rtnExtDataProcessor.hpp

   Descriptive Name = External data processor for rtn.

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
#include "rtnExtDataProcessor.hpp"
#include "rtn.hpp"
#include "rtnTrace.hpp"
#include "dmsStorageDataCapped.hpp"

// Currently we set the size limit of capped collection to 30GB. This may change
// in the future.
#define RTN_CAPPED_CL_MAXSIZE       ( 30 * 1024 * 1024 * 1024LL )
#define RTN_CAPPED_CL_MAXRECNUM     0
#define RTN_FIELD_NAME_RID          "_rid"
#define RTN_FIELD_NAME_SOURCE       "_source"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR__RTNEXTDATAPROCESSOR, "_rtnExtDataProcessor::_rtnExtDataProcessor" )
   _rtnExtDataProcessor::_rtnExtDataProcessor()
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR__RTNEXTDATAPROCESSOR ) ;
      _mbContext = NULL ;
      _id = RTN_EXT_PROCESSOR_INVALID_ID ;
      ossMemset( _cappedCSName, 0, DMS_COLLECTION_SPACE_NAME_SZ + 1 ) ;
      ossMemset( _cappedCLName, 0, DMS_COLLECTION_NAME_SZ + 1 ) ;
      _needUpdateLSN = FALSE ;
      _needOprRec = FALSE ;
      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSOR__RTNEXTDATAPROCESSOR ) ;
   }

   _rtnExtDataProcessor::~_rtnExtDataProcessor()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_INIT, "_rtnExtDataProcessor::init" )
   INT32 _rtnExtDataProcessor::init( INT64 id, const CHAR *csName,
                                     const CHAR *clName,
                                     const CHAR *idxName,
                                     const CHAR *targetName,
                                     const BSONObj &idxKeyDef )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_INIT ) ;
      dmsStorageUnitID suID = DMS_INVALID_SUID ;
      dmsStorageUnit *su = NULL ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      rc = _meta.init( csName, clName, idxName, targetName, idxKeyDef ) ;
      PD_RC_CHECK( rc, PDERROR, "Processor meta init failed[ %d ]", rc ) ;

      rc = setTargetNames( targetName ) ;
      PD_RC_CHECK( rc, PDERROR, "Set target names failed[ %d ]", rc ) ;
      _id = id ;

      rc = dmsCB->nameToSUAndLock( _cappedCSName, suID, &su, SHARED ) ;
      if ( rc )
      {
         if ( SDB_DMS_CS_NOTEXIST == rc )
         {
            // Index is being created. _mbContext will be set in the rebuild.
            rc = SDB_OK ;
            goto error ;
         }
         else
         {
            PD_LOG( PDERROR, "Lock collection space[ %s ] failed[ %d ]",
                   _cappedCSName, rc ) ;
            goto error ;
         }
      }

      // Get the mb context for the capped collection, to update the LSN
      // information after write operation.
      rc = su->data()->getMBContext( &_mbContext, _getExtCLShortName(), -1 ) ;
      PD_RC_CHECK( rc, PDERROR, "Get mbContext for collection[ %s ] "
                   "failed[ %d ]", _cappedCLName, rc ) ;

   done:
      if ( DMS_INVALID_SUID != suID )
      {
         dmsCB->suUnlock( suID, SHARED ) ;
      }

      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_INIT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT64 _rtnExtDataProcessor::getID()
   {
      return _id ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_SETTARGETNAMES, "_rtnExtDataProcessor::setTargetNames" )
   INT32 _rtnExtDataProcessor::setTargetNames( const CHAR *extName )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_SETTARGETNAMES ) ;
      SDB_ASSERT( extName, "cs name is NULL" ) ;

      if ( ossStrlen( extName ) > DMS_COLLECTION_SPACE_NAME_SZ )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "External name too long: %s", extName ) ;
         goto error ;
      }

      ossStrncpy( _cappedCSName, extName, DMS_COLLECTION_SPACE_NAME_SZ + 1 ) ;
      ossSnprintf( _cappedCLName, DMS_COLLECTION_FULL_NAME_SZ + 1,
                   "%s.%s", extName, extName ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_SETTARGETNAMES, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_CHECK, "_rtnExtDataProcessor::check" )
   INT32 _rtnExtDataProcessor::check( DMS_EXTOPR_TYPE type,
                                      const BSONObj *object,
                                      const BSONObj *objectNew )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_CHECK ) ;
      dmsMBContext *context = NULL ;
      dmsStorageUnit *su = NULL ;
      dmsStorageUnitID suID = DMS_INVALID_SUID ;
      const CHAR *collection = NULL ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      // Only check for insert/delete/update when object is not empty.
      if ( type > DMS_EXTOPR_TYPE_UPDATE || !object )
      {
         goto done ;
      }

      _keySet.clear() ;
      _keySetNew.clear() ;
      _needOprRec = FALSE ;

      {
         // Get the key set
         BSONElement arrayEle ;
         ixmIndexKeyGen keygen( _meta._idxKeyDef, GEN_OBJ_KEEP_FIELD_NAME ) ;
         rc = keygen.getKeys( *object, _keySet, &arrayEle, TRUE, TRUE ) ;
         PD_RC_CHECK( rc, PDERROR, "Generate key from object[ % ] failed[ %d ]",
                      object->toString().c_str(), rc ) ;

         // Record with array in its index key will never be indexed on
         // Elasticsearch, and no record for it will be inserted into capped
         // collection.
         if ( DMS_EXTOPR_TYPE_UPDATE != type )
         {
            // Array for insert and delete, skip.
            if ( _keySet.size() > 1 )
            {
               _keySet.clear() ;
               goto done ;
            }
            _needOprRec = ( _keySet.size() > 0 ) ;
         }
         else
         {
            SDB_ASSERT( objectNew, "New object is NULL" ) ;
            // If it's update, need to get the new key set.
            rc = keygen.getKeys( *objectNew, _keySetNew,
                                 NULL, TRUE, TRUE ) ;
            PD_RC_CHECK( rc, PDERROR, "Generate key from object[ %s ] "
                         "failed[ %d ]", objectNew->toString().c_str(), rc ) ;

            {
               BSONObjSet::iterator origItr = _keySet.begin() ;
               BSONObjSet::iterator newItr = _keySetNew.begin() ;

               // There are only two scenarios that we do not insert operation
               // record:
               // 1. Both old and new records have no index field.
               // 2. Both old and new records have index field(s) but they are the
               //    same.
               while ( origItr != _keySet.end() && newItr != _keySetNew.end() )
               {
                  if ( !( *origItr == *newItr ) )
                  {
                     break ;
                  }
                  origItr++ ;
                  newItr++ ;
               }

               if ( ( origItr == _keySet.end() ) && ( newItr == _keySetNew.end() ) )
               {
                  _needOprRec = FALSE ;
               }
               else
               {
                  _needOprRec = TRUE ;
               }
            }

            // New record contains array. It will not be inserted. If there is
            // old record, it will be deleted on ES.
            if ( _keySetNew.size() > 1 )
            {
               _keySetNew.clear() ;
            }
         }
      }

      // If the object contains the index field, check if the capped collection
      // has enough free space.
      if ( _needOprRec )
      {
         rc = rtnResolveCollectionNameAndLock ( _cappedCLName, dmsCB, &su,
                                                &collection, suID ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to resolve collection name: %s, "
                      "rc: %d", _cappedCLName, rc ) ;
         rc = su->data()->getMBContext( &context, collection, -1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", collection, rc ) ;

         if ( FALSE == ((dmsStorageDataCapped *)su->data())->spaceEnough( context,
                                                            DMS_RECORD_MAX_SZ ) )
         {
            rc = SDB_OSS_UP_TO_LIMIT ;
            PD_LOG( PDERROR, "Storage space is not enough for operation" ) ;
            goto error ;
         }
      }

   done:
      if ( context )
      {
         su->data()->releaseMBContext( context ) ;
      }
      if ( DMS_INVALID_SUID != suID )
      {
         dmsCB->suUnlock( suID, SHARED ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_CHECK, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_PROCESSINSERT, "_rtnExtDataProcessor::processInsert" )
   INT32 _rtnExtDataProcessor::processInsert( const BSONObj &inputObj,
                                              pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_PROCESSINSERT ) ;
      BSONObj recordObj ;
      INT32 insertNum = 0 ;
      INT32 ignoreNum = 0 ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      rc = _prepareInsert( inputObj, recordObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Prepare data for insert record[ %s ] "
                   "failed[ %d ]", inputObj.toString().c_str(), rc ) ;

      if ( recordObj.isEmpty() )
      {
         goto done ;
      }

      rc = rtnInsert( _cappedCLName, recordObj, 1, 0,
                      cb, dmsCB, dpsCB, 1, &insertNum, &ignoreNum ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert record insert collection[ %s ] "
                   "failed[ %d ]", _cappedCLName, rc ) ;

      _needUpdateLSN = TRUE ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_PROCESSINSERT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_PROCESSDELETE, "_rtnExtDataProcessor::processDelete" )
   INT32 _rtnExtDataProcessor::processDelete( const BSONObj &inputObj,
                                              pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_PROCESSDELETE ) ;
      BSONObj recordObj ;
      INT32 insertNum = 0 ;
      INT32 ignoreNum = 0 ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      rc = _prepareDelete( inputObj, recordObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Prepare data for delete record[ %s ] "
                   "failed[ %d ]", inputObj.toString().c_str(), rc ) ;

      if ( recordObj.isEmpty() )
      {
         goto done ;
      }

      rc = rtnInsert( _cappedCLName, recordObj, 1, 0,
                      cb, dmsCB, dpsCB, 1, &insertNum, &ignoreNum ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert record insert collection[ %s ] "
                   "failed[ %d ]", _cappedCLName, rc ) ;

      _needUpdateLSN = TRUE ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_PROCESSDELETE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_PROCESSUPDATE, "_rtnExtDataProcessor::processUpdate" )
   INT32 _rtnExtDataProcessor::processUpdate( const BSONObj &originalObj,
                                              const BSONObj &newObj,
                                              pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_PROCESSUPDATE ) ;
      BSONObj recordObj ;
      INT32 insertNum = 0 ;
      INT32 ignoreNum = 0 ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      rc = _prepareUpdate( originalObj, newObj, recordObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Prepare data for update record from[ %s ] "
                   "to[ %s ] failed[ %d ]", originalObj.toString().c_str(),
                   newObj.toString().c_str(), rc ) ;

      if ( recordObj.isEmpty() )
      {
         goto done ;
      }

      rc = rtnInsert( _cappedCLName, recordObj, 1, 0,
                      cb, dmsCB, dpsCB, 1, &insertNum, &ignoreNum ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert record insert collection[ %s ] "
                   "failed[ %d ]", _cappedCLName, rc ) ;

      _needUpdateLSN = TRUE ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_PROCESSUPDATE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_DODROPP1, "_rtnExtDataProcessor::doDropP1" )
   INT32 _rtnExtDataProcessor::doDropP1( pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_DODROPP1 ) ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      rc = rtnDropCollectionSpaceP1( _cappedCSName, cb, dmsCB, dpsCB, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Phase 1 of dropping collection space[ %s ] "
                   "failed[ %d ]", _cappedCSName, rc ) ;
      rtnCB->incTextIdxVersion() ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_DODROPP1, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_DODROPP1CANCEL, "_rtnExtDataProcessor::doDropP1Cancel" )
   INT32 _rtnExtDataProcessor::doDropP1Cancel( pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_DODROPP1CANCEL ) ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      rc = rtnDropCollectionSpaceP1Cancel( _cappedCSName, cb, dmsCB,
                                           dpsCB, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Cancel dropping collection space[ %s ] in "
                   "phase 1 failed[ %d ]", _cappedCSName, rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_DODROPP1CANCEL, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_DODROPP2, "_rtnExtDataProcessor::doDropP2" )
   INT32 _rtnExtDataProcessor::doDropP2( pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_DODROPP2 ) ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      rc = rtnDropCollectionSpaceP2( _cappedCSName, cb, dmsCB,
                                     dpsCB, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Phase 1 of dropping collection space[ %s ] "
                   "failed[ %d ]", _cappedCSName, rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_DODROPP2, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnExtDataProcessor::doLoad()
   {
      // TODO:YSD
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_DOUNLOAD, "_rtnExtDataProcessor::doUnload" )
   INT32 _rtnExtDataProcessor::doUnload( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_DOUNLOAD ) ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      rc = rtnUnloadCollectionSpace( _cappedCSName, cb, dmsCB ) ;
      if ( SDB_DMS_CS_NOTEXIST == rc )
      {
         PD_LOG( PDWARNING, "Capped collection space[ %s ] not found when "
                 "unload", _cappedCSName ) ;
         rc = SDB_OK ;
      }
      else
      {
         PD_RC_CHECK( rc, PDERROR, "Unload capped collection space failed[ %d ]",
                      rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_DOUNLOAD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_DOREBUILD, "_rtnExtDataProcessor::doRebuild" )
   INT32 _rtnExtDataProcessor::doRebuild( pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_DOREBUILD ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DB_STATUS dbStatus = krcb->getDBStatus() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      dmsStorageUnitID suID = DMS_INVALID_SUID ;
      dmsStorageUnit *su = NULL ;

      // In case of rebuilding, we don't know whether the capped collections are
      // valid or not. So we directly drop and re-create the capped collection
      // again.
      if ( SDB_DB_REBUILDING == dbStatus )
      {
         rc = rtnDropCollectionSpaceCommand( _cappedCSName, cb,
                                             dmsCB, dpsCB, TRUE ) ;
         if ( SDB_DMS_CS_NOTEXIST == rc )
         {
            PD_LOG( PDINFO, "Capped collection space[ %s ] not found when "
                    "trying to drop it", _cappedCSName ) ;
         }
         else
         {
            PD_RC_CHECK( rc, PDERROR, "Drop collectionspace[ %s ] failed[ %d ]",
                         _cappedCSName, rc);
         }
         _mbContext = NULL ;
      }
      else if ( SDB_DB_FULLSYNC == dbStatus )
      {
         goto done ;
      }

      rc = _prepareCSAndCL( _cappedCSName, _cappedCLName, cb, dpsCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Create capped collection[ %s.%s ] "
                   "failed[ %d ]", _cappedCSName, _cappedCLName, rc ) ;

      rc = dmsCB->nameToSUAndLock( _cappedCSName, suID, &su, SHARED ) ;
      PD_RC_CHECK( rc, PDERROR, "Lock collection space[ %s ] failed[ %d ]",
                   _cappedCSName, rc ) ;

      rc = su->data()->getMBContext( &_mbContext, _getExtCLShortName(), -1 ) ;
      PD_RC_CHECK( rc, PDERROR, "Get mbContext for collection[ %s ] "
                   "failed[ %d ]", _cappedCLName, rc ) ;

   done:
      if ( DMS_INVALID_SUID != suID )
      {
         dmsCB->suUnlock( suID, SHARED ) ;
      }
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_DOREBUILD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // Update the commit LSN information for the capped collection.
   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_DONE, "_rtnExtDataProcessor::done" )
   INT32 _rtnExtDataProcessor::done( pmdEDUCB *cb )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_DONE ) ;
      if ( _needUpdateLSN )
      {
         SDB_ASSERT( _mbContext, "mbContext should not be NULL" ) ;
         _mbContext->mbStat()->updateLastLSN( cb->getEndLsn(), DMS_FILE_DATA ) ;
      }
      _keySet.clear() ;
      _keySetNew.clear() ;
      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSOR_DONE ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_ABORT, "_rtnExtDataProcessor::abort" )
   INT32 _rtnExtDataProcessor::abort()
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_ABORT ) ;
      _needUpdateLSN = FALSE ;
      _keySet.clear() ;
      _keySetNew.clear() ;
      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSOR_ABORT ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR__ISOWNEDBY, "_rtnExtDataProcessor::isOwnedBy" )
   BOOLEAN _rtnExtDataProcessor::isOwnedBy( const CHAR *csName,
                                            const CHAR *clName,
                                            const CHAR *idxName )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR__ISOWNEDBY ) ;
      BOOLEAN result = FALSE ;

      SDB_ASSERT( csName, "CS name can not be NULL" ) ;

      if ( 0 == ossStrcmp( csName, _meta._csName.c_str() ) )
      {
         result = TRUE ;
      }

      if ( clName && 0 != ossStrcmp( clName, _meta._clName.c_str() ) && result )
      {
         result = FALSE ;
      }

      if ( idxName && 0 != ossStrcmp( idxName, _meta._idxName.c_str() )
           && result )
      {
         result = FALSE ;
      }

      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSOR__ISOWNEDBY ) ;
      return result ;
   }

   BOOLEAN _rtnExtDataProcessor::isOwnedByExt( const CHAR *targetName )
   {
      return ( 0 == ossStrcmp( targetName, _meta._targetName.c_str() ) ) ;
   }

   // The format of external data name is: SYS_<cl unique id>_<idx logical id>
   void _rtnExtDataProcessor::genExtDataNames( utilCLUniqueID clUniqID,
                                               const CHAR *idxName,
                                               string &extCSName,
                                               string &extCLName )
   {
      ostringstream name ;

      if ( UTIL_INVALID_UNIQUEID == clUniqID )
      {
         return ;
      }

      name << SYS_PREFIX"_" << clUniqID << "_" << idxName ;
      extCSName = name.str() ;
      extCLName = extCSName + "." + extCSName ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR__PREPARERECORD, "_rtnExtDataProcessor::_prepareRecord" )
   INT32 _rtnExtDataProcessor::_prepareRecord( _rtnExtOprType oprType,
                                               const BSONElement &idEle,
                                               const BSONObj *dataObj,
                                               BSONObj &recordObj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR__PREPARERECORD ) ;
      BSONObjBuilder objBuilder ;

      if ( RTN_EXT_INVALID == oprType )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid operation type[ %d ]", oprType ) ;
         goto error ;
      }

      try
      {
         // 1. Append operation type.
         objBuilder.append( FIELD_NAME_TYPE, oprType ) ;
         // 2. Append the _id as _rid.
         objBuilder.appendAs( idEle, RTN_FIELD_NAME_RID ) ;
         // 3. Append data if necessarry.
         if ( RTN_EXT_INSERT == oprType || RTN_EXT_UPDATE == oprType )
         {
            if ( !dataObj )
            {
               rc = SDB_SYS ;
               PD_LOG( PDERROR, "Data object is NULL for operation type[ %d ]",
                       oprType ) ;
               goto error ;
            }
            objBuilder.append( RTN_FIELD_NAME_SOURCE, *dataObj ) ;
         }

         recordObj = objBuilder.obj() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR__PREPARERECORD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR__PREPARECSANDCL, "_rtnExtDataProcessor::_prepareCSAndCL" )
   INT32 _rtnExtDataProcessor::_prepareCSAndCL( const CHAR *csName,
                                                const CHAR *clName,
                                                pmdEDUCB *cb,
                                                SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR__PREPARECSANDCL ) ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;
      BOOLEAN csCreated = FALSE ;
      BSONObjBuilder builder ;
      BSONObj extOptions ;

      rc = rtnCreateCollectionSpaceCommand( csName, cb, dmsCB, dpsCB,
                                            UTIL_INVALID_UNIQUEID,
                                            DMS_PAGE_SIZE_DFT,
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
                                       cb, dmsCB, dpsCB, UTIL_INVALID_UNIQUEID,
                                       UTIL_COMPRESSOR_INVALID, 0,
                                       TRUE, &extOptions ) ;
      PD_RC_CHECK( rc, PDERROR, "Create capped collection[ %s ] failed[ %d ]",
                   clName, rc ) ;
      rtnCB->incTextIdxVersion() ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR__PREPARECSANDCL, rc ) ;
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

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_PREPAREINSERT, "_rtnExtDataProcessor::prepareInsert" )
   INT32 _rtnExtDataProcessor::_prepareInsert( const BSONObj &inputObj,
                                               BSONObj &recordObj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_PREPAREINSERT ) ;

      if ( !_needOprRec )
      {
         goto done ;
      }

      try
      {
         BSONElement ele ;
         if ( 0 == _keySet.size() )
         {
            // No index key in the record, skip.
            goto done ;
         }

         // Get the _id from the insert object.
         ele = inputObj.getField( DMS_ID_KEY_NAME ) ;
         if ( ele.eoo() )
         {
            PD_LOG( PDERROR, "Text index can not be used if record has no _id "
                    "field" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         {
            BSONObjSet::iterator it = _keySet.begin();
            BSONObj object( *it ) ;
            rc = _prepareRecord( RTN_EXT_INSERT, ele, &object, recordObj ) ;
            PD_RC_CHECK( rc, PDERROR, "Add operation record failed[ %d ]",
                         rc ) ;
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_PREPAREINSERT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_PREPAREDELETE, "_rtnExtDataProcessor::prepareDelete" )
   INT32 _rtnExtDataProcessor::_prepareDelete( const BSONObj &inputObj,
                                               BSONObj &recordObj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_PREPAREDELETE ) ;

      if ( !_needOprRec )
      {
         goto done ;
      }

      try
      {
         BSONElement ele ;
         if ( 0 == _keySet.size() )
         {
            // No index key in the record, skip.
            goto done ;
         }

         ele = inputObj.getField( DMS_ID_KEY_NAME ) ;
         if ( EOO == ele.type() )
         {
            PD_LOG( PDERROR, "Text index can not be used if record has no _id "
                  "field" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         rc = _prepareRecord( RTN_EXT_DELETE, ele, NULL, recordObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Add operation record failed[ %d ]",
                      rc ) ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_PREPAREDELETE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR_PREPAREUPDATE, "_rtnExtDataProcessor::prepareUpdate" )
   INT32 _rtnExtDataProcessor::_prepareUpdate( const BSONObj &originalObj,
                                               const BSONObj &newObj,
                                               BSONObj &recordObj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR_PREPAREUPDATE ) ;

      if ( !_needOprRec )
      {
         goto done ;
      }

      try
      {
         BSONElement ele ;
         if ( 0 == _keySetNew.size() )
         {
            rc = _prepareDelete( originalObj, recordObj ) ;
            PD_RC_CHECK( rc, PDERROR, "Prepare for delete failed[ %d ]", rc ) ;
            goto done ;
         }

         ele = newObj.getField( DMS_ID_KEY_NAME ) ;
         if ( EOO == ele.type() )
         {
            PD_LOG( PDERROR, "Text index can not be used if record has no _id "
                  "field" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         SDB_ASSERT( 1 == _keySetNew.size(), "Key set size should be 1" ) ;

         {
            BSONObjSet::iterator it = _keySetNew.begin();
            BSONObj object( *it ) ;
            rc = _prepareRecord( RTN_EXT_UPDATE, ele, &object, recordObj ) ;
            PD_RC_CHECK( rc, PDERROR, "Add operation record failed[ %d ]",
                         rc ) ;
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSOR_PREPAREUPDATE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSOR__GETEXTCLSHORTNAME, "_rtnExtDataProcessor::_getExtCLShortName" )
   const CHAR* _rtnExtDataProcessor::_getExtCLShortName() const
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSOR__GETEXTCLSHORTNAME ) ;
      UINT32 curPos = 0 ;
      UINT32 fullLen = (UINT32)ossStrlen( _cappedCLName ) ;
      const CHAR *clShortName = NULL ;

      if ( 0 == fullLen )
      {
         clShortName = NULL ;
         goto done ;
      }

      while ( ( curPos < fullLen ) && ( '.' != _cappedCLName[curPos] ) )
      {
         curPos++ ;
      }

      if ( ( curPos == fullLen ) || ( curPos == fullLen - 1 ) )
      {
         clShortName = NULL ;
         goto done ;
      }

      clShortName = _cappedCLName + curPos + 1 ;

   done:
      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSOR__GETEXTCLSHORTNAME ) ;
      return clShortName ;
   }

   _rtnExtDataProcessorMgr::_rtnExtDataProcessorMgr()
   : _processorID( 0 )
   {
   }

   _rtnExtDataProcessorMgr::~_rtnExtDataProcessorMgr()
   {
      for ( PROCESSOR_MAP_ITR itr = _processorMap.begin();
            itr != _processorMap.end(); ++itr )
      {
         SDB_OSS_DEL itr->second ;
      }

      for ( PROCESSOR_LATCH_MAP_ITR itr = _latchMap.begin();
            itr != _latchMap.end(); ++itr )
      {
         SDB_OSS_DEL itr->second ;
      }
   }

   // activate - To make the processor visible to others.
   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSORMGR_CREATEPROCESSOR, "_rtnExtDataProcessorMgr::createProcessor" )
   INT32 _rtnExtDataProcessorMgr::createProcessor( const CHAR *csName,
                                                   const CHAR *clName,
                                                   const CHAR *idxName,
                                                   const CHAR *extName,
                                                   const BSONObj &idxKeyDef,
                                                   rtnExtDataProcessor *&processor,
                                                   BOOLEAN activate )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_CREATEPROCESSOR ) ;
      rtnExtDataProcessor *processorLocal = NULL ;

      processorLocal = SDB_OSS_NEW rtnExtDataProcessor() ;
      if ( !processorLocal )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Allocate memory for external data processor failed, "
                 "size[ %d ]", sizeof( rtnExtDataProcessor ) ) ;
         goto error ;
      }

      rc = processorLocal->init( _processorID.inc(), csName, clName,
                                 idxName, extName, idxKeyDef ) ;
      PD_RC_CHECK( rc, PDERROR, "Init external data processor failed[ %d ]",
                   rc ) ;

      if ( activate )
      {
         rc = activateProcessor( processorLocal ) ;
         PD_RC_CHECK( rc, PDERROR, "Activate processor failed[ %d ]", rc ) ;
      }

      processor = processorLocal ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSORMGR_CREATEPROCESSOR, rc ) ;
      return rc ;
   error:
      if ( processorLocal )
      {
         SDB_OSS_DEL processorLocal ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSORMGR_ACTIVATEPROCESSOR, "_rtnExtDataProcessorMgr::activateProcessor" )
   INT32 _rtnExtDataProcessorMgr::activateProcessor( rtnExtDataProcessor *processor )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_ACTIVATEPROCESSOR ) ;
      ossRWMutex *mutex = NULL ;

      SDB_ASSERT( processor, "Processor is NULL" ) ;

      // Check if it has been activated already.
      PROCESSOR_MAP_ITR itr = _processorMap.find( processor->getID() ) ;
      if ( itr != _processorMap.end() )
      {
         goto done ;
      }

      mutex = SDB_OSS_NEW ossRWMutex ;
      if ( !mutex )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Allocate memory for mutex failed, size[ %d ]",
                 sizeof( ossRWMutex ) ) ;
         goto error ;
      }

      // Add processor metadata.
      _mutex.get() ;
      // We use the default name of the processors( which are the full names of
      // the capped collections. Each of them are unique.
      _processorMap[ processor->getID() ] = processor ;
      _latchMap[ processor->getID() ] = mutex ;
      _mutex.release() ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSORMGR_ACTIVATEPROCESSOR, rc ) ;
      return rc ;
   error:
      if ( mutex )
      {
         SDB_OSS_DEL mutex ;
      }
      goto done ;
   }

   void _rtnExtDataProcessorMgr::rmProcessorEntry( vector<rtnExtDataProcessor *> &processors,
                                                   INT32 lockType )
   {
      ossScopedLock lock( &_mutex, EXCLUSIVE ) ;

      for ( vector<rtnExtDataProcessor *>::iterator itr = processors.begin();
            itr != processors.end(); ++itr )
      {
         PROCESSOR_LATCH_MAP_ITR mItr = _latchMap.find( (*itr)->getID() ) ;
         if ( SHARED == lockType )
         {
            mItr->second->release_r() ;
         }
         else if ( EXCLUSIVE == lockType )
         {
            mItr->second->release_w() ;
         }
         _processorMap.erase( (*itr)->getID() ) ;
         _latchMap.erase( mItr ) ;
      }
   }

   void _rtnExtDataProcessorMgr::destroyProcessor( vector<rtnExtDataProcessor *> &processors )
   {
      for ( vector<rtnExtDataProcessor *>::iterator itr = processors.begin();
            itr != processors.end(); ++itr )
      {
         SDB_OSS_DEL (*itr) ;
      }

      processors.clear() ;
   }

   UINT32 _rtnExtDataProcessorMgr::number()
   {
      ossScopedLock lock( &_mutex, SHARED ) ;
      return _processorMap.size() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORSBYCS, "_rtnExtDataProcessorMgr::getProcessorsByCS" )
   INT32 _rtnExtDataProcessorMgr::getProcessorsByCS( const CHAR *csName,
                                                     INT32 lockType,
                                                     vector<rtnExtDataProcessor *> &processors )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORSBYCS ) ;

      BOOLEAN lock = ( SHARED == lockType || EXCLUSIVE == lockType ) ;
      vector<ossRWMutex *> lockedProcessors ;

      for ( PROCESSOR_MAP_ITR itr = _processorMap.begin();
            itr != _processorMap.end(); ++itr )
      {
         if ( itr->second->isOwnedBy( csName ) )
         {
            ossRWMutex *mutex = _latchMap.find( itr->first )->second ;
            if ( SHARED == lockType )
            {
               rc = mutex->lock_r( OSS_ONE_SEC ) ;
            }
            else if ( EXCLUSIVE == lockType )
            {
               rc = mutex->lock_w( OSS_ONE_SEC ) ;
            }
            else
            {
               processors.push_back( itr->second ) ;
               continue ;
            }

            // In case of error, all processors which have been locked need to
            // be released.
            if ( rc )
            {
               goto error ;
            }

            if ( lock )
            {
               lockedProcessors.push_back( mutex ) ;
            }
            processors.push_back( itr->second ) ;
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORSBYCS, rc ) ;
      return rc ;
   error:
      processors.clear() ;
      if ( lock )
      {
         for ( vector<ossRWMutex *>::iterator itr = lockedProcessors.begin();
               itr != lockedProcessors.end(); ++itr )
         {
            if ( SHARED == lockType )
            {
               (*itr)->release_r() ;
            }
            else
            {
               (*itr)->release_w() ;
            }
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORSBYCL, "_rtnExtDataProcessorMgr::getProcessorsByCL" )
   INT32 _rtnExtDataProcessorMgr::getProcessorsByCL( const CHAR *csName,
                                                     const CHAR *clName,
                                                     INT32 lockType,
                                                     std::vector<rtnExtDataProcessor *> &processors )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORSBYCL ) ;

      BOOLEAN lock = ( SHARED == lockType || EXCLUSIVE == lockType ) ;
      vector<ossRWMutex *> lockedProcessors ;

      for ( PROCESSOR_MAP_ITR itr = _processorMap.begin();
            itr != _processorMap.end(); ++itr )
      {
         if ( itr->second->isOwnedBy( csName, clName ) )
         {
            ossRWMutex *mutex = _latchMap.find( itr->first )->second ;
            if ( SHARED == lockType )
            {
               rc = mutex->lock_r( OSS_ONE_SEC ) ;
            }
            else if ( EXCLUSIVE == lockType )
            {
               rc = mutex->lock_w( OSS_ONE_SEC ) ;
            }
            else
            {
               continue ;
            }
            if ( rc )
            {
               goto error ;
            }

            if ( lock )
            {
               lockedProcessors.push_back( mutex ) ;
            }
            processors.push_back( itr->second ) ;
         }
      }

      // Currently only support 1 text index for one collection.
      SDB_ASSERT( processors.size() <=1, "Processors on cl greater than 1" ) ;

   done:
      PD_TRACE_EXITRC( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORSBYCL, rc ) ;
      return rc ;
   error:
      processors.clear() ;
      if ( lock )
      {
         for ( vector<ossRWMutex *>::iterator itr = lockedProcessors.begin();
               itr != lockedProcessors.end(); ++itr )
         {
            if ( SHARED == lockType )
            {
               (*itr)->release_r() ;
            }
            else
            {
               (*itr)->release_w() ;
            }
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORBYIDX, "_rtnExtDataProcessorMgr::getProcessorByIdx" )
   INT32 _rtnExtDataProcessorMgr::getProcessorByIdx( const CHAR *csName,
                                                     const CHAR *clName,
                                                     const CHAR *idxName,
                                                     INT32 lockType,
                                                     rtnExtDataProcessor *&processor )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORBYIDX ) ;

      processor = NULL ;

      for ( PROCESSOR_MAP_ITR itr = _processorMap.begin();
            itr != _processorMap.end(); ++itr )
      {
         if ( itr->second->isOwnedBy( csName, clName, idxName ) )
         {
            ossRWMutex *mutex = _latchMap.find( itr->first )->second ;
            if ( SHARED == lockType )
            {
               mutex->lock_r() ;
            }
            else if ( EXCLUSIVE == lockType )
            {
               mutex->lock_w() ;
            }
            processor = itr->second ;
            break ;
         }
      }

      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORBYIDX ) ;
      return SDB_OK ;
   }

   INT32 _rtnExtDataProcessorMgr::getProcessorByExtName( const CHAR *extName,
                                                         INT32 lockType,
                                                         rtnExtDataProcessor *&processor )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORBYIDX ) ;

      processor = NULL  ;
      for ( PROCESSOR_MAP_ITR itr = _processorMap.begin();
            itr != _processorMap.end(); ++itr )
      {
         if ( itr->second->isOwnedByExt( extName ) )
         {
            processor = itr->second ;
            ossRWMutex *mutex = _latchMap.find( itr->first )->second ;
            if ( SHARED == lockType )
            {
               mutex->lock_r() ;
            }
            else if ( EXCLUSIVE == lockType )
            {
               mutex->lock_w() ;
            }
            processor = itr->second ;
            break ;
         }
      }

      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSORMGR_GETPROCESSORBYIDX ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSORMGR_UNLOCKPROCESSOR, "_rtnExtDataProcessorMgr::unlockProcessor" )
   void _rtnExtDataProcessorMgr::unlockProcessor( INT64 processorID,
                                                  INT32 lockType )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_UNLOCKPROCESSOR ) ;

      if ( SHARED != lockType && EXCLUSIVE != lockType )
      {
         return ;
      }

      PROCESSOR_LATCH_MAP_ITR itr = _latchMap.find( processorID ) ;
      if ( itr != _latchMap.end() )
      {
         if ( SHARED == lockType )
         {
            itr->second->release_r() ;
         }
         else
         {
            itr->second->release_w() ;
         }
      }
      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSORMGR_UNLOCKPROCESSOR ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSORMGR_UNLOCKPROCESSORS, "_rtnExtDataProcessorMgr::unlockProcessors" )
   void _rtnExtDataProcessorMgr::unlockProcessors( std::vector<rtnExtDataProcessor *> &processors,
                                                   INT32 lockType )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_UNLOCKPROCESSORS ) ;
      if ( SHARED != lockType && EXCLUSIVE != lockType )
      {
         return ;
      }

      {
         ossScopedLock lock(&_mutex, SHARED);
         for (std::vector<rtnExtDataProcessor *>::iterator itr = processors.begin();
              itr != processors.end(); ++itr) {
            PROCESSOR_LATCH_MAP_ITR latchItr = _latchMap.find((*itr)->getID());
            if (latchItr != _latchMap.end()) {
               if (SHARED == lockType) {
                  latchItr->second->release_r();
               } else if (EXCLUSIVE == lockType) {
                  latchItr->second->release_w();
               }
            }
         }
      }
      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSORMGR_UNLOCKPROCESSORS ) ;
      }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNEXTDATAPROCESSORMGR_DELPROCESSOR, "_rtnExtDataProcessorMgr::delProcessor" )
   void _rtnExtDataProcessorMgr::delProcessor( rtnExtDataProcessor **processor )
   {
      PD_TRACE_ENTRY( SDB__RTNEXTDATAPROCESSORMGR_DELPROCESSOR ) ;

      if ( !processor || !(*processor ) )
      {
         return ;
      }

      {
         ossScopedLock _lock( &_mutex, EXCLUSIVE ) ;
         PROCESSOR_MAP_ITR itr = _processorMap.find( (*processor)->getID() ) ;
         if ( itr != _processorMap.end() )
         {
            _processorMap.erase( itr ) ;
            _latchMap.erase( (*processor)->getID() ) ;
         }
      }

      SDB_OSS_DEL (*processor ) ;
      *processor = NULL ;

      PD_TRACE_EXIT( SDB__RTNEXTDATAPROCESSORMGR_DELPROCESSOR ) ;
   }

   rtnExtDataProcessorMgr* rtnGetExtDataProcessorMgr()
   {
      static rtnExtDataProcessorMgr s_edpMgr ;
      return &s_edpMgr ;
   }
}

