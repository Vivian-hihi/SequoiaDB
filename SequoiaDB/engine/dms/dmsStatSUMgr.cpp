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

   Source File Name = dmsStatSUMgr.cpp

   Descriptive Name = Data Management Service Statistics Table Control Block

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains code logic for
   statistics table creation and release.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "dmsStatSUMgr.hpp"
#include "dmsStorageUnit.hpp"
#include "rtn.hpp"
#include "pdTrace.hpp"
#include "dmsTrace.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
#define DMS_STAT_SPACE_NAME          "SYSSTAT"
#define DMS_STAT_COLLECTION_CL_NAME  DMS_STAT_SPACE_NAME".SYSCOLLECTIONSTAT"
#define DMS_STAT_INDEX_CL_NAME       DMS_STAT_SPACE_NAME".SYSINDEXSTAT"

#define DMS_STAT_CL_IDX_NAME         "STATCLIDX"

#define DMS_STAT_CL_IDX_DEF \
   "{ "IXM_FIELD_NAME_NAME"      : \""DMS_STAT_CL_IDX_NAME"\", \
      "IXM_FIELD_NAME_KEY"       : { "DMS_STAT_COLLECTION_SPACE" : 1, \
                                     "DMS_STAT_COLLECTION_MBID" : 1 } }"

#define DMS_STAT_IDX_IDX_NAME       "STATIDXIDX"

#define DMS_STAT_IDX_IDX_DEF \
   "{ "IXM_FIELD_NAME_NAME"      : \""DMS_STAT_IDX_IDX_NAME"\", \
      "IXM_FIELD_NAME_KEY"       : { "DMS_STAT_COLLECTION_SPACE" : 1, \
                                     "DMS_STAT_COLLECTION_MBID" : 1, \
                                     "DMS_STAT_IDX_INDEX" : 1 } }"

   /*
      _dmsStatSUMgr implement
    */
   _dmsStatSUMgr::_dmsStatSUMgr ( SDB_DMSCB *dmsCB )
   : _dmsSysSUMgr( dmsCB )
   {
      _initialized = FALSE ;
      _collectionHint = BSON( "" << DMS_STAT_CL_IDX_NAME ) ;
      _indexHint = BSON( "" << DMS_STAT_IDX_IDX_NAME ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTATSUMGR_INIT, "_dmsStatSUMgr::init" )
   INT32 _dmsStatSUMgr::init ()
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( SDB__DMSSTATSUMGR_INIT ) ;

      _pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      dmsStorageUnitID suID = DMS_INVALID_CS ;

      SDB_ASSERT ( _dmsCB, "dmsCB can't be NULL" ) ;

      // exclusive lock SYSSTAT cb. this function should be called during
      // process initialization, so it shouldn't be called in parallel by
      // agents
      DMSSYSSUMGR_XLOCK

      // first to load collection space
      rc = rtnCollectionSpaceLock( DMS_STAT_SPACE_NAME, _dmsCB, TRUE,
                                   &_su, suID ) ;
      if ( SDB_DMS_CS_NOTEXIST == rc )
      {
         // create new SYSSTAT collection space
         rc = rtnCreateCollectionSpaceCommand ( DMS_STAT_SPACE_NAME, NULL, _dmsCB,
                                                NULL, DMS_PAGE_SIZE_MAX,
                                                DMS_DO_NOT_CREATE_LOB, TRUE ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to create SYSSTAT collection space, rc: %d",
                      rc ) ;

         rc = rtnCollectionSpaceLock ( DMS_STAT_SPACE_NAME, _dmsCB, TRUE,
                                       &_su, suID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to lock SYSSTAT collection space, rc: %d", rc ) ;
      }
      else if ( SDB_OK != rc )
      {
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to lock collection space [%s], rc: %d",
                      DMS_STAT_SPACE_NAME, rc ) ;
      }

      _dmsCB->suUnlock( suID ) ;
      suID = DMS_INVALID_CS ;

      rc = _ensureStatMetadata( cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create statistics collections or "
                   "indexes, rc: %d", rc ) ;

      _loadStats( cb ) ;

      _initialized = TRUE ;

   done :
      if ( DMS_INVALID_CS != suID )
      {
         _dmsCB->suUnlock ( suID ) ;
      }
      PD_TRACE_EXITRC ( SDB__DMSSTATSUMGR_INIT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_RELOADSTATS, "_dmsStatSUMgr::reloadStats" )
   INT32 _dmsStatSUMgr::reloadStats ( pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_RELOADSTATS ) ;

      // Make sure only one reload in one time
      DMSSYSSUMGR_XLOCK

      PD_CHECK( _initialized, SDB_SYS, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      rc = _loadStats( cb ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to load statistics, rc: %d", rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_RELOADSTATS, rc ) ;
      return SDB_OK ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_ONLOADCS, "_dmsStatSUMgr::onLoadCS" )
   INT32 _dmsStatSUMgr::onLoadCS ( _IDmsEventHolder *pEventHolder,
                                   _IUtilSUCacheHolder *pCacheHolder,
                                   pmdEDUCB *cb,
                                   SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_ONLOADCS ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      PD_CHECK( _initialized, SDB_INVALIDARG, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      if ( pCacheHolder )
      {
         utilSUCache *pCache = pCacheHolder->getSUCache( DMS_CACHE_TYPE_STAT ) ;

         if ( pCache && pCache->isValid() )
         {
              rc = _loadCollectionStatsByCS( pCacheHolder->getCSName(),
                                             *pCache, cb ) ;
              if ( SDB_OK != rc )
              {
                 PD_LOG( PDWARNING, "Failed to load collection statistics for "
                         "collection space [%s], rc: %d",
                         pCacheHolder->getCSName(), rc ) ;
                 pCache->clearCacheUnits() ;
                 goto error ;
              }

              rc = _loadIndexStatsByCS( pCacheHolder->getCSName(), *pCache, cb ) ;
              if ( SDB_OK != rc )
              {
                 PD_LOG( PDWARNING, "Failed to load index statistics for "
                         "collection space [%s], rc: %d",
                         pCacheHolder->getCSName(), rc ) ;
                 pCache->clearCacheUnits() ;
                 goto error ;
             }
         }
         else
         {
            PD_LOG( PDWARNING, "Failed to load statistics for collection space [%s]",
                    pCacheHolder->getCSName() ) ;
         }
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_ONLOADCS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_ONUNLOADCS, "_dmsStatSUMgr::onUnloadCS" )
   INT32 _dmsStatSUMgr::onUnloadCS ( _IDmsEventHolder *pEventHolder,
                                     _IUtilSUCacheHolder *pCacheHolder,
                                     pmdEDUCB *cb,
                                     SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_ONUNLOADCS ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      PD_CHECK( _initialized, SDB_INVALIDARG, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      if ( pCacheHolder )
      {
         utilSUCache *pCache = pCacheHolder->getSUCache( DMS_CACHE_TYPE_STAT ) ;
         if ( pCache )
         {
            pCache->clearCacheUnits() ;
         }
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_ONUNLOADCS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_ONRENAMECS, "_dmsStatSUMgr::onRenameCS" )
   INT32 _dmsStatSUMgr::onRenameCS ( _IDmsEventHolder *pEventHolder,
                                     _IUtilSUCacheHolder *pCacheHolder,
                                     const CHAR *pOldCSName,
                                     const CHAR *pNewCSName,
                                     pmdEDUCB *cb,
                                     SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN renamed = FALSE ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_ONRENAMECS ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      PD_CHECK( _initialized, SDB_INVALIDARG, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      if ( pCacheHolder )
      {
         utilSUCache *pCache = pCacheHolder->getSUCache( DMS_CACHE_TYPE_STAT ) ;
         if ( pCache )
         {
            for ( UINT16 unitID = 0 ; unitID < pCache->getSize() ; unitID ++ )
            {
               dmsCollectionStat *pCollectionStat =
                     (dmsCollectionStat *)pCache->getCacheUnit( unitID ) ;
               if ( pCollectionStat )
               {
                  pCollectionStat->renameCS( pNewCSName ) ;
                  renamed = TRUE ;
               }
            }
         }
      }

      if ( renamed && pEventHolder && dpsCB )
      {
         const CHAR *pOldCSName = pEventHolder->getCSName() ;

         BSONObj boMatcher( BSON( DMS_STAT_COLLECTION_SPACE << pOldCSName ) ) ;
         BSONObj boNewName( BSON( DMS_STAT_COLLECTION_SPACE << pNewCSName ) ) ;
         BSONObj boUpdator( BSON( "$set" << boNewName ) ) ;

         rc = _updateCollectionStat( boMatcher, boUpdator, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to update collection statistics when rename "
                      "collection space [%s] to [%s], rc: %d",
                      pOldCSName, pNewCSName, rc ) ;

         rc = _updateIndexStat( boMatcher, boUpdator, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to update index statistics when rename "
                      "collection space [%s] to [%s], rc: %d",
                      pOldCSName, pNewCSName, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_ONRENAMECS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_ONDROPCS, "_dmsStatSUMgr::onDropCS" )
   INT32 _dmsStatSUMgr::onDropCS ( _IDmsEventHolder *pEventHolder,
                                   _IUtilSUCacheHolder *pCacheHolder,
                                   pmdEDUCB *cb,
                                   SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_ONDROPCS ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      PD_CHECK( _initialized, SDB_INVALIDARG, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      if ( pCacheHolder )
      {
         utilSUCache *pCache = pCacheHolder->getSUCache( DMS_CACHE_TYPE_STAT ) ;
         if ( pCache )
         {
            deleted = pCache->clearCacheUnits() ;
         }
      }

      if ( deleted && pEventHolder && dpsCB )
      {
         const CHAR *pCSName = pEventHolder->getCSName() ;

         BSONObj boMatcher( BSON( DMS_STAT_COLLECTION_SPACE << pCSName ) ) ;

         rc = _deleteCollectionStat( boMatcher, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to drop collection statistics when dropping "
                      "collection space [%s], rc: %d", pCSName, rc ) ;

         rc = _deleteIndexStat( boMatcher, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to delete index statistics when dropping "
                      "collection space [%s], rc: %d", pCSName, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_ONDROPCS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_ONRENAMECL, "_dmsStatSUMgr::onRenameCL" )
   INT32 _dmsStatSUMgr::onRenameCL ( _IDmsEventHolder *pEventHolder,
                                     _IUtilSUCacheHolder *pCacheHolder,
                                     const dmsCLItem &clItem,
                                     const CHAR *pNewCLName,
                                     pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN renamed = FALSE ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_ONRENAMECL ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      PD_CHECK( _initialized, SDB_INVALIDARG, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      if ( pCacheHolder )
      {
         utilSUCache *pCache = pCacheHolder->getSUCache( DMS_CACHE_TYPE_STAT ) ;
         if ( pCache )
         {
            // For statistics cache, mbID is ID of cache unit
            dmsCollectionStat *pCollectionStat =
                  (dmsCollectionStat *)pCache->getCacheUnit( clItem._mbID ) ;
            if ( pCollectionStat )
            {
               pCollectionStat->renameCL( pNewCLName ) ;
               renamed = TRUE ;
            }
         }
      }

      if ( renamed && pEventHolder && dpsCB )
      {
         const CHAR *pCSName = pEventHolder->getCSName() ;
         const CHAR *pOldCLName = clItem._pCLName ;
         UINT16 mbID = clItem._mbID ;

         BSONObj boMatcher( BSON( DMS_STAT_COLLECTION_SPACE << pCSName <<
                                  DMS_STAT_COLLECTION_MBID << (INT32)mbID ) ) ;
         BSONObj boNewName( BSON( DMS_STAT_COLLECTION << pNewCLName ) ) ;
         BSONObj boUpdator( BSON( "$set" << boNewName ) ) ;

         rc = _updateCollectionStat( boMatcher, boUpdator, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to update collection statistics when rename "
                      "collection [%s.%s] to [%s.%s] mbID [%d], rc: %d",
                      pCSName, pOldCLName, pCSName, pNewCLName, mbID, rc ) ;

         rc = _updateIndexStat( boMatcher, boUpdator, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to update index statistics when rename "
                      "collection [%s.%s] to [%s.%s] mbID [%d], rc: %d",
                      pCSName, pOldCLName, pCSName, pNewCLName, mbID, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_ONRENAMECL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_ONTRUNCCL, "_dmsStatSUMgr::onTruncateCL" )
   INT32 _dmsStatSUMgr::onTruncateCL ( _IDmsEventHolder *pEventHolder,
                                       _IUtilSUCacheHolder *pCacheHolder,
                                       const dmsCLItem &clItem,
                                       pmdEDUCB *cb,
                                       SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_ONTRUNCCL ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      PD_CHECK( _initialized, SDB_INVALIDARG, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      if ( pCacheHolder )
      {
         utilSUCache *pCache = pCacheHolder->getSUCache( DMS_CACHE_TYPE_STAT ) ;
         if ( pCache )
         {
            // For statistics cache, mbID is key of cache unit
            deleted = pCache->removeCacheUnit( clItem._mbID, TRUE ) ;
         }
      }

      if ( deleted && pEventHolder && dpsCB )
      {
         const CHAR *pCSName = pEventHolder->getCSName() ;
         UINT16 mbID = clItem._mbID ;

         BSONObj boMatcher( BSON( DMS_STAT_COLLECTION_SPACE << pCSName <<
                                  DMS_STAT_COLLECTION_MBID << (INT32)mbID ) ) ;

         rc = _deleteCollectionStat( boMatcher, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to delete collection statistics when truncating "
                      "collection [ space %s mbID %d ], rc: %d",
                      pCSName, mbID, rc ) ;

         rc = _deleteIndexStat( boMatcher, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to delete index statistics when truncating "
                      "collection [ space %s mbID %d ], rc: %d",
                      pCSName, mbID, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_ONTRUNCCL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_ONDROPCL, "_dmsStatSUMgr::onDropCL" )
   INT32 _dmsStatSUMgr::onDropCL ( _IDmsEventHolder *pEventHolder,
                                   _IUtilSUCacheHolder *pCacheHolder,
                                   const dmsCLItem &clItem,
                                   pmdEDUCB *cb,
                                   SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_ONDROPCL ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      PD_CHECK( _initialized, SDB_INVALIDARG, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      if ( pCacheHolder )
      {
         utilSUCache *pCache = pCacheHolder->getSUCache( DMS_CACHE_TYPE_STAT ) ;
         if ( pCache )
         {
            // For statistics cache, mbID is key of cache unit
            deleted = pCache->removeCacheUnit( clItem._mbID, TRUE ) ;
         }
      }

      if ( deleted && pEventHolder && dpsCB )
      {
         const CHAR *pCSName = pEventHolder->getCSName() ;
         UINT16 mbID = clItem._mbID ;

         BSONObj boMatcher( BSON( DMS_STAT_COLLECTION_SPACE << pCSName <<
                                  DMS_STAT_COLLECTION_MBID << (INT32)mbID ) ) ;

         rc = _deleteCollectionStat( boMatcher, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to delete collection statistics when dropping "
                      "collection [ space %s mbID %d ], rc: %d",
                      pCSName, mbID, rc ) ;

         rc = _deleteIndexStat( boMatcher, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to delete index statistics when dropping "
                      "collection [ space %s mbID %d ], rc: %d",
                      pCSName, mbID, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_ONDROPCL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_ONDROPIDX, "_dmsStatSUMgr::onDropIndex" )
   INT32 _dmsStatSUMgr::onDropIndex ( _IDmsEventHolder *pEventHolder,
                                      _IUtilSUCacheHolder *pCacheHolder,
                                      const dmsCLItem &clItem,
                                      const dmsIdxItem &idxItem,
                                      pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_ONDROPIDX ) ;

      SDB_ASSERT( pEventHolder, "Event holder is invalid" ) ;

      PD_CHECK( _initialized, SDB_INVALIDARG, error, PDWARNING,
                "Statistics SU is not initialized" ) ;

      if ( pCacheHolder )
      {
         utilSUCache *pCache = pCacheHolder->getSUCache( DMS_CACHE_TYPE_STAT ) ;
         if ( pCache )
         {
            // For statistics cache, mbID is ID of cache unit
            dmsCollectionStat *pCollectionStat =
                  (dmsCollectionStat *)pCache->getCacheUnit( clItem._mbID ) ;
            if ( pCollectionStat )
            {
               deleted = pCollectionStat->removeIndexStat( idxItem._pIdxName, TRUE ) ;
            }
         }
      }

      if ( deleted && pEventHolder && dpsCB )
      {
         const CHAR *pCSName = pEventHolder->getCSName() ;
         UINT16 mbID = clItem._mbID ;
         const CHAR *pIdxName = idxItem._pIdxName ;

         BSONObj boMatcher( BSON( DMS_STAT_COLLECTION_SPACE << pCSName <<
                                  DMS_STAT_COLLECTION_MBID << (INT32)mbID <<
                                  DMS_STAT_IDX_INDEX << pIdxName ) ) ;

         rc = _deleteIndexStat( boMatcher, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING,
                      "Failed to delete index statistics when dropping "
                      "index [ space %s mbID %d index %s ] , rc: %d",
                      pCSName, mbID, pIdxName, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_ONDROPIDX, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 _dmsStatSUMgr::_ensureStatMetadata ( pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      rc = rtnTestAndCreateCL( DMS_STAT_COLLECTION_CL_NAME, cb, _dmsCB, NULL,
                               TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create collection [%s], rc: %d",
                   DMS_STAT_COLLECTION_CL_NAME, rc ) ;

      rc = rtnTestAndCreateCL( DMS_STAT_INDEX_CL_NAME, cb, _dmsCB, NULL, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create collection [%s], rc: %d",
                   DMS_STAT_INDEX_CL_NAME, rc ) ;

      {
         BSONObj idxDef ;

         rc = fromjson( DMS_STAT_CL_IDX_DEF, idxDef ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to build index object [%s], rc: %d",
                      DMS_STAT_CL_IDX_DEF, rc ) ;

         // Initialized before rtn, so no sorterCreator could be used, set
         // sort buffer size to 0 to build index without sorterCreator
         rc = rtnTestAndCreateIndex( DMS_STAT_COLLECTION_CL_NAME, idxDef, cb,
                                     _dmsCB, NULL, TRUE, 0 ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create index [%s], rc: %d",
                      DMS_STAT_CL_IDX_DEF, rc ) ;
      }

      {
         BSONObj idxDef ;

         rc = fromjson( DMS_STAT_IDX_IDX_DEF, idxDef ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to build index object [%s], rc: %d",
                      DMS_STAT_IDX_IDX_DEF, rc ) ;

         // Initialized before rtn, so no sorterCreator could be used, set
         // sort buffer size to 0 to build index without sorterCreator
         rc = rtnTestAndCreateIndex( DMS_STAT_INDEX_CL_NAME, idxDef, cb,
                                     _dmsCB, NULL, TRUE, 0 ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create index [%s], rc: %d",
                      DMS_STAT_IDX_IDX_DEF, rc ) ;
      }

   done :
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR_LOADSTATS, "_dmsStatSUMgr::_loadStats" )
   INT32 _dmsStatSUMgr::_loadStats ( pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR_LOADSTATS ) ;

      CS_STAT_MAP csStatMap ;
      set<monCollectionSpace> csList ;

      _dmsCB->dumpInfo( csList, FALSE ) ;

      for ( set<monCollectionSpace>::iterator iterCS = csList.begin() ;
            iterCS != csList.end() ;
            ++ iterCS )
      {
         const _monCollectionSpace &cs = *iterCS ;
         csStatMap[ cs._name ] = NULL ;
      }

      rc = _loadCollectionStats( csStatMap, cb ) ;
      PD_RC_CHECK ( rc, PDWARNING, "Failed to load collection statistics, rc: %d", rc ) ;

      rc = _loadIndexStats( csStatMap, cb ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to load index statistics, rc: %d", rc ) ;

      for ( CS_STAT_MAP::iterator iterStat = csStatMap.begin() ;
            iterStat != csStatMap.end() ;
            ++ iterStat )
      {
         const CHAR *pCSName = iterStat->first._pName ;
         utilSUCache *pStatMap = iterStat->second ;
         _replaceCollectionStats( pCSName, pStatMap ) ;
      }

   done :
      for ( CS_STAT_MAP::iterator iterStat = csStatMap.begin() ;
            iterStat != csStatMap.end() ;
            ++ iterStat )
      {
         utilSUCache *pStatMap = iterStat->second ;
         SAFE_OSS_DELETE( pStatMap ) ;
      }

      csStatMap.clear() ;

      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR_LOADSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__LOADCLSTATS, "_dmsStatSUMgr::_loadCollectionStats" )
   INT32 _dmsStatSUMgr::_loadCollectionStats ( CS_STAT_MAP &csStatMap,
                                               pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__LOADCLSTATS ) ;

      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      INT64 contextID = -1 ;
      BSONObj boDummy ;

      // query
      rc = rtnQuery( DMS_STAT_COLLECTION_CL_NAME, boDummy, boDummy,
                     boDummy, boDummy, 0, cb, 0, -1, _dmsCB, rtnCB,
                     contextID ) ;
      PD_RC_CHECK( rc, PDWARNING, "Query collection [%s] failed, rc: %d",
                   DMS_STAT_COLLECTION_CL_NAME, rc ) ;

      // get more
      while ( TRUE )
      {
         dmsCollectionStat *pCollectionStat = NULL ;
         rtnContextBuf contextBuf ;

         rc = rtnGetMore( contextID, 1, contextBuf, cb, rtnCB ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         PD_RC_CHECK( rc, PDWARNING, "Get more failed, rc: %d", rc ) ;

         pCollectionStat = SDB_OSS_NEW dmsCollectionStat() ;
         PD_CHECK( pCollectionStat, SDB_OOM, error, PDWARNING,
                   "Failed to allocate memory for index statistics" ) ;

         try
         {
            BSONObj boCollectionStat = BSONObj( contextBuf.data() ) ;

            rc = pCollectionStat->init( boCollectionStat ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDWARNING,
                       "Failed to initialize collection statistics with %s",
                       boCollectionStat.toString( FALSE, TRUE ).c_str() ) ;
               SAFE_OSS_DELETE( pCollectionStat ) ;
            }
         }
         catch( std::exception &e )
         {
            PD_LOG( PDWARNING,
                    "Get index statistics for collection occur exception: %s",
                    e.what() ) ;
            rc = SDB_SYS ;
            SAFE_OSS_DELETE( pCollectionStat ) ;
         }

         if ( SDB_OK != rc )
         {
            continue ;
         }

         rc = _addCollectionStat( csStatMap, pCollectionStat, FALSE ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDWARNING,
                    "Failed to add collection statistics [%s.%s], rc: %d",
                    pCollectionStat->getCSName(), pCollectionStat->getCLName(),
                    rc ) ;
            SAFE_OSS_DELETE( pCollectionStat ) ;
            goto error ;
         }
      }

   done :
      if ( -1 != contextID )
      {
         rtnKillContexts( 1 , &contextID, cb, rtnCB ) ;
      }
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__LOADCLSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__LOADIDXSTATS, "_dmsStatSUMgr::_loadIndexStats" )
   INT32 _dmsStatSUMgr::_loadIndexStats ( CS_STAT_MAP &csStatMap, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__LOADIDXSTATS ) ;

      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      INT64 contextID = -1 ;
      BSONObj boDummy ;

      // query
      rc = rtnQuery( DMS_STAT_INDEX_CL_NAME, boDummy, boDummy,
                     boDummy, boDummy, 0, cb, 0, -1, _dmsCB, rtnCB,
                     contextID ) ;
      PD_RC_CHECK( rc, PDWARNING, "Query collection [%s] failed, rc: %d",
                   DMS_STAT_INDEX_CL_NAME, rc ) ;

      // get more
      while ( TRUE )
      {
         dmsIndexStat *pIndexStat = NULL ;
         rtnContextBuf contextBuf ;

         rc = rtnGetMore( contextID, 1, contextBuf, cb, rtnCB ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         PD_RC_CHECK( rc, PDWARNING, "Get more failed, rc: %d", rc ) ;

         pIndexStat = SDB_OSS_NEW dmsIndexStat() ;
         PD_CHECK( pIndexStat, SDB_OOM, error, PDWARNING,
                   "Failed to allocate memory for index statistics" ) ;

         try
         {
            BSONObj boIndexStat = BSONObj( contextBuf.data() ) ;

            rc = pIndexStat->init( boIndexStat ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDWARNING,
                       "Failed to initialize index statistics with %s",
                       boIndexStat.toString( FALSE, TRUE ).c_str() ) ;
               SAFE_OSS_DELETE( pIndexStat ) ;
               goto error ;
            }
         }
         catch( std::exception &e )
         {
            PD_LOG( PDWARNING,
                    "Get index statistics for index occur exception: %s",
                    e.what() ) ;
            rc = SDB_SYS ;
            SAFE_OSS_DELETE( pIndexStat ) ;
            goto error ;
         }

         rc = _addIndexStat( csStatMap, pIndexStat, FALSE ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDWARNING,
                    "Failed to add index statistics [%s.%s, %s], rc: %d",
                    pIndexStat->getCSName(), pIndexStat->getCLName(),
                    pIndexStat->getIndexName(), rc ) ;
            SAFE_OSS_DELETE( pIndexStat ) ;
            goto error ;
         }
      }

   done :
      if ( -1 != contextID )
      {
         rtnKillContexts( 1 , &contextID, cb, rtnCB ) ;
      }
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__LOADIDXSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__LOADCLSTATS_CS, "_dmsStatSUMgr::_loadCollectionStats" )
   INT32 _dmsStatSUMgr::_loadCollectionStatsByCS ( const CHAR *pCSName,
                                                   utilSUCache &statMap,
                                                   pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__LOADCLSTATS_CS ) ;

      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      BSONObj boMatcher( BSON( DMS_STAT_COLLECTION_SPACE << pCSName ) ) ;
      BSONObj boDummy ;

      INT64 contextID = -1 ;

      // query
      rc = rtnQuery( DMS_STAT_COLLECTION_CL_NAME, boDummy, boMatcher,
                     boDummy, _collectionHint, 0, cb, 0, -1, _dmsCB, rtnCB,
                     contextID ) ;
      PD_RC_CHECK( rc, PDWARNING, "Query statistics of collection space [%s] "
                   "from [%s] failed, rc: %d", pCSName,
                   DMS_STAT_COLLECTION_CL_NAME, rc ) ;

      // get more
      while ( TRUE )
      {
         dmsCollectionStat *pCollectionStat = NULL ;
         rtnContextBuf contextBuf ;

         rc = rtnGetMore( contextID, 1, contextBuf, cb, rtnCB ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         PD_RC_CHECK( rc, PDWARNING, "Get more failed, rc: %d", rc ) ;

         pCollectionStat = SDB_OSS_NEW dmsCollectionStat() ;
         PD_CHECK( pCollectionStat, SDB_OOM, error, PDWARNING,
                   "Failed to allocate memory for index statistics" ) ;

         try
         {
            BSONObj boCollectionStat = BSONObj( contextBuf.data() ) ;

            rc = pCollectionStat->init( boCollectionStat ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDWARNING,
                       "Failed to initialize collection statistics with %s",
                       boCollectionStat.toString( FALSE, TRUE ).c_str() ) ;
               SAFE_OSS_DELETE( pCollectionStat ) ;
            }
         }
         catch( std::exception &e )
         {
            PD_LOG( PDWARNING,
                    "Get index statistics for collection occur exception: %s",
                    e.what() ) ;
            rc = SDB_SYS ;
            SAFE_OSS_DELETE( pCollectionStat ) ;
         }

         if ( SDB_OK != rc )
         {
            continue ;
         }

         if ( !statMap.addCacheUnit( pCollectionStat, FALSE ) )
         {
            PD_LOG( PDWARNING,
                    "Failed to add collection statistics [%s.%s]",
                    pCollectionStat->getCSName(), pCollectionStat->getCLName() ) ;
            SAFE_OSS_DELETE( pCollectionStat ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

   done :
      if ( -1 != contextID )
      {
         rtnKillContexts( 1 , &contextID, cb, rtnCB ) ;
      }
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__LOADCLSTATS_CS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__LOADIDXSTATS_CS, "_dmsStatSUMgr::_loadIndexStats" )
   INT32 _dmsStatSUMgr::_loadIndexStatsByCS ( const CHAR *pCSName,
                                              utilSUCache &statMap,
                                              pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__LOADIDXSTATS_CS ) ;

      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      INT64 contextID = -1 ;
      BSONObj boDummy ;

      // query
      rc = rtnQuery( DMS_STAT_INDEX_CL_NAME, boDummy, boDummy,
                     boDummy, boDummy, 0, cb, 0, -1, _dmsCB, rtnCB,
                     contextID ) ;
      PD_RC_CHECK( rc, PDWARNING, "Query collection [%s] failed, rc: %d",
                   DMS_STAT_INDEX_CL_NAME, rc ) ;

      // get more
      while ( TRUE )
      {
         dmsIndexStat *pIndexStat = NULL ;
         rtnContextBuf contextBuf ;

         rc = rtnGetMore( contextID, 1, contextBuf, cb, rtnCB ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         PD_RC_CHECK( rc, PDWARNING, "Get more failed, rc: %d", rc ) ;

         pIndexStat = SDB_OSS_NEW dmsIndexStat() ;
         PD_CHECK( pIndexStat, SDB_OOM, error, PDWARNING,
                   "Failed to allocate memory for index statistics" ) ;

         try
         {
            BSONObj boIndexStat = BSONObj( contextBuf.data() ) ;

            rc = pIndexStat->init( boIndexStat ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDWARNING,
                       "Failed to initialize index statistics with %s",
                       boIndexStat.toString( FALSE, TRUE ).c_str() ) ;
               SAFE_OSS_DELETE( pIndexStat ) ;
            }
         }
         catch( std::exception &e )
         {
            PD_LOG( PDWARNING,
                    "Get index statistics for index occur exception: %s",
                    e.what() ) ;
            rc = SDB_SYS ;
            SAFE_OSS_DELETE( pIndexStat ) ;
         }

         if ( SDB_OK != rc )
         {
            continue ;
         }

         if ( !statMap.addCacheSubUnit( pIndexStat, FALSE ) )
         {
            PD_LOG( PDWARNING, "Failed to add index statistics [%s.%s, %s]",
                    pIndexStat->getCSName(), pIndexStat->getCLName(),
                    pIndexStat->getIndexName() ) ;
            SAFE_OSS_DELETE( pIndexStat ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

   done :
      if ( -1 != contextID )
      {
         rtnKillContexts( 1 , &contextID, cb, rtnCB ) ;
      }
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__LOADIDXSTATS_CS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__CLEARSTATS, "_dmsStatSUMgr::_clearStats" )
   INT32 _dmsStatSUMgr::_clearStats ()
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__CLEARSTATS ) ;

      std::set<monCollectionSpace> csList ;
      _dmsCB->dumpInfo( csList, FALSE ) ;

      for ( std::set<_monCollectionSpace>::const_iterator iterCS = csList.begin() ;
            iterCS != csList.end () ;
            iterCS ++ )
      {
         const _monCollectionSpace &cs = *iterCS ;
         dmsStorageUnitID suID = DMS_INVALID_SUID ;
         dmsStorageUnit *su = NULL ;
         utilSUCache *pStatCache = NULL ;

         INT32 rc = _dmsCB->nameToSUAndLock( cs._name, suID, &su, SHARED ) ;

         if ( SDB_OK != rc )
         {
            PD_LOG( PDWARNING, "Failed to get storage unit [%s], rc: %d",
                    cs._name, rc ) ;
            if ( DMS_INVALID_SUID != suID )
            {
               _dmsCB->suUnlock( suID, SHARED ) ;
            }
            continue ;
         }

         pStatCache = su->getSUCache( DMS_CACHE_TYPE_STAT ) ;
         if ( pStatCache )
         {
            pStatCache->clearCacheUnits() ;
         }

         if ( DMS_INVALID_SUID != suID )
         {
            _dmsCB->suUnlock( suID, SHARED ) ;
         }
      }

      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__CLEARSTATS, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__ADDCLSTAT, "_dmsStatSUMgr::_addCollectionStat" )
   INT32 _dmsStatSUMgr::_addCollectionStat ( CS_STAT_MAP &csStatMap,
                                             dmsCollectionStat *pCollectionStat,
                                             BOOLEAN ignoreVersion )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__ADDCLSTAT ) ;

      const CHAR *pCSName = pCollectionStat->getCSName() ;
      utilSUCache *pStatMap = NULL ;

      CS_STAT_MAP::iterator iterCS = csStatMap.find( pCSName ) ;
      PD_CHECK( iterCS != csStatMap.end(), SDB_INVALIDARG, error, PDWARNING,
                "Collection space [%s] is not valid for statistics", pCSName ) ;

      pStatMap = iterCS->second ;
      if ( !iterCS->second )
      {
         iterCS->second = SDB_OSS_NEW dmsStatCache( NULL ) ;
         pStatMap = iterCS->second ;
      }
      PD_CHECK( pStatMap, SDB_INVALIDARG, error, PDWARNING,
                "Could not allocate statistics map [%s]", pCSName ) ;

      PD_CHECK( pStatMap->addCacheUnit( pCollectionStat, ignoreVersion ),
                SDB_INVALIDARG, error, PDWARNING,
                "Could not add collection statistics [%s] to statistics map [%s]",
                pCollectionStat->getCLName(), pCSName ) ;

  done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__ADDCLSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__ADDIDXSTAT, "_dmsStatSUMgr::_addIndexStat" )
   INT32 _dmsStatSUMgr::_addIndexStat ( CS_STAT_MAP &csStatMap,
                                        dmsIndexStat *pIndexStat,
                                        BOOLEAN ignoreVersion )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__ADDIDXSTAT ) ;

      const CHAR *pCSName = pIndexStat->getCSName() ;
      utilSUCache *pStatMap = NULL ;

      CS_STAT_MAP::iterator iterCS = csStatMap.find( pCSName ) ;
      PD_CHECK( iterCS != csStatMap.end(), SDB_INVALIDARG, error, PDWARNING,
                "Collection space [%s] is not valid for statistics", pCSName ) ;

      pStatMap = iterCS->second ;
      PD_CHECK( pStatMap, SDB_INVALIDARG, error, PDWARNING,
                "Could not find statistics map [%s]", pCSName ) ;

      PD_CHECK( pStatMap->addCacheSubUnit( pIndexStat, ignoreVersion ),
                SDB_INVALIDARG, error, PDWARNING,
                "Could not add index statistics [%s] to collection statistics [%s]",
                pIndexStat->getIndexName(), pIndexStat->getCLName() ) ;

  done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__ADDIDXSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__ADDCLSTATS, "_dmsStatSUMgr::_replaceCollectionStats" )
   INT32 _dmsStatSUMgr::_replaceCollectionStats ( const CHAR *pCSName,
                                                  utilSUCache *pStatMap )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__ADDCLSTATS ) ;

      dmsStorageUnitID suID = DMS_INVALID_CS ;
      dmsStorageUnit *pSu = NULL ;

      utilSUCache *pStatCache = NULL ;

      rc = _dmsCB->nameToSUAndLock ( pCSName, suID, &pSu, EXCLUSIVE, OSS_ONE_SEC ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to get collection space [%s], rc: %d",
                   pCSName, rc ) ;

      pStatCache = pSu->getSUCache( DMS_CACHE_TYPE_STAT ) ;
      PD_CHECK( pStatCache, SDB_INVALIDARG, error, PDWARNING,
                "No statistics manger in storage unit [%s]", pCSName ) ;

      pStatCache->clearCacheUnits() ;

      if ( pStatMap )
      {
         for ( UINT16 unitID = 0 ; unitID < pStatMap->getSize() ; unitID ++ )
         {
            dmsCollectionStat *pCollectionStat =
                  (dmsCollectionStat *)pStatMap->getCacheUnit( unitID ) ;
            if ( pCollectionStat != NULL &&
                 pStatCache->addCacheUnit( pCollectionStat, TRUE ) )
            {
               pStatMap->removeCacheUnit( unitID, FALSE ) ;
            }
         }
      }

   done :
      if ( DMS_INVALID_CS != suID )
      {
         _dmsCB->suUnlock( suID, EXCLUSIVE ) ;
      }
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__ADDCLSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__RMCLSTATS, "_dmsStatSUMgr::_removeCollectionStats" )
   INT32 _dmsStatSUMgr::_removeCollectionStats ( const CHAR *pCSName )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__RMCLSTATS ) ;

      dmsStorageUnitID suID = DMS_INVALID_CS ;
      dmsStorageUnit *pSu = NULL ;

      utilSUCache *pStatCache = NULL ;

      rc = _dmsCB->nameToSUAndLock ( pCSName, suID, &pSu, EXCLUSIVE, OSS_ONE_SEC ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to get collection space [%s], rc: %d",
                   pCSName, rc ) ;

      pStatCache = pSu->getSUCache( DMS_CACHE_TYPE_STAT ) ;
      PD_CHECK( pStatCache, SDB_INVALIDARG, error, PDWARNING,
                "No statistics manger in storage unit [%s]", pCSName ) ;

      pStatCache->clearCacheUnits() ;

   done :
      if ( DMS_INVALID_CS != suID )
      {
         _dmsCB->suUnlock( suID, EXCLUSIVE ) ;
      }
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__RMCLSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__DELCLSTAT, "_dmsStatSUMgr::_deleteCollectionStat" )
   INT32 _dmsStatSUMgr::_deleteCollectionStat ( const BSONObj &boMatcher,
                                             _pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__DELCLSTAT ) ;

      rc = rtnDelete( DMS_STAT_COLLECTION_CL_NAME, boMatcher, _collectionHint,
                      0, cb, _dmsCB, dpsCB, 1 ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Delete collection statistics [%s] failed, rc: %d",
                   boMatcher.toString().c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__DELCLSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__DELIDXSTAT, "_dmsStatSUMgr::_deleteIndexStat" )
   INT32 _dmsStatSUMgr::_deleteIndexStat ( const BSONObj &boMatcher, _pmdEDUCB *cb,
                                        SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__DELIDXSTAT ) ;

      rc = rtnDelete( DMS_STAT_INDEX_CL_NAME, boMatcher, _indexHint, 0, cb,
                      _dmsCB, dpsCB, 1 ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Delete index statistics [%s] failed, rc: %d",
                   boMatcher.toString().c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__DELIDXSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__UPDATECLSTAT, "_dmsStatSUMgr::_updateCollectionStat" )
   INT32 _dmsStatSUMgr::_updateCollectionStat ( const BSONObj &boMatcher,
                                                const BSONObj &boUpdator,
                                                _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__UPDATECLSTAT ) ;

      rc = rtnUpdate( DMS_STAT_COLLECTION_CL_NAME, boMatcher, boUpdator,
                      _collectionHint, 0, cb, _dmsCB, dpsCB, 1 ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Update collection statistics [%s] with [%s] failed, rc: %d",
                   boMatcher.toString( FALSE, TRUE ).c_str(),
                   boUpdator.toString( FALSE, TRUE ).c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__UPDATECLSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATSUMGR__UPDATEIDXSTAT, "_dmsStatSUMgr::_updateIndexStat" )
   INT32 _dmsStatSUMgr::_updateIndexStat ( const BSONObj &boMatcher,
                                           const BSONObj &boUpdator,
                                           _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATSUMGR__UPDATEIDXSTAT ) ;

      rc = rtnUpdate( DMS_STAT_INDEX_CL_NAME, boMatcher, boUpdator,
                      _collectionHint, 0, cb, _dmsCB, dpsCB, 1 ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Update index statistics [%s] with [%s] failed, rc: %d",
                   boMatcher.toString( FALSE, TRUE ).c_str(),
                   boUpdator.toString( FALSE, TRUE ).c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATSUMGR__UPDATEIDXSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

}

