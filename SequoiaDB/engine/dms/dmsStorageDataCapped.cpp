#include "dmsStorageDataCapped.hpp"
#include "dpsOp2Record.hpp"
#include "pmd.hpp"
#include "dmsTrace.hpp"

using namespace bson ;

namespace engine
{
   _dmsStorageDataCapped::_dmsStorageDataCapped( const CHAR* pSuFileName,
                                                 dmsStorageInfo *pInfo,
                                                 _IDmsEventHolder *pEventHolder )
   : _dmsStorageDataCommon( pSuFileName, pInfo, pEventHolder )
   {
      ossMemset( (CHAR *)_options, 0, sizeof(_options) ) ;
      _disableBlockScan() ;
   }

   _dmsStorageDataCapped::~_dmsStorageDataCapped()
   {
   }

   const CHAR* _dmsStorageDataCapped::_getEyeCatcher() const
   {
      return DMS_DATACAPSU_EYECATCHER ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONOPENED, "_dmsStorageDataCapped::_onOpened" )
   INT32 _dmsStorageDataCapped::_onOpened()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONOPENED ) ;
      // Traverse all the collections which are being used, get the collection
      // extend option pointer and the the working extents.
      rc = dmsStorageDataCommon::_onOpened() ;
      if ( rc )
      {
         goto error ;
      }

      for ( UINT16 i = 0 ; i < DMS_MME_SLOTS; ++i )
      {
         if ( DMS_IS_MB_INUSE ( _dmsMME->_mbList[i]._flag ) &&
              ( DMS_INVALID_EXTENT != _dmsMME->_mbList[i]._lastExtentID ) &&
              ( DMS_INVALID_EXTENT != _dmsMME->_mbList[i]._mbOptExtentID ) )
         {
            dmsExtRW extRW = extent2RW( _dmsMME->_mbList[i]._mbOptExtentID,
                                        i ) ;
            extRW.setNothrow( TRUE ) ;
            const dmsOptExtent *extent =
               extRW.readPtr<dmsOptExtent>( 0, pageSize()) ;
            if ( !extent )
            {
               PD_LOG( PDERROR, "Option extent is invalid" ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            _options[i] =
               (dmsCappedCLOptions *)(CHAR *)extent + DMS_OPTEXTENT_HEADER_SZ ;

            // Attach the last extent as the working extent.
            rc = _attachWorkExt( i, _dmsMME->_mbList[i]._lastExtentID ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to switch work extent: %d, "
                         "rc: %d", rc ) ;
         }
      }
   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__ONOPENED, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONCLOSED, "_dmsStorageDataCapped::_onClosed" )
   void _dmsStorageDataCapped::_onClosed()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONCLOSED ) ;
      dmsExtentInfo *extInfo = NULL ;
      // Flush all working extents information to mmap.
      for ( UINT16 i = 0; i < DMS_MME_SLOTS; ++i )
      {
         if ( DMS_IS_MB_INUSE( _dmsMME->_mbList[i]._flag ) )
         {
            extInfo = getWorkExtInfo( i ) ;
            if ( DMS_INVALID_EXTENT != extInfo->getID() )
            {
               rc = _syncWorkExtInfo( i ) ;
               if ( rc )
               {
                  PD_LOG( PDWARNING, "Failed to sync working extent "
                          "information for collection[%s], rc: %d",
                          _dmsMME->_mbList[i]._collectionName, rc ) ;
               }
            }
         }
      }

      dmsStorageDataCommon::_onClosed() ;
      PD_TRACE_EXIT( SDB__DMSSTORAGEDATACAPPED__ONCLOSED ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__PREPAREADDCOLLECTION, "_dmsStorageDataCapped::_prepareAddCollection" )
   INT32 _dmsStorageDataCapped::_prepareAddCollection( const BSONObj *extOption,
                                                       dmsExtentID &extOptExtent,
                                                       UINT16 &extentPageNum )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__PREPAREADDCOLLECTION ) ;
      UINT16 pageNum = 1 ;
      dmsCappedCLOptions options ;
      dmsExtentID extentID = DMS_INVALID_EXTENT ;

      rc = _parseExtendOptions( extOption, options ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to parse options, rc: %d", rc ) ;

      // Allocate one page to store possible options of the collection.
      rc = _findFreeSpace( pageNum, extentID, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Allocate metablock expand option extent "
                   "failed, pageNum: %d, rc: %d", pageNum, rc ) ;
      extOptExtent = extentID ;
      extentPageNum = pageNum ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__PREPAREADDCOLLECTION, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONADDCOLLECTION, "_dmsStorageDataCapped::_onAddCollection" )
   INT32 _dmsStorageDataCapped::_onAddCollection( const BSONObj *extOption,
                                                  dmsExtentID extOptExtent,
                                                  UINT32 extentSize,
                                                  UINT16 collectionID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONADDCOLLECTION ) ;
      dmsExtRW optExtRW ;
      dmsCappedCLOptions options ;
      dmsOptExtent *optExtent = NULL ;

      rc = _parseExtendOptions( extOption, options ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to parse options, rc: %d", rc ) ;

      // Init the option page.
      optExtRW = extent2RW( extOptExtent, collectionID ) ;
      optExtRW.setNothrow( TRUE ) ;
      optExtent = optExtRW.writePtr<dmsOptExtent>() ;
      PD_CHECK( optExtent, SDB_SYS, error, PDERROR,
                "Invalid option extent[%d]", optExtent ) ;
      optExtent->init( extentSize, collectionID ) ;
      optExtent->setOption( (const CHAR *)&options,
                            sizeof( dmsCappedCLOptions )) ;
      rc = optExtent->getOption( (CHAR **)&_options[collectionID], NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Get collection extend option from extent "
                   "failed, rc: %d", rc ) ;
      SDB_ASSERT( _options[collectionID],
                  "Option pointer should not be NULL" ) ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__ONADDCOLLECTION, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONALLOCEXTENT, "_dmsStorageDataCapped::_onAllocExtent" )
   void _dmsStorageDataCapped::_onAllocExtent( dmsMBContext *context,
                                               dmsExtent *extAddr,
                                               SINT32 extentID )
   {
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONALLOCEXTENT ) ;

      SDB_ASSERT( context, "Context should not be NULL" ) ;
      SDB_ASSERT( extAddr, "Extent address should not be NULL" ) ;

      ossMemset( (CHAR *)extAddr + sizeof( dmsExtent ), 0,
                 ( (UINT32)extAddr->_blockSize << pageSizeSquareRoot() )
                 - sizeof( dmsExtent ) ) ;

      _mbStatInfo[context->mbID()]._totalDataFreeSpace +=
            ( (extAddr->_blockSize) << pageSizeSquareRoot() ) -
            DMS_EXTENT_METADATA_SZ ;

      PD_TRACE_EXIT( SDB__DMSSTORAGEDATACAPPED__ONALLOCEXTENT ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__PREPAREINSERTDATA, "_dmsStorageDataCapped::_prepareInsertData" )
   INT32 _dmsStorageDataCapped::_prepareInsertData( const BSONObj &record,
                                                    BOOLEAN mustOID,
                                                    pmdEDUCB *cb,
                                                    dmsRecordData &recordData,
                                                    BOOLEAN &memReallocate )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__PREPAREINSERTDATA ) ;
      LogicalIDToInsert logicalID ;
      LogicalIDToInsertEle logicalIDEle( (CHAR *)(&logicalID) ) ;
      CHAR *mergedData = NULL ;

      if ( !mustOID )
      {
         PD_LOG( PDERROR, "_id is always required by capped "
                 "collection record" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      try
      {
         // _id can not be set explicitly. It will be generated inside.
         // So if there is one in the original record, it will be ignored.
         // We don't return error here because the driver always add the _id
         // field for all records...
         INT32 ignoreSize = 0 ;
         INT32 totalSize = logicalIDEle.size() + record.objsize() ;
         BSONElement ele = record.getField( DMS_ID_KEY_NAME ) ;
         if ( EOO != ele.type() )
         {
            ignoreSize = ele.size() ;
            totalSize -= ignoreSize ;
         }

         // The logical is unknow for now, just reserve the space for it.
         // Its value will be filled in _extentInsertRecord.
         rc = cb->allocBuff( totalSize, &mergedData ) ;
         PD_RC_CHECK( rc, PDERROR, "Allocate memory[size: %u] failed, rc: %d",
                      totalSize, rc ) ;

         *(UINT32*)mergedData = totalSize ;
         ossMemcpy( mergedData + sizeof(UINT32), logicalIDEle.rawdata(),
                    logicalIDEle.size() ) ;
         ossMemcpy( mergedData + sizeof(UINT32) + logicalIDEle.size(),
                    record.objdata() + sizeof(UINT32) + ignoreSize,
                    record.objsize() - sizeof(UINT32) - ignoreSize ) ;
         recordData.setData( mergedData, totalSize, FALSE, TRUE ) ;
         memReallocate = TRUE ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__PREPAREINSERTDATA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__FINALRECORDSIZE, "_dmsStorageDataCapped::_finalRecordSize" )
   void _dmsStorageDataCapped::_finalRecordSize( UINT32 &size,
                                                 const dmsRecordData &recordData )
   {
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__FINALRECORDSIZE ) ;
      // Append the logic id size. Only do that when the data is not compressed.
      // In case of data compressed, logical id is only stored in the record
      // header, and will be copied out when fetch the data.
      if ( !recordData.isCompressed() )
      {
         size += sizeof( LogicalIDToInsert ) ;
      }

      size += DMS_RECORD_METADATA_SZ ;
      // record is ALWAYS 4 bytes aligned
      size = OSS_MIN( DMS_RECORD_MAX_SZ, ossAlignX ( size, 4 ) ) ;
      PD_TRACE2 ( SDB__DMSSTORAGEDATACAPPED__FINALRECORDSIZE,
                  PD_PACK_STRING ( "size after align" ),
                  PD_PACK_UINT ( size ) ) ;
      PD_TRACE_EXIT( SDB__DMSSTORAGEDATACAPPED__FINALRECORDSIZE ) ;
   }

   // The record space allocation strategy is as follows:
   // 1. Before reaching the limit( both size and number ), continue to write
   //    forward. If the freespace in the current extent is not enough,
   //    allocate a new one. No recycle needs to be done.
   // 2. If we reach the limit( either size or number ), recycle the eldest
   //    extent.
   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ALLOCRECORDSPACE, "_dmsStorageDataCapped::_allocRecordSpace" )
   INT32 _dmsStorageDataCapped::_allocRecordSpace( dmsMBContext *context,
                                                   UINT32 size,
                                                   dmsRecordID &foundRID,
                                                   pmdEDUCB *cb  )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ALLOCRECORDSPACE ) ;
      dmsExtentID extID = DMS_INVALID_EXTENT ;
      dmsExtentInfo *workExtInfo = NULL ;
      BOOLEAN newExtAlloc = FALSE ;
      BOOLEAN extSwitched = FALSE ;

      SDB_ASSERT( context, "Context should not be NULL" ) ;
      SDB_ASSERT( cb, "edu cb should not be NULL" ) ;

      workExtInfo = getWorkExtInfo( context->mbID() ) ;
      if ( !context->isMBLock( EXCLUSIVE ) )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Caller must hold exclusive lock[%s]",
                 context->toString().c_str() ) ;
         goto error ;
      }

      // If the working extent is invalid, no data extent has been allocated for
      // the collection yet.
      if ( DMS_INVALID_EXTENT == workExtInfo->getID() )
      {
         SDB_ASSERT( DMS_INVALID_EXTENT == context->mb()->_firstExtentID
                     && DMS_INVALID_EXTENT == context->mb()->_lastExtentID,
                     "The first and last extents should be invalid" ) ;
         rc = _allocateExtent( context, DMS_CAP_EXTENT_PAGE_NUM,
                               TRUE, FALSE, &extID ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to allocate extent, rc: %d", rc ) ;
         newExtAlloc = TRUE ;

         rc = _attachWorkExt( context->mbID(), extID ) ;
         PD_RC_CHECK( rc, PDERROR, "Attach new working extent failed: %d",
                      rc ) ;
         extSwitched = TRUE ;
      }
      else
      {
         // Check if exceeding the Size and Max limitations.
         // If yes, return error or recycle extent, depending on the option.
         if ( _exceedLimit( context, size ) )
         {
            rc = _recycleOneExtent( context ) ;
            PD_RC_CHECK( rc, PDERROR,
                         "Recycle the eldest extent failed[ %d ]", rc ) ;
         }

         // If free space in current working extent is not enough, get another.
         if ( workExtInfo->_freeSpace < (INT32)size )
         {
            rc = _allocateExtent( context, DMS_CAP_EXTENT_PAGE_NUM, TRUE,
                                  FALSE, &extID ) ;
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to allocate extent, rc: %d", rc ) ;
            newExtAlloc = TRUE ;

            rc = _switchWorkExt( context, extID ) ;
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to switch working extent, rc: %d", rc ) ;
            extSwitched = TRUE ;
         }
      }

      foundRID._extent = workExtInfo->getID() ;
      foundRID._offset = workExtInfo->getNextRecOffset() ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__ALLOCRECORDSPACE, rc ) ;
      return rc ;
   error:
      if ( newExtAlloc && !extSwitched )
      {
         INT32 rcTmp = _freeExtent( extID, context->mbID() ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Failed to free extent, rc: %d", rcTmp ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED_EXTRACTDATA, "_dmsStorageDataCapped::extractData" )
   INT32 _dmsStorageDataCapped::extractData( dmsMBContext *mbContext,
                                             dmsRecordRW &recordRW,
                                             pmdEDUCB *cb,
                                             dmsRecordData &recordData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED_EXTRACTDATA ) ;
      monAppCB *pMonAppCB = cb ? cb->getMonAppCB() : NULL ;
      const dmsRecord *pRecord= recordRW.readPtr( 0 ) ;

      recordData.reset() ;

      if ( !mbContext->isMBLock() )
      {
         PD_LOG( PDERROR, "MB Context must be locked" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      recordData.setData( pRecord->getData(), pRecord->getDataLength(),
                          FALSE, TRUE ) ;

      if ( pRecord->isCompressed() )
      {
// Currently compression is not supported for capped collection.
#if 0
         /*
          * It's a little complicated in compression case. The original record
          * without logical ID is compressed, and the logical id is stored in
          * the header of the record. We need to combine them as a single
          * complete BSON object here. So we need to allocate the space before
          * uncompression the data, and reserve the space for logical id. After
          * the uncompression, reformat them.
          *
          *  <--reserve-->|<--uncompressed object--->
          *  ----------------------------------------
          * || lid space || len |       items       ||
          *  ----------------------------------------
          * After format, it should be like:
          *  ----------------------------------------
          * || len || lid object + original objects ||
          * -----------------------------------------
          * Logical id is from the record header.
          * Then it can be translated as a BSON object directly.
          */
         CHAR *buffer = NULL ;
         logicIdToInsert logicID ;
         logicIdToInsertEle logicIDEle( (CHAR *)&logicID ) ;
         const CHAR *uncompressData = NULL ;
         INT32 unCompressDataLen = 0 ;
         utilCompressor *compressor =
            _compressorEntry[ mbContext->mbID() ].getCompressor() ;
         UINT32 totalLen = 0 ;
         rc = compressor->getUncompressedLen( pRecord->getData(),
                                              pRecord->getDataLength(),
                                              totalLen ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get record uncompress length, "
                      "rc: %d", rc ) ;

         totalLen += sizeof( logicIdToInsert ) ;
         // The buffer will be released when reallocation or when the EDU is
         // destroyed.
         buffer = cb->getUncompressBuff( totalLen ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get uncompress buffer, "
                      "requested size: %u, rc: %d", totalLen, rc ) ;

         uncompressData = buffer + sizeof( logicIdToInsert ) ;
         unCompressDataLen = totalLen - sizeof( logicIdToInsert ) ;

         rc = dmsUncompress( cb, &_compressorEntry[ mbContext->mbID() ],
                             pRecord->getData(), pRecord->getDataLength(),
                             &uncompressData, &unCompressDataLen ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to uncompress data, rc: %d", rc ) ;
            goto error ;
         }
         /// check the length
         if ( unCompressDataLen != *(INT32*)uncompressData )
         {
            PD_LOG( PDERROR, "Uncompress data length[%d] does not match "
                    "real length[%d]", unCompressDataLen,
                    *(INT32*)uncompressData ) ;
            rc = SDB_CORRUPTED_RECORD ;
            goto error ;
         }

         // Append the logical id.
         logicID.setID( pRecord->getLogicID() ) ;
         totalLen = sizeof( logicIdToInsert ) + unCompressDataLen ;
         *(UINT32*)buffer = totalLen ;
         ossMemcpy( buffer + sizeof( UINT32 ), logicIDEle.rawdata(),
                    logicIDEle.size() ) ;

         recordData.setData( buffer, totalLen, FALSE, FALSE ) ;
#endif
      }
      DMS_MON_OP_COUNT_INC( pMonAppCB, MON_DATA_READ, 1 ) ;
      DMS_MON_OP_COUNT_INC( pMonAppCB, MON_READ, 1 ) ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED_EXTRACTDATA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__OPERATIONPERMCHK, "_dmsStorageDataCapped::_operationPermChk" )
   INT32 _dmsStorageDataCapped::_operationPermChk( DMS_ACCESS_TYPE accessType )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__OPERATIONPERMCHK ) ;

      switch ( accessType )
      {
         case DMS_ACCESS_TYPE_QUERY:
         case DMS_ACCESS_TYPE_FETCH:
         case DMS_ACCESS_TYPE_INSERT:
         case DMS_ACCESS_TYPE_TRUNCATE:
         case DMS_ACCESS_TYPE_POP:
            break ;
         default:
            PD_LOG( PDERROR, "Operation type[ %d ] is incompatible with "
                    "capped collection", accessType ) ;
            rc = SDB_OPERATION_INCOMPATIBLE ;
            goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__OPERATIONPERMCHK, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__RECYCLEWORKEXT, "_dmsStorageDataCapped::_recycleWorkExt" )
   INT32 _dmsStorageDataCapped::_recycleWorkExt( dmsMBContext *context,
                                                 dmsExtentID extID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__RECYCLEWORKEXT ) ;
      UINT16 mbID = context->mbID() ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;
      INT64 recNum = 0 ;
      INT64 dataSize = 0 ;
      INT64 totalSize = 0 ;
      dmsExtentInfo *extInfo = getWorkExtInfo( context->mbID() ) ;

      extRW = extent2RW( extID, mbID ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsExtent>() ;
      if ( !extent )
      {
         PD_LOG( PDERROR, "Invalid extent: %d", extID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _countRecNumAndSize( context, extID, extInfo->_firstRecordOffset,
                           extent->_lastRecordOffset, 1, recNum, dataSize,
                           totalSize ) ;

      // Initialize the extent, update the statistic information in _mbStatInfo
      // and work extent info.
       extent->init( DMS_CAP_EXTENT_PAGE_NUM, mbID, DMS_CAP_EXTENT_SZ ) ;
      _mbStatInfo[mbID]._totalRecords -= extInfo->_recCount ;
      _mbStatInfo[mbID]._totalDataLen -= dataSize ;
      _mbStatInfo[mbID]._totalOrgDataLen -= dataSize ;
      _mbStatInfo[mbID]._totalDataFreeSpace -= extent->_freeSpace ;

      extInfo->_recCount = 0 ;
      extInfo->_firstRecordOffset = extInfo->_lastRecordOffset ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__RECYCLEWORKEXT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__RECYCLEACTIVEEXT, "_dmsStorageDataCapped::_recycleActiveExt" )
   INT32 _dmsStorageDataCapped::_recycleActiveExt( dmsMBContext *context,
                                                   dmsExtentID extID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__RECYCLEACTIVEEXT ) ;
      UINT16 mbID = context->mbID() ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;
      INT64 recNum = 0 ;
      INT64 dataSize = 0 ;
      INT64 totalSize = 0 ;

      extRW = extent2RW( extID, mbID ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsExtent>() ;
      if ( !extent )
      {
         PD_LOG( PDERROR, "Invalid extent: %d", extID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _countRecNumAndSize( context, extID, extent->_firstRecordOffset,
                           extent->_lastRecordOffset, 1, recNum, dataSize,
                           totalSize ) ;

      _mbStatInfo[mbID]._totalRecords -= extent->_recCount ;
      _mbStatInfo[mbID]._totalDataFreeSpace -= extent->_freeSpace ;
      _mbStatInfo[mbID]._totalOrgDataLen -= totalSize ;
      _mbStatInfo[mbID]._totalDataLen -= totalSize ;

      rc = _freeExtent( context, extID ) ;
      PD_RC_CHECK( rc, PDERROR, "Free extent[%d] failed: %d", extID, rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__RECYCLEACTIVEEXT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__PARSEEXTENTOPTIONS, "_dmsStorageDataCapped::_parseExtendOptions" )
   INT32 _dmsStorageDataCapped::_parseExtendOptions( const BSONObj *extOptions,
                                                     dmsCappedCLOptions &options )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__PARSEEXTENTOPTIONS ) ;

      if ( !extOptions || !extOptions->valid() )
      {
         PD_LOG( PDERROR, "Only capped collection with valid options[Capped/"
                 "Size/Max] can be created on capped collection space" ) ;
         rc = SDB_OPERATION_INCOMPATIBLE ;
         goto error ;
      }

      // Traverse the option object to check.
      {
         BOOLEAN maxSizeFound = FALSE ;
         BOOLEAN maxRecNumFound = FALSE ;
         BSONObjIterator itr( *extOptions ) ;
         while ( itr.more() )
         {
            INT64 maxSize = 0 ;
            INT64 maxRecNum = 0 ;

            BSONElement ele = itr.next() ;
            const CHAR *fieldName = ele.fieldName() ;
            if ( 0 == ossStrcmp( fieldName, FIELD_NAME_SIZE ) )
            {
               if ( !ele.isNumber())
               {
                  PD_LOG( PDERROR, "Invalid type[%d] for option Size",
                          ele.type() ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               maxSize = ele.numberLong() ;

               if ( maxSize < DMS_CAP_CL_MIN_SZ ||
                    maxSize > DMS_MAX_SZ( pageSize() ) )
               {
                  PD_LOG( PDERROR, "Invalid size[%lld] of capped collection",
                          maxSize ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               maxSizeFound = TRUE ;
               options._maxSize = ele.numberLong() ;
            }
            else if ( 0 == ossStrcmp( fieldName, FIELD_NAME_MAX ) )
            {
               if ( !ele.isNumber() )
               {
                  PD_LOG( PDERROR, "Invalid type[%d] for option Max",
                          ele.type() ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               maxRecNum = ele.numberLong() ;

               if ( maxRecNum < 0 )
               {
                  PD_LOG( PDERROR, "Option value[%lld] for Max should not "
                          "be negative", maxRecNum ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               maxRecNumFound = TRUE ;
               options._maxRecNum = ele.numberLong() ;
            }
            else
            {
               PD_LOG( PDERROR, "Unsupported capped collection option: %s",
                       fieldName ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }

         }

         if ( !maxSizeFound || !maxRecNumFound )
         {
            PD_LOG( PDERROR, "Max/Size not specified for capped collection" ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__PARSEEXTENTOPTIONS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__RECYCLEEXTENT, "_dmsStorageDataCapped::_recycleExtent" )
   INT32 _dmsStorageDataCapped::_recycleOneExtent( dmsMBContext *context )
   {
      // Extent will be recycled when the size or record number threshold is hit
      // or when all the records in the extent are popped.
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__RECYCLEEXTENT ) ;
      dmsExtentID extentID = DMS_INVALID_EXTENT ;
      dmsExtentInfo *workExtInfo = getWorkExtInfo( context->mbID() ) ;

      // Always recycle the eldest extent. There are two scenarios:
      // 1. Only one extent in the collection.
      // 2. More than one extents in the collection.
      // The first scenario is a little complicated, as it's the working extent,
      // so some sync or cleaning has to be done.
      extentID = context->mb()->_firstExtentID ;
      if ( extentID == workExtInfo->getID() )
      {
         workExtInfo->reset() ;
         rc = _recycleWorkExt( context, extentID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to recycle working extent, rc: %d", rc ) ;
      }
      else
      {
         rc = _recycleActiveExt( context, extentID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to recycle active extent, rc: %d", rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__RECYCLEEXTENT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__EXTENTINSERTRECORD, "_dmsStorageDataCapped::_extentInsertRecord" )
   INT32 _dmsStorageDataCapped::_extentInsertRecord( dmsMBContext *context,
                                                     dmsExtRW &extRW,
                                                     dmsRecordRW &recordRW,
                                                     const dmsRecordData &recordData,
                                                     UINT32 recordSize,
                                                     pmdEDUCB *cb,
                                                     BOOLEAN isInsert )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__EXTENTINSERTRECORD ) ;
      dmsRecord *pRecord = NULL ;
      monAppCB *pMonAppCB = cb ? cb->getMonAppCB() : NULL ;
      dmsExtentInfo *workExtInfo = getWorkExtInfo( context->mbID() ) ;

      rc = context->mbLock( EXCLUSIVE ) ;
      PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;

      if ( !recordData.isCompressed() &&
           recordData.len() < DMS_MIN_RECORD_DATA_SZ )
      {
         PD_LOG( PDERROR, "Bson obj size[%d] is invalid", recordData.len() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      pRecord = recordRW.writePtr( recordSize ) ;
      // Set the record information. Logical id will be added in setData.
      pRecord->setNormal() ;
      pRecord->resetAttr() ;
      pRecord->setSize( recordSize ) ;
      {
         // Force set the logical id in the record. Logical id is always at the
         // beginning of the record.
         LogicalIDToInsertEle ele( (CHAR *)( recordData.data() +
                                             sizeof(UINT32) ) ) ;
         INT64 *lidPtr = (INT64 *)ele.value() ;
         *lidPtr = workExtInfo->getRecordLogicID() ;
      }

      pRecord->setData( recordData ) ;

      DMS_MON_OP_COUNT_INC( pMonAppCB, MON_DATA_WRITE, 1 ) ;

      _updateStatInfo( context, recordSize, recordData ) ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__EXTENTINSERTRECORD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_extentUpdatedRecord( dmsMBContext *context,
                                                      dmsExtRW &extRW,
                                                      dmsRecordRW &recordRW,
                                                      const dmsRecordData &recordData,
                                                      const BSONObj &newObj,
                                                      pmdEDUCB *cb )
   {
      SDB_ASSERT( FALSE, "Should not be here" ) ;
      return SDB_OPERATION_INCOMPATIBLE ;
   }

   INT32 _dmsStorageDataCapped::_extentRemoveRecord( dmsMBContext *context,
                                                     dmsExtRW &extRW,
                                                     dmsRecordRW &recordRW,
                                                     pmdEDUCB *cb,
                                                     BOOLEAN decCount )
   {
      SDB_ASSERT( FALSE, "Should not be here" ) ;
      return SDB_OPERATION_INCOMPATIBLE ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONINSERTFAIL, "_dmsStorageDataCapped::_onInsertFail" )
   INT32 _dmsStorageDataCapped::_onInsertFail( dmsMBContext *context,
                                               BOOLEAN hasInsert,
                                               dmsRecordID rid,
                                               SDB_DPSCB *dpscb,
                                               ossValuePtr dataPtr,
                                               pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONINSERTFAIL ) ;

      if ( hasInsert )
      {
         // Revert all what has been done in _extentInsertRecord.
         dmsExtRW extRW ;
         dmsExtent *extent = NULL ;
         INT64 logicalID = -1 ;

         extRW = extent2RW( rid._extent, context->mbID() ) ;
         extRW.setNothrow( TRUE ) ;
         extent = extRW.writePtr<dmsExtent>() ;
         if ( !extent )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid extent: %d, rc: %d", rid._extent, rc ) ;
            goto error ;
         }

         logicalID = (INT64)extent->_logicID * DMS_CAP_EXTENT_BODY_SZ +
                     rid._offset - sizeof( dmsExtent ) ;

         rc = popRecord( context, logicalID, cb, dpscb, 1 ) ;
         PD_RC_CHECK( rc, PDERROR, "Remove the inserted record failed[ %d ]",
                      rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__ONINSERTFAIL, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__SYNCWORKEXTINFO, "_dmsStorageDataCapped::_syncWorkExtInfo" )
   INT32 _dmsStorageDataCapped::_syncWorkExtInfo( UINT16 collectionID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__SYNCWORKEXTINFO ) ;
      dmsExtentID extID = DMS_INVALID_EXTENT ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;
      dmsExtentInfo *extInfo = getWorkExtInfo( collectionID ) ;

      extID = extInfo->getID() ;
      if ( DMS_INVALID_EXTENT == extID )
      {
         goto done ;
      }

      extRW = extent2RW( extID, collectionID ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsExtent>() ;
      if ( !extent )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid extent: %d, rc: %d", extID, rc ) ;
         goto error ;
      }

      extent->_recCount = extInfo->_recCount ;
      extent->_freeSpace = extInfo->_freeSpace ;
      extent->_firstRecordOffset = extInfo->_firstRecordOffset ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__SYNCWORKEXTINFO, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__SWITCHWORKEXT, "_dmsStorageDataCapped::_switchWorkExt" )
   INT32 _dmsStorageDataCapped::_switchWorkExt( dmsMBContext *context,
                                                dmsExtentID extID )
   {
      // Detach the current extent, if there is one, flush the meta data.
      // Then attach to the specified new extent.
      // In case of any error, need to restore the original state.
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__SWITCHWORKEXT ) ;
      BOOLEAN detached = FALSE ;
      dmsExtentID currWorkExt = DMS_INVALID_EXTENT ;
      dmsExtentInfo *workExtInfo = getWorkExtInfo( context->mbID() ) ;
      SDB_ASSERT( workExtInfo, "Work extent info pointer should not be NULL" ) ;

      currWorkExt = workExtInfo->getID() ;

      if ( DMS_INVALID_EXTENT == extID )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Extent to swith to is invalid, rc: %d", rc ) ;
         goto error ;
      }

      if ( !(context->isMBLock( EXCLUSIVE ) ) )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Caller must hold exclusive lock on mb: %d, rc: %d",
                 context->mbID(), rc ) ;
         goto error ;
      }

      // If currently attached to a valid extent, need to detach first.
      if ( DMS_INVALID_EXTENT != currWorkExt )
      {
         rc = _detachWorkExt( context->mbID(), TRUE ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to detach working extent, rc: %d", rc ) ;
         detached = TRUE ;
      }

      rc = _attachWorkExt( context->mbID(), extID ) ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to attach to new working extent, rc: %d", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__SWITCHWORKEXT, rc ) ;
      return rc ;
   error:
      // In case of error, may need to switch back to the original extent.
      if ( detached )
      {
         INT32 rcTmp = _attachWorkExt( context->mbID(), currWorkExt ) ;
         SDB_ASSERT( SDB_OK == rcTmp, "Switch to original extent failed" ) ;
         if ( rcTmp )
         {
            PD_LOG( PDERROR, "Failed to switch to original extent failed, "
                    "rc: %d", rcTmp ) ;
         }
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ATTACHWORKEXT, "_dmsStorageDataCapped::_attachWorkExt" )
   INT32 _dmsStorageDataCapped::_attachWorkExt( UINT16 collectionID,
                                                dmsExtentID extID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ATTACHWORKEXT ) ;
      dmsExtRW extRW ;
      const dmsExtent *extent = NULL ;
      dmsExtentInfo *workExtInfo = getWorkExtInfo( collectionID ) ;

      SDB_ASSERT( workExtInfo, "Work extent info pointer should not be NULL" ) ;

      if ( DMS_INVALID_EXTENT == extID )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Invalid extent to attach, rc: %d", rc ) ;
         goto error ;
      }

      extRW = extent2RW( extID, collectionID ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.readPtr<dmsExtent>() ;
      if ( !extent )
      {
         PD_LOG( PDERROR, "Invalid extent: %d", extID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      workExtInfo->_id = extID ;
      workExtInfo->_recCount = extent->_recCount ;
      workExtInfo->_freeSpace = extent->_freeSpace ;
      workExtInfo->_firstRecordOffset = extent->_firstRecordOffset ;
      workExtInfo->_lastRecordOffset = extent->_lastRecordOffset ;
      workExtInfo->_extLogicID = extent->_logicID ;
      // Calculate the write position in this extent.
      if ( DMS_INVALID_OFFSET != extent->_lastRecordOffset )
      {
         dmsRecordID recordID( extID, extent->_lastRecordOffset ) ;
         dmsRecordRW recordRW = record2RW( recordID, collectionID ) ;
         recordRW.setNothrow( TRUE ) ;
         const dmsRecord *record = recordRW.readPtr<dmsRecord>() ;
         if ( !record )
         {
            PD_LOG( PDERROR, "Last record in extent is invalid" ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         workExtInfo->_writePos = workExtInfo->_lastRecordOffset +
                                  record->getSize() - DMS_EXTENT_METADATA_SZ ;
      }
      else
      {
         workExtInfo->_writePos = 0 ;
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__ATTACHWORKEXT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__DETACHWORKEXT, "_dmsStorageDataCapped::_detachWorkExt" )
   INT32 _dmsStorageDataCapped::_detachWorkExt( UINT16 collectionID,
                                                BOOLEAN sync )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__DETACHWORKEXT ) ;
      dmsExtentID workExtID = DMS_INVALID_EXTENT ;
      dmsExtentInfo *workExtInfo = getWorkExtInfo( collectionID ) ;

      SDB_ASSERT( workExtInfo, "Work extent info pointer should not be NULL" ) ;
      workExtID = workExtInfo->getID() ;

      if ( DMS_INVALID_EXTENT == workExtID )
      {
         goto done ;
      }

      if ( sync )
      {
         _syncWorkExtInfo( collectionID ) ;
      }

      workExtInfo->reset() ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__DETACHWORKEXT, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__POPFROMWORKEXT, "_dmsStorageDataCapped::_popFromWorkExt" )
   INT32 _dmsStorageDataCapped::_popFromWorkExt( dmsMBContext *context,
                                                 dmsExtentID extentID,
                                                 dmsOffset offset,
                                                 INT8 direction )
   {
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__POPFROMWORKEXT ) ;
      INT64 recNum = 0 ;
      INT64 dataSize = 0 ;
      INT64 totalSize = 0 ;
      dmsExtentInfo *workExtInfo = &_workExtInfo[context->mbID()] ;

      if ( direction >= 0 )
      {
         _countRecNumAndSize( context, extentID,
                              workExtInfo->_firstRecordOffset, offset,
                              direction, recNum, dataSize, totalSize ) ;
         workExtInfo->_firstRecordOffset += totalSize ;
      }
      else
      {
         _countRecNumAndSize( context, extentID, offset,
                              workExtInfo->_lastRecordOffset,
                              direction, recNum, dataSize, totalSize ) ;
         workExtInfo->_lastRecordOffset -= totalSize ;
         // Do not increase the free space when direction is 1, as that
         workExtInfo->_freeSpace += totalSize ;
         _mbStatInfo[ context->mbID() ]._totalDataFreeSpace += totalSize ;
      }

      workExtInfo->_recCount -= recNum ;

      _mbStatInfo[ context->mbID() ]._totalRecords -= recNum ;
      _mbStatInfo[ context->mbID() ]._totalOrgDataLen -= dataSize ;
      _mbStatInfo[ context->mbID() ]._totalDataLen -= dataSize ;

      PD_TRACE_EXIT( SDB__DMSSTORAGEDATACAPPED__POPFROMWORKEXT ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__POPFROMACTIVEEXTENT, "_dmsStorageDataCapped::_popFromActiveExt" )
   INT32 _dmsStorageDataCapped::_popFromActiveExt( dmsMBContext *context,
                                                   dmsExtentID extentID,
                                                   dmsOffset offset,
                                                   INT8 direction )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__POPFROMACTIVEEXTENT ) ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;
      INT64 recNum = 0 ;
      INT64 dataSize = 0 ;
      INT64 totalSize = 0 ;

      extRW = extent2RW( extentID, context->mbID() ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsExtent>() ;
      if ( !extent )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid extent, rc: %d", rc ) ;
         goto error ;
      }

      if ( direction >= 0 )
      {
         _countRecNumAndSize( context, extentID, extent->_firstRecordOffset,
                              offset, direction, recNum, dataSize, totalSize ) ;
         extent->_firstRecordOffset += totalSize ;
      }
      else
      {
         _countRecNumAndSize( context, extentID, offset, extent->_lastRecordOffset,
                              direction, recNum, dataSize, totalSize ) ;
         extent->_lastRecordOffset -= totalSize ;
      }

      extent->_recCount -= recNum ;
      extent->_freeSpace += totalSize ;

      _mbStatInfo[ context->mbID() ]._totalRecords -= recNum ;
      _mbStatInfo[ context->mbID() ]._totalOrgDataLen -= dataSize ;
      _mbStatInfo[ context->mbID() ]._totalDataLen -= dataSize ;
      _mbStatInfo[ context->mbID() ]._totalDataFreeSpace += totalSize ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__POPFROMACTIVEEXTENT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__POPRECORD, "_dmsStorageDataCapped::_popRecord" )
   INT32 _dmsStorageDataCapped::_popRecord( dmsMBContext *context,
                                            dmsExtentID extID,
                                            dmsOffset offset,
                                            INT8 direction )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__POPRECORD ) ;
      dmsExtRW extRW ;
      dmsExtentInfo *workExtInfo = getWorkExtInfo( context->mbID() ) ;
      // Check the logical ID of all the extents.

      SDB_ASSERT( workExtInfo, "Work extent info pointer should not be NULL" ) ;

      if ( !( context->isMBLock( EXCLUSIVE ) ) )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Caller should hold the exclusive lock, rc: %d",
                 rc ) ;
         goto error ;
      }

      if ( extID == workExtInfo->getID() )
      {
         rc = _popFromWorkExt( context, extID, offset, direction ) ;
      }
      else
      {
         rc = _popFromActiveExt( context, extID, offset, direction ) ;
      }
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to pop records from extent, extent id: %d, "
                   "offset: %d, direction: %c, rc: %d",
                   extID, offset, direction, rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__POPRECORD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__EXTRACTRECLID, "_dmsStorageDataCapped::_extractRecLID" )
   INT32 _dmsStorageDataCapped::_extractRecLID( dmsMBContext *context,
                                                INT64 logicalID,
                                                pmdEDUCB *cb,
                                                dmsExtentID &extentID,
                                                dmsExtentID &extLID,
                                                dmsOffset &offset )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__EXTRACTRECLID ) ;
      dmsRecordID recordID ;
      dmsRecordRW recordRW ;
      const dmsRecord *record = NULL ;
      INT64 lidInRecord = 0 ;

      if ( logicalID < 0 )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Invalid logical id[%lld], rc: %d", logicalID, rc ) ;
         goto error ;
      }

      extentID = _logicID2ExtID( context, logicalID ) ;
      if ( DMS_INVALID_EXTENT == extentID )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Logical ID[%lld] is not in valid range",
                 logicalID ) ;
         goto error ;
      }

      _getExtLIDAndOffsetByLID( logicalID, extLID, offset ) ;

      {
         // Check if we can access the record normally and the logical id is
         // the same.
         const dmsRecordID recordID( extentID, offset ) ;
         recordRW = record2RW( recordID, context->mbID() ) ;
         recordRW.setNothrow( TRUE ) ;
         record = recordRW.readPtr<dmsRecord>() ;
         if ( !record || 0 == record->_head._flag_and_size )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "Invalid logical id[%lld], rc: %d",
                    logicalID, rc ) ;
            goto error ;
         }
         else
         {
            dmsRecordData recordData ;
            rc = extractData( context, recordRW, cb, recordData ) ;
            PD_RC_CHECK( rc, PDERROR, "Extract data failed: %d", rc ) ;
            try
            {
               BSONObj recordObj( recordData.data() ) ;
               BSONElement lidEle = recordObj.getField( DMS_ID_KEY_NAME ) ;
               if ( NumberLong != lidEle.type() )
               {
                  PD_LOG( PDERROR, "Logical id elment is invalid in record" ) ;
                  rc = SDB_SYS ;
                  goto error ;
               }
               lidInRecord = lidEle.numberLong() ;
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
               rc = SDB_SYS ;
               goto error ;
            }

            if ( !( DMS_RECORD_FLAG_NORMAL == record->getState() &&
                    logicalID == lidInRecord ) )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "Invalid logical id[%lld], rc: %d",
                       logicalID, rc ) ;
               goto error ;
            }
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__EXTRACTRECLID, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED_RECYCLEEXTENTS, "_dmsStorageDataCapped::_recycleExtents" )
   INT32 _dmsStorageDataCapped::_recycleExtents( dmsMBContext *context,
                                                 dmsExtentID targetExtID,
                                                 INT8 direction )
   {
      // Recycle all the extents which will be freed by pop.
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED_RECYCLEEXTENTS ) ;
      dmsExtentID extentID = DMS_INVALID_EXTENT ;
      dmsExtentID endExtID = DMS_INVALID_EXTENT ;
      dmsExtRW extRW ;
      const dmsExtent *extent = NULL ;

      extRW = extent2RW( targetExtID, context->mbID() ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.readPtr<dmsExtent>() ;
      if ( !extent )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid extent: %d, rc: %d", targetExtID, rc ) ;
         goto error ;
      }

      if ( direction >= 0 )
      {
         extentID = context->mb()->_firstExtentID ;
         endExtID = extent->_prevExtent ;
      }
      else
      {
         // For backward pop, need to switch the working extent to the target.
         // If the target extent is just the working extent, no switch is needed
         endExtID = getWorkExtInfo( context->mbID() )->getID() ;
         extentID = extent->_nextExtent ;
         if ( getWorkExtInfo( context->mbID() )->getID() != targetExtID )
         {
            rc = _switchWorkExt( context, targetExtID ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to switch working extent, rc: %d",
                         rc ) ;
         }
      }

      while ( DMS_INVALID_EXTENT != extentID && extentID != targetExtID )
      {
         extRW = extent2RW( extentID, context->mbID() ) ;
         extRW.setNothrow( TRUE ) ;
         extent = extRW.readPtr<dmsExtent>() ;
         if ( !extent )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid extent[%d], rc: %d", extentID, rc ) ;
            goto error ;
         }

         rc = _recycleActiveExt( context, extentID ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to recycle active extent, rc: %d", rc ) ;
         if ( extentID == endExtID )
         {
            break ;
         }

         extentID = extent->_nextExtent;
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED_RECYCLEEXTENTS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED_COUNTRECNUMANDSIZE, "_dmsStorageDataCapped::_countRecNumAndSize" )
   void _dmsStorageDataCapped::_countRecNumAndSize( dmsMBContext *context,
                                                    dmsExtentID extentID,
                                                    dmsOffset beginOffset,
                                                    dmsOffset endOffset,
                                                    INT32 direction,
                                                    INT64 &recNum,
                                                    INT64 &dataSize,
                                                    INT64 &totalSize )
   {
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED_COUNTRECNUMANDSIZE ) ;
      dmsRecordRW recordRW ;
      dmsRecord *record = NULL ;

      recNum = 0 ;
      dataSize = 0 ;
      totalSize = 0 ;

      if ( DMS_INVALID_OFFSET == endOffset )
      {
         endOffset = DMS_CAP_EXTENT_SZ ;
      }

      while ( beginOffset <= endOffset )
      {
         dmsRecordID rid( extentID, beginOffset ) ;
         recordRW = record2RW( rid, context->mbID() ) ;
         recordRW.setNothrow( TRUE ) ;
         record = recordRW.writePtr<dmsRecord>() ;
         // When meet an invalid record, break;
         if ( 0 == record->_head._flag_and_size )
         {
            break ;
         }
         recNum++ ;
         beginOffset += record->getSize() ;
         dataSize += record->getDataLength() ;
         totalSize += record->getSize() ;
         // Invalidate the record by set its _flag_and_size to 0. In the scanner
         // it will be a mark of record ending.
         record->_head._flag_and_size = 0 ;
      }

      PD_TRACE_EXIT( SDB__DMSSTORAGEDATACAPPED_COUNTRECNUMANDSIZE ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED_POPRECORD, "_dmsStorageDataCapped::popRecord" )
   INT32 _dmsStorageDataCapped::popRecord ( dmsMBContext *context,
                                            INT64 logicalID,
                                            pmdEDUCB *cb,
                                            SDB_DPSCB *dpscb,
                                            INT8 direction )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED_POPRECORD ) ;
      dmsRecordID firstRID ;
      dmsExtRW extRW ;
      const dmsExtent *startExtent = NULL ;
      UINT32 logRecSize = 0 ;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB() ;
      dpsMergeInfo info ;
      dmsExtentID extentID = DMS_INVALID_EXTENT ;
      dmsExtentID extLID = DMS_INVALID_EXTENT ;
      dmsOffset offset = 0 ;
      dpsLogRecord &dpsRecord = info.getMergeBlock().record() ;
      CHAR fullName[DMS_COLLECTION_FULL_NAME_SZ + 1] = { 0 } ;

      SDB_ASSERT( context, "context should not be NULL" ) ;
      SDB_ASSERT( cb, "edu cb should not be NULL" ) ;

      rc = _extractRecLID( context, logicalID, cb, extentID, extLID, offset ) ;
      PD_RC_CHECK( rc, PDERROR, "Invalid LogicalID[%lld], rc: %d",
                   logicalID, rc ) ;

      if ( !context->isMBLock( EXCLUSIVE ) )
      {
         PD_LOG( PDERROR, "Caller must hold mb exclusive lock[%s]",
                 context->toString().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

#ifdef _DEBUG
      // Here we use delete access type.
      if ( !dmsAccessAndFlagCompatiblity( context->mb()->_flag,
                                          DMS_ACCESS_TYPE_DELETE ) )
      {
         PD_LOG( PDERROR, "Imcompatible collection mode: %d",
                 context->mb()->_flag ) ;
         rc = SDB_DMS_INCOMPATIBLE_MODE ;
         goto error ;
      }
#endif /* _DEBUG */

      // If the collection is emplty, return directly.
      if ( 0 == _mbStatInfo[ context->mbID() ]._totalRecords )
      {
         goto done ;
      }

      if ( dpscb )
      {
         _clFullName( context->mb()->_collectionName, fullName,
                      sizeof(fullName) ) ;
         rc = dpsPop2Record( fullName, firstRID, logicalID,
                             direction, dpsRecord ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to build record, rc: %d", rc ) ;

         rc = dpscb->checkSyncControl( dpsRecord.alignedLen(), cb ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Check sync control failed, rc: %d", rc ) ;

         logRecSize = dpsRecord.alignedLen() ;
         rc = pTransCB->reservedLogSpace( logRecSize, cb ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to reserve log space"
                    "(length=%u), rc: %d", logRecSize, rc ) ;
            logRecSize = 0 ;
            goto error ;
         }
      }

      rc = _recycleExtents( context, extentID, direction ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to recycle extents, rc: %d", rc ) ;

      rc = _popRecord( context, extentID, offset, direction ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to pop records, logical id: %lld, "
                   "rc: %d", logicalID, rc ) ;

      extRW = extent2RW( context->mb()->_firstExtentID, context->mbID() ) ;
      startExtent = extRW.readPtr<dmsExtent>() ;
      if ( dpscb )
      {
         info.enableTrans() ;
         rc = _logDPS( dpscb, info, cb, context, startExtent->_logicID, FALSE,
                       DMS_FILE_DATA ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to insert record into log, rc: %d", rc ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED_POPRECORD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED_DUMPEXTOPTIONS, "_dmsStorageDataCapped::dumpExtOptions" )
   INT32 _dmsStorageDataCapped::dumpExtOptions( dmsMBContext *context,
                                                BSONObj &extOptions )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED_DUMPEXTOPTIONS ) ;
      dmsExtRW extentRW ;
      const dmsOptExtent *optExtent = NULL ;
      dmsCappedCLOptions *options = NULL ;
      UINT32 optSize = 0 ;
      BSONObjBuilder builder ;

      if ( !context->isMBLock() )
      {
         PD_RC_CHECK( rc, PDERROR, "Caller should hold mb shared lock[%s]",
                      context->toString().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( DMS_INVALID_EXTENT == context->mb()->_mbOptExtentID )
      {
         PD_LOG( PDERROR, "Extend option extent id is invalid for "
                 "collection[%s]", context->mb()->_collectionName ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      extentRW = extent2RW( context->mb()->_mbOptExtentID, context->mbID() ) ;
      extentRW.setNothrow( TRUE ) ;
      optExtent = extentRW.readPtr<dmsOptExtent>( 0, pageSize() ) ;
      if ( !optExtent )
      {
         PD_LOG( PDERROR, "Extend option extent is invalid for collection[%s]",
                 context->mb()->_collectionName ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = optExtent->getOption( (CHAR **)&options, &optSize ) ;
      PD_RC_CHECK( rc, PDERROR, "Get extend options from extent failed for "
                   "collection[%s], rc: %d",
                   context->mb()->_collectionName, rc ) ;
      SDB_ASSERT( sizeof( dmsOptExtent ) == optSize,
                  "Option size is not as expected" ) ;

      builder.append( FIELD_NAME_SIZE, options->_maxSize ) ;
      builder.append( FIELD_NAME_MAX, options->_maxRecNum ) ;

      extOptions = builder.obj() ;

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED_DUMPEXTOPTIONS, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

