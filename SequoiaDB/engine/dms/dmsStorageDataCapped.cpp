#include "dmsStorageDataCapped.hpp"
#include "dpsOp2Record.hpp"
#include "pmd.hpp"
#include "mthModifier.hpp"
#include "dmsTrace.hpp"

using namespace bson ;

namespace engine
{
   _dmsStorageDataCapped::_dmsStorageDataCapped( const CHAR* pSuFileName,
                                                 dmsStorageInfo *pInfo,
                                                 _IDmsEventHolder *pEventHolder )
   : _dmsStorageDataCommon( pSuFileName, pInfo, pEventHolder )
   {
   }

   _dmsStorageDataCapped::~_dmsStorageDataCapped()
   {
   }

   const CHAR* _dmsStorageDataCapped::_getEyeCatcher() const
   {
      return DMS_DATACAPSU_EYECATCHER ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONMAPMETA, "_dmsStorageDataCapped::_onMapMeta" )
   INT32 _dmsStorageDataCapped::_onMapMeta( UINT64 curOffSet )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONMAPMETA ) ;
      rc = dmsStorageDataCommon::_onMapMeta( curOffSet ) ;
      if ( rc )
      {
         goto error ;
      }

      for ( UINT16 i = 0; i < DMS_MME_SLOTS; ++i )
      {
         if ( DMS_IS_MB_INUSE( _dmsMME->_mbList[i]._flag ) )
         {
            _mbStatInfo[i]._maxSize = _dmsMME->_mbList[i]._maxSize ;
            _mbStatInfo[i]._maxRecNum = _dmsMME->_mbList[i]._maxRecNum ;
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__ONMAPMETA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONOPENED, "_dmsStorageDataCapped::_onOpened" )
   INT32 _dmsStorageDataCapped::_onOpened()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONOPENED ) ;
      // Traverse all the collections which are being used, and the the working
      // extents.
      rc = dmsStorageDataCommon::_onOpened() ;
      if ( rc )
      {
         goto error ;
      }

      for ( UINT16 i = 0 ; i < DMS_MME_SLOTS; ++i )
      {
         if ( DMS_IS_MB_INUSE ( _dmsMME->_mbList[i]._flag ) &&
              ( DMS_INVALID_EXTENT != _dmsMME->_mbList[i]._lastExtentID ) )
         {
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
      // Flush all working extents information.
      for ( UINT16 i = 0; i < DMS_MME_SLOTS; ++i )
      {
         if ( DMS_IS_MB_INUSE( _dmsMME->_mbList[i]._flag ) )
         {
            extInfo = getWorkExtInfo( i ) ;
            SDB_ASSERT( extInfo, "Impossible" ) ;
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

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONALLOCEXTENT, "_dmsStorageDataCapped::_onAllocExtent" )
   INT32 _dmsStorageDataCapped::_onAllocExtent( dmsMBContext *context,
                                                dmsExtent *extAddr,
                                                SINT32 extentID,
                                                BOOLEAN map2DelList )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONALLOCEXTENT ) ;
      dmsCappedExtent *extent = (dmsCappedExtent *)extAddr ;
      SDB_ASSERT( context, "Context should not be NULL" ) ;
      SDB_ASSERT( extAddr, "Extent address should not be NULL" ) ;

      if ( !context->isMBLock( EXCLUSIVE ) )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Caller should hold mb exclusive lock, rc: %d", rc ) ;
         goto error ;
      }

      if ( DMS_INVALID_EXTENT == context->mb()->_firstLogicExtentID )
      {
         dmsExtRW extRWTmp = extent2RW( extentID, context->mbID() ) ;
         extRWTmp.setNothrow( TRUE ) ;
         dmsCappedExtent *cappedExt = extRWTmp.writePtr<dmsCappedExtent>() ;
         if ( !cappedExt )
         {
            PD_LOG( PDERROR, "Invalid extent[ %d ]",
                    context->mb()->_lastLogicExtentID ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         context->mb()->_firstLogicExtentID = extentID ;
         context->mb()->_lastLogicExtentID = extentID ;
         cappedExt->_preLogicExtent = DMS_INVALID_EXTENT ;
         cappedExt->_nextLogicExtent = DMS_INVALID_EXTENT ;
         cappedExt->_logicID = 0 ;
      }
      else
      {
         // Link to logical extent list.
         SDB_ASSERT( DMS_INVALID_EXTENT != context->mb()->_lastLogicExtentID,
                     "Last logical extent id is invalid" ) ;
         dmsExtRW lastExtRW = extent2RW( context->mb()->_lastLogicExtentID,
                                         context->mbID() ) ;
         lastExtRW.setNothrow( TRUE ) ;
         dmsCappedExtent *lastCapExt = lastExtRW.writePtr<dmsCappedExtent>(0) ;
         if ( !lastCapExt )
         {
            PD_LOG( PDERROR, "Invalid extent[ %d ]",
                    context->mb()->_lastLogicExtentID ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         extent->_logicID = lastCapExt->_logicID + 1 ;
         extent->_preLogicExtent = context->mb()->_lastLogicExtentID ;
         extent->_nextLogicExtent = DMS_INVALID_EXTENT ;
         lastCapExt->_nextLogicExtent = extentID ;
         context->mb()->_lastLogicExtentID = extentID ;
      }

      _mbStatInfo[context->mbID()]._totalDataFreeSpace +=
            ( (extAddr->_blockSize) << pageSizeSquareRoot() ) -
            DMS_CAPPEDEXTENT_METADATA_SZ ;
   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__ONALLOCEXTENT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__ONFREEEXTENT, "_dmsStorageDataCapped::_onFreeExtent" )
   INT32 _dmsStorageDataCapped::_onFreeExtent( dmsMBContext *context,
                                               dmsExtent *extAddr,
                                               SINT32 extentID )
   {
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONFREEEXTENT ) ;
      dmsCappedExtent *cappedExt = (dmsCappedExtent *)extAddr ;

      if ( extentID == context->mb()->_firstLogicExtentID )
      {
         context->mb()->_firstLogicExtentID = cappedExt->_nextLogicExtent ;
      }

      if ( extentID == context->mb()->_lastLogicExtentID )
      {
         context->mb()->_lastLogicExtentID = cappedExt->_preLogicExtent ;
      }

      PD_TRACE_EXIT( SDB__DMSSTORAGEDATACAPPED__ONFREEEXTENT ) ;

      return SDB_OK ;
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
         size += sizeof( logicIdToInsert ) ;
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
                               FALSE, FALSE, &extID ) ;
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
            rc = _allocateExtent( context, DMS_CAP_EXTENT_PAGE_NUM, FALSE,
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
      // TODO: add other conditions
      if ( DMS_ACCESS_TYPE_INSERT != accessType
           && DMS_ACCESS_TYPE_POP != accessType )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Operation is not allowed on capped collection" ) ;
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
      dmsCappedExtent *extent = NULL ;
      INT64 recNum = 0 ;
      INT64 dataSize = 0 ;
      INT64 totalSize = 0 ;
      dmsExtentInfo *extInfo = getWorkExtInfo( context->mbID() ) ;

      extRW = extent2RW( extID, mbID ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsCappedExtent>() ;
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
      _mbStatInfo[mbID]._totalDataLen -= extInfo->_totalDataLen ;
      _mbStatInfo[mbID]._totalOrgDataLen -= extInfo->_totalOrgDataLen ;
      _mbStatInfo[mbID]._totalDataFreeSpace -= extent->_freeSpace ;

      extInfo->_recCount = 0 ;
      extInfo->_firstRecordOffset = extInfo->_lastRecordOffset ;
      extInfo->_totalDataLen = 0 ;
      extInfo->_totalOrgDataLen = 0 ;

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
      dmsCappedExtent *extent = NULL ;
      INT64 recNum = 0 ;
      INT64 dataSize = 0 ;
      INT64 totalSize = 0 ;

      extRW = extent2RW( extID, mbID ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsCappedExtent>() ;
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
      pRecord->setData( workExtInfo->getRecordLogicID(), recordData ) ;

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
      return SDB_PERM ;
   }

   INT32 _dmsStorageDataCapped::_extentRemoveRecord( dmsMBContext *context,
                                                     dmsExtRW &extRW,
                                                     dmsRecordRW &recordRW,
                                                     pmdEDUCB *cb,
                                                     BOOLEAN decCount )
   {
      SDB_ASSERT( FALSE, "Should not be here" ) ;
      return SDB_PERM ;
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
         dmsCappedExtent *extent = NULL ;
         INT64 logicalID = -1 ;

         extRW = extent2RW( rid._extent, context->mbID() ) ;
         extRW.setNothrow( TRUE ) ;
         extent = extRW.writePtr<dmsCappedExtent>() ;
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
      dmsCappedExtent *extent = NULL ;
      dmsExtentInfo *extInfo = getWorkExtInfo( collectionID ) ;

      SDB_ASSERT( extInfo, "Impossible" ) ;

      extID = extInfo->getID() ;
      if ( DMS_INVALID_EXTENT == extID )
      {
         goto done ;
      }

      extRW = extent2RW( extID, collectionID ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsCappedExtent>() ;
      if ( !extent )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid extent: %d, rc: %d", extID, rc ) ;
         goto error ;
      }

      extent->_recCount = extInfo->_recCount ;
      extent->_freeSpace = extInfo->_freeSpace ;
      extent->_firstRecordOffset = extInfo->_firstRecordOffset ;
      extent->_lastRecordOffset = extInfo->_lastRecordOffset ;

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
      const dmsCappedExtent *extent = NULL ;
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
      extent = extRW.readPtr<dmsCappedExtent>() ;
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

      workExtInfo->_totalOrgDataLen = extent->_lastRecordOffset ;
      workExtInfo->_totalDataLen = extent->_lastRecordOffset ;

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
      workExtInfo->_totalOrgDataLen -= dataSize ;
      workExtInfo->_totalDataLen -= dataSize ;

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
      dmsCappedExtent *extent = NULL ;
      INT64 recNum = 0 ;
      INT64 dataSize = 0 ;
      INT64 totalSize = 0 ;

      extRW = extent2RW( extentID, context->mbID() ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsCappedExtent>() ;
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
                                                dmsExtentID &extentID,
                                                dmsExtentID &extLID,
                                                dmsOffset &offset )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__EXTRACTRECLID ) ;
      dmsRecordID recordID ;
      dmsRecordRW recordRW ;
      const dmsRecord *record = NULL ;

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
         if ( !record )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "Invalid logical id[%lld], rc: %d",
                    logicalID, rc ) ;
            goto error ;
         }
         else
         {
            if ( ! ( DMS_RECORD_FLAG_NORMAL == record->getState() &&
                      logicalID == record->getLogicID() ) )
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
      const dmsCappedExtent *extent = NULL ;

      extRW = extent2RW( targetExtID, context->mbID() ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.readPtr<dmsCappedExtent>() ;
      if ( !extent )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid extent: %d, rc: %d", targetExtID, rc ) ;
         goto error ;
      }

      if ( direction >= 0 )
      {
         extentID = context->mb()->_firstLogicExtentID ;
         endExtID = extent->_preLogicExtent ;
      }
      else
      {
         endExtID = getWorkExtInfo( context->mbID() )->getID() ;
         extentID = extent->_nextLogicExtent ;
         rc = _switchWorkExt( context, targetExtID ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to switch working extent, rc: %d",
                      rc ) ;
      }

      while ( DMS_INVALID_EXTENT != extentID )
      {
         extRW = extent2RW( extentID, context->mbID() ) ;
         extRW.setNothrow( TRUE ) ;
         extent = extRW.readPtr<dmsCappedExtent>() ;
         if ( !extent )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid extent[%d], rc: %d", extentID, rc ) ;
            goto error ;
         }
         if ( extentID == targetExtID )
         {
            break ;
         }

         rc = _recycleActiveExt( context, extentID ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to recycle active extent, rc: %d", rc ) ;
         if ( extentID == endExtID )
         {
            break ;
         }

         extentID = extent->_nextLogicExtent;
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
      const dmsRecord *record = NULL ;

      recNum = 0 ;
      dataSize = 0 ;
      totalSize = 0 ;
      // If pop forward, need to go beyond the offset.
      // If pop backward, need to go beyond or just on the offset.
      while ( beginOffset <= endOffset )
      {
         if ( beginOffset == endOffset && direction < 0 )
         {
            break ;
         }
         dmsRecordID rid( extentID, beginOffset ) ;
         recordRW = record2RW( rid, context->mbID() ) ;
         recordRW.setNothrow( TRUE ) ;
         record = recordRW.readPtr<dmsRecord>() ;
         SDB_ASSERT( record->getSize() > 0, "Invalid record size" ) ;
         recNum++ ;
         beginOffset += record->getSize() ;
         dataSize += record->getDataLength() ;
         totalSize += record->getSize() ;
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
      const dmsCappedExtent *startExtent = NULL ;
      UINT32 logRecSize = 0 ;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB() ;
      dpsMergeInfo info ;
      dmsExtentID extentID = DMS_INVALID_EXTENT ;
      dmsExtentID extLID = DMS_INVALID_EXTENT ;
      dmsOffset offset = 0 ;
      dpsLogRecord &dpsRecord = info.getMergeBlock().record() ;
      CHAR fullName[DMS_COLLECTION_FULL_NAME_SZ + 1] = { 0 } ;

      // getbeginlogicid
      // popfromlastextent
      // popfromnonlastextent

      // If the first extent is the working extent, pop record one by one.
      // Otherwise, recycle the first extent.

      SDB_ASSERT( context, "context should not be NULL" ) ;
      SDB_ASSERT( cb, "edu cb should not be NULL" ) ;

      rc = _extractRecLID( context, logicalID, extentID, extLID, offset ) ;
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
      startExtent = extRW.readPtr<dmsCappedExtent>() ;
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
}

