#include "dms.hpp"
#include "dmsCB.hpp"
#include "dmsScanner.hpp"
#include "dmsStorageUnit.hpp"
#include "pmd.hpp"
#include "rtnTrace.hpp"
#include "rtnDictCreatorJob.hpp"

namespace engine
{
   /*
    * Threshold of record number and data total size to create dictionary:
    * 100 records and 10M data. If the final fetched records is less then 80% of
    * these thresholds( thats 80 records and 8M data ), the current create
    * operation will stop, and wait for the next round to create.
    */
   #define RTN_DICT_CREATE_REC_NUM_THRESHOLD    100
   #define RTN_DICT_CREATE_REC_DATA_SIZE        ( 64 << 20 )
   #define RTN_DICT_CREATE_MIN_REC_NUM    \
                              ( RTN_DICT_CREATE_REC_NUM_THRESHOLD * 4 / 5 )
   #define RTN_DICT_CREATE_MIN_DATA_SIZE  \
                              ( RTN_DICT_CREATE_REC_DATA_SIZE * 4 / 5 )
   #define RTN_DICT_BUF_SIZE ( 64 << 20 )
   #define RTN_DICT_LOAD_TO_CACHE_MAX_TRY 3

   _rtnDictCreatorJob::_rtnDictCreatorJob( UINT32 scanInterval )
      : _dictionary( NULL ) ,
        _scanInterval( scanInterval ),
        _srcDataBuf( NULL ),
        _srcDataLen( 0 )
   {
   }

   _rtnDictCreatorJob::~_rtnDictCreatorJob()
   {
   }

   RTN_JOB_TYPE _rtnDictCreatorJob::type () const
   {
      return RTN_JOB_CREATE_DICT;
   }

   const CHAR* _rtnDictCreatorJob::name() const
   {
      return "Job[DictionaryCreator]" ;
   }

   BOOLEAN _rtnDictCreatorJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTN_DICTCREATORJOB_DOIT, "_rtnDictCreatorJob::doit" )
   INT32 _rtnDictCreatorJob::doit ()
   {
      /*
       * This thread will check all the collections in the list. If the
       * condition of creating dictionary is matched, the dictionary of the
       * collection will be created. And once this is done successfully, the
       * collection will be removed from the list, and will never be put into
       * it again. Otherwise it will be put back into the list and try to create
       * again.
       * Note:
       * This job thread should be started after the dictionary caches of all
       * storage units have been created. That is done during the control block
       * initialization phase. So it's fine.
       */
      PD_TRACE_ENTRY ( SDB__RTN_DICTCREATORJOB_DOIT ) ;

      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      pmdEDUMgr *eduMgr = krcb->getEDUMgr() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      pmdEDUCB *cb = eduCB() ;
      pmdEDUEvent event ;
      dmsStorageUnitID suID = DMS_INVALID_SUID ;
      UINT16 mbID = DMS_INVALID_MBID ;
      BOOLEAN listEmpty = FALSE ;
      UINT64 lastStartTime = pmdGetDBTick() ;

      _srcDataBuf = ( CHAR * )SDB_OSS_MALLOC( RTN_DICT_BUF_SIZE ) ;
      PD_CHECK( _srcDataBuf, SDB_OOM, error, PDERROR,
                "Failed to allocate buffer memory for creating dictionary, "
                "requested size: %d", RTN_DICT_BUF_SIZE ) ;
      while ( !PMD_IS_DB_DOWN() && !cb->isForced() )
      {
         /*
          * Before any one is found in the queue, the status of this thread is
          * wait. Once found, it will be changed to running.
          */
         eduMgr->waitEDU( cb->getID() ) ;
         /* Get the first item in the dictionary waiting list. */
         dmsCB->dispatchDictCreateCL( listEmpty, suID, mbID ) ;
         if ( listEmpty )
         {
            /* If no colleciton is waitting for dictionary creating, wait... */
            while ( pmdGetTickSpanTime( lastStartTime ) < _scanInterval )
            {
               cb->waitEvent( event, OSS_ONE_SEC ) ;
            }

            /*
             * Resume all the ones which skipped before, and start the next
             * round.
             */
            dmsCB->dictCreateResumeWaitCL() ;
            lastStartTime = pmdGetDBTick() ;
            continue ;
         }

         eduMgr->activateEDU( cb->getID() ) ;

         /*
          * Check with the fetched storage unit id and mb id. Any arror happened
          * during the creation of the dictionary, it should be skipped this
          * time, and try again in the next round. If everything goes fine,
          * remove it from the list, and never check it again.
          */
         if ( SDB_OK != _checkAndCreateDictForCL( suID, mbID ) )
         {
            dmsCB->skipCurrentDictCreateCL() ;
         }
         else
         {
            dmsCB->popFromDictCreateCLList() ;
         }

         cb->incEventCount() ;
      }

   done:
      if ( _srcDataBuf )
      {
         SDB_OSS_FREE( _srcDataBuf ) ;
         _srcDataBuf = NULL ;
      }
      PD_TRACE_EXIT( SDB__RTN_DICTCREATORJOB_DOIT ) ;
      return rc;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_STARTDICTCREATORJOB, "startDictCreatorJob" )
   INT32 startDictCreatorJob ( EDUID *pEDUID, UINT32 scanInterval )
   {
      INT32 rc = SDB_OK ;
      rtnDictCreatorJob *pJob = NULL;
      PD_TRACE_ENTRY ( SDB_STARTDICTCREATORJOB ) ;

      pJob = SDB_OSS_NEW rtnDictCreatorJob( scanInterval ) ;
      if ( !pJob )
      {
         rc = SDB_OOM ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to allocate memory for dictionary creator" ) ;
      }

      rc = rtnGetJobMgr()->startJob ( pJob, RTN_JOB_MUTEX_RET, pEDUID ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to start dictionary creator job, rc = %d", rc ) ;
   done :
      PD_TRACE_EXITRC ( SDB_STARTDICTCREATORJOB, rc ) ;
      return rc ;
   error :
      SDB_OSS_DEL pJob ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTN_DICTCREATORJOB__CONDITIONMATCH, "_rtnDictCreatorJob::_conditionMatch" )
   BOOLEAN _rtnDictCreatorJob::_conditionMatch( dmsStorageUnit *su,
                                                UINT16 mbID )
   {
      PD_TRACE_ENTRY( SDB__RTN_DICTCREATORJOB__CONDITIONMATCH ) ;
      const dmsMBStatInfo *mbStatInfo = NULL ;
      UINT64 totalSize = 0 ;
      BOOLEAN rc = FALSE ;

      mbStatInfo = su->data()->getMBStatInfo( mbID ) ;
      SDB_ASSERT( mbStatInfo, "mbStatInfo should never be null" ) ;
      totalSize = mbStatInfo->_totalDataPages * su->getPageSize() -
                         mbStatInfo->_totalDataFreeSpace ;

      if ( mbStatInfo->_totalRecords >= RTN_DICT_CREATE_REC_NUM_THRESHOLD
           && totalSize >= RTN_DICT_CREATE_REC_DATA_SIZE )
      {
         rc = TRUE ;
      }
      else
      {
         rc = FALSE ;
      }

      PD_TRACE_EXIT( SDB__RTN_DICTCREATORJOB__CONDITIONMATCH ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTN_DICTCREATORJOB__CREATEDICT, "_rtnDictCreatorJob::_createDict" )
   INT32 _rtnDictCreatorJob::_createDict( dmsStorageData *sd,
                                          dmsMBContext *context )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTN_DICTCREATORJOB__CREATEDICT ) ;
      dmsRecordID recordID ;
      ossValuePtr recordDataPtr = 0 ;
      pmdEDUCB *cb = pmdGetThreadEDUCB() ;
      UINT32 fetchNum = 0 ;
      UINT64 fetchSize = 0 ;
      BOOLEAN noMoreRecord = FALSE ;
      UINT32 srcDataLen = 0 ;
      UINT32 bufFreeLen = RTN_DICT_BUF_SIZE ;
      BOOLEAN dictFull = FALSE ;

      SDB_ASSERT( sd && context, "Invalid argument value" ) ;

      dmsExtScanner scanner( sd, context, NULL, context->mb()->_firstExtentID ) ;
      ossMemset( _srcDataBuf, 0, RTN_DICT_BUF_SIZE ) ;

      /*
       * The loop will end either all records have been fetched, or the
       * dictionary is full.
       */
      do
      {
         rc = scanner.advance( recordID, recordDataPtr, cb, NULL ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = scanner.stepToNextExtent() ;
            if ( SDB_DMS_EOC == rc )
            {
               /* If the dictionary is not full, use the last batch of data. */
               noMoreRecord = TRUE ;
               rc = SDB_OK ;
               break ;
            }
            continue ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to fetch record when creating "
                      "dictionary, rc: %d", rc ) ;

         try
         {
            BSONObj bs( (const CHAR*)recordDataPtr ) ;
            _creator.build( bs.objdata(), bs.objsize(), dictFull ) ;
            if ( dictFull )
            {
               break ;
            }

            fetchNum++ ;
            fetchSize += bs.objsize() ;

            if ( (UINT32)bs.objsize() <= bufFreeLen )
            {
               ossMemcpy( _srcDataBuf + srcDataLen, bs.objdata(), bs.objsize() ) ;
               bufFreeLen -= bs.objsize() ;
               srcDataLen += bs.objsize() ;
               continue ;
            }
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }while ( FALSE == noMoreRecord ) ;

      PD_CHECK( SDB_OK == rc, rc, error, PDERROR,
                "Failed to fetch record when building dictionary, rc: %d, "
                "Collection name: %s", rc, context->mb()->_collectionName ) ;

      /*
       * If the final record number we fetch is less then the threshold, skip
       * it this time, leave it in the queue and wait for the next round to
       * build the dictionary.
       */
      if ( !dictFull && (fetchNum < RTN_DICT_CREATE_MIN_REC_NUM
           || fetchSize < RTN_DICT_CREATE_MIN_DATA_SIZE ) )
      {
         PD_LOG( PDINFO, "Fetched records not enough when creating dictionary. "
                 "Wait for next round to build" ) ;
         rc = RTN_DICT_CREATE_COND_NOT_MATCH ;
         goto error ;
      }

      _srcDataLen = fetchSize ;

   done:
      PD_TRACE_EXITRC( SDB__RTN_DICTCREATORJOB__CREATEDICT, rc ) ;
      return rc ;
   error:
      _creator.reset() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTN_DICTCREATORJOB__TRANSFERDICT, "_rtnDictCreatorJob::_transferDict" )
   INT32 _rtnDictCreatorJob::_transferDict( dmsStorageData *sd,
                                            dmsMBContext *context )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTN_DICTCREATORJOB__TRANSFERDICT ) ;
      CHAR *dictBuf = NULL ;
      UINT32 dictBufLen = DMS_DICT_MAX_SIZE ;

      _dictionary = _creator.getDictionary() ;

      dictBuf = (CHAR*)SDB_OSS_MALLOC( dictBufLen ) ;
      PD_CHECK( dictBuf, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for dictionary, rc: %d", rc ) ;

      rc = _dictionary->finalize( _srcDataBuf, _srcDataLen,
                                  dictBuf, dictBufLen ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to dump dictionary into stream format, rc: %d", rc ) ;

      if ( context->isMBLock() )
      {
         context->mbUnlock() ;
      }

      rc = sd->dictPersist( context->mbID(), context->clLID(),
                            dictBuf, dictBufLen ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to store dictionary in colleciton, rc: %d", rc ) ;
   done:
      if ( dictBuf )
      {
         SDB_OSS_FREE( dictBuf ) ;
      }
      PD_TRACE_EXITRC( SDB__RTN_DICTCREATORJOB__TRANSFERDICT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTN_DICTCREATORJOB__CHECKANDCREATEDICTFORCL, "_rtnDictCreatorJob::_checkAndCreateDictForCL" )
   INT32 _rtnDictCreatorJob::_checkAndCreateDictForCL( dmsStorageUnitID suID,
                                                       UINT16 mbID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTN_DICTCREATORJOB__CHECKANDCREATEDICTFORCL ) ;
      dmsStorageUnit *su = NULL ;
      dmsMBContext *mbContext = NULL ;
      UINT32 clLID = DMS_INVALID_CLID ;
      pmdKRCB *krCB = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB = krCB->getDMSCB() ;
      ossTimestamp begin ;
      ossTimestamp end ;

      /*
       * If the su is not there, the original storage unit(cs) was dropped. So
       * just remove the item from the list.
       */
      su = dmsCB->suLock( suID ) ;
      if ( NULL == su )
      {
         goto done ;
      }

      rc = su->data()->getMBContext( &mbContext, mbID, clLID ) ;
      if ( (UINT32)DMS_INVALID_CLID == mbContext->clLID() )
      {
         /*
          * The corresponding collection has been dropped. So return success
          * here and the item will be removed from the list.
          */
         goto done ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get mb context, rc: %d, mb ID: %d",
                   rc, mbID ) ;

      if ( DMS_INVALID_EXTENT !=  mbContext->mb()->_dictExtentID )
      {
         /*
          * The dictionary has been created already. This scenario may happen in
          * the following case:
          * (1) collection with 'CompresstionType' configured was created, so it
          *     will be added to the dictionary waiting list. Let's suppose its
          *     mb ID is 0.
          * (2) After that the original collection was dropped, but the item in
          *     the dictionary waiting list will NOT be removed, and it will be
          *     scanned by the dictionary creator.
          *     thread.
          * (3) Before the next scanning, another collection is created, and
          *     it reuse the mb ID of the original collection, its
          *     'CompresstionType' option is configured, and enough data has
          *     been inserted. Now the scanner comes, and it found the item in
          *     the list with the this mb ID. So the creating of the dictionary
          *     starts. But when the latter collection was created, a new item
          *     will be pushed into the list by itself. So when this new added
          *     item was saw by the scanner, the collection's dictionary has
          *     already been created.
          *     In this case, just return success, and the new added item will
          *     be removed.
          */
         goto done ;
      }

      /* Currently we only support LZW. */
      if ( UTIL_COMPRESSOR_LZW != mbContext->mb()->_compressorType )
      {
         /*
          * The mbID has been reused, and the current collection's
          * CompressionType is not configured.
          */
         goto done ;
      }

      if ( !_conditionMatch( su, mbID ) )
      {
         rc = RTN_DICT_CREATE_COND_NOT_MATCH ;
         goto error ;
      }

      ossGetCurrentTime( begin ) ;
      /* Now, create the dictionary for the collection. */
      _creator.reset() ;
      rc = _creator.prepare() ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to prepare dictionary creator, rc: %d", rc ) ;

      rc = _createDict( su->data(), mbContext ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create dictionary, rc: %d", rc ) ;

      rc = _transferDict( su->data(), mbContext ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to pass dictionary to dms, rc: %d", rc ) ;
      ossGetCurrentTime( end ) ;

      PD_LOG( PDEVENT, "Compression dictionary created succesfully for "
              "collection[%s]. Time: %llums",
              mbContext->mb()->_collectionName,
              end.time * 1000 + end.microtm / 1000 -
              (begin.time * 1000 + begin.microtm / 1000) ) ;

   done:
      if ( mbContext )
      {
         su->data()->releaseMBContext( mbContext ) ;
      }

      if ( su )
      {
         dmsCB->suUnlock( suID ) ;
      }
      _creator.reset() ;

      PD_TRACE_EXITRC( SDB__RTN_DICTCREATORJOB__CHECKANDCREATEDICTFORCL, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

