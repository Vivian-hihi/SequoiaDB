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

   Source File Name = catSequenceManager.cpp

   Descriptive Name = Sequence manager

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/19/2018  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "catSequenceManager.hpp"
#include "catGTSDef.hpp"
#include "dmsCB.hpp"
#include "dpsLogWrapper.hpp"
#include "rtn.hpp"
#include "rtnContextBuff.hpp"
#include "pmd.hpp"
#include "pdTrace.hpp"
#include "catTrace.hpp"
#include "pd.hpp"

using namespace bson ;

namespace engine
{
   _catSequenceManager::_catSequenceManager()
   {
   }

   _catSequenceManager::~_catSequenceManager()
   {
      _cleanCache( FALSE ) ;
   }

   INT32 _catSequenceManager::active()
   {
      _cleanCache( FALSE ) ;
      return SDB_OK ;
   }

   INT32 _catSequenceManager::deactive()
   {
      _cleanCache( TRUE ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR_CREATE_SEQ, "_catSequenceManager::createSequence" )
   INT32 _catSequenceManager::createSequence( const std::string& name,
                                              const BSONObj& options,
                                              _pmdEDUCB* eduCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR_CREATE_SEQ ) ;

      _catSequence sequence = _catSequence( name ) ;

      rc = sequence.setOptions( options, TRUE, FALSE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to set sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      sequence.setOID( OID::gen() ) ;
      sequence.setVersion( 0 ) ;
      sequence.setInitial( TRUE ) ;

      rc = sequence.validate() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Invalid sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      rc = sequence.toBSONObj( obj, FALSE ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      {
         CAT_SEQ_MAP::Bucket& bucket = _sequenceCache.getBucket( name ) ;
         BUCKET_XLOCK( bucket ) ;

         CAT_SEQ_MAP::map_const_iterator iter = bucket.find( name ) ;
         if ( bucket.end() != iter )
         {
            rc = SDB_SEQUENCE_EXIST ;
            PD_LOG( PDERROR, "Sequence %s is already existing, rc=%d", name.c_str(), rc ) ;
            goto error ;
         }

         rc = _insertSequence( obj, eduCB, w ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to insert sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR_CREATE_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR_DROP_SEQ, "_catSequenceManager::dropSequence" )
   INT32 _catSequenceManager::dropSequence( const std::string& name,
                                            _pmdEDUCB* eduCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR_DROP_SEQ ) ;

      CAT_SEQ_MAP::Bucket& bucket = _sequenceCache.getBucket( name ) ;
      BUCKET_XLOCK( bucket ) ;

      CAT_SEQ_MAP::map_const_iterator iter = bucket.find( name ) ;
      if ( bucket.end() != iter )
      {
         _catSequence* sequence = (*iter).second ;
         SDB_OSS_DEL sequence ;
         bucket.erase( name ) ;
      }

      rc = _deleteSequence( name, eduCB, w ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR_DROP_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR_ALTER_SEQ, "_catSequenceManager::alterSequence" )
   INT32 _catSequenceManager::alterSequence( const std::string& name,
                                             const BSONObj& options,
                                             _pmdEDUCB* eduCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      BOOLEAN changed = FALSE ;
      _catSequence* cache = NULL ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR_ALTER_SEQ ) ;

      _catSequence sequence = _catSequence( name ) ;

      CAT_SEQ_MAP::Bucket& bucket = _sequenceCache.getBucket( name ) ;
      BUCKET_XLOCK( bucket ) ;

      CAT_SEQ_MAP::map_const_iterator iter = bucket.find( name ) ;
      if ( bucket.end() != iter )
      {
         cache = (*iter).second ;
         cache->copyTo( sequence, TRUE ) ;
      }
      else
      {
         BSONObj seqObj ;
         rc = _findSequence( name, seqObj, eduCB ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to find sequence[%s] from system collection, rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }

         rc = sequence.setOptions( seqObj, FALSE, TRUE ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to set sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }
      }

      rc = sequence.setOptions( options, FALSE, FALSE, &changed ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to set sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      if ( !changed )
      {
         // nothing changed
         goto done ;
      }

      sequence.increaseVersion() ;

      rc = sequence.validate() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Invalid sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      rc = sequence.toBSONObj( obj, TRUE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to build BSONObj for sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      rc = _updateSequence( name, obj, eduCB, w ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to update sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      if ( NULL != cache )
      {
         sequence.copyTo( *cache, TRUE ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR_ALTER_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR_ACQUIRE_SEQ, "_catSequenceManager::acquireSequence" )
   INT32 _catSequenceManager::acquireSequence( const std::string& name,
                                               const bson::OID oid,
                                               _catSequenceAcquirer& acquirer,
                                               _pmdEDUCB* eduCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      _catSequence* cache = NULL ;
      _catSequence* sequence = NULL ;
      BOOLEAN needUpdate = FALSE ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR_ACQUIRE_SEQ ) ;

      CAT_SEQ_MAP::Bucket& bucket = _sequenceCache.getBucket( name ) ;
      BUCKET_XLOCK( bucket ) ;

      CAT_SEQ_MAP::map_const_iterator iter = bucket.find( name ) ;
      if ( bucket.end() != iter )
      {
         cache = (*iter).second ;
      }
      else
      {
         BSONObj seqObj ;

         rc = _findSequence( name, seqObj, eduCB ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to find sequence[%s] from system collection, rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }

         sequence = SDB_OSS_NEW _catSequence( name ) ;
         if ( NULL == sequence )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Failed to alloc sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }

         rc = sequence->setOptions( seqObj, FALSE, TRUE ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to set sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }

         try
         {
            bucket.insert( CAT_SEQ_MAP::value_type( name, sequence ) ) ;
         }
         catch( std::exception& e )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Failed to insert sequence[%s] to cache", name.c_str() ) ;
            goto error ;
         }

         cache = sequence ;
         sequence = NULL ;
      }

      // check oid
      if ( oid.isSet() && oid != cache->oid() )
      {
         PD_LOG( PDWARNING, "Mismatch oid(%s) for sequence[%s, %s]",
                 oid.str().c_str(), cache->name().c_str(), cache->oid().str().c_str() ) ;
      }

      if ( cache->increment() > 0 )
      {
         rc = _acquireAscendingSequence( *cache, acquirer, needUpdate ) ;
      }
      else
      {
         rc = _acquireDescendingSequence( *cache, acquirer, needUpdate );
      }

      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to acquire sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      acquirer.oid = cache->oid() ;

      if ( needUpdate )
      {
         BSONObj options ;
         try
         {
            options = BSON( CAT_SEQUENCE_CURRENT_VALUE << cache->currentValue()
                         << CAT_SEQUENCE_INITIAL << (bool) cache->initial() ) ;
         }
         catch( std::exception& e )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Failed to build bson, exception: %s, rc=%d",
                    e.what(), rc ) ;
            goto error ;
         }

         rc = _updateSequence( name, options, eduCB, w ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to update sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }
      }

   done:
      SAFE_OSS_DELETE( sequence ) ;
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR_ACQUIRE_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR_RESET_SEQ, "_catSequenceManager::resetSequence" )
   INT32 _catSequenceManager::resetSequence( const std::string& name, _pmdEDUCB* eduCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      _catSequence* cache = NULL ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR_RESET_SEQ ) ;

      _catSequence sequence = _catSequence( name ) ;

      CAT_SEQ_MAP::Bucket& bucket = _sequenceCache.getBucket( name ) ;
      BUCKET_XLOCK( bucket ) ;

      CAT_SEQ_MAP::map_const_iterator iter = bucket.find( name ) ;
      if ( bucket.end() != iter )
      {
         cache = (*iter).second ;
         cache->copyTo( sequence, TRUE ) ;
      }
      else
      {
         BSONObj seqObj ;
         rc = _findSequence( name, seqObj, eduCB ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to find sequence[%s] from system collection, rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }

         rc = sequence.setOptions( seqObj, FALSE, TRUE ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to set sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }
      }

      sequence.setCachedValue( sequence.startValue() ) ;
      sequence.setCurrentValue( sequence.startValue() ) ;
      sequence.setInitial( TRUE ) ;
      sequence.setExceeded( FALSE ) ;
      sequence.increaseVersion() ;

      rc = sequence.toBSONObj( obj, TRUE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to build BSONObj for sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      rc = _updateSequence( name, obj, eduCB, w ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to reset sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      if ( NULL != cache )
      {
         sequence.copyTo( *cache, TRUE ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR_RESET_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR__ACQUIRE_ASCENDING_SEQ, "_catSequenceManager::_acquireAscendingSequence" )
   INT32 _catSequenceManager::_acquireAscendingSequence( _catSequence& seq, _catSequenceAcquirer& acquirer, BOOLEAN& needUpdate )
   {
      INT32 rc = SDB_OK ;
      INT64 nextValue = 0 ;
      INT64 fetchInc = 0 ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR__ACQUIRE_ASCENDING_SEQ ) ;

      SDB_ASSERT( seq.increment() > 0, "increment should be > 0" ) ;

      needUpdate = FALSE ;

      if ( seq.initial() )
      {
         nextValue = seq.startValue() ;
         seq.setInitial( FALSE ) ;
         needUpdate = TRUE ;
      }
      else if ( seq.exceeded() )
      {
         // reach the maximum limit
         if ( !seq.cycled() )
         {
            rc = SDB_SEQUENCE_EXCEEDED ;
            PD_LOG( PDERROR, "Sequence[%s] value(%lld) is reach the maximum value(%lld)",
                    seq.name().c_str(), seq.currentValue(), seq.maxValue() ) ;
            goto error ;
         }
         else
         {
            // restart from minValue
            nextValue = seq.minValue() ;
            seq.setCurrentValue( nextValue ) ;
            seq.setCachedValue( nextValue ) ;
            seq.setExceeded( FALSE ) ;
            needUpdate = TRUE ;
         }
      }
      else
      {
         // safe to increase value
         nextValue = seq.cachedValue() + seq.increment() ;
      }

      fetchInc = ( seq.acquireSize() - 1 ) * (INT64) seq.increment() ;

      // use minus to avoid overflow
      if ( seq.currentValue() - nextValue >= fetchInc )
      {
         seq.setCachedValue( nextValue + fetchInc ) ;
         acquirer.nextValue = nextValue ;
         acquirer.acquireSize = seq.acquireSize() ;
         acquirer.increment = seq.increment() ;
      }
      else
      {
         INT64 cachedInc = seq.cacheSize() * (INT64) seq.increment() ;
         if ( seq.currentValue() <= seq.maxValue() - cachedInc )
         {
            seq.setCachedValue( nextValue + fetchInc ) ;
            seq.setCurrentValue( seq.currentValue() + cachedInc ) ;
         }
         else
         {
            INT64 newCurrentValue = seq.currentValue() + 
               ( seq.maxValue() - seq.currentValue() ) / seq.increment() * seq.increment() ;
            seq.setCurrentValue( newCurrentValue ) ;
            // use minus to avoid overflow
            if ( seq.currentValue() - nextValue >= fetchInc )
            {
               seq.setCachedValue( nextValue + fetchInc ) ;
            }
            else
            {
               INT64 newCachedValue = seq.cachedValue() +
                  ( seq.currentValue() - seq.cachedValue() ) / seq.increment() * seq.increment() ;
               seq.setCachedValue( newCachedValue ) ;
            }
         }

         acquirer.nextValue = nextValue ;
         acquirer.acquireSize = ( seq.cachedValue() - nextValue ) / seq.increment() + 1 ;
         acquirer.increment = seq.increment() ;
         needUpdate = TRUE ;
      }

      // reach the maxValue, so mark exceeded
      if ( seq.cachedValue() == seq.maxValue() ||
           seq.cachedValue() > seq.maxValue() - seq.increment() )
      {
         seq.setExceeded( TRUE ) ;
         needUpdate = TRUE ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR__ACQUIRE_ASCENDING_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR__ACQUIRE_DESCENDING_SEQ, "_catSequenceManager::_acquireDescendingSequence" )
   INT32 _catSequenceManager::_acquireDescendingSequence( _catSequence& seq, _catSequenceAcquirer& acquirer, BOOLEAN& needUpdate )
   {
      INT32 rc = SDB_OK ;
      INT64 nextValue = 0 ;
      INT64 fetchInc = 0 ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR__ACQUIRE_DESCENDING_SEQ ) ;

      SDB_ASSERT( seq.increment() < 0, "increment should be < 0" ) ;

      needUpdate = FALSE ;

      if ( seq.initial() )
      {
         nextValue = seq.startValue() ;
         seq.setInitial( FALSE ) ;
         needUpdate = TRUE ;
      }
      else if ( seq.exceeded() )
      {
         // reach the minimum limit
         if ( !seq.cycled() )
         {
            rc = SDB_SEQUENCE_EXCEEDED ;
            PD_LOG( PDERROR, "Sequence[%s] value(%lld) is reach the minimum value(%lld)",
                    seq.name().c_str(), seq.currentValue(), seq.minValue() ) ;
            goto error ;
         }
         else
         {
            // restart from maxValue
            nextValue = seq.maxValue() ;
            seq.setCurrentValue( nextValue ) ;
            seq.setCachedValue( nextValue ) ;
            seq.setExceeded( FALSE ) ;
            needUpdate = TRUE ;
         }
      }
      else
      {
         // safe to decrease value
         nextValue = seq.cachedValue() + seq.increment() ;
      }

      fetchInc = ( seq.acquireSize() - 1 ) * (INT64) seq.increment() ;

      // use minus to avoid overflow
      if ( nextValue - seq.currentValue() >= -fetchInc )
      {
         seq.setCachedValue( nextValue + fetchInc ) ;
         acquirer.nextValue = nextValue ;
         acquirer.acquireSize = seq.acquireSize() ;
         acquirer.increment = seq.increment() ;
      }
      else
      {
         INT64 cachedInc = seq.cacheSize() * (INT64) seq.increment() ;
         if ( seq.currentValue() >= seq.minValue() - cachedInc )
         {
            seq.setCachedValue( nextValue + fetchInc ) ;
            seq.setCurrentValue( seq.currentValue() + cachedInc ) ;
         }
         else
         {
            INT64 newCurrentValue = seq.currentValue() + 
               ( seq.minValue() - seq.currentValue() ) / seq.increment() * seq.increment() ;
            seq.setCurrentValue( newCurrentValue ) ;
            // use minus to avoid overflow
            if ( nextValue - seq.currentValue() >= -fetchInc )
            {
               seq.setCachedValue( nextValue + fetchInc ) ;
            }
            else
            {
               INT64 newCachedValue = seq.cachedValue() +
                  ( seq.currentValue() - seq.cachedValue() ) / seq.increment() * seq.increment() ;
               seq.setCachedValue( newCachedValue ) ;
            }
         }

         acquirer.nextValue = nextValue ;
         acquirer.acquireSize = ( seq.cachedValue() - nextValue ) / seq.increment() + 1 ;
         acquirer.increment = seq.increment() ;
         needUpdate = TRUE ;
      }

      // reach the minValue, so mark exceeded
      if ( seq.cachedValue() == seq.minValue() ||
           seq.cachedValue() < seq.minValue() - seq.increment() )
      {
         seq.setExceeded( TRUE ) ;
         needUpdate = TRUE ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR__ACQUIRE_DESCENDING_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR__INSERT_SEQ, "_catSequenceManager::_insertSequence" )
   INT32 _catSequenceManager::_insertSequence( BSONObj& sequence,
                                               _pmdEDUCB* eduCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR__INSERT_SEQ ) ;

      SDB_DMSCB* dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_DPSCB* dpsCB = pmdGetKRCB()->getDPSCB() ;

      rc = rtnInsert( GTS_SEQUENCE_COLLECTION_NAME,
                      sequence, 1, 0, eduCB,
                      dmsCB, dpsCB, w ) ;
      if ( SDB_OK != rc && SDB_IXM_DUP_KEY != rc )
      {
         BSONObj hint ;
         rtnDelete( GTS_SEQUENCE_COLLECTION_NAME,
                    sequence, hint, 0, eduCB,
                    dmsCB, dpsCB ) ;
         goto error ;
      }
      else if ( SDB_IXM_DUP_KEY == rc )
      {
         rc = SDB_SEQUENCE_EXIST ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR__INSERT_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR__DELETE_SEQ, "_catSequenceManager::_deleteSequence" )
   INT32 _catSequenceManager::_deleteSequence( const std::string& name,
                                               _pmdEDUCB* eduCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      INT64 deleteNum = 0 ;
      BSONObj hint ;
      BSONObj matcher ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR__DELETE_SEQ ) ;

      SDB_DMSCB* dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_DPSCB* dpsCB = pmdGetKRCB()->getDPSCB() ;

      try
      {
         matcher = BSON( CAT_SEQUENCE_NAME << name ) ;
      }
      catch( std::exception& e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to build delete matcher for sequence[%s], exception=%s",
                 name.c_str(), e.what() ) ;
         goto error ;
      }

      rc = rtnDelete( GTS_SEQUENCE_COLLECTION_NAME,
                      matcher, hint, 0, eduCB,
                      dmsCB, dpsCB, w, &deleteNum ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to delete sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      if ( 0 == deleteNum )
      {
         rc = SDB_SEQUENCE_NOT_EXIST ;
         PD_LOG( PDERROR, "Sequence[%s] is not found, rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }
      else if ( deleteNum > 1 )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected delete num[%lld] for sequence[%s], rc=%d",
                 deleteNum, name.c_str(), rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR__DELETE_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR__UPDATE_SEQ, "_catSequenceManager::_updateSequence" )
   INT32 _catSequenceManager::_updateSequence( const std::string& name,
                                               const BSONObj& options,
                                               _pmdEDUCB* eduCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      BSONObj matcher ;
      BSONObj updator ;
      BSONObj hint ;
      INT64 updateNum = 0 ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR__UPDATE_SEQ ) ;

      SDB_DMSCB* dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_DPSCB* dpsCB = pmdGetKRCB()->getDPSCB() ;

      SDB_ASSERT( !options.hasField( CAT_SEQUENCE_NAME ), "can't have name" ) ;

      try
      {
         matcher = BSON( CAT_SEQUENCE_NAME << name ) ;
         updator = BSON( "$set" << options ) ;
      }
      catch( std::exception& e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to build matcher or updator for sequence[%s], exception=%s",
                 name.c_str(), e.what() ) ;
         goto error ;
      }

      rc = rtnUpdate( GTS_SEQUENCE_COLLECTION_NAME,
                      matcher, updator, hint, 0, eduCB,
                      dmsCB, dpsCB, w, &updateNum ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to update sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      if ( 0 == updateNum )
      {
         rc = SDB_SEQUENCE_NOT_EXIST ;
         PD_LOG( PDERROR, "Sequence[%s] is not found, rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }
      else if ( updateNum > 1 )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected update num[%lld] for sequence[%s], rc=%d",
                 updateNum, name.c_str(), rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR__UPDATE_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR__FIND_SEQ, "_catSequenceManager::_findSequence" )
   INT32 _catSequenceManager::_findSequence( const std::string& name,
                                             BSONObj& sequence,
                                             _pmdEDUCB* eduCB )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isExist = FALSE ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj orderBy ;
      BSONObj hint ;
      INT64 contextID = -1 ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR__FIND_SEQ ) ;

      SDB_DMSCB* dmsCB = pmdGetKRCB()->getDMSCB() ;
      SDB_RTNCB* rtnCB = pmdGetKRCB()->getRTNCB() ;

      try
      {
         matcher = BSON( CAT_SEQUENCE_NAME << name ) ;
      }
      catch( std::exception& e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to build update matcher for sequence[%s], exception=%s",
                 name.c_str(), e.what() ) ;
         goto error ;
      }

      rc = rtnQuery( GTS_SEQUENCE_COLLECTION_NAME,
                     selector, matcher, orderBy, hint, 0, eduCB, 0, 1,
                     dmsCB, rtnCB, contextID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to query sequence[%s], rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

      while( TRUE )
      {
         rtnContextBuf buffObj ;

         rc = rtnGetMore ( contextID, 1, buffObj, eduCB, rtnCB ) ;
         if ( SDB_OK != rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            PD_LOG( PDERROR, "Failed to get sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }

         {
            BSONObj result( buffObj.data() ) ;
            sequence = result.copy() ;
            isExist = TRUE ;
            break ;
         }
      }

      if ( FALSE == isExist )
      {
         rc = SDB_SEQUENCE_NOT_EXIST ;
         PD_LOG( PDERROR, "Sequence[%s] is not found, rc=%d",
                 name.c_str(), rc ) ;
         goto error ;
      }

   done:
      if( -1 != contextID )
      {
         rtnCB->contextDelete ( contextID, eduCB ) ;
         contextID = -1 ;
      }
      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR__FIND_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GTS_SEQ_MGR__CLEAN_CACHE, "_catSequenceManager::_cleanCache" )
   void _catSequenceManager::_cleanCache( BOOLEAN needFlush )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_GTS_SEQ_MGR__CLEAN_CACHE ) ;

      pmdEDUCB* eduCB = pmdGetThreadEDUCB() ;

      for ( CAT_SEQ_MAP::bucket_iterator bucketIt = _sequenceCache.begin() ;
            bucketIt != _sequenceCache.end() ;
            bucketIt++ )
      {
         CAT_SEQ_MAP::Bucket& bucket = *bucketIt ;
         BUCKET_XLOCK( bucket ) ;

         for ( CAT_SEQ_MAP::map_const_iterator it = bucket.begin() ;
               it != bucket.end() ;
               it++ )
         {
            _catSequence* cache = (*it).second ;
            BSONObj options ;
            rc = SDB_OK ;

            if ( needFlush )
            {
               try
               {
                  options = BSON( CAT_SEQUENCE_CURRENT_VALUE << cache->cachedValue()
                               << CAT_SEQUENCE_INITIAL << (bool) cache->initial() ) ;

                  rc = _updateSequence( cache->name(), options, eduCB, 1 ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDWARNING, "Failed to flush sequence[%s], rc=%d",
                             cache->name().c_str(), rc ) ; 
                  }
               }
               catch( std::exception& e )
               {
                  rc = SDB_SYS ;
                  PD_LOG( PDWARNING, "Failed to flush sequence[%s], exception: %s, rc=%d",
                          cache->name().c_str(), e.what(), rc ) ;
               }
            }

            SDB_OSS_DEL ( cache ) ;
         }

         bucket.clear() ;
      }

      PD_TRACE_EXITRC ( SDB_GTS_SEQ_MGR__CLEAN_CACHE, rc ) ;
   }
}

