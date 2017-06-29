/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = dmsStorageDataCapped.hpp

   Descriptive Name = Data Management wrapper for data type.

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          20/11/2016  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef DMSSTORAGE_DATACAPPED_HPP
#define DMSSTORAGE_DATACAPPED_HPP

#include "dmsStorageDataCommon.hpp"

namespace engine
{
#define DMS_INVALID_REC_LOGICALID         0xffffffffffffffff
#define DMS_CAP_EXTENT_SZ                 (32 * 1024 * 1024)
#define DMS_CAP_EXTENT_BODY_SZ            ( DMS_CAP_EXTENT_SZ - sizeof( dmsExtent ) )
#define DMS_CAP_EXTENT_PAGE_NUM           \
   (DMS_CAP_EXTENT_SZ >> pageSizeSquareRoot())

   class _pmdEDUCB ;
   class _mthModifier ;


   // Information of the working extent.
   struct _dmsExtentInfo
   {
      dmsExtentID _id ;
      INT64 _extLogicID ;
      UINT32 _recCount ;
      INT32 _freeSpace ;
      dmsOffset _firstRecordOffset ;
      dmsOffset _lastRecordOffset ;
      UINT32 _totalOrgDataLen ;
      UINT32 _totalDataLen ;

      _dmsExtentInfo()
      {
         reset() ;
      }

      void reset()
      {
         _id = DMS_INVALID_EXTENT ;
         _extLogicID = DMS_INVALID_EXTENT ;
         _recCount = 0 ;
         _freeSpace = 0 ;
         _firstRecordOffset = DMS_INVALID_OFFSET ;
         _lastRecordOffset = DMS_INVALID_OFFSET ;
         _totalOrgDataLen = 0 ;
         _totalDataLen = 0 ;
      }

      const dmsExtentID getID() const { return _id ; }
      dmsOffset getNextRecOffset() const
      {
         return ( DMS_INVALID_OFFSET == _lastRecordOffset ) ?
                sizeof( dmsExtent ) : _lastRecordOffset ;
      }

      INT64 getRecordLogicID()
      {
         INT64 logicID = _extLogicID * DMS_CAP_EXTENT_BODY_SZ ;
         if ( DMS_INVALID_OFFSET != _lastRecordOffset )
         {
            logicID += _lastRecordOffset - sizeof( dmsExtent ) ;
         }

         return logicID ;
      }
   } ;
   typedef _dmsExtentInfo dmsExtentInfo ;

   class _dmsStorageDataCapped : public _dmsStorageDataCommon
   {
   public:
      _dmsStorageDataCapped( const CHAR* pSuFileName,
                             dmsStorageInfo *pInfo,
                             _IDmsEventHolder *pEventHolder ) ;
      virtual ~_dmsStorageDataCapped() ;

      virtual INT32 popRecord ( dmsMBContext *context,
                                INT64 logicalID,
                                _pmdEDUCB *cb,
                                SDB_DPSCB *dpscb,
                                INT8 direction = 1 ) ;

      OSS_INLINE dmsExtentInfo* getWorkExtInfo( UINT16 mbID ) ;

   private:
      virtual const CHAR* _getEyeCatcher() const ;
      virtual INT32 _onMapMeta( UINT64 curOffSet ) ;
      virtual INT32 _onOpened() ;
      virtual void _onClosed() ;
      virtual INT32 _onAllocExtent( dmsMBContext *context,
                                    dmsExtent * extAddr,
                                    SINT32 extentID,
                                    BOOLEAN map2DelList ) ;

      virtual void _finalRecordSize( UINT32 &size,
                                     const dmsRecordData &recordData ) ;

      virtual INT32 _allocRecordSpace( dmsMBContext *context,
                                       UINT32 size,
                                       dmsRecordID &foundRID,
                                       _pmdEDUCB *cb ) ;

      virtual INT32 _extentInsertRecord( dmsMBContext *context,
                                         dmsExtRW &extRW,
                                         dmsRecordRW &recordRW,
                                         const dmsRecordData &recordData,
                                         UINT32 recordSize,
                                         _pmdEDUCB *cb,
                                         BOOLEAN isInsert = TRUE ) ;

      virtual INT32 _extentUpdatedRecord( dmsMBContext *context,
                                          dmsExtRW &extRW,
                                          dmsRecordRW &recordRW,
                                          const dmsRecordData &recordData,
                                          const BSONObj &newObj,
                                          _pmdEDUCB *cb ) ;

      virtual INT32 _extentRemoveRecord( dmsMBContext *context,
                                         dmsExtRW &extRW,
                                         dmsRecordRW &recordRW,
                                         _pmdEDUCB *cb,
                                         BOOLEAN decCount = TRUE ) ;

      virtual INT32 _onInsertFail( dmsMBContext *context,
                                   BOOLEAN hasInsert,
                                   dmsRecordID rid,
                                   SDB_DPSCB *dpscb,
                                   ossValuePtr dataPtr,
                                   _pmdEDUCB *cb ) ;

      virtual INT32 extractData( dmsMBContext *mbContext,
                                 dmsRecordRW &recordRW,
                                 _pmdEDUCB *cb,
                                 dmsRecordData &recordData ) ;

      virtual INT32 _operationPermChk( DMS_ACCESS_TYPE accessType ) ;

      INT32 _getIdleExt( dmsMBContext *context, dmsExtentID &extID ) ;

      INT32 _transferIdleExt( dmsMBContext *context, dmsExtentID lastIdleExt ) ;
      INT32 _recycleOneExtent( dmsMBContext *context ) ;
      INT32 _recycleWorkExt( dmsMBContext *context, dmsExtentID extID ) ;
      INT32 _recycleActiveExt( dmsMBContext *context,
                               dmsExtentID extID,
                               BOOLEAN moveToEnd = TRUE ) ;

      INT32 _popFromWorkExt( dmsMBContext *context, dmsExtentID extID,
                             dmsOffset offset, INT8 direction = 1 ) ;

      INT32 _popFromActiveExt( dmsMBContext *context,
                               dmsExtentID extentID,
                               dmsOffset offset,
                               INT8 direction ) ;

      INT32 _syncWorkExtInfo( UINT16 collectionID ) ;
      INT32 _switchWorkExt( dmsMBContext *context, dmsExtentID extID ) ;
      INT32 _attachWorkExt( UINT16 collectionID, dmsExtentID extID ) ;
      INT32 _detachWorkExt( UINT16 collectionID, BOOLEAN sync = TRUE ) ;

      INT32 _popRecord( dmsMBContext *context,
                        dmsExtentID extID,
                        dmsOffset offset,
                        INT8 direction = 1 ) ;

      OSS_INLINE void _updateCLStat( dmsMBStatInfo &mbStat, UINT32 totalSize,
                                     const dmsRecordData &recordData ) ;
      OSS_INLINE void _updateWorkExtStat( _dmsExtentInfo &extInfo,
                                          UINT32 totalSize,
                                          const dmsRecordData &recordData ) ;
      OSS_INLINE void _updateStatInfo( dmsMBContext *context,
                                       UINT32 recordSize,
                                       const dmsRecordData &recordData ) ;
      OSS_INLINE BOOLEAN _exceedLimit( dmsMBContext *context, UINT32 size ) ;
      OSS_INLINE void _getExtLIDAndOffsetByLID( INT64 logicalID,
                                                dmsExtentID &extID,
                                                dmsOffset &offset ) ;
      OSS_INLINE dmsExtentID _logicID2ExtID( dmsMBContext *context,
                                             INT64 logicalID ) ;

      INT32 _extractRecLID( dmsMBContext *context,
                            INT64 logicalID,
                            dmsExtentID &extentID,
                            dmsExtentID &extLID,
                            dmsOffset &offset ) ;

      INT32 _recycleExtents( dmsMBContext *context,
                             dmsExtentID targetExtID,
                             INT8 direction ) ;

      void _countRecNumAndSize( dmsMBContext *context,
                                dmsExtentID extentID,
                                dmsOffset beginOffset,
                                dmsOffset endOffset,
                                INT32 direction,
                                INT64 &recNum,
                                INT64 &dataSize,
                                INT64 &totalSize ) ;

   private:
      // The information of the working extents of each collection.
      // Working extent is the one which we are using for inserting record now.
      dmsExtentInfo _workExtInfo[ DMS_MME_SLOTS ] ;
   } ;
   typedef _dmsStorageDataCapped dmsStorageDataCapped ;

   OSS_INLINE dmsExtentInfo* _dmsStorageDataCapped::getWorkExtInfo(UINT16 mbID)
   {
      if ( mbID >= DMS_MME_SLOTS )
      {
         return NULL ;
      }

      return &_workExtInfo[ mbID ] ;
   }

   OSS_INLINE void _dmsStorageDataCapped::_updateCLStat( dmsMBStatInfo &mbStat,
                                                         UINT32 totalSize,
                                                         const dmsRecordData &recordData )
   {
      ++( mbStat._totalRecords ) ;
      mbStat._totalDataFreeSpace -= totalSize ;
      mbStat._totalOrgDataLen += recordData.orgLen() ;
      mbStat._totalDataLen += recordData.len() ;
      mbStat._lastCompressRatio =
         (UINT8)( recordData.getCompressRatio() * 100 ) ;
   }

   OSS_INLINE void _dmsStorageDataCapped::_updateWorkExtStat( _dmsExtentInfo &extInfo,
                                                              UINT32 totalSize,
                                                              const dmsRecordData &recordData )
   {
      ++( extInfo._recCount ) ;
      extInfo._freeSpace -= totalSize ;
      if ( DMS_INVALID_OFFSET == extInfo._firstRecordOffset )
      {
         SDB_ASSERT( DMS_INVALID_OFFSET ==
                     extInfo._lastRecordOffset,
                     "last record offset should be invalid" ) ;
         extInfo._firstRecordOffset = sizeof( dmsExtent ) ;
         extInfo._lastRecordOffset = sizeof( dmsExtent ) + totalSize ;
      }
      else
      {
         SDB_ASSERT( DMS_INVALID_OFFSET != extInfo._firstRecordOffset,
                     "first record offset should not be invalid" ) ;
         extInfo._lastRecordOffset += totalSize ;
      }

      extInfo._totalOrgDataLen += recordData.orgLen() ;
      extInfo._totalDataLen += recordData.len() ;
   }

   OSS_INLINE void _dmsStorageDataCapped::_updateStatInfo( dmsMBContext *context,
                                                           UINT32 recordSize,
                                                           const dmsRecordData &recordData )
   {
      _updateWorkExtStat( _workExtInfo[ context->mbID() ],
                          recordSize, recordData ) ;
      _updateCLStat( _mbStatInfo[ context->mbID() ], recordSize, recordData ) ;
   }

   OSS_INLINE BOOLEAN _dmsStorageDataCapped::_exceedLimit( dmsMBContext *context,
                                                           UINT32 size )
   {
      const dmsMBStatInfo *mbStatInfo = getMBStatInfo( context->mbID() ) ;
      SDB_ASSERT( mbStatInfo, "mbStatInfo should not be NULL" ) ;

      // Check total data size and record number.
      if ( ( (INT64)( mbStatInfo->_totalDataLen + size) > mbStatInfo->_maxSize )
           ||
           ( mbStatInfo->_maxRecNum > 0 &&
             ( (INT64)mbStatInfo->_totalRecords >= mbStatInfo->_maxRecNum  ) ) )
      {
         return TRUE ;
      }

      return FALSE ;
   }

   OSS_INLINE void _dmsStorageDataCapped::_getExtLIDAndOffsetByLID( INT64 logicalID,
                                                                    dmsExtentID &extID,
                                                                    dmsOffset &offset )
   {
      if ( logicalID < 0 )
      {
         extID = DMS_INVALID_EXTENT ;
         offset = 0 ;
      }

      extID = logicalID / DMS_CAP_EXTENT_BODY_SZ ;
      offset = logicalID % DMS_CAP_EXTENT_BODY_SZ + sizeof( dmsExtent ) ;
   }

   OSS_INLINE dmsExtentID _dmsStorageDataCapped::_logicID2ExtID( dmsMBContext *context,
                                                                 INT64 logicalID )
   {
      dmsExtRW extRW ;
      const dmsExtent *extent = NULL ;
      dmsExtentID extentID = DMS_INVALID_EXTENT ;
      dmsExtentID extLID = DMS_INVALID_EXTENT ;
      dmsOffset offset = 0 ;

      dmsExtentInfo *extentInfo =  getWorkExtInfo( context->mbID() ) ;

      if ( logicalID < 0 )
      {
         extentID = DMS_INVALID_EXTENT ;
         goto error ;
      }

      _getExtLIDAndOffsetByLID( logicalID, extLID, offset ) ;

      extentID = context->mb()->_firstExtentID ;

      do
      {
         extRW = extent2RW( extentID, context->mbID() ) ;
         extRW.setNothrow( TRUE ) ;
         extent = extRW.readPtr<dmsExtent>() ;
         if ( extLID < extent->_logicID )
         {
            extentID = DMS_INVALID_EXTENT ;
            goto error ;
         }
         else if ( extLID == extent->_logicID )
         {
            // Found the target extent.
            break ;
         }
         else if ( extentID == extentInfo->getID() )
         {
            // If hit the working extent, and still not found, stop.
            extentID = DMS_INVALID_EXTENT ;
            break ;
         }
         else
         {
            // Recycle the current extent.
            extentID = extent->_nextExtent ;
         }
      } while ( DMS_INVALID_EXTENT != extentID ) ;

   done:
      return extentID ;
   error:
      goto done ;
   }
 }

#endif /* DMSSTORAGE_DATACAPPED_HPP */

