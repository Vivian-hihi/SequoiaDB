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

   Source File Name = rtnStatMgr.cpp

   Descriptive Name = Runtime Statistics Manager Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Statistics
   Manager.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "rtnStatMgr.hpp"
#include "dmsCB.hpp"
#include "dmsStatCB.hpp"
#include "dmsStorageUnit.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "pmd.hpp"

namespace engine
{

   /*
      _dmsCollectionStatMap implement
    */
   _rtnStatMap::_rtnStatMap ( const CHAR *pCSName )
   {
      if ( dmsIsSysCSName( pCSName ) )
      {
         _isValid = FALSE ;
      }
      else
      {
         _isValid = TRUE ;
      }

      ossMemset( _collectionStats, 0, sizeof( _collectionStats ) ) ;
   }

   _rtnStatMap::~_rtnStatMap ()
   {
      clearStats () ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMAP_ADDCLSTAT, "_rtnStatMap::addCollectionStat" )
   BOOLEAN _rtnStatMap::addCollectionStat ( dmsCollectionStat *pCollectionStat,
                                            BOOLEAN ignoreVersion )
   {
      PD_TRACE_ENTRY( SDB_RTNSTATMAP_ADDCLSTAT ) ;

      BOOLEAN added = FALSE ;
      UINT16 mbID = DMS_INVALID_MBID ;

      if ( _isValid && pCollectionStat &&
           ( mbID = pCollectionStat->getMBID() ) < DMS_MME_SLOTS )
      {
         dmsCollectionStat *pTmpStat = _collectionStats[ mbID ] ;
         if ( pTmpStat )
         {
            if ( ignoreVersion ||
                 pTmpStat->getVersion() < pCollectionStat->getVersion() )
            {
               SDB_OSS_DEL pTmpStat ;
               _collectionStats[ mbID ] = pCollectionStat ;
               added = TRUE ;
            }
         }
         else
         {
            _collectionStats[ mbID ] = pCollectionStat ;
            added = TRUE ;
         }
      }

      PD_TRACE_EXIT( SDB_RTNSTATMAP_ADDCLSTAT ) ;

      return added ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMAP_ADDIDXSTAT, "_rtnStatMap::addIndexStat" )
   BOOLEAN _rtnStatMap::addIndexStat ( dmsIndexStat *pIndexStat,
                                       BOOLEAN ignoreVersion )
   {
      PD_TRACE_ENTRY( SDB_RTNSTATMAP_ADDIDXSTAT ) ;

      BOOLEAN added = FALSE ;
      UINT16 mbID = DMS_INVALID_MBID ;

      if ( _isValid && pIndexStat &&
           ( mbID = pIndexStat->getMBID() ) < DMS_MME_SLOTS )
      {
         dmsCollectionStat *pCollectionStat = _collectionStats[ mbID ] ;
         if ( pCollectionStat )
         {
            added = pCollectionStat->addIndexStat( pIndexStat, ignoreVersion ) ;
         }
      }

      PD_TRACE_EXIT( SDB_RTNSTATMAP_ADDIDXSTAT ) ;

      return added ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMAP_RMCLSTAT, "_rtnStatMap::removeCollectionStat" )
   BOOLEAN _rtnStatMap::removeCollectionStat ( UINT16 mbID, BOOLEAN needDelete )
   {
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB_RTNSTATMAP_RMCLSTAT ) ;

      if ( _isValid && mbID < DMS_MME_SLOTS )
      {
         if ( needDelete )
         {
            SAFE_OSS_DELETE( _collectionStats[ mbID ] ) ;
         }
         else
         {
            _collectionStats[ mbID ] = NULL ;
         }
         deleted = TRUE ;
      }

      PD_TRACE_EXIT( SDB_RTNSTATMAP_RMCLSTAT ) ;

      return deleted ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMAP_RMIDXSTAT, "_rtnStatMap::removeIndexStat" )
   BOOLEAN _rtnStatMap::removeIndexStat ( UINT16 mbID, const CHAR *pIndexName )
   {
      BOOLEAN deleted = FALSE ;

      PD_TRACE_ENTRY( SDB_RTNSTATMAP_RMIDXSTAT ) ;

      if ( _isValid && mbID < DMS_MME_SLOTS )
      {
         dmsCollectionStat *pCollectionStat = _collectionStats[ mbID ] ;
         if ( pCollectionStat )
         {
            deleted = pCollectionStat->removeIndexStat( pIndexName, TRUE ) ;
         }
      }

      PD_TRACE_EXIT( SDB_RTNSTATMAP_RMIDXSTAT ) ;

      return deleted ;
   }

   BOOLEAN _rtnStatMap::clearStats ()
   {
      BOOLEAN deleted = FALSE ;

      if ( _isValid )
      {
         for ( UINT32 idx = 0 ; idx < DMS_MME_SLOTS ; idx ++ )
         {
            if ( _collectionStats[idx] )
            {
               SDB_OSS_DEL _collectionStats[idx] ;
               _collectionStats[idx] = NULL ;
               deleted = TRUE ;
            }
         }
      }

      return deleted ;
   }

   /*
      _rtnStatMgr implement
    */
   _rtnStatMgr::_rtnStatMgr ( _dmsStorageUnit* su, const CHAR *pSUName )
   : _rtnStatMap( pSUName )
   {
      _pSu = su ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMGR_ADDCLSTATS, "_rtnStatMgr::addCollectionStats" )
   void _rtnStatMgr::addCollectionStats ( rtnStatMap &statMap )
   {
      PD_TRACE_ENTRY( SDB_RTNSTATMGR_ADDCLSTATS ) ;

      if ( !_isValid )
      {
         PD_LOG( PDDEBUG, "The statistics for collection space [%s] is "
                 "invalid", _pSu->CSName() ) ;
         return ;
      }

      for ( UINT32 idx = 0 ; idx < DMS_MME_SLOTS ; idx ++ )
      {
         INT32 rc = SDB_OK ;
         dmsMBContext *mbContext = NULL ;
         dmsCollectionStat *pCollectionStat = statMap.getCollectionStat( idx ) ;

         if ( !pCollectionStat )
         {
            continue ;
         }

         const CHAR *pCLName = pCollectionStat->getCLName() ;

         rc = _pSu->data()->getMBContext( &mbContext, pCLName, EXCLUSIVE ) ;
         if ( SDB_OK == rc )
         {
            if ( addCollectionStat( pCollectionStat, FALSE ) )
            {
               statMap.removeCollectionStat( idx, FALSE ) ;
            }
         }
         else
         {
            PD_LOG( PDWARNING,
                    "Failed to get dms mb context for collection [%s], rc: %d",
                    pCLName, rc ) ;
         }

         if ( mbContext )
         {
            _pSu->data()->releaseMBContext( mbContext ) ;
         }
      }

      PD_TRACE_EXIT( SDB_RTNSTATMGR_ADDCLSTATS ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMGR_ONDROPCS, "_rtnStatMgr::onDropCollectionSpace" )
   INT32 _rtnStatMgr::onDropCollectionSpace ( _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_RTNSTATMGR_ONDROPCS ) ;

      dmsStatCB *pStatCB = NULL ;

      PD_CHECK( _isValid, SDB_OK, done, PDDEBUG,
                "The statistics for collection space [%s] is invalid",
                _pSu->CSName() ) ;

      if ( clearStats() && dpsCB )
      {
         // No need to update SYSSTAT in backup node, the dps sync will do it
         pStatCB = sdbGetDMSCB()->getStatCB() ;
         rc = pStatCB->onDropCollectionSpace( _pSu->CSName(), cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to process statistics on dropping "
                      "collection space [%s], rc: %d", _pSu->CSName(), rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_RTNSTATMGR_ONDROPCS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMGR_ONDROPCL, "_rtnStatMgr::onDropCollection" )
   INT32 _rtnStatMgr::onDropCollection ( UINT16 mbID, _pmdEDUCB *cb,
                                         SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_RTNSTATMGR_ONDROPCL ) ;

      dmsStatCB *pStatCB = NULL ;

      PD_CHECK( _isValid, SDB_OK, done, PDDEBUG,
                "The statistics for collection space [%s] is invalid",
                _pSu->CSName() ) ;

      if ( removeCollectionStat( mbID, TRUE ) && dpsCB )
      {
         // No need to update SYSSTAT in backup node, the dps sync will do it
         pStatCB = sdbGetDMSCB()->getStatCB() ;
         rc = pStatCB->onDropCollection( _pSu->CSName(), mbID, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to process statistics on dropping "
                      "collection [ space %s mbID %d ], rc: %d",
                      _pSu->CSName(), mbID, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_RTNSTATMGR_ONDROPCL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMGR_ONDROPIDX, "_rtnStatMgr::onDropIndex" )
   INT32 _rtnStatMgr::onDropIndex ( UINT16 mbID, const CHAR *pIndexName,
                                    _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_RTNSTATMGR_ONDROPIDX ) ;

      dmsStatCB *pStatCB = NULL ;

      PD_CHECK( _isValid, SDB_OK, done, PDDEBUG,
                "The statistics for collection space [%s] is invalid",
                _pSu->CSName() ) ;

      if ( removeIndexStat( mbID, pIndexName ) && dpsCB )
      {
         // No need to update SYSSTAT in backup node, the dps sync will do it
         pStatCB = sdbGetDMSCB()->getStatCB() ;
         rc = pStatCB->onDropIndex( _pSu->CSName(), mbID, pIndexName, cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to process statistics on dropping "
                      "index [ space %s mbID %d index %s ], rc: %d",
                      _pSu->CSName(), mbID, pIndexName, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_RTNSTATMGR_ONDROPIDX, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMGR_ONRENAMECS, "_rtnStatMgr::onRenameCollectionSpace" )
   INT32 _rtnStatMgr::onRenameCollectionSpace ( const CHAR *pOldCSName,
                                                const CHAR *pNewCSName,
                                                _pmdEDUCB *cb, SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_RTNSTATMGR_ONRENAMECS ) ;

      dmsStatCB *pStatCB = NULL ;
      BOOLEAN renamed = FALSE ;

      PD_CHECK( _isValid, SDB_OK, done, PDDEBUG,
                "The statistics for collection space [%s] is invalid",
                _pSu->CSName() ) ;

      for ( UINT16 mbID = 0 ; mbID < DMS_MME_SLOTS ; mbID ++ )
      {
         dmsCollectionStat *pCollectionStat = _collectionStats[ mbID ] ;
         if ( pCollectionStat )
         {
            pCollectionStat->setCSName( pNewCSName ) ;
            renamed = TRUE ;
         }
      }

      if ( renamed && dpsCB )
      {
         // No need to update SYSSTAT in backup node, the dps sync will do it
         pStatCB = sdbGetDMSCB()->getStatCB() ;
         rc = pStatCB->onRenameCollectionSpace( pOldCSName, pNewCSName,
                                                cb, dpsCB ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to process statistics on renaming "
                      "collection space [%s] to [%s], rc: %d",
                      pOldCSName, pNewCSName, rc ) ;
      }

   done :
      PD_TRACE_EXITRC( SDB_RTNSTATMGR_ONRENAMECS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNSTATMGR_ONRENAMECL, "_rtnStatMgr::onRenameCollection" )
   INT32 _rtnStatMgr::onRenameCollection ( UINT16 mbID, const CHAR *pOldCLName,
                                           const CHAR *pNewCLName, _pmdEDUCB *cb,
                                           SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB_RTNSTATMGR_ONRENAMECL ) ;

      dmsStatCB *pStatCB = NULL ;
      BOOLEAN renamed = FALSE ;

      PD_CHECK( _isValid, SDB_OK, done, PDDEBUG,
                "The statistics for collection space [%s] is invalid",
                _pSu->CSName() ) ;

      if ( mbID < DMS_MME_SLOTS )
      {
         dmsCollectionStat *pCollectionStat = _collectionStats[ mbID ] ;
         if ( pCollectionStat )
         {
            pCollectionStat->setCLName( pNewCLName ) ;
            renamed = TRUE ;
         }

         if ( renamed && dpsCB )
         {
            // No need to update SYSSTAT in backup node, the dps sync will do it
            pStatCB = sdbGetDMSCB()->getStatCB() ;
            rc = pStatCB->onRenameCollection( _pSu->CSName(), mbID, pOldCLName,
                                              pNewCLName, cb, dpsCB ) ;
            PD_RC_CHECK( rc, PDWARNING, "Failed to process statistics on renaming "
                         "collection [%s.%s] to [%s.%s], rc: %d",
                         _pSu->CSName(), pOldCLName, _pSu->CSName(), pNewCLName, rc ) ;
         }
      }

   done :
      PD_TRACE_EXITRC( SDB_RTNSTATMGR_ONRENAMECL, rc ) ;
      return rc ;
   error :
      goto done ;
   }

}

