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

   Source File Name = dmsStorageUnit.cpp

   Descriptive Name = Data Management Service Storage Unit

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains code logic for
   data insert/update/delete. This file does NOT include index logic.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "dmsStorageUnit.hpp"
#include "dmsScanner.hpp"
#include "mthModifier.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "dmsTrace.hpp"

namespace engine
{
   _dmsStorageUnit::_dmsStorageUnit ( const CHAR *pSUName, UINT32 sequence,
                                      INT32 pageSize )
   :_apm(this)
   {
      SDB_ASSERT ( pSUName, "name can't be null" ) ;

      if ( 0 == pageSize )
      {
         pageSize = DMS_PAGE_SIZE_DFT ;
      }

      CHAR dataFileName[DMS_SU_FILENAME_SZ + 1] = {0} ;
      CHAR idxFileName[DMS_SU_FILENAME_SZ + 1] = {0} ;

      _storageInfo._pageSize = pageSize ;
      ossStrncpy( _storageInfo._suName, pSUName, DMS_SU_NAME_SZ ) ;
      _storageInfo._suName[DMS_SU_NAME_SZ] = 0 ;
      _storageInfo._sequence = sequence ;
      // make secret value
      _storageInfo._secretValue = ossPack32To64( (UINT32)time(NULL),
                                                 (UINT32)(ossRand()*239641) ) ;

      ossSnprintf( dataFileName, DMS_SU_FILENAME_SZ, "%s.%d.%s",
                   _storageInfo._suName, sequence, DMS_DATA_SU_EXT_NAME ) ;
      ossSnprintf( idxFileName, DMS_SU_FILENAME_SZ, "%s.%d.%s",
                   _storageInfo._suName, sequence, DMS_INDEX_SU_EXT_NAME ) ;

      _pDataSu = SDB_OSS_NEW dmsStorageData( dataFileName, &_storageInfo ) ;
      if ( _pDataSu )
      {
         _pIndexSu = SDB_OSS_NEW dmsStorageIndex( idxFileName, &_storageInfo,
                                                  _pDataSu ) ;
      }
   }

   _dmsStorageUnit::~_dmsStorageUnit()
   {
      close() ;

      if ( _pIndexSu )
      {
         SDB_OSS_DEL _pIndexSu ;
         _pIndexSu = NULL ;
      }
      if ( _pDataSu )
      {
         SDB_OSS_DEL _pDataSu ;
         _pDataSu = NULL ;
      }
   }

   INT32 _dmsStorageUnit::open( const CHAR *pDataPath, const CHAR *pIndexPath,
                                BOOLEAN createNew, BOOLEAN delWhenExist )
   {
      INT32 rc = SDB_OK ;

      if ( !_pDataSu || !_pIndexSu )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Alloc memory failed" ) ;
         goto error ;
      }

      // open data
      rc = _pDataSu->openStorage( pDataPath, createNew, delWhenExist ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Open storage data su failed, rc: %d", rc ) ;
         if ( createNew && SDB_FE != rc )
         {
            goto rmdata ;
         }
         goto error ;
      }
      // open index
      rc = _pIndexSu->openStorage( pIndexPath, createNew, delWhenExist ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Open storage index su failed, rc: %d", rc ) ;
         if ( createNew )
         {
            if ( SDB_FE != rc )
            {
               goto rmboth ;
            }
            goto rmdata ;
         }
         goto error ;
      }

   done:
      return rc ;
   error:
      close() ;
      goto done ;
   rmdata:
      {
         INT32 rcTmp = _pDataSu->removeStorage() ;
         if ( rcTmp )
         {
            PD_LOG( PDWARNING, "Failed to remove cs data file[%s] in "
                    "rollback, rc: %d", _pDataSu->getSuFileName(), rc ) ;
         }
      }
      goto done ;
   rmboth:
      {
         INT32 rcTmp = _pIndexSu->removeStorage() ;
         if ( rcTmp )
         {
            PD_LOG( PDWARNING, "Failed to remove cs idnex file[%s] in "
                    "rollback, rc: %d", _pIndexSu->getSuFileName(), rc ) ;
         }
      }
      goto rmdata ;
   }

   void _dmsStorageUnit::close ()
   {
      if ( _pIndexSu )
      {
         _pIndexSu->closeStorage() ;
      }
      if ( _pDataSu )
      {
         _pDataSu->closeStorage() ;
      }
   }

   INT32 _dmsStorageUnit::remove ()
   {
      INT32 rc = SDB_OK ;

      if ( _pDataSu )
      {
         rc = _pDataSu->removeStorage() ;
         PD_RC_CHECK( rc, PDERROR, "Failed to remove collection space[%s] "
                      "data file, rc: %d", CSName(), rc ) ;
      }

      if ( _pIndexSu )
      {
         rc = _pIndexSu->removeStorage() ;
         PD_RC_CHECK( rc, PDERROR, "Failed to remove collection space[%s] "
                      "index file, rc: %d", CSName(), rc ) ;
      }

      PD_LOG( PDEVENT, "Remove collection space[%s] files succeed", CSName() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageUnit::_resetCollection( dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;

      SDB_ASSERT( context, "context can't be NULL" ) ;

      // drop all indexes
      rc = _pIndexSu->dropAllIndexes( context, NULL, NULL ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Drop all indexes failed, rc: %d", rc ) ;
         // don't go to error, continue
      }

      rc = _pDataSu->_truncateCollection( context ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Truncate collection data failed, rc: %d", rc ) ;
      }

      return rc ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSU_LDEXTA, "_dmsStorageUnit::loadExtentA" )
   INT32 _dmsStorageUnit::loadExtentA ( dmsMBContext *mbContext,
                                        const CHAR *pBuffer,
                                        UINT16 numPages,
                                        const BOOLEAN toLoad,
                                        SINT32 *tAllocatedExtent,
                                        dmsExtent **tExtAddr )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DMSSU_LDEXTA ) ;

      dmsExtent *sourceExt  = (dmsExtent*)pBuffer ;
      dmsExtent *extAddr = NULL ;
      SINT32 allocatedExtent = DMS_INVALID_EXTENT ;

      // allocate a new extent
      rc = _pDataSu->_allocateExtent( mbContext, numPages, FALSE, toLoad,
                                      &allocatedExtent ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Can't allocate extent for %d pages, rc = %d",
                  numPages, rc ) ;
         goto error ;
      }

      // get the address
      extAddr = (dmsExtent*)_pDataSu->extentAddr ( allocatedExtent ) ;
      // copy data part
      ossMemcpy ( &((CHAR*)extAddr)[DMS_EXTENT_METADATA_SZ],
                  &pBuffer[DMS_EXTENT_METADATA_SZ],
                  _pDataSu->pageSize() * numPages  - DMS_EXTENT_METADATA_SZ ) ;

      // reset header part
      extAddr->_recCount          = sourceExt->_recCount ;
      extAddr->_firstRecordOffset = sourceExt->_firstRecordOffset ;
      extAddr->_lastRecordOffset  = sourceExt->_lastRecordOffset ;
      extAddr->_freeSpace         = sourceExt->_freeSpace ;

      if ( tAllocatedExtent )
      {
         *tAllocatedExtent = allocatedExtent ;
      }
      if ( tExtAddr )
      {
         *tExtAddr = extAddr ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__DMSSU_LDEXTA, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSU_LDEXT, "_dmsStorageUnit::loadExtent" )
   INT32 _dmsStorageUnit::loadExtent ( dmsMBContext *mbContext,
                                       const CHAR *pBuffer,
                                       UINT16 numPages )
   {
      INT32 rc                 = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DMSSU_LDEXT ) ;

      SINT32 allocatedExtent   = DMS_INVALID_EXTENT ;
      dmsExtent *extAddr       = NULL ;
      SDB_ASSERT ( pBuffer, "buffer can't be NULL" )

      rc = loadExtentA ( mbContext, pBuffer, numPages, FALSE,
                         &allocatedExtent, &extAddr ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to loadExtentA, rc = %d",
                  numPages, rc ) ;
         goto error ;
      }

      // reset delete list
      _pDataSu->_mapExtent2DelList( mbContext->mb(), extAddr,
                                    allocatedExtent ) ;
      // add count
      addExtentRecordCount( mbContext->mb(), extAddr->_recCount ) ;

   done :
      PD_TRACE_EXITRC ( SDB__DMSSU_LDEXT, rc );
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::insertRecord ( const CHAR *pName,
                                         BSONObj &record,
                                         pmdEDUCB *cb,
                                         SDB_DPSCB *dpscb,
                                         BOOLEAN mustOID,
                                         BOOLEAN canUnLock,
                                         dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, -1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }

      rc = _pDataSu->insertRecord( context, record, cb, dpscb, mustOID,
                                   canUnLock ) ;
      if ( rc )
      {
         goto error ;
      }

   done :
      if ( getContext && context )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::updateRecords ( const CHAR *pName,
                                          pmdEDUCB *cb,
                                          SDB_DPSCB *dpscb,
                                          mthMatcher *matcher,
                                          mthModifier &modifier,
                                          SINT64 &numRecords,
                                          SINT64 maxUpdate,
                                          dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, EXCLUSIVE ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }
      else
      {
         rc = context->mbLock( EXCLUSIVE ) ;
         PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;
      }

      {
         dmsRecordID recordID ;
         ossValuePtr recordDataPtr = 0 ;
         numRecords = 0 ;
         dmsTBScanner tbScanner( _pDataSu, context, matcher,
                                 DMS_ACCESS_TYPE_UPDATE, maxUpdate ) ;
         while ( SDB_OK == ( rc = tbScanner.advance( recordID, recordDataPtr,
                                                     cb ) ) )
         {
            rc = _pDataSu->updateRecord( context, recordID, recordDataPtr, cb,
                                         dpscb, modifier ) ;
            PD_RC_CHECK( rc, PDERROR, "Update record failed, rc: %d", rc ) ;

            ++numRecords ;
         }

         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get next record, rc: %d", rc ) ;
            goto error ;
         }
      }

   done :
      if ( context && getContext )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::deleteRecords ( const CHAR *pName,
                                          pmdEDUCB * cb,
                                          SDB_DPSCB *dpscb,
                                          mthMatcher *matcher,
                                          SINT64 &numRecords,
                                          SINT64 maxDelete,
                                          dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, EXCLUSIVE ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }
      else
      {
         rc = context->mbLock( EXCLUSIVE ) ;
         PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;
      }

      {
         dmsRecordID recordID ;
         ossValuePtr recordDataPtr = 0 ;
         numRecords = 0 ;
         dmsTBScanner tbScanner( _pDataSu, context, matcher,
                                 DMS_ACCESS_TYPE_DELETE, maxDelete ) ;
         while ( SDB_OK == ( rc = tbScanner.advance( recordID, recordDataPtr,
                                                     cb ) ) )
         {
            rc = _pDataSu->deleteRecord( context, recordID, recordDataPtr,
                                         cb, dpscb ) ;
            PD_RC_CHECK( rc, PDERROR, "Delete record failed, rc: %d", rc ) ;

            ++numRecords ;
         }

         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
         }
         else if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get next record, rc: %d", rc ) ;
            goto error ;
         }
      }

   done :
      if ( context && getContext )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::rebuildIndexes( const CHAR *pName,
                                          pmdEDUCB * cb,
                                          dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, -1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }

      rc = _pIndexSu->rebuildIndexes( context, cb ) ;
      if ( rc )
      {
         goto error ;
      }

   done :
      if ( context && getContext )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::createIndex( const CHAR *pName, const BSONObj &index,
                                       pmdEDUCB *cb, SDB_DPSCB *dpscb,
                                       BOOLEAN isSys, dmsMBContext * context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, -1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }

      rc = _pIndexSu->createIndex( context, index, cb, dpscb, isSys ) ;
      if ( rc )
      {
         goto error ;
      }

   done :
      if ( context && getContext )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::dropIndex( const CHAR *pName, const CHAR *indexName,
                                     pmdEDUCB *cb, SDB_DPSCB *dpscb,
                                     BOOLEAN isSys, dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, -1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }

      rc = _pIndexSu->dropIndex( context, indexName, cb, dpscb, isSys ) ;
      if ( rc )
      {
         goto error ;
      }

   done :
      if ( context && getContext )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::dropIndex( const CHAR *pName, OID &indexOID,
                                     pmdEDUCB *cb, SDB_DPSCB *dpscb,
                                     BOOLEAN isSys, dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, -1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }

      rc = _pIndexSu->dropIndex( context, indexOID, cb, dpscb, isSys ) ;
      if ( rc )
      {
         goto error ;
      }

   done :
      if ( context && getContext )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::countCollection ( const CHAR *pName,
                                            INT64 &recordNum,
                                            pmdEDUCB *cb,
                                            dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;
      //dmsExtent *pExtent           = NULL ;
      recordNum                    = 0 ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, -1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }

      /*{
         dmsExtentItr itr( _pDataSu, context ) ;
         while ( SDB_OK == ( rc = itr.next( &pExtent, cb ) ) )
         {
            recordNum += pExtent->_recCount ;
         }
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
         }
      }*/
      if ( !context->isMBLock() )
      {
         rc = context->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to lock dms mb context[%s], rc: %d",
                      context->toString().c_str(), rc ) ;
      }
      recordNum = context->mbStat()->_totalRecords ;

   done :
      if ( getContext && context )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::getCollectionFlag( const CHAR *pName, UINT16 &flag,
                                             dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }
      else if ( !context->isMBLock() )
      {
         rc = context->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Lock collection failed, rc: %d", rc ) ;
      }

      flag = context->mb()->_flag ;

   done :
      if ( getContext && context )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::changeCollectionFlag( const CHAR *pName, UINT16 flag,
                                                dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }
      else if ( !context->isMBLock() )
      {
         rc = context->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Lock collection failed, rc: %d", rc ) ;
      }

      context->mb()->_flag = flag ;

   done :
      if ( getContext && context )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::getCollectionAttributes( const CHAR *pName,
                                                   UINT32 &attributes,
                                                   dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }
      else if ( !context->isMBLock() )
      {
         rc = context->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Lock collection failed, rc: %d", rc ) ;
      }

      attributes = context->mb()->_attributes ;

   done :
      if ( getContext && context )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::updateCollectionAttributes( const CHAR *pName,
                                                      UINT32 newAttributes,
                                                      dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }
      else if ( !context->isMBLock() )
      {
         rc = context->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Lock collection failed, rc: %d", rc ) ;
      }

      context->mb()->_attributes = newAttributes ;

   done :
      if ( getContext && context )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::getSegExtents( const CHAR *pName,
                                         vector < dmsExtentID > &segExtents,
                                         dmsMBContext *context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;
      dmsMBEx *mbEx                = NULL ;
      dmsExtentID firstID          = DMS_INVALID_EXTENT ;

      segExtents.clear() ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }
      else
      {
         rc = context->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;
      }

      if ( DMS_INVALID_EXTENT == context->mb()->_mbExExtentID ||
           NULL == ( mbEx = ( dmsMBEx* )_pDataSu->extentAddr(
           context->mb()->_mbExExtentID ) ) )
      {
         PD_LOG( PDERROR, "Invalid meta extent id: %d, collection name: %s",
                 context->mb()->_mbExExtentID,
                 context->mb()->_collectionName ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      for ( UINT32 i = 0 ; i < mbEx->_header._segNum ; ++i )
      {
         mbEx->getFirstExtentID( i, firstID ) ;
         if ( DMS_INVALID_EXTENT != firstID )
         {
            segExtents.push_back( firstID ) ;
         }
      }

   done :
      if ( context && getContext )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStorageUnit::getIndexes( const CHAR *pName,
                                      vector< _monIndex > &resultIndexes,
                                      dmsMBContext * context )
   {
      INT32 rc                     = SDB_OK ;
      BOOLEAN getContext           = FALSE ;
      UINT32 indexID               = 0 ;
      monIndex indexItem ;

      if ( NULL == context )
      {
         SDB_ASSERT( pName, "Collection name can't be NULL" ) ;

         rc = _pDataSu->getMBContext( &context, pName, SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "Get collection[%s] mb context failed, "
                      "rc: %d", pName, rc ) ;
         getContext = TRUE ;
      }
      else
      {
         rc = context->mbLock( SHARED ) ;
         PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;
      }

      for ( indexID = 0 ; indexID < DMS_COLLECTION_MAX_INDEX ; ++indexID )
      {
         if ( DMS_INVALID_EXTENT == context->mb()->_indexExtent[indexID] )
         {
            break ;
         }

         ixmIndexCB indexCB ( context->mb()->_indexExtent[indexID],
                              _pIndexSu, NULL ) ;
         indexItem._indexFlag = indexCB.getFlag () ;
         indexItem._scanExtLID = indexCB.scanExtLID () ;
         indexItem._version = indexCB.version () ;
         // copy the index def to it's owned buffer
         indexItem._indexDef = indexCB.getDef().copy () ;
         // add
         resultIndexes.push_back ( indexItem ) ;
      }

   done :
      if ( context && getContext )
      {
         _pDataSu->releaseMBContext( context ) ;
      }
      return rc ;
   error :
      goto done ;
   }

   void _dmsStorageUnit::dumpInfo ( vector<CHAR*> &collectionList,
                                    BOOLEAN sys )
   {
      // lock meta data
      _pDataSu->_metadataLatch.get_shared() ;

      dmsStorageData::COLNAME_MAP_IT it = _pDataSu->_collectionNameMap.begin() ;
      while ( it != _pDataSu->_collectionNameMap.end() )
      {
         if ( !sys && dmsIsSysCLName( it->first ) )
         {
            ++it ;
            continue ;
         }

         CHAR *pBuffer = (CHAR*)SDB_OSS_MALLOC ( DMS_COLLECTION_NAME_SZ + 1 ) ;
         if ( !pBuffer )
         {
            PD_LOG( PDERROR, "Allocate memory failed" ) ;
            goto error ;
         }
         ossStrncpy ( pBuffer, it->first, DMS_COLLECTION_NAME_SZ ) ;
         pBuffer[ DMS_COLLECTION_NAME_SZ ] = 0 ;
         // add
         collectionList.push_back ( pBuffer ) ;

         ++it ;
      }

   done :
      // release meta lock
      _pDataSu->_metadataLatch.release_shared() ;
      return ;
   error :
      goto done ;
   }

   void _dmsStorageUnit::dumpInfo ( set<_monCollection> &collectionList,
                                    BOOLEAN sys )
   {
      dmsMB *mb = NULL ;
      dmsMBStatInfo *mbStat = NULL ;

      // lock meta
      _pDataSu->_metadataLatch.get_shared() ;

      dmsStorageData::COLNAME_MAP_IT it = _pDataSu->_collectionNameMap.begin() ;
      while ( it != _pDataSu->_collectionNameMap.end() )
      {
         monCollection collection ;
         if ( !sys && dmsIsSysCLName( it->first ) )
         {
            ++it ;
            continue ;
         }

         mb = &_pDataSu->_dmsMME->_mbList[it->second] ;
         mbStat = &_pDataSu->_mbStatInfo[it->second] ;

         ossMemset ( collection._name, 0, sizeof(collection._name) ) ;
         ossStrncpy ( collection._name, CSName(), DMS_SU_NAME_SZ ) ;
         ossStrncat ( collection._name, ".", 1 ) ;
         ossStrncat ( collection._name, mb->_collectionName,
                      DMS_COLLECTION_NAME_SZ ) ;
         collection.addDetails ( CSSequence(),
                                 mb->_numIndexes,
                                 mb->_blockID,
                                 mb->_flag,
                                 mb->_logicalID,
                                 mbStat->_totalRecords,
                                 mbStat->_totalDataPages,
                                 mbStat->_totalIndexPages,
                                 mbStat->_totalDataFreeSpace,
                                 mbStat->_totalIndexFreeSpace ) ;
         //add
         collectionList.insert ( collection ) ;

         ++it ;
      }

      // release meta
      _pDataSu->_metadataLatch.release_shared() ;
   }

   void _dmsStorageUnit::dumpInfo ( set<_monStorageUnit> &storageUnitList,
                                    BOOLEAN sys )
   {
      if ( !sys && dmsIsSysCSName( CSName() ) )
      {
         return ;
      }

      monStorageUnit su ;
      const dmsStorageUnitHeader *dataHeader = _pDataSu->getHeader() ;

      ossMemset ( su._name, 0, sizeof ( su._name ) ) ;
      ossStrncpy ( su._name, CSName(), DMS_SU_NAME_SZ ) ;
      su._pageSize = getPageSize() ;
      su._sequence = CSSequence() ;
      su._numCollections = dataHeader->_numMB ;
      su._collectionHWM = dataHeader->_MBHWM ;
      su._size = totalSize() ;
      su._CSID = CSID() ;
      su._logicalCSID = LogicalCSID() ;

      //add
      storageUnitList.insert ( su ) ;
   }

   INT64 _dmsStorageUnit::totalSize() const
   {
      INT64 totalSize = 0 ;
      if ( _pDataSu && _pIndexSu )
      {
         const dmsStorageUnitHeader *dataHeader = _pDataSu->getHeader() ;
         const dmsStorageUnitHeader *idxHeader = _pIndexSu->getHeader() ;
         totalSize = dataHeader->_storageUnitSize +
                     idxHeader->_storageUnitSize ;
         totalSize = totalSize << _pDataSu->pageSizeSquareRoot() ;
      }
      return totalSize ;
   }

   INT64 _dmsStorageUnit::totalDataPages() const
   {
      INT64 totalDataPages = 0 ;
      if ( _pDataSu && _pIndexSu )
      {
         const dmsStorageUnitHeader *dataHeader = _pDataSu->getHeader() ;
         const dmsStorageUnitHeader *idxHeader = _pIndexSu->getHeader() ;
         totalDataPages = dataHeader->_pageNum + idxHeader->_pageNum ;
      }
      return totalDataPages ;
   }

   INT64 _dmsStorageUnit::totalDataSize() const
   {
      INT64 totalSize = 0 ;
      if ( _pDataSu )
      {
         totalSize = totalDataPages() << _pDataSu->pageSizeSquareRoot() ;
      }
      return totalSize ;
   }

   INT32 _dmsStorageUnit::totalFreePages () const
   {
      INT32 freePages = 0 ;
      if ( _pDataSu && _pIndexSu )
      {
         freePages = (INT32)_pDataSu->freePageNum() +
                     (INT32)_pIndexSu->freePageNum() ;
      }
      return freePages ;
   }

   void _dmsStorageUnit::getStatInfo( dmsStorageUnitStat & statInfo )
   {
      ossMemset( &statInfo, 0, sizeof( dmsStorageUnitStat ) ) ;

      dmsMB *mb = NULL ;
      dmsMBStatInfo *mbStat = NULL ;

      // lock meta
      _pDataSu->_metadataLatch.get_shared() ;

      dmsStorageData::COLNAME_MAP_IT it = _pDataSu->_collectionNameMap.begin() ;
      while ( it != _pDataSu->_collectionNameMap.end() )
      {
         mb = &_pDataSu->_dmsMME->_mbList[it->second] ;
         mbStat = &_pDataSu->_mbStatInfo[it->second] ;

         ++statInfo._clNum ;
         statInfo._totalCount += mbStat->_totalRecords ;
         statInfo._totalDataPages += mbStat->_totalDataPages ;
         statInfo._totalIndexPages += mbStat->_totalIndexPages ;
         statInfo._totalDataFreeSpace += mbStat->_totalDataFreeSpace ;
         statInfo._totalIndexFreeSpace += mbStat->_totalIndexFreeSpace ;

         ++it ;
      }

      // release meta
      _pDataSu->_metadataLatch.release_shared() ;
   }

}  // namespace engine

