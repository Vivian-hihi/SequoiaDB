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
#define DMS_CAP_CL_MIN_SZ                 DMS_CAP_EXTENT_SZ
#define DMS_CAP_EXTENT_BODY_SZ            ( DMS_CAP_EXTENT_SZ - DMS_EXTENT_METADATA_SZ )
#define DMS_CAP_EXTENT_PAGE_NUM           \
   (DMS_CAP_EXTENT_SZ >> pageSizeSquareRoot())

// Default size threshold of capped collection is 30GB.
// Default record number threshold is set to 0, which means no limit on that.
#define DMS_DFT_CAPPEDCL_SIZE             (30 * 1024 * 1024 * 1024LL)
#define DMS_DFT_CAPPEDCL_RECNUM           0

#define DMS_INVALID_LOGICALID             (-1)

#pragma pack(1)
   class _LogicalIDToInsert : public SDBObject
   {
   public :
      CHAR  _type ;
      CHAR  _id[4] ; // _id + '\0'
      INT64 _logicalID ;
      _LogicalIDToInsert ()
      {
         _type = (CHAR)NumberLong ;
         _id[0] = '_' ;
         _id[1] = 'i' ;
         _id[2] = 'd' ;
         _id[3] = 0 ;
         _logicalID = DMS_INVALID_LOGICALID ;
         SDB_ASSERT ( sizeof ( _LogicalIDToInsert) == 13,
                      "LogicalIDToInsert should be 13 bytes" ) ;
      }
   } ;
   typedef class _LogicalIDToInsert LogicalIDToInsert ;

   /*
      _idToInsert define
   */
   class _LogicalIDToInsertEle : public BSONElement
   {
   public :
      _LogicalIDToInsertEle( CHAR* x ) : BSONElement((CHAR*) ( x )){}
   } ;
   typedef class _LogicalIDToInsertEle LogicalIDToInsertEle ;
#pragma pack()

   class _pmdEDUCB ;
   class _mthModifier ;

   struct _dmsCappedCLOptions
   {
      INT64 _maxSize ;
      INT64 _maxRecNum ;

      _dmsCappedCLOptions()
      {
         _maxSize = DMS_DFT_CAPPEDCL_SIZE ;
         _maxRecNum = DMS_DFT_CAPPEDCL_RECNUM ;
      }
   } ;
   typedef _dmsCappedCLOptions dmsCappedCLOptions ;

   // Information of the working extent. Working extent is the one which is
   // used for insertion currently.
   struct _dmsExtentInfo
   {
      dmsExtentID    _id ;
      INT64          _extLogicID ;
      UINT32         _recCount ;
      INT32          _freeSpace ;
      dmsOffset      _firstRecordOffset ;
      dmsOffset      _lastRecordOffset ;
      UINT32         _writePos ; // Currently write position in the working
                                 // extent data area( the extent header
                                 // excluded). Always 4 byte aligned.
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
         _writePos = 0 ;
      }

      const dmsExtentID getID() const { return _id ; }

      dmsOffset getNextRecOffset() const
      {
         return _writePos + DMS_EXTENT_METADATA_SZ ;
      }

      INT64 getRecordLogicID()
      {
         return ( _extLogicID * DMS_CAP_EXTENT_BODY_SZ + _writePos ) ;
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

      virtual INT32 dumpExtOptions( dmsMBContext *context,
                                    BSONObj &extOptions ) ;

      OSS_INLINE dmsExtentInfo* getWorkExtInfo( UINT16 mbID ) ;

   private:
      virtual const CHAR* _getEyeCatcher() const ;
      virtual INT32 _onOpened() ;
      virtual void _onClosed() ;

      virtual INT32 _prepareAddCollection( const BSONObj *extOption,
                                           dmsExtentID &extOptExtent,
                                           UINT16 &extentPageNum ) ;

      virtual INT32 _onAddCollection( const BSONObj *extOption,
                                      dmsExtentID extOptExtent,
                                      UINT32 extentSize,
                                      UINT16 collectionID ) ;

      virtual void _onAllocExtent( dmsMBContext *context,
                                   dmsExtent * extAddr,
                                   SINT32 extentID ) ;

      virtual INT32 _prepareInsertData( const BSONObj &record,
                                        BOOLEAN mustOID,
                                        pmdEDUCB *cb,
                                        dmsRecordData &recordData,
                                        BOOLEAN &memReallocate ) ;

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

      INT32 _parseExtendOptions( const BSONObj *extOptions,
                                 dmsCappedCLOptions &options ) ;

      INT32 _recycleOneExtent( dmsMBContext *context ) ;
      INT32 _recycleWorkExt( dmsMBContext *context, dmsExtentID extID ) ;
      INT32 _recycleActiveExt( dmsMBContext *context,
                               dmsExtentID extID ) ;

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
                            pmdEDUCB *cb,
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
                                INT64 &totalSize,
                                BOOLEAN freeRecord = FALSE,
                                BOOLEAN endInclude = TRUE ) ;

   private:
      dmsCappedCLOptions *_options[ DMS_MME_SLOTS ] ;
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
         extInfo._firstRecordOffset = DMS_EXTENT_METADATA_SZ ;
         extInfo._lastRecordOffset = DMS_EXTENT_METADATA_SZ ;
      }
      else
      {
         SDB_ASSERT( DMS_INVALID_OFFSET != extInfo._firstRecordOffset,
                     "first record offset should not be invalid" ) ;
         extInfo._lastRecordOffset =
            extInfo._writePos + DMS_EXTENT_METADATA_SZ ;
      }
      extInfo._writePos =
         ossRoundUpToMultipleX( extInfo._writePos + totalSize, 4 ) ;
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
      INT64 expectSize = 0 ;
      INT64 expectRecNum = 0 ;
      const dmsMBStatInfo *mbStatInfo = getMBStatInfo( context->mbID() ) ;
      SDB_ASSERT( mbStatInfo, "mbStatInfo should not be NULL" ) ;

      expectSize = mbStatInfo->_totalDataLen + size ;
      expectRecNum = mbStatInfo->_totalRecords ;
      // Check total data size and record number.
      if ( ( expectSize > _options[context->mbID()]->_maxSize ) ||
           ( _options[context->mbID()]->_maxRecNum > 0 &&
             ( expectRecNum >= _options[context->mbID()]->_maxRecNum ) ) )
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
      offset = logicalID % DMS_CAP_EXTENT_BODY_SZ + DMS_EXTENT_METADATA_SZ ;
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

      while ( DMS_INVALID_EXTENT != extentID )
      {
         extRW = extent2RW( extentID, context->mbID() ) ;
         extRW.setNothrow( TRUE ) ;
         extent = extRW.readPtr<dmsExtent>() ;
         if ( !extent || extLID < extent->_logicID )
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
            extentID = extent->_nextExtent;
         }
      }

   done:
      return extentID ;
   error:
      goto done ;
   }
 }

#endif /* DMSSTORAGE_DATACAPPED_HPP */

