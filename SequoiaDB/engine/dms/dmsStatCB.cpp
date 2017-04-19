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

   Source File Name = dmsStatCB.cpp

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

#include "dmsStatCB.hpp"
#include "../bson/bson.h"
#include "dmsStorageUnit.hpp"
#include "rtn.hpp"
#include "pdTrace.hpp"
#include "dmsTrace.hpp"
#include "pmd.hpp"
#include "catCommon.hpp"
#include "monDMS.hpp"
#include "rtnStatMgr.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

using namespace bson ;
namespace fs = boost::filesystem ;

namespace engine
{

   /*
      _dmsStatCB implement
    */
   _dmsStatCB::_dmsStatCB ( _SDB_DMSCB *dmsCB ) : _dmsSysCB( dmsCB )
   {
      _collectionHint = BSON( "" << STAT_CL_IDX_NAME ) ;
      _indexHint = BSON( "" << STAT_IDX_IDX_NAME ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTATCB_INIT, "_dmsStatCB::init" )
   INT32 _dmsStatCB::init ()
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY ( SDB__DMSSTATCB_INIT ) ;

      _pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      dmsStorageUnitID suID = DMS_INVALID_CS ;

      SDB_ASSERT ( _dmsCB, "dmsCB can't be NULL" ) ;

      // exclusive lock SYSSTAT cb. this function should be called during
      // process initialization, so it shouldn't be called in parallel by
      // agents
      DMSSYSCB_XLOCK

      // first to load collection space
      rc = rtnCollectionSpaceLock( SYSSTAT_SPACE_NAME, _dmsCB, TRUE,
                                   &_su, suID ) ;
      if ( SDB_DMS_CS_NOTEXIST == rc )
      {
         // create new SYSSTAT collection space
         rc = rtnCreateCollectionSpaceCommand ( SYSSTAT_SPACE_NAME, NULL, _dmsCB,
                                                NULL, DMS_PAGE_SIZE_MAX,
                                                DMS_DO_NOT_CREATE_LOB, TRUE ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to create SYSSTAT collection space, rc: %d",
                      rc ) ;

         rc = rtnCollectionSpaceLock ( SYSSTAT_SPACE_NAME, _dmsCB, TRUE,
                                       &_su, suID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to lock SYSSTAT collection space, rc: %d", rc ) ;
      }
      else if ( SDB_OK != rc )
      {
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to lock collection space [%s], rc: %d",
                      SYSSTAT_SPACE_NAME, rc ) ;
      }

      _dmsCB->suUnlock( suID ) ;
      suID = DMS_INVALID_CS ;

      rc = _ensureStatMetadata( cb ) ;
      PD_RC_CHECK( rc, PDERROR, "Faild to create statistics collections or "
                   "indexes, rc: %d", rc ) ;

      _loadStats( cb ) ;

   done :
      if ( DMS_INVALID_CS != suID )
      {
         _dmsCB->suUnlock ( suID ) ;
      }
      PD_TRACE_EXITRC ( SDB__DMSSTATCB_INIT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB_RELOADSTATS, "_dmsStatCB::reloadStats" )
   INT32 _dmsStatCB::reloadStats ( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB_RELOADSTATS ) ;

      // Make sure only one reload in one time
      DMSSYSCB_XLOCK

      rc = _loadStats( cb ) ;
      PD_RC_CHECK( rc, PDWARNING, "Failed to load statistics, rc: %d", rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB_RELOADSTATS, rc ) ;
      return SDB_OK ;

   error :
      goto done ;
   }

   INT32 _dmsStatCB::_ensureStatMetadata ( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      rc = catTestAndCreateCL( SYSSTAT_COLLECTION_CL_NAME, cb, _dmsCB, NULL,
                               TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create collection [%s], rc: %d",
                   SYSSTAT_COLLECTION_CL_NAME, rc ) ;

      rc = catTestAndCreateCL( SYSSTAT_INDEX_CL_NAME, cb, _dmsCB, NULL, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create collection [%s], rc: %d",
                   SYSSTAT_INDEX_CL_NAME, rc ) ;

      {
         BSONObj idxDef ;

         rc = fromjson( STAT_CL_IDX_DEF, idxDef ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to build index object [%s], rc: %d",
                      STAT_CL_IDX_DEF, rc ) ;

         rc = catTestAndCreateIndex( SYSSTAT_COLLECTION_CL_NAME, idxDef, cb,
                                     _dmsCB, NULL, TRUE ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create index [%s], rc: %d",
                      STAT_CL_IDX_DEF, rc ) ;
      }

      {
         BSONObj idxDef ;

         rc = fromjson( SYSSTAT_IDX_IDX_DEF, idxDef ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to build index object [%s], rc: %d",
                      SYSSTAT_IDX_IDX_DEF, rc ) ;

         rc = catTestAndCreateIndex( SYSSTAT_INDEX_CL_NAME, idxDef, cb,
                                     _dmsCB, NULL, TRUE ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to create index [%s], rc: %d",
                      SYSSTAT_IDX_IDX_DEF, rc ) ;
      }

   done :
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB_LOADSTATS, "_dmsStatCB::_loadStats" )
   INT32 _dmsStatCB::_loadStats ( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB_LOADSTATS ) ;

      dmsCSStatMap csStatMap ;
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
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to load collection statistics, rc: %d", rc ) ;

      rc = _loadIndexStats( csStatMap, cb ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to load index statistics, rc: %d", rc ) ;

      for ( dmsCSStatMap::iterator iterStat = csStatMap.begin() ;
            iterStat != csStatMap.end() ;
            ++ iterStat )
      {
         rtnStatMap *pStatMap = iterStat->second ;
         if ( pStatMap )
         {
            _addCollectionStats( iterStat->first._pName, *pStatMap ) ;
         }
      }

   done :
      for ( dmsCSStatMap::iterator iterStat = csStatMap.begin() ;
            iterStat != csStatMap.end() ;
            ++ iterStat )
      {
         rtnStatMap *pStatMap = iterStat->second ;
         if ( pStatMap )
         {
            pStatMap->clearStats() ;
            SDB_OSS_DEL pStatMap ;
         }
      }

      csStatMap.clear() ;

      PD_TRACE_EXITRC( SDB_DMSSTATCB_LOADSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB_ONDROPCS, "_dmsStatCB::onDropCollectionSpace" )
   INT32 _dmsStatCB::onDropCollectionSpace ( const CHAR *pCSName,
                                             _pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB_ONDROPCS ) ;

      BSONObj boMatcher( BSON( STAT_COLLECTION_SPACE << pCSName ) ) ;

      rc = _deleteCollectionStat( boMatcher, cb, dpsCB ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to drop collection statistics when dropping "
                   "collection space [%s], rc: %d", pCSName, rc ) ;

      rc = _deleteIndexStat( boMatcher, cb, dpsCB ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to delete index statistics when dropping "
                   "collection space [%s], rc: %d", pCSName, rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB_ONDROPCS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB_ONDROPCL, "_dmsStatCB::onDropCollection" )
   INT32 _dmsStatCB::onDropCollection ( const CHAR *pCSName, UINT16 mbID,
                                        _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB_ONDROPCL ) ;

      BSONObj boMatcher( BSON( STAT_COLLECTION_SPACE << pCSName <<
                               STAT_COLLECTION_MBID << (INT32)mbID ) ) ;

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

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB_ONDROPCL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB_ONDROPIDX, "_dmsStatCB::onDropIndex" )
   INT32 _dmsStatCB::onDropIndex ( const CHAR *pCSName, UINT16 mbID,
                                   const CHAR *pIndexName, _pmdEDUCB *cb,
                                   SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB_ONDROPIDX ) ;

      BSONObj boMatcher( BSON( STAT_COLLECTION_SPACE << pCSName <<
                               STAT_COLLECTION_MBID << (INT32)mbID <<
                               STAT_IDX_INDEX << pIndexName ) ) ;

      rc = _deleteIndexStat( boMatcher, cb, dpsCB ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Failed to delete index statistics when dropping "
                   "index [ space %s mbID %d index %s ] , rc: %d",
                   pCSName, mbID, pIndexName, rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB_ONDROPIDX, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB_ONRENAMECS, "_dmsStatCB::onRenameCollectionSpace" )
   INT32 _dmsStatCB::onRenameCollectionSpace ( const CHAR *pOldCSName,
                                               const CHAR *pNewCSName,
                                               _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB_ONRENAMECS ) ;

      BSONObj boMatcher( BSON( STAT_COLLECTION_SPACE << pOldCSName ) ) ;
      BSONObj boNewName( BSON( STAT_COLLECTION_SPACE << pNewCSName ) ) ;
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

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB_ONRENAMECS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB_ONRENAMECL, "_dmsStatCB::onRenameCollection" )
   INT32 _dmsStatCB::onRenameCollection ( const CHAR *pCSName, UINT16 mbID,
                                          const CHAR *pOldCLName, const CHAR *pNewCLName,
                                          _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB_ONRENAMECL ) ;

      BSONObj boMatcher( BSON( STAT_COLLECTION_SPACE << pCSName <<
                               STAT_COLLECTION_MBID << (INT32)mbID ) ) ;
      BSONObj boNewName( BSON( STAT_COLLECTION << pNewCLName ) ) ;
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

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB_ONRENAMECL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__LOADCLSTATS, "_dmsStatCB::_loadCollectionStats" )
   INT32 _dmsStatCB::_loadCollectionStats ( dmsCSStatMap &csStatMap,
                                            _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__LOADCLSTATS ) ;

      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      INT64 contextID = -1 ;
      BSONObj boDummy ;

      // query
      rc = rtnQuery( SYSSTAT_COLLECTION_CL_NAME, boDummy, boDummy,
                     boDummy, boDummy, 0, cb, 0, -1, _dmsCB, rtnCB,
                     contextID ) ;
      PD_RC_CHECK( rc, PDWARNING, "Query collection [%s] failed, rc: %d",
                   SYSSTAT_COLLECTION_CL_NAME, rc ) ;

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

         pCollectionStat = SDB_OSS_NEW dmsCollectionStat( NULL, NULL ) ;
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
               goto error ;
            }
         }
         catch( std::exception &e )
         {
            PD_LOG( PDWARNING,
                    "Get index statistics for collection occur exception: %s",
                    e.what() ) ;
            rc = SDB_SYS ;
            SAFE_OSS_DELETE( pCollectionStat ) ;
            goto error ;
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
      PD_TRACE_EXITRC( SDB_DMSSTATCB__LOADCLSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__LOADIDXSTATS, "_dmsStatCB::_loadIndexStats" )
   INT32 _dmsStatCB::_loadIndexStats ( dmsCSStatMap &csStatMap,
                                       _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__LOADIDXSTATS ) ;

      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      INT64 contextID = -1 ;
      BSONObj boDummy ;

      // query
      rc = rtnQuery( SYSSTAT_INDEX_CL_NAME, boDummy, boDummy,
                     boDummy, boDummy, 0, cb, 0, -1, _dmsCB, rtnCB,
                     contextID ) ;
      PD_RC_CHECK( rc, PDWARNING, "Query collection [%s] failed, rc: %d",
                   SYSSTAT_INDEX_CL_NAME, rc ) ;

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

         pIndexStat = SDB_OSS_NEW dmsIndexStat( NULL, NULL, NULL ) ;
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
      PD_TRACE_EXITRC( SDB_DMSSTATCB__LOADIDXSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__CLEARSTATS, "_dmsStatCB::_clearStats" )
   INT32 _dmsStatCB::_clearStats ()
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__CLEARSTATS ) ;

      std::set<monCollectionSpace> csList ;
      _dmsCB->dumpInfo( csList, FALSE ) ;

      for ( std::set<_monCollectionSpace>::const_iterator iterCS = csList.begin() ;
            iterCS != csList.end () ;
            iterCS ++ )
      {
         const _monCollectionSpace &cs = *iterCS ;
         dmsStorageUnitID suID = DMS_INVALID_SUID ;
         dmsStorageUnit *su = NULL ;
         rtnStatMgr *pStatMgr = NULL ;

         rc = _dmsCB->nameToSUAndLock( cs._name, suID, &su, SHARED ) ;

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

         pStatMgr = su->getStatMgr() ;
         if ( pStatMgr )
         {
            pStatMgr->clearStats() ;
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Failed to get statistics manager for storage unit [%s]",
                    cs._name ) ;
         }

         if ( DMS_INVALID_SUID != suID )
         {
            _dmsCB->suUnlock( suID, SHARED ) ;
         }
      }

      PD_TRACE_EXITRC( SDB_DMSSTATCB__CLEARSTATS, rc ) ;

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__ADDCLSTAT, "_dmsStatCB::_addCollectionStat" )
   INT32 _dmsStatCB::_addCollectionStat ( dmsCSStatMap &csStatMap,
                                          dmsCollectionStat *pCollectionStat,
                                          BOOLEAN ignoreVersion )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__ADDCLSTAT ) ;

      const CHAR *pCSName = pCollectionStat->getCSName() ;
      const CHAR *pCLName = pCollectionStat->getCLName() ;
      rtnStatMap *pStatMap = NULL ;

      dmsCSStatMap::iterator iterCS = csStatMap.find( pCSName ) ;
      PD_CHECK( iterCS != csStatMap.end(),
                SDB_INVALIDARG, error, PDWARNING,
                "Failed to add collection statistics to collection space [%s]",
                pCSName ) ;

      pStatMap = iterCS->second ;
      if ( !iterCS->second )
      {
         iterCS->second = SDB_OSS_NEW rtnStatMap ( iterCS->first._pName ) ;
         pStatMap = iterCS->second ;
      }
      PD_CHECK( pStatMap,
                SDB_OOM, error, PDWARNING,
                "Could not allocate statistics manger in statistics map [%s]",
                pCSName ) ;

      PD_CHECK( pStatMap->addCollectionStat( pCollectionStat, ignoreVersion ),
                SDB_INVALIDARG, error, PDWARNING,
                "Failed to add collection statistics [%s.%s]",
                pCSName, pCLName ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB__ADDCLSTAT, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__ADDIDXSTAT, "_dmsStatCB::_addIndexStat" )
   INT32 _dmsStatCB::_addIndexStat ( dmsCSStatMap &csStatMap,
                                     dmsIndexStat *pIndexStat,
                                     BOOLEAN ignoreVersion )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__ADDIDXSTAT ) ;

      const CHAR *pCSName = pIndexStat->getCSName() ;
      const CHAR *pCLName = pIndexStat->getCLName() ;
      rtnStatMap *pStatMap = NULL ;

      dmsCSStatMap::iterator iterCS = csStatMap.find( pCSName ) ;
      PD_CHECK( iterCS != csStatMap.end(),
                SDB_INVALIDARG, error, PDWARNING,
                "Failed to add collection statistics to collection space [%s]",
                pCSName ) ;

      pStatMap = iterCS->second ;
      PD_CHECK( pStatMap,
                SDB_INVALIDARG, error, PDWARNING,
                "No statistics manger in statistics map [%s]", pCSName ) ;

      PD_CHECK( pStatMap->addIndexStat( pIndexStat, ignoreVersion ),
                SDB_INVALIDARG, error, PDWARNING,
                "Failed to add index [%s] statistics to collection [%s.%s]",
                pIndexStat->getIndexName(), pCSName, pCLName ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB__ADDIDXSTAT, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__ADDCLSTATS, "_dmsStatCB::_addCollectionStats" )
   INT32 _dmsStatCB::_addCollectionStats ( const CHAR *pCSName,
                                           rtnStatMap &statMap )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__ADDCLSTATS ) ;

      dmsStorageUnitID suID = DMS_INVALID_CS ;
      dmsStorageUnit *pSu = NULL ;

      rtnStatMgr *pStatMgr = NULL ;

      rc = _dmsCB->nameToSUAndLock ( pCSName, suID, &pSu, EXCLUSIVE, OSS_ONE_SEC ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to get collection space [%s], rc: %d",
                   pCSName, rc ) ;

      pStatMgr = pSu->getStatMgr() ;
      PD_CHECK( pStatMgr, SDB_INVALIDARG, error, PDWARNING,
                "No statistics manger in storage unit [%s]", pCSName ) ;

      pStatMgr->clearStats() ;
      pStatMgr->addCollectionStats( statMap ) ;

   done :
      if ( DMS_INVALID_CS != suID )
      {
         _dmsCB->suUnlock( suID, EXCLUSIVE ) ;
      }

      PD_TRACE_EXITRC( SDB_DMSSTATCB__ADDCLSTATS, rc ) ;
      return rc ;

   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__DELCLSTAT, "_dmsStatCB::_deleteCollectionStat" )
   INT32 _dmsStatCB::_deleteCollectionStat ( const BSONObj &boMatcher,
                                             _pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__DELCLSTAT ) ;

      rc = rtnDelete( SYSSTAT_COLLECTION_CL_NAME, boMatcher, _collectionHint,
                      0, cb, _dmsCB, dpsCB, 1 ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Delete collection statistics [%s] failed, rc: %d",
                   boMatcher.toString().c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB__DELCLSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__DELIDXSTAT, "_dmsStatCB::_deleteIndexStat" )
   INT32 _dmsStatCB::_deleteIndexStat ( const BSONObj &boMatcher, _pmdEDUCB *cb,
                                        SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__DELIDXSTAT ) ;

      rc = rtnDelete( SYSSTAT_INDEX_CL_NAME, boMatcher, _indexHint, 0, cb,
                      _dmsCB, dpsCB, 1 ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Delete index statistics [%s] failed, rc: %d",
                   boMatcher.toString().c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB__DELIDXSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__UPDATECLSTAT, "_dmsStatCB::_updateCollectionStat" )
   INT32 _dmsStatCB::_updateCollectionStat ( const BSONObj &boMatcher,
                                             const BSONObj &boUpdator,
                                             _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__UPDATECLSTAT ) ;

      rc = rtnUpdate( SYSSTAT_COLLECTION_CL_NAME, boMatcher, boUpdator,
                      _collectionHint, 0, cb, _dmsCB, dpsCB, 1 ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Update collection statistics [%s] with [%s] failed, rc: %d",
                   boMatcher.toString( FALSE, TRUE ).c_str(),
                   boUpdator.toString( FALSE, TRUE ).c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB__UPDATECLSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_DMSSTATCB__UPDATEIDXSTAT, "_dmsStatCB::_updateIndexStat" )
   INT32 _dmsStatCB::_updateIndexStat ( const BSONObj &boMatcher,
                                        const BSONObj &boUpdator,
                                        _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_DMSSTATCB__UPDATEIDXSTAT ) ;

      rc = rtnUpdate( SYSSTAT_INDEX_CL_NAME, boMatcher, boUpdator,
                      _collectionHint, 0, cb, _dmsCB, dpsCB, 1 ) ;
      PD_RC_CHECK( rc, PDWARNING,
                   "Update index statistics [%s] with [%s] failed, rc: %d",
                   boMatcher.toString( FALSE, TRUE ).c_str(),
                   boUpdator.toString( FALSE, TRUE ).c_str(), rc ) ;

   done :
      PD_TRACE_EXITRC( SDB_DMSSTATCB__UPDATEIDXSTAT, rc ) ;
      return rc ;
   error :
      goto done ;
   }

}

