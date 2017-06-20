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
                                                dmsExtent * extAddr,
                                                SINT32 extentID,
                                                BOOLEAN map2DelList )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__ONALLOCEXTENT ) ;
      SDB_ASSERT( context, "Context should not be NULL" ) ;
      SDB_ASSERT( extAddr, "Extent address should not be NULL" ) ;

      if ( !context->isMBLock( EXCLUSIVE ) )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Caller should hold mb exclusive lock, rc: %d", rc ) ;
         goto error ;
      }

      _mbStatInfo[context->mbID()]._totalDataFreeSpace +=
            ( (extAddr->_blockSize) << pageSizeSquareRoot() ) - sizeof( dmsExtent ) ;
   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__ONALLOCEXTENT, rc ) ;
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
      SDB_ASSERT( workExtInfo, "Impossible" ) ;

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
         rc = _switchWorkExt( context, extID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to switch working extent, rc: %d", rc ) ;
         extSwitched = TRUE ;
      }
      else
      {
         // Check if exceeding the Size and Max limitations.
         if ( _exceedLimit( context, size ) )
         {
            rc = _recycleOneExtent( context ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to recycle extent, rc: %d", rc ) ;
         }

         // If free space in current working extent is not enough, get another.
         if ( workExtInfo->_freeSpace < (INT32)size )
         {
            rc = _getIdleExt( context, extID ) ;
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to reuse idle extent, rc: %d", rc ) ;
            if ( DMS_INVALID_EXTENT == extID )
            {
               rc = _allocateExtent( context, DMS_CAP_EXTENT_PAGE_NUM, FALSE,
                                     FALSE, &extID ) ;
               PD_RC_CHECK( rc, PDERROR,
                            "Failed to allocate extent, rc: %d", rc ) ;
               newExtAlloc = TRUE ;
            }

            rc = _switchWorkExt( context, extID ) ;
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to switch working extent, rc: %d", rc ) ;
            extSwitched = TRUE ;
         }
      }

      _mbStatInfo[context->mbID()]._totalDataFreeSpace -= size ;
      workExtInfo->_freeSpace -= size ;

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
      dmsExtent *extent = NULL ;
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

      // Initialize the extent, update the statistic information in _mbStatInfo
      // and work extent info.
       extent->init( DMS_CAP_EXTENT_PAGE_NUM, mbID, DMS_CAP_EXTENT_SZ ) ;
      _mbStatInfo[mbID]._totalRecords -= extInfo->_recCount ;
      _mbStatInfo[mbID]._totalDataLen -= extInfo->_totalDataLen ;
      _mbStatInfo[mbID]._totalOrgDataLen -= extInfo->_totalOrgDataLen ;

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
                                                   dmsExtentID extID,
                                                   BOOLEAN moveToEnd )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__RECYCLEACTIVEEXT ) ;
      UINT16 mbID = context->mbID() ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;

      extRW = extent2RW( extID, mbID ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsExtent>() ;
      if ( !extent )
      {
         PD_LOG( PDERROR, "Invalid extent: %d", extID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // Set the extent as reserved.
      context->mb()->_flag = DMS_EXTENT_FLAG_RESERVE ;

      _mbStatInfo[mbID]._totalRecords -= extent->_recCount ;
      _mbStatInfo[mbID]._totalDataPages -= extent->_blockSize ;
      _mbStatInfo[mbID]._totalDataFreeSpace -= extent->_freeSpace ;

      if ( moveToEnd )
      {
         _transferIdleExt( context, extID ) ;
      }

   done:
      PD_TRACE_EXITRC( SDB__DMSSTORAGEDATACAPPED__RECYCLEACTIVEEXT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_getIdleExt( dmsMBContext *context,
                                             dmsExtentID &extID )
   {
      INT32 rc = SDB_OK ;
      dmsExtRW extRW ;
      const dmsExtent *extent = NULL ;
      dmsExtentID foundExtID = DMS_INVALID_EXTENT ;

      extRW = extent2RW( context->mb()->_lastExtentID, context->mbID() ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.readPtr<dmsExtent>() ;
      if ( !extent )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid extent[%d], rc: %d",
                 context->mb()->_lastExtentID, rc ) ;
         goto error ;
      }
      foundExtID = extent->_nextExtent ;

      if ( DMS_INVALID_EXTENT != foundExtID )
      {
         // There are idle extents after the last extent.
         dmsExtent *foundExtent = NULL ;
         SDB_ASSERT( DMS_INVALID_EXTENT != context->mb()->_lastIdleExt,
                     "Last idle extent in mb should not be invalid" ) ;
         extRW = extent2RW( foundExtID, context->mbID() ) ;
         extRW.setNothrow( TRUE ) ;
         foundExtent = extRW.writePtr<dmsExtent>() ;
         if ( !foundExtent )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid extent[%d], rc: %d",
                    context->mb()->_lastExtentID, rc ) ;
            goto error ;
         }

         SDB_ASSERT( foundExtent->_flag & DMS_EXTENT_FLAG_RESERVE,
                     "Extent flag invalid" ) ;
         foundExtent->init( DMS_CAP_EXTENT_PAGE_NUM, context->mbID(),
                       DMS_CAP_EXTENT_SZ ) ;
         if ( foundExtID == context->mb()->_lastIdleExt )
         {
            // No more idle extents at the tail of the extent list.
            context->mb()->_lastIdleExt = DMS_INVALID_EXTENT ;
         }
         context->mb()->_lastExtentID = foundExtID ;
         // flushMeta( isSyncDeep() ) ;
      }

      extID = foundExtID ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_transferIdleExt( dmsMBContext *context,
                                                  dmsExtentID lastIdleExt )
   {
      // Transfer the idle extents from the head to the tail of the extent list.
      INT32 rc = SDB_OK ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;
      dmsMB *mb = context->mb() ;
      dmsExtentID firstIdleExt = DMS_INVALID_EXTENT ;

      if ( DMS_INVALID_EXTENT == lastIdleExt )
      {
         goto done ;
      }

      // Change the first extent of the collection.
      if ( lastIdleExt == mb->_firstExtentID )
      {
         extRW = extent2RW( lastIdleExt, context->mbID() ) ;
         extRW.setNothrow( TRUE ) ;
         extent = extRW.writePtr<dmsExtent>() ;
         if ( !extent )
         {
            PD_LOG( PDERROR, "Invalid extent: %d", lastIdleExt ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         mb->_firstExtentID = extent->_nextExtent ;
         extent->_nextExtent = DMS_INVALID_EXTENT ;
      }

      // Link the idle extents to the end of the extent list, and change the
      // last extent of the collection.
      if ( DMS_INVALID_EXTENT != mb->_lastIdleExt )
      {
         extRW = extent2RW( mb->_lastIdleExt, context->mbID() ) ;
      }
      else
      {
         extRW = extent2RW( mb->_lastExtentID, context->mbID() ) ;
      }

      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsExtent>() ;
      if ( !extent )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid extent, rc: %d", rc ) ;
         goto error ;
      }
      extent->_nextExtent = firstIdleExt ;
      mb->_lastIdleExt = lastIdleExt ;

      flushMeta( isSyncDeep() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__DMSSTORAGEDATACAPPED__RECYCLEEXTENT, "_dmsStorageDataCapped::_recycleExtent" )
   INT32 _dmsStorageDataCapped::_recycleOneExtent( dmsMBContext *context )
   {
      // Extent will be recycled when the size or record number threshold is hit
      // or when all the records in the extent are popped. After recycling, one
      // extent becomes idle extent. If the idle extent number reaches 4, they
      // will be put at the end of the extent list.
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__DMSSTORAGEDATACAPPED__RECYCLEEXTENT ) ;
      dmsExtentID extentID = DMS_INVALID_EXTENT ;
      dmsExtentInfo *workExtInfo = getWorkExtInfo( context->mbID() ) ;

      if ( !context->isMBLock( EXCLUSIVE ) )
      {
         PD_LOG( PDERROR, "Caller must hold mb exclusive lock[%s]" ,
                 context->toString().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !workExtInfo )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to get working extent, rc: %d", rc ) ;
         goto error ;
      }

      extentID = context->mb()->_firstExtentID ;
      if ( extentID == workExtInfo->getID() )
      {
         rc = _recycleWorkExt( context, extentID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to recycle working extent, rc: %d", rc ) ;
         rc = _transferIdleExt( context, extentID ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to transfer idle extents, rc: %d", rc ) ;
      }
      else
      {
         rc = _recycleActiveExt( context, extentID, TRUE ) ;
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

   INT32 _dmsStorageDataCapped::_onInsertFail( dmsMBContext *context,
                                               BOOLEAN hasInsert,
                                               dmsRecordID rid,
                                               SDB_DPSCB *dpscb,
                                               ossValuePtr dataPtr,
                                               pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      // TODO: Need to remove the record, and recycle the space.
      if ( hasInsert )
      {
         // Revert all what has been done in _extentInsertRecord.
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_syncWorkExtInfo( UINT16 collectionID )
   {
      INT32 rc = SDB_OK ;
      dmsExtentID extID = DMS_INVALID_EXTENT ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;
      dmsExtentInfo *extInfo = getWorkExtInfo( collectionID ) ;

      SDB_ASSERT( extInfo, "Impossible" ) ;

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
      extent->_lastRecordOffset = extInfo->_lastRecordOffset ;

   done:
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

/*
      if ( DMS_INVALID_REC_LOGICALID == workExtInfo->_recLogicID )
      {
         workExtInfo->_recLogicID = DMS_CAP_EXTENT_BODY_SZ * extent->_logicID ;
      }
      else
      {
         workExtInfo->_recLogicID =
            ( workExtInfo->_recLogicID / DMS_CAP_EXTENT_SZ + 1 )
            * DMS_CAP_EXTENT_SZ + sizeof( dmsExtent ) ;
      }
*/
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
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_popFromWorkExt( dmsMBContext *context,
                                                 dmsExtentID extentID,
                                                 dmsOffset offset,
                                                 INT8 direction )
   {
      INT32 rc = SDB_OK ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;
      dmsOffset currOffset =  0 ;
      dmsRecordRW recordRW ;
      const dmsRecord *record = NULL ;
      UINT32 recNum = 0 ;
      dmsExtentInfo *workExtInfo = &_workExtInfo[context->mbID()] ;

      if ( offset >= workExtInfo->_lastRecordOffset )
      {
         if ( direction >= 0 )
         {
            // TODO: all records are going to be popped.
            workExtInfo->_recCount = 0 ;
            workExtInfo->_firstRecordOffset = workExtInfo->_lastRecordOffset ;
            workExtInfo->_totalOrgDataLen = 0 ;
            workExtInfo->_totalDataLen = 0 ;
            // TODO: update mbstat
            // TODO: sync ?
         }
         goto done ;
      }

      currOffset = workExtInfo->_firstRecordOffset ;
      // If pop forward, need to go beyond the offset.
      // If pop backward, need to go beyond or just on the offset.
      while ( currOffset <= offset )
      {
         if ( currOffset == offset && direction < 0 )
         {
            break ;
         }
         dmsRecordID rid( extentID, currOffset ) ;
         recordRW = record2RW( rid, context->mbID() ) ;
         recordRW.setNothrow( TRUE ) ;
         record = recordRW.readPtr<dmsRecord>() ;
         SDB_ASSERT( record->getSize() > 0, "Invalid record size" ) ;
         recNum++ ;
         currOffset += record->getSize() ;
      }

      if ( direction >= 0 )
      {
         workExtInfo->_firstRecordOffset = currOffset ;
         workExtInfo->_recCount -= recNum ;
         // workExtInfo->_freeSpace +=
      }
      else
      {
         workExtInfo->_lastRecordOffset = currOffset ;
         workExtInfo->_recCount = recNum ;
         workExtInfo->_freeSpace = DMS_CAP_EXTENT_BODY_SZ - currOffset ;
      }

      extRW = extent2RW( extentID, context->mbID() ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsExtent>() ;
      SDB_ASSERT( extent, "Invalid extent" ) ;
      extent->_firstRecordOffset = workExtInfo->_firstRecordOffset ;
      extent->_lastRecordOffset = workExtInfo->_lastRecordOffset ;
      extent->_recCount = workExtInfo->_recCount ;
      extent->_freeSpace = workExtInfo->_freeSpace ;

      _mbStatInfo[context->mbID()]._totalRecords = workExtInfo->_recCount ;
      // TODO: more data to sync
      // TODO: sync ?

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_popFromActiveExt( dmsMBContext *context,
                                                   dmsExtentID extentID,
                                                   dmsOffset offset,
                                                   INT8 direction )
   {
      INT32 rc = SDB_OK ;
      dmsExtRW extRW ;
      dmsExtent *extent = NULL ;
      dmsOffset currOffset =  0 ;
      dmsRecordRW recordRW ;
      const dmsRecord *record = NULL ;
      UINT32 recNum = 0 ;

      extRW = extent2RW( extentID, context->mbID() ) ;
      extRW.setNothrow( TRUE ) ;
      extent = extRW.writePtr<dmsExtent>() ;
      if ( !extent )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Invalid extent, rc: %d", rc ) ;
         goto error ;
      }

      if ( offset >= extent->_lastRecordOffset )
      {
         if ( direction >= 0 )
         {
            // TODO: all records are going to be popped.
            extent->_recCount = 0 ;
            extent->_firstRecordOffset = extent->_lastRecordOffset ;
            // TODO: update mbstat
            // TODO: sync ?
         }
         goto done ;
      }

      currOffset = extent->_firstRecordOffset ;
      // If pop forward, need to go beyond the offset.
      // If pop backward, need to go beyond or just on the offset.
      while ( currOffset <= offset )
      {
         if ( currOffset == offset && direction < 0 )
         {
            break ;
         }
         dmsRecordID rid( extentID, currOffset ) ;
         recordRW = record2RW( rid, context->mbID() ) ;
         recordRW.setNothrow( TRUE ) ;
         record = recordRW.readPtr<dmsRecord>() ;
         SDB_ASSERT( record->getSize() > 0, "Invalid record size" ) ;
         recNum++ ;
         currOffset += record->getSize() ;
      }

      if ( direction >= 0 )
      {
         extent->_firstRecordOffset = currOffset ;
         extent->_recCount -= recNum ;
         // workExtInfo->_freeSpace +=
      }
      else
      {
         // TODO: switch back one by one and pop from work ext?
         // workExtInfo->_lastRecordOffset = currOffset ;
         // workExtInfo->_recCount = recNum ;
         // workExtInfo->_freeSpace = DMS_CAP_EXTENT_BODY_SZ - currOffset ;
      }

      _mbStatInfo[context->mbID()]._totalRecords = extent->_recCount ;
      // TODO: more data to sync
      // TODO: sync ?

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_popRecord( dmsMBContext *context,
                                            dmsExtentID extID,
                                            dmsOffset offset,
                                            INT8 direction )
   {
      INT32 rc = SDB_OK ;
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
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_extractRecLID( dmsMBContext *context,
                                                INT64 logicalID,
                                                dmsExtentID &extentID,
                                                dmsExtentID &extLID,
                                                dmsOffset &offset )
   {
      INT32 rc = SDB_OK ;
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
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsStorageDataCapped::_recycleExtents( dmsMBContext *context,
                                                 dmsExtentID targetExtID,
                                                 INT8 direction )
   {
      // Recycle all the extents which will be freed by pop.
      INT32 rc = SDB_OK ;
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
         endExtID = getWorkExtInfo( context->mbID() )->getID() ;
         extentID = extent->_nextExtent ;
         rc = _switchWorkExt( context, targetExtID ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to switch working extent, rc: %d",
                      rc ) ;
      }

      while ( DMS_INVALID_EXTENT != extentID )
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
         if ( extentID == targetExtID )
         {
            break ;
         }

         rc = _recycleActiveExt( context, extentID, ( direction >= 0 ) ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to recycle active extent, rc: %d", rc ) ;
         if ( extentID == endExtID )
         {
            break ;
         }

         extentID = extent->_nextExtent ;
      }

   done:
      return rc ;
   error:
      goto done ;
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

      // getbeginlogicid
      // popfromlastextent
      // popfromnonlastextent

      // If the first extent is the working extent, pop record one by one.
      // Otherwise, recycle the first extent.

      SDB_ASSERT( context, "context should not be NULL" ) ;
      SDB_ASSERT( cb, "edu cb should not be NULL" ) ;

      rc = _extractRecLID( context, logicalID, extentID, extLID, offset ) ;
      PD_RC_CHECK( rc, PDERROR, "Invalid LogicalID[%lld], rc: %d", rc ) ;

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
      // TODO: restore the data which have been popped out.
      goto done ;
   }
}

