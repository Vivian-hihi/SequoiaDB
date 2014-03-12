/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dmsStorageBase.hpp

   Descriptive Name = Data Management Service Storage Unit Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/08/2013  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef DMSSTORAGE_BASE_HPP_
#define DMSSTORAGE_BASE_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossMmap.hpp"
#include "dms.hpp"
#include "dmsExtent.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "../bson/bson.h"
#include "../bson/bsonobj.h"
#include "../bson/oid.h"
#include "dmsSMEMgr.hpp"

#include <string>

using namespace std ;
using namespace bson ;

namespace engine
{

   #define DMS_HEADER_EYECATCHER_LEN         (8)
   #define DMS_SU_NAME_SZ                    DMS_COLLECTION_SPACE_NAME_SZ

#pragma pack(4)
   /*
      _dmsStorageInfo defined
   */
   struct _dmsStorageInfo
   {
      SINT32      _pageSize ;
      CHAR        _suName [ DMS_SU_NAME_SZ + 1 ] ; // storage unit file name is
                                                   // foo.0 / foo.1, where foo
                                                   // is suName, and 0/1 are
                                                   // _sequence
      UINT32      _sequence ;
      UINT64      _secretValue ;

      _dmsStorageInfo ()
      {
         _pageSize      = DMS_PAGE_SIZE_DFT ;
         ossMemset( _suName, 0, sizeof( _suName ) ) ;
         _sequence      = 0 ;
         _secretValue   = 0 ;
      }
   };
   typedef _dmsStorageInfo dmsStorageInfo ;

   /*
      Storage Unit Header : 65536(64K)
   */
   struct _dmsStorageUnitHeader : public SDBObject
   {
      CHAR   _eyeCatcher[DMS_HEADER_EYECATCHER_LEN] ;
      UINT32 _version ;
      UINT32 _pageSize ;                                 // size of byte
      UINT32 _storageUnitSize ;                          // all file pages
      CHAR   _name [ DMS_SU_NAME_SZ+1 ] ;                // storage unit name
      UINT32 _sequence ;                                 // storage unit seq
      UINT32 _numMB ;                                    // Number of MB
      UINT32 _MBHWM ;
      UINT32 _pageNum ;                                  // current page number
      UINT64 _secretValue ;                              // with the index
      CHAR   _pad [ 65364 ] ;

      _dmsStorageUnitHeader()
      {
         SDB_ASSERT( DMS_PAGE_SIZE_MAX == sizeof( _dmsStorageUnitHeader ),
                     "_dmsStorageUnitHeader size must be 64K" ) ;
         ossMemset( this, 0, DMS_PAGE_SIZE_MAX ) ;
      }
   } ;
   typedef _dmsStorageUnitHeader dmsStorageUnitHeader ;
   #define DMS_HEADER_SZ   sizeof(dmsStorageUnitHeader)

   #define DMS_SME_LEN                 (DMS_MAX_PG/8)
   #define DMS_SME_FREE                 0
   #define DMS_SME_ALLOCATED            1

   /* Space Management Extent, 1 bit for 1 page */
   struct _dmsSpaceManagementExtent : public SDBObject
   {
      CHAR _smeMask [ DMS_SME_LEN ] ;

      _dmsSpaceManagementExtent()
      {
         SDB_ASSERT( DMS_SME_LEN == sizeof( _dmsSpaceManagementExtent ),
                     "SME size error" )
         ossMemset( this, DMS_SME_FREE, sizeof( _dmsSpaceManagementExtent ) ) ;
      }
      CHAR getBitMask( UINT32 bitNum ) const
      {
         SDB_ASSERT( bitNum < DMS_MAX_PG, "Invalid bitNum" )
         return (_smeMask[bitNum >> 3] >> (7 - (bitNum & 7))) & 1 ;
      }
      void freeBitMask( UINT32 bitNum )
      {
         SDB_ASSERT( bitNum < DMS_MAX_PG, "Invalid bitNum" )
         _smeMask[bitNum >> 3] &= ~( 1 << (7 - (bitNum & 7))) ;
      }
      void setBitMask( UINT32 bitNum )
      {
         SDB_ASSERT( bitNum < DMS_MAX_PG, "Invalid bitNum" )
         _smeMask[bitNum >> 3] |= ( 1 << (7 - (bitNum & 7))) ;
      }
   } ;
   typedef _dmsSpaceManagementExtent dmsSpaceManagementExtent ;
   #define DMS_SME_SZ  sizeof(dmsSpaceManagementExtent)

#pragma pack()

   void smeMask2String( CHAR state, CHAR *pBuffer, INT32 buffSize ) ;

   /*
      _dmsContext define
   */
   class _dmsContext : public SDBObject
   {
      public:
         _dmsContext () {}
         virtual ~_dmsContext () {}

      public:
         virtual string toString () const = 0 ;
         virtual INT32  pause () = 0 ;
         virtual INT32  resume () = 0 ;

   };
   typedef _dmsContext  dmsContext ;

   #define DMS_SU_FILENAME_SZ       ( DMS_SU_NAME_SZ + 15 )
   #define DMS_HEADER_OFFSET        ( 0 )
   #define DMS_SME_OFFSET           ( DMS_HEADER_OFFSET + DMS_HEADER_SZ )

   /* 
      Storage Unit Base
   */
   class _dmsStorageBase : public ossMmapFile
   {
      friend class _dmsExtendSegmentJob ;

      public:
         _dmsStorageBase( const CHAR *pSuFileName,
                          dmsStorageInfo *pInfo ) ;
         virtual ~_dmsStorageBase() ;

         const CHAR*    getSuFileName() const ;
         const CHAR*    getSuName() const ;
         const dmsStorageUnitHeader *getHeader() { return _dmsHeader ; }
         const dmsSpaceManagementExtent *getSME () { return _dmsSME ; }
         dmsSMEMgr *getSMEMgr () { return &_smeMgr ; }

         inline UINT64  dataSize () const ;
         inline UINT64  fileSize () const ;

         inline UINT32  pageSize () const ;
         inline UINT32  pageSizeSquareRoot () const ;
         inline UINT32  segmentPages () const ;
         inline UINT32  segmentPagesSquareRoot () const ;
         inline UINT32  pageNum () const ;
         inline INT32   maxSegID () const ;
         inline UINT32  dataStartSegID () const ;
         inline BOOLEAN isTempSU () const { return _isTempSU ; }

         inline UINT32  extent2Segment( dmsExtentID extentID,
                                        UINT32 *pSegOffset = NULL ) ;
         inline dmsExtentID segment2Extent( UINT32 segID,
                                            UINT32 segOffset = 0 ) ;

         inline ossValuePtr extentAddr ( INT32 extentID ) ;
         inline dmsExtentID extentID ( ossValuePtr extendAddr ) ;

      public:
         INT32 openStorage ( const CHAR *pPath, BOOLEAN createNew = TRUE,
                             BOOLEAN delWhenExist = FALSE ) ;
         void  closeStorage () ;
         INT32 removeStorage() ;
         BOOLEAN isOpened() const { return ossMmapFile::_opened ; }
         virtual void  syncMemToMmap () {}

      private:
         virtual UINT64 _dataOffset()  = 0 ;
         virtual const CHAR* _getEyeCatcher() const = 0 ;
         virtual UINT32 _curVersion() const = 0 ;
         virtual INT32  _checkVersion( dmsStorageUnitHeader *pHeader ) = 0 ;
         virtual INT32  _onCreate( OSSFILE *file, UINT64 curOffSet ) = 0 ;
         virtual INT32  _onMapMeta( UINT64 curOffSet ) = 0 ;
         virtual void   _onClosed() = 0 ;
         virtual UINT32 _extendThreshold() const ;

      protected:
         // No space will extent new segment
         INT32    _findFreeSpace ( UINT16 numPages, SINT32 &foundPage,
                                   dmsContext *context ) ;
         INT32    _releaseSpace ( SINT32 pageStart, UINT16 numPages ) ;

         UINT32   _totalFreeSpace() ;

         INT32    _writeFile( OSSFILE &file, const CHAR *pData,
                              INT64 dataLen ) ;

      private:
         INT32    _initializeStorageUnit () ;
         void     _initHeader ( dmsStorageUnitHeader *pHeader ) ;
         INT32    _validateHeader( dmsStorageUnitHeader *pHeader ) ;
         INT32    _extendSegments ( UINT32 numSeg ) ;
         INT32    _preExtendSegment() ;

      protected:
         dmsStorageUnitHeader          *_dmsHeader ;     // 64KB
         dmsSpaceManagementExtent      *_dmsSME ;        // 8MB
         CHAR                          _suFileName[ DMS_SU_FILENAME_SZ + 1 ] ;

         dmsStorageInfo                *_pStorageInfo ;

      private:
         ossSpinSLatch                 _segmentLatch ;
         dmsSMEMgr                     _smeMgr ;
         UINT32                        _dataSegID ;
         UINT32                        _pageNum ;
         INT32                         _maxSegID ;
         UINT32                        _segmentPages ;
         UINT32                        _segmentPagesSquare ;
         UINT32                        _pageSizeSquare ;
         CHAR                          _fullPathName[ OSS_MAX_PATHSIZE + 1 ] ;
         BOOLEAN                       _isTempSU ;

   } ;
   typedef _dmsStorageBase dmsStorageBase ;

   /*
      _dmsStorageBase inline functions :
   */
   inline UINT32 _dmsStorageBase::pageSize () const
   {
      if ( _pStorageInfo )
      {
         return _pStorageInfo->_pageSize ;
      }
      return DMS_INVALID_PAGESIZE ;
   }
   inline UINT32 _dmsStorageBase::pageSizeSquareRoot () const
   {
      return _pageSizeSquare ;
   }
   inline UINT32 _dmsStorageBase::segmentPages () const
   {
      return _segmentPages ;
   }
   inline UINT32 _dmsStorageBase::segmentPagesSquareRoot () const
   {
      return _segmentPagesSquare ;
   }
   inline UINT32 _dmsStorageBase::pageNum () const
   {
      return _pageNum ;
   }
   inline INT32 _dmsStorageBase::maxSegID () const
   {
      return _maxSegID ;
   }
   inline UINT32 _dmsStorageBase::dataStartSegID () const
   {
      return _dataSegID ;
   }
   inline UINT64 _dmsStorageBase::dataSize () const
   {
      return (UINT64)_pageNum << _pageSizeSquare ;
   }
   inline UINT64 _dmsStorageBase::fileSize () const
   {
      if ( _dmsHeader )
      {
         return (UINT64)_dmsHeader->_storageUnitSize << _pageSizeSquare ;
      }
      return 0 ;
   }
   inline UINT32 _dmsStorageBase::extent2Segment( dmsExtentID extentID,
                                                  UINT32 * pSegOffset )
   {
      if ( pSegOffset )
      {
         // the same with : extentID % _segmentPages
         *pSegOffset = extentID & (( 1 << _segmentPagesSquare ) - 1 ) ;
      }
      // the same with: extentID / _segmentPages + _dataSegID
      return ( extentID >> _segmentPagesSquare ) + _dataSegID ;
   }
   inline dmsExtentID _dmsStorageBase::segment2Extent( UINT32 segID,
                                                       UINT32 segOffset )
   {
      if ( segID < _dataSegID )
      {
         return DMS_INVALID_EXTENT ;
      }
      // the same with: ( segID - _dataSegID ) * _segmentPages + segOffset
      return (( segID - _dataSegID ) << _segmentPagesSquare ) + segOffset ;
   }
   inline ossValuePtr _dmsStorageBase::extentAddr( INT32 extentID )
   {
      if ( DMS_INVALID_EXTENT == extentID )
      {
         return 0 ;
      }
      UINT32 segOffset = 0 ;
      UINT32 segID = extent2Segment( extentID, &segOffset ) ;
      if ( (INT32)segID > _maxSegID )
      {
         return 0 ;
      }
      return _segments[ segID ]._ptr +
             (ossValuePtr)( segOffset << _pageSizeSquare ) ;
      // the same with: segOffset * _segmentPages
   }
   inline dmsExtentID _dmsStorageBase::extentID( ossValuePtr extendAddr )
   {
      if ( 0 == extendAddr || _maxSegID < 0 )
      {
         return DMS_INVALID_EXTENT ;
      }
      // find seg ID
      INT32 segID = 0 ;
      UINT32 segOffset = 0 ;
      while ( segID <= _maxSegID )
      {
         if ( _segments[segID]._ptr >= extendAddr &&
              extendAddr < _segments[segID]._ptr +
                           (ossValuePtr)_segments[segID]._length )
         {
            segOffset = (UINT32)((extendAddr - _segments[segID]._ptr) >>
                                  _pageSizeSquare) ;
            break ;
         }
         ++segID ;
      }
      if ( segID > _maxSegID )
      {
         return DMS_INVALID_EXTENT ;
      }
      return segment2Extent( (UINT32)segID, segOffset ) ;
   }

   /*
      DMS Other define
   */
   #define DMS_MON_OP_COUNT_INC( _pMonAppCB_, op, delta )  \
   {                                                       \
      if ( NULL != _pMonAppCB_ )                           \
      {                                                    \
         _pMonAppCB_->monOperationCountInc( op, delta ) ;  \
      }                                                    \
   }

   #define DMS_MON_OP_TIME_INC( _pMonAppCB_, op, delta )   \
   {                                                       \
      if ( NULL != _pMonAppCB_ )                           \
      {                                                    \
         _pMonAppCB_->monOperationTimeInc( op, delta ) ;   \
      }                                                    \
   }

   #define DMS_MON_CONTEXT_COUNT_INC( _monContextCB_, op, delta ) \
   {                                                               \
      if ( NULL != _monContextCB_ )                                \
      {                                                            \
         _monContextCB_->monOperationCountInc ( op, delta ) ;      \
      }                                                            \
   }

   #define DMS_MON_CONTEXT_TIME_INC( _monContextCB_, op, delta )  \
   {                                                               \
      if ( NULL != _monContextCB_ )                                \
      {                                                            \
         _monContextCB_->monOperationTimeInc ( op, delta ) ;       \
      }                                                            \
   }

   /****************************************************************************
    * Specify the matrix for collection flag and access type, returns TRUE means
    * access is allowed, otherwise return FALSE
    * AccessType:   Query  Fetch  Insert  Update  Delete  Truncate CRT-IDX  DROP-IDX
    *  FREE           N      N       N       N       N       N       N         N
    *  NORMAL         Y      Y       Y       Y       Y       Y       Y         Y
    *  DROPPED        N      N       N       N       N       N       N         N
    *  OFFLINE REORG  N (only alloed in shadow copy phase )
    *                        N       N       N       N       N ( only allowed in
    *  truncate phase )                                              N         N
    *  ONLINE REORG   Y      Y       Y       Y       Y       Y       Y         Y
    *  Load           Y      Y       Y       Y       Y       N       Y         Y
    ***************************************************************************/
   BOOLEAN dmsAccessAndFlagCompatiblity ( UINT16 collectionFlag,
                                          DMS_ACCESS_TYPE accessType ) ;

}

#endif //DMSSTORAGE_BASE_HPP_

